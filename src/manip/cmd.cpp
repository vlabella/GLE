/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://glx.sourceforge.net/>          *
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

/*  number    - put it in current cell and move in current dir
	'string   - put string in cell and move in current direction
	range = expression  - evaluate exp for each cell in range
	cell(exp,exp) = expression  - evaluate exp and put in cell
	cmd - execute command, if then else next for save load generate
				   print, input
	& = continued on next line.

	load b.dat  range c3r3   width 10.3  between ", "
	save b.c  c3c4
	set outwidth 0
	set outwidth 10.3
	set between ", "
	copy c1 c2
*/

#include "all.h"
#include "edt.h"
#include "keyword.h"

using namespace std;

extern int in_recover;
extern int curx;
extern int cury;
extern int max_x,max_y;
#define tok(j) (tk[j])
#define dbg if (gle_debug>0)
extern char file_name[];
extern int iserr;
extern int gle_debug,changed;
extern int trace_on;
extern int exit_manip;
int isating;
int islogging;
int load_list;
int dpoints[200];
int coltype[200];
int outwidth1=8,outwidth2=8;
const char *between=" ";
char strmiss[40];

void do_command(char *cmd) {
	static char inbuff[200];
	static char *tk[500];
	static char tkbuff[500];
	int i,al,ntok;
	static char space_str[] = " ";

	if (islogging) log_write(cmd);

	token_space();
	strcat(inbuff,cmd);
	al = strlen(inbuff);
	if (inbuff[al-1]=='&') {
		inbuff[al-1] = 0;
		return;
	}
	token(inbuff,tk,&ntok,tkbuff);
	for (i=ntok+1;i< ntok+5;i++) tk[i] = space_str;
/*   for (i=1;i<=ntok;i++) printf("token{%s} ",tk[i]);
	 printf("\n"); */
	passcmd(inbuff,tk,&ntok);
	inbuff[0] = 0;

}

extern int moving_x, moving_y;

