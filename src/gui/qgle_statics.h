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

#ifndef __QGLE_STATICS_H
#define __QGLE_STATICS_H

#include <QtWidgets>
#include "../gle/gle-interface/gle-interface.h"

//! Class providing static helper functions for the main code
class QGLE
{
public:
	//! Flush the IO so that debug messages appear immediately
	static void flushIO();

	//! The list of commands that can be used
	enum Commands
	{
		// For drawing a new line:
		StartNewLineWithPoint,
		SetLineEndPoint,
		EndNewLine,

		// For drawing a new Tangential Line:
		StartNewTanLineWithPoint,
		SetTanLineEndPoint,
		EndNewTanLine,

		// For drawing a new Perpendicular Line:
		StartNewPerpLineWithPoint,
		SetPerpLineEndPoint,
		EndNewPerpLine,

		// For drawing a new circle
		StartNewCircleWithCentre,
		SetCircleEdgePoint,
		EndNewCircle,

		// For drawing a new ellipse
		StartNewEllipseWithCorner,
		SetEllipseSecondCorner,
		EndNewEllipse,

		// For drawing a new arc
		StartNewArcWithPoint,
		SetArcEndPoint,
		SetArcMidPoint,
		EndNewArc,

		// Create a new text object
		CreateTextObject,

		// User hit the escape key
		Cancel,

		// User pressed delete
		DeleteSelected,

		// For setting the amove
		SetAMove

	};


	//! Print a date in a pretty format
	/*!
	 * This only supports English dates at present: this is because
	 * the readme.txt file is only likely to be written in English
	 * at the moment.  Once internationalisation is done and the
	 * readme.txt about box is replaced with a help system based on
	 * QAssistant, this function can be replaced with one that uses the
	 * current locale.  This uses the long name for the month so as to
	 * avoid ambiguity between dd/mm/yyyy and mm/dd/yyyy.  Its argument
	 * is the date in dd/mm/yyyy format (with leading zeros).
	 */
	static QString prettyDate(QString datestr);

	//! Return the sign of a number
	static int sign(double n);
	//! Return the sign of a number
	static int sign(int n);

	//! Return the quadrant that an angle is in
	static int quadrant(double angle);

	//! Get the filename of an executable
	static QString GetExeName();
	//! Get the directory of a file
	static QString GetDirName(QString fname);

	//! Convert GLE to Qt String
	static QString gleToQString(GLEString* str);
	//! Convert STL to Qt String
	static inline QString stlToQString(const string& str) { return QString::fromUtf8(str.c_str(), str.length()); };
	//! Convert Qt to GLE String
	static void qtToGLEString(const QString& str1, GLEString* str2);

	//! Static member used for converting between coordinate system
	static QPointF absGLEToQt(double gleX, double gleY, double dpi, int areaHeight);
	//! Static member used for converting between coordinate systems
	static QSizeF qtSizeToGLE(QSizeF qt, double dpi);
	//! Static member used for converting between coordinate system
	static QPointF absGLEToQt(QPointF gle, double dpi, int areaHeight);
	static QPointF absGLEToQt(const GLEPoint& gle, double dpi, int areaHeight);
	//! Static member used for converting between coordinate system
	static QPointF relGLEToQt(double gleX, double gleY, double dpi);
	//! Static member used for converting between coordinate system
	static QPointF relGLEToQt(QPointF gle, double dpi);
	//! Static member used for converting between coordinate system
	static QSizeF relGLEToQt(QSizeF gle, double dpi);
	//! Static member used for converting individual relative numbers
	static double relGLEToQt(double gle, double dpi);
	//! Static member used for converting between coordinate system
	static QPointF absQtToGLE(QPointF qt, double dpi, int areaHeight);
	//! Static member used for converting between coordinate system
	static QPointF absQtToGLE(double qtX, double qtY, double dpi, int areaHeight);
	//! Static member used for converting between coordinate system
	static QPointF relQtToGLE(double qtX, double qtY, double dpi);
	//! Static member used for converting between coordinate system
	static QPointF relQtToGLE(QPointF qt, double dpi);
	//! Static member used for converting between coordinate system
	static QSizeF relQtToGLE(QSizeF qt, double dpi);
	//! Static member used for converting individual relative numbers
	static double relQtToGLE(double qt, double dpi);
	//! Static member used for type converting Qt points to GLEPoints
	static GLEPoint QPointFToGLEPoint(const QPointF& pt);
	//! Static member used for type converting GLEPoints to Qt points
	static QPointF GLEPointToQPointF(const GLEPoint& pt);
	//! Static member used for converting from GLE coordinates to a GLE coordinate string
	static QString GLEToStr(QPointF gle);
	//! Static member used for converting from GLE coordinates to a GLE coordinate string
	static QString GLEToStr(double gleX, double gleY);
	//! Static member used for converting from GLE distance to a GLE string
	static QString GLEToStr(double gle);
	//! Static member used for cropping extra zeros from a QString
	static QString ZeroCrop(QString str);

