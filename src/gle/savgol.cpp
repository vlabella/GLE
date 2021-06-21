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

#include <string>
#include <stdlib.h>
#include <string.h>

using namespace std;

/*
Savitski Golay Smoothing routine
*/
int do_svg_smooth(double* xold, double* xnew, int size, int poly, int points, int num)
{
/*
xold....the old data
xnew....the new data
size....the size of the array
poly....the type of polynomial to fit to 3,4,5,6,...
points...the number of points for smoothing 3,5,7,9,.....
num.....the number of times to do the smoothing
*/

	/* test for bad input */
	if(
	xold == NULL ||
	xnew == NULL ||
	size == 0 ||
	size <= points ||
	num  <= 0
	)
	{
		return 0;
	}

 /* for now this is a quadratic or cubic and 7 point smoothing only */
	int i;
	xnew = (double*)calloc(size,sizeof(double));

	for (i =0 ;i <= size ;i++ )
	{
		if(i == 0 || i == 1 || i == size-2 ||  i == size-1)
		{
			/* can't do it*/
			xnew[i]=xold[i];
		}
		else if(i == 2 || i == size-3)
		{
			/* do five point */
			xnew[i] = (-3*xold[i-2]+12*xold[i-1]+17*xold[i]+12*xold[i+1]-3*xold[i+2])/35;
		}
		else if (i == 3 || i == size - 4)
		{
			/* do seven point */
			xnew[i] = (-2*xold[i-3]+3*xold[i-2]+6*xold[i-1]+7*xold[i]+6*xold[i+1]+3*xold[i+2]-2*xold[i+3])/21;
		}
		else if (i >= 4 && i <= size - 5)
		{
			/* do nine point */
			xnew[i] = (-21*xold[i-4]+14*xold[i-3]+39*xold[i-2]+54*xold[i-1]+59*xold[i]+54*xold[i+1]+39*xold[i+2]+14*xold[i+3]-21*xold[i+4])/231;
		}

	}
	memcpy(xold,xnew,size*sizeof(double));
	free(xnew);
	return 0;
}
/*
#include <math.h>
#define NRANSI
#include "nrutil.h"
#define TINY 1.0e-20;

void ludcmp(float **a, int n, int *indx, float *d)
{
        int i,imax,j,k;
        float big,dum,sum,temp;
        float *vv;

        vv=vector(1,n);
        *d=1.0;
        for (i=1;i<=n;i++) {
                big=0.0;
                for (j=1;j<=n;j++)
                        if ((temp=fabs(a[i][j])) > big) big=temp;
                if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
                vv[i]=1.0/big;
        }
        for (j=1;j<=n;j++) {
                for (i=1;i<j;i++) {
                        sum=a[i][j];
                        for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
                        a[i][j]=sum;
                }
                big=0.0;
                for (i=j;i<=n;i++) {
                        sum=a[i][j];
                        for (k=1;k<j;k++)
                                sum -= a[i][k]*a[k][j];
                        a[i][j]=sum;
                        if ( (dum=vv[i]*fabs(sum)) >= big) {
                                big=dum;
                                imax=i;
                        }
                }
                if (j != imax) {
                        for (k=1;k<=n;k++) {
                                dum=a[imax][k];
                                a[imax][k]=a[j][k];
                                a[j][k]=dum;
                        }
                        *d = -(*d);
                        vv[imax]=vv[j];
                }
                indx[j]=imax;
                if (a[j][j] == 0.0) a[j][j]=TINY;
                if (j != n) {
                        dum=1.0/(a[j][j]);
                        for (i=j+1;i<=n;i++) a[i][j] *= dum;
                }
        }
        free_vector(vv,1,n);
}
#undef TINY
#undef NRANSI
*/
