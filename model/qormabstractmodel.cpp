#include "qormabstractmodel.h"
#include "core/qormloader.h"
#include "qormabstractmodel_p.h"

#include <QOrm>

#include "core/qormattributeinfo.h"


QOrmAbstractModel::QOrmAbstractModel(QOrmTableInfo *metaTable, QOrmBasicLoader *loader) :
    QAbstractItemModel(),
    d(new QOrmAbstractModelPrivate())
{
    d->meta = metaTable;
    d->loadFromDB = false;
    int j = 0;
    for(int i = metaTable->metaObject()->propertyOffset();
        i < metaTable->metaObject()->propertyCount(); i++)
    {
        QMetaProperty prop = metaTable->metaObject()->property(i);
        d->roles.insert(Qt::UserRole+1+j, prop.name());
        j++;
    }

    if (loader) d->ld = loader;
    else d->ld = new QOrmBasicLoader(metaTable);
}

void QOrmAbstractModel::objectDataChanged(QOrmObject *obj, int attr)
{
    int r = 0, c = d->list.count();
    if (attr < 0 ) attr = d->meta->attributesCount() - 1;

    r = d->list.indexOf(obj);
    if (r < c) dataChanged(index(r,0), index(r, attr));
}

void QOrmAbstractModel::disconnectAll()
{
    for(QOrmObject* &o : d->list) {
        //disconnect(o, &QOrmObject::dataChanged, this, &QOrmAbstractModel::objectDataChanged);
        o->disconnect(this);
        o->deref();
    }
    for(QOrmObject* &o : d->deleted) {
        //disconnect(o, &QOrmObject::dataChanged, this, &QOrmAbstractModel::objectDataChanged);
        o->disconnect(this);
        o->deref();
    }
}

void QOrmAbstractModel::reconnectAll()
{
    for(QOrmObject* &o : d->list) {
        connect(o, &QOrmObject::dataChanged, this, &QOrmAbstractModel::objectDataChanged);
        connect(o, &QOrmObject::destroyed, this, [=](QObject *o){ this->remove((QOrmObject*)o); });
        o->ref();
    }
}

void QOrmAbstractModel::replace(QOrmObjectList nlst)
{
    for(QOrmObject* &o : nlst) {
        connect(o, &QOrmObject::dataChanged, this, &QOrmAbstractModel::objectDataChanged);
        connect(o, &QOrmObject::destroyed, this, [=](QObject *o){ this->remove((QOrmObject*)o); });
        o->ref();
    }
    for(QOrmObject* &o : d->list) {
        o->disconnect(this);
        o->deref();
    }
    d->list = nlst;
}

QOrmAbstractModel::~QOrmAbstractModel()
{
    clear();
    delete d;
}

QOrmTableInfo *QOrmAbstractModel::metaTable() const
{
    return d->meta;
}

QModelIndex QOrmAbstractModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row >= 0 && row < d->list.count())
        return createIndex(row, column, d->list.at(row));

    return QModelIndex();
}

QModelIndex QOrmAbstractModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int QOrmAbstractModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return d->meta->propertyCount();
}

int QOrmAbstractModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)


    return d->list.count();
}

QVariant QOrmAbstractModel::data(const QModelIndex &index, int role) const
{
    QOrmAbstractModel *mdl = QtOrm::model_cast(index.model());
    QOrmObject *dt = (QOrmObject*)index.internalPointer();
    if (index.isValid() && mdl && dt)
    {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return d->meta->attribute(index.column()).read(dt);
            case Qt::UserRole: return dt->primaryKey();
        }

        if (role > Qt::UserRole)
        {
            int c = role - 1 - (int)Qt::UserRole +  dt->metaObject()->propertyOffset();
            QVariant v = dt->metaObject()->property(c).read(dt);
            return v;
        }
    }

    return QVariant();
}
bool QOrmAbstractModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QOrmAbstractModel *mdl = QtOrm::model_cast(index.model());
    QOrmObject *dt = (QOrmObject*)index.internalPointer();
    if (index.isValid() && value.isValid() && mdl && dt)
    {
        int c = -1;
        if (role == Qt::DisplayRole || role == Qt::EditRole)
            c = index.column();
        if (role > Qt::UserRole)
            c = role - 1 -Qt::UserRole;
        if (c < 0) return false;

        QOrmAttributeInfo a = d->meta->attribute(c);
        if (!a.isValid()) return false;

        dt->blockSignals(true);
        bool b = a.write(dt, value);
        if (b) dataChanged(index, index);
        dt->blockSignals(false);
        return b;
    }


    return false;
}
QVariant QOrmAbstractModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation != Qt::Horizontal)
        return QVariant();

    QOrmTableInfo *m = d->meta;
    if (section >= 0 && section < m->propertyCount())
        return m->attribute(section).name();

    return QVariant();
}

