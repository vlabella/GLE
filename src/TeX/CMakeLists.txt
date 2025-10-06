#
# -- generate inittex.ini from init.tex
#
set(GLE_TOP     ${CMAKE_BINARY_DIR}$<$<PLATFORM_ID:Windows>:/gle>)
set(GLE_BIN_DIR ${CMAKE_BINARY_DIR}/gle$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>)
set(GLE         gle$<$<CONFIG:Debug>:d>)
# inittex.ini is generated from init.tex by running "gle -mkinittex"
# for gle to run:
#  - init.tex must reside one directory level up from location of the gle executable.
#  - glerc    must reside one directory level up from location of the gle executable.
#  - font.dat must reside one directory level up from location of the gle executable in font folder
# gle creates inittex.ini in the same location as init.tex.
# inittex.ini is moved back to the source folder with init.tex so it can get installed.
# inittex.ini is copied to the ${CMAKE_BINARY_DIR}/TeX folder to convince cmake that target has been built
# Both init.tex and inittex.ini ship with GLE.
#
#/home/runner/work/GLE/GLE/build
#/home/runner/work/GLE/GLE/src/TeX
#/home/runner/work/GLE/GLE/build/gle
#/home/runner/work/GLE/GLE/build/gle/glerc
add_custom_command(
 	OUTPUT inittex.ini
 	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/init.tex
 	WORKING_DIRECTORY ${GLE_BIN_DIR}
 	#COMMENT "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}"
 	#COMMENT "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}"
 	#COMMENT "GLE_BIN_DIR: ${GLE_BIN_DIR}"
 	#COMMENT "GLE: ${GLE}"
 	COMMENT "== Generating inittex.ini from init.tex =="
 	COMMAND "$<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_COMMAND};-E;copy;${GLE_BIN_DIR}/glerc;${GLE_TOP}/glerc>"
 	COMMAND "$<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_COMMAND};-E;create_symlink;${GLE_BIN_DIR}/font;${GLE_TOP}/font>"
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${GLE_TOP}/init.tex
 	# set GLE_TOP to this version of GLE to not confuse with an installed version
 	COMMAND ${CMAKE_COMMAND} -E env GLE_TOP=${GLE_TOP} ./gle -info
 	COMMAND ${CMAKE_COMMAND} -E env GLE_TOP=${GLE_TOP} ./gle -mkinittex
	#COMMAND "$<IF:$<PLATFORM_ID:Windows>,set;GLE_TOP=${GLE_TOP}&&;gle;-mkinittex,GLE_TOP=${GLE_TOP};gle;-mkinittex>"
 	COMMAND ${CMAKE_COMMAND} -E copy ${GLE_TOP}/inittex.ini ${CMAKE_CURRENT_SOURCE_DIR}
 	COMMAND ${CMAKE_COMMAND} -E copy ${GLE_TOP}/inittex.ini ${CMAKE_BINARY_DIR}/TeX
 	COMMAND ${CMAKE_COMMAND} -E rm ${GLE_TOP}/init.tex
 	COMMAND ${CMAKE_COMMAND} -E rm ${GLE_TOP}/inittex.ini
 	COMMAND "$<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_COMMAND};-E;rm;${CMAKE_BINARY_DIR}/glerc>"
 	COMMAND "$<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_COMMAND};-E;rm;${CMAKE_BINARY_DIR}/font>"
 	VERBATIM
 	COMMAND_EXPAND_LISTS
)

add_custom_target(inittex ALL
	DEPENDS inittex.ini
)

install(FILES
  	init.tex
  	inittex.ini
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



#"$<IF:$<PLATFORM_ID:Windows>,set;GLE_TOP=${GLE_TOP}&&;gle;-mkinittex,GLE_TOP=${GLE_TOP};gle;-mkinittex>"
	#COMMAND ${GLE} -mkinittex
# #
# # -- generate inittex.ini from init.tex
# #
# set(GLE_BIN_DIR ${CMAKE_BINARY_DIR}/gle$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,/Debug,/Release>>)
# set(GLE gle$<$<CONFIG:Debug>:d>)
# # inittex.ini is generated from init.tex by running "gle -mkinittex"
# # for gle to run:
# #  - init.tex must reside one directory level up from location of the gle executable.
# #  - glerc    must reside one directory level up from location of the gle executable.
# # gle creates inittex.ini in the same location as init.tex.
# # inittex.ini is moved back to the source folder with init.tex so it can get installed.
# # Both files must ship with GLE.
# add_custom_command(
#  	OUTPUT inittex.ini
#  	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/init.tex
#  	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/glerc ${CMAKE_BINARY_DIR}/..
#  	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/init.tex ${GLE_DIR}/../init.tex
#  	COMMAND ${GLE} -mkinittex
#  	COMMAND ${CMAKE_COMMAND} -E rename ${GLE_DIR}/../inittex.ini ${CMAKE_CURRENT_SOURCE_DIR}/inittex.ini
#  	COMMAND ${CMAKE_COMMAND} -E rm ${GLE_DIR}/../init.tex
#  	COMMAND ${CMAKE_COMMAND} -E rm ${CMAKE_BINARY_DIR}/../glerc
#  	WORKING_DIRECTORY ${GLE_DIR}
#  	COMMENT "Generating inittex.ini from init.tex"
# )

# add_custom_target(inittex ALL DEPENDS
#  	inittex.ini
#  	gle
# )

# install(FILES
#   	inittex.ini
#   	init.tex
#   	CONFIGURATIONS Release Debug
#   	DESTINATION .
# )


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


