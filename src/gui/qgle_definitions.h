/***********************************************************************************
 * QGLE - A Graphical Interface to GLE                                             *
 * Copyright (C) 2006  A. S. Budden & J. Struyf                                    *
 *                                                                                 *
 * This program is free software; you can redistribute it and/or                   *
 * modify it under the terms of the GNU General Public License                     *
 * as published by the Free Software Foundation; either version 2                  *
 * of the License, or (at your option) any later version.                          *
 *                                                                                 *
 * This program is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   *
 * GNU General Public License for more details.                                    *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License               *
 * along with this program; if not, write to the Free Software                     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. *
 *                                                                                 *
 * Also add information on how to contact you by electronic and paper mail.        *
 ***********************************************************************************/

#ifndef _QGLE_DEF_H
#define _QGLE_DEF_H

// The version number and application name
#define APP_NAME tr("QGLE")

// Some application defaults/defines
#define DEFAULT_PORT 6667
#define CM_PER_INCH  2.54
//
// This is the offset (in inches) that is added around the border
// of the drawing by GLE
#define GS_OFFSET (1.0/72.0)

// The maximum allowable distance between a click point and an object
// to be selected
#define MAX_SELECT_DISTANCE 8.0

// The length of the AMove cross
#define AMOVE_LENGTH 8

// The step to zoom in by
#define ZOOM_STEP 1.5

// The maximum number of decimal places in GLE coordinates
#define GLE_NUMBER_MAX_DP 6

// The default size for new diagrams
#define DEFAULT_NEW_SIZE 12.0

// The maximum distance at which to draw OSNAP handles
#define MAX_OSNAP_DRAW_DISTANCE 20.0

// The maximum distance at which to set the current position to be
// the osnap
#define MAX_OSNAP_DISTANCE 5.0

// The maximum distance at which to keep snap lines
#define MAX_SNAP_LINE_DISTANCE 10.0

// Half the size of the OSNAP box
#define OSNAP_BOX_SIZE 3

// Prevents unused argument warnings
#define UNUSED_ARG(x) (void) x

// The size of the handles used to resize objects
#define HANDLE_SIZE 3.0

// The minimum on-screen circle radius
#define MINIMUM_CIRCLE_RADIUS 1e-6

// The maximum number of iterations to use when finding
// elliptical perpendicular points
#define MAX_ELLIPSE_PERP_ITERATIONS 15

// The target accuracy (in radians) for elliptical perpendicular
// points
#define ELLIPSE_PERP_ACCURACY 1e-3

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif
