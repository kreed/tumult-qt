include(mediabackend/mediabackend.pri)
include(settings/settings.pri)
include(streams/streams.pri)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
	keys.cpp \
	messagewindow.cpp \
	player.cpp \
	searchbox.cpp \
	statistics.cpp \
	streamnode.cpp \
	trayicon.cpp \
	tumult.cpp

HEADERS += \
	hitlist.h \
	keys.h \
	messagewindow.h \
	player.h \
	searchbox.h \
	statistics.h \
	streamnode.h \
	trayicon.h \
	tumult.h
