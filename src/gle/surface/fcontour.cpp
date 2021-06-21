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

/* contour.f -- translated by f2c (version of 26 February 1990  17:38:00).
   You must link the resulting object file with the libraries:
	-lF77 -lI77 -lm -lc   (in that order)
*/
#ifndef __TURBOC__
#define huge
#endif

#include "f2c.h"

/* Common Block Declarations */

typedef struct {
    real xcur, ycur, xl, yl, cval[10];
    integer clab[10], nch;
} gctcom_;

#define gctcom_1 gctcom_

/* Table of constant values */

static integer c__2 = 2;
static integer c__6 = 6;
static real c_b43 = 0.f;
static integer c__3 = 3;
static real c_b104 = .125f;

/* Subroutine */ int fill0_(integer *bitmap, integer *n)
{
    /* Initialized data */

    static integer nbpw = 31;

    /* System generated locals */
    integer i_1;

    /* Builtin functions */
    integer f_pow_ii(integer *, integer *);

    /* Local variables */
    static integer nblw, loop, i;


/*     fill the first n bits of bitmap with zeroes. */


    /* Parameter adjustments */
    --bitmap;

    /* Function Body */
/*     nbpw is the minimum number of significant bits per word used */
/*     by integer arithmetic.  this is usually one less than the */
/*     actual number of bits per word, but an important exception is */
/*     the cdc-6000 series of machines, where nbpw should be 48. */

    loop = *n / nbpw;
    nblw = *n % nbpw;
    if (loop == 0) {
	goto L20;
    }
    i_1 = loop;
    for (i = 1; i <= i_1; ++i) {
	bitmap[i] = 0;
/* L10: */
    }
L20:
    if (nblw != 0) {
	i_1 = nbpw - nblw;
	bitmap[loop + 1] %= f_pow_ii(&c__2, &i_1);
    }
    return 0;
} /* fill0_ */

/* Subroutine */ int mark1_(integer *bitmap, integer *n)
{
    /* Initialized data */

    static integer nbpw = 31;

    /* System generated locals */
    integer i_1;

    /* Builtin functions */
    integer f_pow_ii(integer *, integer *);

    /* Local variables */
    static integer nbit, i, nword;


/*     put a one in the nth bit of bitmap. */


    /* Parameter adjustments */
    --bitmap;

    /* Function Body */
/*     nbpw is the minimum number of significant bits per word used */
/*     by integer arithmetic.  this is usually one less than the */
/*     actual number of bits per word, but an important exception is */
/*     the cdc-6000 series of machines, where nbpw should be 48. */

    nword = (*n - 1) / nbpw;
    nbit = (*n - 1) % nbpw;
    i_1 = nbpw - nbit - 1;
    i = f_pow_ii(&c__2, &i_1);
    bitmap[nword + 1] += i * (1 - bitmap[nword + 1] / i % 2);
    return 0;
} /* mark1_ */

integer iget_(huge integer *bitmap, integer *n)
{
    /* Initialized data */

    static integer nbpw = 31;

    /* System generated locals */
    integer ret_val, i_1;

    /* Builtin functions */
    integer f_pow_ii(integer *, integer *);

    /* Local variables */
    static integer nbit, nword;


/*     iget=0 if the nth bit of bitmap is zero, else iget is one. */


    /* Parameter adjustments */
    --bitmap;

    /* Function Body */
/*     nbpw is the minimum number of significant bits per word used */
/*     by integer arithmetic.  this is usually one less than the */
/*     actual number of bits per word, but an important exception is */
/*     the cdc-6000 series of machines, where nbpw should be 48. */

    nword = (*n - 1) / nbpw;
    nbit = (*n - 1) % nbpw;
    i_1 = nbpw - nbit - 1;
    ret_val = bitmap[nword + 1] / f_pow_ii(&c__2, &i_1) % 2;
    return ret_val;
} /* iget_ */

