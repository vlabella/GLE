add_executable(glebtool
	glebtool.cpp
	../gle/cutils.cpp
	../gle/file_io.cpp
)

if(WIN32)
	target_link_libraries(glebtool LINK_PUBLIC Ws2_32.lib ) 
endif()

set_target_properties(glebtool PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) 
install(TARGETS glebtool
		CONFIGURATIONS Release Debug
		RUNTIME )


