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

#include <QtGui>
#include "qgle_statics.h"
#include "qgle_definitions.h"
#include "drawingobject.h"

#include <math.h>
#ifdef Q_OS_WIN32
	#include <windows.h>
#endif

/*******************************************
 * Static members used in a lot of classes *
 *******************************************/

void QGLE::flushIO()
{
	// Flush the IO to make debug messages appear quickly
	QTextStream out(stdout);
	out.flush();
	QTextStream err(stderr);
	err.flush();
}

QString QGLE::prettyDate(QString datestr)
{
	// Create a QDate object based on the
	// provided date
	QDate dt = QDate::fromString(datestr, "dd/MM/yyyy");
	QString prettyString = dt.toString("d MMMM yyyy");
	QRegExp rx("^(\\d?)(\\d)(?= )");
	QString suffix;
	if(rx.indexIn(prettyString) > -1)
	{
		if (rx.capturedTexts()[1].toInt() == 1)
			suffix = "th";
		else
			switch(rx.capturedTexts()[2].toInt())
			{
				case 1:
					suffix = "st";
					break;
				case 2:
					suffix = "nd";
					break;
				case 3:
					suffix = "rd";
					break;
				default:
					suffix = "th";
			}
		prettyString.replace(rx, rx.capturedTexts()[0] + suffix);
	}
	return(prettyString);

}

int QGLE::sign(double n)
{
	if (n > 0.0)
		return(1);
	else if (n < 0.0)
		return(-1);
	else
		return(0);
}

int QGLE::sign(int n)
{
	if (n > 0)
		return(1);
	else if (n < 0)
		return(-1);
	else
		return(0);
}

int QGLE::quadrant(double angle)
{
	double degrees = radiansToDegrees(angle);
	int quadrant;

	// Make the number positive
	while(degrees < 0.0)
		degrees += 360.0;

	// Make the number less than 360
	while(degrees > 360.0)
		degrees -= 360.0;

	if (degrees > 270.0)
		quadrant = 4;
	else if (degrees > 180.0)
		quadrant = 3;
	else if (degrees > 90.0)
		quadrant = 2;
	else
		quadrant = 1;

	return(quadrant);
}

QString QGLE::gleToQString(GLEString* str) {
	QString res;
	int len = (int)str->length();
	res.resize(len);
	for (int i = 0; i < len; i++) {
		res[i] = QChar(str->get(i));
	}
	return res;
}

