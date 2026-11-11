#define GRAPHDEF extern

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

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
#include "pass.h"
#include "gle-fitls.h"
#include "GLELet.h"
#include <boost/math/statistics/linear_regression.hpp>

using namespace std;

#define DEFAULT_STEPS 100

extern GLEAxis xx[];

GLELet::GLELet() {
    m_SubMap        = NULL;
    m_LetFrom       = 0.0;
    m_LetTo         = 0.0;
    m_LetStep       = 0.0;
    m_LetNSteps     = 0;
    m_VarX          = -1;
    m_NoFirst       = false;
    m_FineTune      = false;
    m_HasSteps      = false;
    m_HasStepOption = false;
    m_HasFrom       = false;
    m_HasTo         = false;
    m_Ds            = -1;
    m_codeLine      = -1;
    m_fitType       = "none";
    m_HistDS        = -1;
    m_nrBins        = -1;
    m_FitDS         = -1;
    m_limitDataX    = false;
    m_limitDataY    = false;
    m_limitData     = false;
    m_Materialize   = false;
}

GLELet::~GLELet() {
}

void GLELet::createVarBackup(GLEVars* vars, StringIntHash* varHash) {
	vector<int> ids;
	for (StringIntHash::iterator i(varHash->begin()); i != varHash->end(); ++i) {
		ids.push_back(i->second);
	}
	m_varBackup.backup(vars, ids);
}

void GLELet::restoreVarBackup(GLEVars* vars) {
	m_varBackup.restore(vars);
}

void GLELet::setStep(double value) {
	m_LetStep = value;
	setHasSteps(true);
}

void GLELet::initVars() {
	int vartype = 1;
	var_findadd("X", &m_VarX, &vartype);
}

void GLELet::initStep() {
	if (hasSteps()) return;
	int nstep = m_LetNSteps;
	if (nstep == 0) nstep = DEFAULT_STEPS;
	if (xx[1].log) {
		setStep(nstep);
	} else {
		setStep((getTo() - getFrom())/(nstep-1));
	}
}

GLEFunctionParserPcode* GLELet::insertFunction() {
	GLERC<GLEFunctionParserPcode> fct = new GLEFunctionParserPcode();
	m_Fcts.insert(m_Fcts.begin(), fct);
	return fct.get();
}

GLEFunctionParserPcode* GLELet::addFunction() {
	GLEFunctionParserPcode* fct = new GLEFunctionParserPcode();
	m_Fcts.add(fct);
	return fct;
}

GLEFunctionParserPcode* GLELet::addWhere() {
	m_Where = new GLEFunctionParserPcode();
	return m_Where.get();
}

bool GLELet::checkIdenticalRanges(GLEVectorAutoDelete<GLELetDataSet>& datasets) {
	if (datasets.size() == 0) return false;
	if (datasets.size() == 1) return true;
	GLELetDataSet* ds0 = datasets[0];
	unsigned int nbX = dp[ds0->getDatasetID()]->np;
	for (unsigned int i = 1; i < datasets.size(); i++) {
		if (dp[datasets[i]->getDatasetID()]->np != nbX) return false;
	}
	GLEArrayImpl* rangeX0 = dp[ds0->getDatasetID()]->getDimData(0);
	if (rangeX0 == 0) {
		return false;
	}
	for (unsigned int i = 1; i < datasets.size(); i++) {
		GLEArrayImpl* rangeXi = dp[datasets[i]->getDatasetID()]->getDimData(0);
		if (rangeXi == 0 || rangeX0->size() != rangeXi->size()) {
			return false;
		}
		for (unsigned int j = 0; j < rangeX0->size(); j++) {
			if (!gle_memory_cell_equals(rangeX0->get(j), rangeXi->get(j))) {
				return false;
			}
		}
	}
	return true;
}

