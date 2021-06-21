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

/* fit.f -- translated by f2c (version of 26 February 1990  17:38:00).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/

#include "math.h"
#include "stdio.h"
#include "f2c.h"

/* Common Block Declarations */

typedef struct {
    integer nit;
} idlc_;

idlc_ idlc_1;

typedef struct {
    integer itpv;
} idpi_;

idpi_ idpi_1;

void do_fio();
void e_wsfe();
void s_stop();
void s_wsfe();
void err2090_();
void err2091_();
void err2091b_();
void err2092_();
void err2093_();
void err2094_();

int idcldp_(integer *ndp, real* xd, real* yd, integer* ncp, integer* ipc);
int idgrid_(real* xd, real* yd, integer* nt, integer* ipt, integer* nl, integer* ipl, integer* nxi, integer* nyi, real* xi, real* yi, integer* ngp, integer* igp);
int idlctn_(integer* ndp, real* xd, real* yd, integer* nt, integer* ipt, integer* nl, integer* ipl, real* xii, real* yii, integer* iti, integer* iwk, real* wk);
int idpdrv_(integer* ndp, real* xd, real* yd, real* zd, integer* ncp, integer* ipc, real* pd);
int idptip_(real* xd, real* yd, real* zd, integer* nt, integer* ipt, integer* nl, integer* ipl, real* pdd, integer* iti, real* xii, real* yii, real* zii);
int idsfft_(integer *md, integer *ncp, integer *ndp, real *xd, real *yd, real *zd, integer *nxi, integer *nyi, real *xi, real *yi, real *zi, integer *iwk, real *wk);
int idtang_(integer *ndp, real *xd, real *yd, integer *nt, integer *ipt, integer *nl, integer *ipl, integer *iwl, integer *iwp, real *wk);
integer idxchg_(real *x, real *y, integer *i1, integer *i2, integer *i3, integer *i4);

int idbvip_(int *md, int *ncp, int *ndp, double *xd, double *yd, double *zd, int *nip, double *xi, double *yi, double *zi, int *iwk, double *wk) {

    /* System generated locals */
    integer i_1, i_2;

    /* Local variables */
    static integer jwit, jwit0;
    static integer jwipc, jwipl, ncppv, ndppv, jwiwk, nippv, jwipt, jwiwl, jwiwp, nl;
    static integer nt;
    static integer md0, iip, ncp0, ndp0, nip0;

/* this subroutine performs bivariate interpolation when the pro- */
/* jections of the data points in the x-y plane are irregularly */
/* distributed in the plane. */
/* the input parameters are */
/*     md  = mode of computation (must be 1, 2, or 3), */
/*         = 1 for new ncp and/or new xd-yd, */
/*         = 2 for old ncp, old xd-yd, new xi-yi, */
/*         = 3 for old ncp, old xd-yd, old xi-yi, */
/*     ncp = number of additional data points used for esti- */
/*           mating partial derivatives at each data point */
/*           (must be 2 or greater, but smaller than ndp), */
/*     ndp = number of data points (must be 4 or greater), */
/*     xd  = array of dimension ndp containing the x */
/*           coordinates of the data points, */
/*     yd  = array of dimension ndp containing the y */
/*           coordinates of the data points, */
/*     zd  = array of dimension ndp containing the z */
/*           coordinates of the data points, */
/*     nip = number of output points at which interpolation */
/*           is to be performed (must be 1 or greater), */
/*     xi  = array of dimension nip containing the x */
/*           coordinates of the output points, */
/*     yi  = array of dimension nip containing the y */
/*           coordinates of the output points. */
/* the output parameter is */
/*     zi  = array of dimension nip where interpolated z */
/*           values are to be stored. */
/* the other parameters are */
/*     iwk = integer array of dimension */
/*              max0(31,27+ncp)*ndp+nip */
/*           used internally as a work area, */
/*     wk  = array of dimension 8*ndp used internally as a */
/*           work area. */
/* the very first call to this subroutine and the call with a new */
/* ncp value, a new ndp value, and/or new contents of the xd and */
/* yd arrays must be made with md=1.  the call with md=2 must be */
/* preceded by another call with the same ncp and ndp values and */
/* with the same contents of the xd and yd arrays.  the call with */
/* md=3 must be preceded by another call with the same ncp, ndp, */
/* and nip values and with the same contents of the xd, yd, xi, */
/* and yi arrays.  between the call with md=2 or md=3 and its */
/* preceding call, the iwk and wk arrays must not be disturbed. */
/* use of a value between 3 and 5 (inclusive) for ncp is recom- */
/* mended unless there are evidences that dictate otherwise. */
/* the lun constant in the data initialization statement is the */
/* logical unit number of the standard output unit and is, */
/* therefore, system dependent. */
/* this subroutine calls the idcldp, idlctn, idpdrv, idptip, and */
/* idtang subroutines. */
/* declaration statements */
    /* Parameter adjustments */
    --wk;
    --iwk;
    --zi;
    --yi;
    --xi;
    --zd;
    --yd;
    --xd;

    /* Function Body */
/* setting of some input parameters to local variables. */
/* (for md=1,2,3) */
/* L10: */
    md0 = *md;
    ncp0 = *ncp;
    ndp0 = *ndp;
    nip0 = *nip;
/* error check.  (for md=1,2,3) */
/* L20: */
    if (md0 < 1 || md0 > 3) {
	goto L90;
    }
    if (ncp0 < 2 || ncp0 >= ndp0) {
	goto L90;
    }
    if (ndp0 < 4) {
	goto L90;
    }
    if (nip0 < 1) {
	goto L90;
    }
    if (md0 >= 2) {
	goto L21;
    }
    iwk[1] = ncp0;
    iwk[2] = ndp0;
    goto L22;
L21:
    ncppv = iwk[1];
    ndppv = iwk[2];
    if (ncp0 != ncppv) {
	goto L90;
    }
    if (ndp0 != ndppv) {
	goto L90;
    }
L22:
    if (md0 >= 3) {
	goto L23;
    }
    iwk[3] = *nip;
    goto L30;
L23:
    nippv = iwk[3];
    if (nip0 != nippv) {
	goto L90;
    }
/* allocation of storage areas in the iwk array.  (for md=1,2,3) */
L30:
    jwipt = 16;
    jwiwl = ndp0 * 6 + 1;
    jwiwk = jwiwl;
    jwipl = ndp0 * 24 + 1;
    jwiwp = ndp0 * 30 + 1;
    jwipc = ndp0 * 27 + 1;
/* Computing MAX */
    i_1 = 31, i_2 = ncp0 + 27;
    jwit0 = max(i_1,i_2) * ndp0;
/* triangulates the x-y plane.  (for md=1) */
/* L40: */
    if (md0 > 1) {
	goto L50;
    }
    idtang_(&ndp0, &xd[1], &yd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl], &iwk[
	    jwiwl], &iwk[jwiwp], &wk[1]);
    iwk[5] = nt;
    iwk[6] = nl;
    if (nt == 0) {
	return 0;
    }
/* determines ncp points closest to each data point.  (for md=1) */
L50:
    if (md0 > 1) {
	goto L60;
    }
    idcldp_(&ndp0, &xd[1], &yd[1], &ncp0, &iwk[jwipc]);
    if (iwk[jwipc] == 0) {
	return 0;
    }
/* locates all points at which interpolation is to be performed. */
/* (for md=1,2) */
L60:
    if (md0 == 3) {
	goto L70;
    }
    idlc_1.nit = 0;
    jwit = jwit0;
    i_1 = nip0;
    for (iip = 1; iip <= i_1; ++iip) {
	++jwit;
	idlctn_(&ndp0, &xd[1], &yd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl], &
		xi[iip], &yi[iip], &iwk[jwit], &iwk[jwiwk], &wk[1]);
/* L61: */
    }
/* estimates partial derivatives at all data points. */
/* (for md=1,2,3) */
L70:
    idpdrv_(&ndp0, &xd[1], &yd[1], &zd[1], &ncp0, &iwk[jwipc], &wk[1]);
/* interpolates the zi values.  (for md=1,2,3) */
/* L80: */
    idpi_1.itpv = 0;
    jwit = jwit0;
    i_1 = nip0;
    for (iip = 1; iip <= i_1; ++iip) {
	++jwit;
	idptip_(&xd[1], &yd[1], &zd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl], &
		wk[1], &iwk[jwit], &xi[iip], &yi[iip], &zi[iip]);
/* L81: */
    }
    return 0;
/* error exit */
L90:
    err2090_();
    return 0;
/* format statement for error message */
/* L2090: */
} /* idbvip_ */

int idcldp_(integer *ndp, real* xd, real* yd, integer* ncp, integer* ipc) {
    /* Initialized data */

    static integer ncpmx = 25;

    /* System generated locals */
    integer i_1, i_2, i_3;
    real r_1, r_2;

    /* Local variables */
    static real dsqi;
    static integer ip2mn, ip3mn;
    static integer nclpt;
    static real dsqmn;
    static integer j1;
    static real dsqmx;
    static integer j3, j4, j2;
    static real x1, y1;
    static integer ip1, ip2, ip3;
    static real dx12, dy12, dx13, dy13;
    static integer jmx, ipc0[25], ncp0, ndp0;
    static real dsq0[25];

/* this subroutine selects several data points that are closest */
/* to each of the data point. */
/* the input parameters are */
/*     ndp = number of data points, */
/*     xd,yd = arrays of dimension ndp containing the x and y */
/*           coordinates of the data points, */
/*     ncp = number of data points closest to each data */
/*           points. */
/* the output parameter is */
/*     ipc = integer array of dimension ncp*ndp, where the */
/*           point numbers of ncp data points closest to */
/*           each of the ndp data points are to be stored. */
/* this subroutine arbitrarily sets a restriction that ncp must */
/* not exceed 25. */
/* the lun constant in the data initialization statement is the */
/* logical unit number of the standard output unit and is, */
/* therefore, system dependent. */
/* declaration statements */
    /* Parameter adjustments */
    --ipc;
    --yd;
    --xd;

    /* Function Body */
/* statement function */
/* preliminary processing */
/* L10: */
    ndp0 = *ndp;
    ncp0 = *ncp;
    if (ndp0 < 2) {
	goto L90;
    }
    if (ncp0 < 1 || ncp0 > ncpmx || ncp0 >= ndp0) {
	goto L90;
    }
/* calculation */
/* L20: */
    i_1 = ndp0;
    for (ip1 = 1; ip1 <= i_1; ++ip1) {
/* - selects ncp points. */
	x1 = xd[ip1];
	y1 = yd[ip1];
	j1 = 0;
	dsqmx = (float)0.;
	i_2 = ndp0;
	for (ip2 = 1; ip2 <= i_2; ++ip2) {
	    if (ip2 == ip1) {
		goto L22;
	    }
/* Computing 2nd power */
	    r_1 = xd[ip2] - x1;
/* Computing 2nd power */
	    r_2 = yd[ip2] - y1;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    ++j1;
	    dsq0[j1 - 1] = dsqi;
	    ipc0[j1 - 1] = ip2;
	    if (dsqi <= dsqmx) {
		goto L21;
	    }
	    dsqmx = dsqi;
	    jmx = j1;
L21:
	    if (j1 >= ncp0) {
		goto L23;
	    }
L22:
	;}
L23:
	ip2mn = ip2 + 1;
	if (ip2mn > ndp0) {
	    goto L30;
	}
	i_2 = ndp0;
	for (ip2 = ip2mn; ip2 <= i_2; ++ip2) {
	    if (ip2 == ip1) {
		goto L25;
	    }
/* Computing 2nd power */
	    r_1 = xd[ip2] - x1;
/* Computing 2nd power */
	    r_2 = yd[ip2] - y1;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    if (dsqi >= dsqmx) {
		goto L25;
	    }
	    dsq0[jmx - 1] = dsqi;
	    ipc0[jmx - 1] = ip2;
	    dsqmx = (float)0.;
	    i_3 = ncp0;
	    for (j1 = 1; j1 <= i_3; ++j1) {
		if (dsq0[j1 - 1] <= dsqmx) {
		    goto L24;
		}
		dsqmx = dsq0[j1 - 1];
		jmx = j1;
L24:
	    ;}
L25:
	;}
/* - checks if all the ncp+1 points are collinear. */
L30:
	ip2 = ipc0[0];
	dx12 = xd[ip2] - x1;
	dy12 = yd[ip2] - y1;
	i_2 = ncp0;
	for (j3 = 2; j3 <= i_2; ++j3) {
	    ip3 = ipc0[j3 - 1];
	    dx13 = xd[ip3] - x1;
	    dy13 = yd[ip3] - y1;
	    if (dy13 * dx12 - dx13 * dy12 != (float)0.) {
		goto L50;
	    }
/* L31: */
	}
/* - searches for the closest noncollinear point. */
/* L40: */
	nclpt = 0;
	i_2 = ndp0;
	for (ip3 = 1; ip3 <= i_2; ++ip3) {
	    if (ip3 == ip1) {
		goto L43;
	    }
	    i_3 = ncp0;
	    for (j4 = 1; j4 <= i_3; ++j4) {
		if (ip3 == ipc0[j4 - 1]) {
		    goto L43;
		}
/* L41: */
	    }
	    dx13 = xd[ip3] - x1;
	    dy13 = yd[ip3] - y1;
	    if (dy13 * dx12 - dx13 * dy12 == (float)0.) {
		goto L43;
	    }
/* Computing 2nd power */
	    r_1 = xd[ip3] - x1;
/* Computing 2nd power */
	    r_2 = yd[ip3] - y1;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    if (nclpt == 0) {
		goto L42;
	    }
	    if (dsqi >= dsqmn) {
		goto L43;
	    }
L42:
	    nclpt = 1;
	    dsqmn = dsqi;
	    ip3mn = ip3;
L43:
	;}
	if (nclpt == 0) {
	    goto L91;
	}
	dsqmx = dsqmn;
	ipc0[jmx - 1] = ip3mn;
/* - replaces the local array for the output array. */
L50:
	j1 = (ip1 - 1) * ncp0;
	i_2 = ncp0;
	for (j2 = 1; j2 <= i_2; ++j2) {
	    ++j1;
	    ipc[j1] = ipc0[j2 - 1];
/* L51: */
	}
/* L59: */
    }
    return 0;
/* error exit */
L90:
    err2090_();
    goto L92;
L91:
    err2091_();
L92:
    err2092_();
    ipc[1] = 0;
    return 0;
/* format statements for error messages */
/* L2090: */
/* L2091: */
/* L2092: */
} /* idcldp_ */

