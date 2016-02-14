#include "qormattributeinfo.h"
#include "private/qormattributeinfo_p.h"

#include "qormtableinfo.h"
#include "qormobject.h"
#include "qorm.h"

#include <QtCore>
#include <QOrmLoader>

class QOrmObject::Private
{
public:
    QVector<QOrmObject*> oldFks;
    QVector<QVariant> indexes;
};

QOrmAttributeInfo::QOrmAttributeInfo() :
    QObject(0),
    d(new Private())
{
}

QOrmAttributeInfo::QOrmAttributeInfo(const QOrmAttributeInfo &a) :
    QObject(0)
{
    d = a.d->clone();
}

QOrmAttributeInfo &QOrmAttributeInfo::operator =(const QOrmAttributeInfo &attr)
{
    return *this;
}

QOrmAttributeInfo::QOrmAttributeInfo(QString property) :
    QObject(0),
    d(new Private())
{
    d->name = property.toLatin1();
    d->property = property.toLatin1();
}

QOrmAttributeInfo::QOrmAttributeInfo(QString property, QString attribute) :
    QObject(0),
    d(new Private())
{
    d->name = attribute.toLatin1();
    d->property = property.toLatin1();
}

QOrmAttributeInfo::~QOrmAttributeInfo()
{
    delete d;
}

bool QOrmAttributeInfo::isValid() const
{
    return d->valid;
}

bool QOrmAttributeInfo::isWriteable() const
{
    return d->pos >= 0;
}

QString QOrmAttributeInfo::database()
{
    if (!d->table)
        return "";

    return d->table->database();
}

QString QOrmAttributeInfo::table()
{
    if (!d->table)
        return "";

    return d->table->table();
}

int QOrmAttributeInfo::pos()
{
    return d->pos;
}

int QOrmAttributeInfo::refcolumn()
{
    return d->fk;
}

QOrmAttributeInfo QOrmAttributeInfo::refAttribute()
{
    if (!d->reftable) return QOrmAttributeInfo();

    return d->reftable->attribute(d->fk);
}

int QOrmAttributeInfo::indexIndexes()
{
    return d->indexIndex;
}

int QOrmAttributeInfo::foreignKeyIndex()
{
    return d->fkindex;
}

QVariant::Type QOrmAttributeInfo::typeOfProperty()
{
    return d->metaProperty.type();
}

QMetaProperty QOrmAttributeInfo::metaProperty()
{
    return d->metaProperty;
}

QString QOrmAttributeInfo::propertyName()
{
    return d->property;
}

QVariant::Type QOrmAttributeInfo::typeFromString(QString type)
{
    static QMap<QString, QVariant::Type> types;
    if (types.isEmpty())
    {
        types["boolean"] = QVariant::Bool;
        types["bool"] = QVariant::Bool;
        types["int"] = QVariant::Int;
        types["numeric"] = QVariant::Int;
        types["unsigned int"] = QVariant::UInt;
        types["double"] = QVariant::Double;
        types["real"] = QVariant::Double;
        types["float"] = QVariant::Double;
        types["char"] = QVariant::String; //char
        types["character"] = QVariant::String; //char
        types["text"] = QVariant::String;
        types["varchar"] = QVariant::String;
        types["string"] = QVariant::String;
        types["blob"] =   QVariant::ByteArray;
        types["binary"] = QVariant::ByteArray;
        types["date"] = QVariant::Date;
        types["time"] = QVariant::Time;
        types["datetime"] = QVariant::DateTime;
    }

    return types.value(type, QVariant::Bool);
}

bool QOrmAttributeInfo::write(QOrmObject *obj, QVariant value)
{
    if(!d->metaProperty.isValid()) return false;

    if (value.type() == d->metaProperty.type())
    {
        d->metaProperty.write(obj, value);
        return true;
    }
    return false;
}

QVariant QOrmAttributeInfo::read(QOrmObject *obj)
{
    return d->metaProperty.read(obj);
}