void GLELet::transformIdenticalRangeDatasets(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill) {
	// transform single dataset - advantage: dataset does not need to represent a function and is not sorted
	GLELetDataSet* ds0 = datasets[0];
	GLEDataSet* ds0Data = dp[ds0->getDatasetID()];
	unsigned int nbX = ds0Data->np;
	GLEDataPairs ds0Pairs;
	ds0Pairs.copyDimension(ds0Data, 0);
	for (unsigned int j = 0; j < nbX; j++) {
		if (m_HasFrom && ds0Pairs.getX(j) < m_LetFrom) {
			continue;
		}
		if (m_HasTo && ds0Pairs.getX(j) > m_LetTo) {
			continue;
		}
		bool miss = false;
		if (ds0Pairs.getM(j)) {
			miss = true;
		} else {
			for (unsigned int i = 0; i < datasets.size(); i++) {
				GLELetDataSet* dsi = datasets[i];
				GLEArrayImpl* dsiArray = dp[dsi->getDatasetID()]->getDimData(1);
				if (dsiArray != 0 && dsiArray->size() == nbX) {
					if (dsiArray->isUnknown(j)) {
						miss = true;
					} else if (dsi->getVar() != -1) {
						/* dsi->getVar() can be -1 if dataset is only used in "range" expression */
						var_set(dsi->getVar(), dsiArray->get(j));
					}
				}
			}
		}
		if (!miss) {
			// this method sets the x value and computes the values for all dimensions
			fill->selectXValueNoIPol(ds0Pairs.getX(j));
			if (!m_Where.isNull()) {
				if (m_Where->evalBool()) {
					fill->addPoint();
				} else {
					fill->addMissing();
				}
			} else {
				fill->addPoint();
			}
		} else {
			fill->addMissing();
		}
	}
}

void GLELet::combineFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill, double logstep) {
	/* initialize x-range */
	set<double> xvalues;
	for (unsigned int i = 0; i < datasets.size(); i++) {
		if (datasets[i]->isXRangeDS()) {
			for (int j = 0; j < datasets[i]->getNbValues(); j++) {
				double xv = datasets[i]->getXValue(j);
				if ((!m_HasFrom || xv >= m_LetFrom) && (!m_HasTo || xv <= m_LetTo)) {
					xvalues.insert(xv);
				}
			}
			for (int j = 0; j < datasets[i]->getNbMissing(); j++) {
				fill->setMissingValue(datasets[i]->getMissing(j));
			}
		}
	}
	/* get x-values from step option to let command */
	if (datasets.size() == 0 || m_HasStepOption) {
		for (double xv = m_LetFrom; xv < m_LetTo; ) {
			xvalues.insert(xv);
			if (xx[1].log) {
				xv *= logstep;
			} else {
				xv += m_LetStep;
			}
		}
		/* make sure "to" value is in there */
		xvalues.insert(m_LetTo);
	}
	/* iterate over x-values and compute target values */
	fill->setDataSets(&datasets);
	fill->setWhere(m_Where.get());
	bool shouldIPol = fill->isFineTune() && fill->isRangeValid();
	for (set<double>::iterator i = xvalues.begin(); i != xvalues.end(); i++) {
		if (shouldIPol) {
			fill->addPointIPol(*i);
		} else {
			fill->addPoint(*i);
		}
	}
}

void GLELet::complainAboutNoFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets) {
	for (unsigned int i = 0; i < datasets.size(); i++) {
		if (!datasets[i]->isFunction()) {
			datasets[i]->complainNoFunction();
		}
	}
}

