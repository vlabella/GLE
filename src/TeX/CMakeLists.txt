#
# -- generate inittex.ini from init.tex
#
set(GLE_DIR ${CMAKE_BINARY_DIR}/gle$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>)
set(GLE gle$<$<CONFIG:Debug>:d>)
# inittex.ini is generated from init.tex by running "gle -mkinittex"
# for gle to run:
#  - init.tex must reside one directory level up from location of the gle executable.
#  - glerc    must reside one directory level up from location of the gle executable.
#  - font.dat must reside one directory level up from location of the gle executable in font folder
# gle creates inittex.ini in the same location as init.tex.
# inittex.ini is moved back to the source folder with init.tex so it can get installed.
# Both files must ship with GLE.
#Generating inittex.ini from init.tex in
#/home/runner/work/GLE/GLE/build
#/home/runner/work/GLE/GLE/src/TeX
#/home/runner/work/GLE/GLE/build/gle
#/home/runner/work/GLE/GLE/build/gle/glerc
add_custom_command(
 	OUTPUT inittex.ini
 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/init.tex
 	WORKING_DIRECTORY ${GLE_DIR}
 	#COMMENT "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}"
 	#COMMENT "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}"
 	#COMMENT "GLE_DIR: ${GLE_DIR}"
 	#COMMENT "GLE: ${GLE}"
 	COMMENT "== Generating inittex.ini from init.tex =="
 	COMMAND ${CMAKE_COMMAND} -E copy ${GLE_DIR}/glerc ${CMAKE_BINARY_DIR}/glerc
 	COMMAND "$<IF:$<PLATFORM_ID:Windows>,,${CMAKE_COMMAND} -E copy ${GLE_DIR}/glerc ${CMAKE_BINARY_DIR}/glerc>"
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${CMAKE_BINARY_DIR}$<$<PLATFORM_ID:Windows>:/gle>/init.tex
 	COMMAND $<IF:$<PLATFORM_ID:Windows>,,"${CMAKE_COMMAND} -E create_symlink ${GLE_DIR}/font ${CMAKE_BINARY_DIR}/font">
	COMMAND ${GLE} -mkinittex
	COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_BINARY_DIR}$<$<PLATFORM_ID:Windows>:/gle>/inittex.ini ${CMAKE_BINARY_DIR}/TeX
 	COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}$<$<PLATFORM_ID:Windows>:/gle>/inittex.ini ${CMAKE_CURRENT_SOURCE_DIR}/inittex.ini
 	COMMAND ${CMAKE_COMMAND} -E rm ${CMAKE_BINARY_DIR}$<$<PLATFORM_ID:Windows>:/gle>/init.tex
 	COMMAND $<IF:$<PLATFORM_ID:Windows>,,"${CMAKE_COMMAND} -E rm ${CMAKE_BINARY_DIR}/glerc">
 	COMMAND $<IF:$<PLATFORM_ID:Windows>,,"${CMAKE_COMMAND} -E rm ${CMAKE_BINARY_DIR}/font">
)

add_custom_target(inittex ALL
	DEPENDS inittex.ini
)

install(FILES
 	inittex.ini
  	init.tex
  	CONFIGURATIONS Release Debug
  	DESTINATION .
)


# old method does not work as inittex.ini does not get included in the distrubtions with cpack
# install(FILES
# 	init.tex
# 	CONFIGURATIONS Release Debug
# 	DESTINATION .
# )
# # make inittex.ini
# # make the inittex.ini in the install folder
# install(CODE
# 	"execute_process(COMMAND \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/gle$<$<CONFIG:Debug>:d> -mkinittex)"
# )
# #install(CODE
# #	"execute_process(COMMAND ${GLE} -mkinittex)"
# #)
# install(FILES
# 	$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/inittex.ini
# 	CONFIGURATIONS Release Debug
#  	DESTINATION .
# )


