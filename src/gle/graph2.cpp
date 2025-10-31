/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://glx.sourceforge.net/>          *
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

#define GRAPHDEF extern

#include <set>
#include <memory>
#include "all.h"
#include "tokens/stokenizer.h"
#include "cutils.h"
#include "file_io.h"
#include "bitmap/img2ps.h"
#include "op_def.h"
#include "mem_limits.h"
#include "token.h"
#include "gle-interface/gle-interface.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "var.h"
#include "graph.h"
#include "begin.h"
#include "color.h"
//#include "leastsq.h"
#include "core.h"
#include "axis.h"
#include "gprint.h"
#include "key.h"
#include "glearray.h"
#include "keyword.h"
#include "run.h"
#include "sub.h"
#include "numberformat.h"
#include "gle-fitls.h"

#include <boost/math/statistics/linear_regression.hpp>
#include <memory>

using namespace std;

#define DEFAULT_STEPS 100

extern int g_nbar;
extern GLEAxis xx[];

extern vector<int> g_fcalls;
extern vector<int> g_funder;
extern GLEColorMap* g_colormap;

extern GLEGlobalSource* g_Source;
extern GLEGraphBlockData* g_graphBlockData;

void box3d(double x1, double y1, double x2, double y2,double x3d, double y3d, const GLERC<GLEColor>& sidecolor, const GLERC<GLEColor>& topcolor, int notop);
void var_clear_local(void);
void do_svg_smooth(double* , int);
void draw_errbar(double x, double y, double eup, double ewid, GLEDataSet* ds);
void draw_herrbar(double x, double y, double eup, double ewid, GLEDataSet* ds);
void do_draw_lines(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_draw_steps(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_draw_fsteps(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_draw_hist(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_draw_bar(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_draw_impulses(double* xt, double* yt, int* m, int npts, GLEDataSet* ds);
void do_dataset_key_entries();

bool hasDataset(int di) {
	return di > 0 && di <= ndata && dp[di] != NULL && !dp[di]->undefined();
}

GLEDataSet* getDataset(int di, const char* descr = 0) {
	if (hasDataset(di)) {
		return dp[di];
	} else {
		ostringstream err;
		if (descr != 0) {
			err << descr << " ";
		}
		err << "dataset d" << di << " not defined";
		g_throw_parser_error(err.str());
		return 0;
	}
}

double fnx(double value, GLEAxis* axis, GLERange* range) {
	double wmin = range->getMin();
	double wmax = range->getMax();
	if (axis->negate) {
		value = wmax - (value - wmin);
	}
	if (axis->log) {
		return (log10(value) - log10(wmin))/(log10(wmax) - log10(wmin)) * xlength + xbl;
	} else {
		return (value - wmin)/(wmax - wmin) * xlength + xbl;
	}
}

double fny(double value, GLEAxis* axis, GLERange* range) {
	double wmin = range->getMin();
	double wmax = range->getMax();
	if (axis->negate) {
		value = wmax - (value - wmin);
	}
	if (axis->log) {
		return (log10(value) - log10(wmin))/(log10(wmax) - log10(wmin)) * ylength + ybl;
	} else {
		return (((value - wmin)/(wmax - wmin)) * ylength + ybl);
	}
}

double fnx(double value, GLEDataSet* dataSet) {
	return fnx(value, dataSet->getAxis(GLE_DIM_X), dataSet->getDim(GLE_DIM_X)->getRange());
}

double fny(double value, GLEDataSet* dataSet) {
	return fny(value, dataSet->getAxis(GLE_DIM_Y), dataSet->getDim(GLE_DIM_Y)->getRange());
}

double fnx(double value, GLEAxis* axis) {
	return fnx(value, axis, axis->getRange());
}

double fny(double value, GLEAxis* axis) {
	return fny(value, axis, axis->getRange());
}

double fnxInv(double value, GLEAxis* axis, GLERange* range) {
	double wmin = range->getMin();
	double wmax = range->getMax();
	double result = 0.0;
	if (axis->log) {
		result = pow(10, (value - ybl) / xlength * (log10(wmax) - log10(wmin)) + log10(wmin));
	} else {
		result = (value - xbl) / xlength * (wmax - wmin) + wmin;
	}
	if (axis->negate) {
		return wmax + wmin - result;
	} else {
		return result;
	}
}

double fnyInv(double value, GLEAxis* axis, GLERange* range) {
	double wmin = range->getMin();
	double wmax = range->getMax();
	double result = 0.0;
	if (axis->log) {
		result = pow(10, (value - ybl) / xlength * (log10(wmax) - log10(wmin)) + log10(wmin));
	} else {
		result = (value - ybl) / ylength * (wmax - wmin) + wmin;
	}
	if (axis->negate) {
		return wmax + wmin - result;
	} else {
		return result;
	}
}

double fnxInv(double value, GLEAxis* axis) {
	return fnxInv(value, axis, axis->getRange());
}

double fnyInv(double value, GLEAxis* axis) {
	return fnyInv(value, axis, axis->getRange());
}

GLEPoint fnXY(double x, double y, GLEDataSet* dataSet) {
	return GLEPoint(fnx(x, dataSet), fny(y, dataSet));
}

double graph_xgraph(double v) {
	// cout << "graph_xmin: " << graph_xmin << " graph_xmax: " << graph_xmax << endl;
	// cout << "graph_x1: " << graph_x1 << " graph_x2: " << graph_x2 << endl;
	if (graph_xmax == graph_xmin) return 0.0;
	if (xx[GLE_AXIS_X].negate) {
		v = (graph_xmax - v) + graph_xmin;
	}
	if (xx[GLE_AXIS_X].log) {
		return graph_x1 + (log10(v)-log10(graph_xmin))/(log10(graph_xmax)-log10(graph_xmin))*(graph_x2-graph_x1);
	} else {
		return graph_x1 + (v-graph_xmin)/(graph_xmax-graph_xmin)*(graph_x2-graph_x1);
	}
}

double graph_ygraph(double v) {
	if (graph_ymax == graph_ymin) return 0.0;
	if (xx[GLE_AXIS_Y].negate) {
		v = (graph_ymax - v) + graph_ymin;
	}
	if (xx[GLE_AXIS_Y].log) {
		return graph_y1 + (log10(v)-log10(graph_ymin))/(log10(graph_ymax)-log10(graph_ymin))*(graph_y2-graph_y1);
	} else {
		return graph_y1 + (v-graph_ymin)/(graph_ymax-graph_ymin)*(graph_y2-graph_y1);
	}
}

double bar_get_min_interval_bars(int bar) {
	double min = GLE_INF;
	for (int idx = 0; idx < br[bar]->ngrp; idx++) {
		int di = br[bar]->to[idx];
		if (hasDataset(di)) {
			GLEDataPairs data(dp[di]);
			min = std::min(min, data.getMinXInterval());
		}
	}
	return min;
}

bool bar_has_type(bool horiz) {
	for (int bar = 1; bar <= g_nbar; bar++) {
		if (horiz == br[bar]->horiz) return true;
	}
	return false;
}

bar_struct::bar_struct() {
	ngrp = 0;
	width = 0.0;
	dist = 0.0;
	x3d = 0.0;
	y3d = 0.0;
	notop = false;
	horiz = false;
	layer = 0;
	for (int i = 0; i < 20; i++) {
		from[i] = 0;
		to[i] = 0;
		lwidth[i] = 0.0;
		lstyle[i][0] = 0;
		fill[i] = g_get_color_hex(GLE_COLOR_BLACK);
		color[i] = g_get_color_hex(GLE_COLOR_BLACK);
		side[i] = g_get_color_hex(GLE_COLOR_BLACK);
		top[i] = g_get_color_hex(GLE_COLOR_BLACK);
	}
}

GLEGraphPartBars::GLEGraphPartBars() {
}

GLEGraphPartBars::~GLEGraphPartBars() {
}

bool GLEGraphPartBars::shouldDraw(int /* b */) {
	return true;
}

std::set<int> GLEGraphPartBars::getLayers() {
	std::set<int> result;
	for (int b = 1; b <= g_nbar; b++) {
		if (shouldDraw(b)) {
			result.insert(br[b]->layer);
		}
	}
	return result;
}

void GLEGraphPartBars::drawLayerObject(int layer, GLEMemoryCell* object) {
	GLEClassInstance* classObj = getGLEClassInstance(object, g_graphBlockData->getGraphBlockBase()->getClassDefinitions()->getBar());
	if (classObj != 0) {
		int index = classObj->getArray()->getInt(0);
		if (shouldDraw(index) && br[index]->layer == layer) {
			g_gsave();
			drawBar(index);
			g_grestore();
		}
	}
}

void GLEGraphPartBars::drawBar(int b) {
	if (br[b] == 0 || br[b]->ngrp == 0) {
		ostringstream err;
		err << "bar set " << b << " not properly defined";
		g_throw_parser_error(err.str());
	}
	int nbars = br[b]->ngrp;
	double min_int = bar_get_min_interval_bars(b);
	if (br[b]->width == 0) br[b]->width = min_int/(nbars * 2);
	if (br[b]->dist == 0) br[b]->dist = br[b]->width * 1.4;
	for (int bi = 0; bi < nbars; bi++) {
		int df = br[b]->from[bi];
		int dt = br[b]->to[bi];
		// validate "to" dataset
		if (!hasDataset(dt)) {
			ostringstream err;
			err << "bar dataset d" << dt << " not defined";
			g_throw_parser_error(err.str());
		}
		// set bar style
		g_set_line_width(br[b]->lwidth[bi]);
		g_set_line_style(&br[b]->lstyle[bi][0]);
		if (br[b]->color[bi].isNull()) {
			br[b]->color[bi] = g_get_color_hex(GLE_COLOR_BLACK);
		}
		g_set_color(br[b]->color[bi]);
		g_set_fill(br[b]->fill[bi]);
		// compute sizes
		double bwid = br[b]->width;
		double bdis = br[b]->dist;
		double whole_wid = (nbars-1) * bdis + bwid;
		// get data
		GLEDataSet* toDataSet = dp[dt];
		toDataSet->checkRanges();
		GLEDataPairs toData(toDataSet);
		// check if has "from" dataset
		bool hasfrom = hasDataset(df);
		if (hasfrom) {
			if (dp[df]->np != dp[dt]->np) {
				ostringstream err;
				err << "bar 'from' dataset d" << df << " and 'to' dataset d" << dt << " ";
				err << "have a different number of points (" << dp[df]->np << " <> " << dp[dt]->np << ")";
				g_throw_parser_error(err.str());
			}
			GLEDataPairs fromData(dp[df]);
			for (unsigned int i = 0; i < dp[dt]->np; i++) {
				if (fromData.getM(i) != toData.getM(i)) {
					ostringstream err;
					err << "bar 'from' dataset d" << df << " and 'to' dataset d" << dt << " ";
					err << "have inconsistent missing values at point " << (i + 1);
					g_throw_parser_error(err.str());
				}
				if (!equals_rel(fromData.getX(i), toData.getX(i))) {
					ostringstream err;
					err << "bar 'from' dataset d" << df << " and 'to' dataset d" << dt << " ";
					err << "have different x-values at point " << (i + 1) << " (";
					err << fromData.getX(i) << " <> " << toData.getX(i) << ")";
					g_throw_parser_error(err.str());
				}
				if (!toData.getM(i)) {
					draw_bar(toData.getX(i) - whole_wid/2 + bi*bdis, fromData.getY(i), toData.getY(i), bwid, br[b], bi, toDataSet);
				}
			}
		} else {
			for (unsigned int i = 0; i < dp[dt]->np; i++) {
				if (!toData.getM(i)) {
					draw_bar(toData.getX(i) - whole_wid/2 + bi*bdis, 0.0, toData.getY(i), bwid, br[b], bi, toDataSet);
				}
			}

		}
	}
}

double graph_bar_pos(double xpos, int bar, int set) {
	if (set < 1 || set > g_nbar) {
		g_throw_parser_error("illegal bar set: ", set);
	}
	int nb = br[set]->ngrp;
	if (bar < 1 || bar > nb) {
		g_throw_parser_error("illegal bar number: ", bar);
	}
	double bwid = br[set]->width;
	double bdis = br[set]->dist;
	double whole_wid = (nb-1)*bdis+bwid;
	// cout << "bwid:  " << bwid << endl;
	// cout << "bdis:  " << bdis << endl;
	// cout << "nb:    " << nb << endl;
	// cout << "whole: " << whole_wid << endl;
	if (br[set]->horiz) {
		return graph_ygraph(xpos-whole_wid/2+(bar-1)*bdis+bwid/2);
	} else {
		return graph_xgraph(xpos-whole_wid/2+(bar-1)*bdis+bwid/2);
	}
}

void draw_bar(double x, double yf, double yt, double wd, bar_struct* barset, int di, GLEDataSet* toDataSet) {
	/* draw a bar, wd wide, centere at x , from yf, to yt */
	x = x + wd/2;
	double x1 = x - wd/2;
	double y1 = yf;
	double x2 = x + wd/2;
	double y2 = yt;
	double x3d = barset->x3d;
	double y3d = barset->y3d;
	GLERC<GLEColor> topcolor = barset->top[di];
	GLERC<GLEColor> sidecolor = barset->side[di];
	int notop = barset->notop;
	if (barset->horiz) {
		toDataSet->clip(&y1, &x1);
		toDataSet->clip(&y2, &x2);
		double bx1 = x1;
		double bx2 = x2;
		x1 = fnx(y1, toDataSet);  x2 = fnx(y2, toDataSet);
		y1 = fny(bx1, toDataSet); y2 = fny(bx2, toDataSet);
	} else {
		toDataSet->clip(&x1, &y1);
		toDataSet->clip(&x2, &y2);
		x1 = fnx(x1, toDataSet); x2 = fnx(x2, toDataSet);
		y1 = fny(y1, toDataSet); y2 = fny(y2, toDataSet);
	}
	if (x1 == x2 || y1 == y2) {
		// zero size bar
		return;
	}
	if (barset->style[di] == "") {
		if (x3d != 0.0) {
			box3d(x1, y1, x2, y2, x3d, y3d, sidecolor, topcolor, notop);
		}
		g_box_fill(x1, y1, x2, y2);
		g_box_stroke(x1, y1, x2, y2);
	} else {
		double args[6];
		args[0] = x1;
		args[1] = y1;
		args[2] = x2;
		args[3] = y2;
		args[4] = yt;
		args[5] = di;
		string name = string("BAR_") + barset->style[di];
		call_sub_byname(name, args, 6, "(used for defining bar style)");
	}
}

void box3d(double x1, double y1, double x2, double y2, double x3d, double y3d, const GLERC<GLEColor>& sidecolor, const GLERC<GLEColor>& topcolor, int notop) {
	/* assuming x3d is positive for the moment */
	double xx;
	if (x1>x2) { xx = x1; x1 = x2; x2 = xx;}
	if (y1>y2) { xx = y1; y1 = y2; y2 = xx;}
	x3d = x3d*(x2-x1);
	y3d = y3d*(x2-x1);
	if (x3d<0) {
		xx = x1; x1 = x2; x2 = xx;
	}
	g_gsave();
	g_set_path(true);
	g_set_line_join(1);	/* use rounded lines to avoid ucky peeks */
	g_newpath();
	g_move(x2,y1);
	g_line(x2+x3d,y1+y3d);
	g_line(x2+x3d,y2+y3d);
	g_line(x2,y2);
	g_line(x2,y1);
	if (!topcolor.isNull()) {
	   g_set_fill(sidecolor);
	   g_fill();
	}
	g_stroke();
	g_newpath();
	if (!notop) {	/* now draw top of bar  */
	 g_move(x2,y2);
	 g_line(x2+x3d,y2+y3d);
	 g_line(x1+x3d,y2+y3d);
	 g_line(x1,y2);
	 g_line(x2,y2);
	 if (!topcolor.isNull()) {
		g_set_fill(topcolor);
		g_fill();
	 }
	 g_stroke();
	}
	g_newpath();
	g_set_path(false);
	g_newpath();
	g_grestore();
}

void gr_thrownomiss(void) {
	for (int i=1;i<=ndata;i++) {
		if (dp[i]!=NULL) if (dp[i]->nomiss && dp[i]->np>0) {
			gr_nomiss(i);	/* throw away missing points */
		}
	}
}

void gr_nomiss(int i) {
	if (!hasDataset(i)) {
		return;
	}
	unsigned int nbPoints = 0;
	GLEDataSet* dataSet = dp[i];
	dataSet->validateDimensions();
	GLEArrayImpl* dimensions = dataSet->getData();
	vector<int> miss(dataSet->getMissingValues());
	for (unsigned int dim = 0; dim < dimensions->size(); dim++) {
		GLEDataObject* data = dimensions->getObject(dim);
		if (data != 0 && data->getType() == GLEObjectTypeArray) {
			GLEArrayImpl* array = static_cast<GLEArrayImpl*>(data);
			unsigned int target = 0;
			for (unsigned int j = 0; j < array->size(); j++) {
				if (!miss[j]) {
					array->set(target++, array->get(j));
				}
			}
			array->resize(target);
			nbPoints = max(nbPoints, target);
		}
	}
	dataSet->np = nbPoints;
}

void reset_axis_ranges() {
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		xx[axis].initRange();
	}
}

bool should_autorange_based_on_lets() {
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		GLEAxis* ax = &xx[axis];
		if (!ax->getRange()->hasBoth()) {
			if (ax->getNbDimensions() > 0) return true;
			if (g_colormap != NULL && g_colormap->getData() != NULL) return true;
		}
	}
	return false;
}

void window_set(bool showError) {
	// Called twice:
	// - Before processing "let" commands - then "showError = false"
	// - After processing "let" commands - then "showError = true"
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		bool is_horiz = axis_horizontal(axis);
		bool has_bar = bar_has_type(is_horiz);
		xx[axis].roundDataRange(has_bar, !is_horiz);
	}
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		bool is_horiz = axis_horizontal(axis);
		bool has_bar = bar_has_type(is_horiz);
		int ortho = is_horiz ? GLE_AXIS_Y : GLE_AXIS_X;
		int clone = is_horiz ? GLE_AXIS_X : GLE_AXIS_Y;
		xx[axis].makeUpRange(&xx[clone], &xx[ortho], has_bar, !is_horiz);
		if (showError && xx[axis].getRange()->invalidOrEmpty()) {
			stringstream err;
			err << "illegal range for " << axis_type_name(axis) << ": ";
			xx[axis].getRange()->printRange(err);
			g_throw_parser_error(err.str());
		}
	}
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		GLEAxis* ax = &xx[axis];
		for (int dim = 0; dim < ax->getNbDimensions(); dim++) {
			ax->getDim(dim)->getRange()->copyIfNotSet(ax->getRange());
		}
	}
}

