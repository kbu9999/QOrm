#ifndef QORMQUERYPARSER_H
#define QORMQUERYPARSER_H

#include <QtCore>
#include <QOrmTableInfo>

class QOrmQueryParser
{
public:
    virtual QString query(QOrmTableInfo *meta) = 0;
};


class QOrmSelectParser : public QOrmQueryParser
{
public:
    QString query(QOrmTableInfo *meta);
};

class QOrmInsertParser : public QOrmQueryParser
{
public:
    QString query(QOrmTableInfo *meta);
};

class QOrmUpdateParser : public QOrmQueryParser
{
public:
    QString query(QOrmTableInfo *meta);
};

class QOrmDeleteParser : public QOrmQueryParser
{
public:
    QString query(QOrmTableInfo *meta);
};


#endif // QORMQUERYPARSER_H