/* Subroutine */ int gcontr_(real *z, integer *nrz, integer *nx, integer *ny,
	real *cv, integer *ncv, real *zmax, huge integer *bitmap, S_fp draw)
{
    /* Initialized data */

    static integer l1[4] = { 0,0,-1,-1 };
    static integer i1[2] = { 1,0 };
    static integer i2[2] = { 1,-1 };
    static integer i3[6] = { 1,0,0,1,1,0 };

    /* System generated locals */
    integer z_dim1, z_offset, i_1, i_2, i_3;
    static integer equiv_3[4], equiv_5[2];
    static real equiv_7[2];

    /* Builtin functions */
    integer f_i_sign(integer *, integer *);

    /* Local variables */
    static real cval;
    static integer idir;
    extern integer iget_(integer *, integer *);
    static real dmax_;
#define imin (equiv_3 + 2)
#define imax (equiv_3)
#define jmax (equiv_3 + 1)
#define jmin (equiv_3 + 3)
    static integer icur, jcur, jump;
    static real xint[4];
    extern /* Subroutine */ int fill0_(integer *, integer *), mark1_(integer *
	    , integer *);
#define i (equiv_5)
#define j (equiv_5 + 1)
    static integer k, l, iedge, iflag;
#define x (equiv_7)
#define y (equiv_7 + 1)
    static integer ibkey;
#define l2 (equiv_3)
    static real z1, z2;
    static integer ii;
#define ij (equiv_5)
    static integer jj, ni, ks, ix;
#define xy (equiv_7)
    static real zz;
    static integer nxidir, icv;

/*     this subroutine draws a contour through equal values of an array.
*/

/*     *****     formal arguments     ***********************************
*/

/*     z is the array for which contours are to be drawn.  the elements */

/*     of z are assumed to lie upon the nodes of a topologically */
/*     rectangular coordinate system - e.g. cartesian, polar (except */
/*     the origin), etc. */

/*     nrz is the number of rows declared for z in the calling program. */


/*     nx is the limit for the first subscript of z. */

/*     ny is the limit for the second subscript of z. */

/*     cv are the values of the contours to be drawn. */

/*     ncv is the number of contour values in cv. */

/*     zmax is the maximum value of z for consideration.  a value of */
/*     z(i,j) greater than zmax is a signal that that point and the */
/*     grid line segments radiating from that point to it's neighbors */
/*     are to be excluded from contouring. */

/*     bitmap is a work area large enough to hold 2*nx*ny*ncv bits.  it */

/*     is accessed by low-level routines, which are described below. */
/*     let j be the number of useful bits in each word of bitmap, */
/*     as determined by the user machine and implementation of */
/*     the bitmap manipulation subprograms described below.  then */
/*     the number of words required for the bitmap is the floor of */
/*         (2*nx*ny*ncv+j-1)/j. */

/*     draw is a user-provided subroutine used to draw contours. */
/*     the calling sequence for draw is: */

/*         call draw (x,y,iflag) */
/*         let nx = integer part of x, fx = fractional part of x. */
/*         then x should be interpreted such that increases in nx */
/*         correspond to increases in the first subscript of z, and */
/*         fx is the fractional distance from the abscissa corresponding
*/
/*         to nx to the abscissa corresponding to nx+1, */
/*         and y should be interpreted similarly for the second */
/*         subscript of z. */
/*         the low-order digit of iflag will have one of the values: */
/*             1 - continue a contour, */
/*             2 - start a contour at a boundary, */
/*             3 - start a contour not at a boundary, */
/*             4 - finish a contour at a boundary, */
/*             5 - finish a closed contour (not at a boundary). */
/*                 note that requests 1, 4 and 5 are for pen-down */
/*                 moves, and that requests 2 and 3 are for pen-up */
/*                 moves. */
/*             6 - set x and y to the approximate 'pen' position, using */

/*                 the notation discussed above.  this call may be */
/*                 ignored, the result being that the 'pen' position */
/*                 is taken to correspond to z(1,1). */
/*         iflag/10 is the contour number. */

/*     *****     external subprograms     *******************************
*/

/*     draw is the user-supplied line drawing subprogram described above.
*/
/*     draw may be sensitive to the host computer and to the plot device.
*/
/*     fill0 is used to fill a bitmap with zeroes.  call fill0 (bitmap,n)
*/
/*     fills the first n bits of bitmap with zeroes. */
/*     mark1 is used to place a 1 in a specific bit of the bitmap. */
/*     call mark1 (bitmap,n) puts a 1 in the nth bit of the bitmap. */
/*     iget is used to determine the setting of a particular bit in the */

/*     bitmap.  i=iget(bitmap,n) sets i to zero if the nth bit of the */
/*     bitmap is zero, and sets i to one if the nth bit is one. */
/*     fill0, mark1 and iget are machine sensitive. */

/*     ******************************************************************
*/


/*     l1 and l2 contain limits used during the spiral search for the */
/*     beginning of a contour. */
/*     ij stores subcripts used during the spiral search. */


/*     i1, i2 and i3 are used for subscript computations during the */
/*     examination of lines from z(i,j) to it's neighbors. */


/*     xint is used to mark intersections of the contour under */
/*     consideration with the edges of the cell being examined. */


/*     xy is used to compute coordinates for the draw subroutine. */


    /* Parameter adjustments */
    --bitmap;
    --cv;
    z_dim1 = *nrz;
    z_offset = z_dim1 + 1;
    z -= z_offset;

    /* Function Body */

    l1[0] = *nx;
    l1[1] = *ny;
    dmax_ = *zmax;

/*     set the current pen position.  the default position corresponds */
/*     to z(1,1). */

    *x = 1.f;
    *y = 1.f;
    (*draw)(x, y, &c__6);
/* Computing MAX */
/* Computing MIN */
    i_3 = (integer) (*x);
    i_1 = 1, i_2 = min(i_3,*nx);
    icur = max(i_1,i_2);
/* Computing MAX */
/* Computing MIN */
    i_3 = (integer) (*y);
    i_1 = 1, i_2 = min(i_3,*ny);
    jcur = max(i_1,i_2);

/*     clear the bitmap */

    i_1 = (*nx << 1) * *ny * *ncv;
    fill0_(&bitmap[1], &i_1);

/*     search along a rectangular spiral path for a line segment having */

/*     the following properties: */
/*          1.  the end points are not excluded, */
/*          2.  no mark has been recorded for the segment, */
/*          3.  the values of z at the ends of the segment are such that
*/
/*              one z is less than the current contour value, and the */
/*              other is greater than or equal to the current contour */
/*              value. */

/*     search all boundaries first, then search interior line segments. */

/*     note that the interior line segments near excluded points may be */

/*     boundaries. */

    ibkey = 0;
L10:
    *i = icur;
    *j = jcur;
L20:
    *imax = *i;
    *imin = -(*i);
    *jmax = *j;
    *jmin = -(*j);
    idir = 0;
/*     direction zero is +i, 1 is +j, 2 is -i, 3 is -j. */
L30:
    nxidir = idir + 1;
    k = nxidir;
    if (nxidir > 3) {
	nxidir = 0;
    }
L40:
    *i = abs(*i);
    *j = abs(*j);
    if (z[*i + *j * z_dim1] > dmax_) {
	goto L140;
    }
    l = 1;
/*     l=1 means horizontal line, l=2 means vertical line. */
L50:
    if (ij[l - 1] >= l1[l - 1]) {
	goto L130;
    }
    ii = *i + i1[l - 1];
    jj = *j + i1[3 - l - 1];
    if (z[ii + jj * z_dim1] > dmax_) {
	goto L130;
    }
    jump = 0;
/*     the next 15 statements (or so) detect boundaries. */
L60:
    ix = 1;
    if (ij[3 - l - 1] == 1) {
	goto L80;
    }
    ii = *i - i1[3 - l - 1];
    jj = *j - i1[l - 1];
    if (z[ii + jj * z_dim1] > dmax_) {
	goto L70;
    }
    ii = *i + i2[l - 1];
    jj = *j + i2[3 - l - 1];
    if (z[ii + jj * z_dim1] < dmax_) {
	ix = 0;
    }
L70:
    if (ij[3 - l - 1] >= l1[3 - l - 1]) {
	goto L90;
    }
L80:
    ii = *i + i1[3 - l - 1];
    jj = *j + i1[l - 1];
    if (z[ii + jj * z_dim1] > dmax_) {
	goto L90;
    }
    if (z[*i + 1 + (*j + 1) * z_dim1] < dmax_) {
	switch (jump) {
	    case 0: goto L100;
	    case 1: goto L280;
	}
    }
L90:
    ix += 2;
    switch (jump) {
	case 0: goto L100;
	case 1: goto L280;
    }
L100:
    if (ix == 3) {
	goto L130;
    }
    if (ix + ibkey == 0) {
	goto L130;
    }
/*     now determine whether the line segment is crossed by the contour.
*/
    ii = *i + i1[l - 1];
    jj = *j + i1[3 - l - 1];
    z1 = z[*i + *j * z_dim1];
    z2 = z[ii + jj * z_dim1];
    i_1 = *ncv;
    for (icv = 1; icv <= i_1; ++icv) {
	i_2 = ((*nx * (*ny * (icv - 1) + *j - 1) + *i - 1) << 1) + l;
	if (iget_(&bitmap[1], &i_2) != 0) {
	    goto L120;
	}
	if (cv[icv] <= dmin(z1,z2)) {
	    goto L110;
	}
	if (cv[icv] <= dmax(z1,z2)) {
	    goto L190;
	}
L110:
	i_2 = ((*nx * (*ny * (icv - 1) + *j - 1) + *i - 1) << 1) + l;
	mark1_(&bitmap[1], &i_2);
L120:
    ;}
L130:
    ++l;
    if (l <= 2) {
	goto L50;
    }
L140:
    l = idir % 2 + 1;
    ij[l - 1] = f_i_sign(&ij[l - 1], &l1[k - 1]);

/*     lines from z(i,j) to z(i+1,j) and z(i,j+1) are not satisfactory. */

/*     continue the spiral. */

L150:
    if (ij[l - 1] >= l1[k - 1]) {
	goto L170;
    }
    ++ij[l - 1];
    if (ij[l - 1] > l2[k - 1]) {
	goto L160;
    }
    goto L40;
L160:
    l2[k - 1] = ij[l - 1];
    idir = nxidir;
    goto L30;
L170:
    if (idir == nxidir) {
	goto L180;
    }
    ++nxidir;
    ij[l - 1] = l1[k - 1];
    k = nxidir;
    l = 3 - l;
    ij[l - 1] = l2[k - 1];
    if (nxidir > 3) {
	nxidir = 0;
    }
    goto L150;
L180:
    if (ibkey != 0) {
	return 0;
    }
    ibkey = 1;
    goto L10;

/*     an acceptable line segment has been found. */
/*     follow the contour until it either hits a boundary or closes. */

L190:
    iedge = l;
    cval = cv[icv];
    if (ix != 1) {
	iedge += 2;
    }
    iflag = ibkey + 2;
    xint[iedge - 1] = (cval - z1) / (z2 - z1);
L200:
    xy[l - 1] = (real) ij[l - 1] + xint[iedge - 1];
    xy[3 - l - 1] = (real) ij[3 - l - 1];
    i_1 = ((*nx * (*ny * (icv - 1) + *j - 1) + *i - 1) << 1) + l;
    mark1_(&bitmap[1], &i_1);
    i_1 = iflag + icv * 10;
    (*draw)(x, y, &i_1);
    if (iflag < 4) {
	goto L210;
    }
    icur = *i;
    jcur = *j;
    goto L20;

/*     continue a contour.  the edges are numbered clockwise with */
/*     the bottom edge being edge number one. */

L210:
    ni = 1;
    if (iedge < 3) {
	goto L220;
    }
    *i -= i3[iedge - 1];
    *j -= i3[iedge + 1];
L220:
    for (k = 1; k <= 4; ++k) {
	if (k == iedge) {
	    goto L250;
	}
	ii = *i + i3[k - 1];
	jj = *j + i3[k];
	z1 = z[ii + jj * z_dim1];
	ii = *i + i3[k];
	jj = *j + i3[k + 1];
	z2 = z[ii + jj * z_dim1];
	if (cval <= dmin(z1,z2)) {
	    goto L250;
	}
	if (cval > dmax(z1,z2)) {
	    goto L250;
	}
	if (k == 1) {
	    goto L230;
	}
	if (k != 4) {
	    goto L240;
	}
L230:
	zz = z1;
	z1 = z2;
	z2 = zz;
L240:
	xint[k - 1] = (cval - z1) / (z2 - z1);
	++ni;
	ks = k;
L250:
    ;}
    if (ni == 2) {
	goto L260;
    }

/*     the contour crosses all four edges of the cell being examined. */
/*     choose the lines top-to-left and bottom-to-right if the */
/*     interpolation point on the top edge is less than the interpolation
*/
/*     point on the bottom edge.  otherwise, choose the other pair.  this
*/
/*     method produces the same results if the axes are reversed.  the */
/*     contour may close at any edge, but must not cross itself inside */
/*     any cell. */

    ks = 5 - iedge;
    if (xint[2] < xint[0]) {
	goto L260;
    }
    ks = 3 - iedge;
    if (ks <= 0) {
	ks += 4;
    }

/*     determine whether the contour will close or run into a boundary */
/*     at edge ks of the current cell. */

L260:
    l = ks;
    iflag = 1;
    jump = 1;
    if (ks < 3) {
	goto L270;
    }
    *i += i3[ks - 1];
    *j += i3[ks + 1];
    l = ks - 2;
L270:
    i_1 = ((*nx * (*ny * (icv - 1) + *j - 1) + *i - 1) << 1) + l;
    if (iget_(&bitmap[1], &i_1) == 0) {
	goto L60;
    }
    iflag = 5;
    goto L290;
L280:
    if (ix != 0) {
	iflag = 4;
    }
L290:
    iedge = ks + 2;
    if (iedge > 4) {
	iedge += -4;
    }
    xint[iedge - 1] = xint[ks - 1];
    goto L200;

} /* gcontr_ */