void passcmd(char *source,char *tk[],int *ntok) {
	int j,i,ix,vidx,vtype=1;
	double v;
	char *sp;
	find_mkey(tok(1),&ix);
	if (strcmp(tok(2),"=")==0)  {
		do_assign(tok(1),tok(3));
		return;
	}
	sp = strchr(tok(1),'=');
	if (sp!=NULL) {
		*sp = 0;
		do_assign(tok(1),sp+1);
		return;
	}
	if (gle_isnumber(tok(1)) || *tok(1)=='-') {
		set_cell(curx,cury,atof(tok(1)));
		set_newxy(curx+moving_x,cury+moving_y);
		return;
		}
	if ((*tok(1))=='"') {
		*(tok(1)+strlen(tok(1))-1) = 0;
		set_scell(curx,cury,tok(1)+1);
		set_newxy(curx+moving_x,cury+moving_y);
		return;
		}
	if ((*tok(1))=='@') {
		at_open(tok(1)+1);
		return;
	}
	switch (ix) {
	  case k_data: /* data <range> */
		cmd_data(tok(2));
		break;
	  case k_help:/* help load */
		if (*ntok==1) do_help("MANIP","");
		else do_help(tok(2),tok(3));
		manip_refresh();
		break;
	  case k_list: /* list a.a [range] , reads a list of numbers */
		load_list = true;
		cmd_load(tok(2),tok(3),*ntok);
		load_list = false;
		break;
	  case k_load: /* LOAD  A.A  [range] -LIST */
		if (*ntok>3) if (strcmp(tok(4),"-LIST")==0) load_list = true;
		cmd_load(tok(2),tok(3),*ntok);
		load_list = false;
		break;
	  case k_save: /* SAVE [a.a] [range] [format-string] */
		changed = false;
		cmd_save(tok(2),tok(3),tok(4),*ntok);
		break;
	  case k_set: /* set outwidth, set width, set between, */
		if (strcmp(tok(2),"OUTWIDTH")==0) set_outwidth(tok(3));
		else if (strcmp(tok(2),"WIDTH")==0) set_outwidth(tok(3));
		else if (strcmp(tok(2),"BETWEEN")==0) between = unquote(tok(3));
		else if (strcmp(tok(2),"MISSING")==0) strcpy(strmiss,unquote(tok(3)));
		else if (strcmp(tok(2),"DIGITS")==0) outwidth2 = atoi(tok(3));
		else if (strcmp(tok(2),"DPOINTS")==0) {
			i = atoi(tok(3));
			if (*ntok>3) dpoints[atoi(tok(4))] = i;
			else for (j=0;j<200;j++) dpoints[j] = i;
		}
		else if (strcmp(tok(2),"COLTYPE")==0) {
			i = atoi(tok(3));
			if (*tok(3)=='C') i = atoi(tok(3)+1);
			if (strcmp(tok(4),"BOTH")==0) j = 0;
			if (strcmp(tok(4),"DECIMAL")==0) j = 1;
			if (strcmp(tok(4),"EXP")==0) j = 2;
			if (strcmp(tok(4),"DPOINTS")==0) {
				j = atoi(tok(5));
				if (i==0) for (i=0;i<200;i++) dpoints[i] = j;
				else dpoints[i] = j;
			} else {
				if (i==0) for (i=0;i<200;i++) coltype[i] = j;
				else coltype[i] = j;
			}
		}
		else if (strcmp(tok(2),"COLUMNS")==0) set_ncol(atoi(tok(3)));
		else if (strcmp(tok(2),"NCOL")==0) set_ncol(atoi(tok(3)));
		else if (strcmp(tok(2),"COLWIDTH")==0) set_colwidth(atoi(tok(3)));
		else if (strcmp(tok(2),"PRECISION")==0) outwidth2 = atoi(tok(3));
		else if (strcmp(tok(2),"SIZE")==0) {
			set_size(atoi(tok(3)),atoi(tok(4)));
		}
		else fner("Invalid SET command, Use (width,between,digits,dpoints,ncol,colwidth)\n");
		break;
	  case k_new: /* clears the spreadsheet and free's memory */
		clear_data();
		break;
	  case k_clear: /* clear part of the spread sheet */
		cmd_clear(tok(2));
		break;
	  case k_delete: /* delete <range> [if <exp>] */
		cmd_delete(tok(2),tok(4),*ntok);
		trim_data();
		break;
	  case k_insert: /* insert <range> */
		cmd_insert(tok(2));
		break;
	  case k_exit:
		cmd_save(tok(2),tok(3),tok(4),*ntok);
		changed = false;
		exit_manip = true;
		break;
	  case k_quit:
		exit_manip = true;
		break;
	  case k_logging: /* log filename */
		if (islogging) log_close();
		log_open(tk[2]);
		break;
	  case k_close:
		log_close();
		break;
	  case k_at: /* execute file of manip commands */
		at_open(tk[2]);
		break;
	  case k_call:
		manip_refresh();
		break;
	  case k_sort: /* sort <range> on <exp> */
		cmd_sort(tk[2],tk[4],*ntok);
		break;
	  case k_swap: /* swap CaCb  || swap RaRb */
		cmd_swap(tk[2]);
		break;
	  case k_shell:
		window(1,1,80,25); scr_norm(); clrscr();
		printf("Type in EXIT to return to MANIP\n\n");
		#ifndef DJ	/* a.r. */
		if (system("") == -1) {
			printf("Can't show shell prompt\n\n");
		}
		#else
		system(getenv("COMSPEC"));
		#endif
		manip_refresh();
		break;
	  case k_sum:
		cmd_sum(tk[2]);
		break;
	  case k_fit:
		cmd_fit(tk[2]);
		break;
	  case k_parsum:
		cmd_parsum(tk[2],tk[3]);
		break;
	  case k_if:
	  case k_else:
	  case k_end:
	  case k_for:
	  case k_next:
	  case k_print:
	  case k_input: /* input "prompt" variable */
	  case k_copy: /* copy range1  range2  [if exp] (doesn't) */
		cmd_copy(tk[2],tk[3],tk[5],*ntok,true);
		break;
	  case k_move: /* move range1  range2  [if exp] (leaves gaps) */
		cmd_copy(tk[2],tk[3],tk[5],*ntok,false);
		break;
	  case k_prop: /* prop range1  range2  (cycle through range1)  */
		cmd_copy(tk[2],tk[3],tk[5],*ntok,2);
		break;
	  case k_generate: /* gen 2(1,2:4)3  c1  (1 1 2 2 3 3 4 4) 3 times*/
		cmd_generate(tk[2],tk[3],*ntok);
		break;
	  case k_let:
		/* let variable = expression */
		var_findadd(tk[2],&vidx,&vtype);
		evaluate(tk[4],&v);
		var_set(vidx,v);
		break;
	  case k_goto: /* should really use eval, not atoi so var's work */
		set_newxy(atoi(tk[2]),atoi(tk[3]));
		break;
	  default:
		fner("No such command {%s} {%s}\n",tk[1],tk[2]);
	}
}

struct op_key { char *name; int typ; int pos; unsigned int idx; } ;
typedef struct op_key (*OPKEY)[100];

void do_assign(char *t1, char *t2) {
	RANGE rr;
	int cp,vtype;
	double v;
	static int32 pcode[200];
	static char outstr[80];
	int plen;
	vtype = 1;

		/* ------------------       */
		/* variable = expression    */
		/* cell(exp,exp) = exp      */
		/* range = exp          */
		/* ------------------       */

	if (strncmp(t1,"CELL(",5)==0) {     /* cell(exp,exp) = exp */
	  {     char s1[80],s2[80],*s;
		int x,y;
		s = strchr(t1,',');
		if (s==NULL) {
			fner("Expecting CELL(EXP,EXP) = EXP \n");
			return;
		}
		strcpy(s1,t1+5); strcpy(s2,s+1);
		*strchr(s1,',') = 0;
		if (strchr(s2,')')!=NULL) *strchr(s2,')') = 0;
		evaluate(s1,&v); x = (int)v;
		evaluate(s2,&v); y = (int)v;
		evaluate(t2,&v);
		set_cell(x,y,v);
		show_ifcell(x,y);
	  }
	  return;
	}
	if (range_def(t1,&rr)) {                /* range = exp */
	 {
	   int c_idx[20], c_val[20], nc, j;
	   int r_idx[20], r_val[20], nr;

		polish(t2,(char *) pcode,&plen,&vtype);
		var_find_rc(c_idx,c_val,&nc,'C');
		var_find_rc(r_idx,r_val,&nr,'R');
		for (;range_next(&rr);) {
			cp = 0;
			eval_setxy(rr.col,rr.row);
			for (j=0;j<nc;j++) {
				var_set(c_idx[j],vcell(c_val[j],rr.row));
			}
			for (j=0;j<nr;j++) {
				var_set(r_idx[j],vcell(rr.col,r_val[j]));
			}
			eval(pcode,&cp,&v,outstr,&vtype);
			set_cell(rr.col,rr.row,v);
			show_ifcell(rr.col,rr.row);
		}
		return;
	 }
	}
	fner("error, expecting cell(exp,exp) = exp, or range = exp \n");
}

