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

// Built-in function IDs.
//
// Built-in function opcodes are encoded in expression pcode by adding
// FN_BUILTIN_OFFSET to the function ID. The offset separates built-in
// function opcodes from operators and other low-numbered pcode values.
//
// The resulting opcode must be less than LOCAL_START_INDEX, defined in
// mem_limits.h. Built-in function IDs must therefore satisfy:
//
//     FN_BUILTIN_OFFSET + function ID < LOCAL_START_INDEX
//
// See fn.cpp, eval.cpp, and mem_limits.h.
#define FN_BUILTIN_OFFSET              60

#define FN_RGB                         47
#define FN_EVAL                        76
#define FN_NARGS                       81
#define FN_ARG                         82
#define FN_ARGS                        83
#define FN_MIN                         84
#define FN_MAX                         85
#define FN_SDIV                        86
#define FN_XBAR                        87
#define FN_XY2ANGLE                    88
#define FN_DATAXVALUE                  89
#define FN_DATAYVALUE                  90
#define FN_NDATA                       91
#define FN_ATAN2                       92
#define FN_ISNAME                      93
#define FN_FILE                        94
#define FN_PATH                        95
#define FN_FONT                        96
#define FN_GETENV                      97
#define FN_XG3D                        98
#define FN_YG3D                        99
#define FN_JUSTIFY                     100
#define FN_RGBA                        101
#define FN_RGBA255                     102
#define FN_NDATASETS                   103
#define FN_DI                          104
#define FN_FACTORIAL  			       105
#define FN_DOUBLE_FACTORIAL  	       106
#define FN_HERMITE    			       107
#define FN_LAGUERRE   			       108
#define FN_ASSOCIATED_LAGUERRE         109
#define FN_LEGENDRE   			       110
#define FN_ASSOCIATED_LEGENDRE         111
#define FN_SPHERICAL_HARMONIC          112
#define FN_BESSEL_FIRST			       113
#define FN_BESSEL_SECOND		       114
#define FN_ERF                         115
#define FN_AIRY_FIRST                  116
#define FN_AIRY_SECOND                 117
#define FN_CHEBYSHEV_FIRST             118
#define FN_CHEBYSHEV_SECOND            119
#define FN_CBRT                        120
#define FN_CEIL                        121
#define FN_ERFC                        122
#define FN_EXPM1                       123
#define FN_FLOOR                       124
#define FN_GAMMA                       125
#define FN_HYPOT                       126
#define FN_LOG1P                       127
#define FN_LOG_GAMMA                   128
#define FN_ROUND                       129
#define FN_SINC                        130
#define FN_BESSEL_MODIFIED_FIRST       131
#define FN_BESSEL_MODIFIED_SECOND      132
#define FN_SPHERICAL_BESSEL_FIRST      133
#define FN_SPHERICAL_BESSEL_SECOND     134
#define FN_HSV                         135
#define FN_HSL                         136
#define FN_CMYK                        137
#define FN_COLOR_SCALE                 138
#define FN_ISCAIRO                     139
#define FN_ISGHOSTSCRIPT               140
#define FN_RENDER_ENGINE               141
#define FN_CLAMP                       142
#define FN_IS_CLOSE                    143
#define FN_IS_CLOSE_ABS                144
#define FN_COLOR_INTENSITY             145
#define FN_LERP                        146
#define FN_LINEAR_INTERPOLATE          147

#define FN_TEST_PRINT                  999

struct keyw {
	const char *word;
	int index;
	int ret, np, p[5];
};

int binsearch(char *word, struct keyw tab[], int n);