void GLELet::doLet() {
	if (m_LetTo <= m_LetFrom) {
		stringstream ss;
		ss << "illegal range for let expression: ";
		GLERange letRange;
		letRange.setMinMax(m_LetFrom, m_LetTo);
		letRange.printRange(ss);
		g_throw_parser_error(ss.str());
	}
	int dset_id = getDataSet();
	if (ndata < dset_id) ndata = dset_id;
	if (dp[dset_id] == NULL) {
		dp[dset_id] = new GLEDataSet(dset_id);
		copy_default(dset_id);
	}
	/* copy default dataset settings */
	DataFill fill(m_FineTune);
	if (g_discontinuityThreshold < 100.0) {
		fill.setDetectDiscontinuity(true, g_discontinuityThreshold / 100.0);
	}
	fill.setVarX(m_VarX);
	for (int dim = GLE_DIM_X; dim <= GLE_DIM_Y; dim++) {
		DataFillDimension* dim_f = new DataFillDimension(m_Fcts[dim].get());
		fill.addDataDimension(dim_f);
		bool log = xx[dp[dset_id]->getDim(dim)->getAxis()].log;
		dim_f->setRange(dp[dset_id]->getDim(dim)->getRange(), log);
	}
	/* Find data sets used as d[i] */
	GLEVars* vars = getVarsInstance();
	vars->setNameMode(nameMode::DETECT);
	fill.selectXValueNoIPol(0.0);
	if (!m_Where.isNull()) {
		m_Where->evalBool();
	}
	vars->setNameMode(nameMode::RETRIEVE);
	/* Find data set variables */
	int ndn = 0;
	int dn_idx[11], dn_var[11];
	if (!m_SubMap.isNull()) {
		var_find_dn(m_SubMap.get(), dn_idx, dn_var, &ndn);
	}
	double logstep = 1.0;
	if (ndn == 0 && xx[1].log) {
		if (m_LetStep < 2) {
			stringstream ss;
			ss << "with a LOG xaxis scale STEP is taken as the number of steps n," << endl;
			ss << "which should be at least 2, but found: " << m_LetStep;
			g_throw_parser_error(ss.str());
		}
		logstep = pow(m_LetTo/m_LetFrom, 1.0/(m_LetStep-1));
	}
	/* was "range" option given? */
	set<int>& rangeDS = getXRangeDS();
	bool allRangeDS = rangeDS.empty();
	/* initialize input datasets */
	bool allFunctions = true;
	GLEVectorAutoDelete<GLELetDataSet> datasets;
	for (int i = 0; i < ndn; i++) {
		GLELetDataSet* ds = new GLELetDataSet();
		datasets.push_back(ds);
		if (dp[dn_var[i]] == NULL) {
			ostringstream errs;
			errs << "dataset not defined: d" << (dn_var[i]);
			g_throw_parser_error(errs.str());
		}
		ds->initializeFrom(dn_var[i], dn_idx[i]);
		if (!ds->isFunction()) allFunctions = false;
		/* should the x-range include the x-range of this dataset? */
		if (allRangeDS) {
			ds->setIsXRangeDS(true);
		} else {
			set<int>::iterator iter = rangeDS.find(ds->getDatasetID());
			if (iter != rangeDS.end()) {
				ds->setIsXRangeDS(true);
				rangeDS.erase(iter);
			}
		}
	}
	/* add datasets that supply x-range values */
	for (set<int>::iterator iter = rangeDS.begin(); iter != rangeDS.end(); iter++) {
		GLELetDataSet* ds = new GLELetDataSet();
		datasets.push_back(ds);
		if (dp[*iter] == NULL) {
			ostringstream errs;
			errs << "dataset not defined: d" << (*iter);
			g_throw_parser_error(errs.str());
		}
		ds->initializeFrom(*iter, -1);
		if (!ds->isFunction()) allFunctions = false;
		ds->setIsXRangeDS(true);
	}
	/* ok, ready to generate points */
	if (checkIdenticalRanges(datasets) && !m_HasStepOption) {
		// transform datasets with identical x-ranges and no "step" option given
		// in this case, the datasets do not need to be a function!
		transformIdenticalRangeDatasets(datasets, &fill);
	} else {
		if (!allFunctions) {
			// code below assumes all given datasets represent functions
			complainAboutNoFunctions(datasets);
		}
		combineFunctions(datasets, &fill, logstep);
	}
	if (!m_NoFirst) {
		dp[dset_id]->backup();
	} else {
		dp[dset_id]->clearAll();
	}
	/* done, transfer to target dataset */
	fill.toDataset(dp[dset_id]);
	vars->setNameMode(nameMode::NAME);
}

