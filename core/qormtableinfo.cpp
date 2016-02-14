#include "qormtableinfo.h"
#include "private/qormtableinfo_p.h"
#include "qormattributeinfo.h"

#include "qormobject.h"
#include "qormtable.h"
#include "qorm.h"
#include "model/qormabstractmodel.h"

#include <QtCore>

QOrmTableInfo::QOrmTableInfo(QString tableName, const QMetaObject *mobj, QtOrm::QORMCreateFn cfn) :
    d(new Private())
{
    d->table = tableName;
    d->fnc = cfn;
    d->mobj = (QMetaObject*)mobj;
    d->cache = new QOrmTable();
}

QString QOrmTableInfo::database()
{
    return d->db;
}

QString QOrmTableInfo::table()
{
    return d->table;
}

QOrmTable *QOrmTableInfo::cache()
{
    return d->cache;
}

int QOrmTableInfo::attributesCount()
{
    return d->attrs.count();
}

int QOrmTableInfo::indexCount()
{
    return d->indexes.count();
}

int QOrmTableInfo::foreignKeysCount()
{
    return d->fks.count();
}

QOrmAttributeInfo QOrmTableInfo::attribute(int attr)
{
    if (attr < 0 || attr >= d->attrs.count())
        return QOrmAttributeInfo();

    return *d->attrs[attr];
}

QOrmAttributeInfo QOrmTableInfo::index(int ind)
{
    if (ind < 0 || ind >= d->indexes.count())
        return QOrmAttributeInfo();

    return *d->indexes[ind];
}

QOrmAttributeInfo QOrmTableInfo::foreignKey(int fk)
{
    if (fk < 0 || fk >= d->fks.count())
        return QOrmAttributeInfo();

    return *d->fks[fk];
}

QOrmAttributeInfo QOrmTableInfo::findForeignkey(QOrmTableInfo *other)
{
    foreach (QOrmAttributeInfo *a, d->fks) {
        if (a->refTable() == other)
            return *a;
    }

    return QOrmAttributeInfo();
}

QOrmAttributeInfo QOrmTableInfo::findAttribute(QString name)
{
    foreach (QOrmAttributeInfo *a, d->attrs) {
        if (a->name() == name)
            return *a;
    }

    return QOrmAttributeInfo();
}

QMetaObject *QOrmTableInfo::metaObject() const
{
    return d->mobj;
}

int QOrmTableInfo::propertyCount()
{
    return d->mobj->propertyCount() - d->mobj->propertyOffset();
}

QOrmAttributeInfo QOrmTableInfo::property(int i)
{
    //return d->mobj->property(d->mobj->propertyOffset() + i).name();
    QByteArray m = d->mobj->property(d->mobj->propertyOffset() + i).name();
    foreach (QOrmAttributeInfo *a, d->attrs) {
        if (a->metaProperty().name() == m)
            return *a;
    }
    return QOrmAttributeInfo();
}

int QOrmTableInfo::indexAttribute(QString name)
{
    int i = 0;
    foreach (QOrmAttributeInfo *a, d->attrs) {
        if (a->name() == name)
            return i;
        i++;
    }

    return -1;
}

QOrmChildrens QOrmTableInfo::indexChildren(QOrmTableInfo *table)
{
    for(QOrmChildrens *c : d->childrens)
        if (c->table() == table)
            return *c;

    return QOrmChildrens();
}

QVector<QOrmChildrens> QOrmTableInfo::childrens()
{
    QVector<QOrmChildrens> rt;
    for(QOrmChildrens *c : d->childrens)
        rt<<*c;
    return rt;
}

QOrmObject *QOrmTableInfo::create()
{
    if (d->fnc)
        return d->fnc();

    return 0;
}

void QOrmTableInfo::connectAttributes(QOrmObject *obj)
{
    QMetaObject amo = QOrmAttributeInfo::staticMetaObject;
    QMetaMethod mv = amo.method(amo.indexOfMethod("setValue()"));
    QMetaMethod mi = amo.method(amo.indexOfMethod("setIndex()"));
    QMetaMethod mf = amo.method(amo.indexOfMethod("setForeignkey()"));
    for(QOrmAttributeInfo *a : d->attrs)
    {
        if (a->pos() < 0) continue;

        int i = d->mobj->indexOfProperty(a->propertyName().toLatin1());
        QMetaProperty prop = d->mobj->property(i);
        if (!prop.hasNotifySignal()) continue;

        if (a->isForeignKey())
            d->mobj->connect(obj, prop.notifySignalIndex() - 2, a, mf.methodIndex());
        else {
            if (a->isPrimaryKey())
                d->mobj->connect(obj, prop.notifySignalIndex() - 2, a, mi.methodIndex());
            else
                d->mobj->connect(obj, prop.notifySignalIndex() - 2, a, mv.methodIndex());
        }
    }
}

class QOrmObject::Private
{
public:
    QOrmTableInfo *meta;
    QMap<QString, QList<QOrmObject*> > childrens;

    QVector<QVariant> indexes;
};


QString QOrmTableInfo::sqlSelectAll()
{
    return d->sqlS;
}

QString QOrmTableInfo::sqlSelect(QString where)
{
    return where.isEmpty()? d->sqlS : d->sqlS + " WHERE " + where;
}

QString QOrmTableInfo::sqlSelectByIndex()
{
    QOrmAttributeInfo *a = d->indexes.first();
    return QString("%1 WHERE `%2` = :%3").arg(d->sqlS, a->name(), a->name().replace(" ", "_"));
}

QString QOrmTableInfo::sqlInsert()
{
    return d->sqlI;
}

QString QOrmTableInfo::sqlUpdate()
{
    return d->sqlU;
}

QString QOrmTableInfo::sqlDelete()
{
    return d->sqlD;
}


QOrmAbstractModel *QtOrm::model_cast(const QAbstractItemModel *model)
{
    return qobject_cast<QOrmAbstractModel*>(const_cast<QAbstractItemModel*>(model));
}


QOrmObject* QtOrm::modelIndex_cast(const QModelIndex &index)
{
    QOrmAbstractModel *mld = model_cast(index.model());
    if (!mld) return NULL;

    return mld->at(index.row());
}