int idgrid_(real* xd, real* yd, integer* nt, integer* ipt, integer* nl, integer* ipl, integer* nxi, integer* nyi, real* xi, real* yi, integer* ngp, integer* igp) {
    /* System generated locals */
    integer i_1, i_2, i_3, i_4;
    real r_1, r_2;

    /* Local variables */
    static integer il0t3, insd, it0t3;
    static real ximn, yimn, ximx, yimx;
    static integer jigp0, jigp1, jngp0, jngp1, l, ilp1t3, iximn, iximx;
    static real x1, y1, x2, y2, x3, y3;
    static integer jigp1i, il0, nl0, ip1, ip2, it0, nxinyi, ip3, nt0, ixi,
	    iyi;
    static real yii, xii;
    static integer izi;
    static real xmn, ymn, xmx, ymx;
    static integer ngp0, ngp1, ilp1, nxi0, nyi0;

/* this subroutine organizes grid points for surface fitting by */
/* sorting them in ascending order of triangle numbers and of the */
/* border line segment number. */
/* the input parameters are */
/*     xd,yd = arrays of dimension ndp containing the x and y */
/*           coordinates of the data points, where ndp is the */
/*           number of the data points, */
/*     nt  = number of triangles, */
/*     ipt = integer array of dimension 3*nt containing the */
/*           point numbers of the vertexes of the triangles, */
/*     nl  = number of border line segments, */
/*     ipl = integer array of dimension 3*nl containing the */
/*           point numbers of the end points of the border */
/*           line segments and their respective triangle */
/*           numbers, */
/*     nxi = number of grid points in the x coordinate, */
/*     nyi = number of grid points in the y coordinate, */
/*     xi,yi = arrays of dimension nxi and nyi containing */
/*           the x and y coordinates of the grid points, */
/*           respectively. */
/* the output parameters are */
/*     ngp = integer array of dimension 2*(nt+2*nl) where the */
/*           number of grid points that belong to each of the */
/*           triangles or of the border line segments are to */
/*           be stored, */
/*     igp = integer array of dimension nxi*nyi where the */
/*           grid point numbers are to be stored in ascending */
/*           order of the triangle number and the border line */
/*           segment number. */
/* declaration statements */
/* statement functions */
/* preliminary processing */
    /* Parameter adjustments */
    --igp;
    --ngp;
    --yi;
    --xi;
    --ipl;
    --ipt;
    --yd;
    --xd;

    /* Function Body */
    nt0 = *nt;
    nl0 = *nl;
    nxi0 = *nxi;
    nyi0 = *nyi;
    nxinyi = nxi0 * nyi0;
/* Computing MIN */
    r_1 = xi[1], r_2 = xi[nxi0];
    ximn = dmin(r_1,r_2);
/* Computing MAX */
    r_1 = xi[1], r_2 = xi[nxi0];
    ximx = dmax(r_1,r_2);
/* Computing MIN */
    r_1 = yi[1], r_2 = yi[nyi0];
    yimn = dmin(r_1,r_2);
/* Computing MAX */
    r_1 = yi[1], r_2 = yi[nyi0];
    yimx = dmax(r_1,r_2);
/* determines grid points inside the data area. */
    jngp0 = 0;
    jngp1 = ((nt0 + (nl0 << 1)) << 1) + 1;
    jigp0 = 0;
    jigp1 = nxinyi + 1;
    i_1 = nt0;
    for (it0 = 1; it0 <= i_1; ++it0) {
	ngp0 = 0;
	ngp1 = 0;
	it0t3 = it0 * 3;
	ip1 = ipt[it0t3 - 2];
	ip2 = ipt[it0t3 - 1];
	ip3 = ipt[it0t3];
	x1 = xd[ip1];
	y1 = yd[ip1];
	x2 = xd[ip2];
	y2 = yd[ip2];
	x3 = xd[ip3];
	y3 = yd[ip3];
/* Computing MIN */
	r_1 = min(x1,x2);
	xmn = dmin(r_1,x3);
/* Computing MAX */
	r_1 = max(x1,x2);
	xmx = dmax(r_1,x3);
/* Computing MIN */
	r_1 = min(y1,y2);
	ymn = dmin(r_1,y3);
/* Computing MAX */
	r_1 = max(y1,y2);
	ymx = dmax(r_1,y3);
	insd = 0;
	i_2 = nxi0;
	for (ixi = 1; ixi <= i_2; ++ixi) {
	    if (xi[ixi] >= xmn && xi[ixi] <= xmx) {
		goto L10;
	    }
	    if (insd == 0) {
		goto L20;
	    }
	    iximx = ixi - 1;
	    goto L30;
L10:
	    if (insd == 1) {
		goto L20;
	    }
	    insd = 1;
	    iximn = ixi;
L20:
	;}
	if (insd == 0) {
	    goto L150;
	}
	iximx = nxi0;
L30:
	i_2 = nyi0;
	for (iyi = 1; iyi <= i_2; ++iyi) {
	    yii = yi[iyi];
	    if (yii < ymn || yii > ymx) {
		goto L140;
	    }
	    i_3 = iximx;
	    for (ixi = iximn; ixi <= i_3; ++ixi) {
		xii = xi[ixi];
		l = 0;
		if ((r_1 = (x1 - xii) * (y2 - yii) - (y1 - yii) * (x2 - xii))
			< (float)0.) {
		    goto L130;
		} else if (r_1 == 0) {
		    goto L40;
		} else {
		    goto L50;
		}
L40:
		l = 1;
L50:
		if ((r_1 = (x2 - xii) * (y3 - yii) - (y2 - yii) * (x3 - xii))
			< (float)0.) {
		    goto L130;
		} else if (r_1 == 0) {
		    goto L60;
		} else {
		    goto L70;
		}
L60:
		l = 1;
L70:
		if ((r_1 = (x3 - xii) * (y1 - yii) - (y3 - yii) * (x1 - xii))
			< (float)0.) {
		    goto L130;
		} else if (r_1 == 0) {
		    goto L80;
		} else {
		    goto L90;
		}
L80:
		l = 1;
L90:
		izi = nxi0 * (iyi - 1) + ixi;
		if (l == 1) {
		    goto L100;
		}
		++ngp0;
		++jigp0;
		igp[jigp0] = izi;
		goto L130;
L100:
		if (jigp1 > nxinyi) {
		    goto L120;
		}
		i_4 = nxinyi;
		for (jigp1i = jigp1; jigp1i <= i_4; ++jigp1i) {
		    if (izi == igp[jigp1i]) {
			goto L130;
		    }
/* L110: */
		}
L120:
		++ngp1;
		--jigp1;
		igp[jigp1] = izi;
L130:
	    ;}
L140:
	;}
L150:
	++jngp0;
	ngp[jngp0] = ngp0;
	--jngp1;
	ngp[jngp1] = ngp1;
/* L160: */
    }
/* determines grid points outside the data area. */
/* - in semi-infinite rectangular area. */
    i_1 = nl0;
    for (il0 = 1; il0 <= i_1; ++il0) {
	ngp0 = 0;
	ngp1 = 0;
	il0t3 = il0 * 3;
	ip1 = ipl[il0t3 - 2];
	ip2 = ipl[il0t3 - 1];
	x1 = xd[ip1];
	y1 = yd[ip1];
	x2 = xd[ip2];
	y2 = yd[ip2];
	xmn = ximn;
	xmx = ximx;
	ymn = yimn;
	ymx = yimx;
	if (y2 >= y1) {
	    xmn = dmin(x1,x2);
	}
	if (y2 <= y1) {
	    xmx = dmax(x1,x2);
	}
	if (x2 <= x1) {
	    ymn = dmin(y1,y2);
	}
	if (x2 >= x1) {
	    ymx = dmax(y1,y2);
	}
	insd = 0;
	i_2 = nxi0;
	for (ixi = 1; ixi <= i_2; ++ixi) {
	    if (xi[ixi] >= xmn && xi[ixi] <= xmx) {
		goto L170;
	    }
	    if (insd == 0) {
		goto L180;
	    }
	    iximx = ixi - 1;
	    goto L190;
L170:
	    if (insd == 1) {
		goto L180;
	    }
	    insd = 1;
	    iximn = ixi;
L180:
	;}
	if (insd == 0) {
	    goto L310;
	}
	iximx = nxi0;
L190:
	i_2 = nyi0;
	for (iyi = 1; iyi <= i_2; ++iyi) {
	    yii = yi[iyi];
	    if (yii < ymn || yii > ymx) {
		goto L300;
	    }
	    i_3 = iximx;
	    for (ixi = iximn; ixi <= i_3; ++ixi) {
		xii = xi[ixi];
		l = 0;
		if ((r_1 = (x1 - xii) * (y2 - yii) - (y1 - yii) * (x2 - xii))
			< (float)0.) {
		    goto L210;
		} else if (r_1 == 0) {
		    goto L200;
		} else {
		    goto L290;
		}
L200:
		l = 1;
L210:
		if ((r_1 = (x2 - x1) * (xii - x1) + (y2 - y1) * (yii - y1)) <
			(float)0.) {
		    goto L290;
		} else if (r_1 == 0) {
		    goto L220;
		} else {
		    goto L230;
		}
L220:
		l = 1;
L230:
		if ((r_1 = (x1 - x2) * (xii - x2) + (y1 - y2) * (yii - y2)) <
			(float)0.) {
		    goto L290;
		} else if (r_1 == 0) {
		    goto L240;
		} else {
		    goto L250;
		}
L240:
		l = 1;
L250:
		izi = nxi0 * (iyi - 1) + ixi;
		if (l == 1) {
		    goto L260;
		}
		++ngp0;
		++jigp0;
		igp[jigp0] = izi;
		goto L290;
L260:
		if (jigp1 > nxinyi) {
		    goto L280;
		}
		i_4 = nxinyi;
		for (jigp1i = jigp1; jigp1i <= i_4; ++jigp1i) {
		    if (izi == igp[jigp1i]) {
			goto L290;
		    }
/* L270: */
		}
L280:
		++ngp1;
		--jigp1;
		igp[jigp1] = izi;
L290:
	    ;}
L300:
	;}
L310:
	++jngp0;
	ngp[jngp0] = ngp0;
	--jngp1;
	ngp[jngp1] = ngp1;
/* - in semi-infinite triangular area. */
	ngp0 = 0;
	ngp1 = 0;
	ilp1 = il0 % nl0 + 1;
	ilp1t3 = ilp1 * 3;
	ip3 = ipl[ilp1t3 - 1];
	x3 = xd[ip3];
	y3 = yd[ip3];
	xmn = ximn;
	xmx = ximx;
	ymn = yimn;
	ymx = yimx;
	if (y3 >= y2 && y2 >= y1) {
	    xmn = x2;
	}
	if (y3 <= y2 && y2 <= y1) {
	    xmx = x2;
	}
	if (x3 <= x2 && x2 <= x1) {
	    ymn = y2;
	}
	if (x3 >= x2 && x2 >= x1) {
	    ymx = y2;
	}
	insd = 0;
	i_2 = nxi0;
	for (ixi = 1; ixi <= i_2; ++ixi) {
	    if (xi[ixi] >= xmn && xi[ixi] <= xmx) {
		goto L320;
	    }
	    if (insd == 0) {
		goto L330;
	    }
	    iximx = ixi - 1;
	    goto L340;
L320:
	    if (insd == 1) {
		goto L330;
	    }
	    insd = 1;
	    iximn = ixi;
L330:
	;}
	if (insd == 0) {
	    goto L440;
	}
	iximx = nxi0;
L340:
	i_2 = nyi0;
	for (iyi = 1; iyi <= i_2; ++iyi) {
	    yii = yi[iyi];
	    if (yii < ymn || yii > ymx) {
		goto L430;
	    }
	    i_3 = iximx;
	    for (ixi = iximn; ixi <= i_3; ++ixi) {
		xii = xi[ixi];
		l = 0;
		if ((r_1 = (x1 - x2) * (xii - x2) + (y1 - y2) * (yii - y2)) <
			(float)0.) {
		    goto L360;
		} else if (r_1 == 0) {
		    goto L350;
		} else {
		    goto L420;
		}
L350:
		l = 1;
L360:
		if ((r_1 = (x3 - x2) * (xii - x2) + (y3 - y2) * (yii - y2)) <
			(float)0.) {
		    goto L380;
		} else if (r_1 == 0) {
		    goto L370;
		} else {
		    goto L420;
		}
L370:
		l = 1;
L380:
		izi = nxi0 * (iyi - 1) + ixi;
		if (l == 1) {
		    goto L390;
		}
		++ngp0;
		++jigp0;
		igp[jigp0] = izi;
		goto L420;
L390:
		if (jigp1 > nxinyi) {
		    goto L410;
		}
		i_4 = nxinyi;
		for (jigp1i = jigp1; jigp1i <= i_4; ++jigp1i) {
		    if (izi == igp[jigp1i]) {
			goto L420;
		    }
/* L400: */
		}
L410:
		++ngp1;
		--jigp1;
		igp[jigp1] = izi;
L420:
	    ;}
L430:
	;}
L440:
	++jngp0;
	ngp[jngp0] = ngp0;
	--jngp1;
	ngp[jngp1] = ngp1;
/* L450: */
    }
    return 0;
} /* idgrid_ */

