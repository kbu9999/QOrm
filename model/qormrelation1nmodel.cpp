#include "qormrelation1nmodel.h"
#include "qormabstractmodel_p.h"

#include <QOrm>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QOrmRelation1NModel::QOrmRelation1NModel(QOrmTableInfo *meta):
    QOrmAbstractModel(meta, NC)
{
}

void QOrmRelation1NModel::setRootIndex(const QModelIndex &root)
{
    QOrmAbstractModel *mdl = QtOrm::model_cast(root.model());
    if (!mdl || !root.isValid())
    {
        d->rootIndex = QModelIndex();
        d->root = QOrmObjectPtr();
        d->list = QList<QOrmObjectPtr>();
        beginResetModel();
        endResetModel();
        return;
    }

    QOrmObjectPtr dt = mdl->at(root.internalId());
    if (!dt) return;

    QOrmAttributeInfo *a = metaTable()->findForeignkey(dt->metaTable());
    if (!a) return;

    if (dt == d->root) return;
    d->rootIndex = root;
    d->root = dt;

    disconnectAll();

    d->list = dt->childrens(metaTable());
    if (d->list.isEmpty())
    {
        dt->loadChildren(metaTable());
        for(QOrmObjectPtr &o : dt->childrens(metaTable()))
        {
            d->list.append(o);
            a->writeForeingKey(o.data(), dt);
            connect(o.data(), &QOrmObject::dataChanged, this, &QOrmRelation1NModel::objectDataChanged);
        }
        d->loadFromDB = true;
    }
    else reconnectAll();

    beginResetModel();
    endResetModel();
}