void store_window_bounds_to_vars() {
	var_findadd_set("XGMIN", xx[GLE_AXIS_X].getMin());
	var_findadd_set("XGMAX", xx[GLE_AXIS_X].getMax());
	var_findadd_set("YGMIN", xx[GLE_AXIS_Y].getMin());
	var_findadd_set("YGMAX", xx[GLE_AXIS_Y].getMax());
	var_findadd_set("X2GMIN", xx[GLE_AXIS_X2].getMin());
	var_findadd_set("X2GMAX", xx[GLE_AXIS_X2].getMax());
	var_findadd_set("Y2GMIN", xx[GLE_AXIS_Y2].getMin());
	var_findadd_set("Y2GMAX", xx[GLE_AXIS_Y2].getMax());
}

void fitbez(GLEDataPairs* data, bool multi);

GLERC<GLEDataPairs> transform_data(GLEDataSet* ds, bool isline = true) {
	/* isline = false for marker plots */
	GLERC<GLEDataPairs> data = new GLEDataPairs();
	data->copy(ds);
	data->noNaN();
	bool xlog = xx[ds->getDim(GLE_DIM_X)->getAxis()].log;
	bool ylog = xx[ds->getDim(GLE_DIM_Y)->getAxis()].log;
	data->noLogZero(xlog, ylog);
	if (ds->deresolve > 1) {
		data->noMissing();
		if (data->size() >= 1) {
			int pos = 0;
			if (!ds->deresolve_avg) {
				/* deresolve by just skipping some of the original points */
				for (unsigned int j = 0; j < data->size(); j += ds->deresolve) {
					data->set(pos++, data->getX(j), data->getY(j), 0);
				}
				data->set(pos++, data->getX(data->size()-1), data->getY(data->size()-1), 0);
			} else {
				/* deresolve by averaging points */
				/* d1 deresolve 5 average plots points something like */
				/* X = (X[5]+X[0])/2, Y = mean(Y[0]...Y[5]), etc. */
				if (isline) data->set(pos++, data->getX(0), data->getY(0), 0);
				for (unsigned int i = 0; (i+1)*ds->deresolve-1 < data->size(); i++) {
					double yavg = 0.0;
					for (int j = 0; j < ds->deresolve; j++) {
						yavg += data->getY(i*ds->deresolve + j);
					}
					yavg /= ds->deresolve;
					double x = (data->getX(i*ds->deresolve)+
					            data->getX((i+1)*ds->deresolve-1))/2.0;
					data->set(pos++, x, yavg, 0);
				}
				if (isline) data->set(pos++, data->getX(data->size()-1), data->getY(data->size()-1), 0);
			}
			data->resize(pos);
		}
	}
	if (ds->smooth && isline) {
		data->noMissing();
		data->transformLog(xlog, ylog);
		fitbez(data.get(), ds->smoothm);
		data->untransformLog(xlog, ylog);
		/* maybe use real bezier curves for drawing a smoothed graph? */
		/* instead of a huge number of line segments? */
	}
	if (ds->svg_smooth) {
		data->noMissing();
		if (data->size() > 3) {
			if (ds->svg_iter == 0) ds->svg_iter = 1;
			for(int j = 0; j < ds->svg_iter; j++) {
				do_svg_smooth(data->getY(), data->size());
			}
		}
	}
	return data;
}

GLEGraphPartFills::GLEGraphPartFills() {
}

GLEGraphPartFills::~GLEGraphPartFills() {
}

bool GLEGraphPartFills::shouldDraw(int n) {
	return fd[n]->type != 0;
}

std::set<int> GLEGraphPartFills::getLayers() {
	std::set<int> result;
	for (int n = 1; n <= nfd; n++) {
		if (shouldDraw(n)) {
			result.insert(fd[n]->layer);
		}
	}
	return result;
}

void GLEGraphPartFills::drawLayerObject(int layer, GLEMemoryCell* object) {
	GLEClassInstance* classObj = getGLEClassInstance(object, g_graphBlockData->getGraphBlockBase()->getClassDefinitions()->getFill());
	if (classObj != 0) {
		int index = classObj->getArray()->getInt(0);
		if (shouldDraw(index) && fd[index]->layer == layer) {
			g_gsave();
			int origMaxVector = setMaxPSVector(-1);
			drawFill(index);
			setMaxPSVector(origMaxVector);
			g_grestore();
		}
	}
}

void GLEGraphPartFills::drawFill(int n) {
	struct fill_data *ff = fd[n];
	int dn = ff->da;
	if (!hasDataset(dn)) {
		gprint("no data in fill dataset");
		return;
	}
	GLEDataSet* daDS = dp[dn];
	daDS->checkRanges();
	/* set clipping region */
	daDS->clip(&ff->xmin, &ff->ymin);
	daDS->clip(&ff->xmax, &ff->ymax);
	g_beginclip();		/* saves current clipping */
	g_set_path(true);
	g_newpath();
	GLERectangle clipBox;
	clipBox.initRange();
	GLEPoint pMin(fnXY(ff->xmin, ff->ymin, daDS));
	GLEPoint pMax(fnXY(ff->xmax, ff->ymax, daDS));
	clipBox.updateRange(&pMin);
	clipBox.updateRange(&pMax);
	g_box_stroke(&clipBox);
	g_clip();		/* sets current path to be clipping */
	vector<double> fvec;
	GLERC<GLEDataPairs> data1 = transform_data(dp[dn]);
	GLERC<GLEDataPairs> data2;
	data1->noMissing();
	if (data1->size() < 1) {
		return;
	}
	double *xt = data1->getX();
	double *yt = data1->getY();
	double x2 = 0.0, y2 = 0.0;
	double ymx = ff->ymax;
	switch(ff->type) {
		case 1: /* x1,d1 */
			ymx = ff->ymin;
		case 2: /* d1,x2 */
			fill_vec(*xt, ymx, *xt, *yt, &fvec);
			for (unsigned int i = 0; i < data1->size()-1; i++, xt++, yt++) {
				fill_vec(*xt, *yt, *(xt+1), *(yt+1), &fvec);
			}
			fill_vec(*xt, *yt, *xt, ymx, &fvec);
			fill_vec(*xt, ymx, data1->getX(0), ymx, &fvec);
			break;
		case 3: /* d1,d2 */
			for (unsigned int i = 0; i < data1->size()-1; i++, xt++, yt++) {
				fill_vec(*xt, *yt, *(xt+1), *(yt+1), &fvec);
				x2 = *(xt+1); y2 = *(yt+1);
			}
			data2 = transform_data(dp[ff->db]);
			data2->noMissing();
			if (data2->size() < 1) {
				break;
			}
			xt = data2->getX() + data2->size() - 1;
			yt = data2->getY() + data2->size() - 1;
			fill_vec(x2, y2, *xt, *yt, &fvec);
			for (unsigned int i = 0; i < data2->size()-1; i++, xt--, yt--) {
				fill_vec(*xt, *yt, *(xt-1), *(yt-1), &fvec);
			}
			fill_vec(*xt, *yt, data1->getX(0), data1->getY(0), &fvec);
			break;
		case 4: /* d1 */
			for (unsigned int i = 0; i < data1->size()-1; i++, xt++, yt++) {
				fill_vec(*xt, *yt, *(xt+1), *(yt+1), &fvec);
			}
			fill_vec(*xt, *yt, data1->getX(0), data1->getY(0), &fvec);
			break;
	}
	/* Paint the region defined */
	g_set_fill(ff->color);
	g_newpath();
	if (fvec.size() >= 4) {
		g_move(fnXY(fvec[0], fvec[1], daDS));
		double lastx = fvec[0];
		double lasty = fvec[1];
		for (int i = 0; i <= (int)fvec.size()-4; i += 4) {
			if (lastx != fvec[i] || lasty != fvec[i+1])  {
				g_closepath();
				g_move(fnXY(fvec[i], fvec[i+1], daDS));
			}
			g_line(fnXY(fvec[i+2], fvec[i+3], daDS));
			lastx = fvec[i+2]; lasty = fvec[i+3];
		}
	}
	g_closepath();
	g_fill();
	g_set_path(false);
	g_endclip();
}

void fill_vec(double x1, double y1, double x2, double y2, vector<double>* vec) {
	vec->push_back(x1);
	vec->push_back(y1);
	vec->push_back(x2);
	vec->push_back(y2);
}

void setupdown(const string& s, bool *enable, int *dataset, bool *percentage, double *upval) {
	*dataset = 0;
	*enable = true;
	*percentage = false;
	*upval = 0.0;
	unsigned int len = s.size();
	if (len == 0) {
		*enable = false;
	} else if (len >= 1 && toupper(s[0]) == 'D') {
		// dataset identifier
		*dataset = get_dataset_identifier(s.c_str(), false);
	} else if (str_i_str(s, "%") != -1) {
		// percentage
		*percentage = true;
		*upval = atof(s.c_str());
	} else {
		// absolute value
		*upval = atof(s.c_str());
	}
}

/*------------------------------------------------------------------*/
/* 	d3 err .1			*/
/* 	d3 err 10%			*/
/* 	d3 errup 10% errdown d2		*/
/* 	d3 err d1 errwidth .2		*/

void draw_errbar(const GLELineSegment& segment, double ewid, GLEDataSet* ds) {
	if (ds->contains(segment.getP1())) {
		double x = segment.getP1().getX();
		double y = segment.getP1().getY();
		double y2 = segment.getP2().getY();
		draw_vec(x, y, x, y2, ds);
		if (ds->contains(x, y2)) {
			g_move(fnx(x, ds) - ewid/2, fny(y2, ds));
			g_line(fnx(x, ds) + ewid/2, fny(y2, ds));
		}
	}
}

void draw_herrbar(const GLELineSegment& segment, double ewid, GLEDataSet* ds) {
	if (ds->contains(segment.getP1())) {
		double x = segment.getP1().getX();
		double y = segment.getP1().getY();
		double x2 = segment.getP2().getX();
		draw_vec(x, y, x2, y, ds);
		if (ds->contains(x2, y)) {
			g_move(fnx(x2, ds), fny(y, ds) - ewid/2);
			g_line(fnx(x2, ds), fny(y, ds) + ewid/2);
		}
	}
}

void helperGetErrorBarData(GLEDataSet* dataSet, const GLEDataPairs& pairs, int i, double error, bool isHoriz, std::vector<GLELineSegment>* result) {
	if (isHoriz) {
		double x2 = pairs.getX(i) - error;
		if (dataSet->getAxis(GLE_DIM_X)->log && x2 <= 0.0) {
			x2 = 0.0;
		}
		result->push_back(GLELineSegment(pairs.getX(i), pairs.getY(i), x2, pairs.getY(i)));
	} else {
		double y2 = pairs.getY(i) + error;
		if (dataSet->getAxis(GLE_DIM_Y)->log && y2 <= 0.0) {
			y2 = 0.0;
		}
		result->push_back(GLELineSegment(pairs.getX(i), pairs.getY(i), pairs.getX(i), y2));
	}
}

