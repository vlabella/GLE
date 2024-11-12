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

/*
	Contouring program, creates gle files.
*/

#include "../all.h"
#include "../tokens/Tokenizer.h"
#include "../core.h"
#include "../glearray.h"
#include "../file_io.h"
#include "../texinterface.h"
#include "../cutils.h"
#include "../cmdline.h"
#include "../config.h"
#include "../mem_limits.h"
#include "../token.h"
#include "../gprint.h"
#include "../cutils.h"

#define BEGINDEF extern
#include "../begin.h"

#include "f2c.h"
using namespace std;

double get_next_exp(TOKENS tk, int ntk, int *curtok);
void get_next_exp_file(TOKENS tk, int ntok, int *curtok, string* res);

#define kw(ss) if (ct <= ntk && str_i_equals(tk[ct],ss))
#define next_file_eval(s) get_next_exp_file(tk,ntk,&ct,&s)
#define next_str(s) ct+=1;s=tk[ct]

bool smooth = false;
int smoothsub = 5;

void draw_(double *x, double *y, int *iflag);
int gcontr_(double *, int *, int *, int *, double *, int *, double *, int *, S_fp draw);
int glefitcf_(int* mode, double *x, double *y, int* l, int* m, double *u, double *v, int* n);

class GLEContourInfo {
private:
	FILE* m_DatFile;
	FILE* m_LabFile;
	vector <double> m_CVal;
	vector <string> m_CLab;
	vector <double> m_XPt;
	vector <double> m_YPt;
	double m_XCur, m_YCur;
	GLEZData m_Data;
public:
	GLEContourInfo();
	~GLEContourInfo();
	inline int getNbLines() { return m_CVal.size(); }
	inline double getCValue(int i) { return m_CVal[i]; }
	inline void addValue(double v) { m_CVal.push_back(v); }
	inline double* getCValueArray() { return (double*)&m_CVal[0]; }
	inline double getXCur() { return m_XCur; }
	inline double getYCur() { return m_YCur; }
	inline void setXCur(double v) { m_XCur = v; }
	inline void setYCur(double v) { m_YCur = v; }
	inline int getNbDataPoints() { return m_XPt.size(); }
	inline double getDataX(int i) { return m_XPt[i]; }
	inline double getDataY(int i) { return m_YPt[i]; }
	inline double* getDataXArray() { return &m_XPt[0]; }
	inline double* getDataYArray() { return &m_YPt[0]; }
	inline void read(const string& fname) { m_Data.read(fname); }
	inline GLERectangle* getBounds() { return m_Data.getBounds(); }
	inline int getNX() { return m_Data.getNX(); }
	inline int getNY() { return m_Data.getNY(); }
	inline double getZMin() { return m_Data.getZMin(); }
	inline double getZMax() { return m_Data.getZMax(); }
	inline double* getData() { return m_Data.getData(); }
	void addDataPoint(double x, double y);
	void setDataPoint(int i, double x, double y);
	void addAllDataPoints();
	void clearDataPoints();
	double sx(double v);
	double sy(double v);
	void addUnknown();
	void addPoint(double x, double y);
	void addPointScale(double x, double y);
	void addLabel(double x, double y, int i, double v);
	void addVect(int m, double x, double y);
	void fillDefault(double zmin, double zmax, double zdel);
	void createLabels(bool alpha);
	void openData(const string& name, const string& lab);
	void closeData();
	void doContour(double zz[], int nrz, int nx, int ny, double zmax);
	void draw(double* x, double* y, int iflag);
};

GLEContourInfo* g_ContourInfo = NULL;

GLEContourInfo::GLEContourInfo() {
	m_DatFile = NULL;
	m_LabFile = NULL;
	m_XCur = 0.0;
	m_YCur = 0.0;
}

GLEContourInfo::~GLEContourInfo() {
}

void GLEContourInfo::addDataPoint(double x, double y) {
	m_XPt.push_back(x);
	m_YPt.push_back(y);
}

void GLEContourInfo::setDataPoint(int i, double x, double y) {
	m_XPt[i] = x;
	m_YPt[i] = y;
}

void GLEContourInfo::clearDataPoints() {
	m_XPt.clear();
	m_YPt.clear();
}

void GLEContourInfo::addAllDataPoints() {
	for (int i = 0; i < getNbDataPoints(); i++) {
		addPoint(getDataX(i), getDataY(i));
	}
}

void GLEContourInfo::fillDefault(double zmin, double zmax, double zdel) {
	double zval = zmin;
	do {
		m_CVal.push_back(zval);
		zval += zdel;
	} while (zval <= zmax);
}

