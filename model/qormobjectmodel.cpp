#include "qormobjectmodel.h"
#include "qormabstractmodel_p.h"

#include <QOrm>

QOrmAbstractObjectModel::QOrmAbstractObjectModel(QOrmTableInfo *m) :
    QOrmAbstractModel(m, Single)
{
}

QOrmAbstractObjectModel::~QOrmAbstractObjectModel()
{
}

void QOrmAbstractObjectModel::setOrmObject(QOrmObjectPtr obj)
{
    if (d->root == obj) return;
    if (d->root)
        disconnect(d->root.data(), &QOrmObject::dataChanged, this, &QOrmAbstractObjectModel::objectDataChanged);

    d->list.clear();
    d->list.append(obj);

    d->root = obj;
    connect(d->root.data(), &QOrmObject::dataChanged, this, &QOrmAbstractObjectModel::objectDataChanged);

    beginResetModel();
    endResetModel();
}

void QOrmAbstractObjectModel::setRootIndex(const QModelIndex &root)
{
    setOrmObject(QtOrm::modelIndex_cast(root));
}
