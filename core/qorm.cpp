#include "qorm.h"

#include <QtCore>
#include <QtSql>
#include <QDebug>
#include <QOrmAttributeInfo>

#include "private/qormtableinfo_p.h"
#include "private/qormattributeinfo_p.h"

#include "private/qormqueryparser.h"

class QOrm::Private
{
public:
    QSqlDatabase db;
    bool init;
    QMap<QString, QOrmTableInfo*> tables;

    void addTable(QOrmTableInfo::Private *p);
    void reftables();
    void closeTable();

    static QMap<QString, QOrm*> bases;
    static QOrm* m_default;
    static QVector<int> types;
};

QMap<QString, QOrm*> QOrm::Private::bases = QMap<QString, QOrm*>();
QOrm *QOrm::Private::m_default = NULL;
QVector<int> QOrm::Private::types = QVector<int>(1, qMetaTypeId<QOrmObject*>());
//QVector<int> QOrm::Private::types = QVector<int>();

QOrm::QOrm() :
    d(new Private)
{
    d->init = false;
}

QSqlDatabase QOrm::database() const
{
    return d->db;
}

bool QOrm::isInit() const
{
    return d->init;
}

QOrmTableInfo *QOrm::table(QString tableName)
{
    return d->tables.value(tableName);
}

void QOrm::init(QSqlDatabase db)
{
    if (isInit()) return;

    if (!db.open())
    {
        qDebug()<<db.lastError().text();
        return;
    }

    d->db = db;

    QStringList lstab = db.tables();
    foreach (QString t, d->tables.keys())
    {
        if (!lstab.contains(t))
        {
            d->tables.remove(t);
            continue;
        }

        QOrmTableInfo::Private *ntp = d->tables[t]->d;

        ntp->db = db.databaseName();
        ntp->table = t;

        d->addTable(ntp);
    }

    d->reftables();
    d->closeTable();

    d->init = true;
}

bool QOrm::isOrmObject(int type)
{
    return QOrm::Private::types.contains(type);
}

QOrm *QOrm::addDatabase(QString name)
{
    QOrm *ndb = getDatabase(name);
    if (!ndb)
    {
        ndb = new QOrm();
        QOrm::Private::bases[name] = ndb;
    }
    return ndb;
}

QOrm *QOrm::getDatabase(QString name)
{
    if (name.isEmpty())
    {
        if (!QOrm::Private::m_default) QOrm::Private::m_default = new QOrm();
        return QOrm::Private::m_default;
    }

    if (QOrm::Private::bases.contains(name))
        return QOrm::Private::bases[name];

    return 0;
}

bool QOrm::removeDatabase(QString name)
{
    //TODO
    Q_UNUSED(name)
}

QOrmTableInfo *QOrm::_registerTable(QString tableName, const QMetaObject *metaO, QtOrm::QORMCreateFn cfn, QVector<QOrmAttributeInfo*> attrs, QVector<QOrmChildrens*> childrens,
                                    QString select, QString insert, QString deleted, QString update, QString database)
{
    QOrm *db = getDatabase(database);
    if (!db)
    {
        qWarning()<<"base de datos ORM no existe: "<<database;
        return 0;
    }
    QOrmTableInfo *meta = db->table(tableName);
    if (meta)
    {
        qWarning()<<"base de datos ORM ya contiene la tabla: "<<tableName;
        return 0;
    }
    meta = new QOrmTableInfo(tableName, metaO, cfn);
    meta->d->attrs = attrs;
    meta->d->childrens = childrens;
    db->d->tables[tableName] = meta;
    meta->d->sqlS = select;
    meta->d->sqlI = insert;
    meta->d->sqlU = update;
    meta->d->sqlD = deleted;

    return meta;
}

void QOrm::_registerTypes(int id)
{
    QOrm::Private::types.append(id);
}

