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

char *un_quote();
const char *ns[3] = {"Nothing","Number","String"};
extern int gle_debug;

#include "all.h"

void token_norm(void);

#define tok(n)  tk[n]
#define abort goto fatal_err

/*---------------------------------------------------------------------------*/
/* bin = 10..29, binstr = 30..49, fn= 60...139, userfn=200..nnn */

#define stack_bin(i,p) 	stack_op(pcode,plen,stk,stkp,&nstk,i-10+(last_typ*20),p+curpri)
#define stack_fn(i) 	stack_op(pcode,plen,stk,stkp,&nstk,i,10+curpri)
#define dbg if ((gle_debug & 4)>0)

/*---------------------------------------------------------------------------*/
/* Input is token array, and pointer to current point, output is pcode */

/* typedef struct op_key (*OPKEY)[100];  */
/* typedef char (*(*TOKENS)[500]); */

int zpolish(TOKENS tk,int *ntok,int *curtok,char *pcode,int *plen,int *rtype);

void polish(char *expr,char *pcode,int *plen,int *rtype) {
static char *tk[500];
static char tkbuff[500];
static int ntk,ct;
const char *space_str=" ";
	static char buff[50];
	static int start_token;
	double xxx;
	int idx,ret,np,*plist,saveplen,term_bracket;
	int curpri=0;
	int i,v,p,savelen,isa_string,not_string,last_typ;
	int nstk=0,stk[50],stkp[50];	/* stack for operators */
	int unary=1;		/* binary or unary operation expected */
	int ln;			/* length of current token */
	char *cts;		/* current token */
	/* last_typ, 1=number,2=string */

	*plen = 0;
	if (tk[400]==NULL) for (i=0;i<500;i++) tk[i] = (char*)space_str;
	isa_string = false;
	not_string = false;
	if (*rtype==1) not_string = true;
	/* if (*rtype==2) isa_string = true; */
	*plen = *plen*4;	/* change into byte count */
	if (*rtype>0) term_bracket = true;
	last_typ = *rtype;
	saveplen = *plen;

	add_i(pcode,plen,1);	/* expression follows */
	savelen = *plen;	/* Used to set actual length at end */
	add_i(pcode,plen,0);	/* Length of expression */
	dbg gprint("====Start of expression {%s} \n",expr);
	if (strlen(expr)==0) {gprint("Zero length expression\n"); return;}
	if (!start_token) {
		ntk = 0; ct=1;
		token_norm();
		token(expr,tk,&ntk,tkbuff);
		token_space();
	}
	for (;;) {
	  cts = tok(ct);
	  dbg gprint("First word token=%d via (1=unary %d) cts {%s} %d \n "
		,ct,unary,cts,(int)strlen(cts));
	  ln = strlen(cts);
 	  switch (unary) {
	  case 1:  /* a unary operator, or function, or number or variable */
		if (ln==1 && (*cts=='E')) goto notnumber;
		if (ln==1 && (*cts=='-')) goto notnumber;
		if (gle_isnumber(cts))  {
evalagain:	        dbg gprint("Found number {%s}\n",cts);
			if (manip_lastchar(cts,'E'))  {
				strcpy(buff,cts);
				strcat(buff,tok(++ct));
				if (*tok(ct)=='-' || *tok(ct)=='+') {
					strcat(buff,tok(++ct));
				}
				tok(ct) = buff;
				cts = tok(ct);
				goto evalagain;
			}
			xxx = atof(cts);
			add_f(pcode,plen,xxx);
			if (last_typ==2) gprint("Expecting string {%s} \n",cts);
			last_typ = 1;
			unary=2; break;
		}
notnumber:	/* NOT a number, Is it a built in function */
		/* int idx,ret,np,*plist; */
		find_un(cts,&idx,&ret,&np,&plist);	/* 1,2 = +,- */
		if (idx>63) {
		  dbg gprint("Found built in function \n");
			if (*tok(++ct)!='(') {
				gprint("Expecting left bracket after functsion name");
				abort;
			}
			{
			 char fcode[400];
			 int flen,vtype,nparam=0;
			 if (*tok(ct+1)!=')') {
			   while (*tok(ct)!=')') {
				nparam++;
				vtype = *(plist+nparam-1);
				flen = 0;
				(ct)++;
				start_token = true;
				polish((char*)"xx",fcode,&flen,&vtype);
				start_token = false;
				flen = flen * 4;
				if (nparam>np) {gprint("Too many parameters got=%d want=%d \n",nparam,np);abort;}
				if (vtype==0) abort;
				add_pcode(pcode,plen,fcode,&flen);
			   }
			 } else {
				ct++;
			 }
			}
			if (last_typ==(3-ret)) {
				gprint("Function of wrong type Expecting {%s} \n",ns[ret]);
				abort;
			}
			last_typ = ret;
			add_fn(pcode,plen,idx);
			unary = 2; break;
		} else if (idx>0) {
			stack_fn(idx);
			unary=1; break;
		}

		 /* Is it a user-defined function, identical code too above. */
		  sub_find(cts,&idx,&ret,&np,&plist);	/* 1,2 = +,- */
		  if (idx>0) {
		  dbg gprint("Found user function \n");
			if (*tok(++ct)!='(') {
				gprint("Expecting left bracket after function name");
				abort;
			}
			{
			char fcode[400];
			int flen,vtype,nparam=0;
			if (*tok(ct+1)!=')') {
			 while (*tok(ct)!=')') {
				ct++;
				nparam++;
				vtype = *(plist+nparam-1);
				flen = 0;
				start_token = true;
				polish((char*)"xx",fcode,&flen,&vtype);
				start_token = false;
				flen = flen * 4;
				if (nparam>np) {gprint("Too many U parameters got=%d want=%d \n",nparam,np);abort;}
				if (vtype==0) abort;
				add_pcode(pcode,plen,fcode,&flen);
			  }
			 } else {
				ct++;
			 }
			}
			if (last_typ==(3-ret)) {
				gprint("Function of wrong type Expecting {%s} \n",ns[ret]);
				abort;
			}
			if (ret>0 && ret<3) last_typ = ret;
			add_fn(pcode,plen,idx+200);
			unary = 2; break;
		} else if (idx>0) {
			stack_fn(idx);
			unary=1; break;
		}


		/* Is it a 'known' variable */
		var_find(cts,&v,&ret);
		if (v>=0) {
			dbg gprint("Found variable %d \n",v);
			if (last_typ==(3-ret)) {
				gprint("Expecting {%s} \n",ns[last_typ]);
				abort;
			}
			last_typ=ret;
			if (ret==2) add_strvar(pcode,plen,v);
			else add_var(pcode,plen,v);
			unary=2; break;
		}
		/* Is it a atring */
		if (*cts=='"') {
		  dbg gprint("Found string \n");
			if (last_typ==1) {
				gprint("Expecting number {%s} \n",cts);
				abort;
			}
			last_typ = 2;
			add_string(pcode,plen,un_quote(cts));	/* remove quotes */
			unary = 2; break;
		}
		if (*cts=='(') { curpri = curpri + 100; break; }
		if (*cts==')') {
			if (curpri>0) {
				curpri = curpri - 100;
				unary = 2; break;
			}
			gprint("Too many right brackets found in exp \n");

		}
		/* must be unquoted string, unless a binary operator
		was found, in which case it is an undelcared variable */
		if (not_string) {
			dbg gprint("Found un-initialized variable {%s} /n",cts);
			var_add(cts,&v,&ret);
			last_typ=ret;
			add_var(pcode,plen,v) ;
			unary=2;
			break;
		}
		last_typ = 2;
		dbg printf("Unquoted string (%s) \n",cts);
		add_string(pcode,plen,un_quote(cts));	/* remove quotes */
		isa_string = true;
		unary = 2; break;
		isa_string = true;
		add_string(pcode,plen,cts);
		unary = 2;  /* Expecting end of expression next !! */
		break;
	  case 2:  /* a binary operator, or space, or end of line */
		if (ct>ntk || *cts==' ' || *cts==',' ) {
			goto end_expression;
		}
		if (*cts==')' && curpri==0) {
			goto end_expression;
		}
/* MIGHT (gives error with a$ = b$+c$) */
		if (isa_string) {
			gprint("Expression contained unquoted string\n");
			abort;
		}

		not_string = true;
		/* Binary operators, +,-,*,/,^,<,>,<=,>=,.and.,.or. */
		switch (*cts) {
		  case '+' : v = 1; p=2; break;
		  case '-' : v = 2; p=2; break;
		  case '*' : v = 3; p=3; break;
		  case '/' : v = 4; p=3; break;
		  case '^' : v = 5; p=4; break;
		  case '=' : v = 6; p=1; break;
		  case '<' : v = 7; p=1;
			if (*tok(ct+1)=='=') {v=8;++ct;} break;
		  case '>' : v = 9;p=1;
			if (*tok(ct+1)=='=') {v=10;++ct;} break;
		  case '.' : p=1;
			if (strcmp(cts,".AND.")) {v=11;} break;
			if (strcmp(cts,".OR.")) {v=12;} break;
	 	  default : v = 0 ; break;
		}
		if (v>0) {
			if (last_typ<1 || last_typ > 3) last_typ = 1;
			stack_bin(v,p);
			dbg gprint("Found binary operator \n");
			unary=1; break;
		}
		if (*cts==')') {
			if (curpri>0) {
				curpri = curpri - 100;
				unary = 2; break;
			}
			if (term_bracket!=true) {
				gprint("Too many right brackets, expecting binary operator \n");
				abort;
			}
			goto end_expression;
		}
	  }
	  if (++ct>ntk) { goto end_expression; }
/*	gprint("Next token is {%s} \n",tok(ct)); */
	}
end_expression:
	if (*tok(ct)==' ') (ct)++;
	dbg gprint("Got expression , curtok=%d {%s} \n",ct,tok(ct));
	*rtype = last_typ;
	  dbg gprint("Found END OF EXPRESSION \n");
	if (!start_token) if (curpri!=0) {gprint((char*)"Missing right brackets");}
	/* Pop everything off the stack */
	for (i=nstk;i>0;i--) {
		dbg gprint("Adding left over operators  I = %d  op=%d \n",i,stk[i]);
		add_i(pcode,plen,stk[i]);
	}
	* ((int32 *) (pcode+savelen)) = (*plen - savelen)/4-1;  /* Set length of expression */
	*plen = *plen/4;	/* change back to int count */
return;
fatal_err:
	gprint("Aborting expression parsing. \n");
	*plen = saveplen;
	*rtype = 0;
}

