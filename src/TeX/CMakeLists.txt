#
# -- generate inittex.ini from init.tex
#
set(GLE_DIR ${CMAKE_BINARY_DIR}/gle$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>)
set(GLE gle$<$<CONFIG:Debug>:d>)
# inittex.ini is generated from init.tex by running "gle -mkinittex"
# init.tex must reside one directory level up from location of the gle executable.
# inittex.ini is put in the same location as init.tex.
# inittex.ini is moved back to the source folder with init.tex so it can get installed.
# Both files must ship with GLE.
add_custom_command(
 	OUTPUT inittex.ini
 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/init.tex
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${GLE_DIR}/../init.tex
 	COMMAND ${GLE} -mkinittex
 	COMMAND ${CMAKE_COMMAND} -E rename ${GLE_DIR}/../inittex.ini ${CMAKE_CURRENT_SOURCE_DIR}/inittex.ini
 	COMMAND ${CMAKE_COMMAND} -E rm ${GLE_DIR}/../init.tex
 	WORKING_DIRECTORY ${GLE_DIR}
 	COMMENT "Generating inittex.ini from init.tex"
)

add_custom_target(inittex ALL DEPENDS
 	inittex.ini
 	gle
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


