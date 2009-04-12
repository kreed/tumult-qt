TEMPLATE = app
CONFIG += warn_on
TARGET = tumult
QT += phonon dbus

target.path = /usr/local/bin
INSTALLS += target

MOC_DIR = build
OBJECTS_DIR = build

SOURCES += \
	src/*.cpp \
	src/streams/*.cpp

HEADERS += \
	src/*.h \
	src/streams/*.h
