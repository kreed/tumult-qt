lessThan(QT_VERSION, 4.6.0) {
	error(Tumult requires Qt 4.6 or greater)
}

TEMPLATE = app
CONFIG += warn_on
TARGET = tumult
QT += dbus

target.path = /usr/local/bin
INSTALLS += target

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

include(src/src.pri)

QT_MEDIA = $$(QT_MEDIA)
exists($$QT_MEDIA) {
	message(Using QtMobility multimedia API)

	CONFIG(release) {
		QT_MEDIA_BIN = $$QT_MEDIA/build/Release/bin
	} else {
		QT_MEDIA_BIN = $$QT_MEDIA/build/Debug/bin
	}

	DEFINES += LIBPATH=\'\"$$QT_MEDIA_BIN\"\' USE_QTMEDIA=1
	INCLUDEPATH += $$QT_MEDIA $$QT_MEDIA/multimedia
	LIBS += -lQtMedia -L$$QT_MEDIA_BIN
	QMAKE_RPATHDIR += $$QT_MEDIA_BIN
	SOURCES += $$QTMEDIA_SOURCES
	HEADERS += $$QTMEDIA_HEADERS
} else {
	message(Using Phonon)
	DEFINES += USE_PHONON=1
	QT += phonon
	SOURCES += $$PHONON_SOURCES
	HEADERS += $$PHONON_HEADERS
}

RESOURCES += res/res.qrc
SOURCES += src/main.cpp
