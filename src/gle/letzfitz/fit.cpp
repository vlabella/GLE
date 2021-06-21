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

/*
 * Surface fitting program,  creates .z file from data points
 */

#include "../all.h"
#include "../tokens/Tokenizer.h"
#include "../core.h"
#include "../file_io.h"
#include "../texinterface.h"
#include "../cutils.h"
#include "../gprint.h"
#include "../cmdline.h"
#include "../config.h"
#include "../mem_limits.h"
#include "../token.h"
#include "../glearray.h"
#include "../polish.h"
#include "../var.h"
#include "../gprint.h"
#include "../keyword.h"
#include "../run.h"

#define BEGINDEF extern
#include "../begin.h"

double get_next_exp(TOKENS tk, int ntk, int *curtok);
void get_next_exp_file(TOKENS tk, int ntok, int *curtok, string* res);

#define kw(ss) if (ct <= ntk && str_i_equals(tk[ct],ss))
#define next_file_eval(s) get_next_exp_file(tk,ntk,&ct,&s)
#define next_str(s) ct+=1;s=tk[ct]

void sort_data(int npnts, double *xd, double *yd, double *zd);

int idbvip_(int *md, int *ncp, int *ndp, double *xd, double *yd, double *zd, int *nip, double *xi, double *yi, double *zi, int *iwk, double *wk);

class GLEFitZData {
public:
	int ncp;
	double ymin, xmin, xstep;
	double xmax, ymax, ystep;
	vector<double> pntxyz;
	vector<double> dx, dy, dz;
	string data_file;
public:
	GLEFitZData();
	void loadData() throw(ParserError);
	void sortData() throw(ParserError);
};

GLEFitZData::GLEFitZData() {
	xmin = ymin = +10e10;
	xmax = ymax = -10e10;
	ncp = 3;
}

// different from the one in let.cpp because fields are optional here
void get_from_to_step_fitz(TOKENS tk, int ntok, int *curtok, double* from, double* to, double* step) throw(ParserError) {
	(*curtok) = (*curtok) + 1;
	if ((*curtok) >= ntok) {
		return;
	}
	if (str_i_equals(tk[(*curtok)], "FROM")) {
		*from = get_next_exp(tk, ntok, curtok);
		(*curtok) = (*curtok) + 1;
	}
	if ((*curtok) >= ntok) {
		return;
	}
	if (str_i_equals(tk[(*curtok)], "TO")) {
		*to = get_next_exp(tk, ntok, curtok);
		(*curtok) = (*curtok) + 1;
	}
	if ((*curtok) >= ntok) {
		return;
	}
	if (str_i_equals(tk[(*curtok)], "STEP")) {
		*step = get_next_exp(tk, ntok, curtok);
		(*curtok) = (*curtok) + 1;
	}
	if ((*curtok) < ntok) {
		stringstream err;
		err << "illegal keyword in range expression '" << tk[(*curtok)] << "'";
		g_throw_parser_error(err.str());
	}
	if (*from >= *to) {
		ostringstream err;
		err << "from value (" << *from << ") should be strictly smaller than to value (" << *to << ") in letz block";
		g_throw_parser_error(err.str());
	}
	if (*step <= 0.0) {
		ostringstream err;
		err << "step value (" << *from << ") should be strictly positive in letz block";
		g_throw_parser_error(err.str());
	}
}

void begin_fitz(int *pln, int *pcode, int *cp) throw(ParserError) {
	GLEFitZData data;
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
			next_file_eval(data.data_file);
			data.loadData();
			data.sortData();
		} else kw("X") {
			get_from_to_step_fitz(tk, ntk, &ct, &data.xmin, &data.xmax, &data.xstep);
		} else kw("Y") {
			get_from_to_step_fitz(tk, ntk, &ct, &data.ymin, &data.ymax, &data.ystep);
		} else kw("NCONTOUR") {
			data.ncp = atoi(tk[++ct]);
		} else if (ct <= ntk) {
			stringstream err;
			err << "illegal keyword in fitz block: '" << tk[ct] << "'";
			g_throw_parser_error(err.str());
		}
	}
