#include <set>
#include <vector>
#include <string>
#include "GLECheckWindow.h"
using namespace std;

GLECheckWindow::GLECheckWindow() {
	m_HasXMin = m_HasXMax = m_HasYMin = m_HasYMax = false;
	m_XMin = m_XMax = m_YMin = m_YMax = 0.0;
}

GLECheckWindow::~GLECheckWindow() {
}

void GLECheckWindow::setXMin(double v) {
	m_XMin = v;
	m_HasXMin = true;
}

void GLECheckWindow::setXMax(double v) {
	m_XMax = v;
	m_HasXMax = true;
}

void GLECheckWindow::setYMin(double v) {
	m_YMin = v;
	m_HasYMin = true;
}

void GLECheckWindow::setYMax(double v) {
	m_YMax = v;
	m_HasYMax = true;
}

bool GLECheckWindow::valid(double x, double y) {
	if (m_HasXMin && x < m_XMin) return false;
	if (m_HasXMax && x > m_XMax) return false;
	if (m_HasYMin && y < m_YMin) return false;
	if (m_HasYMax && y < m_YMax) return false;
	return true;
}


