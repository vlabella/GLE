#
# -- CMakeLists.txt for GLE - Graphics Layout Engine
#
cmake_minimum_required (VERSION 3.30)
cmake_policy(SET CMP0091 NEW) 	# windows static runtimes selection
cmake_policy(SET CMP0074 NEW)	# uses <PackageName>_ROOT variables.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CMakePrintHelpers)

find_package(ECM CONFIG NO_MODULE) # optional extra-cmake-modules needed for find_poppler
if(ECM_FOUND)
	list(INSERT CMAKE_MODULE_PATH 0 ${ECM_MODULE_PATH})
endif()

set( CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")	# minimum supported by apple
project( gle VERSION 4.3.9 )
add_compile_definitions(GLEVN="${PROJECT_VERSION}")
set( CMAKE_DEBUG_POSTFIX  "d")
#
# -- options
#
set(BUILD_MANIP_DEFAULT ON)
if(WIN32)
	# not working yet
	set(BUILD_MANIP_DEFAULT OFF)
endif()

option(BUILD_GUI "Build the GUI - gle Previewer" ON)
option(BUILD_MANIP "Build manip program" BUILD_MANIP_DEFAULT)
option(BUILD_GLEBTOOL "Build glebtool program - deprecated" OFF)
option(INSTALL_FBUILD "Install the fbuild program - only needed during gle build phase" OFF)
option(INSTALL_MAKEFMT "Install the makefmt program - only needed during gle build phase" OFF)
option(MSVC_USE_STATIC_RUNTIME "Build and link against the windows static runtime (/MT) (windows only)" OFF)
option(GLE_EXAMPLES_LIBRARY_PATH "Path to gle-examples.  Only needed when using cpack to build distributables" "./gle-examples")
option(GLE_USER_MANUAL_PATH "Path to gle-manual when gle-manual.pdf resides.  Only needed when using cpack to build distributables" "./gle-manual")
option(EXTRA_GSLIB_SEARCH_LOCATION "Extra path for searching for gslib on linux and macOS" OFF)
# option(BITMAP_CREATION_POPPLER "Use poppler library for bitmap creation instead of ghostscript" OFF)

if(NOT DEFINED POPPLER_INTERFACE)
	# poppler interface defaults if not set -> CPP for Windows, GLIB for Linux and Apple/Darwin
	# CPP interface to poppler is *more* cross platform and does not rely on GLIB2.
	# vcpkg only installs the cpp interface for windows
	set(POPPLER_INTERFACE_DEFAULT "CPP")
	if(UNIX OR APPLE)
		set(POPPLER_INTERFACE_DEFAULT "GLIB")
	endif()
	set(POPPLER_INTERFACE ${POPPLER_INTERFACE_DEFAULT} CACHE STRING "The poppler (PDFLibarary) interface to utilize GLIB or CPP")
endif()
#
# -- Windows option for static runtimes
#
if(WIN32)
	#
	# -- Windows option for runtime library - assume DLL /MD unless MSVC_USE_STATIC_RUNTIME=ON then use static /MT
	#
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	if(DEFINED MSVC_USE_STATIC_RUNTIME AND MSVC_USE_STATIC_RUNTIME)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		# tag CPACK_SYSTEM_NAME with "s" to denote static runtime - must rebuild the whole name as there is no way to add a tag
		set(CPACK_SYSTEM_NAME "win64s")
		if(${CMAKE_VS_PLATFORM_NAME} STREQUAL "Win32")
			set(CPACK_SYSTEM_NAME "win32s")
		endif()
	endif()
endif()
if (POLICY CMP0069)
	# link-time or inter-process optimization LTO or IPO
	cmake_policy(SET CMP0069 NEW)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT IPO_SUPPORTED)
	if ( IPO_SUPPORTED )
		# only set for release builds - this will apply to all targets: .obj, .exe, .so, .dll, .lib, .a
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
	endif()
