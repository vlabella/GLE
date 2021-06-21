/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://www.gle-graphics.org/>          *
 *                                                                      *
 * Modified BSD License                                                 *
 *                                                                      *
 * Copyright (C) 2009 GLE.                                              *
 *                                                                      *
 * Redistribution and use in source and binary forms, with or without   *
 * modification, are permitted provided that the following conditions   *
 * are met:                                                             *
 *                                                                      *
 *    1. Redistributions of source code must retain the above copyright *
 * notice, this list of conditions and the following disclaimer.        *
 *                                                                      *
 *    2. Redistributions in binary form must reproduce the above        *
 * copyright notice, this list of conditions and the following          *
 * disclaimer in the documentation and/or other materials provided with *
 * the distribution.                                                    *
 *                                                                      *
 *    3. The name of the author may not be used to endorse or promote   *
 * products derived from this software without specific prior written   *
 * permission.                                                          *
 *                                                                      *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR   *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY       *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   *
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER *
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN  *
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
 *                                                                      *
 ************************************************************************/

#ifndef __GLE_SHAPEMATH__
#define __GLE_SHAPEMATH__

class DLLCLASS GLECurve {
public:
	GLECurve();
	virtual ~GLECurve();
	virtual void getC(double t, GLEPoint& p) = 0;
	virtual void getCp(double t, GLEPoint& p) = 0;
	virtual void getCpp(double t, GLEPoint& p) = 0;
	virtual double getT0();
	virtual double getT1();
	virtual double getDist(double t1, double t2);
	virtual double distToParamValue(double t1, double dist, double t2);
	virtual double distToParamValue(double t1, double dist);
	double getDistp(double t);
protected:
	double computeDistRecursive(double t1, GLEPoint& p1, double t2, GLEPoint& p2);
};

class DLLCLASS GLECurveT0T1 : public GLECurve {
protected:
	double m_T0, m_T1;
public:
	GLECurveT0T1();
	~GLECurveT0T1();
	virtual double getT0();
	virtual double getT1();
};

class DLLCLASS GLECircleArc : public GLECurveT0T1 {
protected:
	GLEPoint m_Orig;
	double m_R;
public:
	GLECircleArc(const GLEPoint& orig, double r, double t0, double t1);
	virtual ~GLECircleArc();
	virtual void getC(double t, GLEPoint& p);
	virtual void getCp(double t, GLEPoint& p);
	virtual void getCpp(double t, GLEPoint& p);
	virtual double getDist(double t1, double t2);
	virtual double distToParamValue(double t1, double dist, double t2);
	virtual double distToParamValue(double t1, double dist);
};

class DLLCLASS GLEEllipseArc : public GLECurveT0T1 {
protected:
	GLEPoint m_Orig;
	double m_Rx, m_Ry;
public:
	GLEEllipseArc(const GLEPoint& orig, double rx, double ry, double t0, double t1);
	virtual ~GLEEllipseArc();
	virtual void getC(double t, GLEPoint& p);
	virtual void getCp(double t, GLEPoint& p);
	virtual void getCpp(double t, GLEPoint& p);
};

class DLLCLASS GLEBezier : public GLECurve {
protected:
	GLEPoint m_P0, m_P1, m_P2, m_P3;
	double m_Ax, m_Bx, m_Cx, m_Ay, m_By, m_Cy;
public:
	GLEBezier();
	GLEBezier(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);
	GLEBezier(const GLEBezier& other);
	~GLEBezier();
	inline void setP0(const GLEPoint& p) { m_P0.set(p); }
	inline void setP1(const GLEPoint& p) { m_P1.set(p); }
	inline void setP2(const GLEPoint& p) { m_P2.set(p); }
	inline void setP3(const GLEPoint& p) { m_P3.set(p); }
	inline void setP0(double x, double y) { m_P0.setXY(x, y); }
	inline void setP1(double x, double y) { m_P1.setXY(x, y); }
	inline void setP2(double x, double y) { m_P2.setXY(x, y); }
	inline void setP3(double x, double y) { m_P3.setXY(x, y); }
	inline const GLEPoint& getP0() const { return m_P0; }
	inline const GLEPoint& getP1() const { return m_P1; }
	inline const GLEPoint& getP2() const { return m_P2; }
	inline const GLEPoint& getP3() const { return m_P3; }
	inline GLEPoint& P0() { return m_P0; }
	inline GLEPoint& P1() { return m_P1; }
	inline GLEPoint& P2() { return m_P2; }
	inline GLEPoint& P3() { return m_P3; }
	virtual void getC(double t, GLEPoint& p);
	virtual void getCp(double t, GLEPoint& p);
	virtual void getCpp(double t, GLEPoint& p);
	void throughPoint(GLEPoint& p, GLEPoint& dir1, GLEPoint& dir2);
	void updateEquation();
	void draw();
	void cutAtParamValue(double t);
	void cutFromParamValue(double t);
};

class DLLCLASS GLECurvedArrowHead {
protected:
	GLECurve* m_Curve;
	GLEBezier m_Side1, m_Side2;
	double m_T0, m_TM, m_T1;
	double m_ArrAlpha, m_ArrSize, m_LWidth;
	bool m_Enable, m_Sharp;
	int m_ArrStyle;
public:
	GLECurvedArrowHead(GLECurve* curve);
	virtual ~GLECurvedArrowHead();
	void getA(double t, double pm, GLEPoint& p);
	void getAp(double t, double pm1, double pm2, GLEPoint& p);
	void setArrowAngleSize(int style, double size, double angle);
	void setArrowAngleSizeSharp(int style, double size, double angle);
	void drawDirection(bool dir);
	void setStartEnd(bool dir);
	double getParamValueEnd();
	double getArrowCurveDist();
	void computeAndDraw();
	DLLFCT void computeArrowHead();
	void draw();
	GLEBezier* getSide1() { return &m_Side1; }
	GLEBezier* getSide2() { return &m_Side2; }
	inline int getStyle() { return m_ArrStyle; }
	inline bool isSharp() { return m_Sharp; }
	inline void setSharp(bool sharp) { m_Sharp = sharp; }
	inline void setLineWidth(double lwidth) { m_LWidth = lwidth; }
	inline bool isEnabled() { return m_Enable; }
	inline void setEnabled(bool enable) { m_Enable = enable; }
};

DLLFCT void GLEArcUpdateCurvedArrowHeads(GLECurvedArrowHead* head_start,
                                         GLECurvedArrowHead* head_end,
                                         double* t1,
                                         double* t2,
                                         GLEPropertyStore* props,
                                         double fac,
                                         int arrow);

DLLFCT double GLEArcNormalizedAngle2(double a1, double a2);

#endif // __GLE_SHAPEMATH__
