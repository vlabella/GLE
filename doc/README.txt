GLE Readme
==========

This document contains instructions on how to compile, install, and run GLE4.

Supported platforms: Windows, Linux, Mac OS/X, Unix, OS/2.

Contents
--------

1. What is GLE?
2. Installing a GLE binary
2.1 Installing GLE on Windows
2.2 Installing GLE on OS/2
3. Running GLE
4. Compiling GLE from the source code
4.1 Compiling with GCC (Windows, Linux, Mac OS/X, ...)
4.2 Compiling on Windows (with Microsoft Visual C++)
4.3 Compiling on OS/2 (with GCC 3.2.1)
4.4 Notes for all platforms
5. Source code organization
6. License (BSD)

1. What is GLE?
---------------

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulea in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

More information is available at <http://www.gle-graphics.org/>.

2. Installing a GLE Binary
--------------------------

GLE is available as binary installation packages (one for each supported platform) or as source code. The source code must be compiled before it can be installed. To compile GLE from its source code on different platforms, see Section 4. This section describes how a GLE binary can be installed.

2.1 Installing GLE on Windows
-----------------------------

The easiest way to install GLE on Windows is to download the self installing executable. The self installing executable is named GLE-<version>-exe-win32.exe. By double-clicking this file in Windows Explorer, it will guide you through the installation process.

An installation guide illustrating this process (with screenshots) is available here:
<http://www.gle-graphics.org/tut/windows.html>

During installation, GLE will search for installed programs, such as GhostScript and LaTeX. GLE does not require these for its most basic operation, but it does so for more advanced features, such as producing PDF output and processing scripts including LaTeX commands. This search process runs in a black window (Command Prompt) and the output should be similar to the following:

Running GLE -finddeps "C:\Program Files" (locate GLE fonts and optionally GhostScript/LaTeX): ................................................................................
................................................................................
................................................................................
Found: GLE 4.1.0 in C:\Program Files\Gle4\4.1.0\bin\gle.exe
Found: GLE 4.0.12 in C:\Program Files\Gle4\4.0.12\bin\gle.exe (*)
Found: latex.exe in '?'
Found: pdflatex.exe in '?'
Found: dvips.exe in '?'
Found: gswin32c.exe in 'C:\Program Files\Tools\gs8.53\gs8.53\bin\gswin32c.exe'
Press enter to continue ...

In this example, GLE has detected that GhostScript is installed on my computer, but it did not detect LaTeX. More information on using GLE with LaTeX is given below. For now, just focus the window saying "Press enter to contunue..." and press enter. Next click Finish on the installer screen.

GLE should now have added a new entry titled GLE to your start menu. Clicking it pops up a menu showing the options "Command Prompt", "Readme", "Uninstall", and "Website". Click on "Command Prompt" to open a command prompt. Now type the command "gle" and press enter.

C:\Program Files\Gle4\4.1.0\samples> gle
GLE version 4.1.0
Usage: gle [options] filename.gle
More information: gle /help

GLE should respond as in the example above. If you instead get the message "'gle' is not recognized as an internal or external command, operable program or batch file", then the folder containing "gle.exe" was not correctly added to your system's search path during installation.

If the version number printed by GLE is not correct, then a previous version of GLE may be installed on your computer. Either uninstall that version or remove it from your system's search path (you will still be able to run it through the -v option, as discussed in the reference manual).

I will now show you how to change the search path. This is a list of folders that Windows searches for executable programs when you try to run a particular program from the command prompt. Note that normally the GLE installer should have done this for you. Only perform the next step if you saw an error message or incorrect GLE version in the previous step.

Select "Control Panel" from the start menu and click "System". Navigate to the "Advanced" tab and click the "Environment Variables" button. Select the variable "PATH" and click the "Edit" button. Add to the "Value" field the location of the GLE executable "gle.exe". This location is the installation folder you specified during installation with "\bin" added at the end, for example, C:\Program Files\Gle4\4.1.0\bin. Note that different folders in the PATH are separated with the symbol ";". You might need to change the value of PATH in either the "User" or the "System" variables.

If you have LaTeX installed on your computer, you also need to perform the next step. LaTeX may not be installed in "C:\Program Files", and then GLE does not detect it automatically during installation. Open the command prompt and run "gle -finddeps LATEXPATH", with LATEXPATH the location of your LaTeX installation. The following example assumes that the MiKTeX distribution is installed (http://www.miktex.org/).

C:>gle -finddeps "C:\MiKTeX"
Finding dependencies in: C:\MiKTeX: ...........................
Found: gle.exe in 'C:\Program Files\gle4\bin'
Found: gswin32c.exe in 'C:\Program Files\Tools\gs\gs\gs8.00\bin'
Found: latex.exe in 'C:\MiKTeX\Main\miktex\bin'
Found: dvips.exe in 'C:\MiKTeX\Main\miktex\bin'

Note that both GhostView and LaTeX are optional. For creating simple EPS graphs, they are not required, i.e., GLE will still work if you see question marks for gswin32c.exe, latex.exe and dvips.exe.

You are all set. Information on how to run GLE is available in Section 3 and in the manual.

2.2 Installing GLE on OS/2
--------------------------

Since you are reading this you downloaded the do it yourself .tar.gz package for GLE installation. To install this package:

1) Unzipping gle-graphics-x.y.z-src.tar.gz will create the following files.

