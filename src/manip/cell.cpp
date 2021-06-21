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
#include "edt.h"

#define far
#define huge
#define farcalloc calloc
#define DASHCHAR '.'
#include "vaxconio.h"

extern int changed;
extern int data_x;
extern int data_y;
extern int max_x,max_y;
int usefloat;

#ifdef __TURBOC__
double MISS=1.7e300;
#else
double MISS=1e10;
#endif
char huge **d_strings;
double huge *d_numbers;
float huge *f_numbers;
#define cellindex(x,y) (((x)-1)+((y)-1)* ((int32) data_x))
#define oldindex(x,y) (((x)-1)+((y)-1)* ((int32) odx))
extern char strmiss[];

void set_usefloat() {
	usefloat = true;
#ifdef __TURBOC__
	MISS = 3e38;
#else
	MISS = 3.3e10;
#endif
}

void data_expand(int x, int y) {
	int  datx,daty,mx,my;
	/*fner("Expand %d %d,  max %d %d, data %d %d ",x,y,max_x,max_y
		,data_x,data_y); scr_getch();*/
	mx = max_x; my = max_y;
	datx = data_x; daty = data_y;
	if (x>mx) mx = x;
	if (y>my) my = y;
	if (mx>my) {
		datx = mx * 2;
		daty = my;
	} else {
		datx = mx;
		daty = my * 2;
	}
	xdata_setsize(datx,daty,mx,my);
}

void clear_data() {
	int32 i,j,k;
	if (usefloat) {
	  if (f_numbers!=NULL) free(f_numbers);
	  f_numbers = NULL;
	} else {
	  for (i=1;i<=max_x;i++) for (j=1;j<=max_y;j++) {
		k = cellindex(i,j);
		if (d_strings[k]!=NULL) free(d_strings[k]);
	  }
	  if (d_numbers!=NULL) free(d_numbers);
	  if (d_strings!=NULL) free(d_strings);
	  d_numbers = NULL; d_strings = NULL;
	}
	max_x = 0;
	max_y = 0;
	data_x = 0;
	data_y = 0;
	manip_refresh();
}

void set_size(int x, int y) {
	int32 i,j,k;
	for (i=1;i<=max_x;i++) for (j=1;j<=max_y;j++) {
	  if (i>x || j>y) {
		k = cellindex(i,j);
		if (usefloat) f_numbers[k] = MISS;
		else {
			d_numbers[k] = MISS;
			if (d_strings[k]!=NULL) {free(d_strings[k]); d_strings[k]=NULL;}
		}
	  }
	}
	data_setsize(x,y);
	max_x = x;
	max_y = y;
	manip_refresh();
}

void data_setsize(int x, int y) {
	xdata_setsize(x,y,max_x,max_y);
}

void xdata_setsize(int x, int y,int mx,int my) {
	/* x,y = new size of array,  mx, my = new used part of array */
	char huge **ds;
	int  ox,oy,odx;
	int32 k;
	int32 i,j;
	double huge *dn;
	float huge *fn;
	if (x<=data_x && y<=data_y) return;
	ox = max_x; oy = max_y; odx = data_x;
	max_x = mx;
	max_y = my;
	data_x = x;
	data_y = y;
	if (usefloat) {
	  fn = (float*)farcalloc(((int32) data_x)*data_y, sizeof(float));
	  if (fn==NULL) {fner("memory %d %d %ld\n",data_x,data_y,((int32) data_x)*data_y);}
	  if (fn==NULL) gle_abort("Unable to allocate more storage\n");
	  for (k=0;k<data_x*data_y;k++) fn[k] = MISS;
	  if (f_numbers!=NULL) {
		for (i=1;i<=ox;i++) for (j=1;j<=oy;j++) {
			fn[cellindex(i,j)] = f_numbers[oldindex(i,j)];
		}
		free(f_numbers);
	  }
	  f_numbers = fn;
	  top_line();
	  return;
	}
	/*fner("expand old %d %d, odx=(%d)  new %d %d \n"
		,ox,oy,odx,data_x,data_y); scr_getch();*/
	dn = (double*)farcalloc(((int32) data_x)*data_y, sizeof(double));
	ds = (char**)farcalloc(((int32) data_x)*data_y, sizeof(char *));
/* printf("New dimensions %d %d  %d %d  \n",data_x,data_y,max_x,max_y);*/
	if (dn==NULL || ds==NULL) gle_abort("Unable to allocate more storage\n");
	for (k=0;k<data_x*data_y;k++) dn[k] = MISS;
	if (d_numbers!=NULL) {
		for (i=1;i<=ox;i++) for (j=1;j<=oy;j++) {
			/*fner("copy i=%ld j=%ld   (%ld)  <-- (%ld) \n"
				,i,j,cellindex(i,j),oldindex(i,j)); scr_getch();*/
			dn[cellindex(i,j)] = d_numbers[oldindex(i,j)];
			ds[cellindex(i,j)] = d_strings[oldindex(i,j)];
		}
		free(d_numbers); free(d_strings);
	}
	d_numbers = dn;
	d_strings = ds;
	top_line();
}

void set_scell(int x, int y, char *s) {
	int32 j;
	if (x>data_x || y>data_y) data_expand(x,y);
	if (x>max_x) max_x = x;
	if (y>max_y) max_y = y;
	j = cellindex(x,y);
	/* fner("scell %d %d %ld \n",x,y,j); scr_getch(); */
	if (usefloat) {f_numbers[j] = 0; return;}
	if (d_strings[j]!=NULL) free(d_strings[j]);
	d_strings[j] = (char *) strdup(s);
	d_numbers[j] = 0;
	changed = true;
}