void evaluate(char *exp, double *v) {
	static int32 pcode[200];
	int plen,vtype,cp;
	static char outstr[100];

	vtype = 1;
	polish(exp,(char *) pcode,&plen,&vtype);
	if (plen>150) fner("pcode too int32, report bug to Chris\n");
	cp = 0;
	eval(pcode,&cp,v,outstr,&vtype);
}

int new_line;

bool range_next(RANGE *r) {
	new_line = false;
	if (r->row==0) {
		r->row = r->r1; r->col = r->c1;
		return true;
	}
	if (r->colfirst) {
		r->col++;
		if (r->col > r->c2) {
			new_line = true;
			r->col = r->c1;
			r->row++;
			if (r->row > r->r2) return false;
		}
	} else {
		r->row++;
		if (r->row > r->r2) {
			new_line = true;
			r->row = r->r1;
			r->col++;
			if (r->col > r->c2) return false;
		}
	}
	return true;
}

bool range_def(const char *sxx, RANGE *r) {
  char s[80];
	char *s1;
	char rr,cc;
  strcpy(s,sxx);
	strip_colon(s);
	r->c1 = r->c2 = r->r1 = r->r2 = r->row = r->col = 0;

	if (*s=='C') {cc='C'; rr='R'; r->colfirst = true;}
	else if (*s=='R') {cc='R'; rr='C'; r->colfirst = false;}
	else return false;
/*  c1   c1r1r10   c3c4r7 c3r7  c3c4r1r2 |   r1r2c1c3   c3r7  r2*/

	s1 = range_int(s,&(r->c1));
	if (*s1==cc) {
		s1 = range_int(s1,&r->c2);
	}
	if (*s1==rr) {
		s1 = range_int(s1,&r->r1);
		if (*s1==cc) {
			s1 = range_int(s1,&r->c2);
		}
		if (*s1==rr) {
			s1 = range_int(s1,&r->r2);
		}
		if (*s1==cc) {
			s1 = range_int(s1,&r->c2);
		}
	}
	if (!r->colfirst) {
		swapint(&r->c1,&r->r1);
		swapint(&r->c2,&r->r2);
	}
	if (r->c1 == 0) r->c1 = 1;
	if (r->c2 == 0) {
	  if (!r->colfirst) {
		if (r->c1 > max_x) r->c2 = r->c1; else r->c2 = max_x;
	  } else {
		r->c2 = r->c1;
	  }
	}
	if (r->r1 == 0) r->r1 = 1;
	if (r->r2 == 0) {if (r->colfirst)
		{ if (r->r1>max_y) r->r2 = r->r1; else r->r2 = max_y;}
	  else r->r2 = r->r1;}

/*    printf("range def %d %d   %d %d  | \n",r->c1,r->r1,r->c2,r->r2); getch();*/
	data_expand(r->c2,r->r2);
	if (*s1!=0) return false;
	return true;
}

char *range_int(char *s, int *v) {
	static char buff[40],*b;
	b = buff;
	for (s++;*s!=0 && isdigit(*s);)
		*b++ = *s++;
	*b++ = 0;
	*v = atoi(buff);
	return s;
}

void swapint(int *a, int *b) {
	int c;
	c = *a;
	*a = *b;
	*b = c;
}

const char *sep_chars=", \t\n";

void cmd_load(char *fname, const char *range, int ntok) {
	RANGE rr;
	static char inbuff[8001];
	FILE *fptr;

	m_tokinit(sep_chars);

	if (ntok<2) fname = file_name;
	strcpy(file_name,fname);

	if (ntok>2) {
	 if (range_def(range,&rr)) range_next(&rr);
	 else  {                /* range = exp */
		 fner("Invalid load range given (%s) \n",range);
		 return;
	 }
	} else {
	 rr.col = 1;  rr.row = 1;
	}
	gle_strlwr(fname);
	fptr = fopen(fname,"r");
	if (fptr==NULL) {
		fner("Could not open (%s) \n",fname);
		return;
	}
	for (;!feof(fptr);) {
		if (fgets(inbuff,7000,fptr)!=NULL) {
			if (load_list) cmd_load_list(&rr,inbuff);
			else cmd_load_line(rr.col,rr.row++,inbuff);
		}
	}
	fclose(fptr);
}

static char term_table[256];

