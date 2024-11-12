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

#ifndef INCLUDE_ALL
#define INCLUDE_ALL

#include "basicconf.h"
#include "gle-interface/gle-base.h"

#include <fstream>
#include <sstream>

#ifdef M_PI
	#define GLE_PI M_PI
#else
	#define GLE_PI 3.14159265358979323846
#endif
//
// -- define these constants for use in scripts
//

#if ( defined(__unix__) || defined(__APPLE__) ) && defined(HAVE_X11)
	#define ENABLE_GS_PREVIEW
#endif

//
// -- containers
//
typedef std::vector< std::vector<int> > _GlobalPCode;
typedef std::vector< std::vector<int> >::iterator _itGlobalPCode;
typedef std::vector< std::vector<int> >::const_iterator _citGlobalPCode;

typedef std::vector<int> _PCode;
typedef std::vector<int>::iterator _itPCode;
typedef std::vector<int>::const_iterator _citPCode;

typedef std::vector<std::string> _Tokens;
typedef std::vector<std::string>::iterator _itTokens;
typedef std::vector<std::string>::const_iterator _citTokens;

#include "glepro.h"

#endif