void GLELet::parseFitFunction(const string& fct, GLEParser* parser) {
	Tokenizer* tokens = parser->getTokens();
	string& token = tokens->next_token();
	m_FitDS = get_dataset_identifier(token, parser, true);
	m_fitType = fct;
	m_limitDataX = false;
	m_limitDataY = false;
	m_limitData = false;
	while (true) {
		string& token = tokens->try_next_token();
		if (str_i_equals(token, "WITH")) {
			m_fitFct = tokens->next_multilevel_token();
		} else if (str_i_equals(token, "EQSTR")) {
			parser->evalTokenToString(&m_eqStr);
		} else if (str_i_equals(token, "FORMAT")) {
			parser->evalTokenToString(&m_format);
		} else if (str_i_equals(token, "RSQ")) {
			m_rsq = tokens->next_token();
		} else if (str_i_equals(token, "FROM")) {
			setHasFrom(true);
			setFrom(parser->evalTokenToDouble());
		} else if (str_i_equals(token,"TO")) {
			setHasTo(true);
			setTo(parser->evalTokenToDouble());
		} else if (str_i_equals(token, "STEP")) {
			setHasStepOption(true);
			setStep(parser->evalTokenToDouble());
		} else if (str_i_equals(token, "LIMIT_DATA_X")) {
			// user wants the data plotted from dd xmax to dd xmin
			m_limitDataX = true;
		} else if (str_i_equals(token, "LIMIT_DATA_Y")) {
			// get x values from evaluation of slope
			m_limitDataY = true;
		} else if (str_i_equals(token, "LIMIT_DATA")) {
			m_limitData = true;
		} else if (str_i_equals(token, "XMIN")) {
			double xm = parser->evalTokenToDouble();
			m_window.setXMin(xm);
			setFrom(xm);
		} else if (str_i_equals(token, "XMAX")) {
			double xm = parser->evalTokenToDouble();
			m_window.setXMax(xm);
			setTo(xm);
		} else if (str_i_equals(token, "YMIN")) {
			m_window.setYMin(parser->evalTokenToDouble());
		} else if (str_i_equals(token, "YMAX")) {
			m_window.setYMax(parser->evalTokenToDouble());
		} else {
			if (token != "") tokens->pushback_token();
			break;
		}
	}
	// user can supply three variables to get the fit: SLOPE OFFSET R_SQUARED
	if (tokens->has_more_tokens()) {
		m_varSlope = tokens->next_token();
		ensure_valid_var_name(tokens, m_varSlope);
	}
	if (tokens->has_more_tokens()) {
		m_varIntercept = tokens->next_token();
		ensure_valid_var_name(tokens, m_varIntercept);
	}
	if (tokens->has_more_tokens()) {
		m_varR = tokens->next_token();
		ensure_valid_var_name(tokens, m_varR);
	}
	if (tokens->has_more_tokens()) {
		throw tokens->error("extra tokens at end of let command");
	}
}

