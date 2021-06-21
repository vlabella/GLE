add_executable(fbuild
	fbuild.cpp
	../gle/token.cpp
	../gle/cutils.cpp
	../gle/file_io.cpp
)
if(WIN32)
	target_link_libraries(fbuild LINK_PUBLIC Ws2_32.lib ) 
endif()
set_target_properties(fbuild PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 

install(TARGETS fbuild
		CONFIGURATIONS Release Debug
		RUNTIME )