int idlctn_(integer* ndp, real* xd, real* yd, integer* nt, integer* ipt, integer* nl, integer* ipl, real* xii, real* yii, integer* iti, integer* iwk, real* wk) {
    /* System generated locals */
    integer i_1;
    real r_1, r_2;

    /* Local variables */
    static integer idsc[9], il1t3, itsc, it0t3, jiwk, ntsc[9], ntsci, i1, i2,
	    i3, itipv;
    static real x0, y0, x1, y1, x2, y2, x3, y3, xi, yi;
    static integer il1, il2, nl0, ip1, ip2, it0, ip3, nt0;
    static real xs1, xs2, ys1, ys2;
    static integer idp, isc, ntl, jwk;
    static real xmn, ymn, xmx, ymx;
    static integer ndp0;

/* this subroutine locates a point, i.e., determines to what tri- */
/* angle a given point (xii,yii) belongs.  when the given point */
/* does not lie inside the data area, this subroutine determines */
/* the border line segment when the point lies in an outside */
/* rectangular area, and two border line segments when the point */
/* lies in an outside triangular area. */
/* the input parameters are */
/*     ndp = number of data points, */
/*     xd,yd = arrays of dimension ndp containing the x and y */
/*           coordinates of the data points, */
/*     nt  = number of triangles, */
/*     ipt = integer array of dimension 3*nt containing the */
/*           point numbers of the vertexes of the triangles, */
/*     nl  = number of border line segments, */
/*     ipl = integer array of dimension 3*nl containing the */
/*           point numbers of the end points of the border */
/*           line segments and their respective triangle */
/*           numbers, */
/*     xii,yii = x and y coordinates of the point to be */
/*           located. */
/* the output parameter is */
/*     iti = triangle number, when the point is inside the */
/*           data area, or */
/*           two border line segment numbers, il1 and il2, */
/*           coded to il1*(nt+nl)+il2, when the point is */
/*           outside the data area. */
/* the other parameters are */
/*     iwk = integer array of dimension 18*ndp used inter- */
/*           nally as a work area, */
/*     wk  = array of dimension 8*ndp used internally as a */
/*           work area. */
/* declaration statements */
/* statement functions */
/* preliminary processing */
    /* Parameter adjustments */
    --wk;
    --iwk;
    --ipl;
    --ipt;
    --yd;
    --xd;

    /* Function Body */
    ndp0 = *ndp;
    nt0 = *nt;
    nl0 = *nl;
    ntl = nt0 + nl0;
    x0 = *xii;
    y0 = *yii;
/* processing for a new set of data points */
    if (idlc_1.nit != 0) {
	goto L80;
    }
    idlc_1.nit = 1;
/* - divides the x-y plane into nine rectangular sections. */
    xmn = xd[1];
    xmx = xmn;
    ymn = yd[1];
    ymx = ymn;
    i_1 = ndp0;
    for (idp = 2; idp <= i_1; ++idp) {
	xi = xd[idp];
	yi = yd[idp];
	xmn = dmin(xi,xmn);
	xmx = dmax(xi,xmx);
	ymn = dmin(yi,ymn);
	ymx = dmax(yi,ymx);
/* L10: */
    }
    xs1 = (xmn + xmn + xmx) / (float)3.;
    xs2 = (xmn + xmx + xmx) / (float)3.;
    ys1 = (ymn + ymn + ymx) / (float)3.;
    ys2 = (ymn + ymx + ymx) / (float)3.;
/* - determines and stores in the iwk array triangle numbers of */
/* - the triangles associated with each of the nine sections. */
    for (isc = 1; isc <= 9; ++isc) {
	ntsc[isc - 1] = 0;
	idsc[isc - 1] = 0;
/* L20: */
    }
    it0t3 = 0;
    jwk = 0;
    i_1 = nt0;
    for (it0 = 1; it0 <= i_1; ++it0) {
	it0t3 += 3;
	i1 = ipt[it0t3 - 2];
	i2 = ipt[it0t3 - 1];
	i3 = ipt[it0t3];
/* Computing MIN */
	r_1 = xd[i1], r_2 = xd[i2], r_1 = min(r_1,r_2), r_2 = xd[i3];
	xmn = dmin(r_1,r_2);
/* Computing MAX */
	r_1 = xd[i1], r_2 = xd[i2], r_1 = max(r_1,r_2), r_2 = xd[i3];
	xmx = dmax(r_1,r_2);
/* Computing MIN */
	r_1 = yd[i1], r_2 = yd[i2], r_1 = min(r_1,r_2), r_2 = yd[i3];
	ymn = dmin(r_1,r_2);
/* Computing MAX */
	r_1 = yd[i1], r_2 = yd[i2], r_1 = max(r_1,r_2), r_2 = yd[i3];
	ymx = dmax(r_1,r_2);
	if (ymn > ys1) {
	    goto L30;
	}
	if (xmn <= xs1) {
	    idsc[0] = 1;
	}
	if (xmx >= xs1 && xmn <= xs2) {
	    idsc[1] = 1;
	}
	if (xmx >= xs2) {
	    idsc[2] = 1;
	}
L30:
	if (ymx < ys1 || ymn > ys2) {
	    goto L40;
	}
	if (xmn <= xs1) {
	    idsc[3] = 1;
	}
	if (xmx >= xs1 && xmn <= xs2) {
	    idsc[4] = 1;
	}
	if (xmx >= xs2) {
	    idsc[5] = 1;
	}
L40:
	if (ymx < ys2) {
	    goto L50;
	}
	if (xmn <= xs1) {
	    idsc[6] = 1;
	}
	if (xmx >= xs1 && xmn <= xs2) {
	    idsc[7] = 1;
	}
	if (xmx >= xs2) {
	    idsc[8] = 1;
	}
L50:
	for (isc = 1; isc <= 9; ++isc) {
	    if (idsc[isc - 1] == 0) {
		goto L60;
	    }
	    jiwk = ntsc[isc - 1] * 9 + isc;
	    iwk[jiwk] = it0;
	    ++ntsc[isc - 1];
	    idsc[isc - 1] = 0;
L60:
	;}
/* - stores in the wk array the minimum and maximum of the x and */
/* - y coordinate values for each of the triangle. */
	jwk += 4;
	wk[jwk - 3] = xmn;
	wk[jwk - 2] = xmx;
	wk[jwk - 1] = ymn;
	wk[jwk] = ymx;
/* L70: */
    }
    goto L110;
/* checks if in the same triangle as previous. */
L80:
    it0 = itipv;
    if (it0 > nt0) {
	goto L90;
    }
    it0t3 = it0 * 3;
    ip1 = ipt[it0t3 - 2];
    x1 = xd[ip1];
    y1 = yd[ip1];
    ip2 = ipt[it0t3 - 1];
    x2 = xd[ip2];
    y2 = yd[ip2];
    if ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) < (float)0.) {
	goto L110;
    }
    ip3 = ipt[it0t3];
    x3 = xd[ip3];
    y3 = yd[ip3];
    if ((x2 - x0) * (y3 - y0) - (y2 - y0) * (x3 - x0) < (float)0.) {
	goto L110;
    }
    if ((x3 - x0) * (y1 - y0) - (y3 - y0) * (x1 - x0) < (float)0.) {
	goto L110;
    }
    goto L170;
