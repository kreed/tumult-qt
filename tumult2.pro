TEMPLATE = app
CONFIG += warn_on
TARGET = tumult
QT += dbus

target.path = /usr/local/bin
INSTALLS += target

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

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
} else {
	message(Using Phonon)
	DEFINES += USE_PHONON=1
	QT += phonon
}

INCLUDEPATH += \
	src \
	src/streams

RESOURCES += \
	res/res.qrc

SOURCES += \
	src/*.cpp \
	src/streams/*.cpp

HEADERS += \
	src/*.h \
	src/streams/*.h
