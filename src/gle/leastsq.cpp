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

#include <math.h>
#include <vector>

using namespace std;

#include "all.h"
#include "gprint.h"
#include "leastsq.h"

GLEPowellFunc::GLEPowellFunc() {
}

GLEPowellFunc::~GLEPowellFunc() {
}

void least_square(vector<double>* x,vector<double>* y,double* slope, double* offset, double* rsquared) {
	//
	// does a linear least squares fit to the data x and y which have n elements
	// x,y.... the values
	// n the size of the x and y arrays
	// slope... the tresulting slope
	// offset.. the resulting offset
	// rsquared.. the goodness of fit 1=ideal 0 = poor
	//
	double sum_x=0, sum_y=0, sum_xy=0, sum_xx=0, delta=0, y_avg = 0, n = static_cast<double>(x->size());
	for (size_t i = 0; i < x->size(); i++) {
		// printf("x[%d]=%0.6f y[%d]=%0.6f\n",i,(*x)[i],i,(*y)[i]);
		sum_x += (*x)[i];
		sum_y += (*y)[i];
		sum_xy += (*x)[i]*(*y)[i];
		sum_xx += (*x)[i]*(*x)[i];
	}
	delta = n*sum_xx - sum_x*sum_x;
	*slope = (n*sum_xy - sum_x*sum_y)/delta;
	*offset = (sum_xx*sum_y - sum_x*sum_xy)/delta;
	y_avg = sum_y / n;
	//
	// compute rsquared
	//
	*rsquared = 0;
	double residue = 0.0,syy=0.0;

	for (int i = 0; i < n; i++) {
		residue += pow((*y)[i] - (*slope) * (*x)[i] - *offset,2.0);
		syy += pow((*y)[i] - y_avg,2.0);
	}

	*rsquared = 1 - residue/syy;
}

double *mk_vector(int nl, int nh);
void free_vector(double* v, int nl, int nh);
void mnbrak(double* ax, double* bx, double* cx, double *fa, double* fb, double* fc, double (*func)(double));

#define ITMAX 100
#define CGOLD 0.3819660
#define ZEPS 1.0e-10
#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

double brent(double ax, double bx, double cx, double (*f)(double), double tol, double *xmin) {
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
			*xmin=x;
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
					d=SIGN(tol1,xm-x);
			}
		} else {
			d=CGOLD*(e=(x >= xm ? a-x : b-x));
		}
		u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
		fu=f(u);
		if (fu <= fx) {
			if (u >= x) a=x; else b=x;
			SHFT(v,w,x,u)
			SHFT(fv,fw,fx,fu)
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
	*xmin=x;
	return fx;
}

#undef ITMAX
#undef CGOLD
#undef ZEPS
#undef SIGN

int ncom=0;
double *pcom=0,*xicom=0;
GLEPowellFunc* nrfunc;

double f1dim(double x) {
	int j;
	double f,*xt;

	xt=mk_vector(1,ncom);
	for (j=1;j<=ncom;j++) xt[j]=pcom[j]+x*xicom[j];
	f=nrfunc->fitMSE(xt);
	free_vector(xt,1,ncom);
	return f;
}

#define TOL 2.0e-4

void linmin(double* p, double* xi, int n, double* fret, GLEPowellFunc* func) {
	int j;
	double xx,xmin,fx,fb,fa,bx,ax;

	ncom=n;
	pcom=mk_vector(1,n);
	xicom=mk_vector(1,n);
	nrfunc=func;
	for (j=1;j<=n;j++) {
		pcom[j]=p[j];
		xicom[j]=xi[j];
	}
	ax=0.0;
	xx=1.0;
	bx=2.0;
	mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,f1dim);
	*fret=brent(ax,xx,bx,f1dim,TOL,&xmin);
	for (j=1;j<=n;j++) {
		xi[j] *= xmin;
		p[j] += xi[j];
	}
	free_vector(xicom,1,n);
	free_vector(pcom,1,n);
}

#undef TOL
#include <math.h>