/* checks if on the same border line segment. */
L90:
    il1 = it0 / ntl;
    il2 = it0 - il1 * ntl;
    il1t3 = il1 * 3;
    ip1 = ipl[il1t3 - 2];
    x1 = xd[ip1];
    y1 = yd[ip1];
    ip2 = ipl[il1t3 - 1];
    x2 = xd[ip2];
    y2 = yd[ip2];
    if (il2 != il1) {
	goto L100;
    }
    if ((x1 - x2) * (x0 - x2) + (y1 - y2) * (y0 - y2) < (float)0.) {
	goto L110;
    }
    if ((x2 - x1) * (x0 - x1) + (y2 - y1) * (y0 - y1) < (float)0.) {
	goto L110;
    }
    if ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) > (float)0.) {
	goto L110;
    }
    goto L170;
/* checks if between the same two border line segments. */
L100:
    if ((x1 - x2) * (x0 - x2) + (y1 - y2) * (y0 - y2) > (float)0.) {
	goto L110;
    }
    ip3 = ipl[il2 * 3 - 1];
    x3 = xd[ip3];
    y3 = yd[ip3];
    if ((x3 - x2) * (x0 - x2) + (y3 - y2) * (y0 - y2) <= (float)0.) {
	goto L170;
    }
/* locates inside the data area. */
/* - determines the section in which the point in question lies. */
L110:
    isc = 1;
    if (x0 >= xs1) {
	++isc;
    }
    if (x0 >= xs2) {
	++isc;
    }
    if (y0 >= ys1) {
	isc += 3;
    }
    if (y0 >= ys2) {
	isc += 3;
    }
/* - searches through the triangles associated with the section. */
    ntsci = ntsc[isc - 1];
    if (ntsci <= 0) {
	goto L130;
    }
    jiwk = isc - 9;
    i_1 = ntsci;
    for (itsc = 1; itsc <= i_1; ++itsc) {
	jiwk += 9;
	it0 = iwk[jiwk];
	jwk = it0 << 2;
	if (x0 < wk[jwk - 3]) {
	    goto L120;
	}
	if (x0 > wk[jwk - 2]) {
	    goto L120;
	}
	if (y0 < wk[jwk - 1]) {
	    goto L120;
	}
	if (y0 > wk[jwk]) {
	    goto L120;
	}
	it0t3 = it0 * 3;
	ip1 = ipt[it0t3 - 2];
	x1 = xd[ip1];
	y1 = yd[ip1];
	ip2 = ipt[it0t3 - 1];
	x2 = xd[ip2];
	y2 = yd[ip2];
	if ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) < (float)0.) {
	    goto L120;
	}
	ip3 = ipt[it0t3];
	x3 = xd[ip3];
	y3 = yd[ip3];
	if ((x2 - x0) * (y3 - y0) - (y2 - y0) * (x3 - x0) < (float)0.) {
	    goto L120;
	}
	if ((x3 - x0) * (y1 - y0) - (y3 - y0) * (x1 - x0) < (float)0.) {
	    goto L120;
	}
	goto L170;
L120:
    ;}
/* locates outside the data area. */
L130:
    i_1 = nl0;
    for (il1 = 1; il1 <= i_1; ++il1) {
	il1t3 = il1 * 3;
	ip1 = ipl[il1t3 - 2];
	x1 = xd[ip1];
	y1 = yd[ip1];
	ip2 = ipl[il1t3 - 1];
	x2 = xd[ip2];
	y2 = yd[ip2];
	if ((x2 - x1) * (x0 - x1) + (y2 - y1) * (y0 - y1) < (float)0.) {
	    goto L150;
	}
	if ((x1 - x2) * (x0 - x2) + (y1 - y2) * (y0 - y2) < (float)0.) {
	    goto L140;
	}
	if ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) > (float)0.) {
	    goto L150;
	}
	il2 = il1;
	goto L160;
L140:
	il2 = il1 % nl0 + 1;
	ip3 = ipl[il2 * 3 - 1];
	x3 = xd[ip3];
	y3 = yd[ip3];
	if ((x3 - x2) * (x0 - x2) + (y3 - y2) * (y0 - y2) <= (float)0.) {
	    goto L160;
	}
L150:
    ;}
    it0 = 1;
    goto L170;
L160:
    it0 = il1 * ntl + il2;
/* normal exit */
L170:
    *iti = it0;
    itipv = it0;
    return 0;
} /* idlctn_ */

int idpdrv_(integer* ndp, real* xd, real* yd, real* zd, integer* ncp, integer* ipc, real* pd) {
    /* System generated locals */
    integer i_1, i_2, i_3;

    /* Local variables */
    static integer jipc;
    static real dnmx, dnmy, dnmz, nmxx, nmxy, nmyx, nmyy;
    static integer jipc0, ic2mn, ncpm1;
    static real dnmxx, dnmxy, dnmyx, dnmyy, x0, y0, z0;
    static integer ic1, ic2, ip0;
    static real dx1, dy1, dz1, dx2, dy2, dz2, zx0, zy0;
    static integer jpd, ipi;
    static real nmx, nmy, nmz;
    static integer jpd0, ncp0, ndp0;
    static real dzx1, dzy1, dzx2, dzy2;

/* this subroutine estimates partial derivatives of the first and */
/* second order at the data points. */
/* the input parameters are */
/*     ndp = number of data points, */
/*     xd,yd,zd = arrays of dimension ndp containing the x, */
/*           y, and z coordinates of the data points, */
/*     ncp = number of additional data points used for esti- */
/*           mating partial derivatives at each data point, */
/*     ipc = integer array of dimension ncp*ndp containing */
/*           the point numbers of ncp data points closest to */
/*           each of the ndp data points. */
/* the output parameter is */
/*     pd  = array of dimension 5*ndp, where the estimated */
/*           zx, zy, zxx, zxy, and zyy values at the data */
/*           points are to be stored. */
/* declaration statements */
/* preliminary processing */
    /* Parameter adjustments */
    --pd;
    --ipc;
    --zd;
    --yd;
    --xd;

    /* Function Body */
/* L10: */
    ndp0 = *ndp;
    ncp0 = *ncp;
    ncpm1 = ncp0 - 1;
/* estimation of zx and zy */
/* L20: */
    i_1 = ndp0;
    for (ip0 = 1; ip0 <= i_1; ++ip0) {
	x0 = xd[ip0];
	y0 = yd[ip0];
	z0 = zd[ip0];
	nmx = (float)0.;
	nmy = (float)0.;
	nmz = (float)0.;
	jipc0 = ncp0 * (ip0 - 1);
	i_2 = ncpm1;
	for (ic1 = 1; ic1 <= i_2; ++ic1) {
	    jipc = jipc0 + ic1;
	    ipi = ipc[jipc];
	    dx1 = xd[ipi] - x0;
	    dy1 = yd[ipi] - y0;
	    dz1 = zd[ipi] - z0;
	    ic2mn = ic1 + 1;
	    i_3 = ncp0;
	    for (ic2 = ic2mn; ic2 <= i_3; ++ic2) {
		jipc = jipc0 + ic2;
		ipi = ipc[jipc];
		dx2 = xd[ipi] - x0;
		dy2 = yd[ipi] - y0;
		dnmz = dx1 * dy2 - dy1 * dx2;
		if (dnmz == (float)0.) {
		    goto L22;
		}
		dz2 = zd[ipi] - z0;
		dnmx = dy1 * dz2 - dz1 * dy2;
		dnmy = dz1 * dx2 - dx1 * dz2;
		if (dnmz >= (float)0.) {
		    goto L21;
		}
		dnmx = -(doublereal)dnmx;
		dnmy = -(doublereal)dnmy;
		dnmz = -(doublereal)dnmz;
L21:
		nmx += dnmx;
		nmy += dnmy;
		nmz += dnmz;
L22:
	    ;}
/* L23: */
	}
	jpd0 = ip0 * 5;
	pd[jpd0 - 4] = -(doublereal)nmx / nmz;
	pd[jpd0 - 3] = -(doublereal)nmy / nmz;
/* L24: */
    }
/* estimation of zxx, zxy, and zyy */
/* L30: */
    i_1 = ndp0;
    for (ip0 = 1; ip0 <= i_1; ++ip0) {
	jpd0 += 5;
	x0 = xd[ip0];
	jpd0 = ip0 * 5;
	y0 = yd[ip0];
	zx0 = pd[jpd0 - 4];
	zy0 = pd[jpd0 - 3];
	nmxx = (float)0.;
	nmxy = (float)0.;
	nmyx = (float)0.;
	nmyy = (float)0.;
	nmz = (float)0.;
	jipc0 = ncp0 * (ip0 - 1);
	i_2 = ncpm1;
	for (ic1 = 1; ic1 <= i_2; ++ic1) {
	    jipc = jipc0 + ic1;
	    ipi = ipc[jipc];
	    dx1 = xd[ipi] - x0;
	    dy1 = yd[ipi] - y0;
	    jpd = ipi * 5;
	    dzx1 = pd[jpd - 4] - zx0;
	    dzy1 = pd[jpd - 3] - zy0;
	    ic2mn = ic1 + 1;
	    i_3 = ncp0;
	    for (ic2 = ic2mn; ic2 <= i_3; ++ic2) {
		jipc = jipc0 + ic2;
		ipi = ipc[jipc];
		dx2 = xd[ipi] - x0;
		dy2 = yd[ipi] - y0;
		dnmz = dx1 * dy2 - dy1 * dx2;
		if (dnmz == (float)0.) {
		    goto L32;
		}
		jpd = ipi * 5;
		dzx2 = pd[jpd - 4] - zx0;
		dzy2 = pd[jpd - 3] - zy0;
		dnmxx = dy1 * dzx2 - dzx1 * dy2;
		dnmxy = dzx1 * dx2 - dx1 * dzx2;
		dnmyx = dy1 * dzy2 - dzy1 * dy2;
		dnmyy = dzy1 * dx2 - dx1 * dzy2;
		if (dnmz >= (float)0.) {
		    goto L31;
		}
		dnmxx = -(doublereal)dnmxx;
		dnmxy = -(doublereal)dnmxy;
		dnmyx = -(doublereal)dnmyx;
		dnmyy = -(doublereal)dnmyy;
		dnmz = -(doublereal)dnmz;
L31:
		nmxx += dnmxx;
		nmxy += dnmxy;
		nmyx += dnmyx;
		nmyy += dnmyy;
		nmz += dnmz;
L32:
	    ;}
/* L33: */
	}
	pd[jpd0 - 2] = -(doublereal)nmxx / nmz;
	pd[jpd0 - 1] = -(doublereal)(nmxy + nmyx) / (nmz * (float)2.);
	pd[jpd0] = -(doublereal)nmyy / nmz;
/* L34: */
    }
    return 0;
} /* idpdrv_ */

