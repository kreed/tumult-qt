TEMPLATE = app
CONFIG += warn_on embed_manifest_exe
TARGET = tumult
QT += phonon

target.path = /usr/local/bin
INSTALLS += target

MOC_DIR = build
OBJECTS_DIR = build

SOURCES += src/*.cpp
HEADERS += src/*.h
