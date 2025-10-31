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

#include <math.h>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <boost/math/constants/constants.hpp>
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
#include "gprint.h"
#include "numberformat.h"
#include "gle-fitls.h"

using namespace std;

double **matrix(int nrh, int nch);
void free_matrix(double** m, int nrh, int nch);
void powell(double *p, double **xi, int n, double ftol, int& iter, double& fret, GLEPowellFunc* func);

GLEPowellFunc::GLEPowellFunc() {
}

GLEPowellFunc::~GLEPowellFunc() {
}

GLEFitLS::GLEFitLS() {
	m_IdxX = -1;
	m_NIter = 0;
	m_RSquare = 0.0;
	m_Function = new GLEFunctionParserPcode();
}

GLEFitLS::~GLEFitLS() {
}

void GLEFitLS::polish(const string& str) {
	m_FunctionStr = str;
	m_Function->polish(str.c_str(), &m_VarMap);
	/* Iterate over variables in expression */
	for (StringIntHash::const_iterator i = m_VarMap.begin(); i != m_VarMap.end(); i++ ) {
		if (i->first != "X") {
			m_Vars.push_back(i->second);
		}
	}
}

void GLEFitLS::setXY(vector<double>* x, vector<double>* y) {
	m_X = x;
	m_Y = y;
}

void GLEFitLS::testFit() {
	int nxy = m_X->size();
	double sumy = 0.0;
	for (int i = 0; i < nxy; i++) {
		sumy = sumy + (*m_Y)[i];
	}
	double meany = sumy/nxy;
	double sum1 = 0.0, sum2 = 0.0;
	for (int i = 0; i < nxy; i++) {
		var_set(m_IdxX, (*m_X)[i]);
		double value = m_Function->evalDouble();
		double y = (*m_Y)[i];
		double r1 = value - y;
		double r2 = meany - y;
		sum1 = sum1 + r1*r1;
		sum2 = sum2 + r2*r2;
	}
	m_RSquare = 1 - sum1/sum2;
}

double GLEFitLS::fitMSE(double* vals) {
	/* Set all variables to given values */
	setVarsVals(vals);
	/* Compute MSE */
	double tot = 0.0;
	for (vector<double>::size_type i = 0; i < m_X->size(); i++) {
		var_set(m_IdxX, (*m_X)[i]);
		double value = m_Function->evalDouble();
		double residue = (*m_Y)[i] - value;
		tot += residue*residue;
	}
	/* Return error */
	return tot / m_X->size();
}

void GLEFitLS::toFunctionStr(const string& format, string* str) {
	*str = "";
	string fmt_str = format;
	if (fmt_str == "") fmt_str = "fix 3";
	GLENumberFormat fmt(fmt_str);
	GLEPolish* polish = get_global_polish();
	Tokenizer* tokens = polish->getTokens(m_FunctionStr);
	string uc_token, v_str;
	bool has_plus = false;
	while (tokens->has_more_tokens()) {
		const string& token = tokens->next_token();
		str_to_uppercase(token, uc_token);
		int v_idx = m_VarMap.try_get(uc_token);
		if (uc_token != "X" && v_idx != -1) {
			double value;
			var_get(v_idx, &value);
			fmt.format(value, &v_str);
			if (has_plus && value >= 0) *str = *str + "+";
			*str = *str + v_str;
			has_plus = false;
		} else {
			if (has_plus) *str = *str + "+";
			has_plus = token == "+";
			if (!has_plus) *str = *str + token;
		}
	}
}

void GLEFitLS::setVarsVals(double* vals) {
	/* Set all variables to given values */
	int naz = m_Vars.size();
	for (int i = 0; i < naz; i++) {
		int v_idx = m_Vars[i];
		if (v_idx >= 0) var_set(v_idx, vals[i]);
	}
}

void GLEFitLS::fit() {
	int naz = m_Vars.size();
	double** xi = matrix(naz,naz);
	for (int i = 0; i < naz; i++) {
		for (int j = 0; j < naz; j++) {
			xi[i][j] = 0.0;
		}
		xi[i][i] = 1.0;
	}
	double* pms = new double[naz];
	for (int i = 0; i < naz; i++) {
		int v_idx = m_Vars[i];
		var_get(v_idx, &pms[i]);
	}
	int vtype;
    double fret   = 0.0;
    double anstol = 1.0e-4;
	var_findadd("X", &m_IdxX, &vtype);
	powell(pms, xi, naz, anstol, m_NIter, fret, this);
	free_matrix(xi,naz,naz);
	setVarsVals(pms);
}