int idptip_(real* xd, real* yd, real* zd, integer* nt, integer* ipt, integer* nl, integer* ipl, real* pdd, integer* iti, real* xii, real* yii, real* zii) {
    /* System generated locals */
    static real equiv_0[1];

    /* Local variables */
    static integer jpdd, jipl, jipt;
    static real csuv, thus, thsv, thuv, thxu, a, b, c, d;
    static integer i;
    static real u, v, x[3], y[3], z[3], g1, h1, h2, h3, g2, p0, p1, p2, p3,
	    p4;
#define p5 (equiv_0)
    static real x0, y0, aa, ab, bb, ad, bc, cc, cd, dd, ac, p00, ap, bp, cp,
	    pd[15];
#define p50 (equiv_0)
    static real dp, p10, p01, p20, p11, p02, p30, lu, lv, p40, p03, p04, p05,
	    p41, p14, p21, p31, p12, p13, p22, zu[3], zv[3], p32, p23, dx, dy;

    static integer il1, il2, it0, idp, jpd, kpd;
    static real dlt;
    static integer ntl;
    static real zuu[3], zuv[3], zvv[3], act2, bdt2, adbc;

/* this subroutine performs punctual interpolation or extrapola- */
/* tion, i.e., determines the z value at a point. */
/* the input parameters are */
/*     xd,yd,zd = arrays of dimension ndp containing the x, */
/*           y, and z coordinates of the data points, where */
/*           ndp is the number of the data points, */
/*     nt  = number of triangles, */
/*     ipt = integer array of dimension 3*nt containing the */
/*           point numbers of the vertexes of the triangles, */
/*     nl  = number of border line segments, */
/*     ipl = integer array of dimension 3*nl containing the */
/*           point numbers of the end points of the border */
/*           line segments and their respective triangle */
/*           numbers, */
/*     pdd = array of dimension 5*ndp containing the partial */
/*           derivatives at the data points, */
/*     iti = triangle number of the triangle in which lies */
/*           the point for which interpolation is to be */
/*           performed, */
/*     xii,yii = x and y coordinates of the point for which */
/*           interpolation is to be performed. */
/* the output parameter is */
/*     zii = interpolated z value. */
/* declaration statements */
/* preliminary processing */
    /* Parameter adjustments */
    --pdd;
    --ipl;
    --ipt;
    --zd;
    --yd;
    --xd;

    /* Function Body */
/* L10: */
    it0 = *iti;
    ntl = *nt + *nl;
    if (it0 <= ntl) {
	goto L20;
    }
    il1 = it0 / ntl;
    il2 = it0 - il1 * ntl;
    if (il1 == il2) {
	goto L40;
    }
    goto L60;
/* calculation of zii by interpolation. */
/* checks if the necessary coefficients have been calculated. */
L20:
    if (it0 == idpi_1.itpv) {
	goto L30;
    }
/* loads coordinate and partial derivative values at the */
/* vertexes. */
/* L21: */
    jipt = (it0 - 1) * 3;
    jpd = 0;
    for (i = 1; i <= 3; ++i) {
	++jipt;
	idp = ipt[jipt];
	x[i - 1] = xd[idp];
	y[i - 1] = yd[idp];
	z[i - 1] = zd[idp];
	jpdd = (idp - 1) * 5;
	for (kpd = 1; kpd <= 5; ++kpd) {
	    ++jpd;
	    ++jpdd;
	    pd[jpd - 1] = pdd[jpdd];
/* L22: */
	}
/* L23: */
    }
/* determines the coefficients for the coordinate system */
/* transformation from the x-y system to the u-v system */
/* and vice versa. */
/* L24: */
    x0 = x[0];
    y0 = y[0];
    a = x[1] - x0;
    b = x[2] - x0;
    c = y[1] - y0;
    d = y[2] - y0;
    ad = a * d;
    bc = b * c;
    dlt = ad - bc;
    ap = d / dlt;
    bp = -(doublereal)b / dlt;
    cp = -(doublereal)c / dlt;
    dp = a / dlt;
/* converts the partial derivatives at the vertexes of the */
/* triangle for the u-v coordinate system. */
/* L25: */
    aa = a * a;
    act2 = a * (float)2. * c;
    cc = c * c;
    ab = a * b;
    adbc = ad + bc;
    cd = c * d;
    bb = b * b;
    bdt2 = b * (float)2. * d;
    dd = d * d;
    for (i = 1; i <= 3; ++i) {
	jpd = i * 5;
	zu[i - 1] = a * pd[jpd - 5] + c * pd[jpd - 4];
	zv[i - 1] = b * pd[jpd - 5] + d * pd[jpd - 4];
	zuu[i - 1] = aa * pd[jpd - 3] + act2 * pd[jpd - 2] + cc * pd[jpd - 1];

	zuv[i - 1] = ab * pd[jpd - 3] + adbc * pd[jpd - 2] + cd * pd[jpd - 1];

	zvv[i - 1] = bb * pd[jpd - 3] + bdt2 * pd[jpd - 2] + dd * pd[jpd - 1];

/* L26: */
    }
/* calculates the coefficients of the polynomial. */
/* L27: */
    p00 = z[0];
    p10 = zu[0];
    p01 = zv[0];
    p20 = zuu[0] * (float).5;
    p11 = zuv[0];
    p02 = zvv[0] * (float).5;
    h1 = z[1] - p00 - p10 - p20;
    h2 = zu[1] - p10 - zuu[0];
    h3 = zuu[1] - zuu[0];
    p30 = h1 * (float)10. - h2 * (float)4. + h3 * (float).5;
    p40 = h1 * (float)-15. + h2 * (float)7. - h3;
    *p50 = h1 * (float)6. - h2 * (float)3. + h3 * (float).5;
    h1 = z[2] - p00 - p01 - p02;
    h2 = zv[2] - p01 - zvv[0];
    h3 = zvv[2] - zvv[0];
    p03 = h1 * (float)10. - h2 * (float)4. + h3 * (float).5;
    p04 = h1 * (float)-15. + h2 * (float)7. - h3;
    p05 = h1 * (float)6. - h2 * (float)3. + h3 * (float).5;
    lu = sqrt(aa + cc);
    lv = sqrt(bb + dd);
    thxu = atan2(c, a);
    thuv = atan2(d, b) - thxu;
    csuv = cos(thuv);
    p41 = lv * (float)5. * csuv / lu * *p50;
    p14 = lu * (float)5. * csuv / lv * p05;
    h1 = zv[1] - p01 - p11 - p41;
    h2 = zuv[1] - p11 - p41 * (float)4.;
    p21 = h1 * (float)3. - h2;
    p31 = h1 * (float)-2. + h2;
    h1 = zu[2] - p10 - p11 - p14;
    h2 = zuv[2] - p11 - p14 * (float)4.;
    p12 = h1 * (float)3. - h2;
    p13 = h1 * (float)-2. + h2;
    thus = atan2(d - c, b - a) - thxu;
    thsv = thuv - thus;
    aa = sin(thsv) / lu;
    bb = -(doublereal)cos(thsv) / lu;
    cc = sin(thus) / lv;
    dd = cos(thus) / lv;
    ac = aa * cc;
    ad = aa * dd;
    bc = bb * cc;
    g1 = aa * ac * (bc * (float)3. + ad * (float)2.);
    g2 = cc * ac * (ad * (float)3. + bc * (float)2.);
    h1 = -(doublereal)aa * aa * aa * (aa * (float)5. * bb * *p50 + (bc * (
	    float)4. + ad) * p41) - cc * cc * cc * (cc * (float)5. * dd * p05
	    + (ad * (float)4. + bc) * p14);
    h2 = zvv[1] * (float).5 - p02 - p12;
    h3 = zuu[2] * (float).5 - p20 - p21;
    p22 = (g1 * h2 + g2 * h3 - h1) / (g1 + g2);
    p32 = h2 - p22;
    p23 = h3 - p22;
    idpi_1.itpv = it0;
/* converts xii and yii to u-v system. */
L30:
    dx = *xii - x0;
    dy = *yii - y0;
    u = ap * dx + bp * dy;
    v = cp * dx + dp * dy;
/* evaluates the polynomial. */
/* L31: */
    p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
    p1 = p10 + v * (p11 + v * (p12 + v * (p13 + v * p14)));
    p2 = p20 + v * (p21 + v * (p22 + v * p23));
    p3 = p30 + v * (p31 + v * p32);
    p4 = p40 + v * p41;
    *zii = p0 + u * (p1 + u * (p2 + u * (p3 + u * (p4 + u * *p5))));
    return 0;
/* calculation of zii by extrapolation in the rectangle. */
/* checks if the necessary coefficients have been calculated. */
L40:
    if (it0 == idpi_1.itpv) {
	goto L50;
    }
/* loads coordinate and partial derivative values at the end */
/* points of the border line segment. */
/* L41: */
    jipl = (il1 - 1) * 3;
    jpd = 0;
    for (i = 1; i <= 2; ++i) {
	++jipl;
	idp = ipl[jipl];
	x[i - 1] = xd[idp];
	y[i - 1] = yd[idp];
	z[i - 1] = zd[idp];
	jpdd = (idp - 1) * 5;
	for (kpd = 1; kpd <= 5; ++kpd) {
	    ++jpd;
	    ++jpdd;
	    pd[jpd - 1] = pdd[jpdd];
/* L42: */
	}
/* L43: */
    }
/* determines the coefficients for the coordinate system */
/* transformation from the x-y system to the u-v system */
/* and vice versa. */
/* L44: */
    x0 = x[0];
    y0 = y[0];
    a = y[1] - y[0];
    b = x[1] - x[0];
    c = -(doublereal)b;
    d = a;
    ad = a * d;
    bc = b * c;
    dlt = ad - bc;
    ap = d / dlt;
    bp = -(doublereal)b / dlt;
    cp = -(doublereal)bp;
    dp = ap;
/* converts the partial derivatives at the end points of the */
/* border line segment for the u-v coordinate system. */
/* L45: */
    aa = a * a;
    act2 = a * (float)2. * c;
    cc = c * c;
    ab = a * b;
    adbc = ad + bc;
    cd = c * d;
    bb = b * b;
    bdt2 = b * (float)2. * d;
    dd = d * d;
    for (i = 1; i <= 2; ++i) {
	jpd = i * 5;
	zu[i - 1] = a * pd[jpd - 5] + c * pd[jpd - 4];
	zv[i - 1] = b * pd[jpd - 5] + d * pd[jpd - 4];
	zuu[i - 1] = aa * pd[jpd - 3] + act2 * pd[jpd - 2] + cc * pd[jpd - 1];

	zuv[i - 1] = ab * pd[jpd - 3] + adbc * pd[jpd - 2] + cd * pd[jpd - 1];

	zvv[i - 1] = bb * pd[jpd - 3] + bdt2 * pd[jpd - 2] + dd * pd[jpd - 1];

/* L46: */
    }
/* calculates the coefficients of the polynomial. */
/* L47: */
    p00 = z[0];
    p10 = zu[0];
    p01 = zv[0];
    p20 = zuu[0] * (float).5;
    p11 = zuv[0];
    p02 = zvv[0] * (float).5;
    h1 = z[1] - p00 - p01 - p02;
    h2 = zv[1] - p01 - zvv[0];
    h3 = zvv[1] - zvv[0];
    p03 = h1 * (float)10. - h2 * (float)4. + h3 * (float).5;
    p04 = h1 * (float)-15. + h2 * (float)7. - h3;
    p05 = h1 * (float)6. - h2 * (float)3. + h3 * (float).5;
    h1 = zu[1] - p10 - p11;
    h2 = zuv[1] - p11;
    p12 = h1 * (float)3. - h2;
    p13 = h1 * (float)-2. + h2;
    p21 = (float)0.;
    p23 = -(doublereal)zuu[1] + zuu[0];
    p22 = p23 * (float)-1.5;
    idpi_1.itpv = it0;
/* converts xii and yii to u-v system. */
L50:
    dx = *xii - x0;
    dy = *yii - y0;
    u = ap * dx + bp * dy;
    v = cp * dx + dp * dy;
/* evaluates the polynomial. */
/* L51: */
    p0 = p00 + v * (p01 + v * (p02 + v * (p03 + v * (p04 + v * p05))));
    p1 = p10 + v * (p11 + v * (p12 + v * p13));
    p2 = p20 + v * (p21 + v * (p22 + v * p23));
    *zii = p0 + u * (p1 + u * p2);
    return 0;
/* calculation of zii by extrapolation in the triangle. */
/* checks if the necessary coefficients have been calculated. */
L60:
    if (it0 == idpi_1.itpv) {
	goto L70;
    }
/* loads coordinate and partial derivative values at the vertex */
/* of the triangle. */
/* L61: */
    jipl = il2 * 3 - 2;
    idp = ipl[jipl];
    x[0] = xd[idp];
    y[0] = yd[idp];
    z[0] = zd[idp];
    jpdd = (idp - 1) * 5;
    for (kpd = 1; kpd <= 5; ++kpd) {
	++jpdd;
	pd[kpd - 1] = pdd[jpdd];
/* L62: */
    }
/* calculates the coefficients of the polynomial. */
/* L67: */
    p00 = z[0];
    p10 = pd[0];
    p01 = pd[1];
    p20 = pd[2] * (float).5;
    p11 = pd[3];
    p02 = pd[4] * (float).5;
    idpi_1.itpv = it0;
/* converts xii and yii to u-v system. */
L70:
    u = *xii - x[0];
    v = *yii - y[0];
/* evaluates the polynomial. */
/* L71: */
    p0 = p00 + v * (p01 + v * p02);
    p1 = p10 + v * p11;
    *zii = p0 + u * (p1 + u * p20);
    return 0;
} /* idptip_ */