void GLEContourInfo::createLabels(bool alpha) {
	for (int i = 0; i < getNbLines(); i++) {
		if (alpha) {
			char buff[20];
			sprintf(buff,"%c",i+'A');
			m_CLab.push_back(buff);
		} else {
	  		// use numberformat instead later on
			char buff[50];
			sprintf(buff,"%g",m_CVal[i]);
			m_CLab.push_back(buff);
		}
	}
}

void GLEContourInfo::openData(const string& name, const string& lab) {
	m_DatFile = validate_fopen(name, "w", false);
	m_LabFile = validate_fopen(lab, "w", false);
}

void GLEContourInfo::closeData() {
	fclose(m_DatFile);
	fclose(m_LabFile);
	m_DatFile = NULL;
	m_LabFile = NULL;
}

void GLEContourInfo::doContour(double zz[], int nrz, int nx, int ny, double zmax) {
	int ncv = getNbLines();
	// dimension of work is large enough to contain
	// 2*(dimension of c)*(total dimension of z) useful bits
	int bytes = ((int)sizeof(int) * 2 * ncv * nx * ny)/31 + 10;
	int* work = (int*)malloc(bytes);
	if (work == NULL) {
		printf("Unable to allocate storage for work array\n");
		exit(1);
	}
	memset(work, 0, bytes);
	zmax = zmax + 100; /* no clipping */
	double* cvalp = getCValueArray();
    gcontr_(zz, &nrz, &nx, &ny, cvalp, &ncv, &zmax, work, (S_fp)draw_);
}

double GLEContourInfo::sx(double v) {
	GLERectangle* bounds = getBounds();
	return bounds->getXMin() + (bounds->getXMax()-bounds->getXMin())*(v-1)/(getNX()-1);
}

double GLEContourInfo::sy(double v) {
	GLERectangle* bounds = getBounds();
	return bounds->getYMin() + (bounds->getYMax()-bounds->getYMin())*(v-1)/(getNY()-1);
}

void GLEContourInfo::addUnknown() {
	fprintf(m_DatFile, "* *\n");
}

void GLEContourInfo::addPoint(double x, double y) {
	fprintf(m_DatFile,"%g %g\n", x, y);
}

void GLEContourInfo::addPointScale(double x, double y) {
	fprintf(m_DatFile,"%g %g\n", sx(x), sy(y));
}

void GLEContourInfo::addLabel(double x, double y, int i, double v) {
	fprintf(m_LabFile,"%g %g %d %g\n", x, y, i, v);
}

void GLEContourInfo::draw(double* x, double* y, int iflag) {
	int ih = iflag / 10;
	/* printf("level %d \n",ih); */
	int il = iflag - ih * 10;
	switch (il) {
	  case 6:   /* Get current point */
		*x = getXCur();
	    *y = getYCur();
	    break;
	  case 2: /* start contour at boundary */
	  case 3: /* start contour not at boundary */
		if (smooth) addVect(1, sx(*x), sy(*y));
		else {
			addUnknown();
			addPointScale(*x, *y);
		}
		addLabel(sx(*x),sy(*y), ih-1, getCValue(ih-1));
		break;
	  case 1: /* continue a contour */
		if (smooth) addVect(2, sx(*x), sy(*y));
		else addPointScale(*x, *y);
		break;
	  case 4: /* finish contour at a boundary */
		if (smooth) addVect(4, sx(*x), sy(*y));
		else addPointScale(*x, *y);
		break;
	  case 5: /* finish close contour */
		if (smooth) addVect(3, sx(*x), sy(*y));
		else addPointScale(*x, *y);
		break;
	}
	setXCur(*x);
	setYCur(*y);
}