void m_tokinit(const char *termset) {
	int i;
	for (i=0;i<=255;i++) {
		if (strchr(termset,i)!=NULL)
			term_table[i]=true;
		else  term_table[i]=false;
	}
}

char *m_tokend(char *s) {
	static char *p1,*p2,*p,savechar;
	if (s!=NULL) {
		p1 = s;
	} else {
		*p2 = savechar;
		if (*p2 == 0) return NULL;
		p1 = p2+1;
	}
	for (p=p1; *p != 0; p++) {  /* find a non space */
		if (term_table[(unsigned int)*p]==false) break;
	}
	if (*p==0) return NULL;
	p1 = p;
	if (*p=='"') {
		p2 = strchr(p+1,'"');
		if (p2==NULL) p2 = p;
		p = ++p2;
	} else {
		for (; *p != 0; p++) {  /* find next space */
			if (term_table[(unsigned int)*p]==true) break;
		}
	}
	p2 = p;
	savechar = *p2;
	*p2 = 0;
	return p1;
}

void cmd_load_list(RANGE *r, char *inbuff) {
	char *s;
	s = m_tokend(inbuff);
	for (;s!=NULL;) {
		load_str(r->col,r->row,s);
		s = m_tokend(NULL);
		range_next(r);
	}

}

void cmd_load_line(int x, int y, char *inbuff) {
	char *s;
	s = m_tokend(inbuff);
	for (;s!=NULL;) {
		load_str(x,y,s);
		s = m_tokend(NULL);
		x++;
	}

}

void load_str(int x, int y, char *s) {
	static char bb[88];
	if (strcmp(s,".")==0 || strcmp(s,"-")==0 || strcmp(s,"*")==0) clear_cell(x,y);
	else if (isdigit(*s) || *s=='-' || *s=='+' || *s=='.') set_cell(x,y,atof(s));
	else {
		if (strlen(s)>80) {
			strncpy(bb,s,80);
			bb[80] = 0;
			set_scell(x,y,bb);
		} else set_scell(x,y,s);
	}
	show_ifcell(x,y);
}

void cmd_save(char *fname, const char *range, const char *format, int ntok) {
	RANGE rr;
	static char inbuff[2001];
	static char fmtg[80],buff1[80],savefmtg[80];
	FILE *fptr;
	int lcount=0;
	int i,inlen=0,thelot,k;
	double v;
	char *s;

	gle_strlwr(fname);
	trim_data();
	thelot = false;
	if (*range=='-') {
		const char* tmp = range;
		range = format;
		format = tmp;
	}
	if (strcmp(format,"-COMMA")==0) {
		set_outwidth(0);
		between = strdup(", ");
	}
	if (strcmp(format,"-TAB")==0) {
		set_outwidth(0);
		between = strdup("\t");
	}
	if (strcmp(format,"-SPACE")==0) {
		set_outwidth(0);
		between = strdup(" ");
	}

	sprintf(fmtg,"%%.%dg",outwidth2);
	if (outwidth1==0 && outwidth2==0) strcpy(fmtg,"%g");
	strcpy(savefmtg,fmtg);

	if (ntok<2) fname = file_name;
	if (strcmp(fname,"*")==0) fname = file_name;
	strcpy(file_name,fname);

	/* format strings   "%2.3f, " */
	if (ntok<4) format = (char*)"%g ";
	if (ntok<3 || strcmp(range," ")==0) thelot = true;
	if (thelot) range = (char*)"C1R1";
	if (range_def(range,&rr)) ;
	else  {                 /* range = exp */
		fner("Invalid range given (%s)%d \n",range,strlen(range));
		return;
	}
	if (thelot) rr.c2 = max_x;
	unlink("manip_.bak");
	unlink("manip_.tmp");
	fptr = fopen("manip_.tmp","w");
	if (fptr==NULL) {
		fner("Could not open (%s) \n",fname);
		return;
	}
	for (;range_next(&rr);) {
		if (new_line==true) {
			if (fprintf(fptr,"%s\n",inbuff) <0) printf("Error writing output file\n");
			inbuff[50] = 0;
			if (++lcount==1) fner("{%s} W=%d D=%d {%s}",fname,outwidth1,outwidth2,inbuff);
			inlen = 0;
		}
		if (inlen>0) { strcpy(inbuff+inlen,between); inlen +=strlen(between);}
		get_cellboth(rr.col,rr.row,&v,&s);
		k = rr.col;
		if (k>199) k = 199;
		if (coltype[k]==0) strcpy(fmtg,savefmtg);
		if (coltype[k]==1) strcpy(fmtg,"%f");
		if (coltype[k]==2) strcpy(fmtg,"%e");
		if (s==NULL) {
			i = sprintf(buff1,fmtg,vcell(rr.col,rr.row));
			if (dpoints[k]>0) i = strcpydecimal(inbuff+inlen,buff1,outwidth1,dpoints[k]);
			else i = strcpywidth(inbuff+inlen,buff1,outwidth1);
		} else {
			i = strcpywidth(inbuff+inlen,s,outwidth1);
		}
		inlen += i;
	}
	if (new_line==true) {
		if (fprintf(fptr,"%s\n",inbuff)<0) {
			fner("Error writing output file");
			return;
		}
		inbuff[50] = 0;
		if (++lcount==1) fner("Width=%d Precision=%d {%s}",outwidth1,outwidth2,inbuff);
		inlen = 0;
	}
	fclose(fptr);
	if (rename(fname,"manip_.bak")!=0){/*intentionally empty*/};
	if (rename("manip_.tmp",fname)!=0) fner("Unable to rename manip_.tmp");
}

