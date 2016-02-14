#ifndef QORMOBJECTMODEL_H
#define QORMOBJECTMODEL_H

#include <QOrmObject>
#include <QOrmTableModel>
#include <QOrmAbstractModel>

class QOrmAbstractObjectModel : public QOrmAbstractModel
{
    Q_OBJECT
protected:
    QOrmAbstractObjectModel(QOrmTableInfo *m);
    virtual ~QOrmAbstractObjectModel();

public:
    void setOrmObject(QOrmObjectPtr obj);
    void setRootIndex(const QModelIndex &root);
};

template <class _t>
class QOrmObjectModel :public QOrmAbstractObjectModel
{
public:
    QOrmObjectModel() :  QOrmAbstractObjectModel(_t::staticMetaTable) { }
};

#endif // QORMOBJECTMODEL_H