endif()
#
# -- cpack information  (need more here for NSIS and others - manual and sample gle files)
#
set(CPACK_PACKAGE_VENDOR "GLE")
set(CPACK_VERBATIM_VARIABLES true)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GLE Graphics Programming Language")
#set(CPACK_PACKAGE_DESCRIPTION_FILE )
#set(CPACK_RESOURCE_FILE_WELCOME )
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../doc/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../doc/README.txt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "GLE")
#
# - NSIS specific settings
#
set(CPACK_NSIS_COMPRESSOR "lzma")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/gui/images\\gle.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/gui/images\\gle.ico")
set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../graphics\\nsis_banner_image.bmp")
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}/../graphics\\nsis_welcome_image.bmp")
set(CPACK_NSIS_WELCOME_TITLE "GLE ${PROJECT_VERSION}")
set(CPACK_NSIS_BRANDING_TEXT "GLE ${PROJECT_VERSION}")
set(CPACK_NSIS_DISPLAY_NAME "GLE ${PROJECT_VERSION}")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_PACKAGE_EXECUTABLES 	"qgle" "GLE Previewer"  )
set(CPACK_NSIS_MENU_LINKS
	"doc/gle-manual.pdf" "GLE Manual"
	"https://glx.sourceforge.io" "GLE Web Site"
	"https://github.com/vlabella/GLE" "GLE GitHub page"
)
# extra commands include finddeps and setting file associations
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	Exec \"$INSTDIR\\bin\\qgle.exe -finddeps '$INSTDIR\\glerc.old'\"
	WriteRegStr HKCR '.gle' '' 'GLEFile'
  	WriteRegStr HKCR 'GLEFile' '' 'GLE Script'
  	WriteRegStr HKCR 'GLEFile\\DefaultIcon' '' '$INSTDIR\\bin\\qgle.exe,0'
  	WriteRegStr HKCR 'GLEFile\\shell\\open\\command' '' '$INSTDIR\\bin\\qgle.exe \"%1\"'
  	WriteRegStr HKCR 'GLEFile\\shell\\edit' '' 'Edit GLE Script'
  	WriteRegStr HKCR 'GLEFile\\shell\\edit\\command' '' '$INSTDIR\\bin\\qgle.exe \"%1\"'
  	System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
")
set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	DeleteRegKey HKCR '.gle'
    DeleteRegKey HKCR 'GLEFile'
")
#
# -- wix specific settings unused no .msi generated
# dont change these GUIDs once generated
#set(CPACK_WIX_UPGRADE_GUID "" )
#set(CPACK_WIX_PRODUCT_GUID "" )
#set(CPACK_WIX_PRODUCT_ICON "")
#
# -- DragNDrop MacOS dmg settings
#
set(CPACK_DMG_FORMAT "UDBZ")

