#
#  -- CMakeLists.txt developer diagnostic tools
#
add_executable(findgs
	findgs.cpp
)

add_executable(glebtool
	glebtool.cpp
	../gle/cutils.cpp
	../gle/file_io.cpp
)

add_dependencies(findgs gle)
add_dependencies(glebtool gle)

if(WIN32)
	target_link_libraries(findgs LINK_PUBLIC Ws2_32.lib )
	target_link_libraries(glebtool LINK_PUBLIC Ws2_32.lib )
endif()

set_target_properties(findgs PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(glebtool PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

add_custom_target(run_findgs ALL
 	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 	COMMAND $<TARGET_FILE:findgs>
 	COMMENT " === Running Find GhostScript ==="
 	DEPENDS findgs
)

if(INSTALL_DEV_TOOLS)
	install(TARGETS findgs
			CONFIGURATIONS Release Debug
			RUNTIME
	)
	install(TARGETS glebtool
	        CONFIGURATIONS Release Debug
	        RUNTIME
    )
endif()






