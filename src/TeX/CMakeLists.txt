#
# copy init.tex to INSTALL_ROOT then run gle -mkinittex
#
install(FILES init.tex
		CONFIGURATIONS Release Debug
		RUNTIME DESTINATION . )

install(CODE "execute_process(COMMAND ${INSTALL_ROOT}/bin/gle -mkinittex)")


