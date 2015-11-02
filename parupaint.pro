# basic
CONFIG += 		c++11 debug_and_release
QMAKE_CXXFLAGS +=	-std=c++11 -Wfatal-errors
OBJECTS_DIR =		.obj
MOC_DIR =		.obj/moc

# Colored output for unix
# TODO check if compiler supports this switch
unix:QMAKE_CXXFLAGS += -fdiagnostics-color=auto

# Win32 + release build
win32:release {
	CONFIG -= debug debug_and_release console
	CONFIG += static windows
}

# Normal setup
QT += 			widgets network xml websockets
RESOURCES +=		*.qrc
LIBS +=			-lz

# mac plist doesn't update reliably,
# try clean & rebuild if it doesn't work

win32:RC_ICONS = 	resources/parupaint.ico
macx {
	ICON =	resources/parupaint.icns
	QMAKE_INFO_PLIST = resources/Info.plist
}

HEADERS +=	src/core/*.h \
		src/net/*.h \
		src/overlay/*.h \
		src/bundled/karchive/*.h \
		src/*.h

SOURCES += 	src/core/*.cpp \
		src/net/*.cpp \
		src/overlay/*.cpp \
		src/bundled/karchive/*.cpp \
		$$files(src/*.cpp)

!server_release {
	SOURCES -= 	src/main_server.cpp
}

server_release {
	HEADERS = src/core/*.h \
		  $$files(src/net/*.h) \
		  src/bundled/karchive/*.h \

	SOURCES = src/core/*.cpp \
		  $$files(src/net/*.cpp) \
		  src/bundled/karchive/*.cpp \
		  src/main_server.cpp

	HEADERS -= src/net/parupaintClient.h src/net/parupaintClientInstance.h
	SOURCES -= src/net/parupaintClient.cpp src/net/parupaintClientInstance.cpp

	QT = 	  network gui
}

video_export {
	HEADERS += src/bundled/qtffmpeg/*.h
	SOURCES += src/bundled/qtffmpeg/*.cpp
	LIBS    += -lavutil -lavcodec -lavformat -lswscale

	DEFINES += PARUPAINT_VIDEO_EXPORT __STDC_CONSTANT_MACROS
}

VERSION_MAJOR =	0
VERSION_MINOR =	8
VERSION_PATCH =	5
VERSION = 	$${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

TARGET = 	parupaint
DESTDIR =	bin

