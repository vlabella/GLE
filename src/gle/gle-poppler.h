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

#ifndef INCLUDE_GLE_POPPLER_H
#define INCLUDE_GLE_POPPLER_H

#define GLE_OUTPUT_OPTION_TRANSPARENT 1
#define GLE_OUTPUT_OPTION_GRAYSCALE   2

typedef void (*gle_write_func)(void* closure, char* data, int length);

void gle_glib_init(int argc, char** argv);

void gle_convert_pdf_to_image(char* pdfData,
		                      int pdfLength,
		                      double resolution,
		                      int device,
		                      int options,
		                      gle_write_func writeFunc,
		                      void* closure);

#ifdef HAVE_CAIRO

#include <cairo.h>

void gle_write_cairo_surface_bitmap(cairo_surface_t* surface,
		                            int device,
		                            int options,
		                            gle_write_func writeFunc,
		                            void* closure);

#ifdef HAVE_POPPLER

void gle_convert_pdf_to_image_file(char* pdfData,
		                           int pdfLength,
		                           double resolution,
		                           int device,
		                           int options,
		                           const char* fname);

#endif // HAVE_POPPLER
#endif // HAVE_CAIRO
#endif // INCLUDE_GLE_POPPLER_H
