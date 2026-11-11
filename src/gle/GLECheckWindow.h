#ifndef GLE_CHECK_WINDOW_H
#define GLE_CHECK_WINDOW_H

class GLECheckWindow {
protected:
	bool m_HasXMin, m_HasXMax, m_HasYMin, m_HasYMax;
	double m_XMin, m_XMax, m_YMin, m_YMax;
public:
	GLECheckWindow();
	~GLECheckWindow();
	void setXMin(double v);
	void setXMax(double v);
	void setYMin(double v);
	void setYMax(double v);
	bool valid(double x, double y);
};


#endif