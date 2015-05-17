QT += 		widgets

CONFIG += 	c++11
OBJECTS_DIR=	.obj
MOC_DIR=	.obj/moc

HEADERS +=	src/panvas/*.h 	src/*.h
SOURCES += 	src/panvas/*.cpp src/*.cpp

TARGET = 	parupaint
DESTDIR =	bin

