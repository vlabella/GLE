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

/*----------------------------------------------------------------------*/
/* This is the font builder for GLE, it runs completely independently	*/
/* and only understands the following commands.							*/
/*1 	AMOVE x y														*/
/*2 	ALINE x y 														*/
/*3 	BEZIER x y x y x y												*/
/*4 	CLOSEPATH														*/
/*5 	FILL															*/
/*6 	STROKE															*/
/*8 	SET LWIDTH w													*/
/*7 	FILLWHITE   (no newpath )										*/
/*15	DFONT ENDCHAR 													*/
/* 	DFONT CHAR=nn  or CHAR="A" 											*/
/* 	DFONT DESIGN=v 														*/
/*----------------------------------------------------------------------*/
#define GPRINTDEF

#include "../gle/basicconf.h"
#include "../gle/mem_limits.h"
#include "../gle/token.h"
#include "../gle/cutils.h"
#include "../gle/file_io.h"

#define gprint printf

int debugit=0;
#define dbg if (debugit==1)
double design,gx,gy;
int thischar;
char inbuff[200],zzz[200];
char source[20];
char tk[TOKEN_LENGTH][TOKEN_WIDTH];
char tkbuff[500];
unsigned char *outbuff;
int outpnt[256];
int nbig,nsmall;
int cx,cy;
int nout;

void sendp(int p);
void sendx(double x);
void sendxy(double x, double y);
void sendxyabs(double x, double y);
void sendi(int ix);
void do_line(void);

