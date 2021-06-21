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

#include "op_def.h"
#include "color.h"

/*--------------------------------------------------------------------------------------*/
/* Pos = Offset to find the data                                                        */
/* Idx = For switches, which can only have one value                                    */
/* The pos is the order the items will be placed in the pcode                           */
/*                                                                                      */
/* indexes (idx) for types                                                              */
/* ===================================================================================  */
/* typ_switch   Switches    LONG    Placed in directly, 1 present, 0 not present        */
/* expressions              LONG*   Pointed to, 0 if not present                        */
/* type_fill    color/fill  LONG*   Pointer to exp 0 if not present                     */
/* marker                   LONG*   Pointer to exp 0 if not present                     */
/* lstyle                   LONG*   Pointer to exp 0 if not present                     */
/* typ_str      font        int*    Pointer to string expression                        */
/*                                                                                      */
/*--------------------------------------------------------------------------------------*/

struct op_key op_box[] = {
		{ "JUSTIFY", 	typ_justify, 	1, 0 },
		{ "JUST", 	typ_justify, 	1, 0 },
		{ "FILL", 	typ_fill, 	2, 0 },
		{ "NOSTROKE", 	typ_switch, 	3, 1 },
		{ "NOBORDER", 	typ_switch, 	3, 1 },
		{ "NOBOX", 	typ_switch, 	3, 1 },
		{ "REVERSE",	typ_switch,	4, 1 },
		{ "ROUND",	typ_val, 	5, 0 },
		{ "NAME", 	typ_str, 	6, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_circle[] = {
		{ "JUSTIFY", 	typ_justify, 	1, 0 },
		{ "JUST", 	typ_justify, 	1, 0 },
		{ "NOSTROKE", 	typ_switch, 	2, 1 },
		{ "FILL", 	typ_fill, 	3, 0 },
		{ "NAME", 	typ_str, 	4, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_defmarker[] = {
		{ "FONT", 	typ_str, 	1, 0 },
		{ "CHAR", 	typ_str, 	2, 0 },
		{ "DX", 	typ_val, 	3, 0 },
		{ "DY", 	typ_val, 	4, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_fill[] = {
		{ "FILL", 	typ_fill, 	1, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_for_step[] = {
		{ "STEP", 	typ_val, 	1, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_begin_text[] = {
		{ "WIDTH", 	typ_val, 	1, 0 },
		{ "DEPTH", 	typ_val, 	2, 0 },
		{ "JUSTIFY", 	typ_justify, 	3, 0 },
		{ "JUST", 	typ_justify, 	3, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_justify[] = {
		{ "LEFT", 	typ_switch, 	1, 0x100 },
		{ "CENT", 	typ_switch, 	1, 0x110 },
		{ "CENTER", 	typ_switch, 	1, 0x110 },
		{ "CENTRE", 	typ_switch, 	1, 0x110 },
		{ "RIGHT", 	typ_switch, 	1, 0x120 },
		{ "LT",	 	typ_switch, 	1, 0x02 },
		{ "TL",	 	typ_switch, 	1, 0x02 },
		{ "CL",	 	typ_switch, 	1, 0x01 },
		{ "LC",	 	typ_switch, 	1, 0x01 },
		{ "BL",	 	typ_switch, 	1, 0x00 },
		{ "LB",	 	typ_switch, 	1, 0x00 },
		{ "CB",	 	typ_switch, 	1, 0x10 },
		{ "BC",	 	typ_switch, 	1, 0x10 },
		{ "RB",	 	typ_switch, 	1, 0x20 },
		{ "BR",	 	typ_switch, 	1, 0x20 },
		{ "CR",	 	typ_switch, 	1, 0x21 },
		{ "RC",	 	typ_switch, 	1, 0x21 },
		{ "TR",	 	typ_switch, 	1, 0x22 },
		{ "RT",	 	typ_switch, 	1, 0x22 },
		{ "TC",	 	typ_switch, 	1, 0x12 },
		{ "CT",	 	typ_switch, 	1, 0x12 },
		{ "CC",	 	typ_switch, 	1, 0x11 },
		{ "CI",	 	typ_switch, 	1, 0x1011 },
		{ "C",	 	typ_switch, 	1, 0x1011 },
		{ "VI",	 	typ_switch, 	1, 0x2000 },
		{ "V",	 	typ_switch, 	1, 0x2000 },
		{ "HO",	 	typ_switch, 	1, 0x3000 },
		{ "H",	 	typ_switch, 	1, 0x3000 },
		{ "BOX",	typ_switch, 	1, 0x5011 },
		{ "BO",	 	typ_switch, 	1, 0x5011 },
		{ "END", 	typ_end, 	1, 1} };

struct op_key op_begin[] = {
		{ "path", 	typ_switch, 	1, 1 },
		{ "box", 	typ_switch, 	1, 2 },
		{ "scale", 	typ_switch, 	1, 3 },
		{ "rotate", 	typ_switch, 	1, 4 },
		{ "translate", 	typ_switch, 	1, 5 },
		{ "rot", 	typ_switch, 	1, 4 },
		{ "tran", 	typ_switch, 	1, 5 },
		{ "if", 	typ_switch, 	1, 6 },
		{ "sub", 	typ_switch, 	1, 7 },
		{ "name", 	typ_switch, 	1, 8 },
		{ "text", 	typ_switch, 	1, 9 },
		{ "graph", 	typ_switch, 	1, 10 },
		{ "xaxis", 	typ_switch, 	1, 11 },
		{ "yaxis", 	typ_switch, 	1, 12 },
		{ "x2axis", 	typ_switch, 	1, 13 },
		{ "y2axis", 	typ_switch, 	1, 14 },
		{ "curve",	typ_switch,	1, 15 },
		{ "key",	typ_switch,	1, 16 },
		{ "origin",	typ_switch,	1, 17 },
		{ "table",	typ_switch,	1, 18 },
		{ "clip",	typ_switch,	1, 19 },
		{ "until",	typ_switch,	1, 20 },
		{ "shear",	typ_switch,	1, 21 },
		{ "config",      typ_switch,     1, 22 },
		{ "texpreamble", typ_switch,     1, 23 },
		{ "surface",     typ_switch,     1, 24 },
		{ "letz",        typ_switch,     1, 25 },
		{ "fitz",        typ_switch,     1, 26 },
		{ "fit",         typ_switch,     1, 27 }, /* what previously fitls used to be */
		{ "contour",     typ_switch,     1, 28 },
		{ "tex",         typ_switch,     1, 29 },
		{ "object",      typ_switch,     1, OP_BEGIN_OBJECT },
		{ "length",      typ_switch,     1, OP_BEGIN_LENGTH },
		{ "END", 	typ_end, 	1, 1} };

struct op_key op_set[] = {
		{ "HEI",         typ_switch, 	1, 1 },
		{ "FONT",        typ_switch, 	1, 2 },
		{ "JUSTIFY",     typ_switch, 	1, 3 },
		{ "JUST",        typ_switch, 	1, 3 },
		{ "COLOR",       typ_switch, 	1, 4 },
		{ "BACKGROUND",  typ_switch, 	1, OP_SET_BACKGROUND },
		{ "DASHLEN",     typ_switch, 	1, 5 },
		{ "DASH",        typ_switch, 	1, 6 },
		{ "FILL",        typ_switch, 	1, OP_SET_FILL },
		{ "PATTERN",     typ_switch, 	1, OP_SET_FILL_PATTERN },
		{ "FILLMETHOD",  typ_switch, 	1, OP_SET_FILL_METHOD },
		{ "LDIST",       typ_switch, 	1, 5 },
		{ "LSTYLE",      typ_switch, 	1, 6 },
		{ "LWIDTH",      typ_switch, 	1, 7 },
		{ "JOIN",        typ_switch, 	1, 8 },
		{ "CAP",         typ_switch, 	1, 9 },
		{ "FONTLWIDTH",  typ_switch, 	1, 10 },
		{ "TEXSCALE",    typ_switch, 	1, OP_SET_TEX_SCALE },
		{ "TEXLABELS",   typ_switch, 	1, OP_SET_TEX_LABELS },
		{ "ARROWSTYLE",  typ_switch, 	1, OP_SET_ARROW_STYLE },
		{ "ARROWTIP",    typ_switch, 	1, OP_SET_ARROW_TIP },
		{ "ARROWSIZE",   typ_switch, 	1, OP_SET_ARROW_SIZE },
		{ "ARROWANGLE",  typ_switch, 	1, OP_SET_ARROW_ANGLE },
		{ "IMAGEFORMAT", typ_switch, 	1, OP_SET_IMAGE_FORMAT },
		{ "TITLESCALE",  typ_switch,    1, OP_SET_TITLE_SCALE },
		{ "ATITLESCALE", typ_switch,    1, OP_SET_ATITLE_SCALE },
		{ "ALABELSCALE", typ_switch,    1, OP_SET_ALABEL_SCALE },
		{ "TICKSSCALE",  typ_switch,    1, OP_SET_TICKS_SCALE },
		{ "ATITLEDIST",  typ_switch,    1, OP_SET_ATITLE_DIST },
		{ "ALABELDIST",  typ_switch,    1, OP_SET_ALABEL_DIST },
		{ "END",         typ_end,       1, 1} };

/* colors, fills,  First byte is pattern, then red,green,blue intensities*/
/* currently defined patterns are,  ff=clear, 1=black,  */
struct op_key op_fill_typ[] = {
		{ "CLEAR",	typ_switch,	1, GLE_FILL_CLEAR },
		{ "GRID", 	typ_switch, 	1, 0X02012020 },
		{ "GRID1", 	typ_switch, 	1, 0X02040f0f },
		{ "GRID2", 	typ_switch, 	1, 0X02011010 },
		{ "GRID3", 	typ_switch, 	1, 0X02052020 },
		{ "GRID4", 	typ_switch, 	1, 0X02104040 },
		{ "GRID5", 	typ_switch, 	1, 0X02206060 },
		{ "SHADE", 	typ_switch, 	1, 0X02010020 },
		{ "SHADE1", 	typ_switch, 	1, 0X0204000C },
		{ "SHADE2", 	typ_switch, 	1, 0X02010010 },
		{ "SHADE3", 	typ_switch, 	1, 0X02050020 },
		{ "SHADE4", 	typ_switch, 	1, 0X02100040 },
		{ "SHADE5", 	typ_switch, 	1, 0X02200060 },
		{ "RSHADE", 	typ_switch, 	1, 0X02012000 },
		{ "RSHADE1", 	typ_switch, 	1, 0X02040C00 },
		{ "RSHADE2", 	typ_switch, 	1, 0X02011000 },
		{ "RSHADE3", 	typ_switch, 	1, 0X02052000 },
		{ "RSHADE4", 	typ_switch, 	1, 0X02104000 },
		{ "RSHADE5", 	typ_switch, 	1, 0X02206000 },
		{ "END", 	typ_end, 	1, 1} };

struct op_key op_marker[] = {
		{ "DOT", 	typ_switch, 	1, 1 },
		{ "CROSS", 	typ_switch, 	1, 2 },
		{ "FCIRCLE", 	typ_switch, 	1, 3 },
		{ "FSQUARE", 	typ_switch, 	1, 4 },
		{ "FTRIANGLE", 	typ_switch, 	1, 5 },
		{ "FDIAMOND", 	typ_switch, 	1, 6 },
		{ "CIRCLE", 	typ_switch, 	1, 7 },
		{ "SQUARE", 	typ_switch, 	1, 8 },
		{ "TRIANGLE", 	typ_switch, 	1, 9 },
		{ "DIAMOND", 	typ_switch, 	1, 10 },
		{ "PLUS", 	typ_switch, 	1, 11 },
		{ "CLUB", 	typ_switch, 	1, 12 },
		{ "HEART", 	typ_switch, 	1, 13 },
		{ "DIAMOND", 	typ_switch, 	1, 14 },
		{ "SPADE", 	typ_switch, 	1, 15 },
		{ "STAR", 	typ_switch, 	1, 16 },
		{ "SNAKE", 	typ_switch, 	1, 17 },
		{ "DAG", 	typ_switch, 	1, 18 },
		{ "DDAG", 	typ_switch, 	1, 19 },
		{ "ASTERIX", 	typ_switch, 	1, 20 },
		{ "ASTERISK", 	typ_switch, 	1, 20 },
		{ "OPLUS", 	typ_switch, 	1, 21 },
		{ "OMINUS", 	typ_switch, 	1, 22 },
		{ "OTIMES", 	typ_switch, 	1, 23 },
		{ "ODOT", 	typ_switch, 	1, 24 },
		{ "TRIANGLEZ", 	typ_switch, 	1, 25 },
		{ "DIAMONDZ", 	typ_switch, 	1, 26 },
		{ "WCIRCLE", 	typ_switch, 	1, 27 },
		{ "WTRIANGLE", 	typ_switch, 	1, 28 },
		{ "WSQUARE", 	typ_switch, 	1, 29 },
		{ "WDIAMOND", 	typ_switch, 	1, 30 },
		{ "END", 	typ_end, 	1, 1  } };

struct op_key op_begin_path[] = {
		{ "STROKE", 	typ_switch, 	1, 1 },
		{ "FILL", 	typ_fill, 	2, 0 },
		{ "CLIP", 	typ_switch, 	3, 1 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_begin_box[] = {
		{ "ADD", 	typ_val, 	1, 0 },
		{ "FILL", 	typ_fill, 	2, 0 },
		{ "NOSTROKE", 	typ_switch, 	3, 1 },
		{ "NOBORDER", 	typ_switch, 	3, 1 },
		{ "NOBOX", 	typ_switch, 	3, 1 },
		{ "ROUND",	typ_val, 	4, 0 },
		{ "NAME", 	typ_str, 	5, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_begin_name[] = {
		{ "ADD", 	typ_val, 	1, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_begin_scale[] = {
		{ "TRANSLATE", 	typ_val2, 	1, 0 },
		{ "TRAN", 	typ_val2, 	1, 0 },
		{ "SCALE", 	typ_val2, 	3, 0 },
		{ "ROTATE", 	typ_val, 	5, 0 },
		{ "ROT", 	typ_val, 	5, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_arc[] = {
		{ "ARROW", 	typ_arrow, 	1, 1 },
		{ "CENTER", 	typ_val2, 	2, 0 },
		{ "CENTRE", 	typ_val2, 	2, 0 },
		{ "CENT", 	typ_val2, 	2, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_arrow[] = {
		{ "START", 	typ_switch, 	1, 1 },
		{ "END", 	typ_switch, 	1, 2 },
		{ "BOTH", 	typ_switch, 	1, 3 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_joinname[] = {
		{ "->",	 	typ_switch, 	1, 1 },
		{ "<-",	 	typ_switch, 	1, 2 },
		{ "<->", 	typ_switch, 	1, 3 },
		{ "-", 		typ_switch, 	1, 4 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_line[] = {
		{ "ARROW", 	typ_arrow, 	1, 1 },
		{ "CURVE",	typ_val4,	2, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_curve[] = {
		{ "CURVE",	typ_val4,	1, 0 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_size[] = {
		{ "BOX", 	typ_switch, 	1, 1 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_join[] = {
		{ "MITRE", 	typ_switch, 	1, 0 },
		{ "ROUND", 	typ_switch, 	1, 1 },
		{ "BEVEL", 	typ_switch, 	1, 2 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_cap[] = {
		{ "BUTT", 	typ_switch, 	1, 0 },
		{ "ROUND", 	typ_switch, 	1, 1 },
		{ "SQUARE", 	typ_switch, 	1, 2 },
		{ "END", 	typ_end, 	0, 0 } };

struct op_key op_bitmap[] = {
		{ "COLORS",      typ_val,        1, 0 },
		{ "COMPRESS",	typ_str, 	2, 0 },
		{ "DPI",         typ_val,        3, 0 },
		{ "GRAYSCALE",   typ_switch,     4, 1 },
		{ "RESIZE",      typ_switch,     5, 1 },
		{ "TYPE",	typ_str,        6, 0 },
		{ "END", 	typ_end,	0, 0 } };

struct op_key op_bitmap_info[] = {
		{ "TYPE",	typ_str,        1, 0 },
		{ "END", 	typ_end,	0, 0 } };

struct op_key op_tex[] = {
		{ "ADD", 	typ_val, 	1, 0 },
		{ "NAME",	typ_str,        2, 0 },
		{ "END", 	typ_end,	0, 0 } };

struct op_key op_draw[] = {
		{ "NAME",	typ_str,        2, 0 },
		{ "END", 	typ_end,	0, 0 } };

struct op_key op_orientation[] = {
		{ "PORTRAIT", 	typ_switch, 	1, 0 },
		{ "LANDSCAPE", 	typ_switch, 	1, 1 },
		{ "END", 	typ_end, 	0, 0} };

struct op_key op_colormap[] = {
		{ "COLOR", 	typ_switch, 	1, 1 },
		{ "PALETTE",	typ_str,        2, 0 },
		{ "END", 	typ_end, 	0, 0 } };