Qt::ItemFlags QOrmAbstractModel::flags(const QModelIndex & index ) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (d->deleted.contains((QOrmObject*)index.internalPointer()))
        return Qt::ItemIsSelectable;

    QOrmAttributeInfo a = d->meta->attribute(index.column());
    if (!a.isValid() || a.isForeignKey()) return Qt::ItemIsSelectable;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool QOrmAbstractModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(parent, row + 1, row + count);
    for(int i = 0; i < count; i++)
    {
        QOrmObject* o = d->meta->create();
        d->list.insert(row + i + 1, o);
        o->ref();
    }
    endInsertRows();
    return true;
}

bool QOrmAbstractModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(parent, row, row + count - 1);
    for(int i = 0; i < count; i++)
    {
        QOrmObject *o = d->list.at(row + i);
        d->list.removeAt(row + i);
        o->deref();
        o->disconnect(this);
    }
    endRemoveRows();
    return true;
}

bool QOrmAbstractModel::deleteRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    for (int i = 0; i < count; i++)
    {
        QModelIndex in = index(row + i, 0, parent);
        if (!in.internalPointer()) continue;

        d->deleted.append((QOrmObject*)in.internalPointer());
        dataChanged(in, index(row + i, d->meta->attributesCount(), parent));
    }
    return true;
}

void QOrmAbstractModel::setRootIndex(const QModelIndex &parent)
{
    if (parent.internalPointer() == d->rootIndex.internalPointer())
        return;

    beginResetModel();

    QOrmAbstractModel *mdl = QtOrm::model_cast(parent.model());
    QOrmObject *dt = (QOrmObject*)parent.internalPointer();
    if (parent.isValid() && mdl && dt)
    {
        d->ld->bindValue(dt);
        QOrmObjectList tmp =  d->ld->loadAll();
        d->rootIndex = parent;

        replace(tmp);
    }
    else
    {
        for(QOrmObject* &o : d->list) {
            o->disconnect(this);
            o->deref();
        }
        d->list.clear();
        d->rootIndex = QModelIndex();
    }

    endResetModel();

    emit rootIndexChanged(d->rootIndex);
}

QModelIndex QOrmAbstractModel::rootIndex() const
{
    return d->rootIndex;
}

void QOrmAbstractModel::load()
{
    beginResetModel();
    QList<QOrmObject*> tmp = d->ld->loadAll();
    replace(tmp);
    endResetModel();
}

void QOrmAbstractModel::submitAll()
{
    QOrmSaver saver;
    QList<QOrmObject*> todel;
    for(QOrmObject* &o : d->list)
    {
        if (d->deleted.contains(o))
        {
            todel.append(o);
            saver.pushToDelete(o);
        }
        else
            saver.pushToSave(o);
    }

    if(!saver.run())
    {
        qDebug()<<saver.lastQuery();
        qDebug()<<saver.lastErrors();
        return;
    }

    beginResetModel();
    for(QOrmObject* o : todel) {
        d->list.removeOne(o);
        o->disconnect(this);
        o->deref();
    }
    endResetModel();
}

int QOrmAbstractModel::count()
{
    return d->list.count();
}

QOrmObject* QOrmAbstractModel::at(int row) const
{
    if (row >= 0 && row < d->list.count()) return d->list.at(row);

    return NULL;
}

void QOrmAbstractModel::append(QOrmObject* obj)
{
    if (!obj) return;
    if (d->list.contains(obj)) return;

    int c = d->list.count();

    beginInsertRows(QModelIndex(), c, c);
    d->list.append(obj);
    obj->ref();
    endInsertRows();

    connect(obj, &QOrmObject::dataChanged, this, &QOrmAbstractModel::objectDataChanged);
    connect(obj, &QOrmObject::destroyed, this, [=](QObject *o){ this->remove((QOrmObject*)o); });

    emit dataChanged(index(c,0), index(c, 0));
}

void QOrmAbstractModel::remove(QOrmObject* obj)
{
    if (!obj) return;
    int c = d->list.indexOf(obj);
    if (c < 0) return;

    if (c >= 0)
    {
        beginRemoveRows(QModelIndex(), c, c);
        d->list.removeAt(c);
        obj->disconnect(this);
        obj->deref();
        endRemoveRows();
    }
}

void QOrmAbstractModel::deleteOne(QOrmObject* obj)
{
    if (!obj) return;
    if (!obj->isDeleted()) return;

    d->deleted.append(obj);
}

void QOrmAbstractModel::clear()
{
    d->rootIndex = QModelIndex();
    disconnectAll();
    d->list.clear();
    d->deleted.clear();

    beginResetModel();
    endResetModel();
}

QHash<int, QByteArray> QOrmAbstractModel::roleNames() const
{
    return d->roles;
}
