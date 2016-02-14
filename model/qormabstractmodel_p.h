#ifndef QORMABSTRACTMODELPRIVATE_H
#define QORMABSTRACTMODELPRIVATE_H

#include <QOrmObject>

class QOrmAbstractModelPrivate
{
public:
    QOrmTableInfo *meta;
    QModelIndex rootIndex;
    bool loadFromDB;

    QHash<int, QByteArray> roles;
    QList<QOrmObject*> list;
    QList<QOrmObject*> deleted;

    QOrmBasicLoader *ld;
};

#endif // QORMABSTRACTMODELPRIVATE_H
