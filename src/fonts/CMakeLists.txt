#
# -- font files - create .fve and .afm files for packaging with binary executable
#
# .fve file are created from .gle files of the same name using fbuild
# .afm files are created frin .fmt files using makefmt
#
# all targets (.fve and .afm) files are put in the same /font folder in the distro
#
# -- list of targets to build
#
set(FVE_FILES
	glemark.fve
	plba.fve
	plcc.fve
	plcg.fve
	plci.fve
	plcr.fve
	plcs.fve
	pldr.fve
	plge.fve
	plgg.fve
	plgi.fve
	plsa.fve
	plsg.fve
	plsr.fve
	plss.fve
	plsym1.fve
	plsym2.fve
	plti.fve
	pltr.fve
	texcmb.fve
	texcmex.fve
	texcmitt.fve
	texcmmi.fve
	texcmr.fve
	texcmss.fve
	texcmssb.fve
	texcmssi.fve
	texcmsy.fve
	texcmti.fve
	texcmtt.fve
)

set(FMT_FILES
	glemark.fmt
	plba.fmt
	plcc.fmt
	plcg.fmt
	plci.fmt
	plcr.fmt
	plcs.fmt
	pldr.fmt
	plge.fmt
	plgg.fmt
	plgi.fmt
	plsa.fmt
	plsg.fmt
	plsr.fmt
	plss.fmt
	plsym1.fmt
	plsym2.fmt
	plti.fmt
	pltr.fmt
	pshc.fmt
	pshcb.fmt
	pshcbo.fmt
	pshcdo.fmt
	texcmb.fmt
	texcmex.fmt
	texcmitt.fmt
	texcmmi.fmt
	texcmr.fmt
	texcmsl.fmt
	texcmss.fmt
	texcmssb.fmt
	texcmssi.fmt
	texcmsy.fmt
	texcmti.fmt
	texcmtt.fmt
	cyrillic/arial8.fmt
	cyrillic/arialbd8.fmt
	cyrillic/arialbi8.fmt
	cyrillic/ariali8.fmt
	cyrillic/cokoi8b.fmt
	cyrillic/cokoi8bi.fmt
	cyrillic/cokoi8i.fmt
	cyrillic/cokoi8n.fmt
	cyrillic/times8.fmt
	cyrillic/timesbd8.fmt
	cyrillic/timesbi8.fmt
	cyrillic/timesi8.fmt
)
# specialization for Adboe files the source names are not the same as the target names
set(ADOBE_FMT_FILES
	psagb.fmt
	psagbo.fmt
	psagd.fmt
	psagdo.fmt
	psbd.fmt
	psbdi.fmt
	psbl.fmt
	psbli.fmt
	psc.fmt
	pscb.fmt
	pscbo.fmt
	psco.fmt
	pshb.fmt
	pshbo.fmt
	psh.fmt
	pshn.fmt
	pshnb.fmt
	pshnbo.fmt
	pshno.fmt
	psho.fmt
	psncsb.fmt
	psncsbi.fmt
	psncsi.fmt
	psncsr.fmt
	pspb.fmt
	pspbi.fmt
	pspi.fmt
	pspr.fmt
	pssym.fmt
	pstb.fmt
	pstbi.fmt
	psti.fmt
	pstr.fmt
	pszcmi.fmt
	pszd.fmt
)

set(MAKEFMT ${CMAKE_CURRENT_BINARY_DIR}/../makefmt/$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,Debug,Release>>/makefmt$<$<CONFIG:Debug>:d> )
set(FBUILD  ${CMAKE_CURRENT_BINARY_DIR}/../fbuild/$<$<PLATFORM_ID:Windows>:$<IF:$<CONFIG:debug>,Debug,Release>>/fbuild$<$<CONFIG:Debug>:d> )
set(ADOBE35 Adobe-Core35_AFMs-314)
# all font files are built and put in the binary dir
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/../gle/font)