std::vector<GLELineSegment> getErrorBarData(GLEDataSet* dataSet, const string& errdescr, bool isUp, bool isHoriz, const char* descr) {
	int errID;
	double value;
	bool enable, percentage;
	setupdown(errdescr, &enable, &errID, &percentage, &value);
	GLEDataPairs pairs(dataSet);
	GLEDataPairs errPairs;
	if (errID != 0) {
		GLEDataSet* errDataset = getDataset(errID, descr);
		errPairs.copyDimension(errDataset, 1);
		errDataset->validateNbPoints(dataSet->np, descr);
	}
	std::vector<GLELineSegment> result;
	vector<double>* errDimension = pairs.getDimension(isHoriz ? 0 : 1);
	for (unsigned int i = 0; i < dataSet->np; i++) {
		int miss = false;
		double error = 0.0;
		if (errID != 0) {
			miss = errPairs.getM(i);
			error = errPairs.getY(i);
		} else {
			error = value;
			if (percentage) error *= fabs(errDimension->at(i)) / 100.0;
		}
		if (enable && !pairs.getM(i) && !miss) {
			if (!isUp) {
				error = error * -1;
			}
			helperGetErrorBarData(dataSet, pairs, i, error, isHoriz, &result);
		}
	}
	return result;
}

void draw_err(GLEDataSet* dataSet, const string& errdescr, bool isUp, bool isHoriz, double errwd, const char* descr) {
	dataSet->checkRanges();
	if (errwd == 0) {
		double hei;
		g_get_hei(&hei);
		errwd = hei/3;
	}
	g_set_color(dataSet->mcolor.isNull() ? dataSet->color : dataSet->mcolor); // error bars should have the same color as the markers
	g_set_line_width(dataSet->lwidth);
	std::vector<GLELineSegment> errData(getErrorBarData(dataSet, errdescr, isUp, isHoriz, descr));
	for (unsigned int i = 0; i < errData.size(); i++) {
		if (isHoriz) {
			draw_herrbar(errData[i], errwd, dataSet);
		} else {
			draw_errbar(errData[i], errwd, dataSet);
		}
	}
}

void doMinMaxScaleErrorBars(GLEDataSet* dataSet, int dimensionIndex, const string& errdescr, bool isUp, bool isHoriz, const char* descr, GLERange* range)
{
	std::vector<GLELineSegment> errData(getErrorBarData(dataSet, errdescr, isUp, isHoriz, descr));
	for (unsigned int i = 0; i < errData.size(); i++) {
		GLELineSegment segment(errData[i]);
		if (dimensionIndex == 0) {
			range->updateRange(segment.getP1().getX(), false);
			range->updateRange(segment.getP2().getX(), false);
		} else {
			range->updateRange(segment.getP1().getY(), false);
			range->updateRange(segment.getP2().getY(), false);
		}
	}
}

void doMinMaxScaleErrorBars(GLEDataSet* dataSet, int dimensionIndex, GLERange* range)
{
	if (dataSet->errup.size() != 0) {
		doMinMaxScaleErrorBars(dataSet, dimensionIndex, dataSet->errup, true, false, "error up", range);
	}
	if (dataSet->errdown.size() != 0) {
		doMinMaxScaleErrorBars(dataSet, dimensionIndex, dataSet->errdown, false, false, "error down", range);
	}
	if (dataSet->herrup.size() != 0) {
		doMinMaxScaleErrorBars(dataSet, dimensionIndex, dataSet->herrup, true, true, "error right", range);
	}
	if (dataSet->herrdown.size() != 0) {
		doMinMaxScaleErrorBars(dataSet, dimensionIndex, dataSet->herrdown, false, true, "error left", range);
	}
}

GLEGraphPartErrorBars::GLEGraphPartErrorBars() {
}

GLEGraphPartErrorBars::~GLEGraphPartErrorBars() {
}

bool GLEGraphPartErrorBars::shouldDraw(int dn) {
	if (hasDataset(dn)) {
		GLEDataSet* dataSet = dp[dn];
		if (!dataSet->errup.empty() || !dataSet->errdown.empty() || !dataSet->herrup.empty() || !dataSet->herrdown.empty()) {
			return true;
		}
	}
	return false;
}

std::set<int> GLEGraphPartErrorBars::getLayers() {
	std::set<int> result;
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			result.insert(dp[dn]->layer_error);
		}
	}
	return result;
}

void GLEGraphPartErrorBars::addToOrder(GLEGraphDataSetOrder* order) {
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			order->addDataSet(dn);
		}
	}
}

void GLEGraphPartErrorBars::drawLayerObject(int layer, GLEMemoryCell* object) {
	if (object->Type == GLE_MC_INT) {
		int dn = object->Entry.IntVal;
		if (shouldDraw(dn) && dp[dn]->layer_error == layer) {
			g_gsave();
			drawErrorBars(dn);
			g_grestore();
		}
	}
}

void GLEGraphPartErrorBars::drawErrorBars(int dn) {
	GLEDataSet* dataSet = dp[dn];
	if (dataSet->errup.size() != 0) {
		draw_err(dataSet, dataSet->errup, true, false, dataSet->errwidth, "error up");
	}
	if (dataSet->errdown.size() != 0) {
		draw_err(dataSet, dataSet->errdown, false, false, dataSet->errwidth, "error down");
	}
	if (dataSet->herrup.size() != 0) {
		draw_err(dataSet, dataSet->herrup, true, true, dataSet->herrwidth, "error right");
	}
	if (dataSet->herrdown.size() != 0) {
		draw_err(dataSet, dataSet->herrdown, false, true, dataSet->herrwidth, "error left");
	}
}

GLEGraphPartLines::GLEGraphPartLines() {
}

GLEGraphPartLines::~GLEGraphPartLines() {
}

bool GLEGraphPartLines::shouldDraw(int dn) {
	return hasDataset(dn) && (dp[dn]->line || dp[dn]->lstyle[0] != 0);
}

std::set<int> GLEGraphPartLines::getLayers() {
	std::set<int> result;
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			result.insert(dp[dn]->layer_line);
		}
	}
	return result;
}

void GLEGraphPartLines::addToOrder(GLEGraphDataSetOrder* order) {
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			order->addDataSet(dn);
		}
	}
}

void GLEGraphPartLines::drawLayerObject(int layer, GLEMemoryCell* object) {
	if (object->Type == GLE_MC_INT) {
		int dn = object->Entry.IntVal;
		if (shouldDraw(dn) && dp[dn]->layer_line == layer) {
			g_gsave();
			drawLine(dn);
			g_grestore();
		}
	}
}