#define GOLD 1.618034
#define GLIMIT 100.0
#define TINY 1.0e-20
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SIGN(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

void mnbrak(double* ax, double* bx, double* cx, double *fa, double* fb, double* fc, double (*func)(double)) {
	double ulim,u,r,q,fu,dum;

	*fa=(*func)(*ax);
	*fb=(*func)(*bx);
	if (*fb > *fa) {
		SHFT(dum,*ax,*bx,dum)
		SHFT(dum,*fb,*fa,dum)
	}
	*cx=(*bx)+GOLD*(*bx-*ax);
	*fc=(*func)(*cx);
	while (*fb > *fc) {
		r=(*bx-*ax)*(*fb-*fc);
		q=(*bx-*cx)*(*fb-*fa);
		u=(*bx)-((*bx-*cx)*q-(*bx-*ax)*r)/
			(2.0*SIGN(MAX(fabs(q-r),TINY),q-r));
		ulim=(*bx)+GLIMIT*(*cx-*bx);
		if ((*bx-u)*(u-*cx) > 0.0) {
			fu=(*func)(u);
			if (fu < *fc) {
				*ax=(*bx);
				*bx=u;
				*fa=(*fb);
				*fb=fu;
				return;
			} else if (fu > *fb) {
				*cx=u;
				*fc=fu;
				return;
			}
			u=(*cx)+GOLD*(*cx-*bx);
			fu=(*func)(u);
		} else if ((*cx-u)*(u-ulim) > 0.0) {
			fu=(*func)(u);
			if (fu < *fc) {
				SHFT(*bx,*cx,u,*cx+GOLD*(*cx-*bx))
				SHFT(*fb,*fc,fu,(*func)(u))
			}
		} else if ((u-ulim)*(ulim-*cx) >= 0.0) {
			u=ulim;
			fu=(*func)(u);
		} else {
			u=(*cx)+GOLD*(*cx-*bx);
			fu=(*func)(u);
		}
		SHFT(*ax,*bx,*cx,u)
		SHFT(*fa,*fb,*fc,fu)
	}
}

#undef GOLD
#undef GLIMIT
#undef TINY
#undef MAX
#undef SIGN
#undef SHFT
#include <math.h>

#define ITMAX 200
static double sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)

void powell(double p[],double **xi, int n, double ftol, int *iter, double *fret, GLEPowellFunc* func) {
	int i,ibig,j;
	double t,fptt,fp,del;
	double *pt,*ptt,*xit;

	pt=mk_vector(1,n);
	ptt=mk_vector(1,n);
	xit=mk_vector(1,n);
	*fret = func->fitMSE(p);
	for (j=1;j<=n;j++) pt[j]=p[j];
	for (*iter=1;;(*iter)++) {
		fp=(*fret);
		ibig=0;
		del=0.0;
		for (i=1;i<=n;i++) {
			for (j=1;j<=n;j++) xit[j]=xi[j][i];
			fptt=(*fret);
			linmin(p,xit,n,fret,func);
			if (fabs(fptt-(*fret)) > del) {
				del=fabs(fptt-(*fret));
				ibig=i;
			}
		}
		if (2.0*fabs(fp-(*fret)) <= ftol*(fabs(fp)+fabs(*fret))) {
			free_vector(xit,1,n);
			free_vector(ptt,1,n);
			free_vector(pt,1,n);
			return;
		}
		if (*iter == ITMAX) {
			gprint("Too many iterations in routine POWELL\n");
		}
		for (j=1;j<=n;j++) {
			ptt[j]=2.0*p[j]-pt[j];
			xit[j]=p[j]-pt[j];
			pt[j]=p[j];
		}
		fptt=func->fitMSE(ptt);
		if (fptt < fp) {
			t=2.0*(fp-2.0*(*fret)+fptt)*SQR(fp-(*fret)-del)-del*SQR(fp-fptt);
			if (t < 0.0) {
				linmin(p,xit,n,fret,func);
				for (j=1;j<=n;j++) xi[j][ibig]=xit[j];
			}
		}
	}
}

#undef ITMAX
#undef SQR

#include <stdio.h>

double *mk_vector(int nl, int nh) {
	double *v;
	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) gle_abort("allocation failure in vector()");
	return v-nl;
}

double **matrix(int nrl, int nrh, int ncl, int nch) {
	int i;
	double **m;
	m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*));
	if (!m) gle_abort("allocation failure 1 in matrix()");
	m -= nrl;
	for(i=nrl;i<=nrh;i++) {
		m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) gle_abort("allocation failure 2 in matrix()");
		m[i] -= ncl;
	}
	return m;
}

void free_vector(double* v, int nl, int nh) {
	free((char*) (v+nl));
}

void free_matrix(double** m, int nrl, int nrh, int ncl, int nch) {
	for(int i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}
