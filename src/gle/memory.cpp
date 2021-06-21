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
#include "file_io.h"
#include "gprint.h"

extern char errgle[];

char *sdup(const char *s) {
	char *v;
	v = (char*) malloc((strlen(s)+1) * sizeof(char));
	strcpy(v,s);
	return v;
}

void myfree(void *p) {
	myfrees(p,"UNKNOWN");
}

void myfrees(void *p, const char *s) {
	free(p);
}

void *myallocn(int nitems,int size) {
	return myallocz(nitems*size);
}

void *myalloc(int size) {
	if (size == 0) {
		sprintf(errgle,"\nError, attempt to allocate ZERO memory \n");
		gle_abort(errgle);
	}
	void* p = malloc(size+sizeof(int)*2);
	if (p == NULL) {
		p = malloc(size+sizeof(int)*2);
		if (p == NULL) {
			sprintf(errgle,"\nMemory allocation failure (size %d)\n", size);
			gle_abort(errgle);
		}
	}
	return p;
}

void *myallocz(int size) {
	static void *p;
	if (size==0) {
		sprintf(errgle,"\nError, attempt to allocate ZERO memory \n");
		gle_abort(errgle);
	}
	p = calloc(1,size+sizeof(int)*2);
	if (p == NULL) {
		p = calloc(1,size+sizeof(int)*2);
		if (p == NULL) {
			sprintf(errgle,"\nMemory allocation failure (size %d)\n", size);
			gle_abort(errgle);
		}
	}
	return p;
}

int mem_total() {
	return 0;
}

int mem_worst() {
	return 0;
}

char errgle[90];
int dont_clear;

void gle_abort(const char *s) {
	printf("%s",s);
	exit(1);
}
