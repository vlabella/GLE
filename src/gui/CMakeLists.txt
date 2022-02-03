set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# -- Qt5  set Qt5_DIR
#
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS 
	Widgets
	Network
	DBus
	Gui
	OpenGL
	REQUIRED)

add_executable(qgle WIN32 MACOSX_BUNDLE
	3dviewer.cpp
	about.cpp
	amove.cpp
	arc.cpp
	circle.cpp
	colourbutton.cpp
	colourpicker.cpp
	component.cpp
	consolewindow.cpp
	dialogues.cpp
#    downloader.cpp
	drawingobject.cpp
	ellipse.cpp
	fileinfo.cpp
	gledrawing.cpp
	grid.cpp
	line.cpp
	main.cpp
	mainwindow.cpp
	newfile.cpp
	objectblocks.cpp
	polararray.cpp
	propertyeditor.cpp
	propertymodel.cpp
	qgle_statics.cpp
	qgs.cpp
	qgslibloader.cpp
	serverthread.cpp
	settings.cpp
	settings_dialogue.cpp
	snapline.cpp
	text.cpp
	variantdelegate.cpp
	$<$<PLATFORM_ID:Darwin>:macspecific.cpp>
	qgle.qrc
	$<$<PLATFORM_ID:Windows>:qgle.rc>
)
# no jpeg as its contianed in Qt
target_link_libraries ( qgle LINK_PUBLIC 
	${ZLIB_LIBRARIES}
	${TIFF_LIBRARIES}
	${PNG_LIBRARIES}
	${PIXMAN_LIBRARIES}
	libgle-graphics_s
	Qt::Core
	Qt::Gui
	Qt::Network
	Qt::Widgets
	Qt::OpenGL
	${CAIRO_LIBRARIES}
	)

if(UNIX AND NOT APPLE)
 	target_link_libraries ( qgle LINK_PUBLIC 
 		glut
 		GLU
 		GL 
# 		Qt5::QWaylandIntegrationPlugin
 	)
 	target_link_options(qgle PUBLIC -no-pie)
	qt_import_plugins( qgle
  		INCLUDE_BY_TYPE platforms
	)
endif()

if(WIN32)
	target_link_libraries(qgle LINK_PUBLIC 
	 msimg32.lib 
	 opengl32.lib
	 glu32.lib
	)
endif()
if(APPLE)
	#openGL is depricated on OSX starting 10.9 this silences the warning
	add_compile_definitions(Q_OS_APPLE)
	add_compile_options(-Wno-deprecated-declarations)
	target_link_libraries ( qgle LINK_PUBLIC 
	lzma
	"-framework AppKit"
	"-framework GSS"
	"-framework IOKit"
	"-framework Security"
    "-framework Cocoa"
    "-framework OpenGL"
    "-framework SystemConfiguration"
    "-framework Carbon"
	)
endif()
set_target_properties(qgle PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 

install(TARGETS qgle RUNTIME BUNDLE DESTINATION bin )