// replace these macros with C++ functions
//#define MAX(a,b) ((a) > (b) ? (a) : (b))
// use std::max

//#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
// use std::copysign(a, b);

//#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);
inline void shift(double& a, double& b, double& c, const double& d) {
    a = b;
    b = c;
    c = d;
}
//static double sqrarg;
//#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)
inline double square(double a) {
    return a * a;
}

double *mk_vector(int nh) {
	double *v;
	v=(double *)malloc((unsigned) nh*sizeof(double));
	if (!v) gle_abort("allocation failure in vector()");
	return v;
}

void free_vector(double* v) {
	free(v);
}

double **matrix(int nrh, int nch) {
	int i;
	double **m;
	m=(double **) malloc((unsigned) nrh*sizeof(double*));
	if (!m) gle_abort("allocation failure 1 in matrix()");
	//m -= nrl;
	for(i=0;i<nrh;i++) {
		m[i]=(double *) malloc((unsigned) nch*sizeof(double));
		if (!m[i]) gle_abort("allocation failure 2 in matrix()");
		//m[i] -= ncl;
	}
	return m;
}

void free_matrix(double** m, int nrh, int nch) {
	for(int i=nrh-1;i>=0;i--) free(m[i]);
	free(m);
}

double brent(double ax, double bx, double cx, double (*f)(double), double tol, double& xmin) {
	//#define ITMAX 100
	//#define CGOLD 0.3819660
	//#define ZEPS 1.0e-10
    const double ITMAX = 100;
    //const double CGOLD = 0.3819660;
    constexpr double CGOLD = 1.0/(1.0+boost::math::double_constants::phi);
    constexpr double ZEPS = std::numeric_limits<double>::epsilon() * 1.0e-3;

    int iter;
	double a,b,d = 0.0,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
	double e=0.0;

	a=((ax < cx) ? ax : cx);
	b=((ax > cx) ? ax : cx);
	x = w = v = bx;
	fw = fv = fx = f(x);
	for (iter=1;iter<=ITMAX;iter++) {
		xm=0.5*(a+b);
		tol2=2.0*(tol1=tol*fabs(x)+ZEPS);
		if (fabs(x-xm) <= (tol2-0.5*(b-a))) {
			xmin=x;
			return fx;
		}
		if (fabs(e) > tol1) {
			r=(x-w)*(fx-fv);
			q=(x-v)*(fx-fw);
			p=(x-v)*q-(x-w)*r;
			q=2.0*(q-r);
			if (q > 0.0) p = -p;
			q=fabs(q);
			etemp=e;
			e=d;
			if (fabs(p) >= fabs(0.5*q*etemp) ||
				p <= q*(a-x) || p >= q*(b-x)) {
				d=CGOLD*(e=(x >= xm ? a-x : b-x));
			} else {
				d=p/q;
				u=x+d;
				if (u-a < tol2 || b-u < tol2)
					d=std::copysign(tol1,xm-x);
			}
		} else {
			d=CGOLD*(e=(x >= xm ? a-x : b-x));
		}
		u=(fabs(d) >= tol1 ? x+d : x+std::copysign(tol1,d));
		fu=f(u);
		if (fu <= fx) {
			if (u >= x) a=x; else b=x;
			shift(v,w,x,u);
			shift(fv,fw,fx,fu);
		} else {
			if (u < x) a=u; else b=u;
			if (fu <= fw || w == x) {
				v=w;
				w=u;
				fv=fw;
				fw=fu;
			} else if (fu <= fv || v == x || v == w) {
				v=u;
				fv=fu;
			}
		}
	}
	gprint("Too many iterations in BRENT\n");
	xmin=x;
	return fx;
}

