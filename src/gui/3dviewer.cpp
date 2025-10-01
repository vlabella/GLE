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
#include <QtOpenGL>
#include <QOpenGLWidget>
#include "3dviewer.h"
#include "../config.h"
#include "../gle/cutils.h"
#include "../gle/gle-block.h"
#include "../gle/surface/gsurface.h"

#ifdef Q_OS_WIN32
	#include <windows.h>
#endif


#ifdef HAVE_LIBGLU_H
#ifdef __APPLE__
	#include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif
#endif

#include <math.h>

QGLE3DWidget::QGLE3DWidget(QWidget *parent, GLEInterface* iface)
     : QOpenGLWidget(parent)
{
	 gleInterface = iface;
     object = 0;
     perspAngle	= 60.0;
     GLEPoint3D* eye  = proj.getEye();
     GLEPoint3D* ref  = proj.getReference();
     GLEPoint3D* vvec = proj.getV();
     surface_struct* sf = gleInterface->getSurface();
     if (sf->z != NULL) {
    	 eye->set(3,0,0);
    	 ref->set(0,0,0);
    	 vvec->set(0,0,1);
     }
}

QGLE3DWidget::~QGLE3DWidget()
{
     makeCurrent();
     glDeleteLists(object, 1);
}

QSize QGLE3DWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize QGLE3DWidget::sizeHint() const
{
    return QSize(400, 400);
}

void QGLE3DWidget::zoom(double zoom) {
	proj.zoom(zoom);
	update();
}

void QGLE3DWidget::rotate(double angle, bool horiz) {
	proj.rotate(angle, horiz);
	update();
}

void QGLE3DWidget::reference(const GLEPoint3D& p) {
	proj.reference(p);
	update();
}

void QGLE3DWidget::adjustV(double angle) {
	proj.adjustV(angle);
	update();
}

void QGLE3DWidget::perspectiveAngle(double delta) {
	perspAngle += delta;
	if (perspAngle < 5) perspAngle = 5;
	if (perspAngle > 85) perspAngle = 85;
	update();
}

void QGLE3DWidget::initializeGL()
{
    //qglClearColor(Qt::white);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); /// Alternatively: QColorConstants::Svg::white
    object = makeObject();
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
}

#include <iostream>
using namespace std;

void QGLE3DWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
    gluPerspective(perspAngle, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    GLEPoint3D* eye  = proj.getEye();
    GLEPoint3D* ref  = proj.getReference();
    GLEPoint3D* vvec = proj.getV();
    gluLookAt(eye->get(0),eye->get(1),eye->get(2),
              ref->get(0),ref->get(1),ref->get(2),
              vvec->get(0),vvec->get(1),vvec->get(2));
    surface_struct* sf = gleInterface->getSurface();
    if (sf->z != NULL) {
    	glScaled(2.0/sf->nx, 2.0/sf->ny, 2.0/(sf->zmax-sf->zmin));
    	glTranslated(-0.5*sf->nx, -0.5*sf->ny, -0.5*(sf->zmax-sf->zmin)-sf->zmin);
    }
    glCallList(object);
}

void QGLE3DWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void QGLE3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void QGLE3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if (event->buttons() & Qt::LeftButton) {
    	proj.rotate(-dx*0.1, true);
    	proj.rotate(dy*0.1, false);
    	update();
    }
    lastPos = event->pos();
}

GLuint QGLE3DWidget::makeObject()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    surface_struct* sf = gleInterface->getSurface();
    float* z = sf->z;
    if (z != NULL) {
    	GLEColorList* colors = GLEGetColorList();
    	QString gle_color = QString::fromUtf8(sf->top_color).toUpper();
    	GLEColor* color = colors->getSafeDefaultBlack(gle_color.toLatin1().constData());
    	glEnable(GL_POLYGON_OFFSET_FILL); // Avoid Stitching!
    	glPolygonOffset(1.0, 1.0);
    	double l = 0.8;
    	double lRed = min(color->getRed()+l, 1.0);
    	double lGreen = min(color->getGreen()+l, 1.0);
    	double lBlue = min(color->getBlue()+l, 1.0);
    	for (int x = 0; x < sf->nx-1; x++) {
			for (int y = 0; y < sf->ny-1; y++) {
				glBegin(GL_POLYGON);
  				//glColor3f(1.0f, 1.0f, 1.0f);
  				glColor3f(lRed, lGreen, lBlue);
				glVertex3f(x,   y,   z[x     + y * sf->nx]);
				glVertex3f(x+1, y,   z[(x+1) + y * sf->nx]);
				glVertex3f(x+1, y+1, z[(x+1) + (y+1) * sf->nx]);
				glVertex3f(x,   y+1, z[x + (y+1) * sf->nx]);
	  			glEnd();
  				glBegin(GL_LINE_LOOP);
  				glColor3f(color->getRed(), color->getGreen(), color->getBlue());
				glVertex3f(x,   y,   z[x     + y * sf->nx]);
				glVertex3f(x+1, y,   z[(x+1) + y * sf->nx]);
				glVertex3f(x+1, y+1, z[(x+1) + (y+1) * sf->nx]);
				glVertex3f(x,   y+1, z[x + (y+1) * sf->nx]);
	  			glEnd();
			}
    	}
    }
    glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(0,      0,      sf->zmin);
	glVertex3f(sf->nx, 0,      sf->zmin);
	glVertex3f(sf->nx, sf->ny, sf->zmin);
	glVertex3f(0,      sf->ny, sf->zmin);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(0,       0,      sf->zmax);
	glVertex3f(sf->nx,  0,      sf->zmax);
	glVertex3f(sf->nx,  sf->ny, sf->zmax);
	glVertex3f(0,       sf->ny, sf->zmax);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0,       0,      sf->zmin);
	glVertex3f(0,       0,      sf->zmax);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(sf->nx,  0,      sf->zmin);
	glVertex3f(sf->nx,  0,      sf->zmax);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0,       sf->ny, sf->zmin);
	glVertex3f(0,       sf->ny, sf->zmax);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(sf->nx,  sf->ny, sf->zmin);
	glVertex3f(sf->nx,  sf->ny, sf->zmax);
	glEnd();
    glEndList();
    return list;
}

