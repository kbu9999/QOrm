#include "qormrelation11model.h"
#include "qormabstractmodel_p.h"

#include <QOrm>

QOrmRelation11Model::QOrmRelation11Model(QOrmTableInfo *meta, int fks) :
    QOrmAbstractModel(meta, Single), fk(fks)
{
}

void QOrmRelation11Model::setRootIndex(const QModelIndex &root)
{
    if (!root.isValid()) return;

    QOrmObjectPtr dt = QtOrm::modelIndex_cast(root);
    if (!dt) return;
    if (dt == d->root) return;


    QOrmAttributeInfo *a = dt->metaTable()->foreignKey(fk);


    QOrmObjectPtr fko = a->readForeignKey(dt.data());
    if (!fko)
    {
        dt->loadForeignKeys(fk);
        fko = a->readForeignKey(dt.data());
    }
    /*
    {
        fko = a->readForeignKey(dt.data());

        fko = QOrmObjectPtr(metaTable()->create());
        if (!fko) return;

        QOrmLoader::load(fko, a->readIndex(dt.data()));
        //fko->load(a->readIndex(dt.data()));

        if (!fko->isSaved()) return;

        a->writeForeingKey(dt.data(), fko);
        d->loadFromDB = true;
    } //*/

    if (!fko) return;
    else d->loadFromDB = true;

    if (d->list.count() > 0)
    {
        disconnect(d->list.first().data(), &QOrmObject::dataChanged, this, &QOrmRelation11Model::objectDataChanged);
        d->list.clear();
    }

    d->root = dt;
    d->list.append(fko);
    connect(fko.data(), &QOrmObject::dataChanged, this, &QOrmRelation11Model::objectDataChanged);

    beginResetModel();
    endResetModel();
}