void mnbrak(double& ax, double& bx, double& cx, double& fa, double& fb, double& fc, double (*func)(double)) {
	//#define GOLD 1.618034
	//#define GLIMIT 100.0
	//#define TINY 1.0e-20
    const double GOLD   = boost::math::double_constants::phi;
    const double GLIMIT = 100.0;
    const double TINY   = 1.0e-20;

	double ulim,u,r,q,fu,dum;

	fa=(*func)(ax);
	fb=(*func)(bx);
	if (fb > fa) {
		shift(dum,ax,bx,dum);
		shift(dum,fb,fa,dum);
	}
	cx=bx+GOLD*(bx-ax);
	fc=(*func)(cx);
	while (fb > fc) {
		r=(bx-ax)*(fb-fc);
		q=(bx-cx)*(fb-fa);
		u=(bx)-((bx-cx)*q-(bx-ax)*r)/
			(2.0*std::copysign(std::max(fabs(q-r),TINY),q-r));
		ulim=(bx)+GLIMIT*(cx-bx);
		if ((bx-u)*(u-cx) > 0.0) {
			fu=(*func)(u);
			if (fu < fc) {
				ax=bx;
				bx=u;
				fa=fb;
				fb=fu;
				return;
			} else if (fu > fb) {
				cx=u;
				fc=fu;
				return;
			}
			u=cx+GOLD*(cx-bx);
			fu=(*func)(u);
		} else if ((cx-u)*(u-ulim) > 0.0) {
			fu=(*func)(u);
			if (fu < fc) {
				shift(bx,cx,u,cx+GOLD*(cx-bx));
				shift(fb,fc,fu,(*func)(u));
			}
		} else if ((u-ulim)*(ulim-cx) >= 0.0) {
			u=ulim;
			fu=(*func)(u);
		} else {
			u=cx+GOLD*(cx-bx);
			fu=(*func)(u);
		}
		shift(ax,bx,cx,u);
		shift(fa,fb,fc,fu);
	}
}


int ncom=0;
double *pcom=0,*xicom=0;
GLEPowellFunc* nrfunc;

double f1dim(double x) {
	int j;
	double f,*xt;
	xt=mk_vector(ncom);
	for (j=0;j<ncom;j++) xt[j]=pcom[j]+x*xicom[j];
	f=nrfunc->fitMSE(xt);
	free_vector(xt);
	return f;
}

void linmin(double* p, double* xi, int n, double& fret, GLEPowellFunc* func) {
	//#define TOL 2.0e-4
	const double TOL = 2.0e-4;
	int j;
	double xx,xmin,fx,fb,fa,bx,ax;

	ncom=n;
	pcom=mk_vector(n);
	xicom=mk_vector(n);
	nrfunc=func;
	for (j=0;j<n;j++) {
		pcom[j]=p[j];
		xicom[j]=xi[j];
	}
	ax=0.0;
	xx=1.0;
	bx=2.0;
	mnbrak(ax,xx,bx,fa,fx,fb,f1dim);
	fret=brent(ax,xx,bx,f1dim,TOL,xmin);
	for (j=0;j<n;j++) {
		xi[j] *= xmin;
		p[j] += xi[j];
	}
	free_vector(xicom);
	free_vector(pcom);
	//#undef TOL
}

void powell(double p[],double **xi, int n, double ftol, int& iter, double& fret, GLEPowellFunc* func)
{
	//#define ITMAX 200
	const int ITMAX = 200;
	int i,ibig,j;
	double t,fptt,fp,del;
	double *pt,*ptt,*xit;

	pt=mk_vector(n);
	ptt=mk_vector(n);
	xit=mk_vector(n);
	fret = func->fitMSE(p);
	for (j=0;j<n;j++) pt[j]=p[j];
	for (iter=1 ;; iter++) {
		fp=fret;
		ibig=0;
		del=0.0;
		for (i=0;i<n;i++) {
			for (j=0;j<n;j++) xit[j]=xi[j][i];
			fptt=fret;
			linmin(p,xit,n,fret,func);
			if (fabs(fptt-fret) > del) {
				del=fabs(fptt-fret);
				ibig=i;
			}
		}
		if (2.0*fabs(fp-fret) <= ftol*(fabs(fp)+fabs(fret))) {
			free_vector(xit);
			free_vector(ptt);
			free_vector(pt);
			return;
		}
		if (iter == ITMAX) {
			gprint("Too many iterations in routine POWELL\n");
			return;
		}
		for (j=0;j<n;j++) {
			ptt[j]=2.0*p[j]-pt[j];
			xit[j]=p[j]-pt[j];
			pt[j]=p[j];
		}
		fptt=func->fitMSE(ptt);
		if (fptt < fp) {
			t=2.0*(fp-2.0*fret+fptt)*square(fp-fret-del)-del*square(fp-fptt);
			if (t < 0.0) {
				linmin(p,xit,n,fret,func);
				for (j=0;j<n;j++) xi[j][ibig]=xit[j];
			}
		}
	}
}