void QOrm::Private::addTable(QOrmTableInfo::Private *p)
{
    QSqlQuery query("DESCRIBE " + p->table);
    query.exec();

    int i = 0;
    while (query.next())
    {
        QString name = query.value(0).toByteArray();
        int ai = tables[p->table]->indexAttribute(name);
        if (ai < 0) { qWarning()<<name<<" No es Atributo"; continue; }

        QOrmTableInfo *t = tables[p->table];
        QOrmAttributeInfo *a = t->d->attrs[ai];

        a->d->table = t;

        a->d->pos = i;
        a->d->type = QOrmAttributeInfo::typeFromString(query.value(1).toString().remove(QRegExp("\\(.+\\)")));
        a->d->isNull = query.value(2).toString() == "YES";
        //a->d->isPk = query.value(3).toString() == "PRI";
        a->d->isPk = query.value(3).toString() != "";
        //a->_ai = !query.value(5).isNull();
        i++;
    }
}

void QOrm::Private::reftables()
{
    QSqlQuery query;
    query.prepare("select table_name, referenced_table_name, column_name, referenced_column_name, "
                  "POSITION_IN_UNIQUE_CONSTRAINT from information_schema.key_column_usage "
                  "where referenced_table_name is not null and CONSTRAINT_SCHEMA = :db;");
    query.bindValue(":db", db.databaseName());
    query.exec();

    while (query.next())
    {
        QString qp1 = query.value(0).toString();
        if (!tables.contains(qp1)) continue;

        QString qp2 = query.value(1).toString();
        if (!tables.contains(qp2)) continue;
	
        QOrmTableInfo::Private *p1 = tables[qp1]->d;
        QOrmTableInfo *p2 = tables[qp2];
        QOrmAttributeInfo *a = 0;
        
        int i = 0;
        for(i = 0; i < p1->attrs.count(); i++) {
            if (p1->attrs[i]->name() == query.value(2).toString())
            {
                a = p1->attrs[i];
                break;
            }
        }


        if (!a || !p2) continue;
	
        /*if (!a->d->metaProperty.isValid())
        {
            int jp = p1->mobj->indexOfProperty(a->d->property);
            if (jp >= 0)
            {
                a->d->metaProperty = p1->mobj->property(jp);
            }
        }//*/

        a->d->isPk = true;
        a->d->reftable = p2;
        a->d->fk = query.value(4).toInt() - 1;
        //a->d->valid = true;
    }
}

void QOrm::Private::closeTable()
{
    QOrmSelectParser sl;
    QOrmInsertParser ins;
    QOrmUpdateParser upd;
    QOrmDeleteParser del;

    foreach (QOrmTableInfo *t, tables.values())
    {
        for(QOrmAttributeInfo *a : t->d->attrs)
        {
            if (a->isPrimaryKey())
            {
                a->d->indexIndex = t->d->indexes.count();
                t->d->indexes << a;
            }

            if (a->isForeignKey())
            {
                a->d->fkindex = t->d->fks.count();
                t->d->fks << a;
            }

            if (!a->d->metaProperty.isValid())
            {
                int jp = t->d->mobj->indexOfProperty(a->d->property);
                if (jp >= 0)
                {
                    a->d->metaProperty = t->d->mobj->property(jp);
                    a->d->valid = true;
                }
            }//*/
        }

        if (t->d->sqlS.isEmpty()) t->d->sqlS = sl.query(t);
        if (t->d->sqlI.isEmpty()) t->d->sqlI = ins.query(t);
        if (t->d->sqlU.isEmpty()) t->d->sqlU = upd.query(t);
        if (t->d->sqlD.isEmpty()) t->d->sqlD = del.query(t);
    }

    for (QOrmTableInfo *t : tables.values())
    {
        for (QOrmChildrens *c : t->d->childrens) {
            auto ind = tables.find(c->tname);
            if (ind == tables.end()) continue;

            QOrmTableInfo *tr = ind.value();
            c->tab = tr;
            QOrmAttributeInfo a = tr->findForeignkey(t);
            c->fk = new QOrmAttributeInfo(a);
            if (c->q.isEmpty()) {
                c->q = tr->sqlSelect(
                            QString(" `%2` = :%3")
                            .arg(a.name())
                            .arg(a.name().replace(" ", "_")));
            }
        }
    }
}
