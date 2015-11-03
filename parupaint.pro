# vim: set filetype=make
# basic
CONFIG += 		c++11 debug_and_release
QMAKE_CXXFLAGS +=	-std=c++11 -Wfatal-errors
OBJECTS_DIR =		.obj
MOC_DIR =		.obj/moc

# Colored output for unix
# TODO check if compiler supports this switch
unix:QMAKE_CXXFLAGS += -fdiagnostics-color=auto

# Windows: disable console in Makefile.Release
win32:CONFIG(release, debug|release) {
	CONFIG += static windows
}
win32:CONFIG(debug, debug|release) {
	CONFIG += console
}

# Normal setup
# TODO optional xml
QT += 			widgets network xml websockets
RESOURCES +=		*.qrc
LIBS +=			-lz

# mac plist doesn't update reliably,
# try clean & rebuild if it doesn't work

win32:RC_ICONS = 	resources/parupaint.ico
macx {
 ICON =			resources/parupaint.icns
 QMAKE_INFO_PLIST = 	resources/Info.plist
}

HEADERS +=		$$files(src/core/*.h) \
			$$files(src/net/*.h) \
			$$files(src/overlay/*.h) \
			$$files(src/bundled/karchive/*.h) \
			$$files(src/*.h)

SOURCES += 		$$files(src/core/*.cpp) \
			$$files(src/net/*.cpp) \
			$$files(src/overlay/*.cpp) \
			$$files(src/bundled/karchive/*.cpp) \
			$$files(src/*.cpp)

noffmpeg {
 message("Compiling without ffmpeg support.")
 HEADERS -=		src/core/parupaintAVWriter.h
 SOURCES -=		src/core/parupaintAVWriter.cpp
 DEFINES +=		PARUPAINT_NOFFMPEG
 unix:$$system(touch src/core/parupaintPanvasWriter.cpp)
}

!server_release {
 SOURCES -= 	src/main_server.cpp
}

VERSION_MAJOR =	0
VERSION_MINOR =	8
VERSION_PATCH =	5
VERSION = 	$${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

TARGET = 	parupaint
DESTDIR =	bin