#undef p50
#undef p5

int idsfft_(integer *md, integer *ncp, integer *ndp, real *xd, real *yd, real *zd, integer *nxi, integer *nyi, real *xi, real *yi, real *zi, integer *iwk, real *wk) {
    /* System generated locals */
    integer i_1, i_2;

    /* Local variables */
    static integer jigp, jngp, nngp;
    static integer jwipc, jwigp, jwipl, ncppv, ndppv, jwngp, jwiwl, jwipt,
	    jwiwp, nxipv, nyipv, jig0mn, jig1mn, jig0mx, jig1mx, jwigp0,
	    jwngp0, nl;
    static integer nt;
    static integer md0, il1, il2, iti, ixi, izi, iyi, ncp0, ndp0, ngp0, ngp1,
	    nxi0, nyi0;

/* this subroutine performs smooth surface fitting when the pro- */
/* jections of the data points in the x-y plane are irregularly */
/* distributed in the plane. */
/* the input parameters are */
/*     md  = mode of computation (must be 1, 2, or 3), */
/*         = 1 for new ncp and/or new xd-yd, */
/*         = 2 for old ncp, old xd-yd, new xi-yi, */
/*         = 3 for old ncp, old xd-yd, old xi-yi, */
/*     ncp = number of additional data points used for esti- */
/*           mating partial derivatives at each data point */
/*           (must be 2 or greater, but smaller than ndp), */
/*     ndp = number of data points (must be 4 or greater), */
/*     xd  = array of dimension ndp containing the x */
/*           coordinates of the data points, */
/*     yd  = array of dimension ndp containing the y */
/*           coordinates of the data points, */
/*     zd  = array of dimension ndp containing the z */
/*           coordinates of the data points, */
/*     nxi = number of output grid points in the x coordinate */
/*           (must be 1 or greater), */
/*     nyi = number of output grid points in the y coordinate */
/*           (must be 1 or greater), */
/*     xi  = array of dimension nxi containing the x */
/*           coordinates of the output grid points, */
/*     yi  = array of dimension nyi containing the y */
/*           coordinates of the output grid points. */
/* the output parameter is */
/*     zi  = doubly-dimensioned array of dimension (nxi,nyi), */
/*           where the interpolated z values at the output */
/*           grid points are to be stored. */
/* the other parameters are */
/*     iwk = integer array of dimension */
/*              max0(31,27+ncp)*ndp+nxi*nyi */
/*           used internally as a work area, */
/*     wk  = array of dimension 5*ndp used internally as a */
/*           work area. */
/* the very first call to this subroutine and the call with a new */
/* ncp value, a new ndp value, and/or new contents of the xd and */
/* yd arrays must be made with md=1.  the call with md=2 must be */
/* preceded by another call with the same ncp and ndp values and */
/* with the same contents of the xd and yd arrays.  the call with */
/* md=3 must be preceded by another call with the same ncp, ndp, */
/* nxi, and nyi values and with the same contents of the xd, yd, */
/* xi, and yi arrays.  between the call with md=2 or md=3 and its */
/* preceding call, the iwk and wk arrays must not be disturbed. */
/* use of a value between 3 and 5 (inclusive) for ncp is recom- */
/* mended unless there are evidences that dictate otherwise. */
/* the lun constant in the data initialization statement is the */
/* logical unit number of the standard output unit and is, */
/* therefore, system dependent. */
/* this subroutine calls the idcldp, idgrid, idpdrv, idptip, and */
/* idtang subroutines. */
/* declaration statements */
    /* Parameter adjustments */
    --wk;
    --iwk;
    --zi;
    --yi;
    --xi;
    --zd;
    --yd;
    --xd;

    /* Function Body */
/* setting of some input parameters to local variables. */
/* (for md=1,2,3) */
/* L10: */
    md0 = *md;
    ncp0 = *ncp;
    ndp0 = *ndp;
    nxi0 = *nxi;
    nyi0 = *nyi;
/* error check.  (for md=1,2,3) */
/* L20: */
    if (md0 < 1 || md0 > 3) {
	goto L90;
    }
    if (ncp0 < 2 || ncp0 >= ndp0) {
	goto L90;
    }
    if (ndp0 < 4) {
	goto L90;
    }
    if (nxi0 < 1 || nyi0 < 1) {
	goto L90;
    }
    if (md0 >= 2) {
	goto L21;
    }
    iwk[1] = ncp0;
    iwk[2] = ndp0;
    goto L22;
L21:
    ncppv = iwk[1];
    ndppv = iwk[2];
    if (ncp0 != ncppv) {
	goto L90;
    }
    if (ndp0 != ndppv) {
	goto L90;
    }
L22:
    if (md0 >= 3) {
	goto L23;
    }
    iwk[3] = nxi0;
    iwk[4] = nyi0;
    goto L30;
L23:
    nxipv = iwk[3];
    nyipv = iwk[4];
    if (nxi0 != nxipv) {
	goto L90;
    }
    if (nyi0 != nyipv) {
	goto L90;
    }
/* allocation of storage areas in the iwk array.  (for md=1,2,3) */
L30:
    jwipt = 16;
    jwiwl = ndp0 * 6 + 1;
    jwngp0 = jwiwl - 1;
    jwipl = ndp0 * 24 + 1;
    jwiwp = ndp0 * 30 + 1;
    jwipc = ndp0 * 27 + 1;
/* Computing MAX */
    i_1 = 31, i_2 = ncp0 + 27;
    jwigp0 = max(i_1,i_2) * ndp0;
/* triangulates the x-y plane.  (for md=1) */
/* L40: */
    if (md0 > 1) {
	goto L50;
    }
    idtang_(&ndp0, &xd[1], &yd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl], &iwk[
	    jwiwl], &iwk[jwiwp], &wk[1]);
    iwk[5] = nt;
    iwk[6] = nl;
    if (nt == 0) {
	return 0;
    }
/* determines ncp points closest to each data point.  (for md=1) */
L50:
    if (md0 > 1) {
	goto L60;
    }
    idcldp_(&ndp0, &xd[1], &yd[1], &ncp0, &iwk[jwipc]);
    if (iwk[jwipc] == 0) {
	return 0;
    }
/* sorts output grid points in ascending order of the triangle */
/* number and the border line segment number.  (for md=1,2) */
L60:
    if (md0 == 3) {
	goto L70;
    }
    idgrid_(&xd[1], &yd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl], &nxi0, &nyi0,
	    &xi[1], &yi[1], &iwk[jwngp0 + 1], &iwk[jwigp0 + 1]);
/* estimates partial derivatives at all data points. */
/* (for md=1,2,3) */
L70:
    idpdrv_(&ndp0, &xd[1], &yd[1], &zd[1], &ncp0, &iwk[jwipc], &wk[1]);
/* interpolates the zi values.  (for md=1,2,3) */
/* L80: */
    idpi_1.itpv = 0;
    jig0mx = 0;
    jig1mn = nxi0 * nyi0 + 1;
    nngp = nt + (nl << 1);
    i_1 = nngp;
    for (jngp = 1; jngp <= i_1; ++jngp) {
	iti = jngp;
	if (jngp <= nt) {
	    goto L81;
	}
	il1 = (jngp - nt + 1) / 2;
	il2 = (jngp - nt + 2) / 2;
	if (il2 > nl) {
	    il2 = 1;
	}
	iti = il1 * (nt + nl) + il2;
L81:
	jwngp = jwngp0 + jngp;
	ngp0 = iwk[jwngp];
	if (ngp0 == 0) {
	    goto L86;
	}
	jig0mn = jig0mx + 1;
	jig0mx += ngp0;
	i_2 = jig0mx;
	for (jigp = jig0mn; jigp <= i_2; ++jigp) {
	    jwigp = jwigp0 + jigp;
	    izi = iwk[jwigp];
	    iyi = (izi - 1) / nxi0 + 1;
	    ixi = izi - nxi0 * (iyi - 1);
	    idptip_(&xd[1], &yd[1], &zd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl]
		    , &wk[1], &iti, &xi[ixi], &yi[iyi], &zi[izi]);
/* L82: */
	}
L86:
	jwngp = jwngp0 + (nngp << 1) + 1 - jngp;
	ngp1 = iwk[jwngp];
	if (ngp1 == 0) {
	    goto L89;
	}
	jig1mx = jig1mn - 1;
	jig1mn -= ngp1;
	i_2 = jig1mx;
	for (jigp = jig1mn; jigp <= i_2; ++jigp) {
	    jwigp = jwigp0 + jigp;
	    izi = iwk[jwigp];
	    iyi = (izi - 1) / nxi0 + 1;
	    ixi = izi - nxi0 * (iyi - 1);
	    idptip_(&xd[1], &yd[1], &zd[1], &nt, &iwk[jwipt], &nl, &iwk[jwipl]
		    , &wk[1], &iti, &xi[ixi], &yi[iyi], &zi[izi]);
/* L87: */
	}
L89:
    ;}
    return 0;
/* error exit */
L90:
    err2090_();
    return 0;
/* format statement for error message */
/* L2090: */
} /* idsfft_ */

