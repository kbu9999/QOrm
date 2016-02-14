#ifndef QORMLOADER_H
#define QORMLOADER_H

#include <qorm_global.h>
#include "qormtableinfo.h"
#include "qormattributeinfo.h"

#include <QOrmObject>
#include <QVariantList>

template <class T> class QOrmLoader;
class QOrmBasicLoaderPrivate;

class QOrmFunction : public QPair<QString, QVariant>
{
public:
    QOrmFunction();
    QOrmFunction(QString func, QVariant v = QVariant());

    operator QVariant() const;
};

class QOrmBasicLoader
{
public:
    QOrmBasicLoader(QOrmTableInfo *table);

    bool needParent() const;
    QOrmTableInfo *metaTable() const;

    enum LoadPolicy { Single, AllForeignKeys };
    LoadPolicy loadPolicy() const;
    void setLoadPolicy(LoadPolicy pol);

    void where(QVariant v1, QVariant v2, QString op = "=");
    void andWhere(QVariant v1, QVariant v2, QString op = "=");
    void orWhere(QVariant v1, QVariant v2, QString op = "=");
    void between(QVariant prop, QVariant start, QVariant end);

    void whereForeignKey(QOrmTableInfo *table, QOrmObject *obj);
    template <class TParent> void whereForeignKey() { whereForeignKey( TParent::staticMetaTable, 0); }
    template <class TParent> void whereForeignKey(TParent *parent) { whereForeignKey( TParent::staticMetaTable, parent); }

    void whereParent(QOrmTableInfo *table, QOrmObject *obj);
    template <class TParent> void whereParent() { whereParent( TParent::staticMetaTable, 0); }
    template <class TParent> void whereParent(TParent* parent) { whereParent( TParent::staticMetaTable, parent); }

    void bindValue(QOrmObject *obj);
    void bindValue(QOrmAttributeInfo attr, QVariant v);

    void from(QString view);
    void naturalJoin(QString view);
    void naturalJoin(QOrmTableInfo *table);
    template <class TJoin> void naturalJoin() { naturalJoin(TJoin::staticMetaTable); }
    void order(QOrmAttributeInfo by, bool asc = true);

    void setQuery(QString query);
    QString query();
    void clear();

    QList<QOrmObject*> loadAll(int limit = 0);
    QOrmObject* load(QVariant pk);
    static void reload(QOrmObject *obj);

    template <class TParent, class TChild>
    static QOrmLoader<TChild> *QOrmRelation1N(QOrmBasicLoader::LoadPolicy policy = QOrmBasicLoader::Single);

    template <class TParent, class TFK>
    static QOrmLoader<TFK> *QOrmRelation11(QOrmBasicLoader::LoadPolicy policy = QOrmBasicLoader::Single);

private:
    QOrmBasicLoaderPrivate *d;
};

template <class T>
class QOrmLoader : public QOrmBasicLoader
{
public:
    QOrmLoader() : QOrmBasicLoader(T::staticMetaTable) { }

    inline QList<T*> loadAll() { return QtOrm::list_cast<T>(QOrmBasicLoader::loadAll()); }
    inline T* load(QVariant pk) { return qobject_cast<T*>(QOrmBasicLoader::load(pk)); }

    static T* loadOne(QVariant pk) { QOrmLoader<T> ld; return ld.load(pk); }
};

template <class TParent, class TChild>
QOrmLoader<TChild> *QOrmBasicLoader::QOrmRelation1N(QOrmBasicLoader::LoadPolicy policy)
{
    QOrmLoader<TChild> *ld = new QOrmLoader<TChild>();
    ld->setLoadPolicy(policy);
    ((QOrmBasicLoader*)ld)->whereParent<TParent>();
    return ld;
}

template <class TParent, class TFK>
QOrmLoader<TFK> *QOrmBasicLoader::QOrmRelation11(QOrmBasicLoader::LoadPolicy policy)
{
    QOrmLoader<TFK> *ld = new QOrmLoader<TFK>();
    ld->setLoadPolicy(policy);
    ((QOrmBasicLoader*)ld)->whereForeignKey<TParent>();
    return ld;
}

Q_DECLARE_METATYPE(QOrmFunction)

#endif // QORMLOADER_H
