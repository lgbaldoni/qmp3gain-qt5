SUBDIRS += src
TEMPLATE = subdirs 
CONFIG += warn_on \
          qt \
          thread
QMAKE_DISTCLEAN += qmp3gain.pro.user
