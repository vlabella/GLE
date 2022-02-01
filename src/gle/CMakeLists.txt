
set(GLE_SOURCES
	bitmap/img2ps.cpp
	bitmap/lzwencode.cpp
	bitmap/glegif.cpp
	bitmap/glejpeg.cpp
	bitmap/gletiff.cpp
	bitmap/glepng.cpp
	bitmap/ascii85.cpp
	letzfitz/let.cpp
	letzfitz/fit.cpp
	letzfitz/ffit.cpp
	surface/gsurface.cpp
	surface/hide.cpp
	surface/fcontour.cpp
	surface/ffitcontour.cpp
	surface/gcontour.cpp
	tokens/Tokenizer.cpp
	tokens/StringKeyHash.cpp
	tokens/BinIO.cpp
	axis.cpp
	b_tab.cpp
	b_text.cpp
	begin.cpp
	core.cpp
	curve.cpp
	color.cpp
	drawit.cpp
	eval.cpp
	fitbez.cpp
	fitcf.cpp
	fn.cpp
	font.cpp
	general.cpp
	op_def.cpp
	gprint.cpp
	graph.cpp
	graph2.cpp
	key.cpp
	keyword.cpp
	leastsq.cpp
	memory.cpp
	mychar.cpp
	pass.cpp
	polish.cpp
	run.cpp
	savgol.cpp
	sub.cpp
	tex.cpp
	token.cpp
	var.cpp
	cutils.cpp
	texinterface.cpp
	d_ps.cpp
	d_svg.cpp
	d_x.cpp
	d_dummy.cpp
	d_cairo.cpp
	config.cpp
	numberformat.cpp
	glearray.cpp
	cmdline.cpp
	gle-block.cpp
	gle-sourcefile.cpp
	gle-interface.cpp
	gle-base.cpp
	gle-datatype.cpp
	gle-poppler.cpp
	builtin-double.cpp
	cutils.cpp
	file_io.cpp
	specialfunctions.cpp
)

set( GLEBTOOL_SOURCES
	cutils.cpp 
	file_io.cpp 
	glebtool.cpp
)

add_custom_command(
 	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gle_cpp_lib.cpp
 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gle.cpp
 	COMMAND ${CMAKE_COMMAND} -E copy gle.cpp gle_cpp_lib.cpp
  	COMMENT "Generating gle_cpp_lib.cpp from gle.cpp"
  	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
 	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glerc
 	COMMAND ${CMAKE_COMMAND} -E echo begin config gle > ${CMAKE_CURRENT_BINARY_DIR}/glerc
 	COMMAND ${CMAKE_COMMAND} -E echo current = ${PROJECT_VERSION} >> ${CMAKE_CURRENT_BINARY_DIR}/glerc
 	COMMAND ${CMAKE_COMMAND} -E echo end config >> ${CMAKE_CURRENT_BINARY_DIR}/glerc
  	COMMENT "Generating glerc in ${CMAKE_CURRENT_BINARY_DIR}"
  	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(gle_common OBJECT ${GLE_SOURCES})
add_executable( gle $<TARGET_OBJECTS:gle_common> gle.cpp )
add_library( libgle-graphics_s STATIC $<TARGET_OBJECTS:gle_common> gle_cpp_lib.cpp )
add_library( libgle-graphics SHARED $<TARGET_OBJECTS:gle_common> gle_cpp_lib.cpp )
add_custom_target( glerc_file ALL DEPENDS glerc )
set_source_files_properties( gle_cpp_lib.cpp PROPERTIES COMPILE_FLAGS -DHAVE_LIBGLE)

set_target_properties(gle PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 
set_target_properties(libgle-graphics_s PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 
set_target_properties(libgle-graphics PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 

if(UNIX AND NOT APPLE)
	target_link_options(libgle-graphics PUBLIC -fPIC)
	target_link_options(libgle-graphics_s PUBLIC -fPIC)
endif()


target_link_libraries ( gle LINK_PUBLIC 
	${ZLIB_LIBRARIES}
	${JPEG_LIBRARIES}
	${TIFF_LIBRARIES}
	${PNG_LIBRARIES}
	${CAIRO_LIBRARIES}
	${PIXMAN_LIBRARIES}
	)

target_link_libraries ( libgle-graphics LINK_PUBLIC 
	${ZLIB_LIBRARIES}
	${JPEG_LIBRARIES}
	${TIFF_LIBRARIES}
	${PNG_LIBRARIES}
	${CAIRO_LIBRARIES}
	${PIXMAN_LIBRARIES}
	)

if(WIN32)
	target_link_libraries ( libgle-graphics LINK_PUBLIC 
		opengl32.lib
		wsock32.lib
		Ws2_32.lib 
		msimg32.lib
		)
	target_link_libraries( gle LINK_PUBLIC 
		opengl32.lib
		wsock32.lib
		Ws2_32.lib 
		msimg32.lib
		)
endif()
if(APPLE)
	target_link_libraries ( libgle-graphics LINK_PUBLIC 
		lzma
	)
	target_link_libraries ( gle LINK_PUBLIC 
		lzma
	)
endif()

install(TARGETS gle
		CONFIGURATIONS Release Debug
		RUNTIME )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/glerc
		CONFIGURATIONS Release Debug
		DESTINATION . )

