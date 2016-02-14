#include "qormloader.h"

#include "qormattributeinfo.h"

#include <QSqlQuery>
#include <QSqlRecord>

#include <QOrmObject>
#include "qorm.h"
#include <QtCore>

#include "qormtable.h"

class QOrmBasicLoaderPrivate
{
public:
    QOrmTableInfo* meta;
    QList<QOrmTableInfo*> joins;
    QOrmBasicLoader::LoadPolicy pol;
    bool needP;

    QMap<QString, QVariant> values;

    QString where;
    QString from;
    QString order;
    QString query;

    void setAsLoaded(QOrmLoadedObject *obj);
    QList<QOrmObject *> fromQuery(QSqlQuery query);
    void loadAttr(QOrmObject *obj, QSqlQuery query, QSqlRecord row);

    void parse(QVariant v1, QVariant v2, QString op);
};

void QOrmBasicLoaderPrivate::setAsLoaded(QOrmLoadedObject *obj)
{
    obj->setAsLoaded();
}

QList<QOrmObject*> QOrmBasicLoaderPrivate::fromQuery(QSqlQuery q)
{
    QList<QOrmObject*> lst;
    QSqlRecord row = q.record();
    QOrmObject* obj = 0;
    while(q.next())
    {
        int pos = meta->index(0).pos();
        QVariant pk = q.value(pos);
        obj = meta->cache()->find(pk);
        if (!obj) obj = meta->create();

        obj->blockSignals(true);
        obj->beforeLoad();
        loadAttr(obj, q, row);
        meta->cache()->append(obj);
        obj->blockSignals(false);
        lst.append(obj);
    }

    for(QOrmObject *o : lst)
    {
        if (o->isSaved()) continue;

        o->loadAllForeignKeys();
        o->afterLoad();
        setAsLoaded(o);
    }

    return lst;
}

void QOrmBasicLoaderPrivate::loadAttr(QOrmObject *obj, QSqlQuery query, QSqlRecord row)
{
    for (int i = 0; i < row.count(); i++)
    {
        QOrmAttributeInfo a = meta->findAttribute(row.fieldName(i));
        if (!a.isValid())
        {
            //qDebug()<<"Problema: "<<row.fieldName(i)<<" No encontrado";
            obj->setProperty(row.fieldName(i).toLatin1(), query.value(i));
            continue;
        }

        a.load(obj, query.value(i));
    }
}

void QOrmBasicLoaderPrivate::parse(QVariant v1, QVariant v2, QString op)
{
    QOrmAttributeInfo a;
    QString e;

    if (v1.userType() == qMetaTypeId<QOrmAttributeInfo*>())
    {
        QObject *o = v1.value<QObject*>();
        QOrmAttributeInfo *a2 = qobject_cast<QOrmAttributeInfo*>(o);
        where += QString("`%1`.`%2` ").arg(a2->table()).arg(a2->name());
        //e = ":" + a.table() + "_" + a.name().replace(" ", "_");
        e = ":" + a2->name().replace(" ", "_");
    }
    else if (v1.type() >= QVariant::UserType)
    {
        //if (v1.userType() == qMetaTypeId<QOrmAttributeInfo>())

        if (v1.userType() == qMetaTypeId<QOrmFunction>())
        {
            QOrmFunction f = v1.value<QOrmFunction>();
            int c = values.count();
            e = QString(":f%1").arg(c);
            where += f.first.replace("?", e) + " ";
            values[e] = f.second;
        }
    }
    else
        where += "'" + v1.toString()+"' ";

    if (v2.type() >= QVariant::UserType)
    {
        if (v2.userType() == qMetaTypeId<QOrmAttributeInfo>())
        {
            QOrmAttributeInfo a = v2.value<QOrmAttributeInfo>();
            where += QString("%3 `%1`.`%2` ").arg(a.table()).arg(a.name()).arg(op);
        }
        if (QOrm::isOrmObject(v2))
        {
            QOrmObject* o = v2.value<QOrmObject*>();
            if (!o) return;

            if (a.refTable() == o->metaTable())
            {
                QVariant v = a.refAttribute().readIndex(o);
                where += op + " " + e + " ";
                values[e] = v;

                //if (!pattr.isValid()) pattr = a;
                needP = true;
            }
        }
        if (v2.userType() == qMetaTypeId<QOrmFunction>())
        {
            QOrmFunction f = v2.value<QOrmFunction>();
            int c = values.count();
            e = QString(":f%1").arg(c);
            where += op + " " + f.first.replace("?", e) + " ";
            values[e] = f.second;
        }
    }
    else
    {
        if (a.isValid() && !e.isEmpty())
        {
            where += op + " " + e + " ";
            values[e] = v2;
        }
        else
        {
            switch (v2.type())
            {
                case QVariant::Date:
                case QVariant::DateTime:
                case QVariant::Time:
                case QVariant::String:
                      where += QString("%1 '%2' ").arg(op).arg(v2.toString()); break;
                case QVariant::Invalid:
                case QVariant::UserType: break;
                case QVariant::Int:
                case QVariant::Double:
                default:
                    where += QString("%1 %2 ").arg(op).arg(v2.toString()); break;
            }
            //where += op + " " + v2.toString()+" ";
        }
    }
}