#undef xy
#undef ij
#undef l2
#undef y
#undef x
#undef j
#undef i
#undef jmin
#undef jmax
#undef imax
#undef imin


/*      dimension z(51,51), c(10), work(1680) */
/*     dimension of work is large enough to contain */
/*     2*(dimension of c)*(total dimension of z) useful bits.  see the */
/*     bitmap routines accessed by gcontr. */
/*      real mu */
/*      external draw */
/*      common /cur/ xcur, ycur */
/*      data c(1), c(2), c(3), c(4), c(5) /3.05,3.2,3.5,3.50135,3.6/ */
/*      data c(6), c(7), c(8), c(9), c(10) /3.766413,4.0,4.130149,5.0, */
/*     *  10.0/ */
/*      data nx /51/, ny /51/, nf /10/ */
/*      data xmin /-2.0/, xmax /2.0/, ymin /-2.0/, ymax /2.0/, mu /0.3/ */
/*      dx = (xmax-xmin)/float(nx-1) */
/*      dy = (ymax-ymin)/float(ny-1) */
/*      xcur = 1.0 */
/*      ycur = 1.0 */
/*      if (mod(nx,2).ne.0) ycur = float(ny) */
/*      if (mod(ny,2).ne.0) xcur = float(nx) */
/*      x = xmin - dx */
/*      do 20 i=1,nx */
/*        y = ymin - dy */
/*        x = x + dx */
/*        do 10 j=1,ny */
/*          y = y + dy */
/*          z(i,j) = (1.0-mu)*(2.0/sqrt((x-mu)**2+y**2)+(x-mu)**2+y**2) */
/*     *      + mu*(2.0/sqrt((x+1.0-mu)**2+y**2)+(x+1.0-mu)**2+y**2) */
/*   10   continue */
/*   20 continue */
/*      call gcontr(z, 51, nx, ny, c, nf, 1.e6, work, draw) */
/*      stop */
/*      end */

