# GLE - Graphics Layout Engine

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulae in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

This repo contains the source code to build the executable for GLE.  The [manual](https://github.com/vlabella/gle-manual) and [library of GLE routines and sample code](https://github.com/vlabella/gle-library) that are distributed with the binary packages are contained in separate repositories here: [gle-manual](https://github.com/vlabella/gle-manual) and [gle-library](https://github.com/vlabella/gle-library).

More information and the binary distributions can be found on the GLE website here http://glx.sourceforge.io or using the link below.

[![Download GLE - Graphics Layout Engine](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/glx/files/latest/download)

## Building with CMAKE

GLE can be built on Windows, macOS, and Linux using cmake and system specific toolchains: Visual Studio, Xcode, and gcc.

### Libraries needed to build GLE

* boost
* libtiff
* libpng
* zlib
* pixman
* cairo
* poppler
* jpeg
* ghostscipt (headers only - dll/so is loaded at runtime)
* Qt6 (optional for building GUI qgle  BUILD_GUI=ON)
* curses/ncurses/pdcurses (optional for for building manip BUILD_MANIP=ON)

Dependencies for the above libraries will be needed as well.  For example, the poppler library requires GLIB2 on Linux and Apple platforms and freetype, openjpeg, and iconv on windows. Cmake uses find_package or find_library to resolve the paths for these libraries.  Cmake will search the system default locations. If you installed to other locations then you will need to pass several `<package_name_ROOT>` variables to cmake.  For more information consult cmake documentation on find_package or find_library.  

### Acquire gle-library and gle-manual (optional)

If you want a complete installation with documentation, examples, and include files download (or clone) the [gle-library](https://github.com/vlabella/gle-library) and latest [gle-manual.pdf](https://github.com/vlabella/GLE/releases/latest).  Place them in separate locations and pass the `GLE_EXAMPLES_LIBRARY_PATH` and `GLE_USER_MANUAL_PATH` options in the initial call to cmake.  For example `-DGLE_EXAMPLES_LIBRARY_PATH=/path/to/gle-library -DGLE_USER_MANUAL_PATH=/path/to/gle-manual`.

### Acquire Extra Cmake Modules (ECM)

The [ECM](https://github.com/KDE/extra-cmake-modules) package has the needed code for cmake to find the poppler PDF library.

### Building on Linux or macOS

	cmake -S src -B build
	cd build
	make

To install gle on your machine to `usr/local/bin`

	sudo cmake --install . --config Release

Installation on Linux is now FHS compliant starting with version 4.3.9.  Binaries will be placed in /usr/local/bin. Fonts and includes will be in /usr/local/share/gle and Documentation in /usr/local/share/doc/gle.  This can be altered with `CMAKE_INSTALL_PREFIX` and `DEVELOPER_INSTALLATION` options (see below).

### Building on Windows with Visual Studio as 64 bit executable

To acquire the needed libraries for GLE on Windows it is highly recommended to use [vcpkg](https://vcpkg.io/).  There is a `vcpkg.json` file in the GLE repository that can be utilized to install all the needed dependencies.  Use the `CMAKE_TOOLCHAIN_FILE` and `-DVCPKG_TARGET_TRIPLET` cmake options which will have cmake call vcpkg to install all the needed dependencies.

The command to build GLE on windows (without vcpkg) is

	cmake -S src -B build -A x64 -T host=x64
	cmake --build build

To install gle on your machine in `C:\Program Files\gle`

	cd build & cmake --install . --config Release

Installation on Windows will install all gle files to C:\Program Files\gle by default.  Add C:\Program Files\gle\bin to your PATH environment variable.  The installation location can be changed by setting `CMAKE_INSTALL_PREFEX` on the initial cmake call.

### Post Installation Configuration and Testing

#### Finding Dependencies

After installation run

	gle -finddeps

To have GLE search for it dependency files such as Ghostscript and LaTeX.

To test the installation run.

	gle -info

and output should look something like this on windows

	GLE version:             4.3.9
	Build date:              Oct 29 2025 12:22:58
	GLE_TOP:                 C:\Program Files\GLE
	GLE_BIN_DIR:             C:\Program Files\GLE\bin
	GLE_USRLIB:              
	GhostScript:             C:\Program Files\gs\gs10.06.0\bin\gswin64c.exe
	GS library:              C:\Program Files\gs\gs10.06.0\bin\gsdll64.dll
	Bitmap import:           JPEG, PNG, TIFF, GIF
	Cairo rendering support: Yes
	Poppler PDF support:     Yes

and this on Linux

	GLE version:             4.3.9
	Build date:              Oct 29 2025 10:21:42
	GLE_TOP:                 /usr/local/share/gle
	GLE_BIN_DIR:             /usr/local/bin
	GLE_USRLIB:
	GhostScript:             /usr/bin/gs
	GS library:              /usr/lib/x86_64-linux-gnu/libgs.so
	Bitmap import:           JPEG, PNG, TIFF, GIF
	Cairo rendering support: Yes
	Poppler PDF support:     Yes

#### Optional set GLE_USRLIB and GLE_TOP

GLE will search the path pointed to by environment variable `GLE_USRLIB` for include files.  Set it to a location where you store your include files.  GLE also searches for include files in the current script directory, GLE_BIN_DIR, and GLE_TOP/gleinc by default.  GLE_USRLIB will be searched after these locations.

GLE automatically searches and finds `GLE_TOP` when run but setting it as an environment variable can be helpful. 

### Options that control the build

| Option Name                  | Description                                                                                          | Default Value                     |
|-----------------------------|------------------------------------------------------------------------------------------------------|-----------------------------------|
| `BUILD_GUI`                 | Turn off to disable build of the GUI (`qgle`) that requires Qt.                                      | ON                                |
| `BUILD_MANIP`              | Turn off to disable build of the `manip` program.                                                    | ON (Linux, Apple), OFF (Windows) |
| `CMAKE_INSTALL_PREFIX`     | Set this to a different install location than the default (`/usr/local` or `C:\Program Files\gle`). | System default                    |
| `MSVC_USE_STATIC_RUNTIME`  | Set ON to build against Visual Studio static runtimes (`/MT` instead of `/MD`). Also set `Boost_USE_STATIC_RUNTIME=ON` and ensure all other libraries are built with `/MT`. | OFF                               |
| `ZLIB_USE_STATIC_LIBS`     | Set ON to link to static variants of zlib (`.a`, `.lib` instead of `.so`, `.dll`).                   | OFF                               |
| `GLE_EXAMPLES_LIBRARY_PATH`| Set to the root folder of the `gle-library` folder on your computer. Used during install/packaging.  | Not set                           |
| `GLE_USER_MANUAL_PATH`     | Set to the root folder of the `gle-manual` folder on your computer. Used during install/packaging.   | Not set                           |
| `EXTRA_GSLIB_SEARCH_LOCATION` | Add extra search path for Ghostscript library in `qgle` application.                              | Not set                           |
| `BUILD_GLEBTOOL`           | Build `glebtool` program (deprecated).                                                               | OFF                               |
| `INSTALL_FBUILD`           | Install the `fbuild` program – only needed during GLE build phase.                                   | OFF                               |
| `INSTALL_MAKEFMT`          | Install the `makefmt` program – only needed during GLE build phase.                                  | OFF                               |
| `DEVELOPER_INSTALLATION`   | Linux only: Install all files in staging area; otherwise install in FHS paths on system.             | OFF                               |

### Creating packages with cpack

Cpack can be utilized to create distributable packages.  The gle-manual and gle-library repos should be checked out and built to get included in the package.  Windows platform utilizes NSIS for self installing exe building.  All other packages are for self installers.

	windows:

	cd build & cpack -G "NSIS;ZIP;7Z"

	linux

	cd build & cpack -G "DEB;ZIP"

	macOS

	cd build & cpack -G "DragNDrop;ZIP"


### CI/CD Github Actions

Several GitHib actions exist that build the binaries and distributable packages for all three platforms.  The build artifacts can be found under the Actions tab and then navigate to the appropriate action.

The action "Create Release"  will create a release with a tag from the version number contained in CMakeLists.txt file.  It also will trigger the build and package actions for all three operating systems.  After running this action it is important to bump the version number in CMakeLists.txt file.