QOrmBasicLoader::QOrmBasicLoader(QOrmTableInfo *m) :
    d(new QOrmBasicLoaderPrivate)
{
    d->needP = false;
    d->meta = m;
    d->pol = Single;

    d->from = m->table();
}

bool QOrmBasicLoader::needParent() const
{
    //return d->pattr.isValid();
    return d->needP;
}

QOrmTableInfo *QOrmBasicLoader::metaTable() const
{
    return d->meta;
}

QOrmBasicLoader::LoadPolicy QOrmBasicLoader::loadPolicy() const
{
    return d->pol;
}

void QOrmBasicLoader::setLoadPolicy(QOrmBasicLoader::LoadPolicy pol)
{
    d->pol = pol;
}

void QOrmBasicLoader::where(QVariant v1, QVariant v2, QString op)
{
    //d->where = "WHERE ";
    d->where.clear();
    d->parse(v1, v2, op);
}

void QOrmBasicLoader::andWhere(QVariant v1, QVariant v2, QString op)
{
    d->where += " AND ";
    d->parse(v1, v2, op);
}

void QOrmBasicLoader::orWhere(QVariant v1, QVariant v2, QString op)
{
    d->where += " OR ";
    d->parse(v1, v2, op);
}

void QOrmBasicLoader::whereForeignKey(QOrmTableInfo *table, QOrmObject *obj)
{
    clear();
    QOrmAttributeInfo a = table->findForeignkey(metaTable());
    if (!a.isValid()) return;

    QVariant v;
    if (obj) v = a.readIndex(obj);
    //where(a.refAttribute(), v);

    QString e = ":" + a.table() + "_" + a.name().replace(" ", "_");
    d->values.insert(e, v);
    d->where += QString("`%1` %2 %3").arg(a.refAttribute().name(), " = ", e);

    //d->pattr = a;
    d->needP = true;
}

void QOrmBasicLoader::whereParent(QOrmTableInfo *table, QOrmObject *obj)
{
    clear();
    QOrmAttributeInfo a = metaTable()->findForeignkey(table);
    //qDebug()<<table->table()<<metaTable()->table();
    if (!a.isValid()) return;
    QOrmAttributeInfo attr = a.refAttribute();

    QVariant v;
    if (obj) v = attr.store(obj);

    QString e = ":" + attr.table() + "_" + attr.name().replace(" ", "_");
    d->values.insert(e, v);
    d->where += QString("`%2`.`%1` %3 %4").arg(a.name(), a.table(), " = ", e);

    //d->pattr = a;
    d->needP = true;
}

void QOrmBasicLoader::bindValue(QOrmObject *obj)
{
    if (!obj) return;

    QOrmTableInfo *m = obj->metaTable();
    for (int j = 0; j < m->attributesCount(); j++)
    {
        QOrmAttributeInfo attr = m->attribute(j);
        QString e = ":" + attr.table() + "_" + attr.name().replace(" ", "_");
        QMap<QString, QVariant>::iterator i = d->values.find(e);
        if (i == d->values.end())
            i = d->values.find(":" + attr.name().replace(" ", "_"));

        if (i != d->values.end())  {
            i->setValue(attr.store(obj));
        }
    }
}

