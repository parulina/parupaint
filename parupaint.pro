QT += 		widgets websockets

CONFIG+= 	c++11 debug
QMAKE_CXXFLAGS+= -std=c++11 -fdiagnostics-color=auto -Wfatal-errors
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

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
		src/*.h

SOURCES += 	src/core/*.cpp \
		src/net/*.cpp \
		src/overlay/*.cpp \
		src/qtcolorpicker/*.cpp \
		src/*.cpp

RC_ICONS = 	resources/parupaint.ico
RESOURCES +=	*.qrc

TARGET = 	parupaint
DESTDIR =	bin

