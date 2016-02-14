#include "qormtable.h"

#include <QDebug>

class QOrmTable::Private
{
public:
    QMap<QVariant, QOrmObject*> lst;
};

QList<QOrmObject *> QOrmTable::all() const
{
    return d->lst.values();
}

QOrmObject* QOrmTable::find(QVariant primaryKey)
{
    return d->lst.value(primaryKey);
}

void QOrmTable::append(QOrmObject* obj)
{
    if (!obj) return;
    auto f = d->lst.find(obj->primaryKey());
    if (f != d->lst.end() && f.value() != obj)
        f.value()->deref();

    d->lst[obj->primaryKey()] = obj;
}

void QOrmTable::remove(QOrmObject *obj)
{
    d->lst.remove(obj->primaryKey());
}

void QOrmTable::deref(QOrmObject *obj)
{
    obj->deref();
}

QOrmTable::QOrmTable() :
    d(new Private)
{
}