	//! Static member used for rotating coordinates
	static QPointF parkTransform(QPointF original, double angle);

	//! Static member used for rotating coordinates about a point
	static QPointF rotateAboutPoint(QPointF original, double angle, QPointF basePoint);

	//! Static member used for conversion between radians and degrees
	static double radiansToDegrees(double rads);
	//! Static member used for conversion between radians and degrees
	static double degreesToRadians(double degrees);

	//! Static member used to compute the minimum of two double values
	static double min(double a, double b);
	//! Static member converting from centimeters to PostScript points
	static double cmToPt(double cm);
	//! Static member used to compute the DPI value based on the display and image size (in cm)
	static int computeAutoScaleDPIFromCm(const QSize& bitmapSize, int inset, double imgWd, double imgHi);
	//! Static member used to compute the DPI value based on the display and image size (in pt)
	static int computeAutoScaleDPIFromPts(const QSize& bitmapSize, int inset, double bbWd, double bbHi);

	//! Static member used for finding the distance between two points
	static double distance(QPointF one, QPointF two);
	//! Static member used for finding the distance between two points
	static double distance(double x, double y, QPointF two);

	//! Static member used for finding the angle between two points
	static double angleBetweenTwoPoints(QPointF one, QPointF two);

	//! Static member used to give OS dependent file regular expression
	static QRegExp fileRegExp();

	//! Static member providing the GS lib file name
	static QString gsLibFileName();

	//! Static member providing the GS lib filter
	static QString libraryFilter();

	//! Static member providing the GLE file name
	static QString gleExecutableName();

	//! Static member providing a filter for executables
	static QString executableFilter();

	//! Static member to enable a button if it is not null
	static void setEnabled(QPushButton* button, bool enable);

	//! Static member to draw a box around a point
	static void drawBox(QPainter *p, QPointF origin, double half_size, QPen pen);

	//! Static member to fill a box
	static void fillBox(QPainter *p, QPointF origin, int half_size, const QBrush& brush);

	//! Static member to draw a rotating handle around a point
	static void drawRotator(QPainter *p, QPointF origin, int corner, double half_size, QPen pen);

	//! Static member to draw a line with arrows at the ends
	static void drawArrowLine(QPainter *p, qreal x0, qreal y0, qreal x1, qreal y1, qreal size, qreal angle, bool a0, bool a1, const QPen& pen);

	//! Static member to draw a scale handle around a point
	static void drawScaleHandle(QPainter *p, QPointF origin, int dx, int dy, double half_size, QPen pen);

	//! Static member to draw a cross at a point
	static void drawCross(QPainter *p, QPointF origin, double half_size, QPen pen);

	//! Static member to draw the perpendicular snap mark
	static void drawPerpMark(QPainter *p, QPointF origin, QPen pen);

	//! Static member to draw the tangent snap mark
	static void drawTangentMark(QPainter *p, QPointF origin, QPen pen);

	//! Static member to draw an intersection mark
	static void drawIntersectMark(QPainter *p, QPointF origin, QPen pen);

	//! Static member to draw OSnaps
	static void drawOSnap(QPainter *p, QPair<QPointF, int> snap);

	//! Static member to draw OSnaps
	static void drawOSnap(QPainter *p, QPointF snap, int type);

	//! Static member to decide whether numbers are in order (either way round)
	static bool inOrder(double x, double y, double z);

	//! Static member to convert from Qt Colours to GLE Colours
	static QString colourQtToGLE(QColor colour);

	static QString getFileNameExtension(const QString& name);

	static bool isGraphicsExtension(const QString& name);

	static QString addFileNameExtension(const QString& name, const char* ext);

	static void ensureFileNameExtension(QString* str, const char* ext);

	static QPen osnapLinePen();
	static QPen otrackLinePen();
	static QPen osnapPen();
	static QPen handlePen();

	enum SnapTypes
	{
		EndPointSnap,
		MidPointSnap,
		QuadrantSnap,
		CentreSnap,
		TangentSnap,
		IntersectSnap,
		PerpendicularSnap,
		NearestPointSnap
	};

	enum ScaleMethods
	{
		ProportionalScale,
		HorizontalScale,
		VerticalScale
	};

	static QPointF calculateScaleRatios(QPointF basePoint, QPointF startPoint, QPointF endPoint, int method);

};

#endif
