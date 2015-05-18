QT += 		widgets

CONFIG += 	c++11
QMAKE_CXXFLAGS += -std=c++11
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

HEADERS +=	src/panvas/*.h src/overlay/*.h src/qtcolorpicker/*.hpp src/*.h
SOURCES += 	src/panvas/*.cpp src/overlay/*.cpp src/qtcolorpicker/*.cpp src/*.cpp
RESOURCES +=	*.qrc

TARGET = 	parupaint
DESTDIR =	bin

