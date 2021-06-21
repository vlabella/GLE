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

/* fitcf.f -- translated by f2c (version of 26 February 1990  17:38:00).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include "all.h"
#include "fitcf.h"
#include "gprint.h"

void gd_message__(const char *s,int l)
{
	gprint("%s  %ld \n",s,l);
}


/* --   GUTRE2 */
doublereal gutre2_(real* a,real* b)
/*real *a, *b;*/
{
    /* Initialized data */

    static real zero = (float)0.;
    static real two = (float)2.;
    static real four = (float)4.;

    /* System generated locals */
    real ret_val, r_1;

    /* Local variables */
    static real aabs, babs, r, s;

/*     (Euclidean Norm of 2-Vector) */
/*     Return SQRT(A**2+B**2)  without doing  a square  root,  and */
/*     without destructive underflow or overflow. */
/*     (Cleve Moler, University of New Mexico) */
/*     (09-APR-82) */


    aabs = dabs(*a);
    babs = dabs(*b);
/* --- IF (BABS .GT. AABS) THEN -- SWAP A AND B */
    if (babs <= aabs) {
	goto L10;
    }
    r = babs;
    babs = aabs;
    aabs = r;
/* --- END IF */
/* --- IF (BABS .EQ. ZERO) THEN -- Special case sudden exit */
L10:
    if (babs != zero) {
	goto L20;
    }
    ret_val = aabs;
    goto L40;
/* --- END IF */
/* --- DO FOREVER */
L20:
/* Computing 2nd power */
    r_1 = babs / aabs;
    r = r_1 * r_1;
/* ---      IF ((TWO+R) .EQ. TWO) THEN -- Converged */
    if (two + r != two) {
	goto L30;
    }
    ret_val = aabs;
    goto L40;
/* ---      END IF */
L30:
    s = r / (four + r);
    aabs += two * s * aabs;
    babs = s * babs;
/* --- END FOREVER */
    goto L20;

L40:
    return ret_val;
} /* gutre2_ */


