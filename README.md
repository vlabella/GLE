# GLE
GLE - Graphics Layout Engine

GLE (Graphics Layout Engine) is a graphics scripting language designed for creating publication quality graphs, plots, diagrams, figures and slides. GLE supports various graph types (function plots, histograms, bar graphs, scatter plots, contour lines, color maps, surface plots, ...) through a simple but flexible set of graphing commands. More complex output can be created by relying on GLE's scripting language, which is full featured with subroutines, variables, and logic control. GLE relies on LaTeX for text output and supports mathematical formulea in graphs and figures. GLE's output formats include EPS, PS, PDF, JPEG, and PNG.

This repo contains the source code to build the executables for the GLE package.  The samples and example code as well as the manual will be contained in another repo (soon).

## Building with CMAKE

GLE can be build on windows, Mac and Linux using cmake.

libraries needed to build GLE are

* boost
* libtiff
* libpng
* zlib
* pixman
* cairo
* jpeg
* Qt5 (for GUI)

cmake uses find_package or find_library to resolve the paths for these libraries.  These variables must be set in your environment or passed to cmake

* ZLIB_ROOT
* JPEG_ROOT
* TIFF_ROOT
* PNG_ROOT
* CAIRO_ROOT
* PIXMAN_ROOT
* GHOSTPDL_ROOT
* Qt5_DIR
