#
#  QGLE - A Graphical Interface to GLE
#  Copyright (C) 2006  A. S. Budden & J. Struyf
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#  Also add information on how to contact you by electronic and paper mail.
#

Introduction
------------

The current GUI contains a preview window that can receive messages from 
GLE and display the resulting EPS file.  In addition, it can open GLE 
and EPS files directly (using Ghostscript and GLE).  It now has the 
capability to add and edit simple objects, such as, lines, circles and 
arcs (snapping to a grid if required). It can also change various properties
of the objects, such as, line width, and color. The perpendicular 
line and tangential line commands can be used to produce a line starting
perpendicular or tangential to an existing object.  OSnap can be used 
for the end point of a line.

When drawing objects, hit the escape key to cancel.  The pointer tool 
can be used to select objects and they can then be deleted using the 'del'
key or moved/scaled.  Other keyboard shortcuts include:

F1 : Show this message
F3 : Toggle Object Snap
F6 : Toggle coordinate display (Cartesian, Polar, 
                                Relative Cartesian, Relative Polar,
                                Off)
F7 : Toggle grid visibility
F8 : Toggle Orthographic snap
F9 : Toggle grid snap
F10: Toggle polar snap

'-': Zoom in
'=': Zoom out

Shift: Hold this down to select multiple objects for deletion

At present, there is very little error checking, so crashes are possible.
Also, if compiled in debug mode, there's a fair amount of debugging output 
sent to the console, but I'm gradually cutting this down as it gets more
stable.

Once compiled, run "qgle" to start.  This will bring up the main window
with a status bar message saying "Server Listening".

Once "gle -p" has been run, the eps should appear in the window and the
full file name in the status bar.

BUGS:

* Temporary files (used for rendering GLE code) are never deleted;
* Icons on the toolbar are horrible;
* Very little error checking in the code, therefore very likely to crash 
  or just do something very strange;
* I'm sure there are a lot more bugs, but I haven't found them all yet 
  and haven't written down those that I have found: answers on a 
  postcard to the usual address.

TODO:

* Fix all the bugs!
* Line trimming and extension;
* Allow drawing of boxes etc;
* Change circles and arcs to be specific cases of ellipses and
  elliptical arcs;
* Add object ordering;
* Extend polar snap to work from other snap points;
* Add "object tracking";
* Add exporters: PDF etc;
* Add printing;
* Make lines into polylines and allow object conversion;
* Add custom objects built from subroutines;
* Allow addition of arbitrary gle code at a given position?

Compiling the GUI
-----------------

The program is designed for QT4 (which is fairly different to QT3), so 
make sure you have the right version operable (they can coexist).

Then configure and build GLE as follows:

./configure --with-qt=/path/to/your/qt4/directory
make
make install
qgle

Note that more information on comiling GLE can be found here:
<http://www.gle-graphics.org/tut/linux.html>

On Arch Linux, the QT4 package lives in /opt/qt4 whereas QT3 lives in
/opt/qt.  There is a script called qt-config (not to be confused with
qtconfig), which switches the development environment between the two.

To compile in debug mode in Windows (selectable using the config option 
in qgle.pro), you MUST have compiled the debug libraries (using the 
option on the "Start" menu).  If you haven't done this, you'll probably 
get some random unexplained crashes.

Al <abudden@NOSPAMgataki.co.uk>

# vim:tw=72:fo+=w2:et
