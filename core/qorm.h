#ifndef QORM_H
#define QORM_H

#include <QSqlDatabase>

#include <QOrmObject>
#include <QOrmTableInfo>

#include <QDebug>

class QOrm
{
public:
    QOrm();
    QSqlDatabase database() const;
    bool isInit() const;

    QOrmTableInfo *table(QString tableName);
    void init(QSqlDatabase db);

    template <class T>
    static QOrmTableInfo *registerTable(QString tableName, QVector<QOrmAttributeInfo*> attrs,
                                        QVector<QOrmChildrens*> childrens = QVector<QOrmChildrens*>(),
                                 QString select = "", QString insert = "", QString deleted = "",
                                 QString update = "", QString database = "")
    {
        QOrmTableInfo *meta = _registerTable(tableName, &T::staticMetaObject, &QtOrm::static_create<T>, attrs, childrens,
                                             select, insert, deleted, update, database);
        if (!meta) { qWarning()<<"base de datos Tabla no existe"; return 0; }

        qRegisterMetaType<T*>();
        _registerTypes(qRegisterMetaType<T*>());
        return meta;
    }

    static bool isOrmObject(int type);
    template <class T> static bool isOrmObject() { return isOrmObject(qMetaTypeId<T>()); }
    static bool isOrmObject(QVariant value) { return isOrmObject(value.userType()); }

    //singleton
    static QOrm *addDatabase(QString name = QString());
    static QOrm *getDatabase(QString name = QString());
    static bool removeDatabase(QString name);

private:
    class Private;
    Private *d;

    static QOrmTableInfo *_registerTable(QString tableName,const QMetaObject *metaO, QtOrm::QORMCreateFn cfn, QVector<QOrmAttributeInfo*> attrs,
                                 QVector<QOrmChildrens*> childrens,
                                 QString select, QString insert, QString deleted, QString update,
                                 QString database);
    static void _registerTypes(int id);
};

#endif // QORM_H
