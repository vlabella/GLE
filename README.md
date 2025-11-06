# GLE - Graphics Layout Engine

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulae in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

This repo contains the source code to build the executable for GLE.  The [manual](https://github.com/vlabella/gle-manual) and [library of GLE routines and sample code](https://github.com/vlabella/gle-library) that are distributed with the binary packages are contained in separate repositories here: [gle-manual](https://github.com/vlabella/gle-manual) and [gle-library](https://github.com/vlabella/gle-library).

More information and the binary distributions can be found on the GLE website here http://glx.sourceforge.io or using the link below.

[![Download GLE - Graphics Layout Engine](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/glx/files/latest/download)

## Building GLE

GLE can be built on Windows, macOS, and Linux using cmake and system specific compilers: Visual Studio, Xcode, and gcc.

### Get the GLE source code

Clone the GLE repository or download and unpack the source code.

### Acquire gle-library and gle-manual

For a complete installation with documentation, examples, and include files download (or clone) the [gle-library](https://github.com/vlabella/gle-library) and latest [gle-manual.pdf](https://github.com/vlabella/GLE/releases/latest).  Place them in separate locations and pass the `GLE_EXAMPLES_LIBRARY_PATH` and `GLE_USER_MANUAL_PATH` options in the initial call to cmake.  For example `-DGLE_EXAMPLES_LIBRARY_PATH=/path/to/gle-library -DGLE_USER_MANUAL_PATH=/path/to/gle-manual`.





### Building on Linux

Building on linux requires the gcc compiler and the standard C/C++ libraries.

1. Install the needed packages

	```
	sudo apt-get install cmake freeglut3-dev libboost-dev libcairo-dev libdeflate-dev libgs-dev 
	libjpeg-turbo8-dev liblzma-dev libpixman-1-dev libpng-dev libtiff-dev libz-dev qt6-base-dev 
	libpoppler-dev libpoppler-cpp-dev libpoppler-glib-dev libpoppler-qt6-dev libglib2.0-dev 
	extra-cmake-modules
    ```

2. Run cmake in the gle directory

	```
	cmake -S src -B build -DCMAKE_BUILD_TYPE=Release
	-DGLE_EXAMPLES_LIBRARY_PATH=/path/to/gle-library 
	-DGLE_USER_MANUAL_PATH=/path/to/gle-manual
	```

3. Build 

	```
	cmake --build build
	```

4. Install gle in `usr/local/bin`

	```
	sudo cmake --install build
	```

Installation destinations are [FHS](https://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html) compliant starting with version 4.3.9.  Binaries will be placed in `/usr/local/bin`. Font, config files, and includes will be in `/usr/local/share/gle-graphics` and documentation in `/usr/local/share/doc/gle-graphics`.  This can be altered with `CMAKE_INSTALL_PREFIX` and `DEVELOPER_INSTALLATION` options (see below).


### Building on Windows

Building on windows requires [Visual Studio](https://visualstudio.microsoft.com/), [cmake](https://cmake.org/), and [vcpkg](https://vcpkg.io/) to be installed.  

1. Install the ECM package
	```
	vcpkg install ecm
	```

2. Install ghostpdl library

	Visit [ghostpdl GitHub page](https://github.com/ArtifexSoftware/ghostpdl) and clone the repo or download and unpack the source code.  Only the ghostpdl headers are needed to build GLE.  So building ghostpdl is not required.

3. Run cmake in the gle directory

	```
	cmake -B build -S src
    -DCMAKE_TOOLCHAIN_FILE=VCPKGROOT/scripts/buildsystems/vcpkg.cmake
        -DVCPKG_TARGET_TRIPLET=x64-windows-release
        -DGHOSTPDL_ROOT=/path/to/ghostpdl
        -DGLE_EXAMPLES_LIBRARY_PATH="/path/to/gle-library"
        -DGLE_USER_MANUAL_PATH="/path/to/gle-manual"
        -DECM_DIR="VCPKGROOT/installed/x64-windows/share/ECM"
	```

	Replace VCPKGROOT with the location of vcpkg. This will run vcpkg and download and build the needed libraries.  Be patient it will take some time.

4. Build 

	```
	cmake --build build --config Release
	```

5. Install gle in `C:\Program Files\gle-graphics`

	```
	cmake --install build --config Release
	```

All gle files will be installed to `C:\Program Files\gle-graphics` by default.  Add `C:\Program Files\gle-graphics\bin` to your PATH environment variable.  The installation location can be changed by setting `CMAKE_INSTALL_PREFEX` on the initial cmake call.

### Building on macOS

Building on macOS requires [XCode](https://developer.apple.com/xcode/), [cmake](https://cmake.org/), and [Homebrew](https://brew.sh/) to be installed.

1. Install the needed packages
	```
	brew install --quiet boost cairo ghostscript jpeg-turbo 
    libdeflate libpng libtiff pixman qt zstd poppler glib extra-cmake-modules
	```

2. Run cmake in the gle directory

	```
	cmake -S src -B build
        -DGLE_EXAMPLES_LIBRARY_PATH="/path/to/gle-library"
        -DGLE_USER_MANUAL_PATH="path/to/gle-manual"
	```

3. Build 

	```
	cmake --build build --config Release
	```

4. Install gle in `/usr/local/bin`

	```
	cmake --install build --config Release
	```

All gle files will be installed similar to Linux. The installation location can be changed by setting `CMAKE_INSTALL_PREFEX` on the initial cmake call.

### Post Installation Configuration and Testing

#### Install Needed Software

To run GLE these software packages required:

1. LaTeX - A powerful document preparation system widely used for technical and scientific writing. 

	* Windows: visit [MikTeX](https://miktex.org). Download and install the latest version.
	* Linux: `sudo apt install texlive texlive-latex-extra texlive-science dvipng latexmk`.
	* macOS: `brew install texlive`

2. GhostScript - PS/PDF interpreter.
	* Windows: Visit [Ghostscript](https://www.ghostscript.com/). Download and install the latest version.
	* Linux: `sudo apt install ghostscript`.
	* macOS: `brew install ghostscript`

#### Finding Dependencies

After installation run:

	gle -finddeps

to have GLE search for it dependency files such as GhostScript and LaTeX.

To test the installation run:

	gle -info

the output should look something like this on Windows:

	GLE version:             4.3.9
	Build date:              Nov 5 2025 14:44:22
	GLE_TOP:                 C:\Program Files\gle-graphics
	GLE_BIN_DIR:             C:\Program Files\gle-graphics\bin
	GLE_USRLIB:              
	GLERC (global):          C:\Program Files\gle-graphics\glerc
	GLERC (user):            C:\Users\<username>\AppData\Roaming\gle-graphics\.glerc
	inittex.ini:             C:\Program Files\gle-graphics\inittex.ini
	GhostScript:             C:\Program Files\gs\gs10.06.0\bin\gswin64c.exe
	GS library:              C:\Program Files\gs\gs10.06.0\bin\gsdll64.dll
	Bitmap import:           JPEG, PNG, TIFF, GIF
	Cairo rendering support: Yes
	Poppler PDF support:     Yes

and this on Linux:

	GLE version:             4.3.9
	Build date:              Nov 5 2025 19:28:28
	GLE_TOP:                 /usr/local/share/gle-graphics
	GLE_BIN_DIR:             /usr/local/bin
	GLE_USRLIB:
	GLERC (global):          /usr/local/share/gle-graphics/glerc
	GLERC (user):            /home/<username>/.glerc
	inittex.ini:             /usr/local/share/gle-graphics/inittex.ini
	GhostScript:             /usr/bin/gs
	GS library:              /lib/x86_64-linux-gnu/libgs.so
	Bitmap import:           JPEG, PNG, TIFF, GIF
	Cairo rendering support: Yes
	Poppler PDF support:     Yes

#### Optional set GLE_USRLIB and GLE_TOP

GLE will search the path pointed to by environment variable `GLE_USRLIB` for include files.  Set it to a location where you store your include files.  GLE also searches for include files in the current script directory, GLE_BIN_DIR, and GLE_TOP/gleinc by default.  GLE_USRLIB will be searched after these locations.

GLE automatically searches and finds `GLE_TOP` when it starts but setting it as an environment variable can be helpful. 

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
| `BUILD_GLEBTOOL`           | Build `glebtool` program (deprecated).                                                               | OFF                               |
| `BUILD_TEST    `           | Build testing programs.                                                                              | OFF                               |
| `INSTALL_FBUILD`           | Install the `fbuild` program – only needed during GLE build phase.                                   | OFF                               |
| `INSTALL_MAKEFMT`          | Install the `makefmt` program – only needed during GLE build phase.                                  | OFF                               |
| `DEVELOPER_INSTALLATION`   | Linux only: Install all files in staging area; otherwise install in FHS paths on system.             | OFF                               |

### Creating packages with cpack

Cpack can be utilized to create distributable packages.  The gle-manual and gle-library repos should be checked out and built to get included in the package.  Windows platform utilizes [NSIS](https://nsis.sourceforge.io/) for self installing exe building.  Debain packages can be created and require `dpkg-dev` to be installed on the local machine.


	windows:

	cd build && cpack -G "NSIS;ZIP;7Z"

	linux

	cd build && cpack -G "DEB;ZIP;7Z"

	macOS

	cd build && cpack -G "DragNDrop;ZIP;7Z"


### CI/CD Github Actions

Several GitHib actions exist that build the binaries and distributable packages for all three platforms.  The build artifacts can be found under the Actions tab and then navigate to the appropriate action.

The action "Create Release"  will create a release with a tag from the version number contained in CMakeLists.txt file.  It also will trigger the build and package actions for all three operating systems.  After running this action it is important to bump the version number in CMakeLists.txt file.

