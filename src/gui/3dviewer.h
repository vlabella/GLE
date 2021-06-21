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

#ifndef _3DVIEWER_H
#define _3DVIEWER_H

#include <QDialog>
#include <QGLWidget>
#include "mainwindow.h"

 class QGLE3DWidget : public QGLWidget
 {
     Q_OBJECT

 public:
	 QGLE3DWidget(QWidget *parent = 0, GLEInterface* iface = NULL);
     ~QGLE3DWidget();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;
     void zoom(double zoom);
     void rotate(double angle, bool horiz);
     void reference(const GLEPoint3D& p);
     void adjustV(double angle);
     void perspectiveAngle(double delta);

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);

 private:
     GLuint makeObject();
     void normalizeAngle(int *angle);

     double perspAngle;
     GLuint object;
     QPoint lastPos;
     GLEInterface* gleInterface;
     GLEProjection proj;
};

typedef enum {CTRL_EYE, CTRL_REF, CTRL_VVEC, CTRL_PERSP_ANG} CTRL_CMD;

class QGLE3DViewer : public QDialog
{
    Q_OBJECT

public:
	QGLE3DViewer(QWidget* parent, GLEInterface* iface);

protected:
	//! Called when the user presses a key
	void keyPressEvent(QKeyEvent *event);

private slots:
	void ctrlSelectionChanged(int value);

private:
    QSlider *createSlider();

    QGLE3DWidget *glWidget;
    QComboBox *crCtrl;

    CTRL_CMD ctrl;
    double zoomStep;
    double rotateStep;
    double distanceStep;
};

#endif