void GLELet::doFitFunction() {
	bool linfit = false, logefit = false, log10fit = false, powxfit = false, genfit = false;
	// doing fitting routines
	if (str_i_equals(m_fitType, "LINFIT"))   linfit = true;
	if (str_i_equals(m_fitType, "LOGEFIT"))  logefit = true;
	if (str_i_equals(m_fitType, "LOG10FIT")) log10fit = true;
	if (str_i_equals(m_fitType, "POWXFIT"))  powxfit = true;
	if (str_i_equals(m_fitType, "FIT"))      genfit = true;
	// copy to vectors but ignore miss
	vector<double> x, y;
	double xmax = -GLE_INF;
	double ymax = -GLE_INF;
	double xmin = +GLE_INF;
	double ymin = +GLE_INF;
	GLEDataPairs fitPairs(getDataset(m_FitDS));
	for (unsigned int i = 0; i < fitPairs.size(); i++) {
		if (!fitPairs.getM(i)) {
			if (m_window.valid(fitPairs.getX(i), fitPairs.getY(i))) {
				xmax = max(xmax, fitPairs.getX(i));
				xmin = min(xmin, fitPairs.getX(i));
				ymax = max(ymax, fitPairs.getY(i));
				ymin = min(ymin, fitPairs.getY(i));
				x.push_back(fitPairs.getX(i));
				y.push_back(fitPairs.getY(i));
			}
		}
	}
	if (m_limitDataX) {
		setFrom(xmin);
		setTo(xmax);
	}
	int fitds = getDataSet();
	double slope = 0, offset = 0, rsquared = 0;
	char* new_let = new char[1000];
	if (linfit) {
		//least_square(&x, &y, &slope, &offset, &rsquared);
		auto result = boost::math::statistics::simple_ordinary_least_squares_with_R_squared(x, y);
		offset   = std::get<0>(result);
		slope    = std::get<1>(result);
		rsquared = std::get<2>(result);
		if (m_limitDataY || m_limitData) {
			// find x values from ymax and min
			// y=slope*x+offset
			// x=(y-offset)/slope
			double temp_xmax = max((ymax-offset)/slope, (ymin-offset)/slope);
			double temp_xmin = min((ymax-offset)/slope, (ymin-offset)/slope);
			if (m_limitDataY) {
				setTo(temp_xmax);
				setFrom(temp_xmin);
			} else if (m_limitData) {
				setTo(max(temp_xmax, xmax));
				setFrom(min(temp_xmin, xmin));
			}
		}
		sprintf(new_let, "let d%d = %0.10e*x+%0.10e FROM %0.10e TO %0.10e", fitds, slope, offset, getFrom(), getTo());
	} else if (logefit) {
		//take loge of y values
		for (vector<double>::iterator vdi = y.begin(); vdi != y.end(); ++vdi) {
			(*vdi) = log(*vdi);
		}
		//least_square(&x, &y, &slope, &offset, &rsquared);
		auto result = boost::math::statistics::simple_ordinary_least_squares_with_R_squared(x, y);
		offset   = std::get<0>(result);
		slope    = std::get<1>(result);
		rsquared = std::get<2>(result);
		if (m_limitDataY || m_limitData) {
			// find x values from ymax and min
			// y=exp(offset)*exp(slope*x)
			// x=(log(y)-offset)/slope
			double temp_xmax = max((log(ymax)-offset)/slope, (log(ymin)-offset)/slope);
			double temp_xmin = min((log(ymax)-offset)/slope, (log(ymin)-offset)/slope);
			if (m_limitDataY) {
				setTo(temp_xmax);
				setFrom(temp_xmin);
			} else if (m_limitData) {
				setTo(max(temp_xmax, xmax));
				setFrom(min(temp_xmin, xmin));
			}
		}
		offset = exp(offset);
		sprintf(new_let, "let d%d = %0.10e*exp(%0.10e*x) FROM %0.10e TO %0.10e", fitds, offset, slope, getFrom(), getTo());
	} else if (log10fit) {
		//take log10 of y values
		for (vector<double>::iterator vdi = y.begin(); vdi != y.end(); ++vdi) {
			(*vdi) = log10(*vdi);
		}
		//least_square(&x,&y,&slope,&offset,&rsquared);
		auto result = boost::math::statistics::simple_ordinary_least_squares_with_R_squared(x, y);
		offset   = std::get<0>(result);
		slope    = std::get<1>(result);
		rsquared = std::get<2>(result);
		if (m_limitDataY || m_limitData) {
			// find x values from ymax and min
			// y=exp(offset)*exp(slope*x)
			// x=(log10(y)-offset)/slope
			double temp_xmax = max((log10(ymax)-offset)/slope, (log10(ymin)-offset)/slope);
			double temp_xmin = min((log10(ymax)-offset)/slope, (log10(ymin)-offset)/slope);
			if (m_limitDataY) {
				setTo(temp_xmax);
				setFrom(temp_xmin);
			} else if (m_limitData) {
				setTo(max(temp_xmax, xmax));
				setFrom(min(temp_xmin, xmin));
			}
		}
		offset = pow(10.0,offset);
		sprintf(new_let, "let d%d = %0.10e*10^(%0.10e*x) FROM %0.10e TO %0.10e", fitds, offset, slope, getFrom(), getTo());
	} else if (powxfit) {
		//take loge of both axis
		for (vector<double>::iterator vdi = y.begin(); vdi != y.end(); ++vdi) {
			(*vdi) = log(*vdi);
		}
		for (vector<double>::iterator vdi = x.begin(); vdi != x.end(); ++vdi) {
			(*vdi) = log(*vdi);
		}
		//least_square(&x, &y, &slope, &offset, &rsquared);
		auto result = boost::math::statistics::simple_ordinary_least_squares_with_R_squared(x, y);
		offset   = std::get<0>(result);
		slope    = std::get<1>(result);
		rsquared = std::get<2>(result);
		if (m_limitDataY || m_limitData) {
			// find x values from ymax and min
			// y=exp(offset)*x^(slope)
			// x = ( y / exp(offset) ) ^ (1/slope)
			double temp_xmax = max(pow(ymax/exp(offset),1.0/slope), pow(ymin/exp(offset),1.0/slope) );
			double temp_xmin = min(pow(ymax/exp(offset),1.0/slope), pow(ymin/exp(offset),1.0/slope) );
			if (m_limitDataY) {
				setTo(temp_xmax);
				setFrom(temp_xmin);
			} else if (m_limitData) {
				setTo(max(temp_xmax, xmax));
				setFrom(min(temp_xmin, xmin));
			}
		}
		offset = exp(offset);
		sprintf(new_let, "let d%d = %0.10e*x^(%0.10e) FROM %0.10e TO %0.10e", fitds, offset, slope, getFrom(), getTo());
	} else if (genfit) {
		GLEFitLS fitls;
		fitls.polish(m_fitFct);
		fitls.setXY(&x,&y);
		fitls.fit();
		fitls.testFit();
		initVars();
		initStep();
		addFunction()->polishX();
		addFunction(fitls.getFunction());
		doLet();
		if (m_eqStr != "") {
			string eqstr_v;
			str_to_uppercase(m_eqStr);
			fitls.toFunctionStr(m_format, &eqstr_v);
			var_findadd_set(m_eqStr.c_str(), eqstr_v);
		}
		if (m_rsq != "") {
			str_to_uppercase(m_rsq);
			var_findadd_set((char*)m_rsq.c_str(), fitls.getRSquare());
		}
		delete []new_let;
		return;
	}
	// add the variables -- they will be globals?
	if (m_varSlope != "") {
		str_to_uppercase(m_varSlope);
		var_findadd_set((char*)m_varSlope.c_str(), slope);
	}
	if (m_varIntercept != "") {
		str_to_uppercase(m_varIntercept);
		var_findadd_set((char*)m_varIntercept.c_str(), offset);
	}
	if (m_varR != "") {
		str_to_uppercase(m_varR);
		var_findadd_set((char*)m_varR.c_str(), rsquared);
	}
	string letFct(new_let);
	delete []new_let;
	GLELet* let = parseLet(letFct, getCodeLine());
	::doLet(let, m_FineTune);
	delete let;
}

