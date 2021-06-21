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

#ifndef INCLUDE_MEM_LIMTS
#define INCLUDE_MEM_LIMTS
//
// mem_limts.h
//
/*
 memory limits for GLE.
 I pulled these out of the code, most were hard-coded into the source
 these are all statically defined arrays, soon they will be replaced with STL
 containers

These is gleaned from the file var.c

also I had to modify the files sub.c polish.c eval.c to
include LOCAL_START_INDEX

it seems that all the commands,functions and variables are indexed by one variable

if the index is > LOCAL_START_INDEX then it is treated as a local variable
and stored in the lvar_name and lvar_val

else the variable is a global variable
*/
#define NUM_GLOBAL 1000	 //number of global variables
#define NUM_LOCAL  500  //number of local variables each subroutine can have
#define MAX_GLOBAL NUM_GLOBAL-1 //GLE will warn the user
#define MAX_LOCAL  NUM_LOCAL-1  //GLE will warn the user
//
// all the variables are kept on the same stack except that the
// local variables start at a higher index (don't change these)
//
#define MAX_INDEX		NUM_GLOBAL+NUM_LOCAL
#define LOCAL_START_INDEX	NUM_GLOBAL

//
// for the token arrays
//
#define TOKEN_WIDTH 1000
#define TOKEN_LENGTH 500

//
// these are for graphing routines
//
// maximum number of items in each line of the data file
#define MAX_NUMBER_OF_ITEMS 1000

//
// maximum element in the postscript line.  This affects speed of
// PS rendereing.  any line that has more than
// MAXIMUM_PS_VECTOR points in it will be broken up
// into lines of MAXIMUM_PS_VECTOR
// not this is assinged to MAX_VECTOR in gle.cpp
// no warning is printed if this is exceeded as was done in 3.3h
//
#define MAXIMUM_PS_VECTOR 500

// Maximum number of datasets
#define MAX_NB_DATA 1001

#endif