gle\
gle\doc\readme           (you are reading this)
gle\doc\gle-manual.pdf
gle\gle.exe
gle\gcc321m.dll
gle\fbuild.exe
gle\makefmt.exe
gle\inittex.ini
gle\glerc
gle\font\*

2) Put gle.exe somewhere into your PATH and *.dll into your LIBPATH.

3) Add "SET GLE_TOP=x:/path/gle" to your config.sys.

3. Running GLE
--------------

Running GLE is similar on all supported platforms.

Note that GLE is a command line application. It takes as input a GLE script file and outputs an EPS, PS, PDF, JPG, or PNG file. GLE cannot be run from inside a file manager such as Windows Explorer. Instead it should be started from a terminal: an X-terminal or console application in Linux or the command prompt in Windows.

To start GLE, open a terminal/command prompt and issue the command:

gle -d outputtype yourfile.gle

where outputtype is one of: eps, teps, ps, pdf, jpg, png

and yourfile.gle is the file containing the your GLE script.

E.g. (Windows):

---------------------------------------------------------

C:\Program Files\Gle4\samples>gle -d eps sample.gle
GLE 4.1.0 [sample.gle]-C-R-[sample.eps]

---------------------------------------------------------

Preview the output file sample.eps with GhostView (e.g., by browsing with your favorite file manager to in this case C:\Program Files\Gle4\samples and clicking sample.eps).

See the GLE manual for more information.
<http://glx.sourceforge.net/>

Or contact the GLE mailing list (glx-general) if you have any questions.
<https://lists.sourceforge.net/lists/listinfo/glx-general>

There is also an installation guide / introductory tutorial available for Windows here:
<http://www.gle-graphics.org/tut/windows.html>

and for Linux here:
<http://www.gle-graphics.org/tut/linux.html>

4. Compiling GLE from the source code
-------------------------------------

The GLE source code is distributed as gle-graphics-x.y.z-src.tar.gz.

4.1 Compiling with GCC (Windows, Linux, Mac OS/X, ...)
------------------------------------------------------

To install GLE system-wide:

	./configure
	make
	make doc
	su
	make install

To install GLE in your home directory:

	./configure --prefix=$HOME/apps
	make
	make doc
	make install

A detailed description of the above process can be found here:
<http://www.gle-graphics.org/tut/gcc.html>

The "make doc" builds the GLE manual in PDF format. This step is optional and requires that you have "pdflatex" installed.

4.1.1 Installing the additional fonts from "gle-graphics-extrafonts-1.0.tar.gz"
-------------------------------------------------------------------------------

GLE has a set of additional fonts that are distributed separately. To include these in your GLE installation, you have to download them and extract them in the gle-graphics-x.y.z directory before compiling GLE. You also have to include the option "--with-extrafonts=yes" while configuring. For example,

$ cd gle-graphics-x.y.z
$ tar -xvzf $HOME/gle-graphics-extrafonts-1.0.tar.gz
$ ./configure --with-extrafonts=yes
$ make clean
$ make
$ make doc
$ make install

These instructions assume that you downloaded gle-graphics-extrafonts-1.0.tar.gz and saved it into your home directory.

4.2 Compiling on Windows (with Microsoft Visual C++)
----------------------------------------------------

The compilation process uses "Makefiles". Visual Studio project files are not supported. Makefiles and config files ending in .vc are for the Microsoft Visual C++ compiler.

To compile GLE with support for including bitmap files
	libtiff <www.libtiff.org> and
	libpng <www.libpng.org>

are required, and
	HAVE_LIBTIFF = 1
	HAVE_LIBPNG = 1

must be enabled in config.i (to disable bitmap support, comment these out with '#')

	you must have the environment variable GLE_TOP point to the location
	of C:\Path\To\gle4\bin

	you must have the environment variable LIBTIFFDIR point to the location
	of the libtiff.h and .lib files

	you must have the environment variable LIBPNGDIR point to the location
	of the png.h and .lib files

	nmake -f Makefile.vc

4.3 Compiling on OS/2 (with gcc 3.2.1)
--------------------------------------

	change the values of
		INSTALL_DIR     (D:/UTILS/gle)
		INSTALL_BIN     (D:/UTILS/bin)
		ADD_DLLS	(D:/GCC.321/dll/gcc321m.dll)
		ADD_DOCS	(readme ../gle-refman/gle-manual.pdf)
                PROJECT_DIR     (E:/Works/Projects/CGLE/current/gle4)
        in config.os2

        make -f Makefile.os2

4.4 Notes for all platforms
---------------------------

Note: If you get the following error:
      StringKeyHash.h:8: ext/hash_map: No such file or directory
      Then add -DGCC2 to EXTRA_DEFS in config.{gcc,os2,nocygwin}

      (Or upgrade to a newer version of gcc.)

The compilation process will built:

gle(.exe) : GLE main executable
fbuild(.exe) : makes *.fve fron *.gle using fbuild in the src\font directory
makefmt(.exe) : makes font files *.fmt from *.afm in the src\font directory

It will also run the fbuild and makefmt commands and put the
font files in the build/font directory along with the font.dat file.

It will also run your newly built gle on the files
in the src\samples subdirectory.

The clean target is also defined on all makefiles.

5. Source code organization
---------------------------

The source code is contained in the src sub directory, where each module has its
own directory (and makefiles). Binaries are put in the build/bin sub directory.

6. License
----------

See LICENSE.txt.
