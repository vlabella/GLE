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
	$<$<PLATFORM_ID:Windows>:turbo.cpp>
	$<$<PLATFORM_ID:Windows>:turboscr.cpp>
	$<$<PLATFORM_ID:Windows>:inkey.cpp>
	../gle/cutils.cpp
	../gle/file_io.cpp
)

if(WIN32)
	target_link_libraries(manip LINK_PUBLIC
		opengl32.lib
		wsock32.lib
		Ws2_32.lib
		msimg32.lib
	)
endif()

target_link_libraries(manip PRIVATE ${CURSES_LIBRARIES})
target_include_directories(manip SYSTEM PRIVATE ${CURSES_INCLUDE_DIRS})

set_target_properties(manip PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set(TARGET_NAME manip)
install(TARGETS ${TARGET_NAME}
        CONFIGURATIONS Release Debug
        RUNTIME
    )

# if(MSVC_INSTALL_MSVC_RUNTIME OR NOT WIN32)
#     install(TARGETS ${TARGET_NAME}
#       CONFIGURATIONS Release Debug
#       RUNTIME
#       BUNDLE DESTINATION bin  # setting this to Applications causes problems with cpack
#     )
# else()
#     install(TARGETS ${TARGET_NAME}
#       CONFIGURATIONS Release Debug
#       RUNTIME
#         PRE_EXCLUDE_REGEXES
#             ".*msvcp.*"
#             ".*vcruntime.*"
#             ".*concrt.*"
#             ".*api-ms-win.*"
#             ".*ucrtbase.*"
#     )
# endif()

