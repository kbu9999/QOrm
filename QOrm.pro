#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T22:58:09
#
#-------------------------------------------------

QT       += core gui sql widgets
#QT       -= gui

TARGET = QOrm
TEMPLATE = lib

CONFIG += c++11

DEFINES += QORM_LIBRARY

SOURCES += core/qorm.cpp \
    core/qormobject.cpp \
    core/qormtableinfo.cpp \
    core/qormloader.cpp \
    core/qormattributeinfo.cpp \
    core/qormtable.cpp \
    core/qormsaver.cpp \
    model/qormabstractmodel.cpp \
    #model/qormrelation11model.cpp \
    #model/qormrelation1nmodel.cpp \
    #model/qormobjectmodel.cpp \
    #model/qormtablemodel.cpp \
    #gui/qormrelationdelegate.cpp \
    #gui/qormforeignkeydelegate.cpp \
    #gui/qormcomboboxhelper.cpp
    private/qormqueryparser.cpp

PUBLIC_HEADERS += qorm_global.h \
    core/qorm.h \
    core/qormloader.h \
    core/qormobject.h \
    core/qormtableinfo.h \
    core/qormattributeinfo.h \
    core/qormtable.h \
    core/qormsaver.h \
    model/qormtablemodel.h \
    model/qormabstractmodel_p.h \
    model/qormabstractmodel.h \
    #model/qormrelation11model.h \
    #model/qormrelation1nmodel.h \
    #model/qormobjectmodel.h \
    #gui/qormrelationdelegate.h \
    #gui/qormforeignkeydelegate.h \
    #gui/qormcomboboxhelper.h

PRIVATE_HEADERS += private/qormtableinfo_p.h \
    private/qormattributeinfo_p.h

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \
    private/qormqueryparser.h \
    private/qormobject_p.h

unix {
    target.path = /usr/lib

    hcore.path = /usr/include/QOrm/core
    hcore.files = core/qorm.h \
        core/qormloader.h \
        core/qormobject.h \
        core/qormtableinfo.h \
        core/qormattributeinfo.h \
        core/qormtable.h \
        core/qormsaver.h

    #hgui.path = /usr/include/QOrm/gui
    #hgui.files = gui/qormrelationdelegate.h \
    #    gui/qormforeignkeydelegate.h \
    #    gui/qormcomboboxhelper.h

    hmodel.path = /usr/include/QOrm/model
    hmodel.files = model/qormtablemodel.h \
        model/qormabstractmodel_p.h \
        model/qormabstractmodel.h
        #model/qormrelation11model.h \
        #model/qormrelation1nmodel.h \
        #model/qormobjectmodel.h \

    hs.path = /usr/include/QOrm
    hs.files = qorm_global.h \
        QOrm\
        QOrmObject\
        QOrmAbstractModel\
        QOrmAttributeInfo\
        #QOrmObjectModel\
        #QOrmRelation1NModel\
        #QOrmRelation11Model\
        QOrmTableInfo\
        QOrmLoader\
        QOrmTableModel

    INSTALLS += target hcore hgui hmodel hs
}