include(CPack)
#
# -- create text file with version information in it for reading in a makefile
#
string(TOLOWER ${CMAKE_PROJECT_NAME} CMAKE_PROJECT_NAME_LC )
FILE(WRITE
${CMAKE_CURRENT_SOURCE_DIR}/../project_info.mak
"#\n"
"# -- project_info.mak - Auto generated by CmakeLists.txt\n"
"#    DO NOT EDIT - ALL CHANGES WILL BE LOST\n"
"#\n"
"PROJECT_NAME		=${CMAKE_PROJECT_NAME}\n"
"PROJECT_NAME_LC		=${CMAKE_PROJECT_NAME_LC}\n"
"SYSTEM_NAME 		=${CPACK_SYSTEM_NAME}\n"
"VERSION_NUMBER		=${PROJECT_VERSION}\n"
"PACKAGE_BASE		=\$(PROJECT_NAME)-\$(VERSION_NUMBER)\n"
"PACKAGE_BASENAME	=\$(PACKAGE_BASE)-\$(SYSTEM_NAME)\n"
)
#
# -- external libraries user can pass ***_ROOT to modify search location
#
find_package(ZLIB REQUIRED)
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG)
find_package(JPEG)
if(BUILD_MANIP)
	# manip needs curses or pdcurses on windows
	if(UNIX OR APPLE)
		find_package(Curses REQUIRED)
	else()
		set(CURSES_FOUND)
		get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
		if(_isMultiConfig)
			find_library(CURSES_RELEASE NAMES pdcurses PATHS ${CURSES_ROOT} PATH_SUFFIXES lib lib64 )
			string(COMPARE NOTEQUAL "${CURSES_RELEASE}" "CURSES_RELEASE-NOTFOUND" CURSES_FOUND)
			find_library(CURSES_DEBUG NAMES pdcursesd PATHS ${CURSES_ROOT} PATH_SUFFIXES lib lib64 )
			set(HAS_DEBUG )
			string(COMPARE NOTEQUAL "${CURSES_DEBUG}" "CURSES_DEBUG-NOTFOUND" HAS_DEBUG)
			if(HAS_DEBUG)
				list(APPEND CURSES_LIBRARIES "optimized")
				list(APPEND CURSES_LIBRARIES ${CURSES_RELEASE})
				list(APPEND CURSES_LIBRARIES "debug")
				list(APPEND CURSES_LIBRARIES ${CURSES_DEBUG})
			else()
				list(APPEND CURSES_LIBRARIES ${CURSES_RELEASE})
			endif()
		else()
			find_library(CURSES_LIBRARIES NAMES pdcurses pdcursesd PATHS ${CURSES_ROOT} PATH_SUFFIXES lib lib64 )
			string(COMPARE NOTEQUAL "${CURSES_LIBRARIES}" "CURSES_LIBRARIES-NOTFOUND" CURSES_FOUND)
		endif()
		find_path(CURSES_INCLUDE_DIRS curses.h PATHS ${CURSES_ROOT} PATH_SUFFIXES include include/pdcurses )
		if(CURSES_FOUND)
			message(STATUS "Found LIBCURSES: ${CURSES_LIBRARIES}" )
		endif()
	endif()
endif()
find_package(TIFF)

if(WIN32 AND TIFF_FOUND)
	#
	# -- find optional packages for LibTIFF only if linking against static libraries on windows
	#
	find_package(LibLZMA) # -- for libTIFF optional
	find_package(JBIG QUIET) # -- for libtiff optional
	if(JBIG_FOUND)
		message(STATUS "Found JBIG" )
	endif()
	find_package(ZSTD) # -- for libtiff optional
	if(ZSTD_FOUND)
		message(STATUS "Found ZSTD" )
	endif()
	#
	# -- libdeflate for libtiff - optional
	#
	set(DEFLATE_FOUND)
	get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(_isMultiConfig)
		find_library(DEFLATE_RELEASE NAMES libdeflate libdeflatestatic PATHS ${DEFLATE_ROOT} PATH_SUFFIXES lib )
		string(COMPARE NOTEQUAL "${DEFLATE_RELEASE}" "DEFLATE_RELEASE-NOTFOUND" DEFLATE_FOUND)
		find_library(DEFLATE_DEBUG NAMES defalted libdeflatestaticd PATHS ${DEFLATE_ROOT} PATH_SUFFIXES lib )
		set(HAS_DEBUG )
		string(COMPARE NOTEQUAL "${DEFLATE_DEBUG}" "DEFLATE_DEBUG-NOTFOUND" HAS_DEBUG)
		if(HAS_DEBUG)
			list(APPEND DEFLATE_LIBRARIES "optimized")
			list(APPEND DEFLATE_LIBRARIES ${DEFLATE_RELEASE})
			list(APPEND DEFLATE_LIBRARIES "debug")
			list(APPEND DEFLATE_LIBRARIES ${DEFLATE_DEBUG})
		else()
			list(APPEND DEFLATE_LIBRARIES ${DEFLATE_RELEASE})
		endif()
	else()
		find_library(DEFLATE_LIBRARIES NAMES deflate deflated libdeflate.a libdeflated.a PATHS ${DEFLATE_ROOT} PATH_SUFFIXES lib )
		string(COMPARE NOTEQUAL "${DEFLATE_LIBRARIES}" "DEFLATE_LIBRARIES-NOTFOUND" DEFLATE_FOUND)
	endif()
	find_path(DEFLATE_INCLUDE_DIRS libdeflate.h PATHS ${DEFLATE_ROOT} PATH_SUFFIXES include )
	if(DEFLATE_FOUND)
		message(STATUS "Found LIBDEFLATE: ${DEFLATE_LIBRARIES}" )
	endif()
