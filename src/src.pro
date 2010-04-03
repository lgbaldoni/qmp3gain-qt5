TEMPLATE = app
CONFIG += warn_on \
    thread \
	qt \
	help
TARGET = qmp3gain
DESTDIR = ../bin
QT += gui \
	xml \
	webkit
# Input
HEADERS += doubleprogressbar.h \
    mymessagebox.h \
    constantgainchangedialog.h \
    aboutdialog.h \
    advancedoptionsdialog.h \
    backenddialog.h \
    disclaimerdialog.h \
    logoptionsdialog.h \
	mainwindow.h \
	donationdialog.h
FORMS += constantgainchangedialog.ui \
    aboutdialog.ui \
    advancedoptionsdialog.ui \
    backenddialog.ui \
    disclaimerdialog.ui \
    logoptionsdialog.ui \
    mainwindow.ui \
    donationdialog.ui
SOURCES += doubleprogressbar.cpp \
    mymessagebox.cpp \
    constantgainchangedialog.cpp \
    aboutdialog.cpp \
    advancedoptionsdialog.cpp \
    backenddialog.cpp \
    disclaimerdialog.cpp \
    logoptionsdialog.cpp \
    main.cpp \
	mainwindow.cpp \
	donationdialog.cpp
RESOURCES += ../resources/qmp3gain.qrc
TRANSLATIONS = ../translations/qmp3gain_hu.ts
HELPS = ../help/qmp3gain
include( ../translations/translations.pri )
include( ../help/help.pri )
QMAKE_DISTCLEAN += object_script.qmp3gain.Debug \
	object_script.qmp3gain.Release