int idtang_(integer *ndp, real *xd, real *yd, integer *nt, integer *ipt, integer *nl, integer *ipl, integer *iwl, integer *iwp, real *wk) {
    /* Initialized data */

    static real ratio = (float)1e-6;
    static integer nrep = 100;

    /* System generated locals */
    integer i_1, i_2, i_3, i_4;
    real r_1, r_2;

    /* Local variables */
    static integer nlfc, ip1p1;
    static real dsq12, armn;
    static integer irep;
    static real dsqi;
    static integer jp2t3, jp3t3, jpmn;
    static real dxmn, dymn, xdmp, ydmp, armx;
    static integer ipti, it1t3, it2t3, jpmx;
    static real dxmx, dymx;
    static integer ndpm1, ilft2, iplj1, iplj2, ipmn1, ipmn2, ipti1, ipti2,
	    nlft2, nlnt3, nsht3, itt3r;
    static real dsqmn;
    static integer ntt3p3;
    static real dsqmx, x1, y1;
    static integer jwl1mn;
    static real ar;
    static integer ip, jp;
    static real dx, dy;
    static integer it;
    static integer ip1, ip2, jp1, jp2, ip3, nl0, nt0, ilf, jpc;
    static real dx21, dy21;
    static integer nlf, itf[2], nln, nsh, ntf, jwl, its, ndp0, ipl1, ipl2,
	    jlt3, ipt1, ipt2, ipt3, nlt3, jwl1, itt3, ntt3;

/* this subroutine performs triangulation.  it divides the x-y */
/* plane into a number of triangles according to given data */
/* points in the plane, determines line segments that form the */
/* border of data area, and determines the triangle numbers */
/* corresponding to the border line segments. */
/* at completion, point numbers of the vertexes of each triangle */
/* are listed counter-clockwise.  point numbers of the end points */
/* of each border line segment are listed counter-clockwise, */
/* listing order of the line segments being counter-clockwise. */
/* the lun constant in the data initialization statement is the */
/* logical unit number of the standard output unit and is, */
/* therefore, system dependent. */
/* this subroutine calls the idxchg function. */
/* the input parameters are */
/*     ndp = number of data points, */
/*     xd  = array of dimension ndp containing the */
/*           x coordinates of the data points, */
/*     yd  = array of dimension ndp containing the */
/*           y coordinates of the data points. */
/* the output parameters are */
/*     nt  = number of triangles, */
/*     ipt = integer array of dimension 6*ndp-15, where the */
/*           point numbers of the vertexes of the (it)th */
/*           triangle are to be stored as the (3*it-2)nd, */
/*           (3*it-1)st, and (3*it)th elements, */
/*           it=1,2,...,nt, */
/*     nl  = number of border line segments, */
/*     ipl = integer array of dimension 6*ndp, where the */
/*           point numbers of the end points of the (il)th */
/*           border line segment and its respective triangle */
/*           number are to be stored as the (3*il-2)nd, */
/*           (3*il-1)st, and (3*il)th elements, */
/*           il=1,2,..., nl. */
/* the other parameters are */
/*     iwl = integer array of dimension 18*ndp used */
/*           internally as a work area, */
/*     iwp = integer array of dimension ndp used */
/*           internally as a work area, */
/*     wk  = array of dimension ndp used internally as a */
/*           work area. */
/* declaration statements */
    /* Parameter adjustments */
    --wk;
    --iwp;
    --iwl;
    --ipl;
    --ipt;
    --yd;
    --xd;

    /* Function Body */
/* statement functions */
/* preliminary processing */
/* L10: */
    ndp0 = *ndp;
    ndpm1 = ndp0 - 1;
    if (ndp0 < 4) {
	goto L90;
    }
/* determines the closest pair of data points and their midpoint. */
/* L20: */
/* Computing 2nd power */
    r_1 = xd[2] - xd[1];
/* Computing 2nd power */
    r_2 = yd[2] - yd[1];
    dsqmn = r_1 * r_1 + r_2 * r_2;
    ipmn1 = 1;
    ipmn2 = 2;
    i_1 = ndpm1;
    for (ip1 = 1; ip1 <= i_1; ++ip1) {
	x1 = xd[ip1];
	y1 = yd[ip1];
	ip1p1 = ip1 + 1;
	i_2 = ndp0;
	for (ip2 = ip1p1; ip2 <= i_2; ++ip2) {
/* Computing 2nd power */
	    r_1 = xd[ip2] - x1;
/* Computing 2nd power */
	    r_2 = yd[ip2] - y1;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    if (dsqi == (float)0.) {
		goto L91;
	    }
	    if (dsqi >= dsqmn) {
		goto L21;
	    }
	    dsqmn = dsqi;
	    ipmn1 = ip1;
	    ipmn2 = ip2;
L21:
	;}
/* L22: */
    }
    dsq12 = dsqmn;
    xdmp = (xd[ipmn1] + xd[ipmn2]) / (float)2.;
    ydmp = (yd[ipmn1] + yd[ipmn2]) / (float)2.;
/* sorts the other (ndp-2) data points in ascending order of */
/* distance from the midpoint and stores the sorted data point */
/* numbers in the iwp array. */
/* L30: */
    jp1 = 2;
    i_1 = ndp0;
    for (ip1 = 1; ip1 <= i_1; ++ip1) {
	if (ip1 == ipmn1 || ip1 == ipmn2) {
	    goto L31;
	}
	++jp1;
	iwp[jp1] = ip1;
/* Computing 2nd power */
	r_1 = xd[ip1] - xdmp;
/* Computing 2nd power */
	r_2 = yd[ip1] - ydmp;
	wk[jp1] = r_1 * r_1 + r_2 * r_2;
L31:
    ;}
    i_1 = ndpm1;
    for (jp1 = 3; jp1 <= i_1; ++jp1) {
	dsqmn = wk[jp1];
	jpmn = jp1;
	i_2 = ndp0;
	for (jp2 = jp1; jp2 <= i_2; ++jp2) {
	    if (wk[jp2] >= dsqmn) {
		goto L32;
	    }
	    dsqmn = wk[jp2];
	    jpmn = jp2;
L32:
	;}
	its = iwp[jp1];
	iwp[jp1] = iwp[jpmn];
	iwp[jpmn] = its;
	wk[jpmn] = wk[jp1];
/* L33: */
    }
/* if necessary, modifies the ordering in such a way that the */
/* first three data points are not collinear. */
/* L35: */
    ar = dsq12 * ratio;
    x1 = xd[ipmn1];
    y1 = yd[ipmn1];
    dx21 = xd[ipmn2] - x1;
    dy21 = yd[ipmn2] - y1;
    i_1 = ndp0;
    for (jp = 3; jp <= i_1; ++jp) {
	ip = iwp[jp];
	if ((r_1 = (yd[ip] - y1) * dx21 - (xd[ip] - x1) * dy21, dabs(r_1)) >
		ar) {
	    goto L37;
	}
/* L36: */
    }
    goto L92;
L37:
    if (jp == 3) {
	goto L40;
    }
    jpmx = jp;
    jp = jpmx + 1;
    i_1 = jpmx;
    for (jpc = 4; jpc <= i_1; ++jpc) {
	--jp;
	iwp[jp] = iwp[jp - 1];
/* L38: */
    }
    iwp[3] = ip;
/* forms the first triangle.  stores point numbers of the ver- */
/* texes of the triangle in the ipt array, and stores point num- */
/* bers of the border line segments and the triangle number in */
/* the ipl array. */
L40:
    ip1 = ipmn1;
    ip2 = ipmn2;
    ip3 = iwp[3];
    if ((yd[ip3] - yd[ip1]) * (xd[ip2] - xd[ip1]) - (xd[ip3] - xd[ip1]) * (yd[
	    ip2] - yd[ip1]) >= (float)0.) {
	goto L41;
    }
    ip1 = ipmn2;
    ip2 = ipmn1;
L41:
    nt0 = 1;
    ntt3 = 3;
    ipt[1] = ip1;
    ipt[2] = ip2;
    ipt[3] = ip3;
    nl0 = 3;
    nlt3 = 9;
    ipl[1] = ip1;
    ipl[2] = ip2;
    ipl[3] = 1;
    ipl[4] = ip2;
    ipl[5] = ip3;
    ipl[6] = 1;
    ipl[7] = ip3;
    ipl[8] = ip1;
    ipl[9] = 1;
/* adds the remaining (ndp-3) data points, one by one. */
/* L50: */
    i_1 = ndp0;
    for (jp1 = 4; jp1 <= i_1; ++jp1) {
	ip1 = iwp[jp1];
	x1 = xd[ip1];
	y1 = yd[ip1];
/* - determines the visible border line segments. */
	ip2 = ipl[1];
	jpmn = 1;
	dxmn = xd[ip2] - x1;
	dymn = yd[ip2] - y1;
/* Computing 2nd power */
	r_1 = dxmn;
/* Computing 2nd power */
	r_2 = dymn;
	dsqmn = r_1 * r_1 + r_2 * r_2;
	armn = dsqmn * ratio;
	jpmx = 1;
	dxmx = dxmn;
	dymx = dymn;
	dsqmx = dsqmn;
	armx = armn;
	i_2 = nl0;
	for (jp2 = 2; jp2 <= i_2; ++jp2) {
	    ip2 = ipl[jp2 * 3 - 2];
	    dx = xd[ip2] - x1;
	    dy = yd[ip2] - y1;
	    ar = dy * dxmn - dx * dymn;
	    if (ar > armn) {
		goto L51;
	    }
/* Computing 2nd power */
	    r_1 = dx;
/* Computing 2nd power */
	    r_2 = dy;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    if (ar >= -(doublereal)armn && dsqi >= dsqmn) {
		goto L51;
	    }
	    jpmn = jp2;
	    dxmn = dx;
	    dymn = dy;
	    dsqmn = dsqi;
	    armn = dsqmn * ratio;
L51:
	    ar = dy * dxmx - dx * dymx;
	    if (ar < -(doublereal)armx) {
		goto L52;
	    }
/* Computing 2nd power */
	    r_1 = dx;
/* Computing 2nd power */
	    r_2 = dy;
	    dsqi = r_1 * r_1 + r_2 * r_2;
	    if (ar <= armx && dsqi >= dsqmx) {
		goto L52;
	    }
	    jpmx = jp2;
	    dxmx = dx;
	    dymx = dy;
	    dsqmx = dsqi;
	    armx = dsqmx * ratio;
L52:
	;}
	if (jpmx < jpmn) {
	    jpmx += nl0;
	}
	nsh = jpmn - 1;
	if (nsh <= 0) {
	    goto L60;
	}
/* - shifts (rotates) the ipl array to have the invisible border */
/* - line segments contained in the first part of the ipl array. */
	nsht3 = nsh * 3;
	i_2 = nsht3;
	for (jp2t3 = 3; jp2t3 <= i_2; jp2t3 += 3) {
	    jp3t3 = jp2t3 + nlt3;
	    ipl[jp3t3 - 2] = ipl[jp2t3 - 2];
	    ipl[jp3t3 - 1] = ipl[jp2t3 - 1];
	    ipl[jp3t3] = ipl[jp2t3];
/* L53: */
	}
	i_2 = nlt3;
	for (jp2t3 = 3; jp2t3 <= i_2; jp2t3 += 3) {
	    jp3t3 = jp2t3 + nsht3;
	    ipl[jp2t3 - 2] = ipl[jp3t3 - 2];
	    ipl[jp2t3 - 1] = ipl[jp3t3 - 1];
	    ipl[jp2t3] = ipl[jp3t3];
/* L54: */
	}
	jpmx -= nsh;
/* - adds triangles to the ipt array, updates border line */
/* - segments in the ipl array, and sets flags for the border */
/* - line segments to be reexamined in the iwl array. */
L60:
	jwl = 0;
	i_2 = nl0;
	for (jp2 = jpmx; jp2 <= i_2; ++jp2) {
	    jp2t3 = jp2 * 3;
	    ipl1 = ipl[jp2t3 - 2];
	    ipl2 = ipl[jp2t3 - 1];
	    it = ipl[jp2t3];
/* - - adds a triangle to the ipt array. */
	    ++nt0;
	    ntt3 += 3;
	    ipt[ntt3 - 2] = ipl2;
	    ipt[ntt3 - 1] = ipl1;
	    ipt[ntt3] = ip1;
/* - - updates border line segments in the ipl array. */
	    if (jp2 != jpmx) {
		goto L61;
	    }
	    ipl[jp2t3 - 1] = ip1;
	    ipl[jp2t3] = nt0;
L61:
	    if (jp2 != nl0) {
		goto L62;
	    }
	    nln = jpmx + 1;
	    nlnt3 = nln * 3;
	    ipl[nlnt3 - 2] = ip1;
	    ipl[nlnt3 - 1] = ipl[1];
	    ipl[nlnt3] = nt0;
/* - - determines the vertex that does not lie on the border */
/* - - line segments. */
L62:
	    itt3 = it * 3;
	    ipti = ipt[itt3 - 2];
	    if (ipti != ipl1 && ipti != ipl2) {
		goto L63;
	    }
	    ipti = ipt[itt3 - 1];
	    if (ipti != ipl1 && ipti != ipl2) {
		goto L63;
	    }
	    ipti = ipt[itt3];
/* - - checks if the exchange is necessary. */
L63:
	    if (idxchg_(&xd[1], &yd[1], &ip1, &ipti, &ipl1, &ipl2) == 0) {
		goto L64;
	    }
/* - - modifies the ipt array when necessary. */
	    ipt[itt3 - 2] = ipti;
	    ipt[itt3 - 1] = ipl1;
	    ipt[itt3] = ip1;
	    ipt[ntt3 - 1] = ipti;
	    if (jp2 == jpmx) {
		ipl[jp2t3] = it;
	    }
	    if (jp2 == nl0 && ipl[3] == it) {
		ipl[3] = nt0;
	    }
/* - - sets flags in the iwl array. */
	    jwl += 4;
	    iwl[jwl - 3] = ipl1;
	    iwl[jwl - 2] = ipti;
	    iwl[jwl - 1] = ipti;
	    iwl[jwl] = ipl2;
L64:
	;}
	nl0 = nln;
	nlt3 = nlnt3;
	nlf = jwl / 2;
	if (nlf == 0) {
	    goto L79;
	}
/* - improves triangulation. */
/* L70: */
	ntt3p3 = ntt3 + 3;
	i_2 = nrep;
	for (irep = 1; irep <= i_2; ++irep) {
	    i_3 = nlf;
	    for (ilf = 1; ilf <= i_3; ++ilf) {
		ilft2 = ilf << 1;
		ipl1 = iwl[ilft2 - 1];
		ipl2 = iwl[ilft2];
/* - - locates in the ipt array two triangles on both sides
of */
/* - - the flagged line segment. */
		ntf = 0;
		i_4 = ntt3;
		for (itt3r = 3; itt3r <= i_4; itt3r += 3) {
		    itt3 = ntt3p3 - itt3r;
		    ipt1 = ipt[itt3 - 2];
		    ipt2 = ipt[itt3 - 1];
		    ipt3 = ipt[itt3];
		    if (ipl1 != ipt1 && ipl1 != ipt2 && ipl1 != ipt3) {
			goto L71;
		    }
		    if (ipl2 != ipt1 && ipl2 != ipt2 && ipl2 != ipt3) {
			goto L71;
		    }
		    ++ntf;
		    itf[ntf - 1] = itt3 / 3;
		    if (ntf == 2) {
			goto L72;
		    }
L71:
		;}
		if (ntf < 2) {
		    goto L76;
		}
/* - - determines the vertexes of the triangles that do not
lie */
/* - - on the line segment. */
L72:
		it1t3 = itf[0] * 3;
		ipti1 = ipt[it1t3 - 2];
		if (ipti1 != ipl1 && ipti1 != ipl2) {
		    goto L73;
		}
		ipti1 = ipt[it1t3 - 1];
		if (ipti1 != ipl1 && ipti1 != ipl2) {
		    goto L73;
		}
		ipti1 = ipt[it1t3];
L73:
		it2t3 = itf[1] * 3;
		ipti2 = ipt[it2t3 - 2];
		if (ipti2 != ipl1 && ipti2 != ipl2) {
		    goto L74;
		}
		ipti2 = ipt[it2t3 - 1];
		if (ipti2 != ipl1 && ipti2 != ipl2) {
		    goto L74;
		}
		ipti2 = ipt[it2t3];
/* - - checks if the exchange is necessary. */
L74:
		if (idxchg_(&xd[1], &yd[1], &ipti1, &ipti2, &ipl1, &ipl2) ==
			0) {
		    goto L76;
		}
/* - - modifies the ipt array when necessary. */
		ipt[it1t3 - 2] = ipti1;
		ipt[it1t3 - 1] = ipti2;
		ipt[it1t3] = ipl1;
		ipt[it2t3 - 2] = ipti2;
		ipt[it2t3 - 1] = ipti1;
		ipt[it2t3] = ipl2;
/* - - sets new flags. */
		jwl += 8;
		iwl[jwl - 7] = ipl1;
		iwl[jwl - 6] = ipti1;
		iwl[jwl - 5] = ipti1;
		iwl[jwl - 4] = ipl2;
		iwl[jwl - 3] = ipl2;
		iwl[jwl - 2] = ipti2;
		iwl[jwl - 1] = ipti2;
		iwl[jwl] = ipl1;
		i_4 = nlt3;
		for (jlt3 = 3; jlt3 <= i_4; jlt3 += 3) {
		    iplj1 = ipl[jlt3 - 2];
		    iplj2 = ipl[jlt3 - 1];
		    if ((iplj1 == ipl1 && iplj2 == ipti2) || (iplj2 == ipl1 && iplj1 == ipti2)) {
		    	ipl[jlt3] = itf[0];
		    }
		    if ((iplj1 == ipl2 && iplj2 == ipti1) || (iplj2 == ipl2 && iplj1 == ipti1)) {
		    	ipl[jlt3] = itf[1];
		    }
/* L75: */
		}
L76:
	    ;}
	    nlfc = nlf;
	    nlf = jwl / 2;
	    if (nlf == nlfc) {
		goto L79;
	    }
/* - - resets the iwl array for the next round. */
	    jwl = 0;
	    jwl1mn = (nlfc + 1) << 1;
	    nlft2 = nlf << 1;
	    i_3 = nlft2;
	    for (jwl1 = jwl1mn; jwl1 <= i_3; jwl1 += 2) {
		jwl += 2;
		iwl[jwl - 1] = iwl[jwl1 - 1];
		iwl[jwl] = iwl[jwl1];
/* L77: */
	    }
	    nlf = jwl / 2;
/* L78: */
	}
L79:
    ;}
/* rearranges the ipt array so that the vertexes of each triangle */
/* are listed counter-clockwise. */
/* L80: */
    i_1 = ntt3;
    for (itt3 = 3; itt3 <= i_1; itt3 += 3) {
	ip1 = ipt[itt3 - 2];
	ip2 = ipt[itt3 - 1];
	ip3 = ipt[itt3];
	if ((yd[ip3] - yd[ip1]) * (xd[ip2] - xd[ip1]) - (xd[ip3] - xd[ip1]) *
		(yd[ip2] - yd[ip1]) >= (float)0.) {
	    goto L81;
	}
	ipt[itt3 - 2] = ip2;
	ipt[itt3 - 1] = ip1;
L81:
    ;}
    *nt = nt0;
    *nl = nl0;
    return 0;
/* error exit */
L90:
    err2090_();
    goto L93;
L91:
    err2091b_();
    goto L93;
L92:
    err2092_();
L93:
    err2093_();
    *nt = 0;
    return 0;
/* format statements */
/* L2090: */
/* L2091: */
/* L2092: */
/* L2093: */
} /* idtang_ */

