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
#include "../gle/file_io.h"

FILE *jouptr;
extern int in_recover,single_step;

void init_logging(char *infile) {
	GLEFileIO jptr;
	int nbRead = 0;
	unsigned char ccc;
	static char buff[80];
	if (in_recover) {
		jouptr = fopen("manip_.j1","rb");
		if (jouptr==NULL) fner("Unable to open/read journal file manip_.j1");
		nbRead += fread(&ccc,1,1,jouptr);
		nbRead += fread(buff,1,ccc,jouptr);
		if (strlen(infile)==0) strcpy(infile,buff);
		return;
	}
	unlink("manip_.j3");
	rename("manip_.j2","manip_.j3");
	rename("manip_.j1","manip_.j2");
	jptr.open("manip_.j1","wb");
	jptr.fputc(strlen(infile));
	jptr.fwrite(infile, 1, strlen(infile));
	if (!jptr.isOpen()) fner("Unable to open journal file manip_.j1");
	jptr.close();
}

unsigned char mjl_buff[80];

void mjl_key(int c) {
	int s;
	if (in_recover) return;
	if (c==0) return;
	s = strlen((char*)mjl_buff);
	mjl_buff[s] = c;
	mjl_buff[s+1] = 0;
	if (s>40) mjl_flush();
}

void mjl_flush() {
	if (in_recover) return;
	if (strlen((char*)mjl_buff)<7) return;
	GLEFileIO fptr;
	fptr.open("manip_.j1","ab");
	if (!fptr.isOpen()) fner("Unable to append to journal file manip_.j1");
	fptr.fwrite(mjl_buff, 1, strlen((char*)mjl_buff));
	fptr.close();
	mjl_buff[0] = 0;
}