# Adobe specialization file mapping of adobe names to gle names
foreach( fmt_file ${ADOBE_FMT_FILES} )
	set(OPTIONE -e adobestd.enc)
	if( ${fmt_file} STREQUAL psagb.fmt )
		set(afm_file ITCAvantGarde-Book.afm )
	elseif( ${fmt_file} STREQUAL psagbo.fmt )
		set(afm_file ITCAvantGarde-BookOblique.afm )
	elseif( ${fmt_file} STREQUAL psagd.fmt)
		set(afm_file ITCAvantGarde-Demi.afm)
	elseif( ${fmt_file} STREQUAL psagdo.fmt)
		set(afm_file ITCAvantGarde-DemiOblique.afm)
	elseif( ${fmt_file} STREQUAL psbd.fmt)
		set(afm_file ITCBookman-Demi.afm)
	elseif( ${fmt_file} STREQUAL psbdi.fmt)
		set(afm_file ITCBookman-DemiItalic.afm)
	elseif( ${fmt_file} STREQUAL psbli.fmt)
		set(afm_file ITCBookman-LightItalic.afm)
	elseif( ${fmt_file} STREQUAL psbl.fmt)
		set(afm_file ITCBookman-Light.afm)
	elseif( ${fmt_file} STREQUAL psc.fmt)
		set(afm_file Courier.afm)
	elseif( ${fmt_file} STREQUAL pscb.fmt)
		set(afm_file Courier-Bold.afm)
	elseif( ${fmt_file} STREQUAL pscbo.fmt)
		set(afm_file Courier-BoldOblique.afm)
	elseif( ${fmt_file} STREQUAL psco.fmt)
		set(afm_file Courier-Oblique.afm)
	elseif( ${fmt_file} STREQUAL psh.fmt)
		set(afm_file Helvetica.afm)
	elseif( ${fmt_file} STREQUAL pshb.fmt)
		set(afm_file Helvetica-Bold.afm)
	elseif( ${fmt_file} STREQUAL pshbo.fmt)
		set(afm_file Helvetica-BoldOblique.afm)
	elseif( ${fmt_file} STREQUAL pshn.fmt)
		set(afm_file Helvetica-Narrow.afm)
	elseif( ${fmt_file} STREQUAL pshnb.fmt)
		set(afm_file Helvetica-NarrowBold.afm)
	elseif( ${fmt_file} STREQUAL pshnbo.fmt)
		set(afm_file Helvetica-NarrowBoldOblique.afm)
	elseif( ${fmt_file} STREQUAL pshno.fmt)
		set(afm_file Helvetica-NarrowOblique.afm)
	elseif( ${fmt_file} STREQUAL psho.fmt)
		set(afm_file Helvetica-Oblique.afm)
	elseif( ${fmt_file} STREQUAL psncsb.fmt)
		set(afm_file NewCenturySchlbk-Bold.afm)
	elseif( ${fmt_file} STREQUAL psncsbi.fmt)
		set(afm_file NewCenturySchlbk-BoldItalic.afm)
	elseif( ${fmt_file} STREQUAL psncsi.fmt)
		set(afm_file NewCenturySchlbk-Italic.afm)
	elseif( ${fmt_file} STREQUAL psncsr.fmt)
		set(afm_file NewCenturySchlbk-Roman.afm)
	elseif( ${fmt_file} STREQUAL pspb.fmt)
		set(afm_file Palatino-Bold.afm)
	elseif( ${fmt_file} STREQUAL pspbi.fmt)
		set(afm_file Palatino-BoldItalic.afm)
	elseif( ${fmt_file} STREQUAL pspi.fmt)
		set(afm_file Palatino-Italic.afm)
	elseif( ${fmt_file} STREQUAL pspr.fmt)
		set(afm_file Palatino-Roman.afm)
	elseif( ${fmt_file} STREQUAL pstb.fmt)
		set(afm_file Times-Bold.afm)
	elseif( ${fmt_file} STREQUAL pstbi.fmt)
		set(afm_file Times-BoldItalic.afm)
	elseif( ${fmt_file} STREQUAL psti.fmt)
		set(afm_file Times-Italic.afm)
	elseif( ${fmt_file} STREQUAL pstr.fmt)
		set(afm_file Times-Roman.afm)
	elseif( ${fmt_file} STREQUAL pszcmi.fmt)
		set(afm_file ITCZapfChancery-MediumItalic.afm)
	elseif( ${fmt_file} STREQUAL ssym.fmt)
		set(afm_file Symbol.afm)
		set(OPTIONE )
	elseif( ${fmt_file} STREQUAL pszd.fmt)
		set(afm_file ZapfDingbats.afm)
		set(OPTIONE )
	endif()
	add_custom_command(
		OUTPUT ${OUTPUT_DIR}/${fmt_file}
		DEPENDS ${ADOBE35}/${afm_file}
		COMMAND ${MAKEFMT} ${OPTIONE} -o ${OUTPUT_DIR}/${fmt_file} ${ADOBE35}/${afm_file}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating ${fmt_file} from ${ADOBE35}/${afm_file}"
	)
