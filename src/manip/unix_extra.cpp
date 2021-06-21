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
#include "unix_extra.h"

int trimcr(char *s);

int32 coreleft(void) {
	return 4000000l;
}

int trim_file(char *s) {
	char *t;
	t = strchr(s,']');
	if (t!=NULL) memcpy(s,t+1,strlen(t));
	t = strchr(s,';');
	if (s!=NULL) *t = 0;
	return 0;
}

char *getsymbol(char *sym) {
	static char mystr[200];
	const char *s;
	s = getenv(sym);
	if (s==NULL) s = "";
	strcpy(mystr,s);
	return mystr;
}

FILE *wilddir;

/*struct ffblk {char *ff_name;};*/

int findfirst(char *wild, struct ffblk *ffblk, int zz) {
	static char buff[200];
	int op=0;
       #ifdef aix /* AIX with xlc != POSIX */
	union wait *status;
	int p;
       #else
	int status;
	pid_t p;
       #endif


   	unlink("gledir.tmp");
	p = fork();
	if (p==-1) {perror("Unable to fork ls command \n"); return true;}
	if (p==0) {
		strcpy(buff,"ls ");
		strcat(buff,wild);
		strcat(buff,">gledir.tmp");
		execlp("sh","sh","-c", buff, NULL);
		exit(1);
	}
       #ifdef aix
	waitpid(p,status,op);
       #else
	waitpid(p,&status,op);
       #endif
	wilddir = fopen("gledir.tmp","r");
	if (wilddir==NULL) {printf("Fork ls failed \n"); return true;}
	if (feof(wilddir)) { fclose(wilddir); return true;}
	if (fgets(buff,100,wilddir) != 0) {
		trimcr(buff);
		ffblk->ff_name = buff;
	}
	return false;
}

int findnext(struct ffblk *ffblk) {
	static char buff[200];
	if (feof(wilddir)) { fclose(wilddir); return true;}
	if (fgets(buff,100,wilddir) != 0) {
		trimcr(buff);
		ffblk->ff_name = buff;
	}
	return false;
}

int trimcr(char *s)
{
	s = strchr(s,'\n');
	if (s!=NULL) *s = 0;
	return 0;
}

