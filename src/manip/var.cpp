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

int var_local;		/* 0 = global, 1 = local */
char *var_names[100];	/* Global variables */
int var_type[100];
int nvar;
char *lvar_names[100];	/*  Local variables  */
int lvar_type[100];
int nlvar;

static int ndp;
static double (*lvar_val)[100];
static char *(*lvar_str)[100];
static double var_val[100];
static char *var_str[100];

void var_check(int *j) {
	if (*j<0 || *j>400) {
		gprint("Variable index is wrong %d \n",*j);
		*j = 1;
	}
	if (lvar_val == NULL  && *j>=200) {
		*j = 0;
		gprint("No local variables assigned \n");
	}
}

void var_alloc_local() {
/*	dp_stack[++ndp] = lvar_val;
	dp_stackstr[ndp] = lvar_str;
	lvar_val = (double* (*)[100])myallocz(sizeof(*lvar_val));
	lvar_str = (char* (*)[100])myallocz(sizeof(*lvar_str)); */
}

void var_free_local() {
	if (ndp==0) {gprint("Cannot free local as none saved \n"); return;}
/*	myfree(lvar_val);
	myfree(lvar_str);
	lvar_val = dp_stack[ndp];
	lvar_str = dp_stackstr[ndp--];*/
}

void var_set(int jj, double v) {
	var_check(&jj);
	if (jj<200)
		var_val[jj] = v;
	else
		(*lvar_val)[jj-200] = v;
}

void var_setstr(int jj, char *s) {
	var_check(&jj);
	if (jj<200)
		mystrcpy(&var_str[jj],s);
	else
		mystrcpy(&(*lvar_str)[jj-200],s);
}

void var_getstr(int jj, char *s) {
	var_check(&jj);
	if (jj<200) {
		if (var_str[jj]!=NULL) strcpy(s,var_str[jj]);
		else {
			strcpy(s,"");
			gprint("String Variable not defined %d \n",jj);
		}
	} else {
		if ((*lvar_str)[jj-200]!=NULL) strcpy(s,(*lvar_str)[jj-200]);
		if ((*lvar_str)[jj-200]==NULL) gprint("ERROR, variable not defined\n");
	}

}

void var_get(int jj, double *v) {
	var_check(&jj);
	if (jj<200)
		*v = var_val[jj];
	else
		*v = (*lvar_val)[jj-200];
}

void var_nlocal(int *l) {
	*l = nlvar;
}

void var_clear_global() {
	nvar = 0;
}

void var_set_local() {
	var_local = true;
	nlvar = 0;
}

void var_set_global() {
	var_local = false;
}

void var_clear_local() {
	nlvar = 0;
}

/* Add a variable to the list */
void var_findadd(const char *name,int *idx,int *type) {
	var_find(name,idx,type);
	if (*idx==-1) var_add(name,idx,type);
}

/* Add a variable to the list */
void var_add(const char *name,int *idx,int *type) {
	if (var_local==0) {
		if (nvar>90) gprint("Too many global variables \n");
		mystrcpy(&var_names[nvar],name);
		*idx = nvar;
		if (manip_lastchar(name,'$')) *type=2;
		else *type=1;
		var_type[nvar++] = *type;
	} else {
		if (nlvar>90) gprint("Too many local variables \n");
		mystrcpy(&lvar_names[nlvar],name);
		*idx = nlvar+200;
		if (manip_lastchar(name,'$')) *type=2;
		else *type=1;
		lvar_type[nlvar++] = *type;
	}
}

/* Find a variable in the list */
void var_find(const char *name,int *idx,int *type) {
	int i;
	for (i=0;i<nlvar;i++) {
		if (strcmp(lvar_names[i],name)==0) {
			*idx = i+200;
			*type = lvar_type[i];
			return;
		}
	}
	for (i=0;i<nvar;i++) {
		if (strcmp(var_names[i],name)==0) {
			*idx = i;
			*type = var_type[i];
			return;
		}
	}
	*idx = -1;
}

void var_find_dn(int *idx, int *var, int *nd) {
	int i,d;
	*nd = 0;
	for (i=0;i<nlvar;i++) {
		if (strncmp(lvar_names[i],"D",1)==0) {
			d = atoi(lvar_names[i]+1);
			if (d>0 && d<100) {
				++*nd;
				*idx++ = i+200;
				*var++ = d;
			}
		}
	}
}

void var_find_rc(int *idx, int *var, int *nd, int c) {
	int i,d;
	*nd = 0;
	for (i=0;i<nlvar;i++) {
		if (*lvar_names[i]==c) {
			d = atoi(lvar_names[i]+1);
			if (d>0) {
				++*nd;
				*idx++ = i+200;
				*var++ = d;
			}
		}
	}
	for (i=0;i<nvar;i++) {
		if (*var_names[i]==c) {
			d = atoi(var_names[i]+1);
			if (d>0) {
				++*nd;
				*idx++ = i;
				*var++ = d;
			}
		}
	}
}

bool str_var(char* s) {
	int i;
	i = strlen(s);
	if (*(s+i-1)=='$') return true;
	  else return false;
}

bool valid_var(const char *s) {
	return true;
}
