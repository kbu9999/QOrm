#ifndef QORMTABLEINFO_P_H
#define QORMTABLEINFO_P_H

#include <QtCore>
#include <QOrmAttributeInfo>

class QOrmTableInfo::Private
{
public:
    QString db;
    QString table;

    QString sqlS, sqlI, sqlU, sqlD;

    QtOrm::QORMCreateFn fnc;
    QMetaObject *mobj;

    QOrmTable *cache;

    QVector<QOrmAttributeInfo*> attrs, fks, indexes;
    QVector<QOrmChildrens*> childrens;
};

#endif // QORMTABLEINFO_P_H