void GLELet::parseHistogram(GLEParser* parser) {
	Tokenizer* tokens = parser->getTokens();
	string& token = tokens->next_token();
	m_nrBins = -1;
	m_HistDS = get_dataset_identifier(token, parser, true);
	while (tokens->has_more_tokens()) {
		string& token = tokens->next_token();
		if (str_i_equals(token, "FROM")) {
			setHasFrom(true);
			setFrom(parser->evalTokenToDouble());
		} else if (str_i_equals(token,"TO")) {
			setHasTo(true);
			setTo(parser->evalTokenToDouble());
		} else if (str_i_equals(token, "STEP")) {
			setHasStepOption(true);
			setStep(parser->evalTokenToDouble());
		} else if (str_i_equals(token, "BINS")) {
			m_nrBins = (int)floor(parser->evalTokenToDouble()+0.5);
		} else {
			stringstream errstr;
			errstr << "unknown token in 'let' expression: '" << token << "'";
			throw tokens->error(errstr.str());
		}
	}
}

void GLELet::doHistogram() {
	int bins = m_nrBins;
	GLEDataPairs histData(getDataset(m_HistDS));
	if (!hasFrom() || !hasTo()) {
		// Take from axis range if explicitly given
		GLEAxis* ax = &xx[GLE_AXIS_X];
		if (!hasFrom() && ax->getRange()->hasMin()) {
			setHasFrom(true);
			setFrom(ax->getMin());
		}
		if (!hasTo() && ax->getRange()->hasMax()) {
			setHasTo(true);
			setTo(ax->getMax());
		}
		// Take from data range
		GLERange range;
		for (unsigned int i = 0; i < histData.size(); i++) {
			range.updateRange(histData.getY(i), histData.getM(i));
		}
		roundrange(&range, false, false, 0.0);
		if (range.validNotEmpty()) {
			if (!hasFrom()) {
				setHasFrom(true);
				setFrom(range.getMin());
			}
			if (!hasTo()) {
				setHasTo(true);
				setTo(range.getMax());
			}
		}
	}
	if (bins == -1 && !hasSteps()) {
		bins = 10;
	}
	vector<double> from;
	vector<double> counts;
	if (bins != -1) {
		for (int i = 0; i < bins; i++) {
			from.push_back(getFrom() + (getTo()-getFrom())*i/bins);
			counts.push_back(0.0);
		}
		from.push_back(getTo());
	} else {
		int i = 0;
		double value = getFrom();
		while (value < getTo()) {
			from.push_back(value);
			counts.push_back(0.0);
			i++;
			// avoids rounding errors
			value = getFrom()+i*getStep();
		}
		from.push_back(value);
	}
	for (unsigned int i = 0; i < dp[m_HistDS]->np; i++) {
		if (!histData.getM(i)) {
			int found = -1;
			double yv = histData.getY(i);
			for (vector<double>::size_type j = 0; j < counts.size(); j++) {
				if (yv >= from[j] && yv < from[j+1]) {
					found = j;
					break;
				}
			}
			if (found != -1) counts[found]++;
			else if (yv == getTo()) counts[counts.size()-1]++;
		}
	}
	DataFill fill(false);
	for (int dim = GLE_DIM_X; dim <= GLE_DIM_Y; dim++) {
		DataFillDimension* dim_f = new DataFillDimension(NULL);
		fill.addDataDimension(dim_f);
	}
	for (vector<double>::size_type i = 0; i < from.size()-1; i++) {
		fill.addPoint((from[i]+from[i+1])/2.0, counts[i]);
	}
	int resds = getDataSet();
	dp[resds]->clearAll();
	fill.toDataset(dp[resds]);
}
