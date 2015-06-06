QT += 		widgets network xml

CONFIG+= 	c++11 debug
QMAKE_CXXFLAGS+= -std=c++11 -fdiagnostics-color=auto -Wfatal-errors
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

# add video export support
#CONFIG += video_export

win32 {
	QMAKE_CXXFLAGS -= -fdiagnostics-color=auto
	CONFIG -= debug
	CONFIG += static
}

LIBS += 	-lKF5Archive -lz
HEADERS +=	src/core/*.h \
		src/net/*.h \
		src/overlay/*.h \
		src/qtcolorpicker/*.hpp \
		src/net/QtWebsocket/*.h \
		src/*.h

SOURCES += 	src/core/*.cpp \
		src/net/*.cpp \
		src/overlay/*.cpp \
		src/qtcolorpicker/*.cpp \
		src/net/QtWebsocket/*.cpp \
		src/*.cpp

video_export {
	HEADERS += src/qtffmpeg/*.h
	SOURCES += src/qtffmpeg/*.cpp
	LIBS    += -lavutil -lavcodec -lavformat -lswscale

	DEFINES += PARUPAINT_VIDEO_EXPORT __STDC_CONSTANT_MACROS
}

RC_ICONS = 	resources/parupaint.ico
RESOURCES +=	*.qrc

VERSION_MAJOR =	0
VERSION_MINOR =	7
VERSION_PATCH =	5
VERSION = 	$${VERSION_MAJOR}$${VERSION_MINOR}$${VERSION_PATCH}

TARGET = 	parupaint
DESTDIR =	bin

