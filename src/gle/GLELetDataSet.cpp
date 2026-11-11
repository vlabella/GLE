#define GRAPHDEF extern

#include <set>
#include <vector>
#include <string>
#include "all.h"
#include "tokens/stokenizer.h"
#include "cutils.h"
#include "file_io.h"
#include "bitmap/img2ps.h"
#include "op_def.h"
#include "mem_limits.h"
#include "token.h"
#include "gle-interface/gle-interface.h"
#include "var.h"
#include "graph.h"
#include "GLELetDataSet.h"

using namespace std;

GLELetDataSet::GLELetDataSet() {
    m_DN         = -1;
    m_Var        = -1;
    m_CrP        = 0;
    m_IsFunction = true;
    m_IsXRangeDS = false;
}

GLELetDataSet::~GLELetDataSet() {
}

void GLELetDataSet::initializeFrom(int dn, int var) {
	m_DN = dn;
	m_Var = var;
	/* points */
	GLEDataSet* dataSet = dp[dn];
	GLEDataPairs pairs(dataSet);
	double* xv = pairs.getX();
	double* yv = pairs.getY();
	int* mm = pairs.getM();
	/* get data from datasets */
	/* record x-values for which y-value is missing separately */
	int nbvals = 0;
	double xprev = GLE_INF;
	for (unsigned int i = 0; i < dataSet->np; i++) {
		if (!mm[i]) {
			if (xv[i] == xprev && nbvals > 0) {
				/* same x-value, but different y-value */
				/* support for datasets with discontinuities */
				m_Vals[nbvals-1].y[GLE_DS_R] = yv[i];
			} else {
				DataSetVal val;
				val.x = xv[i];
				val.y[GLE_DS_L] = yv[i];
				val.y[GLE_DS_R] = yv[i];
				m_Vals.push_back(val);
				xprev = val.x;
				nbvals++;
			}
		} else {
			m_MissX.push_back(xv[i]);
		}
	}
	// Sort data if required
	bool sorted = true;
	for (unsigned int i = 1; i < m_Vals.size(); i++) {
		if (m_Vals[i].x <= m_Vals[i-1].x) sorted = false;
	}
	if (!sorted) {
		std::sort(m_Vals.begin(), m_Vals.end(), DataSetValCMP);
	}
	// Check if dataset represents a function
	m_IsFunction = true;
	for (unsigned int i = 1; i < m_Vals.size(); i++) {
		if (m_Vals[i].x == m_Vals[i-1].x) {
			// Note: GLE can't combine different datasets if they don't represent functions in one let command
			m_IsFunction = false;
		}
	}
}

void GLELetDataSet::complainNoFunction() {
	for (unsigned int i = 1; i < m_Vals.size(); i++) {
		if (m_Vals[i].x == m_Vals[i-1].x) {
			ostringstream errs;
			errs << "dataset d" << m_DN << " not a function - duplicate range value: '" << m_Vals[i].x << "'";
			g_throw_parser_error(errs.str());
		}
	}
}

bool GLELetDataSet::interpolateTo(double x, int lr) {
	/* returns true if dataset has discontinuity at point x */
	if (m_Var == -1) {
		/* dataset not used in let expression, only in 'range" part */
		return false;
	}
	int mx = m_Vals.size();
	if (mx == 0) {
		var_set(m_Var, GLE_NAN);
	} else if (mx == 1) {
		double x_val = m_Vals[0].x;
		if (x < x_val) {
			var_set(m_Var, m_Vals[0].y[GLE_DS_L]);
		} else if (x > x_val) {
			var_set(m_Var, m_Vals[0].y[GLE_DS_R]);
		} else {
			DataSetVal* v = &m_Vals[0];
			var_set(m_Var, v->y[lr]);
			if (lr == GLE_DS_L && v->y[GLE_DS_L] != v->y[GLE_DS_R]) {
				return true;
			}
		}
	} else {
		while (x < m_Vals[m_CrP].x || x > m_Vals[m_CrP+1].x) {
			/* x too small or too large for current interval -> move interval */
			if (x < m_Vals[m_CrP].x) {
				if (m_CrP <= 0) {
					var_set(m_Var, m_Vals[0].y[GLE_DS_L]);
					return false;
				}
				m_CrP--;
			}
			if (x > m_Vals[m_CrP+1].x) {
				if (m_CrP+2 >= mx) {
					var_set(m_Var, m_Vals[mx-1].y[GLE_DS_R]);
					return false;
				}
				m_CrP++;
			}
		}
		if (x == m_Vals[m_CrP].x) {
			DataSetVal* v = &m_Vals[m_CrP];
			var_set(m_Var, v->y[lr]);
			if (lr == GLE_DS_L && v->y[GLE_DS_L] != v->y[GLE_DS_R]) {
				return true;
			}
		} else if (x == m_Vals[m_CrP+1].x) {
			DataSetVal* v = &m_Vals[m_CrP+1];
			var_set(m_Var, v->y[lr]);
			if (lr == GLE_DS_L && v->y[GLE_DS_L] != v->y[GLE_DS_R]) {
				return true;
			}
		} else {
			double y1 = m_Vals[m_CrP].y[GLE_DS_R];
			double y2 = m_Vals[m_CrP+1].y[GLE_DS_L];
			double x1 = m_Vals[m_CrP].x;
			double x2 = m_Vals[m_CrP+1].x;
			var_set(m_Var, (x-x1)/(x2-x1)*(y2-y1)+y1);
		}
	}
	return false;
}
