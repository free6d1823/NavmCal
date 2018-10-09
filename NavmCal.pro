#-------------------------------------------------
#
# Project created by QtCreator 2018-09-19T13:54:01
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NavmCal
TEMPLATE = app
LIBS += -lglut -lGLU

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp\
        mainwindow.cpp \
    imagewin.cpp \
    control1.cpp \
    inifile/inifile.cpp \
    imglab/Mat.cpp \
    imglab/ImgProcess.cpp \
    imglab/vecmath.cpp \
    control0.cpp \
    control2.cpp \
    controlpanel.cpp \
    control3.cpp \
    control4.cpp \
    view1.cpp \
    view2.cpp \
    view34.cpp

HEADERS  += mainwindow.h \
    imagewin.h \
    inifile/inifile.h \
    imglab/Mat.h \
    imglab/ImgProcess.h \
    imglab/vecmath.h \
    common.h \
    controlpanel.h \
    fpview.h

FORMS    += mainwindow.ui \
    control1.ui \
    control0.ui \
    control2.ui \
    control3.ui \
    control4.ui

RESOURCES += \
    images.qrc
