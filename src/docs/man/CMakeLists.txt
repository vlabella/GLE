# docs/man/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)
find_program(DOCBOOK_TO_MAN docbook-to-man)

# 1) Template → gle.1
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gle.1.in
               ${CMAKE_CURRENT_BINARY_DIR}/gle.1 @ONLY)

# 2) SGML → *.1 via docbook-to-man
set(SGML_MAN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/glx.sgml
  ${CMAKE_CURRENT_SOURCE_DIR}/qgle.sgml
)

if(BUILD_GLEBTOOL)
  list(APPEND SGML_MAN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/glebtool.sgml
  )
endif()

if(BUILD_MANIP)
  list(APPEND SGML_MAN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/manip.sgml
  )
endif()


set(MAN1_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/gle.1)

if(NOT DOCBOOK_TO_MAN)
  message(FATAL_ERROR "docbook-to-man not found. Install it (e.g., apt install docbook-to-man).")
endif()

foreach(s ${SGML_MAN_SOURCES})
  get_filename_component(name ${s} NAME_WE)

  set(man    ${CMAKE_CURRENT_BINARY_DIR}/${name}.1)
  set(man_gz ${CMAKE_CURRENT_BINARY_DIR}/${name}.1.gz)

  # Generate the man page
  add_custom_command(
    OUTPUT ${man}
    COMMAND ${DOCBOOK_TO_MAN} ${s} > ${man}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${s}
    VERBATIM
  )

  # Compress it
  add_custom_command(
    OUTPUT ${man_gz}
    COMMAND gzip -f -c ${man} > ${man_gz}
    DEPENDS ${man}
    VERBATIM
  )

  list(APPEND MAN1_GENERATED ${man_gz})
endforeach()

add_custom_target(manpages ALL DEPENDS ${MAN1_GENERATED})

# assume system installation FHS
set(MY_DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
if(DEVELOPER_INSTALLATION OR WIN32)
  # install one directory above executable
  set(MY_DESTINATION man1)
endif()

install(FILES
  ${MAN1_GENERATED}
  CONFIGURATIONS Release Debug
  DESTINATION ${MY_DESTINATION})
