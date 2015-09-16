QT += 		widgets network xml

CONFIG+= 	c++11 debug_and_release
QMAKE_CXXFLAGS+= -std=c++11 -fdiagnostics-color=auto -Wfatal-errors
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

# add video export support
#CONFIG += video_export

win32 {
	QMAKE_CXXFLAGS -= -fdiagnostics-color=auto
	CONFIG -= debug debug_and_release
	CONFIG += static console
}

LIBS += 	-lz
HEADERS +=	src/core/*.h \
		src/net/*.h \
		src/overlay/*.h \
		src/bundled/karchive/*.h \
		src/bundled/qtwebsocket/*.h \
		src/*.h

SOURCES += 	src/core/*.cpp \
		src/net/*.cpp \
		src/overlay/*.cpp \
		src/bundled/karchive/*.cpp \
		src/bundled/qtwebsocket/*.cpp \
		src/*.cpp

video_export {
	HEADERS += src/bundled/qtffmpeg/*.h
	SOURCES += src/bundled/qtffmpeg/*.cpp
	LIBS    += -lavutil -lavcodec -lavformat -lswscale

	DEFINES += PARUPAINT_VIDEO_EXPORT __STDC_CONSTANT_MACROS
}

RC_ICONS = 	resources/parupaint.ico
macx {
	ICON =	resources/parupaint.icns
	QMAKE_INFO_PLIST = resources/Info.plist
}
RESOURCES +=	*.qrc

VERSION_MAJOR =	0
VERSION_MINOR =	8
VERSION_PATCH =	4
VERSION = 	$${VERSION_MAJOR}$${VERSION_MINOR}$${VERSION_PATCH}

TARGET = 	parupaint
DESTDIR =	bin

