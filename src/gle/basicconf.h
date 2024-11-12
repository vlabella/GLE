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

#ifndef INCLUDE_BASICCONF_H
#define INCLUDE_BASICCONF_H

#ifdef HAVE_CONFIG_H
    #include "../config.h"
#else
    #include "../config_noauto.h"
#endif

#include <stdio.h>
#if HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#if STDC_HEADERS
	#include <stdlib.h>
	#include <stddef.h>
#else
	#if HAVE_STDLIB_H
		#include <stdlib.h>
	#endif
#endif
#if HAVE_STRING_H
	#if !STDC_HEADERS && HAVE_MEMORY_H
		#include <memory.h>
	#endif
	#include <string.h>
#endif
#if HAVE_STRINGS_H
	#include <strings.h>
#endif
#if HAVE_UNISTD_H
	#include <unistd.h>
#endif
#if HAVE_CSTRING
	#include <cstring>
#endif
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>

// using namespace std;  should not reside in header file

#if defined(__OS2__) && defined(__EMX__)
#define uint unsigned int
#endif

#ifdef __WIN32__
	#define DLLEXPORT __declspec( dllexport )
	#define DLLIMPORT __declspec( dllimport )
#else
	#define DLLEXPORT
	#define DLLIMPORT
#endif

#endif

#define GLE_INF std::numeric_limits<double>::infinity()
#define GLE_NAN std::numeric_limits<double>::quiet_NaN()
