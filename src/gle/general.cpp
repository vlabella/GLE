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

#include "all.h"
#include "gprint.h"

#ifdef VAXC
double myatan2(double y, double x);
#endif

void showpcode(int *p)
{
	union {int l; short s[2];} bth;
	int i;

	gprint("GP> ");
	for (i=0;i<12;i++) {
		bth.l = *(p++);
		gprint("%x %x  ",bth.s[0],bth.s[1]);
	}
	gprint("\n");
}
void polar_xy(double r, double angle, double *dx, double *dy)
{
	*dx = r*cos(angle*GLE_PI/180);
	*dy = r*sin(angle*GLE_PI/180);
}
void polar_xy(double rx, double ry, double angle, double *dx, double *dy)
{
	*dx = rx*cos(angle*GLE_PI/180);
	*dy = ry*sin(angle*GLE_PI/180);
}

void xy_polar(double dx,double dy,double *radius,double *angle)
{
	if (dx==0 && dy==0) {
		*angle = 0;
		gprint("Cannot work out angle of zero length vector\n");
		return;
	}
	if (dx==0) {
		*angle = 90;
		if (dy<0) *angle = -90;
	} else {
		*angle = myatan2(dy,dx)*180/GLE_PI;
	}
	*radius = sqrt(dx*dx + dy*dy);
}

void fpolar_xy(float r, float angle, float *dx, float *dy)
{
	*dx = r*cos(angle*GLE_PI/180);
	*dy = r*sin(angle*GLE_PI/180);
}

void fxy_polar(float dx,float dy,float *radius,float *angle)
{
	if (dx==0 && dy==0) {
		gprint("Cannot work out angle of zero length vector\n");
		return;
	}
	if (dx==0) {
		*angle = 90;
		if (dy<0) *angle = -90;
	} else {
		*angle = myatan2(dy,dx)*180/GLE_PI;
	}
	*radius = sqrt(dx*dx+dy*dy);
}

void ncpy(char *d, const char *s, int n)
{
	strncpy(d,s,n);
	*(d+n) = 0;
}

void ncat(char *d, char *s, int n)
{
	int i;
	i = strlen(d);
	strncat(d,s,n);
	*(d+i+n) = 0;
}

#ifdef VAXC
double myatan2(double y, double x)
{
	static double one,test,xx,yy,zero,at2;
	zero = 0;
	one = 1;
	xx = fabs(x);
	yy = fabs(y);
	if (x==0) {
		at2 = GLE_PI/2;
	} else {
		if (yy<=xx) {
			at2 = fabs(atan(yy/xx));
		} else {
			test = one + (xx/yy);
			if (test!=one) {
				at2 = fabs(atan(yy/xx));
			} else {
				at2 = GLE_PI/2;
			}
		}
		if (x<zero) at2 = GLE_PI - at2;
	}
	if (y<0) at2 = -at2;
	return at2;
}
#else
double myatan2(double y, double x)
{
	return atan2(y,x);
}
#endif


