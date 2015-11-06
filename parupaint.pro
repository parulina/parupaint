OBJECTS_DIR =		.obj
MOC_DIR =		.obj/moc
DESTDIR =		bin

CONFIG += 		c++11 debug_and_release
QMAKE_CXXFLAGS +=	-std=c++11 -Wfatal-errors
TARGET = 		parupaint

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
QT += 			widgets network xml
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
			$$files(src/net/ws/*.h) \
			$$files(src/overlay/*.h) \
			$$files(src/bundled/*.h) \
			$$files(src/bundled/karchive/*.h) \
			$$files(src/*.h)

SOURCES += 		$$files(src/core/*.cpp) \
			$$files(src/net/*.cpp) \
			$$files(src/net/ws/*.cpp) \
			$$files(src/overlay/*.cpp) \
			$$files(src/bundled/*.cpp) \
			$$files(src/bundled/karchive/*.cpp) \
			$$files(src/*.cpp)
SOURCES -=		src/main_server.cpp

INCLUDEPATH +=		src/bundled/ffmpeg



nogui {
 !build_pass:message("Compiling without GUI (standalone).")
 QT -=		widgets
 HEADERS -=	$$files(src/net/parupaintClient*.h) $$files(src/overlay/*.h)
 SOURCES -=	$$files(src/net/parupaintClient*.cpp) $$files(src/overlay/*.cpp)
 HEADERS -= 	$$files(src/*.h)
 SOURCES -= 	$$files(src/*.cpp)

 SOURCES += 	src/main_server.cpp
 TARGET = 	parupaint-server
}

noxml {
 !build_pass:message("Compiling without ORA support.")
 QT -= xml
}

noffmpeg {
 !build_pass:message("Compiling without ffmpeg support.")
 HEADERS -=		src/core/parupaintAVWriter.h
 SOURCES -=		src/core/parupaintAVWriter.cpp
 DEFINES +=		PARUPAINT_NOFFMPEG
}

VERSION_MAJOR =	0
VERSION_MINOR =	8
VERSION_PATCH =	5
VERSION = 	$${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}