/*      subroutine draw(x, y, iflag) */

/*     do output for gcontr. */

/*      integer print */
/*      common /cur/ xcur, ycur */
/*      data print /6/ */
/*     print is the system printer fortran i/o unit number. */
/*      icont = iflag/10 */
/*      jump = mod(iflag,10) */
/*      go to (10, 20, 30, 40, 50, 60), jump */
/*   10 write (print,99999) icont, x, y */
/*      go to 70 */
/*   20 write (print,99998) icont, x, y */
/*      go to 70 */
/*   30 write (print,99997) icont, x, y */
/*      go to 70 */
/*   40 write (print,99996) icont, x, y */
/*      go to 70 */
/*   50 write (print,99995) icont, x, y */
/*      go to 70 */
/*   60 write (print,99994) */
/*      x = xcur */
/*      y = ycur */
/*   70 return */
/* 99999 format (17h continue contour, i3, 3h to, 1p2e14.7) */
/* 99998 format (14h start contour, i3, 19h on the boundary at, 1p2e14.7) */
/* 99997 format (14h start contour, i3, 19h in the interior at, 1p2e14.7) */
/* 99996 format (15h finish contour, i3, 19h on the boundary at, 1p2e14.7) */
/* 99995 format (15h finish contour, i3, 19h in the interior at, 1p2e14.7) */
/* 99994 format (33h request for current pen position) */
/*      end */
/* Subroutine */ int cgrid_(integer *nopt, integer *nx, real *sx, real *xs,
	real *xf, integer *ny, real *sy, real *ys, real *yf)
{
    /* System generated locals */
    integer i_1;
    real r_1, r_2;

    /* Builtin functions */
    double f_r_sign(real *, real *);

    /* Local variables */
    static real xinc, yinc, xmin, ymin, xmax, ymax;
    extern /* Subroutine */ void plot_(real *, real *, integer *);
    static integer i, j, n;
    static real xlgth, ylgth, x1, x2, y2, y1, xx;


/* subroutine which draws a frame around the plot and draws */
/* either tick marks or grid lines. */

/* parameters:  nopt -- =0, draw ticks only */
/*                      =1, draw grid lines */
/*                      =2, draw grid lines to edge of frame. */
/*              nx -- number of intervals in x direction */
/*              sx -- spacing in inches between tick marks or grid lines
*/
/*                    along the x axis */
/*              xs -- location of first tick or grid line on x axis */
/*              xf -- location of right edge of frame */
/*              ny -- number of intervals in y direction */
/*              sy -- spacing in inches between tick marks or grid lines
*/
/*                    along the y axis */
/*              ys -- location of first tick or grid line on y axis */
/*              yf -- location of top edge of frame */
/* assumptions: nx, sx, ny, sy all positive. */
/*              the lower left-hand corner of the frame is drawn at (0,0)
*/
/*              if xs<0, use 0; if ys<0, use 0 */
/*              if xf<=0, use nx*sx; if yf<=0, use ny*sy. */

    xinc = *sx;
    yinc = *sy;
    xlgth = (real) (*nx) * *sx;
    ylgth = (real) (*ny) * *sy;
    xmin = dmax(*xs,0.f);
    ymin = dmax(*ys,0.f);
/* Computing MAX */
    r_1 = *xf, r_2 = xlgth + xmin;
    xmax = dmax(r_1,r_2);
/* Computing MAX */
    r_1 = *yf, r_2 = ylgth + ymin;
    ymax = dmax(r_1,r_2);

/*     draw frame. */

    plot_(&c_b43, &c_b43, &c__3);
    plot_(&xmax, &c_b43, &c__2);
    plot_(&xmax, &ymax, &c__2);
    plot_(&c_b43, &ymax, &c__2);
    plot_(&c_b43, &c_b43, &c__2);
    if (*nopt != 0) {
	goto L130;
    }

/*     draw tick marks. */

    for (j = 1; j <= 4; ++j) {
	switch (j) {
	    case 1:  goto L10;
	    case 2:  goto L50;
	    case 3:  goto L20;
	    case 4:  goto L40;
	}
L10:
	x2 = 0.f;
	if (xmin != 0.f) {
	    x2 = xmin - *sx;
	}
	y2 = 0.f;
	goto L30;
L20:
	xinc = -(doublereal)(*sx);
	x2 = xmin + xlgth + *sx;
	if (xmax == xmin + xlgth) {
	    x2 = xmax;
	}
	y2 = ymax;
L30:
	y1 = y2;
	y2 += f_r_sign(&c_b104, &xinc);
	n = *nx;
	if ((r_1 = xmax - xmin - xlgth, dabs(r_1)) + dabs(xmin) != 0.f) {
	    goto L70;
	} else {
	    goto L80;
	}
L40:
	yinc = -(doublereal)(*sy);
	y2 = ymin + ylgth + *sy;
	if (ymax == ymin + ylgth) {
	    y2 = ymax;
	}
	x2 = 0.f;
	goto L60;
L50:
	y2 = 0.f;
	if (ymin != 0.f) {
	    y2 = ymin - *sy;
	}
	x2 = xmax;
L60:
	x1 = x2;
	n = *ny;
	x2 -= f_r_sign(&c_b104, &yinc);
	if ((r_1 = ymax - ymin - ylgth, dabs(r_1)) + dabs(ymin) != 0.f) {
	    goto L70;
	} else {
	    goto L80;
	}
L70:
	++n;
L80:
	i_1 = n;
	for (i = 1; i <= i_1; ++i) {
	    if (j % 2 == 0) {
		goto L90;
	    }
	    x2 += xinc;
	    x1 = x2;
	    goto L100;
L90:
	    y2 += yinc;
	    y1 = y2;
L100:
	    plot_(&x1, &y1, &c__3);
	    plot_(&x2, &y2, &c__2);
/* L110: */
	}
/* L120: */
    }
    goto L240;

/*     draw grid lines */

L130:
    x1 = xmin;
    x2 = xmin + xlgth;
    if (*nopt != 2) {
	goto L140;
    }
    x1 = 0.f;
    x2 = xmax;
L140:
    y1 = ymin - *sy;
    n = *ny + 1;
    if (ymax == ymin + ylgth) {
	--n;
    }
    if (ymin != 0.f) {
	goto L150;
    }
    y1 = 0.f;
    --n;
L150:
    if (n <= 0) {
	goto L170;
    }
    j = 1;
    i_1 = n;
    for (i = 1; i <= i_1; ++i) {
	j = -j;
	y1 += *sy;
	plot_(&x1, &y1, &c__3);
	plot_(&x2, &y1, &c__2);
	xx = x1;
	x1 = x2;
	x2 = xx;
/* L160: */
    }
L170:
    y1 = ymin + ylgth;
    y2 = ymin;
    if (*nopt != 2) {
	goto L180;
    }
    y1 = ymax;
    y2 = 0.f;
L180:
    n = *nx + 1;
    if (j < 0) {
	goto L200;
    }
    x1 = xmin - *sx;
    if (xmax == xmin + xlgth) {
	--n;
    }
    if (xmin != 0.f) {
	goto L190;
    }
    x1 = 0.f;
    --n;
L190:
    if (n <= 0) {
	goto L240;
    }
    xinc = *sx;
    goto L220;
L200:
    x1 = xmin + xlgth + *sx;
    if (xmin == 0.f) {
	--n;
    }
    if (xmax != xlgth + xmin) {
	goto L210;
    }
    --n;
    x1 = xmax;
L210:
    xinc = -(doublereal)(*sx);
L220:
    i_1 = n;
    for (i = 1; i <= i_1; ++i) {
	x1 += xinc;
	plot_(&x1, &y1, &c__3);
	plot_(&x1, &y2, &c__2);
	xx = y1;
	y1 = y2;
	y2 = xx;
/* L230: */
    }
L240:
    return 0;

} /* cgrid_ */

