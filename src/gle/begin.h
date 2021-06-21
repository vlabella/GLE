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

#ifndef INCLUDE_BEGIN
#define INCLUDE_BEGIN
extern int **gpcode;   /* gpcode is a pointer to an array of poiter to int */
extern int *gplen;     /* gpcode is a pointer to an array of int */
extern int ngpcode;

//#define TOKEN_LENGTH 500
//#define TOKEN_WIDTH 500
#define STRBUF_LENGTH 200
#define SRCLIN_LENGTH 1000
#define OUTBUFF_LENGTH 300
#define INBUFF_LENGTH 300
#define TKBUFF_LENGTH 9600

BEGINDEF char tk[TOKEN_LENGTH][TOKEN_WIDTH];
BEGINDEF char strbuf[STRBUF_LENGTH];
BEGINDEF char srclin[SRCLIN_LENGTH];
BEGINDEF char outbuff[OUTBUFF_LENGTH];
BEGINDEF char inbuff[OUTBUFF_LENGTH];
BEGINDEF char tkbuff[TKBUFF_LENGTH];
BEGINDEF char space_str[3];
BEGINDEF int ntk;

class GLESourceLine;

int begin_token(GLESourceLine& sline, char *srclin, TOKENS tk, int *ntk, char *outbuff, bool replaceExpr);
int begin_token(int **pcode,int *cp,int *pln,char *srclin,TOKENS tk,int *ntk,char *outbuff);
void begin_init(void);
int begin_next_line(int *pcode, int *cp);
bool begin_line(int *pln, char *srclin, int *len);
bool begin_line(int *pln, string& srclin);
bool begin_line_norep(int *pln, string& srclin);

void replace_exp(string& exp);

double token_next_double(int i) throw(ParserError);

bool get_block_line(int pln, string& srclin);

#endif
