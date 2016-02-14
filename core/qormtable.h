#ifndef QORMTABLE_H
#define QORMTABLE_H

#include <QList>
#include <QVariant>
#include <QOrmObject>

class QOrmTable
{
public:
    QList<QOrmObject*> all() const;

    QOrmObject* find(QVariant primaryKey);
    void append(QOrmObject *obj);
    void remove(QOrmObject *obj);

    void deref(QOrmObject *obj);

private:
    QOrmTable();

    class Private;
    Private *d;

    friend class QOrmTableInfo;
};

#endif // QORMTABLE_H
