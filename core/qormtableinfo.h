#ifndef QORMTABLEINFO_H
#define QORMTABLEINFO_H

#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QSqlDatabase>
#include <QSharedPointer>

//#include <QOrmObject>
class QOrmObject;

class QOrmTableInfo;
class QOrmTable;
class QOrmAttributeInfo;
class QOrmChildrens;
class QOrmAbstractModel;

namespace QtOrm {
typedef QOrmObject* (*QORMCreateFn)();
typedef QList<QOrmObject*> (QOrmObject::*QOrmChild);

template <class _t> static QOrmObject *static_create() { return new _t(); }

QOrmAbstractModel *model_cast(const QAbstractItemModel *model);
QOrmObject *modelIndex_cast(const QModelIndex &index);

template <class _t> QSharedPointer<_t> modelIndex_cast2(const QModelIndex &index)
{
    return qSharedPointerCast<_t>(modelIndex_cast(index));
}

template <class _t> QList<_t*> list_cast(QList<QOrmObject*> list)
{
    QList<_t*> lst;
    foreach(QOrmObject* o, list) lst<< qobject_cast<_t*>(o);
    return lst;
}

}

class QOrmTableInfo
{
public:
    QOrmTableInfo(QString tableName, const QMetaObject *mobj, QtOrm::QORMCreateFn cfn);

    QString database();
    QString table();

    QOrmTable *cache();

    int attributesCount();
    int indexCount();
    int foreignKeysCount();

    QOrmAttributeInfo attribute(int attr);
    QOrmAttributeInfo index(int ind);
    QOrmAttributeInfo foreignKey(int fk);

    QOrmAttributeInfo findForeignkey(QOrmTableInfo *other);
    QOrmAttributeInfo findAttribute(QString name);

    int propertyCount();
    QOrmAttributeInfo property(int i);
    QMetaObject *metaObject() const;

    int indexAttribute(QString name);

    QOrmChildrens indexChildren(QOrmTableInfo *table);
    QVector<QOrmChildrens> childrens();

    QOrmObject *create();
    void connectAttributes(QOrmObject *obj);

    virtual QString sqlSelectAll();
    virtual QString sqlSelect(QString where);
    virtual QString sqlSelectByIndex();
    virtual QString sqlInsert();
    virtual QString sqlUpdate();
    virtual QString sqlDelete();

private:
    class Private;
    Private *d;

    friend class QOrm;
};

#endif // QORMTABLEINFO_H
