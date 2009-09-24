include(mediabackend/mediabackend.pri)
include(streams/streams.pri)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
	keys.cpp \
	messagewindow.cpp \
	player.cpp \
	searchbox.cpp \
	trayicon.cpp \
	tumult.cpp

HEADERS += \
	keys.h \
	messagewindow.h \
	player.h \
	searchbox.h \
	trayicon.h \
	tumult.h