void GLEContourInfo::addVect(int m, double x, double y) {
	if (m == 1) {
		if (getNbDataPoints() != 0) printf("Error, some points not drawn \n");
		clearDataPoints();
	}
	int nbpts = getNbDataPoints();
	if (nbpts > 0 && getDataX(nbpts-1) == x && getDataY(nbpts-1) == y) {
		if (m <= 2) addDataPoint(x, y);
	} else {
		addDataPoint(x, y);
	}
	// Connect the ends by adding some points if contour is right sort
	// 4 = boundary, 3 = closed shape
	if (m == 3 || m == 4) {
		if (nbpts < 2) {
			addAllDataPoints();
			clearDataPoints();
			return;
		}
		bool snipends = false;
		if (m == 3) {
			snipends = true;
			int nbpts = getNbDataPoints();
			addDataPoint(getDataX(nbpts-1), getDataY(nbpts-1));
			for (int i = nbpts-1; i > 0; i--) {
				setDataPoint(i, getDataX(i-1), getDataY(i-1));
			}
			setDataPoint(0, getDataX(nbpts-1), getDataY(nbpts-1));
			addDataPoint(getDataX(2), getDataY(2));
		}
		int ninp = getNbDataPoints();
		int mode = 2; 		// multi valued
		// int nsub = smoothsub;	// user parameter
		// if (nsub < 3) nsub = 3;

		int nsub = 10;
		int nout = (ninp-1)*nsub+1;
		cout << "nsub = " << nsub << endl;
		double* xout = (double*)malloc(nout*sizeof(double));
		double* yout = (double*)malloc(nout*sizeof(double));
		glefitcf_(&mode,getDataXArray(),getDataYArray(),&ninp,&nsub,xout,yout,&nout);
		clearDataPoints();
		addUnknown();
		if (snipends) {
		 	for (int i = nsub; i < nout-nsub; i++) {
				addPoint(xout[i], yout[i]);
		 	}
		} else {
			cout << "nin = " << ninp << " nout = " << nout << endl;
			for (int i = 0; i < nout; i++) {
				addPoint(xout[i], yout[i]);
		 	}
		}
		free(xout); free(yout);
	}
}

void get_contour_values(GLEContourInfo* info, int ct) {
	double from, to, step;
	bool has_from = false;
	bool has_to = false;
	bool has_step = false;
	while (ct < ntk) {
		if (str_i_equals(tk[ct+1], "FROM")) {
			ct++;
			from = get_next_exp(tk, ntk, &ct);
			has_from = true;
		} else if (str_i_equals(tk[ct+1], "TO")) {
			ct++;
			to = get_next_exp(tk, ntk, &ct);
			has_to = true;
		} else if (str_i_equals(tk[ct+1], "STEP")) {
			ct++;
			step = get_next_exp(tk, ntk, &ct);
			has_step = true;
		} else {
			double v = get_next_exp(tk, ntk, &ct);
			info->addValue(v);
		}
	}
	if (has_from && has_to && has_step) {
		info->fillDefault(from, to, step);
	}
}

void begin_contour(int *pln, int *pcode, int *cp) {
	string data_file;
	vector<double> cval;
	vector<string> clab;
	// Create new contourinfo object
	if (g_ContourInfo != NULL) {
		delete g_ContourInfo;
		g_ContourInfo = NULL;
	}
	g_ContourInfo = new GLEContourInfo();
	// Start with pcode from the next line
	(*pln)++;
	begin_init();
	while (true) {
		int st = begin_token(&pcode,cp,pln,srclin,tk,&ntk,outbuff);
		if (!st) {
			/* exit loop */
			break;
		}
		int ct = 1;
		// cout << "ct = " << ct << " tk = " << ntk << " tk = " << tk[ct] << endl;
		kw("DATA") {
			next_file_eval(data_file);
			g_ContourInfo->read(data_file);
		} else kw("VALUES") {
			get_contour_values(g_ContourInfo, ct);
		} else kw("LABELS") {
			// get_contour_label_sett(&alpha);
		} else kw("SMOOTH") {
			smoothsub = atoi(tk[++ct]);
		} else if (ct <= ntk) {
			stringstream err;
			err << "illegal keyword in contour block: '" << tk[ct] << "'";
			g_throw_parser_error(err.str());
		}
	}
	int nx = g_ContourInfo->getNX();
	int ny = g_ContourInfo->getNY();
	double zmin = g_ContourInfo->getZMin();
	double zmax = g_ContourInfo->getZMax();
	if (g_ContourInfo->getNbLines() == 0) {
		g_ContourInfo->fillDefault(zmin, zmax, (zmax-zmin)/10.0);
	}
	int zdim = nx;
	g_ContourInfo->createLabels(true);
	string dat_file, val_file, lab_file;
	GetMainName(data_file, dat_file);
	GetMainName(data_file, val_file);
	GetMainName(data_file, lab_file);
	dat_file += "-cdata.dat";
	val_file += "-cvalues.dat";
	lab_file += "-clabels.dat";
	FILE* val_f = validate_fopen(val_file, "w", false);
	if (val_f != NULL) {
		for (int i = 0; i < g_ContourInfo->getNbLines(); i++) {
			fprintf(val_f, "%g\n", g_ContourInfo->getCValue(i));
		}
		fclose(val_f);
	}
	g_ContourInfo->openData(dat_file, lab_file);
	g_ContourInfo->doContour(g_ContourInfo->getData(),zdim,nx,ny,zmax);
	g_ContourInfo->closeData();
}

void plot_(double *a, double *b, int *c) {
}

void draw_(double *x, double *y, int *iflag) {
	g_ContourInfo->draw(x, y, *iflag);
}
