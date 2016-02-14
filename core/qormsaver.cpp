#include "qormsaver.h"
#include <QOrmTableInfo>
#include <QOrmAttributeInfo>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QOrmObject>

#include "qormtable.h"

class QOrmSaver::Private
{
public:
    struct inserted_id
    {
        int column;
        int index;
    };

    QString parse(QOrmObject *obj);
    QString parseChildrens(QOrmObject *obj);
    QString parseObject(QOrmObject *obj, QString query);
    QString parseReferences(QOrmObject *obj);
    QString parseDelete(QOrmObject *obj);
    //QString parseAttribute(QVariant v, QOrmAttributeInfo *a);

    bool obtainIds();

    QStack<QOrmObject*> stack;
    QList<QOrmObject*> to_save;
    QList<QOrmObject*> to_del;
    QMap<QOrmObject*, inserted_id> inserted;

    QString query;
    QString erros;
    QString lastQuery;
};

QOrmSaver::QOrmSaver() :
    d(new Private)
{
}

QString QOrmSaver::lastQuery()
{
    return d->lastQuery;
}

void QOrmSaver::pushToSave(QOrmObject *obj)
{
    if (!obj) return;
    if (d->to_save.contains(obj)) return;

    d->to_save.append(obj);
    d->query += d->parse(obj);
}

void QOrmSaver::pushToDelete(QOrmObject *obj)
{
    if (!obj) return;
    if (d->to_save.contains(obj)) return;
    if (d->to_del.contains(obj)) return;

    d->to_del.append(obj);
    d->query += d->parseDelete(obj);
}

bool QOrmSaver::run()
{
    d->erros.clear();
    d->lastQuery = d->query;
    if (d->query.isEmpty())
    {
        d->erros += "Query is empty\n";
        return true;
    }
    //if (!q.exec("SET AUTOCOMMIT=0;START TRANSACTION;"))
    //    return false;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen())
    {
        qDebug()<<"reopen db: "<<db.open();
    }

    QSqlQuery q;

    if (!db.transaction())
    {
        d->erros += "error transaction\n";
        d->erros += db.lastError().text() + '\n';
        d->query.clear();
        return false;
    }

    if (q.exec(d->query))
    {
        //qDebug()<<d->query;

        //if (!db.commit())
        if(!q.exec("COMMIT;SET AUTOCOMMIT=1;"))
        {
            d->erros += "error commit\n";
            d->erros += db.lastError().text()+'\n';
            //d->erros += "rollback: "+db.rollback()+'\n';
            //q.exec("ROLLBACK;SET AUTOCOMMIT=1;");
            db.rollback();
        }

        bool b = d->obtainIds();
        if (b)
            foreach (QOrmObject *obj, d->to_save) { obj->setAsLoaded(); }
        else
            d->erros += "error al obtener id";

        foreach (QOrmObject *obj, d->to_del) { obj->setAsDeleted(); }

        d->to_save.clear();
        d->to_del.clear();
        d->inserted.clear();
        d->stack.clear();
        d->query.clear();

        qDebug()<<"qorm: "<<d->erros;

        return b;
    }
    else
    {
        d->erros += q.lastError().text() + '\n';
        d->query.clear();
        //q.exec("ROLLBACK;SET AUTOCOMMIT=1;");
        db.rollback();

        return false;
    } //*/
    return false;
}

QString QOrmSaver::lastErrors()
{
    return d->erros;
}

bool QOrmSaver::Private::obtainIds()
{
    QSqlQuery q;
    QList<QOrmObject*> keys = inserted.keys();
    foreach (QOrmObject *obj, keys)
    {
        inserted_id id = inserted[obj];
        if (!q.exec(QString("SELECT @last_insert_%1;").arg(id.index)))
            return false;

        if (!q.next())
            return false;

        QVariant v = q.value(0);
        if (v.isNull())
            return false;

        obj->metaTable()->attribute(id.column).write(obj, v.toInt());
        //obj->metaTable()->cache()->append(obj);
    }
    return true;
}


QString QOrmSaver::Private::parse(QOrmObject *obj)
{
    if (!obj)
        return "";
    if (obj->isEmpty())
        return "";
    if (stack.contains(obj))
        return "";
    stack.append(obj);

    QOrmTableInfo *meta = obj->metaTable();
    QString query;
    if (obj->isSaved())
    {
        if (obj->isDirt())
            query = meta->sqlUpdate();
    }
    else
    {
        query += meta->sqlInsert();
        if (!obj->indexes().first().isValid())
        {
            int first_index = meta->index(0).pos();

            inserted_id toi;
            toi.index = inserted.count();
            toi.column = first_index;

            inserted.insert(obj, toi);

            query += QString("SELECT @last_insert_%1:=LAST_INSERT_ID();").arg(toi.index);
        }
    }

    QString q;
    q += parseReferences(obj);
    q += parseObject(obj, query) + '\n';;
    q += parseChildrens(obj);
    return q;
}

QString QOrmSaver::Private::parseChildrens(QOrmObject *obj)
{
    QString q;
    for(QOrmChildrens cht : obj->metaTable()->childrens())
    {
        for(QOrmObject* &o : cht.list(obj))
            q += parse(o);
    }
    return q;
}

QString QOrmSaver::Private::parseObject(QOrmObject *obj, QString query)
{
    if (query.isEmpty())
        return "";

    QOrmTableInfo *meta = obj->metaTable();
    int c = meta->attributesCount();
    for (int i = 0; i < c; i++)
    {
        QOrmAttributeInfo ai = meta->attribute(i);

        QOrmObject* fko = ai.readForeignKey(obj);
        if (inserted.contains(fko))
        {
            QString r = QString("@last_insert_%1").arg(inserted[fko].index);
            query = query.replace(":"+ai.name().replace(" ", "_"), r);
            continue;
        }
        else
        {
            query = query.replace(":"+ai.name().replace(" ", "_"), ai.store(obj));
        }
    }
    to_save.append(obj);

    return query;
}

QString QOrmSaver::Private::parseReferences(QOrmObject *obj)
{
    QString q;
    QOrmTableInfo *meta = obj->metaTable();
    for(int i = 0; i < meta->foreignKeysCount(); i++)
    {
        QOrmObject *fko = meta->foreignKey(i).readForeignKey(obj);
        if(fko) q += parse(fko);
    }
    return q;
}

QString QOrmSaver::Private::parseDelete(QOrmObject *obj)
{
    if (!obj)
        return "";
    if (stack.contains(obj))
        return "";
    stack.append(obj);
    to_del.append(obj);

    QOrmTableInfo *meta = obj->metaTable();
    QString str = obj->metaTable()->sqlDelete();
    for(int i = 0; i < meta->indexCount(); i++)
    {
        QOrmAttributeInfo ai = meta->attribute(i);
        str = str.replace("%"+ai.name()+"%", ai.store(obj));
    }
    return str;
}
