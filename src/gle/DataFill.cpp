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
#include "glearray.h"
#include "polish.h"
#include "graph.h"
#include "GLELetDataSet.h"
#include "DataFill.h"

using namespace std;

DataFillDimension::DataFillDimension(GLEFunctionParserPcode* fct) {
    m_Log    = false;
    m_Fct    = fct;
    m_Values = new GLEDoubleArray();
}

DataFillDimension::~DataFillDimension() {
}

void DataFillDimension::setRange(GLERange* range, bool log) {
	m_Log = log;
	if (range->invalid()) {
		range->initRange();
	} else {
		m_Range.setMin(range->getMin() - range->getWidth()/100.0);
		m_Range.setMax(range->getMax() + range->getWidth()/100.0);
	}
}

bool DataFillDimension::isYValid() {
	if (gle_isnan(m_Value)) return false;
	if (m_Value < m_Range.getMin() || m_Value > m_Range.getMax()) return false;
	return true;
}


DataFill::DataFill(bool finetune) {
    m_Size                   = 0;
    m_VarX                   = -1;
    m_PrevInvalid            = false;
    m_PrevPoint              = false;
    m_FineTune               = finetune;
    m_PrevXValue             = GLE_INF;
    m_CrEnable               = true;
    m_Missing                = new GLEBoolArray();
    m_Where                  = NULL;
    m_detectDiscontinuity    = false;
    m_discontinuityThreshold = GLE_INF;
    m_tuneDistance           = 1e-6;
    m_tuneIterationsMin      = 50;
    m_tuneIterationsMax      = 10000;
}

DataFill::~DataFill() {
}

bool DataFill::isYValid() {
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		if (!dim->isYValid()) return false;
	}
	return true;
}

bool DataFill::isYNotNan() {
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		if (dim->isYNan()) return false;
	}
	return true;
}

bool DataFill::isRangeValid() {
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		if (dim->getRange()->invalid()) return false;
	}
	return true;
}

void DataFill::selectXValueNoIPol(double x) {
	/* set parameter variable */
	if (m_VarX >= 0) var_set(m_VarX, x);
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		dim->computeValue();
	}
}

bool DataFill::selectXValue(double x, int lr) {
	/* set parameter variable */
	if (m_VarX >= 0) var_set(m_VarX, x);
	/* set dataset variables */
	bool onceMore = false;
	for (unsigned int i = 0; i < m_DataSets->size(); i++) {
		onceMore |= (*m_DataSets)[i]->interpolateTo(x, lr);
	}
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		dim->computeValue();
	}
	return onceMore;
}

double DataFill::maxDistanceTo(double x) {
	GLERange range;
	minMaxDistanceTo(x, &range);
	return range.getMax();
}

void DataFill::minMaxDistanceTo(double x, GLERange* distanceRange) {
	/* set parameter variable */
	if (m_VarX >= 0) var_set(m_VarX, x);
	/* set dataset variables */
	for (unsigned int i = 0; i < m_DataSets->size(); i++) {
		(*m_DataSets)[i]->interpolateTo(x, GLELetDataSet::GLE_DS_L);
	}
	double maxValue = 0.0;
	double minValue = GLE_INF;
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		if (dim->isYValid()) {
			double v1 = dim->getValue();
			dim->computeValue();
			if (dim->isYValid()) {
				double v2 = dim->getValue();
				double d = axis_range_dist_perc(v1, v2, dim->getRange(), dim->isLog());
				maxValue = max(maxValue, d);
				minValue = min(minValue, d);
			}
		}
	}
	distanceRange->setMinMax(minValue, maxValue);
}

void DataFill::tryIPol(double other, double nanval) {
	int iter = 0;
	double not_xmid = 0.0;
	while (true) {
		double xmid = (other + nanval)/2;
		selectXValue(xmid, GLELetDataSet::GLE_DS_L);
		if (!isYValid()) {
			nanval = xmid;
			not_xmid = other;
		} else {
			other = xmid;
			not_xmid = nanval;
		}
		if (iter > m_tuneIterationsMax || (iter > m_tuneIterationsMin
										   && maxDistanceTo(not_xmid) < m_tuneDistance)) {
			addPointLR(xmid, GLELetDataSet::GLE_DS_L);
			return;
		}
		iter++;
	}
}

void DataFill::tryAddMissing(double x, int lr) {
	if (m_MissX.find(x) != m_MissX.end()) {
		addMissingLR(x, lr);
	}
}

void DataFill::addPoint(double x, double y) {
	m_Dim[0]->setDoubleAt(x, m_Size);
	m_Dim[1]->setDoubleAt(y, m_Size);
	m_Missing->setBoolAt(false, m_Size);
	m_Size++;
}

void DataFill::addMissing() {
	m_Missing->setBoolAt(true, m_Size);
	m_Size++;
}

