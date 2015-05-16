find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Test)

add_subdirectory(panvas)

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
)


add_executable(parupaint main.cpp ${SOURCES})
target_link_libraries(parupaint Qt5::Widgets)

