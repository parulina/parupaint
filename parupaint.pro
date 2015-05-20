QT += 		widgets

CONFIG += 	c++11 debug
QMAKE_CXXFLAGS+= -std=c++11 -fdiagnostics-color=auto
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

LIBS += 	-lz
HEADERS +=	src/panvas/*.h \
		src/overlay/*.h \
		src/qtcolorpicker/*.hpp \
		src/stroke/*.cpp \
		src/karchive/*.h \
		src/*.h

SOURCES += 	src/panvas/*.cpp \
		src/overlay/*.cpp \
		src/qtcolorpicker/*.cpp \
		src/stroke/*.cpp \
		src/karchive/*.cpp \
		src/*.cpp

RC_ICONS = 	resources/parupaint.ico
RESOURCES +=	*.qrc

TARGET = 	parupaint
DESTDIR =	bin