void QOrmBasicLoader::bindValue(QOrmAttributeInfo attr, QVariant v)
{
    QString e = ":" + attr.name().replace(" ", "_");
    QMap<QString, QVariant>::iterator i = d->values.find(e);
    if (i == d->values.end()) return;

    i->setValue(v);
}

void QOrmBasicLoader::from(QString view)
{
    d->from = view;
}

void QOrmBasicLoader::naturalJoin(QString view)
{
    d->from += " NATURAL JOIN " + view;
}

void QOrmBasicLoader::naturalJoin(QOrmTableInfo *table)
{
    if (d->joins.contains(table)) return;

    d->from += " NATURAL JOIN " + table->table();
    d->joins.append(table);
}

void QOrmBasicLoader::order(QOrmAttributeInfo by, bool asc)
{
    d->order = " ORDER BY "+by.name();
    if (!asc) d->order += " desc";
}

void QOrmBasicLoader::setQuery(QString query)
{
    clear();
    d->query = query;

    QRegularExpression re("(:\\w+)");
    QRegularExpressionMatchIterator i = re.globalMatch(query);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        d->values.insert(match.captured(1), QVariant());
    }
}

QString QOrmBasicLoader::query()
{
    if (!d->query.isEmpty())
        return d->query;

    QString q = d->meta->sqlSelect(d->where);
    if (!d->from.isEmpty())
        q = q.replace(QRegularExpression("FROM(.+)WHERE"),
                      QString("FROM %1 WHERE").arg(d->from));

    if (!d->order.isEmpty())
        q += d->order;
    return q + ";";
}

void QOrmBasicLoader::clear()
{
    d->values.clear();
    d->joins.clear();
    d->from = d->meta->table();
    //d->pattr = QOrmAttributeInfo();
    //d->where = "WHERE ";
    d->where.clear();
    d->needP = false;
}



#include <QSqlError>
QList<QOrmObject *> QOrmBasicLoader::loadAll(int limit)
{
    QSqlQuery q;

    QString where = query();
    if (limit > 0) where += QString(" LIMIT %1").arg(limit);

    q.prepare(where);
    for(QMap<QString, QVariant>::iterator i = d->values.begin(); i != d->values.end(); i++)
    {
        q.bindValue(i.key(), i.value());
    }

    //qDebug()<<q.lastQuery();
    if (q.exec()) return d->fromQuery(q);
    //qDebug()<<q.lastError().text();
    return QList<QOrmObject*>();
}

QOrmObject *QOrmBasicLoader::load(QVariant pk)
{
    QOrmObject *wp = d->meta->cache()->find(pk);
    if (wp) return wp;

    QString query = d->meta->sqlSelect("%1 = ? LIMIT 1; ")
            .arg(d->meta->index(0).name());
    QSqlQuery q;
    q.prepare(query);
    q.bindValue(0, pk);

    if (q.exec())
    {
        QList<QOrmObject*> lst = d->fromQuery(q);
        if (!lst.isEmpty()) return lst.first();
    }
    else qWarning()<<q.lastError().text()<<q.lastQuery();

    return NULL;
}

void QOrmBasicLoader::reload(QOrmObject *obj)
{
    QOrmTableInfo *m = obj->metaTable();
    QString query = m->sqlSelect("%1 = ? LIMIT 1; ")
            .arg(m->index(0).name());
    QSqlQuery q;
    q.prepare(query);
    q.bindValue(0, obj->primaryKey());

    if (!q.exec()) return;
    if (!q.next()) return;

    QSqlRecord row = q.record();

    QOrmBasicLoaderPrivate d;
    d.meta = obj->metaTable();

    //obj->blockSignals(true);
    obj->beforeLoad();
    d.loadAttr(obj, q, row);
    obj->afterLoad();
    //obj->blockSignals(false);

    d.setAsLoaded(obj);
}


QOrmFunction::QOrmFunction() : QPair<QString, QVariant>() {}

QOrmFunction::QOrmFunction(QString func, QVariant v):
    QPair<QString, QVariant>(func, v) {}

QOrmFunction::operator QVariant() const
{
    return QVariant::fromValue<QOrmFunction>(*this);
}
