QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../include
INCLUDEPATH += ../include/autocomplete

SOURCES += \
    autocompletedisplay.cpp \
    editor.cc \
    directory_tree.cc \
    mainwindow.cc \
    terminal.cc \
    main.cc
    syntax_highlighter.cc \

HEADERS += \
    ../include/editor.h \
    ../include/mainwindow.h \
    ../include/directory_tree.h \
    ../include/syntax_highlighter.h \
    ../include/terminal.h \
    autocompletedisplay.h

FORMS += \
    autocompletedisplay.ui \
    directory_tree.ui \
    mainwindow.ui \
    terminal.ui

RESOURCES += \
    resources


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.txt


