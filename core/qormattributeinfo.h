#ifndef QATTRIBUTEINFO_H
#define QATTRIBUTEINFO_H

#include <QString>
#include <QVariant>
#include <QOrmObject>

class QOrmAttributeInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(QString database READ database)
    Q_PROPERTY(QString table    READ table)
    Q_PROPERTY(QString name     READ name)
    Q_PROPERTY(QString property READ propertyName)
    Q_PROPERTY(QVariant::Type type READ type)
    Q_PROPERTY(bool primaryKey  READ isPrimaryKey)
    Q_PROPERTY(bool foreignKey  READ isForeignKey)
    Q_PROPERTY(bool null        READ isNull)
    Q_PROPERTY(int refColumn    READ refcolumn)
    Q_PROPERTY(int index        READ indexIndexes)
public:
    typedef void (QOrmObject::*LoadFn)(QVariant);
    typedef QString (QOrmObject::*StoreFn)();

    QOrmAttributeInfo();
    QOrmAttributeInfo(QString property);
    QOrmAttributeInfo(QString property, QString attribute);
    virtual ~QOrmAttributeInfo();

    QOrmAttributeInfo(const QOrmAttributeInfo &a);
    QOrmAttributeInfo &operator =(const QOrmAttributeInfo & attr);

    template <typename LDfunc, typename STfunc>
    QOrmAttributeInfo(QString property, QString attribute, LDfunc loadf, STfunc storef) :
        QOrmAttributeInfo(property, attribute)
    {
        setFunctions((LoadFn)loadf, (StoreFn)storef);
    }

    bool isValid() const;
    bool isWriteable() const;
    
    QString database();
    QString table();

    int pos();
    QString name();
    QVariant::Type type();
    bool isPrimaryKey();
    bool isForeignKey();
    bool isNull();
    QOrmTableInfo *metaTable();

    QOrmTableInfo *refTable();
    int refcolumn();
    QOrmAttributeInfo refAttribute();

    int indexIndexes();
    int foreignKeyIndex();

    QVariant::Type typeOfProperty();
    QMetaProperty metaProperty();
    QString propertyName();

    static QVariant::Type typeFromString(QString type);

    bool write(QOrmObject *obj, QVariant value);
    QVariant read(QOrmObject *obj);

    QOrmObject* readForeignKey(QOrmObject *obj);
    bool writeForeingKey(QOrmObject *obj, QOrmObject* fko);

    QVariant readIndex(QOrmObject *obj);
    bool writeIndex(QOrmObject *obj, QVariant pk);

    void load(QOrmObject *obj, QVariant value);
    QString store(QOrmObject *obj);

    operator QVariant();

protected slots:
    void setValue();
    void setIndex();
    void setForeignkey();

private:
    class Private;
    Private *d;

    void setFunctions(LoadFn lfn, StoreFn sfn);

    friend class QOrm;
};

class QOrmChildrens
{
public:
    typedef void (QOrmObject::*AppendFn)(QOrmObject*);
    typedef void (QOrmObject::*RemoveFn)(QOrmObject*);
    QOrmChildrens();
    template <typename Olst, typename Afunc, typename Rfunc>
    QOrmChildrens(QString tableName, Olst list, Afunc afn, Rfunc rfn, QString query = "") :
         QOrmChildrens()
    {
        q = query;
        tname = tableName;
        lst = (QtOrm::QOrmChild)list;
        setFunction((AppendFn) afn,(RemoveFn)rfn);
    }

    QString query() const;
    QOrmTableInfo *table();
    QOrmAttributeInfo *foreignkey();
    bool isValid() const;

    void load(QOrmObject *obj);

    QList<QOrmObject *> list(QOrmObject *o);
    void append(QOrmObject *own, QOrmObject *toAdd);
    void remove(QOrmObject *own, QOrmObject *toRem);

private:
    void setFunction(AppendFn afn, RemoveFn rfn);

    QString tname;
    QOrmTableInfo *tab;
    QOrmAttributeInfo *fk;
    QtOrm::QOrmChild lst;
    QString q;
    AppendFn m_afn;
    RemoveFn m_rfn;
    friend class QOrm;
};


Q_DECLARE_METATYPE(QOrmAttributeInfo)
Q_DECLARE_METATYPE(QOrmChildrens)

#endif // QATTRIBUTEINFO_H
