include(mediabackend/mediabackend.pri)
include(settings/settings.pri)
include(streams/streams.pri)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
	hitlist.cpp \
	keys.cpp \
	messagewindow.cpp \
	player.cpp \
	searchbox.cpp \
	streamnode.cpp \
	trayicon.cpp \
	tumult.cpp

HEADERS += \
	hitlist.h \
	keys.h \
	messagewindow.h \
	player.h \
	searchbox.h \
	streamnode.h \
	trayicon.h \
	tumult.h
