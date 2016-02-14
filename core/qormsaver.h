#ifndef QORMSACER_H
#define QORMSACER_H

#include <QtCore>

class QOrmObject;

class QOrmSaver
{
public:
    void pushToSave(QOrmObject *obj);
    void pushToDelete(QOrmObject *obj);
    bool run();
    //bool waitForFinnished();
    QString lastErrors();
    QString lastQuery();

private:
    QOrmSaver();

    class Private;
    Private *d;

    friend class QOrmAbstractModel;
    friend class QOrmObject;
};

#endif // QORMSACER_H