void QGLE3DWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
	*angle += 360 * 16;
    while (*angle > 360 * 16)
	*angle -= 360 * 16;
}

QGLE3DViewer::QGLE3DViewer(QWidget* parent, GLEInterface* iface) : QDialog(parent)
{
    glWidget = new QGLE3DWidget(this, iface);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget, 1);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel(tr("Control (Space toggles):")));
    crCtrl = new QComboBox();
    crCtrl->addItem(tr("Eye"));
    crCtrl->addItem(tr("Reference"));
    crCtrl->addItem(tr("V-Vector"));
    crCtrl->addItem(tr("Perspective Angle"));
    crCtrl->setCurrentIndex(0);
    connect(crCtrl, SIGNAL(activated(int)), this, SLOT(ctrlSelectionChanged(int)));
    rightLayout->addWidget(crCtrl);
    rightLayout->addStretch(1);
    mainLayout->addLayout(rightLayout, 0);
    setLayout(mainLayout);

    setWindowTitle(tr("QGLE 3D Viewer"));

	// Accept keyboard events
	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::OtherFocusReason);

	zoomStep = 5.0;
	rotateStep = 1.0;
	distanceStep = 0.1;
	ctrl = CTRL_EYE;
}

void QGLE3DViewer::ctrlSelectionChanged(int value) {
	ctrl = (CTRL_CMD)value;
}

//! Called when the user presses a key
void QGLE3DViewer::keyPressEvent(QKeyEvent *event)
{
	if (event->modifiers() != Qt::NoModifier) {
		event->ignore();
	}
	GLEPoint3D vec;
	switch (event->key())
	{
		case Qt::Key_Space:
			if (ctrl == CTRL_EYE) ctrl = CTRL_REF;
			else if (ctrl == CTRL_REF) ctrl = CTRL_VVEC;
			else if (ctrl == CTRL_VVEC) ctrl = CTRL_PERSP_ANG;
			else ctrl = CTRL_EYE;
			crCtrl->setCurrentIndex((int)ctrl);
			break;
		case Qt::Key_PageUp:
			vec.set(0,0,1);
			vec.dotScalar(distanceStep);
			if (ctrl != CTRL_REF) glWidget->zoom(-1.0*zoomStep/100.0);
			else glWidget->reference(vec);
			break;
		case Qt::Key_PageDown:
			vec.set(0,0,-1);
			vec.dotScalar(distanceStep);
			if (ctrl != CTRL_REF) glWidget->zoom(+1.0*zoomStep/100.0);
			else glWidget->reference(vec);
			break;
		case Qt::Key_Down:
			vec.set(0,1,0);
			vec.dotScalar(distanceStep);
			if (ctrl == CTRL_PERSP_ANG) glWidget->perspectiveAngle(-5.0);
			else if (ctrl != CTRL_REF) glWidget->rotate(-1.0f*rotateStep, false);
			else glWidget->reference(vec);
			break;
		case Qt::Key_Up:
			vec.set(0,-1,0);
			vec.dotScalar(distanceStep);
			if (ctrl == CTRL_PERSP_ANG) glWidget->perspectiveAngle(+5.0);
			else if (ctrl != CTRL_REF) glWidget->rotate(+1.0f*rotateStep, false);
			else glWidget->reference(vec);
			break;
		case Qt::Key_Left:
			vec.set(1,0,0);
			vec.dotScalar(distanceStep);
			if (ctrl == CTRL_EYE) glWidget->rotate(-1.0f*rotateStep, true);
			else if (ctrl == CTRL_REF) glWidget->reference(vec);
			else if (ctrl == CTRL_VVEC) glWidget->adjustV(+1.0f*rotateStep);
			break;
		case Qt::Key_Right:
			vec.set(-1,0,0);
			vec.dotScalar(distanceStep);
			if (ctrl == CTRL_EYE) glWidget->rotate(+1.0f*rotateStep, true);
			else if (ctrl == CTRL_REF) glWidget->reference(vec);
			else if (ctrl == CTRL_VVEC) glWidget->adjustV(-1.0f*rotateStep);
			break;
		default:
			event->ignore();
	}
}

