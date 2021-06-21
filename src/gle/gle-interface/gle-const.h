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

#ifndef INCLUDE_GLECONST
#define INCLUDE_GLECONST

#define GLE_DEVICE_NONE        -1
#define GLE_DEVICE_EPS          0
#define GLE_DEVICE_PS           1
#define GLE_DEVICE_PDF          2
#define GLE_DEVICE_SVG          3
#define GLE_DEVICE_JPEG         4
#define GLE_DEVICE_PNG          5
#define GLE_DEVICE_X11          6
#define GLE_DEVICE_EMF          7
#define GLE_DEVICE_DUMMY        8
#define GLE_DEVICE_CAIRO_PDF    9
#define GLE_DEVICE_CAIRO_EPS    10
#define GLE_DEVICE_CAIRO_PS     11
#define GLE_DEVICE_CAIRO_SVG    12

#define BITMAP_TYPE_USER 0
#define BITMAP_TYPE_TIFF 1
#define BITMAP_TYPE_GIF  2
#define BITMAP_TYPE_PNG  3
#define BITMAP_TYPE_JPEG 4
#define BITMAP_TYPE_UNK  5

#define CM_PER_INCH 2.54
#define PS_POINTS_PER_INCH 72.0

#endif