QOrmObject *QOrmAttributeInfo::readForeignKey(QOrmObject *obj)
{
    if (QOrm::isOrmObject(d->metaProperty.userType()))
        return d->metaProperty.read(obj).value<QOrmObject*>();

    return 0;
}

bool QOrmAttributeInfo::writeForeingKey(QOrmObject *obj, QOrmObject *fko)
{
    if (!isForeignKey()) return false;
    //if (!fko) return false;

    if (obj->signalsBlocked())
    {
        QOrmObject *old = obj->d->oldFks.at(d->fkindex);
        obj->d->oldFks.replace(d->fkindex, fko);

        if (old)
        {
            old->deref();
            old->disconnect(obj);
        }

        if (fko)
        {
            fko->ref();
            writeIndex(obj, refAttribute().readIndex(fko));
            connect(fko, &QObject::destroyed, obj, &QOrmObject::fkDestroyed);
        }
        else
            writeIndex(obj, QVariant());
    }
    d->metaProperty.write(obj, QVariant::fromValue<QOrmObject*>(fko));

    return true;
}

QVariant QOrmAttributeInfo::readIndex(QOrmObject *obj)
{
    if (!isPrimaryKey()) return QVariant();

    if (isForeignKey())
    {
        QOrmObject* ofk = readForeignKey(obj);
        if (ofk) return refAttribute().read(ofk);
    }
    return obj->d->indexes.at(d->indexIndex);
}

bool QOrmAttributeInfo::writeIndex(QOrmObject *obj, QVariant pk)
{
    if(!isPrimaryKey()) return false;

    obj->d->indexes.replace(d->indexIndex, pk);
    return true;
}

void QOrmAttributeInfo::load(QOrmObject *obj, QVariant value)
{
    if (!d->valid) return;

    if (!d->f_ld)
    {
        if (isForeignKey() || isPrimaryKey()) writeIndex(obj, value);
        if (d->metaProperty.type() < QVariant::UserType)
            //d->metaProperty.write(obj, value);
            obj->setProperty(d->property, value);
        return;
    }

    (obj->*(d->f_ld))(value);
}

QString QOrmAttributeInfo::store(QOrmObject *obj)
{
    if (!d->f_st)
    {
        QVariant v = read(obj);
        if (v.isNull())
        {
            if (isForeignKey() || isPrimaryKey())
                v = readIndex(obj);
        }
        else
        {
            //qDebug()<<QOrm::isOrmObject(v)<<v<<isForeignKey();
            if (isForeignKey() && QOrm::isOrmObject(v))
            //if (QOrm::isOrmObject(v))
            {
                QOrmObject *fk = v.value<QOrmObject*>();
                if (!fk) v = QVariant();
                else v = refAttribute().readIndex(fk);
            }
        }

        switch (v.type())
        {
            case QVariant::Date:
            case QVariant::DateTime:
            case QVariant::Time:
            case QVariant::String:
                return '\'' + v.toString() + '\'';
            case QVariant::Invalid:
            case QVariant::UserType:
                return "NULL";
            case QVariant::Int:
            case QVariant::Double:
            default:
                return v.toString();
        }
    }

    return (obj->*(d->f_st))();
}

void QOrmAttributeInfo::setValue()
{
    QOrmObject *s = qobject_cast<QOrmObject*>(sender());
    if (!s) return;

    //QVariant value = s->property(d->property);
    s->modified();
    s->dataChanged(s, d->pos);
}

void QOrmAttributeInfo::setIndex()
{
    QOrmObject *s = qobject_cast<QOrmObject*>(sender());
    if (!s) return;

    QVariant index = s->property(d->property);

    s->d->indexes.replace(d->indexIndex, index);

    s->modified();
    s->dataChanged(s, d->pos);
}

