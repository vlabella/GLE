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
if(EXTRA_GSLIB_SEARCH_LOCATION)
target_compile_definitions(qgslibloader.cpp -DEXTRA_GSLIB_SEARCH_LOCATION="${EXTRA_GSLIB_SEARCH_LOCATION}")
endif()
#
# - care must be taken with library order for Linux
#
# order matters: first static files (and libgle), then dynamic libraries
list(APPEND QGLE_LIBRARIES
	gle-graphics_s
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
	${PIXMAN_LIBRARIES}
)
if(UNIX OR APPLE)
	# these come in QT on windows ??
	list(APPEND QGLE_LIBRARIES
		${JPEG_LIBRARIES}
		TIFF::TIFF
	)
endif()

list(APPEND QGLE_LIBRARIES
	Qt::Core
	Qt::Gui
	Qt::Network
	Qt::Widgets
	Qt::OpenGL
	${CAIRO_LIBRARIES}
	${CMAKE_DL_LIBS}
)

if(UNIX AND NOT APPLE)
	list(APPEND QGLE_LIBRARIES
 		glut
 		GLU
 		GL
# 		Qt5::QWaylandIntegrationPlugin
	)
endif()
if(WIN32)
	list(APPEND QGLE_LIBRARIES
	 msimg32.lib
	 opengl32.lib
	 glu32.lib
	)
endif()

if(ZSTD_FOUND)
	list(APPEND QGLE_LIBRARIES
 		zstd::libzstd_static
 	)
endif()

if(DEFLATE_FOUND)
	list(APPEND QGLE_LIBRARIES
		${DEFLATE_LIBRARIES}
	)
endif()

if(JBIG_FOUND)
	list(APPEND QGLE_LIBRARIES
		JBIG::JBIG
	)
endif()

if(LibLZMA_FOUND)
	list(APPEND QGLE_LIBRARIES
		${LIBLZMA_LIBRARIES}
	)
endif()

if(POPPLER_FOUND)
	list(APPEND QGLE_LIBRARIES
		${POPPLER_LIBRARIES_GLE}
 	)
endif()

target_link_libraries ( qgle LINK_PUBLIC
	${QGLE_LIBRARIES}
	)

if(UNIX AND NOT APPLE)
 	target_link_options(qgle PUBLIC -no-pie)
	qt_import_plugins( qgle
  		INCLUDE_BY_TYPE platforms
	)
endif()

if(APPLE)
	#openGL is deprecated on macOS starting at 10.9; this silences the warning
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
target_compile_features(qgle PUBLIC cxx_std_17)
set_target_properties(qgle PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

install(TARGETS qgle RUNTIME BUNDLE DESTINATION bin )
