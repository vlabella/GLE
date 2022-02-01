# GLE - Graphics Layout Engine

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulea in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

This repo contains the source code to build the executables for the GLE package.  The manual, samples, and library of gle routines are (will be) in separate repositories.

More information can be found on the gle website here http://glx.sourceforge.io

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
 * GLE_EXAMPLES_LIBRARY_PATH - set to the root folder of the gle-library folder on your computer (only needed if packaging)
 * GLE_USER_MANUAL_PATH - set to the root folder of the gle-manual folder on your computer (only needed if packaging)

### Creating packages with cpack

Cpack can be utilized to create distributable packages.  Note that gle-manual and gle-library repos should be checked out and built on your computer for them to get included in the package.


	cd build & cpack -G "NSIS;ZIP;7Z"

	on linux

	cd build & cpack -G "DEB;ZIP"

	on macOS

	cd build & cpack -G "DragNDrop;ZIP"