endif()
# note findPNG calls findZLIB
find_package(PNG )
# find debug if it exists for multi config generators only
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
	find_library(PNG_DEBUG NAMES pngd libpngd PATHS ${PNG_ROOT} PATH_SUFFIXES lib )
	if(NOT ${PNG_DEBUG} STREQUAL "PNG_DEBUG-NOTFOUND" )
		list(INSERT PNG_LIBRARIES 0 "optimized")
		list(APPEND PNG_LIBRARIES "debug")
		list(APPEND PNG_LIBRARIES ${PNG_DEBUG})
	endif()
endif()
#find_package(Cairo )
#find_package(Pixman )
#
# -- cairo library - cmake does not supply findCAIRO.cmake
#
set(CAIRO_FOUND )
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
	find_library(CAIRO_RELEASE NAMES cairo PATHS ${CAIRO_ROOT} PATH_SUFFIXES lib REQUIRED )
	string(COMPARE NOTEQUAL "${CAIRO_RELEASE}" "CAIRO_RELEASE-NOTFOUND" CAIRO_FOUND)
	find_library(CAIRO_DEBUG NAMES cairod PATHS ${CAIRO_ROOT} PATH_SUFFIXES lib )
	set(HAS_DEBUG )
	string(COMPARE NOTEQUAL "${CAIRO_DEBUG}" "CAIRO_DEBUG-NOTFOUND" HAS_DEBUG)
	if(HAS_DEBUG)
		list(APPEND CAIRO_LIBRARIES "optimized")
		list(APPEND CAIRO_LIBRARIES ${CAIRO_RELEASE})
		list(APPEND CAIRO_LIBRARIES "debug")
		list(APPEND CAIRO_LIBRARIES ${CAIRO_DEBUG})
	else()
		list(APPEND CAIRO_LIBRARIES ${CAIRO_RELEASE})
	endif()
else()
	find_library(CAIRO_LIBRARIES NAMES cairo cairod PATHS ${CAIRO_ROOT} PATH_SUFFIXES lib REQUIRED)
	string(COMPARE NOTEQUAL "${CAIRO_LIBRARIES}" "CAIRO_LIBRARIES-NOTFOUND" CAIRO_FOUND)
endif()
find_path(CAIRO_INCLUDE_DIRS cairo.h PATHS ${CAIRO_ROOT} PATH_SUFFIXES include include/cairo REQUIRED)
if(CAIRO_FOUND)
	message(STATUS "Found CAIRO: ${CAIRO_LIBRARIES}" )
endif()
#
# -- pixman library - cmake does not supply findPIXMAN.cmake
#
set(PIXMAN_FOUND )
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
	find_library(PIXMAN_RELEASE NAMES pixman pixman-1 PATHS ${PIXMAN_ROOT} PATH_SUFFIXES lib REQUIRED)
	string(COMPARE NOTEQUAL "${PIXMAN_RELEASE}" "PIXMAN_RELEASE-NOTFOUND" PIXMAN_FOUND)
	find_library(PIXMAN_DEBUG NAMES pixmand pixmand-1 PATHS ${PIXMAN_ROOT} PATH_SUFFIXES lib )
	set(HAS_DEBUG )
	string(COMPARE NOTEQUAL "${PIXMAN_DEBUG}" "PIXMAN_DEBUG-NOTFOUND" HAS_DEBUG)
	if(HAS_DEBUG)
		list(APPEND PIXMAN_LIBRARIES "optimized")
		list(APPEND PIXMAN_LIBRARIES ${PIXMAN_RELEASE})
		list(APPEND PIXMAN_LIBRARIES "debug")
		list(APPEND PIXMAN_LIBRARIES ${PIXMAN_DEBUG})
	else()
		list(APPEND PIXMAN_LIBRARIES ${PIXMAN_RELEASE})
	endif()
