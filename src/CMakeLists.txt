find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Test)

set(
	SOURCES
	parupaintApp.cpp
	parupaintWindow.cpp
	parupaintCanvasView.cpp
	parupaintCanvasPool.cpp
	parupaintCanvasBrush.cpp
	parupaintCanvasObject.cpp
	panvas/parupaintPanvas.cpp
	panvas/parupaintLayer.cpp
	panvas/parupaintFrame.cpp
	main.cpp
)

set(EXECNAME "parupaint")

add_executable(${EXECNAME} ${SOURCES})
target_link_libraries(${EXECNAME} Qt5::Widgets)
