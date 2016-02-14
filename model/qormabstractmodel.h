#ifndef QORMABSTRACTMODEL_H
#define QORMABSTRACTMODEL_H

#include <QObject>
#include <QOrmObject>
#include <QAbstractItemModel>

class QOrmTableInfo;
class QOrmBasicLoader;
class QOrmAbstractModelPrivate;

class QOrmAbstractModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
public:
    virtual ~QOrmAbstractModel();

    QOrmTableInfo *metaTable() const;

    Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData (int section, Qt::Orientation orientation, int role ) const;
    Qt::ItemFlags flags (const QModelIndex & index ) const;

    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE bool deleteRows(int row, int count, const QModelIndex &parent = QModelIndex());
    inline bool deleteRow(int row, const QModelIndex &parent = QModelIndex());

    void setRootIndex(const QModelIndex &root);
    QModelIndex rootIndex() const;

    Q_INVOKABLE virtual void load();
    Q_INVOKABLE void submitAll();

    Q_INVOKABLE int count();
    Q_INVOKABLE QOrmObject* at(int row) const;
    Q_INVOKABLE void append(QOrmObject *obj);
    Q_INVOKABLE void remove(QOrmObject *obj);
    Q_INVOKABLE void deleteOne(QOrmObject *obj);
    Q_INVOKABLE void clear();

    QHash<int, QByteArray> roleNames() const;

signals:
    void rootIndexChanged(const QModelIndex &root);

protected:
    QOrmAbstractModel(QOrmTableInfo *metaTable, QOrmBasicLoader *loader = NULL);
    QOrmAbstractModelPrivate *d;

    void objectDataChanged(QOrmObject *obj, int attr);

    void disconnectAll();
    void reconnectAll();
    void replace(QOrmObjectList nlst);
};

inline bool QOrmAbstractModel::deleteRow(int row, const QModelIndex &parent) { return deleteRows(row, 1, parent); }

#endif // QORMABSTRACTMODEL_H
