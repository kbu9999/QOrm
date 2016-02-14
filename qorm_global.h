#ifndef QORM_GLOBAL_H
#define QORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QORM_LIBRARY)
#  define QORMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QORMSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <QList>
#include <QString>
#include <QSharedPointer>

class QOrmObject;

#endif // QORM_GLOBAL_H
