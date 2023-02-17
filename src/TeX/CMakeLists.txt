#
# -- generate inittex.ini from init.tex
#
# set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/../gle)

# # crazy specializaton here
# if(WIN32)
# set(GLE ${OUTPUT_DIR}$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>/gle$<$<CONFIG:Debug>:d> )

# add_custom_command(
# 	OUTPUT ${OUTPUT_DIR}/inittex.ini
# 	DEPENDS init.tex
# 	COMMAND ${CMAKE_COMMAND} -E make-directory ${OUTPUT_DIR}/bin
# 	COMMAND ${CMAKE_COMMAND} -E create_symlink ${OUTPUT_DIR}/gle ${OUTPUT_DIR}/bin/gle
# 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${OUTPUT_DIR}/init.tex
# 	COMMAND ${GLE} -mkinittex
# 	WORKING_DIRECTORY ${OUTPUT_DIR}/bin
# 	COMMENT "Generating inittex.ini from init.tex"
# )
# else()
# set(GLE ${OUTPUT_DIR}$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>/gle$<$<CONFIG:Debug>:d> )

# add_custom_command(
# 	OUTPUT ${OUTPUT_DIR}/inittex.ini
# 	DEPENDS init.tex
# 	COMMAND ${CMAKE_COMMAND} -E make-directory ${OUTPUT_DIR}/bin
# 	COMMAND ${CMAKE_COMMAND} -E create_symlink ${OUTPUT_DIR}/gle ${OUTPUT_DIR}/bin/gle
# 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${OUTPUT_DIR}/init.tex
# 	COMMAND ${GLE} -mkinittex
# 	WORKING_DIRECTORY ${OUTPUT_DIR}/bin
# 	COMMENT "Generating inittex.ini from init.tex"
# )


# endif()
# add_custom_target(inittex ALL DEPENDS
# 	${OUTPUT_DIR}/inittex.ini
# 	gle
# )

# install(FILES
# 	${OUTPUT_DIR}/inittex.ini
# 	${OUTPUT_DIR}/init.tex
# 	CONFIGURATIONS Release Debug
# 	DESTINATION .
# )

install(FILES
	init.tex
	CONFIGURATIONS Release Debug
 	DESTINATION .
)
# make the inittex.ini in the install folder
install(CODE
	"execute_process(COMMAND ${CMAKE_INSTALL_PREFIX}/bin/gle$<$<CONFIG:Debug>:d> -mkinittex)"
)