integer idxchg_(real *x, real *y, integer *i1, integer *i2, integer *i3, integer *i4) {
    /* System generated locals */
    integer ret_val;
    real r_1, r_2;
    static real equiv_0[1], equiv_1[1], equiv_2[1], equiv_3[1], equiv_4[1],
	    equiv_5[1];

    /* Local variables */
    static real u1, u2, u3, x1, y1, x2, y2, x3, y3, x4, y4, u4;
    static integer idx;
#define a1sq (equiv_2)
#define b1sq (equiv_3)
#define c1sq (equiv_0)
#define c2sq (equiv_0)
#define a3sq (equiv_1)
#define b2sq (equiv_1)
#define b3sq (equiv_2)
#define a4sq (equiv_3)
#define b4sq (equiv_4)
#define a2sq (equiv_4)
#define c4sq (equiv_5)
#define c3sq (equiv_5)
    static real s1sq, s2sq, s3sq, s4sq;

/* this function determines whether or not the exchange of two */
/* triangles is necessary on the basis of max-min-angle criterion */
/* by c. l. lawson. */
/* the input parameters are */
/*     x,y = arrays containing the coordinates of the data */
/*           points, */
/*     i1,i2,i3,i4 = point numbers of four points p1, p2, */
/*           p3, and p4 that form a quadrilateral with p3 */
/*           and p4 connected diagonally. */
/* this function returns an integer value 1 (one) when an ex- */
/* change is necessary, and 0 (zero) otherwise. */
/* declaration statements */
/* preliminary processing */
    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
/* L10: */
    x1 = x[*i1];
    y1 = y[*i1];
    x2 = x[*i2];
    y2 = y[*i2];
    x3 = x[*i3];
    y3 = y[*i3];
    x4 = x[*i4];
    y4 = y[*i4];
/* calculation */
/* L20: */
    idx = 0;
    u3 = (y2 - y3) * (x1 - x3) - (x2 - x3) * (y1 - y3);
    u4 = (y1 - y4) * (x2 - x4) - (x1 - x4) * (y2 - y4);
    if (u3 * u4 <= (float)0.) {
	goto L30;
    }
    u1 = (y3 - y1) * (x4 - x1) - (x3 - x1) * (y4 - y1);
    u2 = (y4 - y2) * (x3 - x2) - (x4 - x2) * (y3 - y2);
/* Computing 2nd power */
    r_1 = x1 - x3;
/* Computing 2nd power */
    r_2 = y1 - y3;
    *a1sq = r_1 * r_1 + r_2 * r_2;
/* Computing 2nd power */
    r_1 = x4 - x1;
/* Computing 2nd power */
    r_2 = y4 - y1;
    *b1sq = r_1 * r_1 + r_2 * r_2;
/* Computing 2nd power */
    r_1 = x3 - x4;
/* Computing 2nd power */
    r_2 = y3 - y4;
    *c1sq = r_1 * r_1 + r_2 * r_2;
/* Computing 2nd power */
    r_1 = x2 - x4;
/* Computing 2nd power */
    r_2 = y2 - y4;
    *a2sq = r_1 * r_1 + r_2 * r_2;
/* Computing 2nd power */
    r_1 = x3 - x2;
/* Computing 2nd power */
    r_2 = y3 - y2;
    *b2sq = r_1 * r_1 + r_2 * r_2;
/* Computing 2nd power */
    r_1 = x2 - x1;
/* Computing 2nd power */
    r_2 = y2 - y1;
    *c3sq = r_1 * r_1 + r_2 * r_2;
    s1sq = u1 * u1 / (*c1sq * dmax(*a1sq,*b1sq));
    s2sq = u2 * u2 / (*c2sq * dmax(*a2sq,*b2sq));
    s3sq = u3 * u3 / (*c3sq * dmax(*a3sq,*b3sq));
    s4sq = u4 * u4 / (*c4sq * dmax(*a4sq,*b4sq));
    if (dmin(s1sq,s2sq) < dmin(s3sq,s4sq)) {
	idx = 1;
    }
L30:
    ret_val = idx;
    return ret_val;
} /* idxchg_ */

void do_fio() {
	printf("do_fio\n");
}

void e_wsfe() {
	printf("e_wsfe\n");
}

void s_stop() {
	printf("s_stop\n");
}

void s_wsfe() {
	printf("s_wsfe\n");
}

void err2090_() {
	printf("err2090 Improper input parameter values\n");
}

void err2091_() {
	printf("err2091  All colinear data points, or identical points\n");
}

void err2091b_() {
	printf("err2091b All colinear data points, or identical points\n");
}

void err2092_(){
	printf("err2092 error detected in routine idcldp \n");
}

void err2093_(){
	printf("err2093 error detected in routine   idtang \n");
}

void err2094_(){
	printf("err2094 This error is never called.\n");
}

#undef c3sq
#undef c4sq
#undef a2sq
#undef b4sq
#undef a4sq
#undef b3sq
#undef b2sq
#undef a3sq
#undef c2sq
#undef c1sq
#undef b1sq
#undef a1sq