void set_cell(int x, int y, double d) {
	char *s;
	if (x>data_x || y>data_y) data_expand(x,y);
	if (x>max_x) max_x = x;
	if (y>max_y) max_y = y;
	if (usefloat) {f_numbers[cellindex(x,y)] = d; return;}
	d_numbers[cellindex(x,y)] = d;
	s = d_strings[cellindex(x,y)];
	if (s!=NULL) {free(s); d_strings[cellindex(x,y)] = NULL;}
	changed = true;
}

bool cell_greater(int x1,int y1, int x2, int y2) {
	int32 j1,j2;
	if (x1>max_x || y1>max_y) return false;
	if (x2>max_x || y2>max_y) return true;
	j1 = cellindex(x1,y1);
	j2 = cellindex(x2,y2);
	if (!usefloat) {
	if (d_strings[j1]!=NULL && d_strings[j2]!=NULL) {
		if (strcmp(d_strings[j1],d_strings[j2])>0)
			return true;
		else
			return false;
	}
	}
	if (vcell(x1,y1)>vcell(x2,y2)) return true;
	return false;
}

char *scell(int x, int y) {
	int32 j;
	static char buff[80];
	if (x>max_x || y>max_y) return strmiss;
	j = cellindex(x,y);
	if (usefloat) {
		if (f_numbers[j]>=MISS) return strmiss;
		sprintf(buff,"%g",f_numbers[j]);
		return buff;
	}
	if (d_strings[j]!=NULL) {
		return d_strings[j];
	}
	if (d_numbers[j]>=MISS) return strmiss;
	sprintf(buff,"%g",d_numbers[j]);
	return buff;
}

double vcell(int x, int y) {
	int32 j;
	if (x>max_x || y>max_y) return 0;
	j = cellindex(x,y);
	if (usefloat) {
		if (f_numbers[j]>=MISS) return 0;
		return f_numbers[j];
	}
	if (d_strings[j]!=NULL) return 0;
	if (d_numbers[j]>=MISS) return 0;
	return d_numbers[j];
}

void get_cell(int x, int y, double *v) {
	int32 j;
	*v = 0;
	if (x>max_x || y>max_y) return;
	j = cellindex(x,y);
	if (usefloat) {
		if (f_numbers[j]>=MISS) return ;
		*v =  f_numbers[j];
		return;
	}
	if (d_strings[j]!=NULL) return;
	if (d_numbers[j]>=MISS) return;
	*v = d_numbers[j];
}

void copy_cell(int x, int y, int x2, int y2) {
	int32 j;
	if (x>max_x || y>max_y) {set_cell(x2,y2,MISS); return;}
	j = cellindex(x,y);
	if (usefloat) {set_cell(x2,y2,f_numbers[j]); return;}
	if (d_strings[j]!=NULL) {set_scell(x2,y2,d_strings[j]); return;}
	set_cell(x2,y2,d_numbers[j]);
}

void swap_cell(int x, int y, int x2, int y2) {
	char *s;
	double v;
	int32 j,j2;
	if (x>data_x || y>data_y) data_expand(x,y);
	if (x2>data_x || y2>data_y) data_expand(x2,y2);
	if (x>max_x) max_x = x;
	if (y>max_y) max_y = y;
	if (x2>max_x) max_x = x2;
	if (y2>max_y) max_y = y2;

	j = cellindex(x,y);
	j2 = cellindex(x2,y2);
	if (usefloat) {
		v = f_numbers[j];
		f_numbers[j] = f_numbers[j2];
		f_numbers[j2] = v;
		return;
	}
	v = d_numbers[j];
	s = d_strings[j];
	d_numbers[j] = d_numbers[j2];
	d_strings[j] = d_strings[j2];
	d_numbers[j2] = v;
	d_strings[j2] = s;
}

void clear_cell(int x, int y) {
	int32 j;
	if (x>max_x || y>max_y) {return;}
	j = cellindex(x,y);
	if (usefloat) {f_numbers[j] = MISS; return;}
	if (d_strings[j]!=NULL) {free(d_strings[j]); d_strings[j] = NULL;}
	d_numbers[j] = MISS;
}

void get_cellboth(int x, int y, double *v, char **s) {
	int32 j;
	*s = NULL;
	*v = 0;
	if (x>max_x || y>max_y) {*s = strmiss; return;}
	j = cellindex(x,y);
	if (usefloat) {
		if (f_numbers[j]>=MISS) {*s = strmiss; return;}
		*v = f_numbers[j];
		*s = NULL;
		return;
	}
	if (d_strings[j]!=NULL) {*s = d_strings[j]; return;}
	if (d_numbers[j]>=MISS) {*s = strmiss; return;}
	*v = d_numbers[j];
}

void trim_data() {
	int32 i,j,k,mx,my;
	mx = my = 0;
	for (i=1;i<=max_x;i++) for (j=1;j<=max_y;j++) {
 	  k = cellindex(i,j);
	  if (usefloat) {
		if (f_numbers[k]!=MISS) {
			if (i>mx) mx = i;
			if (j>my) my = j;
		}
	  } else {
		if (d_numbers[k]!=MISS) {
			if (i>mx) mx = i;
			if (j>my) my = j;
		}
	  }
	}
	max_x = mx;
	max_y = my;
}
