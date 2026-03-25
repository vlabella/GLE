# docs/man/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)
find_program(XMLTO xmlto)

# 1) Template → gle.1
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gle.1.in
               ${CMAKE_CURRENT_BINARY_DIR}/gle.1 @ONLY)

# 2) SGML → *.1 via xmlto
set(SGML_MAN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/glebtool.sgml
  ${CMAKE_CURRENT_SOURCE_DIR}/glx.sgml
  ${CMAKE_CURRENT_SOURCE_DIR}/manip.sgml
  ${CMAKE_CURRENT_SOURCE_DIR}/qgle.sgml
)

set(MAN1_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/gle.1)

if(NOT XMLTO)
  message(FATAL_ERROR "xmlto not found. Install it (e.g., apt install xmlto).")
endif()

foreach(s ${SGML_MAN_SOURCES})
  get_filename_component(name ${s} NAME_WE)
  set(out ${CMAKE_CURRENT_BINARY_DIR}/${name}.1)
  add_custom_command(
    OUTPUT ${out}
    COMMAND ${XMLTO} man ${s}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${s}
    VERBATIM)
  list(APPEND MAN1_GENERATED ${out})
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