void GLEGraphPartLines::drawLine(int dn) {
	GLEDataSet* dataSet = dp[dn];
	dataSet->checkRanges();
	GLERC<GLEDataPairs> data = transform_data(dataSet);
	g_set_line_style(dp[dn]->lstyle);
	g_set_color(dp[dn]->color);
	g_set_line_width(dp[dn]->lwidth); /* does nothing if arg = 0 */
	last_vecx = GLE_INF;
	last_vecy = GLE_INF;
	switch (dp[dn]->line_mode) {
		case GLE_GRAPH_LM_PLAIN:
			do_draw_lines(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
		case GLE_GRAPH_LM_STEPS:
			do_draw_steps(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
		case GLE_GRAPH_LM_FSTEPS:
			do_draw_fsteps(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
		case GLE_GRAPH_LM_HIST:
			do_draw_hist(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
		case GLE_GRAPH_LM_IMPULSES:
			do_draw_impulses(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
		case GLE_GRAPH_LM_BAR:
			do_draw_bar(data->getX(), data->getY(), data->getM(), data->size(), dataSet);
			break;
	}
}

void do_draw_lines(double* xt, double* yt, int* m, int npnts, GLEDataSet* ds) {
	for (int i = 0; i < (npnts-1); i++, m++, xt++, yt++) {
		if (!*m && !*(m+1)) draw_vec(*xt, *yt, *(xt+1), *(yt+1), ds);
	}
}

void do_draw_steps(double* xt, double* yt, int* m, int npnts, GLEDataSet* ds) {
	// STEPS: (x1,y1) to (x2,y1) and the second from (x2,y1) to (x2,y2). See demo.
	for (int i = 0; i < (npnts-1); i++, m++, xt++, yt++) {
		if (!*m && !*(m+1)) {
			draw_vec(*xt, *yt, *(xt+1), *yt, ds);
			draw_vec(*(xt+1), *yt, *(xt+1), *(yt+1), ds);
		}
	}
}

void do_draw_fsteps(double* xt, double* yt, int* m, int npnts, GLEDataSet* ds) {
	// FSTEPS: (x1,y1) to (x1,y2) and the second from (x1,y2) to (x2,y2)
	for (int i = 0; i < (npnts-1); i++, m++, xt++, yt++) {
		if (!*m && !*(m+1)) {
			draw_vec(*xt, *yt, *xt, *(yt+1), ds);
			draw_vec(*xt, *(yt+1), *(xt+1), *(yt+1), ds);
		}
	}
}

void do_draw_hist(double* xt, double* yt, int* m, int npnts, GLEDataSet* ds) {
	double hist_prev_x = 0, hist_prev_y = 0, hist_x1, hist_x2;
	bool hist_has_prev = false;
	for (int i = 0; i < npnts; i++, m++, xt++, yt++) {
		if (!*m) {
			bool hist_valid = true;
			if (i < npnts-1 && !*(m+1)) {
				hist_x2 = (*(xt+1) + (*xt))/2;
				if (hist_has_prev) {
					hist_x1 = (hist_prev_x + (*xt))/2;
				} else {
					hist_x1 = 2*(*xt) - hist_x2;
				}
			} else {
				if (hist_has_prev) {
					hist_x1 = (hist_prev_x + (*xt))/2;
					hist_x2 = 2*(*xt) - hist_x1;
				} else {
					hist_valid = false;
				}
			}
			if (hist_valid) {
				if (hist_has_prev) {
					draw_vec(hist_x1, hist_prev_y, hist_x1, *yt, ds);
				}
				draw_vec(hist_x1, *yt, hist_x2, *yt, ds);
			}
			hist_has_prev = true;
			hist_prev_x = *xt; hist_prev_y = *yt;
		} else {
			hist_has_prev = false;
		}
	}
}

double impulsesOrig(GLEDataSet* ds) {
	GLERange* range = ds->getDim(GLE_DIM_Y)->getRange();
	if (range->getMin() > 0.0) return range->getMin();
	if (range->getMax() < 0.0) return range->getMax();
	return 0.0;
}

void do_draw_impulses(double* xt, double* yt, int* m, int npnts, GLEDataSet* ds) {
	double impulses_orig = impulsesOrig(ds);
	for (int i = 0; i < npnts; i++, m++, xt++, yt++) {
		if (!*m) draw_vec(*xt, impulses_orig, *xt, *yt, ds);
	}
}

void do_draw_bar(double* xt, double* yt, int* m, int npts, GLEDataSet* ds) {
	do_draw_hist(xt, yt, m, npts, ds);
	double hist_prev_x = 0, hist_prev_y = 0, hist_x1, hist_x2;
	bool hist_has_prev = false;
	double impulses_orig = impulsesOrig(ds);
	for (int i = 0; i < npts; i++, m++, xt++, yt++) {
		if (!*m) {
			bool hist_valid = true;
			if (i < npts-1 && !*(m+1)) {
				hist_x2 = (*(xt+1) + (*xt))/2;
				if (hist_has_prev) {
					hist_x1 = (hist_prev_x + (*xt))/2;
				} else {
					hist_x1 = 2*(*xt) - hist_x2;
				}
			} else {
				if (hist_has_prev) {
					hist_x1 = (hist_prev_x + (*xt))/2;
					hist_x2 = 2*(*xt) - hist_x1;
				} else {
					hist_valid = false;
				}
			}
			if (hist_valid && hist_has_prev) {
				// g_set_color(GLE_COLOR_BLACK);
				double yval = hist_prev_y;
				if (fabs(hist_prev_y-impulses_orig) > fabs(*yt-impulses_orig)) {
					yval = *yt;
				}
				draw_vec(hist_x1, impulses_orig, hist_x1, yval, ds);
			}
			hist_has_prev = true;
			hist_prev_x = *xt; hist_prev_y = *yt;
		} else {
			hist_has_prev = false;
		}
	}

}

void do_svg_smooth(double* xold, int size) {
 /* for now this is a quadratic or cubic and 7 point smoothing only */
	int i;
	double* xnew;
	xnew = (double*) calloc(size,sizeof(double));
	for (i =0 ;i <= size ;i++ ) {
		if (i == 0 || i == 1 || i == size-2 ||  i == size-1) {
			/* can't do it*/
			xnew[i]=xold[i];
		} else if (i == 2 || i == size-3) {
			/* do five point */
			xnew[i] = (-3*xold[i-2]+12*xold[i-1]+17*xold[i]+12*xold[i+1]-3*xold[i+2])/35;
		} else if (i == 3 || i == size - 4) {
			/* do seven point */
			xnew[i] = (-2*xold[i-3]+3*xold[i-2]+6*xold[i-1]+7*xold[i]+6*xold[i+1]+3*xold[i+2]-2*xold[i+3])/21;
		} else if (i >= 4 && i <= size - 5) {
			/* do nine point */
			xnew[i] = (-21*xold[i-4]+14*xold[i-3]+39*xold[i-2]+54*xold[i-1]+59*xold[i]+54*xold[i+1]+39*xold[i+2]+14*xold[i+3]-21*xold[i+4])/231;
		}
	}
	memcpy(xold,xnew,size*sizeof(double));
	free(xnew);
}

void g_move_safe(const GLEPoint& p) {
	if (gle_isnan(p.getX())) return;
	if (gle_isnan(p.getY())) return;
	g_move(p);
}

void g_line_safe(const GLEPoint& p) {
	if (gle_isnan(p.getX())) return;
	if (gle_isnan(p.getY())) return;
	g_line(p);
}

void draw_vec(double x1, double y1, double x2, double y2, GLEDataSet* ds) {
	if (!ds->contains(x1, y1) || !ds->contains(x2, y2)) {
		/* ok one or both are outside our box */
		GLERange* xrange = ds->getDim(GLE_DIM_X)->getRange();
		GLERange* yrange = ds->getDim(GLE_DIM_Y)->getRange();
		double txmin = xrange->getMin(), tymin = yrange->getMin();
		double txmax = xrange->getMax(), tymax = yrange->getMax();
		if (ds->getAxis(GLE_DIM_X)->log) {
			x1 = log10(x1); x2 = log10(x2);
			txmin = log10(txmin); txmax = log10(txmax);
		}
		if (ds->getAxis(GLE_DIM_Y)->log) {
			y1 = log10(y1); y2 = log10(y2);
			tymin = log10(tymin); tymax = log10(tymax);
		}
		bool invis = gclip(&x1, &y1, &x2, &y2, txmin, tymin, txmax, tymax);
		if (invis) return;
		if (ds->getAxis(GLE_DIM_X)->log) {
			x1 = pow(10.0,x1); x2 = pow(10.0,x2);
		}
		if (ds->getAxis(GLE_DIM_Y)->log) {
			y1 = pow(10.0,y1); y2 = pow(10.0,y2);
		}
	}
	if (x1 != last_vecx || y1 != last_vecy) {
		g_move_safe(fnXY(x1, y1, ds));
	}
	g_line_safe(fnXY(x2, y2, ds));
	last_vecx = x2;
	last_vecy = y2;
}

GLEGraphPartMarkers::GLEGraphPartMarkers() {
}

GLEGraphPartMarkers::~GLEGraphPartMarkers() {
}

bool GLEGraphPartMarkers::shouldDraw(int dn) {
	return hasDataset(dn) && dp[dn]->marker != 0;
}

std::set<int> GLEGraphPartMarkers::getLayers() {
	std::set<int> result;
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			result.insert(dp[dn]->layer_marker);
		}
	}
	return result;
}

void GLEGraphPartMarkers::addToOrder(GLEGraphDataSetOrder* order) {
	for (int dn = 1; dn <= ndata; dn++) {
		if (shouldDraw(dn)) {
			order->addDataSet(dn);
		}
	}
}

void GLEGraphPartMarkers::drawLayerObject(int layer, GLEMemoryCell* object) {
	if (object->Type == GLE_MC_INT) {
		int dn = object->Entry.IntVal;
		if (shouldDraw(dn) && dp[dn]->layer_marker == layer) {
			g_gsave();
			drawMarkers(dn);
			g_grestore();
		}
	}
}

void GLEGraphPartMarkers::drawMarkers(int dn) {
	GLEDataSet* dataSet = dp[dn];
	dataSet->checkRanges();
	GLERC<GLEDataPairs> data = transform_data(dataSet, false);
	g_set_color(dataSet->mcolor.isNull() ? dataSet->color : dataSet->mcolor);
	g_set_line_width(dataSet->lwidth);
	g_set_line_style("1");
	double msize = dataSet->msize;
	if (msize == 0) msize = g_fontsz;
	if (dataSet->mscale != 0) msize = msize * dataSet->mscale;
	double mdist = dataSet->mdist;
	if (mdist == 0) {
		GLEDataPairs mdataPairs;
		if (dataSet->mdata != 0) {
			GLEDataSet* mdataSet = getDataset(dataSet->mdata, "marker mdata");
			mdataPairs.copyDimension(mdataSet, 1);
			mdataSet->validateNbPoints(data->size(), "marker mdata");
		}
		for (unsigned int i = 0; i < data->size(); i++) {
			if (!data->getM(i)) {
				double dval = 1.0;
				if (dataSet->mdata != 0) {
					dval = mdataPairs.getY(i);
				}
				draw_mark(data->getX(i), data->getY(i), dataSet->marker, msize, dval, dataSet);
			}
		}
	} else {
		data->noMissing();
		double* xt = data->getX();
		double* yt = data->getY();
		if (data->size() >= 1) {
			double len = 0.0;
			double x0 = fnx(xt[0], dataSet);
			double y0 = fny(yt[0], dataSet);
			for (unsigned int i = 1; i < data->size(); i++) {
				double x = fnx(xt[i], dataSet);
				double y = fny(yt[i], dataSet);
				len += sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
				x0 = x; y0 = y;
			}
			x0 = fnx(xt[0], dataSet);
			y0 = fny(yt[0], dataSet);
			double prev_dist = mdist - fmod(len, mdist)/2.0;
			for (unsigned int i = 1; i < data->size(); i++) {
				double x = fnx(xt[i], dataSet);
				double y = fny(yt[i], dataSet);
				double dist = sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
				while (prev_dist + dist > mdist) {
					double offs = mdist - prev_dist;
					double xp = (offs*x + (dist-offs)*x0)/dist;
					double yp = (offs*y + (dist-offs)*y0)/dist;
					if (xp >= xbl && xp <= xbl+xlength && yp >= ybl && yp <= ybl+ylength) {
						g_move(xp, yp);
						g_marker2(dataSet->marker,msize,1);
					}
					x0 = xp; y0 = yp;
					dist = sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
					prev_dist = 0.0;
				}
				prev_dist += dist;
				x0 = x; y0 = y;
			}
		}
	}
}

void draw_mark(double x, double y, int mrk, double msize, double dval, GLEDataSet* ds) {
	if (ds->contains(x, y)) {
		g_move(fnXY(x, y, ds));
		g_marker2(mrk, msize, dval);
	}
}

void gclip_simple(double *v, double vmin, double vmax) {
	if (*v < vmin) { *v = vmin; }
	if (*v > vmax) { *v = vmax; }
}

bool gclip(double *x1, double *y1, double *x2, double *y2,
	   double xmin, double ymin, double xmax, double ymax) {
	// Take care of infinite values
	if (gle_isinf(*y1)) {
		gclip_simple(y1, ymin, ymax);
		if (gle_isinf(*y2)) {
			if (*y1 == *y2) return true;
			else gclip_simple(y2, ymin, ymax);
		} else {
			*x1 = *x2;
		}
	} else if (gle_isinf(*y2)) {
		gclip_simple(y2, ymin, ymax);
		*x2 = *x1;
	}
	if (gle_isinf(*x1)) {
		gclip_simple(x1, xmin, xmax);
		if (gle_isinf(*x2)) {
			if (*x1 == *x2) return true;
			else gclip_simple(x2, xmin, xmax);
		} else {
			*y1 = *y2;
		}
	} else if (gle_isinf(*x2)) {
		gclip_simple(x2, xmin, xmax);
		*y2 = *y1;
	}
	// Check upper bound for all variables
	// Check X
	if (*x2 > xmax) {
		if (*x1 > xmax) return true;
		double dx = *x2 - *x1;
		if (dx == 0) return true;
		double dy = *y2 - *y1;
		*y2 = *y1 + dy*(xmax-*x1)/dx;
		*x2 = xmax;
	}
	if (*x1 > xmax) {
		double dx = *x1 - *x2;
		if (dx == 0) return true;
		double dy = *y1 - *y2;
		*y1 = *y2 + dy*(xmax-*x2)/dx;
		*x1 = xmax;
	}
	// Check Y
	if (*y2 > ymax) {
		if (*y1 > ymax) return true;
		double dy = *y2 - *y1;
		if (dy == 0) return true;
		double dx = *x2 - *x1;
		*x2 = *x1 + dx*(ymax-*y1)/dy;
		*y2 = ymax;
	}
	if (*y1 > ymax) {
		double dy = *y1 - *y2;
		if (dy == 0) return true;
		double dx = *x1 - *x2;
		*x1 = *x2 + dx*(ymax-*y2)/dy;
		*y1 = ymax ;
	}
	// Check lower bound for all variables
	// Check X
	if (*x2 < xmin) {
		if (*x1 < xmin) return true;
		double dx = *x2 - *x1;
		if (dx == 0) return true;
		double dy = *y2 - *y1;
		*y2 = *y1 + dy*(xmin-*x1)/dx;
		*x2 = xmin;
	}
	if (*x1 < xmin) {
		double dx = *x1 - *x2 ;
		if (dx == 0) return true;
		double dy = *y1 - *y2 ;
		*y1 = *y2 + dy*(xmin-*x2)/dx;
		*x1 = xmin;
	}
	// Check Y
	if (*y2 < ymin) {
		if (*y1 < ymin) return true;
		double dy = *y2 - *y1;
		if (dy == 0) return true;
		double dx = *x2 - *x1;
		*x2 = *x1 + dx*(ymin-*y1)/dy;
		*y2 = ymin;
	}
	if (*y1 < ymin) {
		double dy = *y1 - *y2;
		if (dy == 0) return true;
		double dx = *x1 - *x2;
		*x1 = *x2 + dx*(ymin-*y2)/dy;
		*y1 = ymin;
	}
	return false;
}

void vinit_axis(int i) {
	xx[i].init(i);
}

void vinit_title_axis() {
	xx[GLE_AXIS_T].log = 0;
	xx[GLE_AXIS_T].getRange()->setMinMaxSet(0, 1);
	xx[GLE_AXIS_T].label_off = 1;
	xx[GLE_AXIS_T].side_off = 1;
	xx[GLE_AXIS_T].ticks_off = 1;
	xx[GLE_AXIS_T].subticks_off = 1;
	xx[GLE_AXIS_T].title_off = 0;
}

void doskip(char *s,int *ct) {
	if (*s==' ') (*ct)++;
}

double get_next_exp(TOKENS tk,int ntok,int *curtok) {
	static int i;
	static double x;
	(*curtok)++;
	dbg for (i=1;i<=ntok;i++)  gprint("{%s} ",(*tk)[i]);
	dbg gprint("\n");
	dbg gprint("**get exp token ct %d  {%s} \n",*curtok,(*tk)[*curtok]);
	if (strlen(tk[*curtok])==0){
		/* this is added for optional parameters on smoothing. They must be at end for this to work*/
		dbg gprint("zero length expression in get expression no polish called\n");
		x=0;
	} else {
		polish_eval(tk[*curtok],&x);
	}
	return x;
}

void get_next_exp_string(TOKENS tk, int ntok, int *curtok, string* res) {
	(*curtok)++;
	if ((*curtok) <= ntok) {
		polish_eval_string(tk[*curtok], res);
	} else {
		*res = "";
	}
}

void get_next_exp_file(TOKENS tk, int ntok, int *curtok, string* res) {
	(*curtok)++;
	if ((*curtok) <= ntok) {
		pass_file_name(tk[*curtok], *res);
	} else {
		*res = "";
	}
}

bool checktok(char *t, char *want) {
	if (str_i_equals(t,want)) return true;
	else {
		gprint("Found token {%s} Wanted {%s} \n",t,want);
		return false;
	}
}

#define GLE_DS_L  0
#define GLE_DS_R 1

typedef struct {
	double x, y[2];
} DataSetVal;

bool DataSetValCMP(const DataSetVal& e1, const DataSetVal& e2) {
   return e1.x < e2.x;
}

class GLELetDataSet {
protected:
	int m_DN;
	int m_Var;
	int m_CrP;
	bool m_IsFunction;
	bool m_IsXRangeDS;
	vector<DataSetVal> m_Vals;
	vector<double> m_MissX;
public:
	GLELetDataSet();
	~GLELetDataSet();
	void initializeFrom(int dn, int var);
	void complainNoFunction();
	bool interpolateTo(double x, int lr);
	inline int getNbValues() { return m_Vals.size(); }
	inline double getXValue(int i) { return m_Vals[i].x; }
	inline int getNbMissing() { return m_MissX.size(); }
	inline double getMissing(int i) { return m_MissX[i]; }
	inline bool isFunction() { return m_IsFunction; }
	inline int getDatasetID() { return m_DN; }
	inline int getVar() { return m_Var; }
	inline void setIsXRangeDS(bool value) { m_IsXRangeDS = value; }
	inline bool isXRangeDS() { return m_IsXRangeDS; }
};

GLELetDataSet::GLELetDataSet() {
	m_DN = -1;
	m_Var = -1;
	m_CrP = 0;
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

class DataFillDimension {
protected:
	bool m_Log;
	GLERange m_Range;
	double m_Value;
	GLEFunctionParserPcode* m_Fct;
	RefCountPtr<GLEDoubleArray> m_Values;
public:
	DataFillDimension(GLEFunctionParserPcode* fct);
	~DataFillDimension();
	void setRange(GLERange* range, bool log);
	bool isYValid();
	inline bool isLog() { return m_Log; }
	inline GLERange* getRange() { return &m_Range; }
	inline void setDoubleAt(double v, int i) { m_Values->setDoubleAt(v, i); }
	inline GLEDoubleArray* getValues() { return m_Values.get(); }
	inline double getValue() { return m_Value; }
	inline void computeValue() { m_Value = m_Fct->evalDouble(); }
	inline bool isYNan() { return gle_isnan(m_Value); }
};

DataFillDimension::DataFillDimension(GLEFunctionParserPcode* fct) {
	m_Log = false;
	m_Fct = fct;
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

class DataFill {
protected:
	int m_Size;
	int m_VarX;
	bool m_PrevInvalid;
	bool m_PrevPoint;
	bool m_FineTune;
	bool m_CrEnable;
	double m_PrevXValue;
	bool m_detectDiscontinuity;
	double m_discontinuityThreshold;
	double m_tuneDistance;
	int m_tuneIterationsMin;
	int m_tuneIterationsMax;
	set<double> m_MissX;
	GLEVectorAutoDelete<GLELetDataSet>* m_DataSets;
	GLEVectorAutoDelete<DataFillDimension> m_Dim;
	RefCountPtr<GLEBoolArray> m_Missing;
	GLEFunctionParserPcode* m_Where;
public:
	DataFill(bool finetune);
	~DataFill();
	void addPointFineTune(double x, int lr);
	void addPointIPol(double x);
	void addPoint();
	void addPointLR(double x, int lr);
	void addPoint(double x);
	void addPoint(double x, double y);
	void addMissing();
	void addMissingLR(double x, int lr);
	bool isYValid();
	bool isYNotNan();
	bool isRangeValid();
	bool selectXValue(double x, int lr);
	void selectXValueNoIPol(double x);
	double maxDistanceTo(double x);
	void minMaxDistanceTo(double x, GLERange* distanceRange);
	void tryIPol(double other, double nanval);
	void checkDiscontinuity(double xPrev, double xNext, int lr);
	void tryAddMissing(double x, int lr);
	void setInfo(GLEFunctionParserPcode* expr, int varx);
	void setYMinYMax(double ymin, double ymax);
	void setDetectDiscontinuity(bool detect, double threshold);
	void toDataset(GLEDataSet* dataset);
	inline int size() { return m_Size; }
	inline double* getX() { return m_Dim[0]->getValues()->toArray(); }
	inline double* getY() { return m_Dim[1]->getValues()->toArray(); }
	inline int* getM() { return m_Missing->toArray(); }
	inline void addDataDimension(DataFillDimension* fill) { m_Dim.push_back(fill); }
	inline void setDataSets(GLEVectorAutoDelete<GLELetDataSet>* sets) { m_DataSets = sets; }
	inline void setVarX(int varx) { m_VarX = varx; }
	inline bool isFineTune() { return m_FineTune; }
	inline void setMissingValue(double x) { m_MissX.insert(x); }
	inline void setWhere(GLEFunctionParserPcode* where) { m_Where = where; }
};

DataFill::DataFill(bool finetune) {
	m_Size = 0;
	m_VarX = -1;
	m_PrevInvalid = false;
	m_PrevPoint = false;
	m_FineTune = finetune;
	m_PrevXValue = GLE_INF;
	m_CrEnable = true;
	m_Missing = new GLEBoolArray();
	m_Where = NULL;
	m_detectDiscontinuity = false;
	m_discontinuityThreshold = GLE_INF;
	m_tuneDistance = 1e-6;
	m_tuneIterationsMin = 50;
	m_tuneIterationsMax = 10000;	
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
		(*m_DataSets)[i]->interpolateTo(x, GLE_DS_L);
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
		selectXValue(xmid, GLE_DS_L);
		if (!isYValid()) {
			nanval = xmid;
			not_xmid = other;
		} else {
			other = xmid;
			not_xmid = nanval;
		}
		if (iter > m_tuneIterationsMax || (iter > m_tuneIterationsMin 
										   && maxDistanceTo(not_xmid) < m_tuneDistance)) {
			addPointLR(xmid, GLE_DS_L);
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
	int lr = GLE_DS_L;
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
	int lr = GLE_DS_L;
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


class GLELet {
protected:
	GLERC<GLEVarSubMap> m_SubMap;
	GLERCVector<GLEFunctionParserPcode> m_Fcts;
	GLERC<GLEFunctionParserPcode> m_Where;
	GLEVarBackup m_varBackup;
	set<int> m_XRangeDS;
	double m_LetFrom, m_LetTo, m_LetStep, m_LogStep;
	bool m_FineTune, m_NoFirst, m_HasSteps, m_HasStepOption, m_HasFrom, m_HasTo;
	int m_VarX, m_Ds, m_LetNSteps;
	int m_codeLine;

	// For histogram routine
	int m_nrBins;
	int m_HistDS;

	// For fitting routines
	int m_FitDS;
	bool m_limitDataX;
	bool m_limitDataY;
	bool m_limitData;
	string m_fitType;
	string m_fitFct;
	string m_eqStr;
	string m_format;
	string m_rsq;
	string m_varSlope;
	string m_varIntercept;
	string m_varR;
	GLECheckWindow m_window;

public:
	GLELet();
	~GLELet();
	void initVars();
	void initStep();
	void doLet();
	void parseFitFunction(const string& fct, GLEParser* parser);
	void parseHistogram(GLEParser* parser);
	void doFitFunction();
	void doHistogram();
	void complainAboutNoFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets);
	bool checkIdenticalRanges(GLEVectorAutoDelete<GLELetDataSet>& datasets);
	void transformIdenticalRangeDatasets(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill);
	void combineFunctions(GLEVectorAutoDelete<GLELetDataSet>& datasets, DataFill* fill, double logstep);
	void setStep(double value);
	void createVarBackup(GLEVars* vars, StringIntHash* varHash);
	void restoreVarBackup(GLEVars* vars);
	GLEFunctionParserPcode* insertFunction();
	GLEFunctionParserPcode* addFunction();
	GLEFunctionParserPcode* addWhere();
	inline void setVarSubMap(GLEVarSubMap* map) { m_SubMap = map; }
	inline GLEVarSubMap* getVarSubMap() { return m_SubMap.get(); }
	inline void setFrom(double value) { m_LetFrom = value; }
	inline void setTo(double value) { m_LetTo = value; }
	inline void setNSteps(int value) { m_LetNSteps = value; }
	inline void setFineTune(bool value) { m_FineTune = value; }
	inline void setNoFirst(bool value) { m_NoFirst = value; }
	inline double getFrom() { return m_LetFrom; }
	inline double getTo() { return m_LetTo; }
	inline double getStep() { return m_LetStep; }
	inline bool hasSteps() { return m_HasSteps; }
	inline bool hasFrom() { return m_HasFrom; }
	inline bool hasTo() { return m_HasTo; }
	inline void setHasStepOption(bool has) { m_HasStepOption = has; }
	inline void setHasSteps(bool has) { m_HasSteps = has; }
	inline void setHasFrom(bool has) { m_HasFrom = has; }
	inline void setHasTo(bool has) { m_HasTo = has; }
	inline void setDataSet(int ds) { m_Ds = ds; }
	inline int getDataSet() { return m_Ds; }
	inline int getDimension() { return m_Fcts.size(); }
	inline void addFunction(GLEFunctionParserPcode* fct) { m_Fcts.add(fct); }
	inline void addXRangeDS(int ds) { m_XRangeDS.insert(ds); }
	inline set<int>& getXRangeDS() { return m_XRangeDS; }
	inline bool isHistogram() const  { return m_HistDS != -1; }
	inline bool isFit() const  { return m_FitDS != -1; }
	inline void setCodeLine(int line) { m_codeLine = line; }
	inline int getCodeLine() const { return m_codeLine; }
};

GLELet::GLELet() {
	m_SubMap = NULL;
	m_LetFrom = 0.0;
	m_LetTo = 0.0;
	m_LetStep = 0.0;
	m_LetNSteps = 0;
	m_VarX = -1;
	m_NoFirst = false;
	m_FineTune = false;
	m_HasSteps = false;
	m_HasStepOption = false;
	m_HasFrom = false;
	m_HasTo = false;
	m_Ds = -1;
	m_codeLine = -1;
	m_fitType = "none";
	m_HistDS = -1;
	m_nrBins = -1;
	m_FitDS = -1;
	m_limitDataX = false;
	m_limitDataY = false;
	m_limitData = false;
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

void deleteLet(GLELet* let) {
	delete let;
}

/*  LET d2 = exp(x) [ FROM exp TO exp [ STEP exp ] ]	*/
/*  LET d3 = exp(dn,d...)				*/
// added by V.L.
//	LET d3 = LINFIT(dn) [FROM exp TO exp] or [DATA] [SLOPE] [OFFSET] [R_SQUARED]
// [SLOPE] [OFFSET] [R_SQUARED] are user defined variables
//  no option after linfit results in line being drawn over the whole graph
// DATA results in the line being drawn from the minimum to the maximum of the data series

GLELet* parseLet(GLEParser* parser, int codeLine) {
	GLELet* let = new GLELet();
	let->setCodeLine(codeLine);
	Tokenizer* tokens = parser->getTokens();
	tokens->ensure_next_token_i("LET");

	// read the target data set identifier
	string& token = tokens->next_token();
	let->setDataSet(get_dataset_identifier(token, parser, false));
	ensureDataSetCreated(let->getDataSet());

	// next token should be an equal sign
	tokens->ensure_next_token("=");

	string let_fct = tokens->next_multilevel_token();
	if (str_i_equals(let_fct,"LINFIT")   ||
	    str_i_equals(let_fct,"LOGEFIT")  ||
	    str_i_equals(let_fct,"LOG10FIT") ||
	    str_i_equals(let_fct,"POWXFIT")  ||
	    str_i_equals(let_fct,"FIT")) {
		let->parseFitFunction(let_fct, parser);
		return let;
	}

	if (str_i_equals(let_fct,"HIST")) {
		let->parseHistogram(parser);
		return let;
	}

	GLEVars* vars = getVarsInstance();
	let->setVarSubMap(vars->addLocalSubMap());
	let->initVars();

	// read in multi-dimensional let
	StringIntHash varHash;
	GLEFunctionParserPcode* dim_1 = let->addFunction();
	dim_1->polishPos(let_fct.c_str(), tokens->token_pos_col(), &varHash);
	while (tokens->is_next_token(",")) {
		const string& let_fct_n = tokens->next_multilevel_token();
		GLEFunctionParserPcode* dim_n = let->addFunction();
		dim_n->polishPos(let_fct_n.c_str(), tokens->token_pos_col(), &varHash);
	}
	// if one dimensional, then first dimension is just "x"
	if (let->getDimension() == 1) {
		GLEFunctionParserPcode* dim_0 = let->insertFunction();
		dim_0->polishX();
	}
	// check for too high dimension
	if (let->getDimension() > 2) {
		ostringstream errstr;
		errstr << "let dimension > 2 not supported (found dimension " << let->getDimension() << ")";
		throw tokens->error(errstr.str());
	}
	let->createVarBackup(vars, &varHash);

	// iterate over all tokens
	while (tokens->has_more_tokens()) {
		token = tokens->next_token();
		if (str_i_equals(token,"FROM")) {
			let->setHasFrom(true);
			let->setFrom(parser->evalTokenToDouble());
		} else if (str_i_equals(token,"TO")) {
			let->setHasTo(true);
			let->setTo(parser->evalTokenToDouble());
		} else if (str_i_equals(token,"STEP")) {
			let->setHasStepOption(true);
			let->setStep(parser->evalTokenToDouble());
		} else if (str_i_equals(token,"NSTEPS")) {
			let->setNSteps((int)floor(parser->evalTokenToDouble()+0.5));
		} else if (str_i_equals(token,"NOTUNE")) {
			let->setFineTune(false);
		} else if (str_i_equals(token,"WHERE")) {
			const string& where_s = tokens->next_multilevel_token();
			GLEFunctionParserPcode* where = let->addWhere();
			where->polishPos(where_s.c_str(), tokens->token_pos_col());
		} else if (str_i_equals(token,"RANGE")) {
			token = tokens->next_token();
			let->addXRangeDS(get_dataset_identifier(token.c_str()));
			while (tokens->is_next_token(",")) {
				token = tokens->next_token();
				let->addXRangeDS(get_dataset_identifier(token.c_str()));
			}
		} else {
			ostringstream errstr;
			errstr << "unknown token in 'let' expression: '" << token << "'";
			throw tokens->error(errstr.str());
		}
	}

	vars->removeLocalSubMap();
	return let;
}

GLELet* parseLet(GLESourceLine& sline) {
	GLEParser* parser = get_global_parser();
	parser->setString(sline.getCodeCStr());
	return parseLet(parser, sline.getGlobalLineNo());
}

GLELet* parseLet(const string& letFct, int codeLine) {
	GLEParser* parser = get_global_parser();
	parser->setString(letFct.c_str());
	return parseLet(parser, codeLine);
}

void doLet(GLELet* let, bool nofirst) {
	g_set_error_line(let->getCodeLine());
	let->setNoFirst(nofirst);
	let->setFineTune(nofirst);
	GLEAxis* ax = &xx[GLE_AXIS_X];
	if (!let->hasFrom()) {
		let->setFrom(ax->getMin());
	}
	if (!let->hasTo()) {
		let->setTo(ax->getMax());
	}
	if (let->isHistogram()) {
		let->doHistogram();
	} else if (let->isFit()) {
		let->doFitFunction();
	} else {
		GLEVars* vars = getVarsInstance();
		vars->addLocalSubMap(let->getVarSubMap());
		let->restoreVarBackup(vars);
		let->initStep();
		let->doLet();
		vars->removeLocalSubMap();
	}
}

void next_lstyle(char* s,int* ct) {
	char next[200];
	double temp;
	int is_alpha;
	int i,len;

	is_alpha=false;
	(*ct)++;
	doskip(tk[*ct],ct);
	strcpy(next,tk[*ct]);
	len = strlen(next);

	for(i=0 ; i < len ; i++) {
		is_alpha=isalpha((int)next[i]);
		if (is_alpha) i = len;
	}
	if (is_alpha) {
		/* this is a variable so evaluate it */
		polish_eval(next,&temp);
		sprintf(s,"%g",temp);
	} else {
		/*		it is a number so just copy over*/
		if (len < 9) {
			strcpy(s,next);
		} else {
			gprint("ERROR line style string too long %s\n",next);
		}
	}
}

void next_svg_iter(int* s, int* ct) {
	char next[200];
	double temp;
	int is_alpha;
	int i,len,idx,type;
	is_alpha=false;
	(*ct)++;
	doskip(tk[*ct],ct);
	strcpy(next,tk[*ct]);
	len = strlen(next);
	printf("len=%d next=%s\n",len,next);
	// check to see if number or alphanumeric
	if ( len > 0 ) {
		for(i=0 ; i < len ; i++) {
			is_alpha=isalpha((int)next[i]);
			if (is_alpha) i = len;
		}
		if (is_alpha) {
			// it is alphanumeric so see if it is a variable
			// if not set it to zero
			var_find(next,&idx,&type);
			if (idx != -1 )	{
				/* this is a variable so evaluate it */
				polish_eval(next,&temp);
				*s = (int) temp;
			} else {
				// it's not a variable; must be another keyword
				// set iter to 1
				(*ct)--;
				*s = 1;
			}
			//sprintf(s,"%g\0",temp);
		} else {
			/* it is a number so just copy over*/
			*s = atoi(next);
		}
	} else {
		//len == 0 so there isnothing there set to 1
		*s = 1;
		(*ct)--;
	}
}

#define next_dn  (ct+=1,skipspace,atoi(tk[ct]+1))

void fixup_err(string& err) {
	if (err.size() >= 1 && toupper(err[0]) == 'D') {
		// If d[i] is used, then this should be converted now
		int dataset = get_dataset_identifier(err.c_str(), false);
		std::ostringstream fixupName;
		fixupName << "d" << dataset;
		err = fixupName.str();
	}
}

void do_dataset(int d, GLEGraphBlockInstance* graphBlock) {
	int ct = 2;
	while (ct <= ntk)	{
		kw("LINE") {
			dp[d]->line = true;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("LSTYLE") {
			next_lstyle(dp[d]->lstyle,&ct);
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("LWIDTH") {
			dp[d]->lwidth = next_exp;
		} else kw("MARKER") {
			dp[d]->marker = next_marker;
			dp[d]->layer_marker = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_MARKER);
		} else kw("MDATA") {
			dp[d]->mdata = next_dn;
		} else kw("COLOR") {
			dp[d]->color = next_color;
		} else kw("KEYFILL") {
			dp[d]->key_fill = next_color;
		} else kw("MSIZE") {
			dp[d]->msize = next_exp;
		} else kw("MDIST") {
			dp[d]->mdist = next_exp;
		} else kw("MSCALE") {
			dp[d]->mscale = next_exp;
		} else kw("MCOLOR") {
			dp[d]->mcolor = next_color;
		} else kw("KEY") {
			next_vquote_cpp(dp[d]->key_name);
		} else kw("AUTOSCALE") {
			dp[d]->autoscale = true;
		} else kw("AUTO") {
			dp[d]->autoscale = true;
		} else kw("NOMISS") {
			dp[d]->nomiss = true;
		} else kw("NOMISSING") {
			dp[d]->nomiss = true;
		} else kw("FILE") {
			next_vquote(dp[d]->bigfile);
		} else kw("BIGFILE") {
			next_vquote(dp[d]->bigfile);
		} else kw("STEPS") {
			dp[d]->line_mode = GLE_GRAPH_LM_STEPS;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("FSTEPS") {
			dp[d]->line_mode = GLE_GRAPH_LM_FSTEPS;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("HIST") {
			dp[d]->line_mode = GLE_GRAPH_LM_HIST;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("BAR") {
			dp[d]->line_mode = GLE_GRAPH_LM_BAR;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("IMPULSES") {
			dp[d]->line_mode = GLE_GRAPH_LM_IMPULSES;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
		} else kw("XAXIS") {
			dp[d]->getDim(GLE_DIM_X)->setAxis(GLE_AXIS_X);
		} else kw("YAXIS") {
			dp[d]->getDim(GLE_DIM_Y)->setAxis(GLE_AXIS_Y);
		} else kw("X2AXIS") {
			dp[d]->getDim(GLE_DIM_X)->setAxis(GLE_AXIS_X2);
		} else kw("Y2AXIS") {
			dp[d]->getDim(GLE_DIM_Y)->setAxis(GLE_AXIS_Y2);
		} else kw("X0AXIS") {
			dp[d]->getDim(GLE_DIM_X)->setAxis(GLE_AXIS_X0);
		} else kw("Y0AXIS") {
			dp[d]->getDim(GLE_DIM_Y)->setAxis(GLE_AXIS_Y0);
		} else kw("SMOOTH") {
			dp[d]->line = true;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
			dp[d]->smoothm = false;
			dp[d]->smooth = true;
		} else kw("SMOOTHM") 	{
			dp[d]->line = true;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
			dp[d]->smoothm = true;
			dp[d]->smooth = true;
		} else kw("SVG_SMOOTH") {
			dp[d]->line = true;
			dp[d]->layer_line = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_LINE);
			dp[d]->svg_iter = (int) next_exp;
			if (dp[d]->svg_iter == 0) dp[d]->svg_iter = 1; // if they say svg_smooth they want it at least once
			dp[d]->svg_smooth = true;
			dp[d]->smoothm = false;
			dp[d]->smooth = false;
		} else kw("DERESOLVE") {
			dp[d]->deresolve = (int) next_exp;
			dp[d]->deresolve_avg = false;
			if (str_i_equals(tk[ct+1], "AVERAGE")) {
				dp[d]->deresolve_avg = true;
				ct++;
			}
		} else kw("XMIN") {
			dp[d]->getDim(GLE_DIM_X)->getRange()->setMinSet(next_exp);
		} else kw("XMAX") {
			dp[d]->getDim(GLE_DIM_X)->getRange()->setMaxSet(next_exp);
		} else kw("YMIN") {
			dp[d]->getDim(GLE_DIM_Y)->getRange()->setMinSet(next_exp);
		} else kw("YMAX") {
			dp[d]->getDim(GLE_DIM_Y)->getRange()->setMaxSet(next_exp);
		} else kw("HERR") {
			next_str_cpp(dp[d]->herrup);
			fixup_err(dp[d]->herrup);
			dp[d]->herrdown = dp[d]->herrup;
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("HERRLEFT") {
			next_str_cpp(dp[d]->herrup);
			fixup_err(dp[d]->herrup);
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("HERRRIGHT") {
			next_str_cpp(dp[d]->herrdown);
			fixup_err(dp[d]->herrdown);
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("HERRWIDTH") {
			dp[d]->herrwidth = next_exp;
		} else kw("ERR") {
			next_str_cpp(dp[d]->errup);
			fixup_err(dp[d]->errup);
			dp[d]->errdown = dp[d]->errup;
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("ERRUP") {
			next_str_cpp(dp[d]->errup);
			fixup_err(dp[d]->errup);
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("ERRDOWN") {
			next_str_cpp(dp[d]->errdown);
			fixup_err(dp[d]->errdown);
			dp[d]->layer_error = graphBlock->getLayerWithDefault(GLE_GRAPH_LAYER_ERROR_BAR);
		} else kw("ERRWIDTH") {
			dp[d]->errwidth = next_exp;
		} else {
			g_throw_parser_error("unrecognised GRAPH DN sub command: '",tk[ct],"'");
		}
		ct++;
	}
	// some options imply "di line"
	if (dp[d]->line_mode != GLE_GRAPH_LM_PLAIN) {
		dp[d]->line = true;
	}
	if (dp[d]->lstyle[0] != 0) {
		dp[d]->line = true;
	}
	if (dp[d]->line || dp[d]->marker != 0) {
		// make sure axisscale is also on if "dn line" given
		// for individual datasets "di ...", this is enabled before this method is called
		dp[d]->axisscale = true;
	}
}

void do_each_dataset_settings() {
	// Set datasets in to/from of bar commands as used
	for (int bar = 1; bar <= g_nbar; bar++) {
		for (int i = 0; i < br[bar]->ngrp; i++) {
			int to_bar = br[bar]->to[i];
			int from_bar = br[bar]->from[i];
			if (to_bar != 0 && to_bar <= ndata && dp[to_bar] != NULL) {
				dp[to_bar]->axisscale = true;
				if (br[bar]->horiz) dp[to_bar]->inverted = true;
			}
			if (from_bar != 0 && from_bar <= ndata && dp[from_bar] != NULL) {
				dp[from_bar]->axisscale = true;
				if (br[bar]->horiz) dp[from_bar]->inverted = true;
			}
		}
	}
	/* Add dataset to key */
	for (int dn = 1; dn <= ndata; dn++) {
		if (dp[dn] != NULL && dp[dn]->axisscale) {
			g_graphBlockData->getOrder()->addDataSet(dn);
			/* automatically turn on labels on axis for this dataset */
			for (int dim = GLE_DIM_X; dim <= GLE_DIM_Y; dim++) {
				GLEAxis* ax = &xx[dp[dn]->getDim(dim)->getAxis()];
				if (!ax->has_label_onoff) ax->label_off = false;
			}
		}
	}
	do_dataset_key_entries();
	// If no dataset is used, then scale based on all datasets
	bool has = false;
	for (int dn = 1; dn <= ndata; dn++) {
		if (dp[dn] != NULL && dp[dn]->axisscale) has = true;
	}
	if (!has) {
		for (int dn = 1; dn <= ndata; dn++) {
			if (dp[dn] != NULL) dp[dn]->axisscale = true;
		}
	}
	// Make sure no axis has datasets associated to it
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		xx[axis].removeAllDimensions();
	}
	// Add datasets to axis
	for (int dn = 1; dn <= ndata; dn++) {
		if (dp[dn] != NULL && dp[dn]->axisscale) {
			for (int dim = 0; dim < 2; dim++) {
				GLEDataSetDimension* dimension = dp[dn]->getDim(dim);
				xx[dimension->getAxis()].addDimension(dimension);
			}
		}
	}
}

extern KeyInfo* g_keyInfo;

void do_dataset_key(int d) {
	if (dp[d] != NULL && dp[d]->key_name != "") {
		KeyEntry* entry = g_keyInfo->createEntry();
		entry->fill = dp[d]->key_fill;
		entry->color = dp[d]->color;
		entry->mcolor = dp[d]->mcolor;
		entry->lwidth = dp[d]->lwidth;
		entry->marker = dp[d]->marker;
		entry->msize = dp[d]->msize;
		strcpy(entry->lstyle,dp[d]->lstyle);
		if (entry->lstyle[0] == 0 && dp[d]->line) {
			entry->lstyle[0] = '1';
			entry->lstyle[1] = 0;
		}
		entry->descrip = dp[d]->key_name;
		if (g_get_tex_labels()) {
			entry->descrip.insert(0, "\\tex{");
			entry->descrip.append("}");
		}
	}
}

void do_dataset_key_entries()
{
	GLEArrayImpl* order = g_graphBlockData->getOrder()->getArray();
	for (unsigned int i = 0; i < order->size(); ++i) {
		if (order->getType(i) == GLEObjectTypeInt) {
			do_dataset_key(order->getInt(i));
		}
		if (order->getType(i) == GLEObjectTypeClassInstance) {
			GLEClassInstance* classInstance = static_cast<GLEClassInstance*>(order->getObject(i));
			if (classInstance->getDefinition() == g_graphBlockData->getGraphBlockBase()->getClassDefinitions()->getKeySeparator()) {
				if (i == 0 || i + 1 == order->size()) {
					g_throw_parser_error("key separator not in between key entries");
				}
				KeyEntry* entry = g_keyInfo->lastEntry();
				if (classInstance->getArray()->size() > 0) {
					entry->sepstyle = classInstance->getArray()->getInt(0);
				}
				g_keyInfo->addColumn();
			}
		}
	}
}

void set_bar_axis_places() {
	for (int bar = 1; bar <= g_nbar; bar++) {
		for (int i = 0; i < br[bar]->ngrp; i++) {
			int to_bar = br[bar]->to[i];
			if (to_bar != 0 && to_bar <= ndata && dp[to_bar] != NULL) {
				GLEAxis* axis = br[bar]->horiz ? &xx[2] : &xx[1];
				if (axis->hasNames() && !axis->hasPlaces()) {
					int np = dp[to_bar]->np;
					if (np == axis->getNbNames()) {
						GLEDataPairs barData;
						barData.copyDimension(getDataset(to_bar), 0);
						for (unsigned int i = 0; i < barData.size(); i++) {
							axis->addPlace(barData.getX(i));
						}
					}
				}
			}
		}
	}
}

void min_max_scale(GLEAxis* ax) {
	GLERange* range = ax->getDataRange();
	for (int dim = 0; dim < ax->getNbDimensions(); dim++) {
		GLEDataSet* dataset = ax->getDim(dim)->getDataSet();
		if (dataset->np > 0) {
			GLEDataPairs pairs(dataset);
			vector<double>* values = pairs.getDimension(ax->getDim(dim)->getDataDimensionIndex());
			for (unsigned int i = 0; i < pairs.size(); i++) {
				range->updateRange(values->at(i), pairs.getM(i));
			}
			doMinMaxScaleErrorBars(dataset, ax->getDim(dim)->getDataDimensionIndex(), range);
		}
	}
}

void quantile_scale(GLEAxis* ax) {
	/* building list for quantile calculation */
	vector<double> q_list;
	for (int dim = 0; dim < ax->getNbDimensions(); dim++) {
		GLEDataSet* dataset = ax->getDim(dim)->getDataSet();
		if (dataset->np > 0) {
			GLEDataPairs pairs(dataset);
			vector<double>* values = pairs.getDimension(ax->getDim(dim)->getDataDimensionIndex());
			for (unsigned int i = 0; i < pairs.size(); i++) {
				if (!pairs.getM(i)) {
					q_list.push_back(values->at(i));
				}
			}
		}
	}
	std::sort(q_list.begin(), q_list.end());
	int q_size = q_list.size();
	/* only do quantile scale if at least two values given */
	if (q_size > 1) {
		GLEAxisQuantileScale* q_scale = ax->getQuantileScale();
		/* calculating ymin */
		double q_intpart;
		double q_fracpart = modf((q_size-1)*q_scale->getLowerQuantile(), &q_intpart);
		double q_lower = q_list[(int)q_intpart];
		if ((int)q_intpart + 1 < q_size - 1) {
			q_lower = q_lower*(1.0-q_fracpart)+q_list[(int)q_intpart+1]*q_fracpart;
		}
		/* calculating ymax */
		q_fracpart = modf((q_size-1)*q_scale->getUpperQuantile(), &q_intpart);
		double q_upper = q_list[(int)q_intpart];
		if ((int)q_intpart + 1 < q_size - 1) {
			q_upper = q_upper*(1.0-q_fracpart)+q_list[(int)q_intpart+1]*q_fracpart;
		}
		double q_ymin = q_lower-(q_upper-q_lower)*q_scale->getLowerQuantileFactor();
		double q_ymax = q_upper+(q_upper-q_lower)*q_scale->getUpperQuantileFactor();
		/* store the axis range */
		GLERange* range = ax->getDataRange();
		range->updateRange(q_ymin);
		range->updateRange(q_ymax);
	} else {
		min_max_scale(ax);
	}
}

void get_dataset_ranges() {
	// Reset axis ranges
	reset_axis_ranges();
	// Get ranges from color map
	if (g_colormap != NULL && g_colormap->getData() != NULL) {
		GLERectangle* bounds = g_colormap->getData()->getBounds();
		bounds->addToRangeX(xx[GLE_AXIS_X].getDataRange());
		bounds->addToRangeY(xx[GLE_AXIS_Y].getDataRange());
	}
	// Set datasets in to/from of bar commands as used
	for (int bar = 1; bar <= g_nbar; bar++) {
		for (int i = 0; i < br[bar]->ngrp; i++) {
			int to_bar = br[bar]->to[i];
			if (to_bar != 0 && to_bar <= ndata && dp[to_bar] != NULL && dp[to_bar]->np > 0) {
				// Extend range at both sides for a bar dataset
				// So that bars are visible
				int np = dp[to_bar]->np;
				if (np > 0) {
					GLEDataSetDimension* xdim = dp[to_bar]->getDimXInv();
					GLERange* xrange = xx[xdim->getAxis()].getDataRange();
					GLEDataPairs barData(dp[to_bar]);
					double delta = barData.getMinXInterval();
					xrange->updateRange(barData.getX(0) - delta/2, barData.getM(0));
					xrange->updateRange(barData.getX(np - 1) + delta/2, barData.getM(np-1));
				}
			}
		}
	}
	// Autoscale axis
	for (int axis = GLE_AXIS_X; axis <= GLE_AXIS_Y0; axis++) {
		GLEAxis* ax = &xx[axis];
		if (!ax->getRange()->hasBoth()) {
			if (ax->shouldPerformQuantileScale()) {
				quantile_scale(ax);
			} else {
				min_max_scale(ax);
			}
		}
	}
}

void do_bigfile_compatibility_dn(int dn) {
	string infile = dp[dn]->bigfile;
	if (infile.length() >= 1 && infile[infile.length()-1] == '$') {
		int idx, typ;
		var_find(infile.c_str(), &idx, &typ) ;
		if (idx >= 0) var_getstr(idx, infile);
	}
	string fname;
	int bigcol1 = 1;
	int bigcol2 = 2;
	bool bigally = false;
	char_separator sep1(",", "");
	tokenizer<char_separator> tokens(infile, sep1);
	if (tokens.has_more()) {
		fname = tokens.next_token();
	}
	if (tokens.has_more()) {
		string col1 = tokens.next_token();
		if (col1 == "*") {
			bigally = true;
		} else {
			bigcol1 = atoi(col1.c_str());
		}
		if (tokens.has_more()) {
			string col2 = tokens.next_token();
			if (col2 == "*") {
				bigally = true;
			} else {
				bigcol2 = atoi(col2.c_str());
				if (bigcol2 == 0) {
					g_throw_parser_error_sys("expecting \"file,xcoloumn,ycolumn\", but found \"", infile.c_str(), "\"");
				}
			}
		}
	}
	if (fname.length() >= 1 && fname[fname.length()-1] == '$') {
		int idx, typ;
		var_find(fname.c_str(), &idx, &typ) ;
		if (idx >= 0) var_getstr(idx, fname);
	}
	string line;
	ifstream file;
	validate_open_input_stream(file, fname);
	vector<double> xp;
	vector<double> yp;
	vector<int> miss;
	vector<double> columns;
	vector<bool> columns_miss;
	char_separator sep2(" ,;\t\n", "!");
	tokenizer<char_separator> data_tokens(sep2);
	while (file.good()) {
		getline(file, line);
		data_tokens.set_input(line);
		if (bigally) {
			// only data points - not in columns
			while (data_tokens.has_more()) {
				const string& token = data_tokens.next_token();
				if (token == "!") {
					break;
				}
				xp.push_back(xp.size()+1);
				if (token == "*" || token == "?" || token == "-" || token == ".") {
					yp.push_back(0.0);
					miss.push_back(true);
				} else {
					yp.push_back(atof(token.c_str()));
					miss.push_back(false);
				}
			}
		} else {
			// read in columns
			int nbcol = 0;
			while (data_tokens.has_more()) {
				const string& token = data_tokens.next_token();
				if (nbcol >= (int)columns.size()) {
					columns.push_back(0.0);
					columns_miss.push_back(false);
				}
				if (token == "!") {
					break;
				}
				if (token == "*" || token == "?" || token == "-" || token == ".") {
					columns[nbcol] = 0.0;
					columns_miss[nbcol] = true;
				} else {
					columns[nbcol] = atof(token.c_str());
					columns_miss[nbcol] = false;
				}
				nbcol++;
			}
			// get data from columns
			if (bigcol1 == 0) {
				if (bigcol2 >= 1 && bigcol2 <= nbcol) {
					xp.push_back(xp.size()+1);
					yp.push_back(columns[bigcol2-1]);
					miss.push_back(columns_miss[bigcol2-1]);
				}
			} else {
				if (bigcol1 >= 1 && bigcol1 <= nbcol &&
				    bigcol2 >= 1 && bigcol2 <= nbcol) {
					xp.push_back(columns[bigcol1-1]);
					yp.push_back(columns[bigcol2-1]);
					miss.push_back(columns_miss[bigcol1-1] || columns_miss[bigcol2-1]);
				}
			}
		}
	}
	file.close();
	dp[dn]->clearAll();
	dp[dn]->fromData(xp, yp, miss);
}

void do_bigfile_compatibility() {
	for (int dn = 1; dn <= ndata; dn++) {
		if (dp[dn] != NULL) {
			if (dp[dn]->bigfile != NULL) {
				do_bigfile_compatibility_dn(dn);
			}
		}
	}
}

GLEToView::GLEToView() {
}

GLEToView::~GLEToView() {
}

GLEToGraphView::GLEToGraphView(GLEAxis* xAxis, GLEAxis* yAxis):
	m_xAxis(xAxis),
	m_yAxis(yAxis)
{
}

GLEToGraphView::~GLEToGraphView() {
}

GLEPoint GLEToGraphView::fnXY(const GLEPoint& xy) {
	return GLEPoint(fnx(xy.getX(), m_xAxis, m_xAxis->getRange()), fny(xy.getY(), m_yAxis, m_yAxis->getRange()));
}

GLEPoint GLEToGraphView::fnXYInv(const GLEPoint& xy) {
	return GLEPoint(fnxInv(xy.getX(), m_xAxis, m_xAxis->getRange()), fnyInv(xy.getY(), m_yAxis, m_yAxis->getRange()));
}

GLEToRectangularView::GLEToRectangularView()
{
}

GLEToRectangularView::~GLEToRectangularView() {
}

GLEPoint GLEToRectangularView::fnXY(const GLEPoint& xy) {
	CUtilsAssert(false);
	return xy;
}

GLEPoint GLEToRectangularView::fnXYInv(const GLEPoint& xy) {
	double xpos = (xy.getX() - m_origin.getX())/ m_size.getX() * m_xRange.getWidth() + m_xRange.getMin();
	double ypos = (xy.getY() - m_origin.getY())/ m_size.getY() * m_yRange.getWidth() + m_yRange.getMin();
	return GLEPoint(xpos, ypos);
}

class GLEColorMapBitmap : public GLEBitmap {
protected:
	GLEZData* m_Data;
	GLEColorMap* m_map;
	GLEPoint m_origin;
	GLEPoint m_size;
	double m_ZMin;
	double m_ZMax;
	GLESub* m_sub;
	GLEBYTE* m_pal;
	GLEBYTE* m_scanLine;
	GLEToView* m_toView;
public:
	GLEColorMapBitmap(GLEToView* toView, GLEColorMap* map, const GLEPoint& origin, const GLEPoint& size, GLEZData* data = NULL);
	virtual ~GLEColorMapBitmap();
	GLEBYTE* createColorPalette();
	virtual int readHeader();
	virtual int decode(GLEByteStream* output);
	void plotFunction(GLEPcode& code, int varx, int vary, GLEByteStream* output);
	void plotData(GLEZData* data, GLEByteStream* output);
	inline void setZRange(double zmin, double zmax) { m_ZMin = zmin; m_ZMax = zmax; }
	inline double getZMin() { return m_ZMin; }
	inline double getZMax() { return m_ZMax; }
	inline bool isFunction() { return m_Data == NULL; }
	inline GLEZData* getData() { return m_Data; }
private:
	void updateScanLine(int* pos, double zvalue);
	void init();
	void cleanUp();
};

GLEColorMapBitmap::GLEColorMapBitmap(GLEToView* toView, GLEColorMap* map, const GLEPoint& origin, const GLEPoint& size, GLEZData* data):
	GLEBitmap(),
	m_Data(data),
	m_map(map),
	m_origin(origin),
	m_size(size),
	m_ZMin(GLE_INF),
	m_ZMax(-GLE_INF),
	m_sub(0),
	m_pal(0),
	m_scanLine(0),
	m_toView(toView)
{
}

GLEColorMapBitmap::~GLEColorMapBitmap() {
}

int GLEColorMapBitmap::readHeader() {
	m_Width = m_map->getWidth();
	m_Height = m_map->getHeight();
	m_BitsPerComponent = 8;
	if (m_map->isColor() || m_map->hasPalette()) {
		setMode(GLE_BITMAP_RGB);
		setComponents(3);
	} else {
		setMode(GLE_BITMAP_GRAYSCALE);
		setComponents(1);
	}
	return GLE_IMAGE_ERROR_NONE;
}

int fixRange(int v, int min, int max) {
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

void GLEColorMapBitmap::init() {
	cleanUp();
	if (m_map->isColor()) {
		m_pal = GLEBitmapCreateColorPalette(32761);
	} else if (m_map->hasPalette()) {
		m_sub = sub_find(m_map->getPaletteFunction().c_str());
		if (m_sub == NULL)  {
			stringstream err;
			err << "palette subroutine '" << m_map->getPaletteFunction() << "' not found";
			g_throw_parser_error(err.str());
		} else if (m_sub->getNbParam() != 1) {
			stringstream err;
			err << "palette subroutine '" << m_map->getPaletteFunction() << "' should take one argument";
			g_throw_parser_error(err.str());
		}
	}
	m_scanLine = new GLEBYTE[getScanlineSize()];
}

void GLEColorMapBitmap::cleanUp() {
	delete[] m_pal;
	delete[] m_scanLine;
}

void GLEColorMapBitmap::updateScanLine(int* pos, double zvalue) {
	if (m_map->isColor()) {
		int color = (int)floor(zvalue * 32760 + 0.5);
		if (color > 32760) color = 32760;
		if (color < 0) color = 0;
		m_scanLine[(*pos)++] = m_pal[color*3];
		m_scanLine[(*pos)++] = m_pal[color*3+1];
		m_scanLine[(*pos)++] = m_pal[color*3+2];
	} else if (m_map->hasPalette()) {
		GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
		setEvalStack(stk.get(), 0, zvalue);
		getGLERunInstance()->sub_call_stack(m_sub, stk.get());
		GLEColor* color = getEvalStackColor(stk.get(), 0);
		m_scanLine[(*pos)++] = color->getRedI();
		m_scanLine[(*pos)++] = color->getGreenI();
		m_scanLine[(*pos)++] = color->getBlueI();
	} else {
		double grey = floor(zvalue*255+0.5);
		if (grey > 255) grey = 255;
		if (grey < 0) grey = 0;
		m_scanLine[(*pos)++] = (int)grey;
	}
}

void GLEColorMapBitmap::plotData(GLEZData* zdata, GLEByteStream* output) {
	double zmin = zdata->getZMin();
	double zmax = zdata->getZMax();
	if (m_map->hasZMin()) zmin = m_map->getZMin();
	if (m_map->hasZMax()) zmax = m_map->getZMax();
	IpolDoubleMatrix ipd(zdata->getData(), zdata->getNX(), zdata->getNY());
	std::unique_ptr<Ipol> ipol;
	if (m_map->getIpolType() == IPOL_TYPE_BICUBIC) {
		ipol.reset(new BicubicIpol(&ipd));
	} else {
		ipol.reset(new NearestIpol(&ipd));
	}
	double scale = zmax - zmin;
	GLERectangle* bounds = zdata->getBounds();
	for (int i = getHeight() - 1; i >= 0; i--) {
		int pos = 0;
		double yView = m_origin.getY() + m_size.getY() * (i + 0.5) / getHeight();
		for (int j = 0; j < getWidth(); j++) {
			double xView = m_origin.getX() + m_size.getX() * (j + 0.5) / getWidth();
			GLEPoint xy(m_toView->fnXYInv(GLEPoint(xView, yView)));
			double xpos = gle_limit_range((xy.getX() - bounds->getXMin()) / bounds->getWidth(), 0.0, 1.0);
			double ypos = gle_limit_range((xy.getY() - bounds->getYMin()) / bounds->getHeight(), 0.0, 1.0);
			double zvalue = 0.0;
			if (m_map->isInverted()) {
				zvalue = (zmax - ipol->ipol(xpos, ypos)) / scale;
			} else {
				zvalue = (ipol->ipol(xpos, ypos) - zmin) / scale;
			}
			updateScanLine(&pos, zvalue);
		}
		output->send(m_scanLine, getScanlineSize());
		output->endScanLine();
	}
	setZRange(zmin, zmax);
}

void GLEColorMapBitmap::plotFunction(GLEPcode& code, int varx, int vary, GLEByteStream* output) {
	double zmax = -GLE_INF;
	double zmin = GLE_INF;
	double set_zmin = 0.0;
	double set_zmax = 1.0;
	if (m_map->hasZMin()) {
		set_zmin = m_map->getZMin();
	}
	if (m_map->hasZMax()) {
		set_zmax = m_map->getZMax();
	}
	double scale = set_zmax - set_zmin;
	for (int i = getHeight() - 1; i >= 0; i--) {
		int pos = 0;
		double yView = m_origin.getY() + m_size.getY() * (i + 0.5) / getHeight();
		for (int j = 0; j < getWidth(); j++) {
			double xView = m_origin.getX() + m_size.getX() * (j + 0.5) / getWidth();
			GLEPoint xy(m_toView->fnXYInv(GLEPoint(xView, yView)));
			var_set(varx, xy.getX());
			var_set(vary, xy.getY());
			double zvalue;
			eval_pcode(code, &zvalue);
			if (zvalue > zmax) zmax = zvalue;
			if (zvalue < zmin) zmin = zvalue;
			if (m_map->isInverted()) {
				zvalue = (set_zmax - zvalue) / scale;
			} else {
				zvalue = (zvalue - set_zmin) / scale;
			}
			updateScanLine(&pos, zvalue);
		}
		output->send(m_scanLine, getScanlineSize());
		output->endScanLine();
	}
	setZRange(zmin, zmax);
}

int GLEColorMapBitmap::decode(GLEByteStream* output) {
	init();
	if (isFunction()) {
		int varx, vary;
		int vartype = 1;
		GLEVars* vars = getVarsInstance();
		GLERC<GLEVarSubMap> submap(vars->addLocalSubMap());
		var_findadd("X", &varx, &vartype);
		var_findadd("Y", &vary, &vartype);
		GLEPcodeList pc_list;
		GLEPcode my_pcode(&pc_list);
		polish((char*)m_map->getFunction().c_str(), my_pcode, &etype);
		plotFunction(my_pcode, varx, vary, output);
		vars->removeLocalSubMap();
	} else {
		plotData(getData(), output);
	}
	cleanUp();
	var_findadd_set("ZGMIN", getZMin());
	var_findadd_set("ZGMAX", getZMax());
	return GLE_IMAGE_ERROR_NONE;
}

GLEColorMap::GLEColorMap() {
	m_color = false;
	m_wd = 50; m_hi = 50;
	m_zmin = 0.0; m_zmax = 1.0;
	m_has_zmin = false;
	m_has_zmax = false;
	m_invert = false;
	m_haspal = false;
	m_Data = NULL;
	m_ipolType = IPOL_TYPE_BICUBIC;
}

GLEColorMap::~GLEColorMap() {
	if (m_Data != NULL) delete m_Data;
}

void GLEColorMap::readData() {
	string fname;
	int varx, vary;
	int vartype = 1;
	GLEVars* vars = getVarsInstance();
	GLERC<GLEVarSubMap> submap(vars->addLocalSubMap());
	var_findadd("X", &varx, &vartype);
	var_findadd("Y", &vary, &vartype);
	polish_eval_string(getFunction().c_str(), &fname, true);
	vars->removeLocalSubMap();
	if (str_i_ends_with(fname, ".Z") || str_i_ends_with(fname, ".GZ")) {
		m_Data = new GLEZData();
		m_Data->read(fname);
	}
}
/*
Thanks for pointing this out! Indeed, if you use a function (not
a .z file), then the range of the function should be 0 - 1.
Normally, it should be possible to specify the range with zmin
and zmax, but this does not work correctly. The function is
multiplied by (zmax - zmin) and not by 1/(zmax - zmin). I have
now corrected this behaviour. The fix is in the current
development version and will be included in the next release.
*/
void GLEColorMap::setPalette(const string& pal) {
	m_palette = pal;
	m_haspal = true;
}

void GLEColorMap::setZMin(double val) {
	m_has_zmin = true;
	m_zmin = val;
}

void GLEColorMap::setZMax(double val) {
	m_has_zmax = true;
	m_zmax = val;
}

void GLEColorMap::draw(GLEToView* toView, double x0, double y0, double wd, double hi) {
	GLEZData* zdata = getData();
	if (zdata != NULL) {
		/* figure out position of bitmap */
		GLERectangle* bounds = zdata->getBounds();
		GLERectangle screenBounds;
		screenBounds.initRange();
		screenBounds.updateRange(fnx(bounds->getXMin(), &xx[GLE_AXIS_X]), fny(bounds->getYMin(), &xx[GLE_AXIS_Y]));
		screenBounds.updateRange(fnx(bounds->getXMax(), &xx[GLE_AXIS_X]), fny(bounds->getYMax(), &xx[GLE_AXIS_Y]));
		double xMin = std::max<double>(x0, screenBounds.getXMin());
		double yMin = std::max<double>(y0, screenBounds.getYMin());
		double xMax = std::min<double>(x0 + wd, screenBounds.getXMax());
		double yMax = std::min<double>(y0 + hi, screenBounds.getYMax());
		if (xMin > xMax || yMin > yMax) {
			return;
		}
		g_move(xMin, yMin);
		GLEColorMapBitmap bitmap(toView, this, GLEPoint(xMin, yMin), GLEPoint(xMax - xMin, yMax - yMin), zdata);
		g_bitmap(&bitmap, xMax - xMin, yMax - yMin, BITMAP_TYPE_USER);
	} else {
		g_move(x0, y0);
		GLEColorMapBitmap bitmap(toView, this, GLEPoint(x0, y0), GLEPoint(wd, hi));
		g_bitmap(&bitmap, wd, hi, BITMAP_TYPE_USER);
	}
}

GLEDataSet::GLEDataSet(int identifier) {
	id = identifier;
	nomiss = 0;
	np = 0;      /* NUMBER OF POINTS */
	autoscale = 0;
	axisscale = false;
	inverted = false;
	lstyle[0] = 0;
	key_name = "";
	bigfile = NULL;
	key_fill = 0;
	color = g_get_color_hex(GLE_COLOR_BLACK);
	mcolor = 0;
	errup[0] = 0;
	errdown[0] = 0;
	errwidth = 0.0;
	herrup[0] = 0;
	herrdown[0] = 0;
	herrwidth = 0.0;
	msize = 0.0;
	mdist = 0.0;
	lwidth = -1.0;
	yv_str = NULL;
	marker = 0;
	smooth = 0;
	smoothm = 0;
	svg_smooth = 0;         /* Savitski Golay filtering true=on */
	svg_poly = 0;           /* the type of polynomial 2,3,4,5...*/
	svg_points = 0;         /* the number of points 5,7,9,11.... */
	svg_iter = 0;           /* numb or time to do svg smoothing */
	deresolve = 0;          /* Only plot every N points: true = on */
	deresolve_avg = false;  /* dresolve + average points */
	line_mode = GLE_GRAPH_LM_PLAIN;
	mdata = 0;
	mscale = 0;
	line = 0;
	layer_line = GLE_GRAPH_LAYER_LINE;
	layer_marker = GLE_GRAPH_LAYER_MARKER;
	layer_error = GLE_GRAPH_LAYER_ERROR_BAR;
	rx1 = ry1 = rx2 = ry2 = 0;
	initBackup();
	getDim(0)->setAxis(GLE_AXIS_X);
	getDim(1)->setAxis(GLE_AXIS_Y);
	getDim(0)->setIndex(0);
	getDim(1)->setIndex(1);
	getDim(0)->setDataSet(this);
	getDim(1)->setDataSet(this);
}

GLEDataSet::~GLEDataSet() {
	clearAll();
}

void GLEDataSet::initBackup() {
	m_dataBackup.clear();
}

void GLEDataSet::clearAll() {
	np = 0;
	m_dataBackup.clear();
	m_data.clear();
}

bool GLEDataSet::undefined() {
	return m_data.size() == 0;
}

void GLEDataSet::copy(GLEDataSet* other) {
	clearAll();
	axisscale = false;
	nomiss = other->nomiss;
	autoscale = other->autoscale;
	inverted = other->inverted;
	strcpy(lstyle, other->lstyle);
	key_fill = other->key_fill;
	errup = other->errup;
	errdown = other->errdown;
	errwidth = other->errwidth;
	herrup = other->herrup;
	herrdown = other->herrdown;
	herrwidth = other->herrwidth;
	msize = other->msize;
	mdist = other->mdist;
	lwidth = other->lwidth;
	marker = other->marker;
	smooth = other->smooth;
	smoothm = other->smoothm;
	svg_smooth = other->svg_smooth;
	svg_poly = other->svg_poly;
	svg_points = other->svg_points;
	svg_iter = other->svg_iter;
	deresolve = other->deresolve;
	deresolve_avg = other->deresolve_avg;
	line_mode = other->line_mode;
	mdata = other->mdata;
	color = other->color;
	mscale = other->mscale;
	mcolor = other->mcolor;
	line = other->line;
	rx1 = other->rx1;
	ry1 = other->ry1;
	rx2 = other->rx2;
	ry2 = other->ry2;
	layer_line = other->layer_line;
	layer_marker = other->layer_marker;
	layer_error = other->layer_error;
	initBackup();
	getDim(0)->copy(other->getDim(0));
	getDim(1)->copy(other->getDim(1));
}

void GLEDataSet::backup() {
	m_dataBackup.ensure(m_data.size());
	for (unsigned int i = 0; i < m_data.size(); i++) {
		m_dataBackup.set(i, m_data.get(i));
	}
}

void GLEDataSet::restore() {
	if (m_dataBackup.size() != 0) {
		for (unsigned int i = 0; i < m_dataBackup.size(); i++) {
			m_data.set(i, m_dataBackup.get(i));
			GLEDataObject* dimI = m_dataBackup.getObject(i);
			if (dimI != 0 && dimI->getType() == GLEObjectTypeArray) {
				np = static_cast<GLEArrayImpl*>(dimI)->size();
			}
		}
	}
	initBackup();
}

GLEDataSetDimension* GLEDataSet::getDimXInv() {
	return inverted ? getDim(1) : getDim(0);
}

GLEDataSetDimension* GLEDataSet::getDimYInv() {
	return inverted ? getDim(0) : getDim(1);
}

GLEAxis* GLEDataSet::getAxis(int i)
{
	return &xx[getDim(i)->getAxis()];
}

void GLEDataSet::clip(double *x, double *y) {
	getDim(GLE_DIM_X)->getRange()->clip(x);
	getDim(GLE_DIM_Y)->getRange()->clip(y);
}

bool GLEDataSet::contains(double x, double y) {
	return getDim(GLE_DIM_X)->getRange()->contains(x)
		   && getDim(GLE_DIM_Y)->getRange()->contains(y);
}

bool GLEDataSet::contains(const GLEPoint& p) {
	return contains(p.getX(), p.getY());
}

void GLEDataSet::copyRangeIfRequired(int dimension) {
	if (!getDim(dimension)->getRange()->valid()) {
		getDim(dimension)->getRange()->copyIfNotSet(getAxis(dimension)->getRange());
	}
}

void GLEDataSet::validateDimensions() {
	GLEArrayImpl* data = getData();
	for (unsigned int i = 0; i < data->size(); i++) {
		GLEDataObject* dim = data->getObject(i);
		if (dim == 0 || dim->getType() != GLEObjectTypeArray) {
			ostringstream err;
			err << "dataset d" << id << " dimension " << dimension2String(i) << " not a double array";
			g_throw_parser_error(err.str());
		}
		GLEArrayImpl* array = static_cast<GLEArrayImpl*>(dim);
		if (array->size() != np) {
			ostringstream err;
			err << "dataset d" << id << " dimension " << dimension2String(i)
				<< " has an incorrect number of data points (" << array->size() << " <> " << np << ")";
			g_throw_parser_error(err.str());
		}
	}
}

void GLEDataSet::validateNbPoints(unsigned int expectedNb, const char* descr) {
	if (np != expectedNb) {
		ostringstream err;
		if (descr != 0) {
			err << descr << " ";
		}
		err << "dataset d" << id << " contains " << np;
		err << " data points, but " << expectedNb << " are required";
		g_throw_parser_error(err.str());
	}
}

GLEArrayImpl* GLEDataSet::getDimData(unsigned int dim) {
	if (dim >= getData()->size()) {
		return 0;
	}
	GLEDataObject* data = getData()->getObject(dim);
	if (data != 0 && data->getType() == GLEObjectTypeArray) {
		return static_cast<GLEArrayImpl*>(data);
	} else {
		return 0;
	}
}

void GLEDataSet::fromData(const vector<double>& xp, const vector<double>& yp, const vector<int>& miss) {
	np = xp.size();
	GLEArrayImpl* dimensions = getData();
	unsigned int dims = 2;
	dimensions->ensure(dims);
	for (unsigned int dim = 0; dim < dims; dim++) {
		GLEArrayImpl* newArray = new GLEArrayImpl();
		newArray->ensure(np);
		dimensions->setObject(dim, newArray);
		for (unsigned int i = 0; i < np; i++) {
			if (miss[i]) {
				newArray->setUnknown(i);
			} else {
				newArray->setDouble(i, dim == 0 ? xp[i] : yp[i]);
			}
		}
	}

}

vector<int> GLEDataSet::getMissingValues() {
	vector<int> result;
	result.assign(np, false);
	GLEArrayImpl* dimensions = getData();
	for (unsigned int dim = 0; dim < dimensions->size(); dim++) {
		GLEDataObject* data = dimensions->getObject(dim);
		if (data != 0 && data->getType() == GLEObjectTypeArray) {
			GLEArrayImpl* array = static_cast<GLEArrayImpl*>(data);
			unsigned int count = min(np, array->size());
			for (unsigned int i = 0; i < count; i++) {
				if (array->isUnknown(i)) {
					result[i] = true;
				}
			}
		}
	}
	return result;
}

void GLEDataSet::checkRanges() {
	// when parsing "let" -> already create dataset with ensureCreate...
	// so that dn command applies to it
	copyRangeIfRequired(GLE_DIM_X);
	copyRangeIfRequired(GLE_DIM_Y);
	if (!getDim(GLE_DIM_X)->getRange()->valid()) {
		g_throw_parser_error("invalid range for dimension X");
	}
	if (!getDim(GLE_DIM_Y)->getRange()->valid()) {
		g_throw_parser_error("invalid range for dimension Y");
	}
}

GLEDataSetDimension::GLEDataSetDimension() {
	m_Axis = GLE_AXIS_NONE;
}

GLEDataSetDimension::~GLEDataSetDimension() {
}

void GLEDataSetDimension::copy(GLEDataSetDimension* other) {
	setAxis(other->getAxis());
	getRange()->copySet(other->getRange());
}

int GLEDataSetDimension::getDataDimensionIndex() {
	if (getDataSet()->inverted) {
		return getIndex() == 0 ? 1 : 0;
	} else {
		return getIndex();
	}
}

GLEDataPairs::GLEDataPairs() {
}

GLEDataPairs::GLEDataPairs(double* x, double* y, int* m, int np) {
	set(x, y, m, np);
}

GLEDataPairs::GLEDataPairs(GLEDataSet* dataSet) {
	copy(dataSet);
}

GLEDataPairs::~GLEDataPairs() {
}

void GLEDataPairs::validate(GLEDataSet* data, unsigned int minDim) {
	if (data->getData()->size() < minDim) {
		ostringstream err;
		err << "dataset d" << data->id << " has " << data->getData()->size() << " dimensions, but " << minDim << " dimensions are required";
		g_throw_parser_error(err.str());
	}
	data->validateDimensions();
}

double GLEDataPairs::getDataPoint(GLEMemoryCell* element, int datasetID, unsigned int dimension, unsigned int arrayIdx) {
	double result = 0.0;
	if (!gle_memory_cell_to_double(element, &result)) {
		ostringstream err;
		err << "dataset d" << datasetID << " dimension " << dimension2String(dimension)
			<< " point " << (arrayIdx + 1) << ": expected double but found '";
		gle_memory_cell_print(element, err);
		err << "'";
		g_throw_parser_error(err.str());
	}
	return result;
}

void GLEDataPairs::copyDimensionImpl(GLEArrayImpl* data, unsigned int np, int datasetID, unsigned int dim) {
	GLEArrayImpl* array = static_cast<GLEArrayImpl*>(data->getObject(dim));
	vector<double>* dimDataPoints = getDimension(dim);
	dimDataPoints->resize(np);
	for (unsigned int i = 0; i < np; i++) {
		GLEMemoryCell* element = array->get(i);
		if (element->Type == GLE_MC_UNKNOWN) {
			m_M[i] = true;
			dimDataPoints->at(i) = 0.0;
		} else {
			dimDataPoints->at(i) = getDataPoint(element, datasetID, dim, i);
		}
	}
}

void GLEDataPairs::copyDimension(GLEDataSet* dataSet, unsigned int dim) {
	validate(dataSet, dim + 1);
	m_M.assign(dataSet->np, false);
	copyDimensionImpl(dataSet->getData(), dataSet->np, dataSet->id, dim);
}

void GLEDataPairs::copy(GLEDataSet* dataSet) {
	validate(dataSet, 2);
	m_M.assign(dataSet->np, false);
	GLEArrayImpl* data = dataSet->getData();
	for (unsigned int dim = 0; dim < data->size(); dim++) {
		copyDimensionImpl(data, dataSet->np, dataSet->id, dim);
	}
}

void GLEDataPairs::resize(int np) {
	m_X.resize(np);
	m_Y.resize(np);
	m_M.resize(np);
}

void GLEDataPairs::set(double* x, double* y, int* m, int np) {
	resize(np);
	for (int i = 0; i < np; i++) {
		m_X[i] = x[i]; m_Y[i] = y[i]; m_M[i] = m[i];
	}
}

void GLEDataPairs::set(unsigned int i, double x, double y, int m) {
	if (i < size()) {
		m_X[i] = x; m_Y[i] = y; m_M[i] = m;
	}
}

void GLEDataPairs::add(double x, double y, int m) {
	m_X.push_back(x);
	m_Y.push_back(y);
	m_M.push_back(m);
}

void GLEDataPairs::noMissing() {
	int pos = 0;
	int npnts = size();
	for (int j = 0; j < npnts; j++) {
		if (!m_M[j]) {
			m_X[pos] = m_X[j]; m_Y[pos] = m_Y[j]; m_M[pos] = m_M[j];
			pos++;
		}
	}
	resize(pos);
}

void GLEDataPairs::noNaN() {
	int pos = 0;
	int npnts = size();
	for (int j = 0; j < npnts; j++) {
		if (m_M[j] || (!gle_isnan(m_X[j]) && !gle_isnan(m_Y[j]))) {
			m_X[pos] = m_X[j]; m_Y[pos] = m_Y[j]; m_M[pos] = m_M[j];
			pos++;
		}
	}
	resize(pos);
}

void GLEDataPairs::noLogZero(bool xlog, bool ylog) {
	int pos = 0;
	int npnts = size();
	for (int j = 0; j < npnts; j++) {
		if (!(xlog && m_X[j] < 0) && !(ylog && m_Y[j] < 0)) {
			m_X[pos] = m_X[j]; m_Y[pos] = m_Y[j]; m_M[pos] = m_M[j];
			pos++;
		}
	}
	resize(pos);
}

void GLEDataPairs::transformLog(bool xlog, bool ylog) {
	if (xlog) {
		for (unsigned int i = 0; i < size(); i++) {
			m_X[i] = log10(m_X[i]);
		}
	}
	if (ylog) {
		for (unsigned int i = 0; i < size(); i++) {
			m_Y[i] = log10(m_Y[i]);
		}
	}
}

void GLEDataPairs::untransformLog(bool xlog, bool ylog) {
	if (xlog) {
		for (unsigned int i = 0; i < size(); i++) {
			m_X[i] = pow(10.0, m_X[i]);
		}
	}
	if (ylog) {
		for (unsigned int i = 0; i < size(); i++) {
			m_Y[i] = pow(10.0, m_Y[i]);
		}
	}
}

vector<double>* GLEDataPairs::getDimension(unsigned int i) {
	switch (i) {
	case 0:
		return &m_X;
	case 1:
		return &m_Y;
	default:
		return NULL;
	}
}

double GLEDataPairs::getMinXInterval() {
	double min = GLE_INF;
	for (unsigned int i = 1; i < m_X.size(); i++) {
		double w = m_X[i] - m_X[i-1];
		if (w > 0.0 && w < min) {
			min = w;
		}
	}
	return min;
}