else()
	find_library(PIXMAN_LIBRARIES NAMES pixman pixmand pixman-1 PATHS ${PIXMAN_ROOT} PATH_SUFFIXES lib REQUIRED)
	string(COMPARE NOTEQUAL "${PIXMAN_LIBRARIES}" "PIXMAN_LIBRARIES-NOTFOUND" PIXMAN_FOUND)
endif()
find_path(PIXMAN_INCLUDE_DIRS pixman.h PATHS ${PIXMAN_ROOT} PATH_SUFFIXES include include/pixman-1 REQUIRED)
if(PIXMAN_FOUND)
	message(STATUS "Found PIXMAN: ${PIXMAN_LIBRARIES}" )
endif()
#
# -- poppler
#
find_package(Poppler)
if(POPPLER_FOUND)
	add_compile_definitions(HAVE_POPPLER)
	#if(BITMAP_CREATION_POPPLER)
	#	add_compile_definitions(BITMAP_CREATION_POPPLER)
	#endif()
	if(POPPLER_INTERFACE STREQUAL "CPP")
		find_package(Freetype)
		find_package(openjpeg QUIET)
		find_package(Iconv)
		find_package(BZip2)
	elseif(POPPLER_INTERFACE STREQUAL "GLIB")
		find_package(GLIB2 REQUIRED)
		if(NOT DEFINED GLIB2_GOBJECT_LIBRARIES)
			# findGLIB2 on ubuntu does not find the object library due to outdated version of ECM 5.9
			# works fine on macOS - needs ECM >= 6.7
			find_library(GLIB2_GOBJECT_LIBRARIES NAMES gobject-2.0 HINTS ${PC_GLIB2_LIBDIR})
		endif()
	else()
		message(STATUS "Unknown Poppler interface: ${POPPLER_INTERFACE}")
	endif()
	message(STATUS "Poppler_INCLUDE_DIRS: ${Poppler_INCLUDE_DIRS}")
endif()
#
# -- GHOSTscript or GHOSTPDL include directory only since gle links to installed version at runtime
#
find_path(GDIR1 iapi.h PATHS ${GHOSTPDL_ROOT} PATH_SUFFIXES psi base devices ghostscript )
list(APPEND GHOSTPDL_INCLUDE_DIRS ${GDIR1} )
find_path(GDIR2 gdevdsp.h PATHS ${GHOSTPDL_ROOT} PATH_SUFFIXES psi base devices ghostscript )
list(APPEND GHOSTPDL_INCLUDE_DIRS ${GDIR2} )
find_path(GDIR3 gserrors.h PATHS ${GHOSTPDL_ROOT} PATH_SUFFIXES psi base devices ghostscript )
list(APPEND GHOSTPDL_INCLUDE_DIRS ${GDIR3} )

