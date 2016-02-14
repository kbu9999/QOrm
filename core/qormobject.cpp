#include "qormobject.h"

#include "qorm.h"
#include "qormtableinfo.h"
#include "qormattributeinfo.h"

#include "qormloader.h"
#include "qormsaver.h"
#include "qormtable.h"

#include <QVector>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

class QOrmLoadedObject::Private
{
public:
    bool empty, saved, deleted, dirt;
    QOrmTableInfo *meta;

    Private(QOrmTableInfo *metaTable) :
        empty(true), saved(false), deleted(false), dirt(false),
        meta(metaTable)
    {
    }
};

class QOrmObject::Private
{
public:
    QVector<QOrmObject*> oldFks;
    QVector<QVariant> indexes;

    void loadFKs(QOrmObject *obj, QOrmAttributeInfo a);
};

QOrmObject::QOrmObject(QOrmTableInfo *metatable)  :
    QOrmLoadedObject(metatable),
    d(new Private())
{
    d->indexes = QVector<QVariant>(metatable->indexCount());
    d->oldFks = QVector<QOrmObject*>(metatable->foreignKeysCount(), NULL);
    metatable->connectAttributes(this);
}

bool QOrmObject::save()
{
    QOrmSaver saver;
    saver.pushToSave(this);
    bool b = saver.run();
    if (!b) qDebug()<<"qorm: "<<saver.lastErrors()<<saver.lastQuery();
    //qDebug()<<saver.lastQuery();
    return b;
}

void QOrmObject::clear()
{
    d->indexes.clear();
    d->indexes = QVector<QVariant>(metaTable()->indexCount());
    d->oldFks = QVector<QOrmObject*>(metaTable()->foreignKeysCount(), NULL);

    QOrmLoadedObject::clear();
}

void QOrmObject::loadChildren(QOrmTableInfo *childTable)
{
    QOrmChildrens chld = metaTable()->indexChildren(childTable);
    if (!chld.isValid()) return;

    chld.load(this);
}

void QOrmObject::loadForeignKeys(QOrmTableInfo *fkTable)
{
    QOrmAttributeInfo a = metaTable()->findForeignkey(fkTable);
    if (!a.isValid()) return;

    d->loadFKs(this, a);
}

void QOrmObject::fkDestroyed(QObject *od)
{
    int i = d->oldFks.indexOf((QOrmObject*)od);
    metaTable()->foreignKey(i).writeForeingKey(this, NULL);
}

void QOrmObject::loadAllForeignKeys()
{
    for (int i = 0; i < metaTable()->foreignKeysCount(); ++i)
        d->loadFKs(this, metaTable()->foreignKey(i));
}

QVariant QOrmObject::primaryKey()
{
    //return d->indexes.at(metaTable()->index(0).indexIndexes());
    return d->indexes.at(0);
}

QVariantList QOrmObject::indexes()
{
    return d->indexes.toList();
}

void QOrmObject::afterLoad()
{

}

void QOrmObject::beforeLoad()
{
}

QOrmObject::~QOrmObject()
{
    blockSignals(true);
    //for(QOrmChildrens c : metaTable()->childrens())
    //    for (QOrmObject * o : c.list(this)) removeChildren(o);

    int c = metaTable()->foreignKeysCount();
    for (int i = 0; i < c; i++)
    {
        QOrmAttributeInfo fk = metaTable()->foreignKey(i);
        fk.writeForeingKey(this, NULL);
    }
    metaTable()->cache()->remove(this);
    blockSignals(false);
}

QOrmTableInfo *QOrmObject::metaTable() const
{
    return QOrmLoadedObject::metaTable();
}

QOrmLoadedObject::QOrmLoadedObject(QOrmTableInfo *meta) :
    QOrmReferedObject(),
    dlo(new Private(meta))
{
}

QOrmLoadedObject::~QOrmLoadedObject()
{
}

bool QOrmLoadedObject::isSaved() const
{
    return dlo->saved;
}

bool QOrmLoadedObject::isDeleted() const
{
    return dlo->deleted;
}

bool QOrmLoadedObject::isEmpty() const
{
    return dlo->empty;
}

bool QOrmLoadedObject::isDirt() const
{
    return dlo->dirt;
}

void QOrmLoadedObject::clear()
{
    dlo->deleted = false;
    dlo->saved = false;
    dlo->dirt = false;
    dlo->empty = true;

    deletedChanged(dlo->deleted);
    savedChanged(dlo->saved);
    dirtChanged(dlo->dirt);
    emptyChanged(dlo->empty);
}

QOrmTableInfo *QOrmLoadedObject::metaTable() const
{
    return dlo->meta;
}

void QOrmLoadedObject::modified()
{
    if (dlo->deleted)
        return;

    if (dlo->empty)
        dlo->empty = false;

    dlo->dirt = true;

    dirtChanged(dlo->dirt);
    emptyChanged(dlo->empty);
}

void QOrmLoadedObject::appendChildren(QOrmObject *obj)
{
}

void QOrmLoadedObject::removeChildren(QOrmObject *obj)
{
}

void QOrmLoadedObject::setAsLoaded()
{
    if (dlo->deleted) return;
    dlo->dirt = false;
    dlo->saved = true;
    dlo->empty = false;

    dlo->meta->cache()->append((QOrmObject*)this);

    savedChanged(dlo->saved);
    dirtChanged(dlo->dirt);
    emptyChanged(dlo->empty);
}

void QOrmLoadedObject::setAsDeleted()
{
    if (dlo->empty)
        return;

    dlo->deleted = true;

    deletedChanged(dlo->deleted);
}

void QOrmLoadedObject::childDestroyed(QObject *od)
{
    QOrmObject *o = (QOrmObject*)od;
    QOrmChildrens c = metaTable()->indexChildren(o->metaTable());
    if (!c.isValid()) return;

    c.remove((QOrmObject*)this, o);
}

void QOrmObject::Private::loadFKs(QOrmObject *obj, QOrmAttributeInfo a)
{
    if (a.typeOfProperty() < QVariant::UserType) return;

    QOrmObject* fko = a.readForeignKey(obj);
    if (fko) return;

    QVariant pk = a.readIndex(obj);
    if (pk.isNull()) return;

    QOrmBasicLoader ld(a.refTable());
    fko = ld.load(pk);
    if (!fko) return;

    a.writeForeingKey(obj, fko);
}


bool QOrmReferedObject::tryDelete()
{
    //refs--;
    if (refs > 1) return false;

    deleteLater();
    return true;
}

QOrmReferedObject::QOrmReferedObject() :
    QObject(),
    refs(0)
{
}

void QOrmReferedObject::ref()
{
    refs++;
}

void QOrmReferedObject::deref()
{
    refs--;
    //qDebug()<<this<<refs;
    if (refs <= 0)
        deleteLater();
}