endforeach(fmt_file)
#
# makefmt - generate .fmt from .afm files
#
foreach( fmt_file ${FMT_FILES} )
	string(REPLACE ".fmt" ".afm" afm_file ${fmt_file} )
	#strip off source folder name if it exists
	get_filename_component(TARGET_FMT ${fmt_file} NAME)
	add_custom_command(
		OUTPUT ${OUTPUT_DIR}/${TARGET_FMT}
		DEPENDS ${afm_file}
		COMMAND ${MAKEFMT} -o ${OUTPUT_DIR}/${TARGET_FMT} ${CMAKE_CURRENT_SOURCE_DIR}/${afm_file}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating ${fmt_file} from ${afm_file}"
	)
endforeach(fmt_file)
#
# fbuild - generate .fve from .gle files
#
foreach( fve_file ${FVE_FILES} )
	string(REPLACE ".fve" ".gle" gle_file ${fve_file} )
	add_custom_command(
		OUTPUT ${OUTPUT_DIR}/${fve_file}
		DEPENDS ${gle_file}
		COMMAND ${FBUILD} ${fve_file}
		COMMAND ${CMAKE_COMMAND} -E rename ${fve_file} ${OUTPUT_DIR}/${fve_file}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating ${fve_file} from ${gle_file}"
	)
endforeach(fve_file)

add_custom_command(
	OUTPUT ${OUTPUT_DIR}/psfont.dat
	COMMAND ${CMAKE_COMMAND} -E copy psfont.dat ${OUTPUT_DIR}/psfont.dat
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
	OUTPUT ${OUTPUT_DIR}/font.dat
	COMMAND ${CMAKE_COMMAND} -E copy font.dat ${OUTPUT_DIR}/font.dat
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

#add_custom_target(
	#create_font_dir ALL
	#COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
	#COMMENT "making font dir"
#)

add_custom_target(fonts ALL
	DEPENDS
#	create_font_dir
	adobe_fmt_fonts
	fmt_fonts
	fve_fonts
	font.dat
	psfont.dat
)

# transform the lists  - prepend the output directory to them and strip off any source directories

list(TRANSFORM FMT_FILES REPLACE "^.*/" "")
list(TRANSFORM FVE_FILES REPLACE "^.*/" "")
list(TRANSFORM ADOBE_FMT_FILES REPLACE "^.*/" "")
list(TRANSFORM FVE_FILES PREPEND "${OUTPUT_DIR}/")
list(TRANSFORM FMT_FILES PREPEND "${OUTPUT_DIR}/")
list(TRANSFORM ADOBE_FMT_FILES PREPEND "${OUTPUT_DIR}/")
add_custom_target(adobe_fmt_fonts DEPENDS
	${ADOBE_FMT_FILES}
	makefmt
)

add_custom_target(fmt_fonts DEPENDS
	adobe_fmt_fonts
	${FMT_FILES}
	makefmt
)

add_custom_target(fve_fonts DEPENDS
	${FVE_FILES}
	fbuild
)

# assume system installation FHS
set(MY_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/font)
if(DEVELOPER_INSTALLATION OR WIN32)
	# install one directory above executable
	set(MY_DESTINATION font)
endif()

install(FILES
	font.dat
	psfont.dat
	${ADOBE_FMT_FILES}
	${FMT_FILES}
	${FVE_FILES}
	CONFIGURATIONS Release Debug
	DESTINATION ${MY_DESTINATION})

