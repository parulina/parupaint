find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(
	SOURCES
	parupaintApp.cpp
	parupaintWindow.cpp
	parupaintCanvasView.cpp
	parupaintCanvasPool.cpp
	parupaintCanvasBrush.cpp
	main.cpp
)


add_executable(parupaint ${SOURCES})
target_link_libraries(parupaint Qt5::Widgets)
