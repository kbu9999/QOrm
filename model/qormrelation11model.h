#ifndef QORMRELATION11MODEL_H
#define QORMRELATION11MODEL_H

#include <QOrmAbstractModel>

class QOrmRelation11Model : public QOrmAbstractModel
{
    Q_OBJECT
public:
    QOrmRelation11Model(QOrmTableInfo *meta, int fks);
    void setRootIndex(const QModelIndex &root);

private:
    int fk;
};

#endif // QORMRELATION11MODEL_H
