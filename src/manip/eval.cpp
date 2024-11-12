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

/*---------------------------------------------------------------------------*/

#include "all.h"
#include "mygraph.h"
#include "eval.h"

typedef union {
	struct {unsigned char r,g,b,f;} rgb ;
	int32 l;
} colortyp;
colortyp colvar;

char *eval_str();
int pass_marker(char *s);

/*---------------------------------------------------------------------------*/
/* bin = 10..29, binstr = 30..49, fn= 60...139, userfn=200..nnn */
/* pcode:,  1=exp,len  2=double,val 3=var,int32 4,string_var, 5=string,.../0 */
/*---------------------------------------------------------------------------*/
/* Input is exp-pcode, output is number or string */

const char *binop[] = { "", "+", "-", "*", "/", "^", "=", "<", "<=", ">", ">=", "<>", ".AND.", ".OR." };

struct keyw {const char *word; int index; int ret,np,p[5]; } ;
extern struct keyw keywfn[] ;

double stk[60];
int stk_var[100];
char *stk_str[100];
int stk_strlen[100];
char sbuf[512];
char sbuf2[112];
int nstk=0;
extern int gle_debug;
#define dbg if ((gle_debug & 2)>0)

void eval(int32 *pcode,int *cp,double *oval,char *ostr,int *otyp) {
		/* a pointer to the pcode to execute 		*/
		/* Current point in this line of pcode 		*/
		/* place to put result number 			*/
		/* place to put result string 			*/
		/* place to put result type, 1=num, 2=str 	*/
	union {double d; int32 l[2];} both;
	int plen,i,c;
	time_t today;
	double xx;

	dbg gprint("%%EXP-START, Current point in eval %d %d \n",*cp,(int) *(pcode+(*cp)));
	dbg for (i=0;i<10;i++) gprint("%ld ",*(pcode+i));
	dbg gprint("\n");
	dbg debug_polish(pcode,cp);
	if (*(pcode+(*cp))==8) {	/*  Single constant  */
		both.l[0] = *(pcode+ ++(*cp));
		both.l[1] = 0;
		dbg gprint("Constant %ld \n",both.l[0]);
		memcpy(oval,&both.d,sizeof(both.d));
		memcpy(&both.d,oval,sizeof(both.d));
		++(*cp);
		return;
	}

	if (*(pcode+(*cp)++)!=1) {
		gprint("PCODE, Expecting expression, v=%ld cp=%d \n",*(pcode+((*cp)-1)),*cp);
		return;
	}
	plen = *(pcode+*(cp));
	dbg gprint(" plen = %d ",plen);
	if (plen>1000) gprint("Expression is suspiciously int32 %d \n",plen);
	for (c=(*cp)+1;c<=(plen+ *cp);c++) {
	  switch (*(pcode+c)) {
		/* Special commands 1..9  ------------------------------- */
		case 1:	/* Start of another expression (function param) */
			c++;	/* skip over exp length */
			break;
		case 2: /* doubleing point number follows */
			*otyp = 1;
			both.l[0] = *(pcode+(++c));
			both.l[1] = *(pcode+(++c));
			stk[++nstk] =  both.d;
 			dbg gprint("Got double %f %d %f \n",stk[nstk],nstk,(double)*(pcode+(c)));
			break;
		case 3: /* doubleing_point variable number follows */
			*otyp = 1;
			var_get(*(pcode+(++c)),&xx);
			dbg gprint("Got variable value %ld %f \n",*(pcode+(c)),xx);
			stk[++nstk] = xx;
			break;
		case 4: /* string variable number follows */
			*otyp = 2;
			var_getstr(*(pcode+(++c)),sbuf); nstk++;
			if (stk_str[nstk]!=NULL)  myfree(stk_str[nstk]);
			stk_str[nstk] = sdup(sbuf);
 			break;
		case 5: /* Null terminated string follows (int32 aligned) */
			*otyp = 2;
			c++;nstk++;
			strcpy(sbuf,eval_str(pcode,&c));
			if (stk_str[nstk]!=NULL)  myfree(stk_str[nstk]);
			stk_str[nstk] = sdup(sbuf);
			break;
		/* Numeric Binary operators 10..29 ----------------------- */
		case 11:  /* + */
			nstk--;
			stk[nstk] = stk[nstk+1] + stk[nstk];
			break;
		case 12:  /* - */
			stk[nstk-1] = stk[nstk-1] - stk[nstk];
			nstk--;
			break;
		case 13:  /* * */
			stk[nstk-1] = stk[nstk-1] * stk[nstk];
			nstk--;
			break;
		case 14:  /* / */
			if (stk[nstk]==0) {
				gprint("Divide by zero %g %g \n",
					stk[nstk-1],stk[nstk]);
			} else {
				stk[nstk-1] = stk[nstk-1] / stk[nstk];
			}
			nstk--;
			break;
		case 15:  /* ^ */
			stk[nstk-1] = pow(stk[nstk-1],stk[nstk]);
			nstk--;
			break;
		case 16:  /* = */
			nstk--;
			if (stk[nstk] == stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 17:  /* <   */
			nstk--;
			if (stk[nstk] < stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 18:  /* <=  */
			nstk--;
			if (stk[nstk] <= stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 19:  /* >   */
			nstk--;
			if (stk[nstk] > stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 20:  /* >=  */
			nstk--;
			if (stk[nstk] >= stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 21:  /*  <>  */
			nstk--;
			if (stk[nstk] != stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 22:  /* .AND.  */
			nstk--;
			if (stk[nstk] && stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		case 23:  /* .OR.   */
			nstk--;
			if (stk[nstk] || stk[nstk+1]) stk[nstk]=true;
			else stk[nstk]=false;
			break;
		/* String Binary operators 30..49 ----------------------- */
		case 31:  /* + */
			*otyp = 2;
			nstk--;
			if (stk_str[nstk]!=NULL) strcpy(sbuf,stk_str[nstk]);
			if (stk_str[nstk+1]!=NULL) strcat(sbuf,stk_str[nstk+1]);
			if (stk_str[nstk] != NULL) myfree(stk_str[nstk]);
			stk_str[nstk] = sdup(sbuf);
			break;
		case 32:  /* - */
			break;
		case 33:  /* * */
			break;
		case 34:  /* / */
			break;
		case 35:  /* ^ */
			break;
		case 36:  /* = */
			break;
		case 37:  /* <   */
			break;
		case 38:  /* <=  */
			break;
		case 39:  /* >   */
			break;
		case 40:  /* >=  */
			break;
		case 41:  /* .AND.  */
			break;
		case 42:  /* .OR.   */
			break;

		/* Built in functions 60..199 ----------------------------- */
		case f_plus: /* unary plus */
			break;
		case f_minus: /* unary minus */
			stk[nstk] = -stk[nstk];
			break;
		case f_abs: /* abs */
			stk[nstk] = fabs(stk[nstk]);
			break;
		case f_atn: /* atn */
			if (stk[nstk]<0) stk[nstk] = -atan(-stk[nstk]);
			else stk[nstk] = atan(stk[nstk]);
			break;
		case f_cell: /* cell(x,y) */
   			get_cell((int)stk[nstk-1],(int)stk[nstk],&stk[nstk-1]);
   			nstk--;
   			break;
		case f_miss: /* miss(x,y) */
			nstk--;
   			if (strcmp(scell((int)stk[nstk],(int)stk[nstk+1]),"-")==0) {
   				stk[nstk]=true;
   			} else {
   				stk[nstk]=false;
   			}
   			break;
		case f_cos: /* cos */
			stk[nstk] = cos(stk[nstk]);
			break;
		case f_date: /* date$ */
			*otyp = 2;
			time(&today);
			strcpy(sbuf2,ctime(&today));
			strcpy(sbuf,sbuf2);
			strcpy(sbuf+11,sbuf2+20);
			sbuf[strlen(sbuf)-1] = 0;
			setdstr(&stk_str[++nstk],sbuf);
			break;
		case f_exp: /* exp */
			stk[nstk] = exp(stk[nstk]);
			break;
		case f_fix: /* fix*/
			stk[nstk] = floor(stk[nstk]);
			break;
		case f_left: /* left$ */
			*otyp = 2;
			ncpy(sbuf,stk_str[nstk-1],(int) stk[nstk]);
			setdstr(&stk_str[--nstk],sbuf);
			break;
		case f_len: /* len */
			*otyp = 1;
			stk[nstk] = strlen(stk_str[nstk]);
			break;
		case f_log: /* log */
			stk[nstk] = log(stk[nstk]);
			break;
		case f_log10: /* log10 */
			stk[nstk] = log10(stk[nstk]);
			break;
		case f_not: /* not */
			break;
		case f_num: /* num$ */
			*otyp = 2;
			sprintf(sbuf,"%g ",stk[nstk]);
			if (stk_str[nstk] != NULL) myfree(stk_str[nstk]);
			stk_str[nstk] = sdup(sbuf);
			break;
		case f_num1: /* num1$ */
			*otyp = 2;
			sprintf(sbuf,"%g",stk[nstk]);
			if (stk_str[nstk] != NULL) myfree(stk_str[nstk]);
			stk_str[nstk] = sdup(sbuf);
			break;
		case f_pos: /* pos */
			break;
		case f_right: /* right$ */
			*otyp = 2;
			strcpy(sbuf,stk_str[nstk-1] + (int) stk[nstk] - 1);
			setdstr(&stk_str[--nstk],sbuf);
			break;
		case f_rnd: /* rnd */
			break;
		case f_seg: /* seg$ */
			*otyp = 2;
			strcpy(sbuf,stk_str[nstk-2] + (int) stk[nstk-1] - 1);
			ncpy(sbuf2,sbuf,(int)(stk[nstk] - stk[nstk-1] + 1));
			nstk-=2;
			setdstr(&stk_str[nstk],sbuf2);
			break;
		case f_sgn: /* sgn */
			break;
		case f_sin: /* sin */
			stk[nstk] = sin(stk[nstk]);
			break;
		case f_sqr: /* sqr */
			stk[nstk] = pow(stk[nstk],2.0);
			break;
		case f_sqrt: /* sqrt */
			stk[nstk] = sqrt(stk[nstk]);
			break;
		case f_tan: /* tan */
			stk[nstk] = tan(stk[nstk]);
			break;
		case f_time: /* time$ */
			*otyp = 2;
			time(&today);
			ncpy(sbuf,ctime(&today)+11,9);
			setdstr(&stk_str[++nstk],sbuf);
			break;
		case f_val: /* val */
			break;
		/* User function 200..nnn , or error */
		default:
			  /* Is it a user defined function */
			if (*(pcode+c)>200)  {
	/*			pass the address of some numbers */
	/*			pass address of variables if possible*/
				sub_call(*(pcode+c)-200,stk,stk_str,&nstk);
			}
			else gprint("Unrecognezed pcode exp prim %d at position=%d \n",(int)*(pcode+c),c);
			break;
	  }
	}
	dbg printf("RESULT ISa ==== %d [1] %f   [nstk] %f \n",nstk,stk[1],stk[nstk]);
	dbg scr_getch();
	memcpy( oval,&(stk[nstk]),sizeof(double));
	*ostr = '\0';
	if (*otyp==2) if (stk_str[nstk]!=NULL) strcpy(ostr,stk_str[nstk]);
	if (*otyp==2) dbg gprint("Evaluated string = {%s} \n",ostr);
	nstk--;
	if (nstk<0) {
 		gprint("Stack stuffed up in EVAL %d \n",nstk);
		nstk = 0;
	}
	*cp = *cp + plen + 1;
}

void debug_polish(int32 *pcode,int *zcp) {
	int32 *cp,cpval;
	int plen,c,cde;
	cpval = *zcp;
	cp = &cpval;
	if (*(pcode+(*cp)++)!=1) {
		gprint("Expecting expression, v=%d \n",(int) *(pcode+--(*cp)) );
		return;
	}
	plen = *(pcode+*(cp));
	gprint("Expression length %d current point %d \n",plen,(int) *cp);
	if (plen>1000) gprint("Expression is suspiciously int %d \n",plen);
	for (c=(*cp)+1;(c-*cp)<=plen;c++) {
	  cde = *(pcode+c);
	gprint("Code=%d ",cde);
		if (cde==0) {
			gprint("# ZERO \n");
		} else if (cde==1) {
			gprint("# Expression, length ??? \n");
			c++;
		} else if (cde==2) {
			gprint("# doubleing point number %8x \n",(int)*(pcode+(++c)));
			c++;	/* because it's a DOUBLE which is a quad word */
		} else if (cde==3) {
			gprint("# Variable \n");  c++;
		} else if (cde==4) {
			gprint("# String Variable \n"); c++;
		} else if (cde==5) {
			c++;
			gprint("# String constant {%s} \n",eval_str(pcode,&c));
		} else if (cde<29) {
			gprint("# Binary operator {%s} \n",binop[cde-10]);
		} else if (cde<49) {
			gprint("# Binary string op {%s} \n",binop[cde-30]);
		} else if (cde<200) {
			gprint("# Built in function (with salt) {%s} \n",keywfn[cde-60].word);
		} else {
			gprint("# User defined function %d \n",cde);
		}

	}
}

char *eval_str(int32 *pcode,int *plen) {
	char *s;
	int sl;
	s = (char *) (pcode+*plen);
	sl = strlen(s)+1;
	sl = ((sl + 3) & 0xfffc);
	*plen = *plen + sl/4 - 1;
	return s;
}

void setdstr(char **s,char *in) {
	if (*s != NULL) myfree(*s);
	*s = sdup(in);
}

void eval_setxy(int x, int y) {
	static int xx= -1,yy= -1,type;
	if (xx== -1) var_findadd("C",&xx,&type);
	if (yy== -1) var_findadd("R",&yy,&type);
	var_set(xx,(double) x);
	var_set(yy,(double) y);
}

void eval_setxyd(int x, int y) {
	static int xx= -1,yy= -1,type;
	if (xx== -1) var_findadd("DC",&xx,&type);
	if (yy== -1) var_findadd("DR",&yy,&type);
	var_set(xx,(double) x);
	var_set(yy,(double) y);
}