/*------------------------------------------------------------------*/
/* append fcode to pcode */

void add_pcode(char *pcode,int *plen,char *fcode,int *flen) {
	char *p;
	p = pcode + *plen;
	memcpy(p,fcode,*flen);
	*plen = *plen + *flen;
}

void add_i(char *pcode,int *plen,int32 i) {
	int32 *p;
	p = (int32 *) (pcode + *plen);
	*p = i;
	*plen += 4;
}

void add_f(char *pcode,int *plen,double f) {
	union { double d ; int32 l[2]; short s[4]; } both;
	both.d = f;
	add_i(pcode,plen,2);
	add_i(pcode,plen,both.l[0]);
	add_i(pcode,plen,both.l[1]);
}

void add_var(char *pcode,int *plen,int i) {
	add_i(pcode,plen,3);
	add_i(pcode,plen,i);
}

void add_strvar(char *pcode,int *plen,int i) {
	add_i(pcode,plen,4);
	add_i(pcode,plen,i);
}

void add_fn(char *pcode, int *plen, int i) {
	add_i(pcode,plen,i);
	dbg gprint(" add Function %d \n",i);
}

void add_string(char *pcode, int *plen, char *s) {
	char *p;
	int sl;
	dbg gprint("adding string {%s} \n",s);
	add_i(pcode,plen,5);
	sl = strlen(s)+1;
	p = pcode + *plen;
	sl = ((sl + 3) & 0xfffc);
 	strncpy(p,s,sl);
	*plen = *plen + sl;
}

void stack_op(char *pcode, int *plen, int stk[], int stkp[], int *nstk,  int i, int p) {
	dbg gprint("Stack oper %d priority %d \n",i,p);
	while (p<=stkp[*nstk] &&(*nstk)>0) {
		dbg gprint("ADDING oper stack = %d  oper=%d \n",*nstk,stk[(*nstk)]);
		add_i(pcode,plen,stk[(*nstk)--]);
	}
	stk[++(*nstk)] = i;
	stkp[*nstk] = p;
}

bool gle_isnumber(char *s) {
	while (*s!='\0') {
		if (isdigit(*s) || *s=='.' || *s=='E' ) s++;
		else  return false;
	}
	return true;
}

char manip_lastchar(const char *s, char c) {
	while ((*s)!='\0') {s++;}
	return *(--s)==c;
}