include_directories(
	${ZLIB_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${TIFF_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	${PIXMAN_INCLUDE_DIRS}
	${GHOSTPDL_INCLUDE_DIRS}
	${Poppler_INCLUDE_DIRS}
	${GLIB2_INCLUDE_DIR}
)
#
# -- compile time definitions
#
if(APPLE)
	# use __APPLE__ in source code to identify apple/mac platforms which is defined by compiler
	add_compile_definitions(
		HAVE_CONFIG_H
		HAVE_CAIRO
		HAVE_LIBZ
		HAVE_LIBPNG
		HAVE_LIBJPEG
		HAVE_LIBTIFF
		HAVE_EXTRA_FONTS
		HAVE_LIBGLU_H
	)
	CHECK_INCLUDE_FILE(assert.h    HAVE_ASSERT_H)
	CHECK_INCLUDE_FILE(dlfcn.h     HAVE_DLFCN_H)
	CHECK_INCLUDE_FILE(fcntl.h     HAVE_FCNTL_H)
	CHECK_INCLUDE_FILE(inttypes.h  HAVE_INTTYPES_H)
	CHECK_INCLUDE_FILE(io.h        HAVE_IO_H)
	CHECK_INCLUDE_FILE(search.h    HAVE_SEARCH_H)
	CHECK_INCLUDE_FILE(stdint.h    HAVE_STDINT_H)
	CHECK_INCLUDE_FILE(stdlib.h    HAVE_STDLIB_H)
	CHECK_INCLUDE_FILE(stdlib.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    HAVE_STDDEF_H)
	CHECK_INCLUDE_FILE(string.h    HAVE_STRING_H)
	CHECK_INCLUDE_FILE(strings.h   HAVE_STRINGS_H)
	CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
	CHECK_INCLUDE_FILE(sys/time.h  HAVE_SYS_TIME_H)
	CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
	CHECK_INCLUDE_FILE(sys/wait.h  HAVE_SYS_WAIT_H)
	CHECK_INCLUDE_FILE(sys/stat.h  HAVE_SYS_STAT_H)
	CHECK_INCLUDE_FILE(unistd.h    HAVE_UNISTD_H)
	CHECK_INCLUDE_FILE(sys/socket.h HAVE_SOCKETS)
	#set(CONFIG_H_SOURCE "config.h.cmake")
#	add_compile_options(-framework OpenGL)
#	add_compile_options(-std=c++11)  per target config
#	add_compile_options(-Wno-c++11-compat-deprecated-writable-strings)
	add_compile_options(-Wno-unsequenced)
	add_compile_options(-Wno-macro-redefined)
	add_compile_options(-Wno-deprecated-register)
	add_compile_options(-Wno-deprecated-declarations)
	add_compile_definitions(BOOST_MATH_DISABLE_DEPRECATED_03_WARNING)
endif()
if(UNIX AND NOT APPLE)
	# use __unix__ in source code to identify linux platforms which is defined by compiler
	add_compile_definitions(
		HAVE_CONFIG_H
		HAVE_CAIRO
		HAVE_LIBZ
		HAVE_LIBPNG
		HAVE_LIBJPEG
		HAVE_LIBTIFF
		HAVE_EXTRA_FONTS
		HAVE_LIBGLU_H
	)
	CHECK_INCLUDE_FILE(assert.h    HAVE_ASSERT_H)
	CHECK_INCLUDE_FILE(dlfcn.h     HAVE_DLFCN_H)
	CHECK_INCLUDE_FILE(fcntl.h     HAVE_FCNTL_H)
	CHECK_INCLUDE_FILE(inttypes.h  HAVE_INTTYPES_H)
	CHECK_INCLUDE_FILE(io.h        HAVE_IO_H)
	CHECK_INCLUDE_FILE(search.h    HAVE_SEARCH_H)
	CHECK_INCLUDE_FILE(stdint.h    HAVE_STDINT_H)
	CHECK_INCLUDE_FILE(stdlib.h    HAVE_STDLIB_H)
	CHECK_INCLUDE_FILE(stdlib.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    HAVE_STDDEF_H)
	CHECK_INCLUDE_FILE(string.h    HAVE_STRING_H)
	CHECK_INCLUDE_FILE(strings.h   HAVE_STRINGS_H)
	CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
	CHECK_INCLUDE_FILE(sys/time.h  HAVE_SYS_TIME_H)
	CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
	CHECK_INCLUDE_FILE(sys/stat.h  HAVE_SYS_STAT_H)
	CHECK_INCLUDE_FILE(sys/wait.h  HAVE_SYS_WAIT_H)
	CHECK_INCLUDE_FILE(unistd.h    HAVE_UNISTD_H)
	CHECK_INCLUDE_FILE(sys/socket.h HAVE_SOCKETS)
	#set(CONFIG_H_SOURCE "config.h.cmake")
	add_compile_options(-Wno-deprecated)
	add_compile_options(-Wno-write-strings)
	add_compile_options(-Wno-stringop-overflow)
	add_compile_options(-Wno-deprecated-declarations)
	add_compile_options(-fpermissive)
	add_compile_options(-fPIC)
	#add_compile_options(-no-pie)
endif()
if(WIN32)
	# use _WIN32 in source code to identify windows platforms which is defined by compiler
	add_compile_definitions(
		NO_CYGWIN
		NOMINMAX
		HAVE_CONFIG_H
		HAVE_CAIRO
		HAVE_LIBZ
		HAVE_LIBPNG
		HAVE_LIBJPEG
		HAVE_LIBTIFF
		HAVE_EXTRA_FONTS
		CAIRO_WIN32_STATIC_BUILD
		HAVE_SOCKETS
		HAVE_LIBGLU_H
		_HAS_STD_BYTE=0
	)
	CHECK_INCLUDE_FILE(assert.h    HAVE_ASSERT_H)
	CHECK_INCLUDE_FILE(dlfcn.h     HAVE_DLFCN_H)
	CHECK_INCLUDE_FILE(fcntl.h     HAVE_FCNTL_H)
	CHECK_INCLUDE_FILE(inttypes.h  HAVE_INTTYPES_H)
	CHECK_INCLUDE_FILE(io.h        HAVE_IO_H)
	CHECK_INCLUDE_FILE(search.h    HAVE_SEARCH_H)
	CHECK_INCLUDE_FILE(stdint.h    HAVE_STDINT_H)
	CHECK_INCLUDE_FILE(stdlib.h    HAVE_STDLIB_H)
	CHECK_INCLUDE_FILE(stdlib.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    STDC_HEADERS)
	CHECK_INCLUDE_FILE(stddef.h    HAVE_STDDEF_H)
	CHECK_INCLUDE_FILE(string.h    HAVE_STRING_H)
	CHECK_INCLUDE_FILE(strings.h   HAVE_STRINGS_H)
	CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
	CHECK_INCLUDE_FILE(sys/time.h  HAVE_SYS_TIME_H)
	CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
	CHECK_INCLUDE_FILE(sys/stat.h  HAVE_SYS_STAT_H)
	CHECK_INCLUDE_FILE(sys/wait.h  HAVE_SYS_WAIT_H)
	CHECK_INCLUDE_FILE(unistd.h    HAVE_UNISTD_H)
	# should comment out and modify code so these warnings are not thrown
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_definitions(_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
	add_compile_definitions(_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
	add_compile_options(/FS)
	add_compile_options(/wd4290)
	add_compile_options(/wd4244)
	add_compile_options(/wd4099)
	add_compile_options(/wd4305) # double to float truncation
	add_compile_options(/wd4805) # = unsafe mix of type in and type bool
	add_compile_options(/wd4804) # < unsafe use of type bool
	add_compile_options(/wd4267) # 64 bit only: conversion from size_t to int possible loss of data
	add_compile_options(/wd4477) # 'sprintf' : format string '%08lx' requires an argument of type 'unsigned long ', but variadic argument 1 has type 'DWORD64' [C:\sandbox\gle\build-x64\gui\qgle.vcxproj])
	#set(CONFIG_H_SOURCE "config.h.cmake")
endif()

if(CURSES_HAVE_CURSES_H)
	add_compile_definitions(HAVE_CURSES_H)
endif()
if(CURSES_HAVE_NCURSES_H)
	add_compile_definitions(HAVE_NCURSES_H)
endif()
if(HAVE_SYS_TYPES_H)
    add_compile_definitions(HAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    add_compile_definitions(HAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    add_compile_definitions(HAVE_STDDEF_H)
endif()
if(HAVE_ASSERT_H)
	add_compile_definitions(HAVE_ASSERT_H)
endif()
if(HAVE_DLFCN_H)
	add_compile_definitions(HAVE_DLFCN_H)
endif()
if(HAVE_FCNTL_H)
	add_compile_definitions(HAVE_FCNTL_H)
endif()
if(HAVE_INTTYPES_H)
	add_compile_definitions(HAVE_INTTYPES_H)
endif()
if(HAVE_IO_H)
	add_compile_definitions(HAVE_IO_H)
endif()
if(HAVE_SEARCH_H)
	add_compile_definitions(HAVE_SEARCH_H)
endif()
if(HAVE_STDLIB_H)
	add_compile_definitions(HAVE_STDLIB_H)
endif()
if(STDC_HEADERS)
	add_compile_definitions(STDC_HEADERS)
endif()
if(HAVE_STRING_H)
	add_compile_definitions(HAVE_STRING_H)
endif()
if(HAVE_STRINGS_H)
	add_compile_definitions(HAVE_STRINGS_H)
endif()
if(HAVE_CSTRING)
	add_compile_definitions(HAVE_CSTRING)
endif()
if(HAVE_SYS_TIME_H)
	add_compile_definitions(HAVE_SYS_TIME_H)
endif()
if(HAVE_SYS_WAIT_H)
	add_compile_definitions(HAVE_SYS_WAIT_H)
endif()
if(HAVE_SYS_STAT_H)
	add_compile_definitions(HAVE_SYS_STAT_H)
endif()
if(HAVE_UNISTD_H)
	add_compile_definitions(HAVE_UNISTD_H)
endif()
set(POPPLER_LIBRARIES_GLE )
if(POPPLER_FOUND)
	add_compile_definitions(HAVE_POPPLER)
	if(POPPLER_INTERFACE STREQUAL "CPP")
		add_compile_definitions(POPPLER_CPP)
		set(POPPLER_LIBRARIES_GLE
			${Poppler_LIBRARIES}
			Iconv::Iconv
			Freetype::Freetype
			${OPENJPEG_LIBRARIES}
			${BZIP2_LIBRARIES}
	 	)
	elseif(POPPLER_INTERFACE STREQUAL "GLIB")
		add_compile_definitions(POPPLER_GLIB)
		set(POPPLER_LIBRARIES_GLE
			${Poppler_LIBRARIES}
			GLIB2::GLIB2
			${GLIB2_GOBJECT_LIBRARIES}
	 	)
	else()
		message(STATUS "ERROR Unknown Poppler Interface ${POPPLER_INTERFACE}")
	endif()
endif()

add_subdirectory(gle)
if(BUILD_MANIP)
	add_subdirectory(manip)
endif()
add_subdirectory(fbuild)
add_subdirectory(makefmt)
add_subdirectory(fonts)
add_subdirectory(TeX)
add_dependencies(fbuild gle)
add_dependencies(makefmt gle)
add_dependencies(fonts fbuild makefmt)
add_dependencies(inittex gle fonts fbuild makefmt)
if(BUILD_GUI)
add_subdirectory(gui)
add_dependencies(qgle gle inittex)
endif()
if(BUILD_GLEBTOOL)
add_subdirectory(glebtool)
add_dependencies(glebtool gle)
endif()

# documentation file installation in doc folder
install(FILES
	../doc/LICENSE.txt
	../doc/README.txt
	../doc/ChangeLog.txt
	CONFIGURATIONS Release Debug
	DESTINATION doc
)
#
# -- install include files from gle-library repo
#
install(DIRECTORY
	${GLE_EXAMPLES_LIBRARY_PATH}/include/
	DESTINATION gleinc
	CONFIGURATIONS Release Debug
	OPTIONAL
	PATTERN "*.gle"

)
#
# -- install manual from gle-manual repo
#
install(FILES
	${GLE_USER_MANUAL_PATH}/gle-manual.pdf
	DESTINATION doc
	CONFIGURATIONS Release Debug
	OPTIONAL
)