void QOrmAttributeInfo::setForeignkey()
{
    QOrmObject *s = qobject_cast<QOrmObject*>(sender());
    if (!s) return;

    QVariant prop = s->property(d->property);
    //qDebug()<<prop;
    QOrmObject *fko = prop.value<QOrmObject*>();
    QOrmObject *old = s->d->oldFks.at(d->fkindex);
    s->d->oldFks.replace(d->fkindex, fko);

    QVariant index;
    if (fko) index = refAttribute().readIndex(fko);
    s->d->indexes.replace(d->indexIndex, index);

    if (old)
    {
        old->deref();
        old->disconnect(s);
    }

    if (fko)
    {
        fko->ref();
        connect(fko, &QObject::destroyed, s, &QOrmObject::fkDestroyed);
    }

    s->modified();
    s->dataChanged(s, d->pos);
}

QOrmAttributeInfo::operator QVariant()
{
    return QVariant::fromValue<QOrmAttributeInfo*>(this);
}

void QOrmAttributeInfo::setFunctions(QOrmAttributeInfo::LoadFn lfn, QOrmAttributeInfo::StoreFn sfn)
{
    d->f_ld = lfn;
    d->f_st = sfn;
}


QString QOrmAttributeInfo::name()
{
    return d->name;
}

QVariant::Type QOrmAttributeInfo::type()
{
    return d->type;
}

bool QOrmAttributeInfo::isPrimaryKey()
{
    return d->isPk;
}

bool QOrmAttributeInfo::isForeignKey()
{
    return !d->fk;
}

bool QOrmAttributeInfo::isNull()
{
    return d->isNull;
}

QOrmTableInfo *QOrmAttributeInfo::refTable()
{
    return d->reftable;
}

QOrmTableInfo *QOrmAttributeInfo::metaTable()
{
    return d->table;
}


QOrmAttributeInfo::Private::Private()
{
    valid = false;
    pos = fk = indexIndex = -1;
    type = QVariant::Invalid;
    isPk = isNull = false;
    table = reftable = NULL;
    f_ld = NULL;
    f_st = NULL;
}

QOrmAttributeInfo::Private *QOrmAttributeInfo::Private::clone()
{
    Private *d = new Private();
    d->pos = pos;
    d->name = name;
    d->property = property;
    d->type = type;
    d->isPk = isPk;
    d->isNull = isNull;
    d->fk = fk;
    d->indexIndex = indexIndex;
    d->fkindex = fkindex;

    d->table = table;
    d->reftable = reftable;

    d->metaProperty = metaProperty;
    d->f_ld = f_ld;
    d->f_st = f_st;

    d->valid = valid;
    return d;
}

QString QOrmChildrens::query() const
{
    return q;
}

QOrmChildrens::QOrmChildrens() :
    tab(0), lst(0), m_afn(0), m_rfn(0)
{
}

QOrmTableInfo *QOrmChildrens::table()
{
    return tab;
}

QOrmAttributeInfo *QOrmChildrens::foreignkey()
{
    return fk;
}

bool QOrmChildrens::isValid() const
{
    return tab && fk && lst;
}

void QOrmChildrens::load(QOrmObject *obj)
{
    if (!isValid()) return;

    QOrmBasicLoader ld(tab);
    ld.setQuery(q);
    ld.bindValue(obj);

    (obj->*lst).clear();
    for(QOrmObject *a : ld.loadAll())
    {
        append(obj, a);
        fk->writeForeingKey(a, obj);
    }
}

QList<QOrmObject *> QOrmChildrens::list(QOrmObject *o)
{
    if (!o || !lst) return QList<QOrmObject *>();

    return (o->*lst);
}

void QOrmChildrens::append(QOrmObject *own, QOrmObject *toAdd)
{
    if (!own || !m_afn) return;

    //(own->*lst).append(toAdd);
    (own->*(m_afn))(toAdd);
}

void QOrmChildrens::remove(QOrmObject *own, QOrmObject *toRem)
{
    if (!own || !m_rfn) return;

    //(own->*lst).removeOne(toRem);
    (own->*(m_rfn))(toRem);
}

void QOrmChildrens::setFunction(QOrmChildrens::AppendFn afn, QOrmChildrens::RemoveFn rfn)
{
    m_afn = afn;
    m_rfn = rfn;
}

#include "moc_qormattributeinfo.cpp"