int main(int argc, char **argv) {
	int ntok;
	int i;
	FILE *fptr;
	char *s;
	char space_str[] = " ";
	char fname[80];
	char outfile[80];
	design = 1;
	token_equal();
	strcpy(fname,*(++argv));
    s = strchr(fname,'.');
    if (s!=NULL) *s = 0;
	strcpy(outfile,fname);
	if (strchr(fname,':')!=NULL) strcpy(outfile,strchr(fname,':')+1);
	strcat(outfile,".fve");
	strcat(fname,".gle");
	printf("[%s]==>[%s]\n",fname,outfile);
	outbuff = (unsigned char*) malloc(64000);
	GLEFileIO fout;
	fout.open(outfile, WRITE_BIN);
	if (!fout.isOpen()) perror("Could not open output file \n");
	fptr = fopen(fname,"r");
	if (fptr==NULL) perror("Could not open input file \n");
	for (i=0;i<TOKEN_LENGTH;i++) strcpy(tk[i],space_str);
	if (fgets(inbuff,200,fptr) != 0) {
		while (!feof(fptr)) {
			#ifdef __UNIX__
				// remove DOS CR if present
				i = strlen(inbuff);
				if (i >= 2 && inbuff[i-2] == '\r') {
					inbuff[i-2] = '\n';
					inbuff[i-1] = '\0';
				}
			#endif
	/* 		printf("Source | %s",inbuff); */
			token(inbuff,tk,&ntok,tkbuff);
			do_line();
			if (fgets(inbuff,200,fptr) == 0) {
				break;
			}
		}
	}
	//printf(" nbig %d nsmall %d nbytes %d\n",nbig,nsmall,nout);
/*	for (i=0;i<100;i++) if (outpnt[i]>0) dbg printf("index %d %d ",i,outpnt[i]);*/
	dbg printf("\n\n");
	dbg printf("\n nbig %d nsmall %d nbytes %d\n\n",nbig,nsmall,nout);
	for (i=0;i<100;i++) dbg printf("%d ",*(outbuff+i));
	dbg printf("\n nbig %d nsmall %d nbytes %d\n\n",nbig,nsmall,nout);
	outpnt[0] = nout;
	fout.fwrite(outpnt, sizeof(i), 256);
	fout.fwrite(outbuff, 1, nout);
	fout.close();
	fclose(fptr);
	return 0;
}
/*--------------------------------------------------------------------------*/
#define skipspace if (*tk[ct]==' ') ct++;
void do_line()
{
	int ct;

	ct = 1;
	if (str_i_equals(tk[ct],"ALINE")) 	{
		sendp(2);
		sendxy(gx=atof(tk[2]),gy=atof(tk[3]));
	} else if (str_i_equals(tk[ct],"RLINE")) 	{
		sendp(2);
		gx = gx + atof(tk[2]);
		gy = gy + atof(tk[3]);
		sendxy(gx,gy);
	} else if (str_i_equals(tk[ct],"RMOVE")) 	{
		sendp(1);
		gx = gx + atof(tk[2]);
		gy = gy + atof(tk[3]);
		sendxyabs(gx,gy);
	} else if (str_i_equals(tk[ct],"BEZIER")) {
		sendp(3);
		sendxy(atof(tk[2]),atof(tk[3]));
		sendxy(atof(tk[4]),atof(tk[5]));
		sendxy(gx=atof(tk[6]),gy=atof(tk[7]));
	} else if (str_i_equals(tk[ct],"AMOVE")) {
		sendp(1);
		sendxyabs(gx=atof(tk[2]),gy=atof(tk[3]));
	} else if (str_i_equals(tk[ct],"ASETPOS")) {
		sendp(9);
		sendxyabs(gx=atof(tk[2]),gy=atof(tk[3]));
	} else if (str_i_equals(tk[ct],"CIRCLE")) {
		sendp(10);
		sendx(atof(tk[2]));
	} else if (str_i_equals(tk[ct],"CLOSEPATH")) 	sendp(4);
	else if (str_i_equals(tk[ct],"FILL")) 	sendp(5);
	else if (str_i_equals(tk[ct],"FILLWHITE")) sendp(7);
	else if (str_i_equals(tk[ct],"STROKE"))	sendp(6);
	else if (strcmp(tk[ct],"\n")==0)	;
	else if (str_i_equals(tk[ct],"FBY"))	;
	else if (str_i_equals(tk[ct],"NEWPATH"))	;
	else if (str_i_equals(tk[ct],"\n"))	;
	else if (strcmp(tk[ct],"!")==0)		;
	else if (str_i_equals(tk[ct],"SET"))	{
		if (str_i_equals(tk[2],"LWIDTH")) {
			sendp(8); sendx(atof(tk[3]));
		}
	}
	else if (str_i_equals(tk[ct],"DFONT")) {
		ct+=1;
		if (str_i_equals(tk[ct],"DESIGN"))	 {
			ct++;ct++;
			design = atof(tk[ct]);
			//printf(" %g",design);
		} else if (str_i_equals(tk[ct],"CHAR")) {
			ct++;ct++;
			if (*tk[ct]=='"') thischar = *(tk[ct]+1);
			else thischar = atoi(tk[ct]);
			if (thischar==0) thischar=254;
			outpnt[thischar] = nout;
			dbg printf("This char %d {%s} nout %d \n",thischar,tk[ct],nout);
		} else if (str_i_equals(tk[ct],"ENDCHAR")) sendp(15) ;
		else if (str_i_equals(tk[ct],"WID")) ;
		else if (str_i_equals(tk[ct],"FBY")) ;
		else dbg printf("DFONT unknown command {%s} \n",tk[ct]);
	}
	else dbg printf("Unrecoginzed FONT BUILD command {%s} \n",tk[ct]);
	ct++;
}
void sendp(int p)
{
	*(outbuff+nout++) = p;
}
int scl(double x);
int scl(double x)
{
 	return (int)(1000*x/design);
}
void sendxyabs(double x, double y)
{
	cx = scl(x);
	cy = scl(y);
	sendi(cx); sendi(cy);
/* 	printf("Move cx cy %d %d %g %g \n",cx,cy,x,y); */
}
void sendxy(double x, double y)
{
/* 	printf("rove cx cy %d %d %d %d %g %g \n",cx,cy,scl(x),scl(y),x,y); */
	sendi(scl(x)-cx); sendi(scl(y)-cy);
	cx = scl(x);
	cy = scl(y);
}
void sendi(int ix)
{
	union jjj {char a[2]; short b;} both;
/*	if (thischar=='i') printf("ix %d \n",ix);
*/
	if (ix>120 || ix <-120) {
		both.b = ix;
		*(outbuff+nout++) = 127;
		*(outbuff+nout++) = both.a[0];
		*(outbuff+nout++) = both.a[1];
		nbig++;
	} else { *(outbuff+nout++) = ix;	 nsmall++; }
}
void sendx(double x)
{
	sendi(scl(x));
}


