#ifndef QORMATTRIBUTEINFO_P_H
#define QORMATTRIBUTEINFO_P_H

#include <QMetaProperty>

class QOrmAttributeInfo::Private
{
public:
    Private();

    int pos;
    QByteArray name;
    QByteArray property;
    QVariant::Type type;
    bool isPk;
    bool isNull;
    int fk;
    int indexIndex;
    int fkindex;

    QOrmTableInfo *table;
    QOrmTableInfo *reftable;

    QMetaProperty metaProperty;
    QOrmAttributeInfo::LoadFn f_ld;
    QOrmAttributeInfo::StoreFn f_st;

    bool valid;

    Private *clone();
};

#endif // QORMATTRIBUTEINFO_P_H
