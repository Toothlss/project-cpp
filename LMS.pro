QT += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LMS
TEMPLATE = app


SOURCES += main.cpp \
    mainwindow.cpp \
    commande.cpp \
    client.cpp \
    commandewidget.cpp \
    clientwidget.cpp \
    database.cpp


HEADERS += mainwindow.h \
    commande.h \
    client.h \
    commandewidget.h \
    clientwidget.h \
    database.h

FORMS += mainwindow.ui \
    commande.ui \
    client.ui

RESOURCES += 
