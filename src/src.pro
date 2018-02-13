APP_MAJOR_VER = 0
APP_MINOR_VER = 9
APP_SUBMINOR_VER = 0
# comment the following 2 lines unless development version is to be made
#APP_LASTCOMMIT_ID = $$system(git rev-parse --short HEAD)
#APP_LASTCOMMIT_DATE = $$system('git log --pretty=format:"%ad" --date=iso -1')

TEMPLATE = app
CONFIG += warn_on \
    thread \
	qt
TARGET = qmp3gain
DESTDIR = ../bin
RESOURCEDIR = ../resources
win32 {
	RC_FILE = $$RESOURCEDIR/win32/qmp3gain.rc
	INSTALLDIR = ../installer/win32/generated
}
else {
	INSTALLDIR_BIN = /usr/bin
	INSTALLDIR_SHARE = /usr/share/qmp3gain
	CONFIG += link_pkgconfig
	PKGCONFIG += phonon4qt5
}
QT += help \
	gui \
	multimedia \
	xml \
	webkitwidgets
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
include( ../translations/translations.pri )
HELPS = ../help/qmp3gain
include( ../help/help.pri )
QMAKE_DISTCLEAN += object_script.qmp3gain.Debug \
	object_script.qmp3gain.Release \
	object_script.qmp3gain
DEFINES += APP_MAJOR_VER=\"\\\"$$APP_MAJOR_VER\\\"\"
DEFINES += APP_MINOR_VER=\"\\\"$$APP_MINOR_VER\\\"\"
DEFINES += APP_SUBMINOR_VER=\"\\\"$$APP_SUBMINOR_VER\\\"\"
!isEmpty(APP_LASTCOMMIT_ID) {
	DEFINES += APP_LASTCOMMIT_ID=\"\\\"$$APP_LASTCOMMIT_ID\\\"\"
}
!isEmpty(APP_LASTCOMMIT_DATE) {
	QMAKE_CXXFLAGS += -DAPP_LASTCOMMIT_DATE=\"\\\"$$APP_LASTCOMMIT_DATE\\\"\"
}

win32 {
	target.path = $${INSTALLDIR}
	INSTALLS += target

	resources.path = $${INSTALLDIR}/resources/sounds
	resources.files = $$RESOURCEDIR/sounds/*
	INSTALLS += resources

	bins.path = $${INSTALLDIR}
	bins.files = $$[QT_INSTALL_BINS]/assistant.exe
	bins.files += $$[QT_INSTALL_BINS]/libgcc_s_dw2-1.dll
	bins.files += $$[QT_INSTALL_BINS]/mingwm10.dll
	bins.files += $$[QT_INSTALL_BINS]/phonon4.dll
	bins.files += $$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlodbc4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtCLucene4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtCore4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtGui4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtHelp4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtNetwork4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtSql4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtWebKit4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtXml4.dll
	bins.files += $$[QT_INSTALL_BINS]/QtXmlPatterns4.dll
	INSTALLS += bins

	plugins_imageformats.path = $${INSTALLDIR}/imageformats
	plugins_imageformats.files = $$[QT_INSTALL_PLUGINS]/imageformats/qgif4.dll
	plugins_imageformats.files += $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg4.dll
	INSTALLS += plugins_imageformats

	plugins_sqldrivers.path = $${INSTALLDIR}/sqldrivers
	plugins_sqldrivers.files = $$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlite4.dll
	INSTALLS += plugins_sqldrivers

	# backend mp3gain and nsis win32 installer script
	installer.path = $${INSTALLDIR}
	installer.files = $$DESTDIR/mp3gain.exe
	isEmpty(APP_LASTCOMMIT_ID) {
		APP_VERSION_NSI = "$${APP_MAJOR_VER}.$${APP_MINOR_VER}.$${APP_SUBMINOR_VER}"
	}
	else {
		APP_VERSION_NSI = "$${APP_MAJOR_VER}.$${APP_MINOR_VER}.$${APP_SUBMINOR_VER}.$${APP_LASTCOMMIT_ID}"
	}
	installer.extra = "echo !define VERSION \"$$APP_VERSION_NSI\" > $$INSTALLDIR/../qmp3gain.nsh"
	INSTALLS += installer
}
else {
	target.path = $${INSTALLDIR_BIN}
	INSTALLS += target

	resources_sounds.path = $${INSTALLDIR_SHARE}/resources/sounds
	resources_sounds.files = $$RESOURCEDIR/sounds/*
	INSTALLS += resources_sounds

	resources_kde4_desktop.path = /usr/share/applications
	resources_kde4_desktop.files = $$RESOURCEDIR/kde4/applications/qmp3gain.desktop
	INSTALLS += resources_kde4_desktop

	resources_kde4_icons_16.path = /usr/share/icons/hicolor/16x16/apps
	resources_kde4_icons_16.files = $$RESOURCEDIR/kde4/icons/16x16/qmp3gain.png
	INSTALLS += resources_kde4_icons_16

	resources_kde4_icons_32.path = /usr/share/icons/hicolor/32x32/apps
	resources_kde4_icons_32.files = $$RESOURCEDIR/kde4/icons/32x32/qmp3gain.png
	INSTALLS += resources_kde4_icons_32
}