void QGLE::qtToGLEString(const QString& str1, GLEString* str2) {
	unsigned int size = str1.size();
	str2->setSize(size);
	for (unsigned int i = 0; i < size; i++) {
		str2->set(i, str1[i].unicode());
	}
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::absGLEToQt(double gleX, double gleY, double dpi, int areaHeight)
{
	double qtX, qtY;

	QPointF relative = relGLEToQt(gleX,gleY,dpi);
	qtX = relative.x() + (GS_OFFSET*dpi);
	qtY = areaHeight - (relative.y() + (GS_OFFSET*dpi));

	return(QPointF(qtX, qtY));
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::absGLEToQt(QPointF gle, double dpi, int areaHeight)
{
	return(absGLEToQt(gle.x(), gle.y(), dpi, areaHeight));
}

QPointF QGLE::absGLEToQt(const GLEPoint& gle, double dpi, int areaHeight)
{
	return(absGLEToQt(gle.getX(), gle.getY(), dpi, areaHeight));
}

QSizeF QGLE::qtSizeToGLE(QSizeF qt, double dpi)
{
	// DO NOT RELY ON THIS FOR ACCURACY!
	double gleX, gleY;
	qt.setWidth(qt.width() - 2*GS_OFFSET*dpi);
	qt.setHeight(qt.height() - 2*GS_OFFSET*dpi);
	QPointF rel = relQtToGLE(qt.width(), qt.height(), dpi);
	gleX = rel.x();
	gleY = rel.y();

	return(QSizeF(gleX,gleY));
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::relGLEToQt(double gleX, double gleY, double dpi)
{
	double qtX, qtY;

	qtX = gleX*dpi/CM_PER_INCH;
	qtY = gleY*dpi/CM_PER_INCH;

	return(QPointF(qtX, qtY));
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::relGLEToQt(QPointF gle, double dpi)
{
	return(relGLEToQt(gle.x(), gle.y(), dpi));
}

// Static member used to convert betweeen coordinate systems
QSizeF QGLE::relGLEToQt(QSizeF gle, double dpi)
{
	QPointF qt;
	qt = relGLEToQt(gle.width(), gle.height(), dpi);
	return(QSizeF(qt.x(),qt.y()));
}

double QGLE::relGLEToQt(double gle, double dpi)
{
	return(gle*dpi/CM_PER_INCH);
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::relQtToGLE(double qtX, double qtY, double dpi)
{
	double gleX, gleY;

	gleX = qtX*CM_PER_INCH/dpi;
	gleY = qtY*CM_PER_INCH/dpi;

	return(QPointF(gleX, gleY));
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::relQtToGLE(QPointF qt, double dpi)
{
	return(relQtToGLE(qt.x(), qt.y(), dpi));
}

// Static member used to convert betweeen coordinate systems
QSizeF QGLE::relQtToGLE(QSizeF qt, double dpi)
{
	QPointF gle;
	gle = relQtToGLE(qt.width(), qt.height(), dpi);
	return(QSizeF(gle.x(),gle.y()));
}

double QGLE::relQtToGLE(double qt, double dpi)
{
	return(qt*CM_PER_INCH/dpi);
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::absQtToGLE(double qtX, double qtY, double dpi, int areaHeight)
{
	double gleX, gleY;

	gleX = ((qtX/dpi)-GS_OFFSET)*CM_PER_INCH;
	gleY = (((areaHeight - qtY)/dpi)-GS_OFFSET)*CM_PER_INCH;

	return(QPointF(gleX, gleY));
}

// Static member used to convert betweeen coordinate systems
QPointF QGLE::absQtToGLE(QPointF qt, double dpi, int areaHeight)
{
	return(absQtToGLE(qt.x(), qt.y(), dpi, areaHeight));
}

// Static member used for type converting Qt points to GLEPoints
GLEPoint QGLE::QPointFToGLEPoint(const QPointF& pt) {
	return GLEPoint(pt.x(), pt.y());
}

// Static member used for type converting GLEPoints to Qt points
QPointF QGLE::GLEPointToQPointF(const GLEPoint& pt) {
	return QPointF(pt.getX(), pt.getY());
}

//! Static member used for converting from GLE coordinates to a GLE coordinate string
QString QGLE::GLEToStr(QPointF gle)
{
	return(GLEToStr(gle.x(), gle.y()));
}
//! Static member used for converting from GLE coordinates to a GLE coordinate string
QString QGLE::GLEToStr(double gleX, double gleY)
{
	QString x, y;

	x.setNum(gleX, 'f', GLE_NUMBER_MAX_DP);
	y.setNum(gleY, 'f', GLE_NUMBER_MAX_DP);

	x = ZeroCrop(x);
	y = ZeroCrop(y);

	return(QString("%1 %2").arg(x).arg(y));
}

//! Static member used for converting from GLE distance to a GLE string
QString QGLE::GLEToStr(double gle)
{
	QString x;

	x.setNum(gle, 'f', GLE_NUMBER_MAX_DP);

	x = ZeroCrop(x);

	return(QString("%1").arg(x));
}

//! Static member used to crop unnecessary zeros from the end of a number string
QString QGLE::ZeroCrop(QString str)
{
	int last0;
	int i;

	last0 = str.size();
	for(i=str.size()-1;i>0;i--)
	{
		if (str.at(i) == '0')
		{
			last0 = i;
		}
		else if (str.at(i) == '.')
		{
			last0++;
			break;
		}
		else
		{
			break;
		}
	}
	if (last0 != str.size())
		str.chop(str.size()-last0);

	return(str);
}

//! Static member used for finding the distance between two points
double QGLE::distance(QPointF one, QPointF two)
{
	return(sqrt(pow(one.x()-two.x(),2) + pow(one.y()-two.y(),2)));
}

//! Static member used for finding the distance between two points
double QGLE::distance(double x, double y, QPointF two)
{
	double dx = x-two.x();
	double dy = y-two.y();
	return(sqrt(dx*dx + dy*dy));
}

double QGLE::angleBetweenTwoPoints(QPointF one, QPointF two)
{
	return(atan2(two.y()-one.y(),two.x()-one.x()));
}

double QGLE::radiansToDegrees(double rads)
{
	return(rads*180.0/M_PI);
}

double QGLE::degreesToRadians(double degrees)
{
	return(degrees*M_PI/180.0);
}

//! Static member used to compute the minimum of two double values
double QGLE::min(double a, double b) {
	return a < b ? a : b;
}

//! Static member comverting from centimeters to PostScript points
double QGLE::cmToPt(double cm) {
	return cm*PS_POINTS_PER_INCH/CM_PER_INCH;
}

//! Static member used to compute the DPI value based on the display and image size (in cm)
int QGLE::computeAutoScaleDPIFromCm(const QSize& bitmapSize, int inset, double imgWd, double imgHi) {
	// GLE always adds two points to the size of a drawing
	return computeAutoScaleDPIFromPts(bitmapSize, inset, cmToPt(imgWd)+2.0, cmToPt(imgHi)+2.0);
}

//! Static member used to compute the DPI value based on the display and image size (in pt)
int QGLE::computeAutoScaleDPIFromPts(const QSize& bitmapSize, int inset, double bbWd, double bbHi) {
	// GLE: BitmapWd = (int)floor((double)dpi/PS_POINTS_PER_INCH*BoundingBoxWD+1)
	//      BitmapWd + 0.1 = dpi/PS_POINTS_PER_INCH*BoundingBoxWD+1
	// -> Solve for dpi
	double dpi_wd = PS_POINTS_PER_INCH * ((double)bitmapSize.width()-inset-0.9) / bbWd;
	double dpi_hi = PS_POINTS_PER_INCH * ((double)bitmapSize.height()-inset-0.9) / bbHi;
	return (int)floor(min(dpi_wd, dpi_hi));
}

QString QGLE::GetExeName()
{
	QString result = QString::null;
#ifdef Q_OS_WIN32
	char name[1024];
	DWORD res = GetModuleFileNameA(NULL, name, 1023);
	if (res > 0)
	{
		name[res] = 0;
		result = name;
	}
#elif defined(Q_OS_HURD) || defined Q_OS_LINUX
	return(QFileInfo("/proc/self/exe").readLink());
#else
	return(QApplication::applicationFilePath());
#endif
	return result;
}

QString QGLE::GetDirName(QString fname)
{
	QString result = fname;
	result.replace('\\', '/');
	int i = result.lastIndexOf('/');
	if (i != -1)
	{
		result.truncate(i+1);
	}
	else
	{
		result = "";
	}
	return result;
	// // Could alternatively be achieved with
	// // something like:
	// QFileInfo fi(fname);
	// return(fi.absolutePath());
}

QRegExp QGLE::fileRegExp()
{
#ifdef Q_OS_WIN32
	QRegExp rx("(file:///)?(.*\\.(?:gle|eps))", Qt::CaseInsensitive);
#else
//#elif defined(Q_OS_HURD) || defined(Q_OS_LINUX)
	QRegExp rx("(file://)?(.*\\.(?:gle|eps))", Qt::CaseInsensitive);
#endif
	return(rx);
}

QString QGLE::gsLibFileName()
{
#ifdef Q_OS_WIN32
	#if _WIN32
		// VS builds
		#if _WIN64
			return(QObject::tr("gsdll64.dll"));
		#else
			return(QObject::tr("gsdll32.dll"));
		#endif
	#endif
	#if __GNUC__
		// gcc builds
		#if __x86_64__ || __ppc64__
			return(QObject::tr("gsdll64.dll"));
		#else
			return(QObject::tr("gsdll32.dll"));
		#endif
	#endif
#elif defined(Q_OS_HURD) || defined(Q_OS_LINUX)
	return(QObject::tr("libgs.so"));
#elif defined(Q_OS_MACOS)
	return(QObject::tr("libgs.dylib"));
#else
#error "What operating system are you using?"
#endif
}

QString QGLE::libraryFilter()
{
#ifdef Q_OS_WIN32
	return(QObject::tr("DLLs (*.dll)"));
#elif defined(Q_OS_HURD) || defined(Q_OS_LINUX)
	return(QObject::tr("Shared objects (*.so*)"));
#elif defined(Q_OS_MACOS)
	return(QObject::tr("Dynamic libraries (*.dylib);; Ghostscript framework (*)"));
#else
#error What operating system are you using?
#endif
}

QString QGLE::gleExecutableName()
{
#ifdef Q_OS_WIN32
	return(QString("gle.exe"));
#else
//#elif defined(Q_OS_LINUX)
	return(QString("gle"));
#endif
}

QString QGLE::executableFilter()
{
#ifdef Q_OS_WIN32
	return(QObject::tr("Executables (*.exe)"));
#else
//#elif defined(Q_OS_HURD) || defined(Q_OS_LINUX)
	return(QObject::tr("All files (*)"));
#endif
}

//! Static member to enable a button if it is not null
void QGLE::setEnabled(QPushButton* button, bool enable) {
	if (button != NULL) {
		button->setEnabled(enable);
	}
}

//! Static member to draw a box around a point
void QGLE::drawBox(QPainter *p, QPointF origin, double half_size, QPen pen)
{
	p->setPen(pen);
	p->drawLine(QLineF(origin.x() - half_size,
				origin.y() - half_size,
				origin.x() - half_size,
				origin.y() + half_size));
	p->drawLine(QLineF(origin.x() - half_size,
				origin.y() + half_size,
				origin.x() + half_size,
				origin.y() + half_size));
	p->drawLine(QLineF(origin.x() + half_size,
				origin.y() + half_size,
				origin.x() + half_size,
				origin.y() - half_size));
	p->drawLine(QLineF(origin.x() + half_size,
				origin.y() - half_size,
				origin.x() - half_size,
				origin.y() - half_size));
}

//! Static member to fill a box
void QGLE::fillBox(QPainter *p, QPointF origin, int half_size, const QBrush& brush)
{
	int x0 = (int)floor(origin.x()-half_size+0.5);
	int y0 = (int)floor(origin.y()-half_size+0.5);
	p->fillRect(x0, y0, 2*half_size, 2*half_size, brush);
}

//! Static member to draw a rotating handle around a point
void QGLE::drawRotator(QPainter *p, QPointF origin, int corner, double half_size, QPen pen)
{
	//double arrSize = 2.0;
//	pen.setWidthF(pen.widthF()*4.0);
	p->setPen(pen);
	p->save();
	p->translate(origin);

	// Work on the top right corner, but rotate according to which corner
	// it actually is:
	if (corner == GLEDrawingArea::TopLeft)
		p->rotate(-90.0);
	else if (corner == GLEDrawingArea::BottomLeft)
		p->rotate(180.0);
	else if (corner == GLEDrawingArea::BottomRight)
		p->rotate(90.0);

//	p->drawArc(QRectF(-half_size, half_size, 2*half_size, 2*half_size), 0, 16*90);
//	p->drawLine(QLineF(half_size, 0.0, half_size - arrSize, -arrSize));
//	p->drawLine(QLineF(half_size, 0.0, half_size + arrSize, -arrSize));
//	p->drawLine(QLineF(0.0, half_size, arrSize, half_size - arrSize));
//	p->drawLine(QLineF(0.0, half_size, arrSize, half_size + arrSize));
	p->drawEllipse(QRectF(-half_size, -half_size, 2*half_size, 2*half_size));

	p->restore();
}

//! Static member to draw a line with arrows at the ends
void QGLE::drawArrowLine(QPainter *p, qreal x0, qreal y0, qreal x1, qreal y1, qreal size, qreal angle, bool a0, bool a1, const QPen& pen) {
	double dx = x1 - x0;
	double dy = y1 - y0;
	GLEPoint start_pt((double)x0, (double)y0);
	GLEPoint end_pt((double)x1, (double)y1);
	GLEArrowPoints pts1, pts2;
	GLEGetArrowPointsNoProps(start_pt, dx,  dy, 1, size, angle, &pts1);
	GLEGetArrowPointsNoProps(end_pt,  -dx, -dy, 1, size, angle, &pts2);
	QPen new_pen = pen;
	new_pen.setJoinStyle(Qt::MiterJoin);
	new_pen.setMiterLimit(20);
	if (a0) start_pt.setXY(pts1.xl, pts1.yl);
	if (a1) end_pt.setXY(pts2.xl, pts2.yl);
	p->setPen(pen);
	p->drawLine(QPointF(start_pt.getX(), start_pt.getY()), QPointF(end_pt.getX(), end_pt.getY()));
	p->setPen(new_pen);
	if (a0) {
		QPointF poly[3];
		poly[0] = QPointF(pts1.xa, pts1.ya);
		poly[1] = QPointF(pts1.xt, pts1.yt);
		poly[2] = QPointF(pts1.xb, pts1.yb);
		p->setBrush(QBrush(Qt::black));
		p->drawConvexPolygon(poly, 3);
	}
	if (a1) {
		QPointF poly[3];
		poly[0] = QPointF(pts2.xa, pts2.ya);
		poly[1] = QPointF(pts2.xt, pts2.yt);
		poly[2] = QPointF(pts2.xb, pts2.yb);
		p->setBrush(QBrush(Qt::black));
		p->drawConvexPolygon(poly, 3);
	}
}

//! Static member to draw a scale handle around a point
void QGLE::drawScaleHandle(QPainter *p, QPointF origin, int dx, int dy, double half_size, QPen pen) {
	QGLE::drawBox(p, origin, half_size, pen);
	QGLE::drawArrowLine(p, origin.x()+dx*half_size, origin.y()+dy*half_size,
						origin.x()+4*dx*half_size, origin.y()+4*dy*half_size, 1.5*half_size, 40, true, true, pen);
}

//! Static member to draw a cross at a point
void QGLE::drawCross(QPainter *p, QPointF origin, double half_size, QPen pen)
{
	p->setPen(pen);
	p->drawLine(QLineF(origin.x()-half_size,
				origin.y()-half_size,
				origin.x()+half_size,
				origin.y()+half_size));
	p->drawLine(QLineF(origin.x()-half_size,
				origin.y()+half_size,
				origin.x()+half_size,
				origin.y()-half_size));
}

void QGLE::drawPerpMark(QPainter *p, QPointF origin, QPen pen)
{
	p->setPen(pen);
	double half_size = OSNAP_BOX_SIZE;
	p->drawLine(QLineF(origin.x()-2*half_size,
				origin.y(),
				origin.x()+2*half_size,
				origin.y()));
	p->drawLine(QLineF(origin.x(),origin.y(),
				origin.x(),origin.y()-4*half_size));
	p->drawLine(QLineF(origin.x()-half_size,
				origin.y(),
				origin.x()-half_size,
				origin.y()-half_size));
	p->drawLine(QLineF(origin.x()-half_size,
				origin.y()-half_size,
				origin.x(),
				origin.y()-half_size));
}

void QGLE::drawTangentMark(QPainter *p, QPointF origin, QPen pen)
{
	p->setPen(pen);
	double half_size = OSNAP_BOX_SIZE;
	p->drawLine(QLineF(origin.x()-2*half_size,
				origin.y(),
				origin.x()+2*half_size,
				origin.y()));
	p->drawArc(QRectF(origin.x()-2*half_size,
				origin.y(),
				4*half_size,
				4*half_size), 0,180*16);

}

void QGLE::drawIntersectMark(QPainter *p, QPointF origin, QPen pen)
{
	p->setPen(pen);
	double half_size = OSNAP_BOX_SIZE;
	p->drawLine(QLineF(origin.x()-2*half_size,
				origin.y()-2*half_size,
				origin.x()+2*half_size,
				origin.y()+2*half_size));
	p->drawLine(QLineF(origin.x(),
				origin.y()-2*half_size,
				origin.x(),
				origin.y()+2*half_size));

}

void QGLE::drawOSnap(QPainter *p, QPointF snap, int type)
{
	QPair<QPointF, int> combined;
	combined.first = snap;
	combined.second = type;
	drawOSnap(p, combined);
}

void QGLE::drawOSnap(QPainter *p, QPair<QPointF, int> snap)
{
	p->setPen(osnapPen());
	// Switch on snap.second
	switch(snap.second)
	{
		case PerpendicularSnap:
			drawPerpMark(p,snap.first,osnapPen());
			break;

		case TangentSnap:
			drawTangentMark(p,snap.first,osnapPen());
			break;

		case IntersectSnap:
			drawIntersectMark(p, snap.first, osnapPen());
			break;

		default:
			drawBox(p,snap.first,OSNAP_BOX_SIZE,osnapPen());
	}
}

bool QGLE::inOrder(double x, double y, double z)
{
	if ((y >= x) && (z >= y))
		return(true);
	else if ((y < x) && (z < y))
		return(true);
	else
		return(false);
}
QPen QGLE::osnapPen()
{
	QPen pen;
	pen.setColor(Qt::darkYellow);
	pen.setWidth(1);
	return(pen);
}

QPen QGLE::osnapLinePen()
{
	QPen pen = osnapPen();
	pen.setStyle(Qt::DashLine);
	return(pen);
}

QPen QGLE::otrackLinePen()
{
	QPen pen = osnapPen();
	pen.setStyle(Qt::DashLine);
	return(pen);
}

QPen QGLE::handlePen()
{
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidth(1);
	return(pen);
}

QPointF QGLE::calculateScaleRatios(QPointF basePoint, QPointF startPoint, QPointF endPoint, int method)
{
	QPointF oldSize = startPoint - basePoint;
	QPointF newSize = endPoint - basePoint;
	double yscale, xscale;

	switch (method)
	{
		case ProportionalScale:
			if (oldSize.x() == 0.0)
				xscale = 1e6;
			else
				xscale = newSize.x() / oldSize.x();

			if (oldSize.y() == 0.0)
				yscale = 1e6;
			else
				yscale = newSize.y() / oldSize.y();

			if (xscale > yscale)
				yscale = xscale;
			else
				xscale = yscale;
			break;



		case HorizontalScale:
			yscale = 1.0;
			if (oldSize.x() == 0.0)
				xscale = 1.0;
			else
				xscale = newSize.x() / oldSize.x();
			break;

		case VerticalScale:
			xscale = 1.0;
			if (oldSize.y() == 0.0)
				yscale = 1.0;
			else
				yscale = newSize.y() / oldSize.y();
			break;


		default:
			xscale = yscale = 1.0;
	}

	return(QPointF(xscale, yscale));
}

QPointF QGLE::parkTransform(QPointF original, double angle)
{
	QPointF newPoint;

	newPoint.setX(cos(angle)*original.x() + sin(angle)*original.y());
	newPoint.setY(cos(angle)*original.y() - sin(angle)*original.x());
	return(newPoint);
}

QPointF QGLE::rotateAboutPoint(QPointF original, double angle, QPointF basePoint)
{
	// Get the point relative to new origin
	QPointF newPoint = original - basePoint;

	// Rotate
	newPoint = parkTransform(newPoint, angle);

	// Move back to the old origin
	newPoint = newPoint + basePoint;

	return(newPoint);
}

QString QGLE::colourQtToGLE(QColor colour)
{

	if (!colour.isValid())
		return("");

	// TODO:
	// It would be better to detect predefined GLE colours
	// and use their names where possible and only resort
	// to CVTRGB if a standard colour cannot be matched.
	return(QString("CVTRGB(%1/255,%2/255,%3/255)")
			.arg(colour.red())
			.arg(colour.green())
			.arg(colour.blue()));
}

QString QGLE::getFileNameExtension(const QString& name)
{
	int pos = name.length();
	// Never search to before "/" or "\\": file may not have extension
	// And don't find "." in path before file name
	while (pos >= 1 && name[pos-1] != '/' && name[pos-1] != '\\' && name[pos-1] != '.' ) {
		pos--;
	}
	if (pos >= 1 && name[pos-1] == '.') {
		return name.mid(pos).toLower();
	} else {
		return QString();
	}
}

bool QGLE::isGraphicsExtension(const QString& name)
{
	QString ext = QGLE::getFileNameExtension(name);
	if (ext == "pdf") return true;
	if (ext == "eps") return true;
	if (ext == "ps") return true;
	if (ext == "png") return true;
	if (ext == "jpg") return true;
	if (ext == "jpeg") return true;
	if (ext == "gif") return true;
	if (ext == "tif") return true;
	if (ext == "tiff") return true;
	return false;
}

QString QGLE::addFileNameExtension(const QString& name, const char* ext)
{
	int  i = name.lastIndexOf('.');
	if (i != -1)
	{
		QString result = name.left(i);
		result.append(".");
		result.append(QString::fromUtf8(ext));
		return result;
	}
	else
	{
		QString result = name;
		result.append(".");
		result.append(QString::fromUtf8(ext));
		return result;
	}
}

void QGLE::ensureFileNameExtension(QString* str, const char* ext) {
	QRegExp rxExt(QString(".*\\.") + ext);
	rxExt.setCaseSensitivity(Qt::CaseInsensitive);
	if (!rxExt.exactMatch(*str)) {
		*str += ".";
		*str += ext;
	}
}