int strcpydecimal(char *dest, char *src, int wid, int dpoints) {
	/* 123 ,  12.031,  0.0003321 */
	/* dpoints 2 goes too   */
	/* 123.00  12.03  0.00  */
	static char buff[80];

	strcpy(buff,src);
	if (strchr(buff,'.')==NULL) strcat(buff,".");
	strcat(buff,"0000000000000000000000000000000000");
	*(strchr(buff,'.')+dpoints+1) = 0;
	strcpywidth(dest,buff,wid);
	return strlen(dest);
}

int strcpywidth(char *dest, char *src, int wid) {
	static char buff[80];
	int i = wid-strlen(src);
    if (i < 0) {
    	buff[0] = 0;
    } else {
    	memset(buff, (int)' ',i);
    	buff[i] = 0;
    }
	strcpy(dest,buff);
	strcat(dest,src);
	return strlen(dest);
}

void  set_outwidth(char *s) {
	char *ss;
	ss = strchr(s,'.');
	if (ss==NULL) {
		outwidth1 = atoi(s);
		outwidth2 = 8;
	} else {
		*ss++ = 0;
		outwidth1 = atoi(s);
		outwidth2 = atoi(ss);
	}
}

char *unquote(char *s) {
	char ss[80];
	strcpy(ss,s);
	if (*s=='"') {
	ss[strlen(ss)-1] = 0;
	return strdup(ss+1);
	} else return strdup(ss);
}

void cmd_copy(char *src, char *dest, char *ifexp, int ntok, int always) {
	RANGE ss,dd,savess;
	int c_idx[20], c_val[20], nc, j, isif=false;
	int r_idx[20], r_val[20], nr;
	int cp,vtype,ddok,ssok;
	double v;
	char outstr[30];
	int32 pcode[200];
	int plen;
	vtype = 1;

	if (ntok>4) isif = true;
	if (!range_def(src,&ss)) { fner("Error in source range {%s} \n",src); return;}
	if (!range_def(dest,&dd)) { fner("Error in destination range {%s} \n",dest); return;}

	if (isif) {
		polish(ifexp,(char *) pcode,&plen,&vtype);
		var_find_rc(c_idx,c_val,&nc,'C');
		var_find_rc(r_idx,r_val,&nr,'R');
	}
	ddok = range_next(&dd);
	ssok = range_next(&ss);
	savess = ss;
	for (;ssok && ddok;) {
		if (isif) {
			cp = 0;
			eval_setxy(ss.col,ss.row);
			eval_setxyd(dd.col,dd.row);
			for (j=0;j<nc;j++) {
				var_set(c_idx[j],vcell(c_val[j],ss.row));
			}
			for (j=0;j<nr;j++) {
				var_set(r_idx[j],vcell(ss.col,r_val[j]));
			}
			eval(pcode,&cp,&v,outstr,&vtype);
		}
		if (v==true || !isif) {
			copy_cell(ss.col,ss.row,dd.col,dd.row);
			show_ifcell(dd.col,dd.row);
			ddok = range_next(&dd);
		} else if (always) ddok = range_next(&dd);
		if (always!=2) ddok = true;
		ssok = range_next(&ss);
		if (!ssok && always==2) {
			ss = savess;    ssok = true;
		}
	}
}

void cmd_generate(char *patx, char *dest, int ntok) {
	int left,right;
	RANGE rr;
	static char pat[188];
	static char middle[80];
	double v;
	int i,j;

	strcpy(pat,patx);
	if (strchr(pat,'(')==NULL) {fner("Expecting left bracket"); return;}
	if (strchr(pat,')')==NULL) {fner("Expecting left bracket"); return;}
	if (ntok!=3) {fner("Expecting (GEN pattern range)"); return;}

	if (pat[0]=='(') {strcpy(pat,"1"); strcat(pat,patx);}
	if (pat[strlen(pat)-1] == ')') strcat(pat,"1");
	left = atoi(strtok(pat,"()"));
	strcpy(middle,strtok(NULL,"()"));
	right = atoi(strtok(NULL,"()"));
	if (left==0) left = 1;
	if (right==0) right = 1;
	if (!range_def(dest,&rr))  {fner("Invalid range given (%s) \n",dest); return;}
	for (i=0;i<right;i++) {
	  gen_next(middle,&v);
	  for (;gen_next(NULL,&v);) {
		for (j=0;j<left;j++) {
		range_next(&rr);
		set_cell(rr.col,rr.row,v);
		show_ifcell(rr.col,rr.row);
		}
	  }
	}
}

