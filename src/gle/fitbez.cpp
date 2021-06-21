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

#define GRAPHDEF extern
#include "all.h"
#include "gle-interface/gle-interface.h"
#include "mem_limits.h"
#include "cutils.h"
#include "graph.h"

int glefitcf_(int *mode,float *x, float *y, int *l, int *m, float *u, float *v, int *n);

void fitbez(GLEDataPairs* data, bool multi) {
	if (data->size() > 200 || data->size() <= 2) {
		return;
	}
	int np = data->size();
	vector<float> xin(np);
	vector<float> yin(np);
	for (int i = 0; i < np; i++) {
		xin[i] = (float)data->getX(i);
		yin[i] = (float)data->getY(i);
	}
	int mode = (multi ? 2 : 1);
	int nsub = 300/(np-1);
	if (nsub <= 1) nsub = 2;
	int np_out = (np-1)*nsub+1;
	vector<float> xout(np_out);
	vector<float> yout(np_out);
	glefitcf_(&mode, &xin[0], &yin[0], &np, &nsub, &xout[0], &yout[0], &np_out);
	data->resize(np_out);
	for (int i = 0; i < np_out; i++) {
		data->set(i, xout[i], yout[i], 0);
	}
}

void transform_log(double* v, int np) {
	for (int i = 0; i < np; i++) {
		v[i] = log10(v[i]);
	}
}

void untransform_log(double* v, int np) {
	for (int i = 0; i < np; i++) {
		v[i] = pow(10.0, v[i]);
	}
}
