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

int var_alloc_local(void);
int var_free_local(void);
extern int32 *(*gpcode)[];   /* gpcode is a pointer to an array of poiter to int32 */
extern int32 (*gplen)[];   /* gpcode is a pointer to an array of int32 */
extern int ngpcode;
extern int gle_debug;
#define dbg if ((gle_debug & 128)>0)

struct sub_st {char name[40];int typ; int np
		; int ptyp[20]; char *pname[20]; int start; int end ; }  ;
struct sub_st *sb[100];
int nsb;
double return_value=0;
char return_string[80];
int return_type;


void sub_param(int idx,char *s) {
	int vi,vt;
	mystrcpy(&( sb[idx]->pname[ ++(sb[idx]->np) ] ) ,s);
	/* should be set ptype according to num/string variable */
	var_add(s,&vi,&vt);
	sb[idx]->ptyp[ (sb[idx]->np) ] = vt;
}

int sub_find(char *s,int *idx,int *zret, int *np, int **plist) {
	int i;
	for (i=1;i<=nsb;i++) {
		if (strcmp(sb[i]->name,s)==0) {
			*idx = i;
			*zret = sb[i]->typ;
			*np = sb[i]->np;
			*plist = &(sb[i]->ptyp[1]);
			return i;
		}
	}
	return 0;
}

void sub_clear() {
	int i,j;
	for (i=1;i<=nsb;i++) {
	  if (sb[i] != NULL) {
		for (j=1; j<= sb[i]->np; j++) {
			if (sb[i]->pname[j] != NULL) myfree(sb[i]->pname[j]);
		}
	  }
	  myfree(sb[i]);
	  sb[i] = NULL;
	}
	nsb = 0;
}

int sub_def(char *s) {
	int i;
	for (i=1;i<=nsb;i++) {
		if (strcmp(sb[i]->name,s)==0) {
			strcpy(sb[i]->name,"^");
		}
	}
	if (i>nsb) {
		nsb = i;
		sb[i] = (sub_st*)myallocz(sizeof(*sb[0]));
		strcpy(sb[i]->name,s);
	}
	sb[i]->np = 0;
	return i;
}

void sub_set_startend(int idx, int ss, int ee) {
	if (idx<0 || idx>1000) {
		gprint("idx is out of range \n");
		return;
	}
	sb[idx]->start = ss;
	sb[idx]->end = ee;
}

void sub_get_startend(int idx, int *ss, int *ee) {
	*ss = sb[idx]->start;
	*ee = sb[idx]->end;
}

/* Run a user defined function  */
void sub_call(int idx,double *pval,char **pstr,int *npm) {
	int i;
	double save_return_value;

    	save_return_value = return_value;
	var_alloc_local();
	dbg for (i=0;i<4;i++) gprint("STACK IN SUBCALL, (%d) = %f \n",i,*(pval+i));
	if (*npm<sb[idx]->np) gprint("parameters in sub_call, not enough **\n");
	for (i = sb[idx]->np;i>=1;i--) {
		if (sb[idx]->ptyp[i] == 1)  {
			var_set(200 + i-1,*(pval+(*npm)--));
		} else	{
			var_setstr(200 + i-1,*(pstr+(*npm)--));
		}
	}

	dbg gprint("SUB CALL ----- startline %d   end %d \n",
		sb[idx]->start,sb[idx]->end);

	for (i = sb[idx]->start + 1;i< (sb[idx]->end);i++) {
		dbg gprint("=Call do pcode, line %d ",i);
		// do_pcode(&i,(*gpcode)[i],(*gplen)[i],&endp);
		dbg gprint("AFTER DO_PCODE I = %d \n",i);
	}
	dbg gprint("FINISHED CALL ------\n");
	*(pval + ++(*npm)) = return_value;
	return_value = save_return_value;
	var_free_local();
	dbg for (i=0;i<=*npm;i++) gprint("STACK IN SUBCALL, (%d) = %f \n",i,*(pval+i));
}

void sub_set_return(double d) {
	return_value = d;
}