bool gen_next(char *pat, double *v) {
	static char *s,p[200];
	static double v1,v2,v3;
	static int instep;
	char *c1,*c2;
	if (pat!=NULL) {
		strcpy(p,pat);
		s = strtok(p,",");
		return true;
	}
	if (instep) {
		   v1 += v3;
		   *v = v1;
		   if (v1<=v2) return true;
		   else instep = false;
	}
	if (s==NULL) return false;
	c2 = c1 = strchr(s,':');
	if (c1!=NULL) c2 = strchr(c1+1,':');

	if (c1==NULL) {
		*v = atof(s);
		s = strtok(NULL,",");
		return true;
	}
	v1 = atof(s);
	v2 = atof(c1+1);
	if (c2==NULL) v3 = 1; else v3 = atof(c2+1);
	*v = v1;
	instep = true;
	s = strtok(NULL,",");
	if (v1>v2) return false;
	return true;
}

int c_idx[20], c_val[20], ncc;
int r_idx[20], r_val[20], nrr;
int32 cpcode[200];

void cmd_polish(char *exp) {
	int vtype;
	int plen;
	vtype = 1;
	polish(exp,(char *) cpcode,&plen,&vtype);
	var_find_rc(c_idx,c_val,&ncc,'C');
	var_find_rc(r_idx,r_val,&nrr,'R');
}

void cmd_eval(int x1, int y1, double *v) {
	int cp,vtype,j;
	char outstr[30];
	vtype = 1;

	cp = 0;
	eval_setxy(x1,y1);
	eval_setxyd(x1,y1);
	for (j=0;j<ncc;j++) {
		var_set(c_idx[j],vcell(c_val[j],y1));
	}
	for (j=0;j<nrr;j++) {
		var_set(r_idx[j],vcell(x1,r_val[j]));
	}
	eval(cpcode,&cp,v,outstr,&vtype);
}

void cmd_delete(char *range, char *ifexp, int ntok) {
	RANGE rr;
	double v = 0.0;
	int isif = false,j,wid,w;
	if (ntok>2) isif = true; else isif = false;
	if (!range_def(range,&rr)) { fner("Error in range {%s} \n",range); return;}

	if (isif) cmd_polish(ifexp);
	range_next(&rr);
	if (rr.colfirst) {
		w = rr.c2-rr.c1+1;
		wid = max_x-rr.c1;
	} else {
		w = rr.r2-rr.r1+1;
		wid = max_y-rr.r1;
	}
	for (;;) {
		if (rr.colfirst) {
		  if (isif) cmd_eval(rr.col,rr.row,&v);
		  if (v==true || !isif) {
			for (j=0;j<wid; j++) {
			  copy_cell(rr.col+j+w,rr.row,rr.col+j,rr.row);
			  show_ifcell(rr.col+j,rr.row);
			}
			for (j=0;j<w;j++) {
			  clear_cell(rr.c2+wid-j,rr.row);
			  show_ifcell(rr.c2+wid-j,rr.row);
			}
		  }
		  rr.row++;
		  if (rr.row>rr.r2) break;
		} else {
		  if (isif) cmd_eval(rr.col,rr.row,&v);
		  if (v==true || !isif) {
			for (j=0;j<wid; j++) {
			  copy_cell(rr.col,rr.row+j+w,rr.col,rr.row+j);
			  show_ifcell(rr.col,rr.row+j);
			}
			for (j=0;j<w;j++) {
			  clear_cell(rr.col,rr.r2+wid-j);
			  show_ifcell(rr.col,rr.r2+wid-j);
			}
		  }
		  rr.col++;
		  if (rr.col>rr.c2) break;
		}
	}
}

int oncol;

void cmd_sort(char *range, char *ifexp, int ntok) {
	RANGE rr;
	double v;
	int *pnt;
	int i,j,c,savemax;
	if (ntok<3) {fner("Expecting   SORT  c1c2  ON <exp> "); return; }
	if (!range_def(range,&rr)) { fner("Error in range {%s} \n",range); return;}

	savemax = max_x;
	oncol = max_x + 1;
	cmd_polish(ifexp);
	range_next(&rr);
	if (ntok==5) {
	  for (i=rr.r1;i<=rr.r2;i++) {
		copy_cell(rr.c1,i,oncol,i);
	  }
	} else {
	  for (i=rr.r1;i<=rr.r2;i++) {
		cmd_eval(rr.c1,i,&v);
		set_cell(oncol,i,v);
	  }
	}
	pnt = (int*)calloc(max_y+2,sizeof(int));
	if (pnt==NULL) {fner("Not enough memory to sort"); return;}
	for (i=0;i<=max_y+1;i++) pnt[i] = i;
	for (i=rr.r1+1;i<=rr.r2;i++) {
		if (cell_greater(oncol,pnt[i-1],oncol,pnt[i])) {
			sort_shuffle(i,rr.r1,pnt);
		}
	}
	for (c=rr.c1; c<=rr.c2; c++) {
		for (j=rr.r1;j<=rr.r2;j++) {
			copy_cell(c,pnt[j],oncol,j);
		}
		for (j=rr.r1;j<=rr.r2;j++) {
			copy_cell(oncol,j,c,j);
			show_ifcell(c,j);
		}
	}
	for (j=1;j<=max_y;j++) clear_cell(oncol,j);
	max_x = savemax;
}

