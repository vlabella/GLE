# GLE - Graphics Layout Engine

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulea in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

This repo contains the source code to build the executables for GLE.  The manual and library of GLE routines and samples that are distributed with the binary packages are contained in separate repositories here: [gle-manual](https://github.com/vlabella/gle-manual) and [gle-library](https://github.com/vlabella/gle-library).

More information and the binary distributions can be found on the GLE website here http://glx.sourceforge.io or using the link below.

[![Download GLE - Graphics Layout Engine](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/glx/files/latest/download)

## Building with CMAKE

GLE can be built on Windows, Mac, and Linux using cmake and system specific toolchains: Visual Studio, Xcode, and gcc.

Libraries needed to build GLE are

* boost
* libtiff
* libpng
* zlib
* pixman
* cairo
* jpeg
* Qt5 (for GUI)

Cmake uses find_package or find_library to resolve the paths for these libraries.  These variables must be set in your environment or passed to cmake. If not, cmake will look for them in the system default locations.  For more information consult cmake documentation on find_package or find_library.  

* ZLIB_ROOT
* JPEG_ROOT
* TIFF_ROOT
* PNG_ROOT
* CAIRO_ROOT
* PIXMAN_ROOT
* GHOSTPDL_ROOT
* Qt5_DIR

### Building on linux or OSX

	cmake -S src -B build
	cd build ; make

to install gle on you machine after building

	cd build ; make install

### Building on Windows with Visual Studio as 64 bit executable

	cmake -S src -B build -A x64 -T host=x64
	cmake --build build

To install gle on your machine after building

	cmake -P build/cmake_install.cmake

### options that control the build

 * CMAKE_INSTALL_PREFIX - set this to a different location than the default on your system if desired.
 * USE_STATIC_RUNTIME - set this ON to build against Visual Studio static runtimes: /MT instead of /MD.  Also set Boost_USE_STATIC_RUNTIME=ON and have all other libraries built with /MT as well.
 * ZLIB_USE_STATIC_LIBS - set ON to link to static variants of zlib: .a .lib instead of .so and .dll
 * GLE_EXAMPLES_LIBRARY_PATH - set to the root folder of the gle-library folder on your computer. Utilized during installation and packaging.
 * GLE_USER_MANUAL_PATH - set to the root folder of the gle-manual folder on your computer.  Utilized during installation and packaging.

### Creating packages with cpack

Cpack can be utilized to create distributable packages.  The gle-manual and gle-library repos should be checked out and built to get included in the package.  Windows platform utilizes NSIS for self installing exe building.  All other packages are for self installers.

	windows:

	cd build & cpack -G "NSIS;ZIP;7Z"

	linux

	cd build & cpack -G "DEB;ZIP"

	macOS

	cd build & cpack -G "DragNDrop;ZIP"