/*
	char bfile[80],dfile[80],xrange[80];
	real *xd, *yd, *zd;
	real *rx, *ry, *rz;
	int32 nx,ny;
	int32 nrp;
	int32 *iwk;
	real *wk;
	int32 i,j,k;
*/
/*
	printf("Number of points to use for contouring each point ? [3] "); ncp = getf();
	if (ncp==0) ncp = 3;
	printf("Range of output x values [%g,%g,%g] ? ",xmin,xmax,xstep); getstr(xrange);
	getrange(xrange,&xmin,&xmax,&xstep);
	printf("Range of output y values [%g,%g,%g] ? ",ymin,ymax,ystep); getstr(xrange);
	getrange(xrange,&ymin,&ymax,&ystep);
*/
	double xmax = data.xmax;
	double xmin = data.xmin;
	double ymax = data.ymax;
	double ymin = data.ymin;
	double xstep = data.xstep;
	double ystep = data.ystep;
	int nx = (int)floor((xmax-xmin)/xstep + 1);
	int ny = (int)floor((ymax-ymin)/ystep + 1);
	vector<double> rx0, ry0, rz0;
	double y = ymin;
	for (int j = 0; j < ny; j++) {
		double x = xmin;
		for (int i = 0; i < nx; i++) {
			rx0.push_back(x);
			ry0.push_back(y);
			rz0.push_back(0);
			x += xstep;
		}
		y += ystep;
	}
	int ncp = data.ncp;
	int ndp = data.dx.size();
	int md = 1;
	int nrp = nx*ny;
	int i = 27+ncp;
	if (i<31) i = 31;
	i = (i * ndp + nrp) * sizeof(int);
	int _j = 8*ndp*sizeof(double); // VL: _j to silence msvc warning about conflicting scope with j in for loop
	int* iwk = (int*)malloc(i);
	double* wk = (double*)malloc(_j);
	if (iwk == NULL || wk == NULL) {
		stringstream err;
		err << "unable to allocate enough workspace iwk = " << i << " wk = " << _j;
		g_throw_parser_error(err.str());
	}
	double* xd = (double*)&data.dx[0];
	double* yd = (double*)&data.dy[0];
	double* zd = (double*)&data.dz[0];
	double* rx = (double*)&rx0[0];
	double* ry = (double*)&ry0[0];
	double* rz = (double*)&rz0[0];
	idbvip_(&md,&ncp,&ndp,xd,yd,zd,&nrp,rx,ry,rz,iwk,wk);
	string out_file;
	GetMainName(data.data_file, out_file);
	out_file += ".z";
	/* Save output to file */
	FILE* fp = validate_fopen(out_file, "wb", false);
	if (fp == NULL) {
		stringstream err;
		err << "unable to create .z file '" << out_file << "'";
		g_throw_parser_error(err.str());
	}
	fprintf(fp,"! nx %d ny %d xmin %g xmax %g ymin %g ymax %g\n",nx,ny,xmin,xmax,ymin,ymax);
	/* Write data matrix to file */
	int k = 0;
	y = ymin;
	for (int j = 0; j < ny; j++) {
		double x = xmin;
		for (int i = 0; i < nx; i++) {
			fprintf(fp,"%g ",rz[k++]);
			x += xstep;
		}
		fprintf(fp,"\n");
		y += ystep;
	}
	fclose(fp);
}

void setminmax(double v, double *min, double *max) {
	if (v< *min) *min = v;
	if (v> *max) *max = v;
}

void GLEFitZData::loadData() throw(ParserError) {
	TokenizerLanguage lang;
	StreamTokenizer tokens(&lang);
	string expanded(GLEExpandEnvironmentVariables(data_file));
	validate_file_name(expanded, false);
	tokens.open_tokens(expanded.c_str());
	lang.setLineCommentTokens("!");
	lang.setSpaceTokens(" \t\r,");
	lang.setSingleCharTokens("\n");
	while (tokens.has_more_tokens()) {
		if (!tokens.is_next_token("\n")) {
			for (int i = 0; i < 3; i++) {
				string& token = tokens.next_token();
				if (is_float(token)) {
					pntxyz.push_back(atof(token.c_str()));
				} else {
					stringstream err;
					err << "not a valid number: '" << token << "'";
					throw tokens.error(err.str());
				}
			}
			string& token = tokens.next_token();
			if (token != "\n") {
				throw tokens.error("more than 3 columns in data file");
			}
		}
	}
}

void GLEFitZData::sortData() throw(ParserError) {
	/* Copy data */
	for (vector<double>::size_type i = 0; i < pntxyz.size(); i+=3) {
		double xp = pntxyz[i];
		double yp = pntxyz[i+1];
		double zp = pntxyz[i+2];
		dx.push_back(xp);
		dy.push_back(yp);
		dz.push_back(zp);
		setminmax(xp,&xmin,&xmax);
		setminmax(yp,&ymin,&ymax);
	}
	pntxyz.clear();
	if (dx.empty()) {
		g_throw_parser_error("empty data file in fitz block");
	}
	sort_data(dx.size(), (double*)&dx[0], (double*)&dy[0], (double*)&dz[0]);
	for (vector<double>::size_type i = 0; i < dx.size()-1; i++) {
		if (dx[i] == dx[i+1] && dy[i] == dy[i+1]) {
			stringstream err;
			err << "duplicate data point: (" << dx[i] << "," << dy[i]  << "," << dz[i] << ")";
			g_throw_parser_error(err.str());
		}
	}
	xstep = (xmax-xmin)/15.0;
	ystep = (ymax-ymin)/15.0;
}

double *xxx,*yyy,*zzz;

void myswap(int i, int j) {
	static double a;
	a = xxx[i]; xxx[i] = xxx[j]; xxx[j] = a;
	a = yyy[i]; yyy[i] = yyy[j]; yyy[j] = a;
	a = zzz[i]; zzz[i] = zzz[j]; zzz[j] = a;
}

int mycmp(int i, double x1, double y1) {
	if (xxx[i]<x1) return -1;
	if (xxx[i]>x1) return 1;
	if (yyy[i]<y1) return -1;
	if (yyy[i]>y1) return 1;
	return 0;
}

void (*ffswap) (int i, int j);
int (*ffcmp) (int i, double x1, double x2);

void qquick_sort(int left, int right) {
	int i,j,xx;
	double x1,x2;
	i = left; j = right;
	xx = (left+right)/2;
	x1 = xxx[xx]; x2 = yyy[xx];
	do {
		while((*ffcmp)(i,x1,x2)<0 && i<right) i++;
		while((*ffcmp)(j,x1,x2)>0 && j>left) j--;
		if (i<=j) { (*ffswap)(i,j); i++; j--; }
	} while (i<=j);
	if (left<j) qquick_sort(left,j);
	if (i<right) qquick_sort(i,right);
}

void quick_sort(int nitems, void (*fswap) (int i, int j), int (*fcmp) (int i, double x1, double x2)) {
	if (nitems <= 0) {
		return;
	}
	ffswap = fswap;
	ffcmp = fcmp;
	qquick_sort(0,nitems-1);
}

void sort_data(int npnts, double *xd, double *yd, double *zd) {
	xxx = xd; yyy = yd; zzz = zd;
	quick_sort(npnts,myswap,mycmp);
}