void sort_shuffle(int i, int r1, int *pnt) {
	int ins,j,x;
	ins = r1;
	for (j=i-1;j>=r1;j--) {
		if (cell_greater(oncol,pnt[i],oncol,pnt[j])) {
			ins = j + 1;
			break;
		}
	}
	x = pnt[i];
	for (j=i;j>=ins;j--) pnt[j] = pnt[j-1];
	pnt[ins] = x;
}

void cmd_insert(char *range) {
	RANGE rr;
	int j,cwid,cw,rwid,rw;
	if (!range_def(range,&rr)) { fner("Error in range {%s} \n",range); return;}

	range_next(&rr);
	cw = rr.c2-rr.c1+1;
	cwid = max_x-rr.c1+1;
	rw = rr.r2-rr.r1+1;
	rwid = max_y-rr.r1+1;
	for (;;) {
		if (rr.colfirst) {
			for (j=cwid-1;j>=0; j--) {
			  copy_cell(rr.col+j,rr.row,rr.col+j+cw,rr.row);
			  show_ifcell(rr.col+j+cw,rr.row);
			}
			for (j=0;j<cw;j++) {
			  clear_cell(rr.c1+j,rr.row);
			  show_ifcell(rr.c1+j,rr.row);
			}
			rr.row++;
			if (rr.row>rr.r2) break;
		} else {
			for (j=rwid-1;j>=0; j--) {
			  copy_cell(rr.col,rr.row+j,rr.col,rr.row+j+rw);
			  show_ifcell(rr.col,rr.row+j+rw);
			}
			for (j=0;j<rw;j++) {
			  clear_cell(rr.col,rr.r1+j);
			  show_ifcell(rr.col,rr.r1+j);
			}
			rr.col++;
			if (rr.col>rr.c2) break;
		}
	}
}

void cmd_data(char *range) {
	RANGE rr;
	static char ans[155];
	int cmd;
	if (!range_def(range,&rr)) {
		fner("Invalid range given (%s)%d \n",range,strlen(range));
		return;
	}
	for (;;) {
		range_next(&rr);
		mjl_flush();
xxx:        set_newxy(rr.col,rr.row);
		if (!iserr) fner("Press ^Z or ESC when data entry finished\n");
		read_command(&cmd,ans,"DATA% ");
		if (cmd==eescape || cmd==equit) break;
		if (cmd!=0) {
			do_arrow(cmd);
			rr.col = curx; rr.row = cury;
			goto xxx;
		} else {
		  if (gle_isnumber(ans)) {
			set_cell(curx,cury,atof(ans));
		  } else {
			set_scell(curx,cury,ans);
		  }
		  show_cellwide(curx,cury);
		}
	}
}

bool swap_def(char *s, RANGE *r) {
	char *s1;
	char rr,cc;

	strip_colon(s);

	r->c1 = r->c2 = r->r1 = r->r2 = r->row = r->col = 0;

	if (*s=='C') {cc='C'; rr='R'; r->colfirst = true;}
	else if (*s=='R') {cc='R'; rr='C'; r->colfirst = false;}
	else return false;
/*  c1   c1r1r10   c3c4r7 c3r7  c3c4r1r2 |   r1r2c1c3   c3r7  r2*/

	s1 = range_int(s,&(r->c1));
	if (*s1==cc) {
		s1 = range_int(s1,&r->c2);
	}
	if (*s1==rr) {
		s1 = range_int(s1,&r->r1);
		if (*s1==cc) {
			s1 = range_int(s1,&r->c2);
		}
		if (*s1==rr) {
			s1 = range_int(s1,&r->r2);
		}
		if (*s1==cc) {
			s1 = range_int(s1,&r->c2);
		}
	}
	if (!r->colfirst) {
		swapint(&r->c1,&r->r1);
		swapint(&r->c2,&r->r2);
	}
	if (*s1!=0) return false;
	return true;
}

void cmd_swap(char *range) {
	RANGE rr;
	int i;
	if (!swap_def(range,&rr)) {
		fner("Invalid range given (%s) expecting c1c2 or r1r2 \n",range);
		return;
	}
	if (rr.colfirst) {
	  if (rr.c1==0 || rr.c2==0) {fner("Invalid range, expected c1c2\n"); return;}
	  for (i=1;i<=max_y;i++) {
		swap_cell(rr.c1,i,rr.c2,i);
		show_ifcell(rr.c1,i); show_ifcell(rr.c2,i);
	  }
	} else {
	  if (rr.r1==0 || rr.r2==0) {fner("Invalid range, expected r1r2\n"); return;}
	  for (i=1;i<=max_x;i++) {
		swap_cell(i,rr.r1,i,rr.r2);
		show_ifcell(i,rr.r1); show_ifcell(i,rr.r2);
	  }
	}
}

FILE *logfile;

void log_open(char *fname) {
	add_dotman(fname);
	gle_strlwr(fname);
	logfile = fopen(fname,"w");
	if (logfile==NULL) {
		fner("Could not open (%s) \n",fname);
		return;
	}
	islogging = true;
}

void log_write(char *s) {
	fprintf(logfile,"%s\n",s);
}

void log_close() {
	if (islogging) fclose(logfile);
	islogging = false;
}

void add_dotman(char *s) {
	if (strstr(s,".")==0) strcat(s,".man");
}

FILE *atfile;

