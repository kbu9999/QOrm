#ifndef QORMTABLEMODEL_H
#define QORMTABLEMODEL_H

#include <QOrmAbstractModel>

template <class T>
class QOrmTableModel : public QOrmAbstractModel
{
public:
    QOrmTableModel(QOrmBasicLoader *loader = NULL) : QOrmAbstractModel(T::staticMetaTable, loader) { }

    inline T *at(int row) const { return qobject_cast<T*>(QOrmAbstractModel::at(row)); }

    inline void append(T* obj) { QOrmAbstractModel::append(obj); }
    inline void remove(T* obj) { QOrmAbstractModel::remove(obj); }

    inline QList<T*> toList() {
        QList<T*> ret;
        for (int i = 0; i < rowCount(); i++) ret << at(i);
        return ret;
    }
};




#endif // QORMTABLEMODEL_H
