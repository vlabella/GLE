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

struct op_key {
	char name[256];
	int typ;
	int pos;
	unsigned int idx;
};

#define typ_end     0
#define typ_val     1
#define typ_val2    2
#define typ_val4    3
#define typ_str     4
#define typ_switch  5
#define typ_color   6
#define typ_fill    7
#define typ_marker  8
#define typ_lstyle  9
#define typ_justify 10
#define typ_arrow   11

#define OP_SET_ARROW_SIZE    11
#define OP_SET_ARROW_ANGLE   12
#define OP_SET_ARROW_STYLE   13
#define OP_SET_ARROW_TIP     14
#define OP_SET_IMAGE_FORMAT  15
#define OP_SET_TITLE_SCALE   16
#define OP_SET_ATITLE_SCALE  17
#define OP_SET_ALABEL_SCALE  18
#define OP_SET_TICKS_SCALE   19
#define OP_SET_ATITLE_DIST   20
#define OP_SET_ALABEL_DIST   21
#define OP_SET_TEX_SCALE     22
#define OP_SET_TEX_LABELS    23
#define OP_SET_FILL          24
#define OP_SET_FILL_PATTERN  25
#define OP_SET_BACKGROUND    26
#define OP_SET_FILL_METHOD   27

#define OP_BEGIN_OBJECT      30
#define OP_BEGIN_LENGTH      31

extern struct op_key op_box[];
extern struct op_key op_circle[];
extern struct op_key op_defmarker[];
extern struct op_key op_fill[];
extern struct op_key op_for_step[];
extern struct op_key op_begin_text[];
extern struct op_key op_justify[];
extern struct op_key op_begin[];
extern struct op_key op_set[];
extern struct op_key op_fill_typ[];
extern struct op_key op_marker[];
extern struct op_key op_begin_path[];
extern struct op_key op_begin_box[];
extern struct op_key op_begin_name[];
extern struct op_key op_begin_scale[];
extern struct op_key op_arc[];
extern struct op_key op_arrow[];
extern struct op_key op_joinname[];
extern struct op_key op_line[];
extern struct op_key op_curve[];
extern struct op_key op_size[];
extern struct op_key op_join[];
extern struct op_key op_cap[];
extern struct op_key op_bitmap[];
extern struct op_key op_bitmap_info[];
extern struct op_key op_tex[];
extern struct op_key op_orientation[];
extern struct op_key op_colormap[];
extern struct op_key op_draw[];