void at_open(char *fname) {
	add_dotman(fname);
	gle_strlwr(fname);
	atfile = fopen(fname,"r");
	if (atfile==NULL) {
		fner("Could not open (%s) \n",fname);
		return;
	}
	isating = true;
}

bool at_read(char *s) {
	s[0] = 0;
	if (feof(atfile)) {
		fclose(atfile); atfile = NULL; isating = false; return false;
	}
	s[0] = 0;
	if (fgets(s,200,atfile)==NULL) return false;
	return true;
}

void strip_colon(char *s) {
	char *ss;
	ss = s;
	for (;*s!=0;s++) {
		if (*s!=':') *ss++ = *s;
	}
	*ss++ = 0;
}

void cmd_parsum(char *range, char *dest) {
	RANGE rr,dd;
	double total=0;
	if (!range_def(range,&rr)) { fner("Invalid range given (%s) \n",range); return;}
	if (!range_def(dest,&dd)) { fner("Invalid destination given (%s) \n",range); return;}
	for (;range_next(&rr);) {
		range_next(&dd);
		total = total + vcell(rr.col,rr.row);
		set_cell(dd.col,dd.row,total);
		show_ifcell(dd.col,dd.row);
	}
}

void cmd_sum(char *range) {
	RANGE rr,rrsave;
	double v,var,stddev;
	char *s;
	double total=0;
	int32 ntot=0;
	if (!range_def(range,&rr)) { fner("Invalid range given (%s) \n",range); return;}
	rrsave = rr;
	for (;range_next(&rr);) {
		get_cellboth(rr.col,rr.row,&v,&s);
		if (s==NULL) {
			total = total + v;
			ntot++;
		}
	}
	if (ntot>0) {
		rangestd(&rrsave,total/ntot,ntot,&var,&stddev);
		printmess("Total %g, Avg %g, sd %g, var %g, Cells %ld \n",total,total/ntot,stddev,var,ntot);
	} else fner("No values in range\n");
}

void rangestd(RANGE *rr, double mean, int32 numrow, double *variance, double *stddev) {
   char *s;
   double xsqr,v;

   xsqr = 0.0;
   if (numrow<2) return;
   for (;range_next(rr);) {
	  get_cellboth(rr->col,rr->row,&v,&s);
	  if (s==NULL) {
	xsqr = xsqr + v*v;
	  }
   }
   (*variance) = (xsqr - numrow * mean*mean) / (numrow - 1);
   (*stddev) = sqrt(fabs((*variance)));
}

void cmd_clear(char *range) {
	RANGE rr;
	if (!range_def(range,&rr)) { fner("Invalid range given (%s) \n",range); return;}
	for (;range_next(&rr);) {
		clear_cell(rr.col,rr.row);
		show_ifcell(rr.col,rr.row);
	}
}

void cmd_fit(char *range) {
	RANGE rr,rrsave;
	char *s;
	double v,a,b,siga,sigb,chi2,q;
	if (!range_def(range,&rr)) { fner("Invalid range given (%s) \n",range); return;}
	rr.c2 = rr.c1;
	rrsave = rr;
	int ntot = 0;
	for (;range_next(&rr);) {
		get_cellboth(rr.col,rr.row,&v,&s);
		if (s==NULL) {
			ntot++;
		}
	}
	if (ntot>0) {
	  fitlsq(&rrsave,ntot,&a,&b,&siga,&sigb,&chi2,&q);
	  printmess("y = %g + b*%g    siga=%g sigb=%g  chi2=%g\n",a,b,siga,sigb,chi2);
	} else fner("No values in range\n");
}

static double sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)

void fitlsq(RANGE *rrr,int ndata, double *a, double *b, double *siga, double *sigb, double *chi2, double *q) {
	RANGE  rr;
	double x,y,sx=0,sy=0,st2=0,t,ss,sigdat,sxoss;
	char *s;
	*b = 0.0;
	for (rr = *rrr;range_next(&rr);) {
		get_cellboth(rr.col,rr.row,&x,&s);
		get_cellboth(rr.col+1,rr.row,&y,&s);
		if (s==NULL) {
			sx += x;
			sy += y;
		}
	}
	ss = ndata;
	sxoss = sx/ss;
	for (rr = *rrr;range_next(&rr);) {
		get_cellboth(rr.col,rr.row,&x,&s);
		get_cellboth(rr.col+1,rr.row,&y,&s);
		if (s==NULL) {
			t=x-sxoss;
			st2 += t*t;
			*b += t*y;
		}
	}
	*b /= st2;
	*a = (sy-sx*(*b))/ss;
	*siga = sqrt((1.0+sx*sx/(ss*st2))/ss);
	*sigb = sqrt(1.0/st2);
	*chi2=0.0;
	for (rr = *rrr;range_next(&rr);) {
		get_cellboth(rr.col,rr.row,&x,&s);
		get_cellboth(rr.col+1,rr.row,&y,&s);
		if (s==NULL) {
			*chi2 += SQR(y-(*a)-(*b)*x);
		}
	}
	*q = 1.0;
	sigdat = sqrt((*chi2)/(ndata-2));
	*siga *= sigdat;
	*sigb *= sigdat;
}
