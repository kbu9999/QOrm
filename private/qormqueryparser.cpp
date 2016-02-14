#include "qormqueryparser.h"

#include <QOrmAttributeInfo>

QString QOrmSelectParser::query(QOrmTableInfo *meta)
{
    QStringList attr;
    for(int i = 0; i < meta->attributesCount(); i++)
        attr << "`" + meta->attribute(i).name() + "`";

    return QString("SELECT %3 FROM `%1`.`%2` ").arg(meta->database())
             .arg(meta->table())
             .arg(attr.join(", "));
}


QString QOrmInsertParser::query(QOrmTableInfo *meta)
{
    QStringList attr, val;
    for(int i = 0; i < meta->attributesCount(); i++)
    {
        if (!meta->attribute(i).isWriteable()) continue;

        attr << QString("`%1`").arg(meta->attribute(i).name());
        val << QString(":%1").arg(meta->attribute(i).name().
                                  replace(" ", "_"));
    }

    return QString("INSERT INTO `%1`.`%2` (%3) VALUES (%4); ")
            .arg(meta->database())
            .arg(meta->table())
            .arg(attr.join(", "))
            .arg(val.join(", "));
}


QString QOrmUpdateParser::query(QOrmTableInfo *meta)
{
    QStringList set, whe;
    for(int i = 0; i < meta->attributesCount(); i++)
    {
        QOrmAttributeInfo a = meta->attribute(i);
        if (!a.isWriteable()) continue;

        QString v = QString("`%1` = :%2")
               .arg(a.name())
               .arg(a.name().replace(" ", "_"));

        if (a.isPrimaryKey() && !a.isForeignKey()) whe << v;
        else set << v;
    }

    return QString("UPDATE `%1`.`%2` SET %3 WHERE %4; ")
            .arg(meta->database())
            .arg(meta->table())
            .arg(set.join(", "))
            .arg(whe.join(" AND "));
}


QString QOrmDeleteParser::query(QOrmTableInfo *meta)
{
    QOrmAttributeInfo a = meta->index(0);
    QString ret = "DELETE FROM `%1`.`%2` WHERE `%3` = :%4; ";
    ret = ret.arg(meta->database());
    ret = ret.arg(meta->table());
    ret = ret.arg(a.name());
    ret = ret.arg(a.name().replace(" ", "_"));
    return ret;
}
