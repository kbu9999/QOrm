#ifndef QORMOBJECT_H
#define QORMOBJECT_H

#include <QObject>
#include <QOrmTableInfo>
#include <QOrmAttributeInfo>

#define QORM_OBJECT \
public: \
    static QOrmTableInfo *staticMetaTable; \
    static QOrmAttributeInfo attribute(QString name) { return staticMetaTable->findAttribute(name); }


class QOrmReferedObject : public QObject
{
public:
    bool tryDelete();

protected:
    QOrmReferedObject();

private:
    unsigned int refs;
    void ref();
    void deref();

    friend class QOrmTable;
    friend class QOrmAbstractModel;
    friend class QOrmAttributeInfo;
};

class QOrmLoadedObject : public QOrmReferedObject
{
    Q_OBJECT
    Q_PROPERTY(bool saved READ isSaved NOTIFY savedChanged)
    Q_PROPERTY(bool deleted READ isDeleted NOTIFY deletedChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    Q_PROPERTY(bool dirt READ isDirt NOTIFY dirtChanged)
public:
    virtual ~QOrmLoadedObject();
    bool isSaved() const;
    bool isDeleted() const;
    bool isEmpty() const;
    bool isDirt() const;

    virtual void clear();

    QOrmTableInfo *metaTable() const;

signals:
    void savedChanged(bool value);
    void deletedChanged(bool value);
    void emptyChanged(bool value);
    void dirtChanged(bool value);

protected:
    QOrmLoadedObject(QOrmTableInfo *meta);
    void modified();
    void appendChildren(QOrmObject* obj);
    void removeChildren(QOrmObject* obj);

private:
    void setAsLoaded();
    void setAsDeleted();
    void childDestroyed(QObject *od);

    class Private;
    Private *dlo;

    friend class QOrmBasicLoaderPrivate;
    friend class QOrmSaver;
};

class QOrmObject : public QOrmLoadedObject
{
    Q_OBJECT
public:
    virtual ~QOrmObject();

    QOrmTableInfo *metaTable() const;

    Q_INVOKABLE bool save();
    Q_INVOKABLE void clear();

    QVariant primaryKey();
    QVariantList indexes();

    virtual void afterLoad();
    virtual void beforeLoad();

    void loadAllForeignKeys();
    template <class T> void loadChildren() { loadChildren(T::staticMetaTable); }
    template <class T> void loadForeignKeys() { loadForeignKeys(T::staticMetaTable); }

signals:
    void dataChanged(QOrmObject *obj, int attr);

protected:
    QOrmObject(QOrmTableInfo *metatable);
    void loadChildren(QOrmTableInfo *childTable);
    void loadForeignKeys(QOrmTableInfo *fkTable);

private:
    class Private;
    Private *d;

    void fkDestroyed(QObject *od);

    friend class QOrmAttributeInfo;
};

typedef QList<QOrmObject*> QOrmObjectList;

Q_DECLARE_METATYPE(QOrmObject*)

#endif // QORMOBJECT_H
