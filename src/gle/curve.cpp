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
#include "core.h"
#include "var.h"
#include "keyword.h"
#include "run.h"

int ncvec=0;
void cvec_list(GLEPcodeList* pclist, int *pcode);
double cvecx[30],cvecy[30];
double dcvecx[30],dcvecy[30];
void rbezier(double x1, double y1, double x2, double y2, double x3, double y3);

void g_curve(GLEPcodeList* pclist, int *pcode) {
	double dx1,dy1;
	ncvec = 0;
	cvec_list(pclist, pcode);
	dx1 = cvecx[1] - cvecx[0];
	dy1 = cvecy[1] - cvecy[0];
	dcvecx[0] = cvecx[ncvec] - cvecx[ncvec-1];
	dcvecy[0] = cvecy[ncvec] - cvecy[ncvec-1];
	for (int i = 0; i <= ncvec; i++) {
		cvecx[i] = cvecx[i] - dx1;
		cvecy[i] = cvecy[i] - dy1;
	}
	for (int i = 1; i < ncvec; i++) {
		dcvecx[i] = (cvecx[i+1] - cvecx[i-1]) * .25;
		dcvecy[i] = (cvecy[i+1] - cvecy[i-1]) * .25;
	}
	for (int i = 1; i < ncvec-1; i++) {
		rbezier(dcvecx[i],
				dcvecy[i],
				dcvecx[i+1],
				dcvecy[i+1],
				cvecx[i+1],
				cvecy[i+1]);
	}
}

void rbezier(double x1, double y1, double x2, double y2, double x3, double y3) {
	double cx,cy;
	g_get_xy(&cx, &cy);
	g_bezier(x1+cx, y1+cy, x3-x2, y3-y2, x3, y3);
}

void cvec_list(GLEPcodeList* pclist, int *pcode) {
	int cp=0;
	double cx,cy;
	g_get_xy(&cx,&cy);
	ncvec = 0;
	cvecx[0] = cx;
	cvecy[0] = cy;
	GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
	while ( *(pcode + cp++)==111) {
		if (ncvec>27) {gprint("Too many param in curve\n"); return; }
		double x1 = evalDouble(stk.get(), pclist, pcode, &cp);
		double y1 = evalDouble(stk.get(), pclist, pcode, &cp);
		cvecx[++ncvec] = x1;
		cvecx[ncvec] = cvecx[ncvec] + cvecx[ncvec-1];
		cvecy[ncvec] = y1;
		cvecy[ncvec] = cvecy[ncvec] + cvecy[ncvec-1];
	}
}
