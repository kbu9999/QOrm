#ifndef QORMRELATION1NMODEL_H
#define QORMRELATION1NMODEL_H

#include <QOrmAbstractModel>

class QOrmRelation1NModel : public QOrmAbstractModel
{
    Q_OBJECT
public:
    QOrmRelation1NModel(QOrmTableInfo *meta);

    void setRootIndex(const QModelIndex &root);

protected:

};

#endif // QORMRELATION1NMODEL_H