void DataFill::addPoint(double x) {
	int lr = GLELetDataSet::GLE_DS_L;
	while (true) {
		bool more = selectXValue(x, lr);
		if (!more && m_MissX.find(x) != m_MissX.end()) {
			addMissingLR(x, lr);
			return;
		}
		if (m_Where != NULL) {
			bool wval = m_Where->evalBool();
			if (m_CrEnable && !wval) {
				// disable from now on -> emit missing value
				addMissingLR(x, lr);
			}
			m_CrEnable = wval;
		}
		if (m_CrEnable) addPoint();
		if (more) {
			tryAddMissing(x, lr);
			lr++;
		} else {
			break;
		}
	}
}

void DataFill::addPoint() {
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		dim->setDoubleAt(dim->getValue(), m_Size);
	}
	m_Missing->setBoolAt(false, m_Size);
	m_Size++;
}

void DataFill::addPointLR(double x, int lr) {
	selectXValue(x, lr);
	addPoint();
}

void DataFill::addMissingLR(double x, int lr) {
	selectXValue(x, lr);
	for (unsigned int i = 0; i < m_Dim.size(); i++) {
		DataFillDimension* dim = m_Dim[i];
		if (dim->isYNan()) dim->setDoubleAt(GLE_NAN, m_Size);
		else dim->setDoubleAt(dim->getValue(), m_Size);
	}
	m_Missing->setBoolAt(true, m_Size);
	m_Size++;
}

void DataFill::checkDiscontinuity(double xPrev, double xNext, int lr) {
	if (!m_detectDiscontinuity) {
		return;
	}
	double leftX = xPrev;
	double rightX = xNext;
	selectXValue(leftX, lr);
	double distance = maxDistanceTo(rightX);
	if (distance > m_discontinuityThreshold) {
		int iter = 0;
		GLERange leftDistance;
		GLERange rightDistance;
		while (true) {
			double xMid = (leftX + rightX) / 2;
			selectXValue(xMid, lr);
			minMaxDistanceTo(leftX, &leftDistance);
			selectXValue(xMid, lr);
			minMaxDistanceTo(rightX, &rightDistance);
			double minDistance = leftDistance.getMin() + rightDistance.getMin();
			if (leftDistance.getMax() > m_discontinuityThreshold) {
				rightX = xMid;
				distance = leftDistance.getMax();
			} else if (rightDistance.getMax() > m_discontinuityThreshold) {
				leftX = xMid;
				distance = rightDistance.getMax();
			} else {
				return;
			}
			iter++;
			if (iter > m_tuneIterationsMax || (iter > m_tuneIterationsMin && minDistance < m_tuneDistance)) {
				if (leftX != xPrev) {
					addPointLR(leftX, lr);
				}
				addMissingLR(xMid, lr);
				if (rightX != xNext) {
					addPointLR(rightX, lr);
				}
				return;
			}
		}
	}
}

void DataFill::addPointFineTune(double x, int lr) {
	if (!isYValid()) {
		bool yNotNan = isYNotNan();
		if (!m_PrevInvalid && m_PrevPoint) {
			tryIPol(m_PrevXValue, x);
		}
		m_PrevInvalid = true;
		addMissingLR(x, lr);
		if (yNotNan) addPointLR(x, lr);
	} else {
		if (m_PrevInvalid) {
			tryIPol(x, m_PrevXValue);
			m_PrevInvalid = false;
		} else {
			checkDiscontinuity(m_PrevXValue, x, lr);
		}
		addPointLR(x, lr);
	}
}

void DataFill::addPointIPol(double x) {
	int lr = GLELetDataSet::GLE_DS_L;
	while (true) {
		bool more = selectXValue(x, lr);
		if (!more && m_MissX.find(x) != m_MissX.end()) {
			// if more, then missing value is emitted in-between left and right limit
			addMissingLR(x, lr);
			return;
		}
		if (m_Where != NULL) {
			bool wval = m_Where->evalBool();
			if (m_CrEnable && !wval) {
				// disable from now on -> emit missing value
				addMissingLR(x, lr);
				m_PrevPoint = false;
				m_PrevInvalid = false;
			}
			m_CrEnable = wval;
		}
		if (m_CrEnable) {
			if (m_FineTune) {
				addPointFineTune(x, lr);
			} else {
				addPoint();
			}
			m_PrevPoint = true;
			m_PrevXValue = x;
		}
		if (more) {
			tryAddMissing(x, lr);
			lr++;
		} else {
			break;
		}
	}
}

void DataFill::setDetectDiscontinuity(bool detect, double threshold) {
	m_detectDiscontinuity = detect;
	m_discontinuityThreshold = threshold;
}

void DataFill::toDataset(GLEDataSet* dataset) {
	dataset->np = size();
	GLEArrayImpl* dimensions = dataset->getData();
	dimensions->ensure(m_Dim.size());
	for (unsigned int dim = 0; dim < m_Dim.size(); dim++) {
		GLEArrayImpl* newArray = new GLEArrayImpl();
		newArray->ensure(dataset->np);
		dimensions->setObject(dim, newArray);
		GLEDoubleArray* values = m_Dim[dim]->getValues();
		for (unsigned int i = 0; i < dataset->np; i++) {
			if (m_Missing->getBoolAt(i)) {
				newArray->setUnknown(i);
			} else {
				newArray->setDouble(i, values->getDoubleAt(i));
			}
		}
	}
}
