add_executable(manip 
	cell.cpp 
	cmd.cpp 
	eval.cpp 
	filemenu.cpp 
	fn.cpp 
	general.cpp 
	keyword.cpp 
	manip.cpp 
	mjl.cpp 
	polish.cpp 
	sub.cpp 
	token.cpp 
	var.cpp 
	varargs.cpp 
	$<$<PLATFORM_ID:Linux>:unix_extra.cpp> 
	$<$<PLATFORM_ID:Linux>:unixinkey.cpp>
	$<$<PLATFORM_ID:Linux>:unixscr.cpp>
	$<$<PLATFORM_ID:Darwin>:unix_extra.cpp> 
	$<$<PLATFORM_ID:Darwin>:unixinkey.cpp>
	$<$<PLATFORM_ID:Darwin>:unixscr.cpp>
	../gle/cutils.cpp
	../gle/file_io.cpp
)

if(WIN32)
	target_link_libraries(manip LINK_PUBLIC Ws2_32.lib ) 
endif()

if(UNIX)
	target_link_libraries(manip PRIVATE ${CURSES_LIBRARIES})
	target_include_directories(manip SYSTEM PRIVATE ${CURSES_INCLUDE_DIRS})
endif()

set_target_properties(manip  PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 
install(TARGETS manip 
		CONFIGURATIONS Release Debug
		RUNTIME )


