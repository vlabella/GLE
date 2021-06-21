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

#define pi 3.14159265
#define farcalloc calloc

#include "all.h"

void polar_xy(double r, double angle, double *dx, double *dy) {
	*dx = r*cos(angle*3.14159265/180);
	*dy = r*sin(angle*3.14159265/180);
}

void xy_polar(double dx,double dy,double *radius,double *angle) {
	if (dx==0 && dy==0) {
		printf("Cannot work out angle of zero length vector\n");
		return;
	}
	if (dx==0) {
		*angle = 90;
		if (dy<0) *angle = -90;
	} else {
		*angle = atan2(dy,dx)*180/pi;
	}
	*radius = sqrt(pow(dx,2)+pow(dy,2));
}

void ncpy(char *d, const char *s, int n) {
	strncpy(d,s,n);
	*(d+n) = 0;
}

void ncat(char *d, char *s, int n) {
	int i;
	i = strlen(d);
	strncat(d,s,n);
	*(d+i+n) = 0;
}

char *sdup(const char *s) {
	char *v;
	v = (char*)malloc(strlen(s)+1);
	strcpy(v,s);
	return v;
}

/*
var_getstr(int varnum,char *s){}
sub_clear(void){}
int sub_def(char *s){}
sub_call(int idx,double *pval,char **pstr,int *npm){}
sub_find(char *s,int *idx,int *zret, int *np, int **plist){*idx = 0;}
sub_get_startend(int idx, int *ss, int *ee){}
sub_param(int idx,char *s){}
sub_set_return(double d){}
sub_set_startend(int idx, int ss, int ee){}
var_add(char *name,int *idx,int *type){}
var_find(char *name,int *idx,int *type){*idx = 0;}
var_findadd(char *name,int *idx,int *type){}
var_get(int jj, double *v){}
var_nlocal(int *l){}
var_set(int jj, double v){}
var_setstr(int jj, char *s){}
*/

void *myallocz(int32 size) {
	static void *p;
	p = farcalloc(1,size);
	return p;
}

void mystrcpy(char **d, const char *s) {
	if (*d!=0) free(*d);
	*d = 0;
	*d = (char*)malloc(strlen(s)+1);
	if (d==NULL) gle_abort("Memory gone\n");
	strcpy(*d,s);
}

void do_pcode() {
}

int gpcode;
int gplen;
