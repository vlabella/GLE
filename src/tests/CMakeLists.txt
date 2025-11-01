add_executable(findgs
	findgs.cpp
)

if(WIN32)
	target_link_libraries(findgs LINK_PUBLIC Ws2_32.lib )
endif()

set_target_properties(findgs PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

add_custom_target(run_findgs ALL
 	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
 	COMMAND $<TARGET_FILE:findgs>
 	COMMENT " === Running Find GhostScript ==="
 	DEPENDS findgs
)

if(INSTALL_TESTS)
install(TARGETS findgs
		CONFIGURATIONS Release Debug
		RUNTIME )
endif()