/* --   FITCF */
/* Subroutine */
int glefitcf_(integer* mode,real* x,real* y,integer* l,integer* m,real* u,real* v,integer* n)
/*
integer *mode;
real *x, *y;
integer *l, *m;
real *u, *v;
integer *n;
*/
{
    /* System generated locals */
    integer i_1, i_2;
    real r_1;
    static real equiv_1[1], equiv_2[1], equiv_3[1], equiv_4[1], equiv_5[1],
	    equiv_6[1], equiv_7[1], equiv_8[1], equiv_9[1], equiv_10[1],
	    equiv_11[1], equiv_12[1];

    /* Builtin functions */
  //  double sqrt();

    /* Local variables */
    static integer mode0, i, j, k;
#define r (equiv_10)
#define z (equiv_10)
#define a1 (equiv_7)
#define a2 (equiv_8)
    static real a3, a4;
#define b1 (equiv_9)
#define b2 (equiv_1)
#define b3 (equiv_2)
#define b4 (equiv_3)
    static integer l0, m0, n0;
#define m1 (equiv_9)
    static integer k5;
#define m2 (equiv_1)
#define m3 (equiv_2)
#define m4 (equiv_3)
#define p0 (equiv_5)
    static real p1;
#define p2 (equiv_7)
#define p3 (equiv_9)
#define q0 (equiv_6)
#define q1 (equiv_4)
#define q2 (equiv_11)
#define q3 (equiv_12)
#define t2 (equiv_4)
    static real t3;
#define w2 (equiv_11)
#define w3 (equiv_12)
#define x2 (equiv_5)
    static real x3;
    static integer modem1;
    static real x4, x5;
#define y2 (equiv_6)
    static real y3, y4, y5;
//    extern doublereal gutre2_();
#define dz (equiv_8)
    static real rm;
#define sw (equiv_10)
//    extern /* Subroutine */ int gd_message__();
    static integer lm1, mm1;
    static real cos2, cos3, sin2, sin3;

/*     (Smooth Curve Fitting) */
/*     This subroutine fits a smooth curve to a given set of input */
/*     data points in  an X-Y  plane.  It  interpolates points  in */
/*     each interval between a pair of data points and generates a */
/*     set of output  points consisting of  the input data  points */
/*     and the  interpolated  points.   It can  process  either  a */
/*     single-valued function or a multiple-valued function. */

/*     The input arguments are: */

/*     MODE = mode of the curve (must be 1 or 2) */
/*          = 1 for a single-valued function */
/*          = 2 for multiple-valued function */
/*     X  = Array of  dimension L storing  the abscissas  of input */
/*          data points (in ascending or descending order for mode */
/*          = 1) */
/*     Y  = Array of  dimension L storing  the ordinates  of input */
/*          data points */
/*     L  = Number of input data points (must be 2 or greater) */
/*     M  = Number of subintervals between each pair of input data */
/*          points (must be 2 or greater). */
/*     N  = Number of output points */
/*        = (L-1)*M+1 */

/*     The output arguments are: */

/*     U  = Array of dimension N where the abscissas of output */
/*          points are to be displayed */
/*     V  = Array of dimension N where the ordinates of output */
/*          points are to be displayed */

/*     Author:  Hiroshi Akima,  "Interpolation  and  Smooth  Curve */
/*              Fitting Based on Local Procedures", COMM.  ACM 15, */
/*              914-918 (1972), and "A New Method of Interpolation */
/*              and  Smooth   Curve   Fitting   Based   on   Local */
/*              Procedures", J. ACM 17, 589-602 (1970). */

/*     Corrections: M.R. Andersen, "Remark on Algorithm 433", ACM */
/*                  Trans. on Math. Software, 2, 208 (1976). */
/*     (30-JAN-82) */

/*     Preliminary processing */

    /* Parameter adjustments */
    --v;
    --u;
    --y;
    --x;

    /* Function Body */
    mode0 = *mode;
    modem1 = mode0 - 1;
    l0 = *l;
    lm1 = l0 - 1;
    m0 = *m;
    mm1 = m0 - 1;
    n0 = *n;
    if (mode0 <= 0) {
	goto L320;
    }
    if (mode0 >= 3) {
	goto L320;
    }
    if (lm1 <= 0) {
	goto L330;
    }
    if (mm1 <= 0) {
	goto L340;
    }
    if (n0 != lm1 * m0 + 1) {
	gprint("Improper n value %ld, wanted %ld \n",n0,lm1 * m0 + 1);
	gprint("n0 %ld, lm1 %ld,  m0 %ld  l0 %ld \n",n0,lm1,m0,l0);
	goto L350;
    }

    switch (mode0) {
	case 1:  goto L10;
	case 2:  goto L60;
    }
L10:
    i = 2;
    if ((r_1 = x[1] - x[2]) < (float)0.) {
	goto L20;
    } else if (r_1 == 0) {
	gprint("Identical x values 1, %g %g \n",(double) x[1], (double) x[2]);
	goto L360;
    } else {
	goto L40;
    }
L20:
    if (l0 <= 2) {
	goto L80;
    }
    i_1 = l0;
    for (i = 3; i <= i_1; ++i) {
	if ((r_1 = x[i - 1] - x[i]) < (float)0.) {
	    goto L30;
	} else if (r_1 == 0) {
	gprint("Identical x values i, %g %g \n",(int) i,(double) x[i-1], (double) x[i]);
	    goto L360;
	} else {
	    goto L370;
	}
L30:
    ;}
    goto L80;
L40:
    if (l0 <= 2) {
	goto L80;
    }
    i_1 = l0;
    for (i = 3; i <= i_1; ++i) {
	if ((r_1 = x[i - 1] - x[i]) < (float)0.) {
	    goto L370;
	} else if (r_1 == 0) {
	gprint("Identical x aavalues i, %g %g \n",(int) i,(double) x[i-1], (double) x[i]);
	    goto L360;
	} else {
	    goto L50;
	}
L50:
    ;}
    goto L80;
L60:
    i_1 = l0;
    for (i = 2; i <= i_1; ++i) {
	if (x[i - 1] != x[i]) {
	    goto L70;
	}
	if (y[i - 1] == y[i]) {
	    goto L380;
	}
L70:
    ;}

L80:
    k = n0 + m0;
    i = l0 + 1;
    i_1 = l0;
    for (j = 1; j <= i_1; ++j) {
	k -= m0;
	--i;
	u[k] = x[i];
/* L90: */
	v[k] = y[i];
    }
    rm = (real) m0;
    rm = (float)1. / rm;

/*     Main DO-loop */

    k5 = m0 + 1;
    i_1 = l0;
    for (i = 1; i <= i_1; ++i) {

/*     Routines to  pick  up  necessary  X and  Y  values  and  to */
/*     estimate them if necessary */

	if (i > 1) {
	    goto L130;
	}
	x3 = u[1];
	y3 = v[1];
	x4 = u[m0 + 1];
	y4 = v[m0 + 1];
	a3 = x4 - x3;
	*b3 = y4 - y3;
	if (modem1 == 0) {
	    *m3 = *b3 / a3;
	}
	if (l0 != 2) {
	    goto L140;
	}
	a4 = a3;
	*b4 = *b3;
L100:
	switch (mode0) {
	    case 1:  goto L120;
	    case 2:  goto L110;
	}
L110:
	*a2 = a3 + a3 - a4;
	*a1 = *a2 + *a2 - a3;
L120:
	*b2 = *b3 + *b3 - *b4;
	*b1 = *b2 + *b2 - *b3;
	switch (mode0) {
	    case 1:  goto L180;
	    case 2:  goto L210;
	}
L130:
	*x2 = x3;
	*y2 = y3;
	x3 = x4;
	y3 = y4;
	x4 = x5;
	y4 = y5;
	*a1 = *a2;
	*b1 = *b2;
	*a2 = a3;
	*b2 = *b3;
	a3 = a4;
	*b3 = *b4;
	if (i >= lm1) {
	    goto L150;
	}
L140:
	k5 += m0;
	x5 = u[k5];
	y5 = v[k5];
	a4 = x5 - x4;
	*b4 = y5 - y4;
	if (modem1 == 0) {
	    *m4 = *b4 / a4;
	}
	goto L160;
L150:
	if (modem1 != 0) {
	    a4 = a3 + a3 - *a2;
	}
	*b4 = *b3 + *b3 - *b2;
L160:
	if (i == 1) {
	    goto L100;
	}
	switch (mode0) {
	    case 1:  goto L170;
	    case 2:  goto L200;
	}

/*     Numerical differentiation */

L170:
	*t2 = t3;
L180:
	*w2 = (r_1 = *m4 - *m3, dabs(r_1));
	*w3 = (r_1 = *m2 - *m1, dabs(r_1));
	*sw = *w2 + *w3;
	if (*sw != (float)0.) {
	    goto L190;
	}
	*w2 = (float).5;
	*w3 = (float).5;
	*sw = (float)1.;
L190:
	t3 = (*w2 * *m2 + *w3 * *m3) / *sw;
	if (i - 1 <= 0) {
	    goto L310;
	} else {
	    goto L240;
	}

L200:
	cos2 = cos3;
	sin2 = sin3;
L210:
	*w2 = (r_1 = a3 * *b4 - a4 * *b3, dabs(r_1));
	*w3 = (r_1 = *a1 * *b2 - *a2 * *b1, dabs(r_1));
	if (*w2 + *w3 != (float)0.) {
	    goto L220;
	}
	*w2 = gutre2_(&a3, b3);
	*w3 = gutre2_(a2, b2);
L220:
	cos3 = *w2 * *a2 + *w3 * a3;
	sin3 = *w2 * *b2 + *w3 * *b3;
	*r = cos3 * cos3 + sin3 * sin3;
	if (*r == (float)0.) {
	    goto L230;
	}
	*r = sqrt(*r);
	cos3 /= *r;
	sin3 /= *r;
L230:
	if (i - 1 <= 0) {
	    goto L310;
	} else {
	    goto L250;
	}

/*     Determination of the coefficients */

L240:
	*q2 = ((*m2 - *t2) * (float)2. + *m2 - t3) / *a2;
	*q3 = (-(doublereal)(*m2) - *m2 + *t2 + t3) / (*a2 * *a2);
	goto L260;

L250:
	*r = gutre2_(a2, b2);
	p1 = *r * cos2;
	*p2 = *a2 * (float)3. - *r * (cos2 + cos2 + cos3);
	*p3 = *a2 - p1 - *p2;
	*q1 = *r * sin2;
	*q2 = *b2 * (float)3. - *r * (sin2 + sin2 + sin3);
	*q3 = *b2 - *q1 - *q2;
	goto L280;

/*     Computation of the polynomials */

L260:
	*dz = *a2 * rm;
	*z = (float)0.;
	i_2 = mm1;
	for (j = 1; j <= i_2; ++j) {
	    ++k;
	    *z += *dz;
	    u[k] = *p0 + *z;
/* L270: */
	    v[k] = *q0 + *z * (*q1 + *z * (*q2 + *z * *q3));
	}
	goto L300;

L280:
	*z = (float)0.;
	i_2 = mm1;
	for (j = 1; j <= i_2; ++j) {
	    ++k;
	    *z += rm;
	    u[k] = *p0 + *z * (p1 + *z * (*p2 + *z * *p3));
/* L290: */
	    v[k] = *q0 + *z * (*q1 + *z * (*q2 + *z * *q3));
	}

L300:
	++k;
L310:
    ;}
    goto L410;

/*     Error exit */

L320:
    gd_message__("Cannot SMOOTH: Mode out of proper range 1..2", 29L);
    goto L400;
L330:
    gd_message__("Cannot SMOOTH: L = 1 or less", 13L);
    goto L400;
L340:
    gd_message__("Cannot SMOOTH: M = 1 or less", 13L);
    goto L400;
L350:
    gd_message__("Cannot SMOOTH: Improper N value", 16L);
    goto L400;
L360:
    gd_message__("Cannot SMOOTH: Identical X values", 18L);
    goto L390;
L370:
    gd_message__("Cannot SMOOTH: X values out of sequence", 24L);
    goto L390;
L380:
    gd_message__("Cannot SMOOTH: Identical X and Y values", 24L);
L390:
L400:

L410:
    return 0;
} /* glefitcf_ */

#undef sw
#undef dz
#undef y2
#undef x2
#undef w3
#undef w2
#undef t2
#undef q3
#undef q2
#undef q1
#undef q0
#undef p3
#undef p2
#undef p0
#undef m4
#undef m3
#undef m2
#undef m1
#undef b4
#undef b3
#undef b2
#undef b1
#undef a2
#undef a1
#undef z
#undef r























