#
# -- makefmt - used during build phase to create font .fmt files
#
add_executable(makefmt
	makefmt.cpp
	parseAFM.cpp
	../gle/cutils.cpp
	../gle/file_io.cpp
)

if(WIN32)
	target_link_libraries(makefmt LINK_PUBLIC Ws2_32.lib )
endif()

set_target_properties(makefmt PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(INSTALL_MAKEFMT)
install(TARGETS makefmt
		CONFIGURATIONS Release Debug
		RUNTIME )
endif()


