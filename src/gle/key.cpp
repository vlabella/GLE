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
#include "mem_limits.h"
#include "token.h"
#include "cutils.h"
#include "core.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "gle-block.h"
#include "key.h"
#include "justify.h"
#include "color.h"
#include "gprint.h"
#include "op_def.h"
#include "var.h"
#include "sub.h"

/* for key command and gx(), gy() */
extern double graph_x1,graph_y1,graph_x2,graph_y2;  /* in cm */
extern double graph_xmin,graph_ymin,graph_xmax,graph_ymax; /* graph units */

#define BEGINDEF extern
#include "begin.h"
#include <math.h>
#define dbg if ((gle_debug & 64)>0)
#define LARGE_NUM 1E30

char *un_quote(char *ct);
extern int gle_debug;
void doskip(char *s,int *ct);
double get_next_exp(TOKENS tk,int ntk,int *curtok);

#define kw(ss) if (str_i_equals(tk[ct],ss))
#define true (!false)
#define false 0
#define skipspace doskip(tk[ct],&ct)
#define tok(n)  (*tk)[n]
#define next_exp (get_next_exp(tk,ntk,&ct))
#define next_font ((ct+=1),pass_font(tk[ct]))
#define next_marker ((ct+=1),pass_marker(tk[ct]))
#define next_color ((ct+=1),pass_color_var(tk[ct]))
#define next_fill ((ct+=1),pass_color_var(tk[ct]))
#define next_str(s)  (ct+=1,strcpy(s,tk[ct]))
#define next_vstr(s)  (ct+=1,mystrcpy(&s,tk[ct]))
#define next_quote(s) (ct+=1,strcpy(&s,un_quote(tk[ct])))
#define next_vquote_cpp(s) (ct+=1,skipspace,pass_file_name(tk[ct],s))

#define KEY_FILL_HEI_FY 0.66
#define KEY_FILL_HEI_FX 0.7

class ParseGeneralKeyInfo {
public:
	ParseGeneralKeyInfo() {
		GLESubDefinitionHelper passKeyInfo("key");
		offset = passKeyInfo.addPointArgument("offset", new GLEPointDataObject(0.0, 0.0), false);
		margins = passKeyInfo.addPointArgument("margins", new GLEPointDataObject(0.0, 0.0), false);
		absolute = passKeyInfo.addPointArgument("absolute", 0, false);
		// background = passKeyInfo.addPointArgument("background", GLE_FILL_CLEAR, false);
		base = passKeyInfo.addDoubleArgumentNoDefault("base", false);
		passKeyInfo.addArgumentAlias(base, "row");
		linepos = passKeyInfo.addDoubleArgumentNoDefault("lpos", false);
		linelen = passKeyInfo.addDoubleArgumentNoDefault("llen", false);
/*
		else kw("NOBOX") info.setNoBox(true);
		else kw("NOLINE") info.setNoLines(true);
		else kw("COMPACT") info.setCompact(true);
		else kw("OFF") info.setDisabled(true);
		else kw("HEI") khei = next_exp;
		else kw("POSITION") next_str(info.getJustify());
		else kw("POS") next_str(info.getJustify());
		else kw("BOXCOLOR") info.setBoxColor(next_color);
		else kw("SEPARATOR") {
*/
	}

public:
	unsigned int offset;
	unsigned int margins;
	unsigned int absolute;
	unsigned int background;
	unsigned int base;
	unsigned int linepos;
	unsigned int linelen;
};

class GLEKeyBlockInstance : public GLEBlockInstance {
public:
	GLEKeyBlockInstance(GLEKeyBlockBase* parent);
	virtual ~GLEKeyBlockInstance();
	virtual void executeLine(GLESourceLine& sline);
	virtual void endExecuteBlock();

private:
	KeyInfo m_info;
	bool m_hasPattern;
};

GLEKeyBlockInstance::GLEKeyBlockInstance(GLEKeyBlockBase* parent):
	GLEBlockInstance(parent),
	m_hasPattern(false)
{
}

GLEKeyBlockInstance::~GLEKeyBlockInstance() {
}

void GLEKeyBlockInstance::executeLine(GLESourceLine& sline)	{
	double zzhei;
	g_get_hei(&zzhei);
	begin_init();
	int st = begin_token(sline, srclin, tk, &ntk, outbuff, true);
	if (!st) {
		/* exit loop */
		return;
	}
	/* line count variable*/
	int ct = 1;
	KeyEntry* entry = m_info.lastEntry();
	while (ct <= ntk) {
		skipspace;
		kw("OFFSET") {
			m_info.setOffsetX(next_exp);
			m_info.setOffsetY(next_exp);
		}
		else kw("MARGINS") {
			double mx = next_exp;
			double my = next_exp;
			m_info.setMarginXY(mx, my);
		}
		else kw("ABSOLUTE") {
			if (ct <= ntk-1) {
				m_info.setOffsetX(next_exp);
				m_info.setOffsetY(next_exp);
			}
			m_info.setAbsolute(true);
		}
		else if (!m_hasPattern && str_i_equals(tk[ct],"BACKGROUND")) m_info.setBackgroundColor(next_color);
		else kw("ROW") m_info.setBase(next_exp);
		else kw("BASE") m_info.setBase(next_exp);
		else kw("LPOS") m_info.setLinePos(next_exp);
		else kw("LLEN") m_info.setLineLen(next_exp);
		else kw("NOBOX") m_info.setNoBox(true);
		else kw("NOLINE") m_info.setNoLines(true);
		else kw("COMPACT") m_info.setCompact(true);
		else kw("OFF") m_info.setDisabled(true);
		else kw("HEI") m_info.setHei(next_exp);
		else kw("POSITION") next_str(m_info.getJustify());
		else kw("POS") next_str(m_info.getJustify());
		else kw("BOXCOLOR") m_info.setBoxColor(next_color);
		else kw("SEPARATOR") {
			if (entry == 0) {
				g_throw_parser_error("key: 'separator' should come after a valid key entry");
			} else {
				ct++;
				kw("LSTYLE") {
					entry->sepstyle = (int)floor(next_exp + 0.5);
				} else {
					ct--;
				}
				m_info.addColumn();
			}
		}
		else kw("JUSTIFY") {
			next_str(m_info.getJustify());
			m_info.setPosOrJust(false);
		}
		else kw("JUST") {
			next_str(m_info.getJustify());
			m_info.setPosOrJust(false);
		}
		else kw("DIST") m_info.setDist(next_exp);
		else kw("COLDIST") m_info.setColDist(next_exp);
		else {
			if (ct == 1) {
				entry = m_info.createEntry();
			}
			if (entry == 0) {
				return;
			}
			kw("TEXT") {
				next_vquote_cpp(entry->descrip);
				if (g_get_tex_labels()) {
					entry->descrip.insert(0, "\\tex{");
					entry->descrip.append("}");
				}
			}
			else kw("MARKER") {
				entry->marker = next_marker;
			}
			else kw("MSIZE") entry->msize = next_exp;
			else kw("MSCALE") entry->msize = (next_exp) * zzhei;
			else kw("COLOR") entry->color = next_color;
			else kw("TEXTCOLOR") entry->textcolor = next_color;
			else kw("FILL") {
				GLERC<GLEColor> fillColor(next_fill);
				update_color_foreground_and_pattern(entry->fill.get(), fillColor.get());
			} else kw("PATTERN") {
				GLERC<GLEColor> fillPattern(next_fill);
				if (fillPattern->isFill() && fillPattern->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
					update_color_fill_pattern(entry->fill.get(), static_cast<GLEPatternFill*>(fillPattern->getFill()));
				} else {
					g_throw_parser_error("expected fill pattern");
				}
			} else kw("BACKGROUND") {
				GLERC<GLEColor> backColor(next_fill);
				update_color_fill_background(entry->fill.get(), backColor.get());
			}
			else kw("LSTYLE") next_str(entry->lstyle);
			else kw("LINE") strcpy(entry->lstyle, "1");
			else kw("LWIDTH") entry->lwidth = next_exp;
			else g_throw_parser_error("unrecognised KEY sub command: '", tk[ct], "'");
		}
		ct++;
	}
}

void GLEKeyBlockInstance::endExecuteBlock() {
	draw_key(&m_info);
}

GLEKeyBlockBase::GLEKeyBlockBase():
	GLEBlockWithSimpleKeywords("key", false)
{
	const char* commands[] = {
		"OFFSET", "MARGINS", "ABSOLUTE", "BACKGROUND", "ROW",
		"BASE", "LPOS", "LLEN", "NOBOX", "NOLINE", "COMPACT",
		"OFF", "HEI", "POSITION", "POS", "BOXCOLOR", "SEPARATOR",
		"LSTYLE", "JUSTIFY", "JUST", "DIST", "COLDIST", "TEXT",
		"MARKER", "MSIZE", "MSCALE", "COLOR", "TEXTCOLOR", "FILL",
		"PATTERN", "LINE", "LWIDTH", ""};
	for (int i = 0; commands[i][0] != 0; ++i) {
		addKeyWord(commands[i]);
	}
}

GLEKeyBlockBase::~GLEKeyBlockBase()
{
}

GLEBlockInstance* GLEKeyBlockBase::beginExecuteBlockImpl(GLESourceLine& /* sline */, int* /* pcode */, int* /* cp */) {
	return new GLEKeyBlockInstance(this);
}

KeyRCInfo::KeyRCInfo() {
	size = 0.0;
	offs = 0.0;
	descent = 0.0;
	elems = 0;
	mleft = 0.0;
	mright = 0.0;
	m_Line = false;
	m_Marker = false;
	m_Fill = false;
}

KeyRCInfo::KeyRCInfo(const KeyRCInfo& other) {
	size = other.size;
	offs = other.offs;
	descent = other.descent;
	elems = other.elems;
	mleft = other.mleft;
	mright = other.mright;
	m_Line = other.m_Line;
	m_Marker = other.m_Marker;
	m_Fill = other.m_Fill;
}

KeyInfo::KeyInfo() {
	m_MaxRow = 0;
	m_NoBox = false;
	m_Fill = false;
	m_MarginX = -1e30;
	m_MarginY = -1e30;
	m_Hei = 0.0;
	m_Base = 0.0;
	m_TotHei = 0.0;
	m_ColDist = -1e30;
	m_Dist = -1e30;
	m_LinePos = -1e30;
	m_LineLen = -1e30;
	m_OffsX = 0.0;
	m_OffsY = 0.0;
	m_PosOrJust = true;
	m_Absolute = false;
	m_HasOffset = false;
	m_HasBoxColor = false;
	m_Compact = false;
	m_NoLines = false;
	m_Disabled = false;
	m_BoxColor = 0;
	m_ExtraY = 0.0;
	m_BackgroundColor = g_get_fill_clear();
	m_col = 0;
	strcpy(m_Justify, "");
}

KeyInfo::~KeyInfo() {
	for (size_t i = 0; i < m_entries.size(); ++i) {
		delete m_entries[i];
	}
}

void KeyInfo::setOffsetX(double x) {
	m_OffsX = x;
	m_HasOffset = true;
}

void KeyInfo::setOffsetY(double y) {
	m_OffsY = y;
	m_HasOffset = true;
}

void KeyInfo::setBoxColor(const GLERC<GLEColor>& col) {
	m_BoxColor = col;
	m_HasBoxColor = true;
}

void KeyInfo::initPosition() {
	if (m_Justify[0] == 0) {
		// no justify or position given
		if (hasOffset()) {
			strcpy(m_Justify, "BL");
			setPosOrJust(false);
		} else {
			strcpy(m_Justify, "TR");
			setPosOrJust(true);
		}
	}
}

KeyRCInfo* KeyInfo::expandToCol(int col) {
	while ((int)m_ColInfo.size() <= col) {
		m_ColInfo.push_back(KeyRCInfo());
	}
	return &m_ColInfo[col];
}

void KeyInfo::expandToRow(int row) {
	while ((int)m_RowInfo.size() <= row) {
		m_RowInfo.push_back(KeyRCInfo());
	}
}

KeyEntry* KeyInfo::createEntry() {
	KeyEntry* entry = new KeyEntry(m_col);
	m_entries.push_back(entry);
	return entry;
}

KeyEntry* KeyInfo::lastEntry() {
	if (m_entries.empty()) {
		return 0;
	} else {
		return m_entries.back();
	}
}

void draw_key(KeyInfo* info) {
	if (info->getNbEntries() == 0) {
		return;
	}
	GLEPoint savept;
	g_get_xy(&savept);
	if (info->getBackgroundColor()->isTransparent()) {
		info->setBackgroundColor(g_get_color_hex(GLE_COLOR_WHITE));
	}
	measure_key(info);
	draw_key_after_measure(info);
	g_move(savept);
}

void measure_key_v_recent(KeyInfo* info, GLEPoint* orig) {
	/* Add separator dist */
	for (int i = 0; i < info->getNbEntries(); i++) {
		info->getCol(info->getEntry(i)->column)->size += info->getEntry(i)->sepdist;
	}
	/* Compute sum of column widths and max number of rows */
	int maxrow = 0;
	double sumwid = 0.0;
	for (int i = 0; i < info->getNbCols(); i++) {
		sumwid += info->getCol(i)->size;
		if (info->getCol(i)->elems > maxrow) maxrow = info->getCol(i)->elems;
	}
	info->setMaxRow(maxrow);
	/* Initialize offsets for each column */
	double rowhi = info->getBase();
	info->getCol(0)->offs = 0.0;
	for (int i = 1; i < info->getNbCols(); i++) {
		double entry_wd = 0.0;
		KeyRCInfo* prev_col = info->getCol(i-1);
		if (prev_col->hasLine() && !info->isCompact() && !info->isNoLines()) {
			entry_wd += info->getLineLen() + info->getDist();
		}
		if (prev_col->hasMarker()) entry_wd += info->getDist();
		if (prev_col->hasFill()) entry_wd += KEY_FILL_HEI_FX*rowhi + info->getDist();
		info->getCol(i)->offs = prev_col->offs + prev_col->size + entry_wd + info->getColDist() +
		                        prev_col->mleft + prev_col->mright;
	}
	/* Initialize offsets for each row */
	double rowoffs = 0.0;
	for (int i = info->getNbRows()-2; i >= 0; i--) {
		double prev_rowhi = info->getRow(i)->descent * 1.3 + info->getRow(i+1)->size * 1.1;
		if (rowhi > prev_rowhi) prev_rowhi = rowhi;
		rowoffs += prev_rowhi;
		info->getRow(i)->offs = rowoffs;
	}
	/* Measure entire key */
	GLEMeasureBox measure;
	measure.measureStart();
	do_draw_key(0.0, 0.0, true, info);
	measure.measureEnd();
	double sx = measure.getWidth()+2*info->getMarginX();
	double sy = measure.getHeight()+2*info->getMarginY();
	info->setTotalHei(sy);
	info->setExtraY(0.0);
	// Make sure text does not go out of box
	double b_descent = info->getRow(info->getNbRows()-1)->descent;
	if (b_descent * 1.3 > info->getMarginY()) {
		info->setExtraY(b_descent * 1.3 - info->getMarginY());
		sy += info->getExtraY();
	}
	double ox, oy;
	double dx = info->getOffsetX();
	double dy = info->getOffsetY();
	if (info->isPosOrJust()) {
		/* Relative to graph */
		double midx = graph_x1 + (graph_x2-graph_x1)/2;
		double midy = graph_y1 + (graph_y2-graph_y1)/2;
		if (dx < 0.0) dx = dx-sx;
		if (dy < 0.0) dy = dy-sy;
		if (str_i_equals(info->getJustify(),"TL"))      { ox = graph_x1+dx;    oy = graph_y2-sy-dy; }
		else if (str_i_equals(info->getJustify(),"BL")) { ox = graph_x1+dx;    oy = graph_y1+dy;    }
		else if (str_i_equals(info->getJustify(),"BR")) { ox = graph_x2-sx-dx; oy = graph_y1+dy;    }
		else if (str_i_equals(info->getJustify(),"TR")) { ox = graph_x2-sx-dx; oy = graph_y2-sy-dy; }
		else if (str_i_equals(info->getJustify(),"TC")) { ox = midx-sx/2+dx;   oy = graph_y2-sy-dy; }
		else if (str_i_equals(info->getJustify(),"BC")) { ox = midx-sx/2+dx;   oy = graph_y1+dy;    }
		else if (str_i_equals(info->getJustify(),"RC")) { ox = graph_x2-sx-dx; oy = midy-sy/2+dy;   }
		else if (str_i_equals(info->getJustify(),"LC")) { ox = graph_x1+dx;    oy = midy-sy/2+dy;   }
		else if (str_i_equals(info->getJustify(),"CC")) { ox = midx-sx/2+dx;   oy = midy-sy/2+dy;   }
		else {
			if (strlen(info->getJustify())>0) gprint("Expecting POS BL,BR,TR or TL\n");
			ox = graph_x2-sx; oy = graph_y2-sy;
		}
	} else {
		ox = dx; oy = dy;
		if (!info->isAbsolute()) {
			ox += orig->getX();
			oy += orig->getY();
		}
		/* Support different relative positions for offset command */
		if (str_i_equals(info->getJustify(),"TL"))      { oy -= sy;               }
		else if (str_i_equals(info->getJustify(),"BR")) { ox -= sx;               }
		else if (str_i_equals(info->getJustify(),"TR")) { ox -= sx;   oy -= sy;   }
		else if (str_i_equals(info->getJustify(),"TC")) { ox -= sx/2; oy -= sy;   }
		else if (str_i_equals(info->getJustify(),"BC")) { ox -= sx/2;             }
		else if (str_i_equals(info->getJustify(),"RC")) { ox -= sx;   oy -= sy/2; }
		else if (str_i_equals(info->getJustify(),"LC")) { oy -= sy/2;             }
		else if (str_i_equals(info->getJustify(),"CC")) { ox -= sx/2; oy -= sy/2; }
	}
	info->getRect()->setDimensions(ox, oy, ox+sx, oy+sy);
	info->getComputedMargins()->setXY(info->getMarginX()-measure.getXMin(), info->getMarginY()-measure.getYMin());
}

void measure_key_v35(KeyInfo* info, GLEPoint* orig) {
	double sx = 0;
	double cr = info->getBase();
	KeyRCInfo* col_info = info->getCol(0);
	if (col_info->hasLine()) sx = sx + 2*cr;
	if (col_info->hasMarker()) sx = sx + 1.5*cr;
	if (col_info->hasFill()) sx = sx + 1.3*cr;
	sx = sx + col_info->size;
	sx = sx + 1.2*cr;
	double sy = info->getNbEntries()*cr + 1.2*cr-0.3*info->getHei();
	double ox, oy;
	if (!info->hasOffset()) {
		double midx = graph_x1 + (graph_x2-graph_x1)/2;
		double midy = graph_y1 + (graph_y2-graph_y1)/2;
		if (str_i_equals(info->getJustify(),"TL")) {ox = graph_x1;oy = graph_y2-sy;}
		else if (str_i_equals(info->getJustify(),"BL")) {ox = graph_x1; oy = graph_y1;}
		else if (str_i_equals(info->getJustify(),"BR")) {ox = graph_x2-sx;oy = graph_y1;}
		else if (str_i_equals(info->getJustify(),"TR")) {ox = graph_x2-sx; oy = graph_y2-sy;}
		else if (str_i_equals(info->getJustify(),"TC")) {ox = midx-sx/2; oy = graph_y2-sy;}
		else if (str_i_equals(info->getJustify(),"BC")) {ox = midx-sx/2; oy = graph_y1;}
		else if (str_i_equals(info->getJustify(),"RC")) {ox = graph_x2-sx; oy = midy-sy/2;}
		else if (str_i_equals(info->getJustify(),"LC")) {ox = graph_x1; oy = midy-sy/2;}
		else if (str_i_equals(info->getJustify(),"CC")) {ox = midx-sx/2; oy = midy-sy/2;}
		else {
			if (strlen(info->getJustify())>0) gprint("Expecting POS BL,BR,TR or TL\n");
			ox = graph_x2-sx; oy = graph_y2-sy;
		}
	} else {
		ox = orig->getX() + info->getOffsetX();
		oy = orig->getY() + info->getOffsetY();
	}
	info->getRect()->setDimensions(ox, oy, ox+sx, oy+sy);
}

void do_draw_key_v35(double ox, double oy, KeyInfo* info){
	double cr = info->getBase();
	KeyRCInfo* col_info = info->getCol(0);
	g_set_hei(info->getHei());
	for (int i = info->getNbEntries() - 1; i >= 0; i--) {
		KeyEntry* entry = info->getEntry(i);
		g_move(ox+0.6*cr, oy+0.6*cr+cr*(info->getNbEntries()-i-1));
		if (!entry->color.isNull()) {
			g_set_color(entry->color);
		}
		if (col_info->hasMarker()) {
			g_rmove(cr/2, info->getHei()*.35);
			double z = entry->msize;
			if (z==0) z = info->getHei();
			if (entry->marker!=0) g_marker(entry->marker, z);
			g_rmove(cr, -info->getHei()*.35);
		}
		if (col_info->hasLine()) {
			double savelw;
			g_set_line_style(entry->lstyle);
			g_get_line_width(&savelw);
			g_set_line_width(entry->lwidth);
			g_rmove(0.0, cr*.3);
			if (entry->lstyle[0] == 0) g_rmove(1.5*cr, 0.0);
			else g_rline(1.5*cr, 0.0);
			g_rmove(cr/2, -cr*.3);
			g_set_line_style("1");
			g_set_line_width(savelw);
		}
		if (col_info->hasFill()) {
			if (entry->hasFill()) {
				g_set_fill(entry->fill);
				double cx, cy;
				g_get_xy(&cx,&cy);
				g_box_fill(cx,cy,cx+cr*.7, cy+cr*.66);
				g_box_stroke(cx,cy,cx+cr*.7, cy+cr*.66);
			}
			g_rmove(1.3*cr, 0.0);
		}
		if (!entry->color.isNull()) {
			g_set_color(info->getDefaultColor());
		}
		g_set_just(JUST_LEFT);
		if (entry->descrip != "") g_text((char*)entry->descrip.c_str());
	}
}

void measure_key(KeyInfo* info) {
	GLEPoint orig;
	double save_hei;
	GLERectangle save_bounds;
	/* Initialize */
	info->initPosition();
	g_get_xy(&orig);
	GLERC<GLEColor> old_color(g_get_color());
	GLERC<GLEColor> old_fill(g_get_fill());
	g_get_hei(&save_hei);
	g_get_bounds(&save_bounds);
	if (!info->hasHei()) {
		info->setHei(save_hei);
	}
	double khei = info->getHei();
	if (!info->hasBase()) {
		info->setBase(1.2*khei);
	}
	double rowhi = info->getBase();
	info->setDefaultColor(old_color);
	/* Init margins */
	double margin_x = 0.45*rowhi;
	double margin_y = 0.45*rowhi;
	if (info->hasMargins()) {
		margin_x = info->getMarginX();
		margin_y = info->getMarginY();
	} else {
		info->setMarginXY(margin_x, margin_y);
	}
	if (!info->hasColDist()) {
		info->setColDist(margin_x);
	}
	if (!info->hasDist()) {
		info->setDist(margin_x * 0.85);
	}
	if (!info->hasLineLen()) {
		info->setLineLen(1.5*rowhi);
	}
	/* Use fill somewhere? */
	for (int i = 0; i < info->getNbEntries(); i++) {
		if (info->getEntry(i)->hasFill()) {
			info->setHasFill(true);
		}
	}
	/* Empty key? */
	if (info->getNbEntries() == 0) {
		return;
	}
	/* Use dummy device for measurements */
	GLEDevice* old_device = g_set_dummy_device();
	/* Measure all labels and count rows in each column */
	g_set_hei(khei);
	// cout << "key height: " << khei << endl;
	double linePos = GLE_INF;
	for (int i = 0; i < info->getNbEntries(); i++) {
		KeyEntry* entry = info->getEntry(i);
		int col = entry->column;
		KeyRCInfo* colinfo = info->expandToCol(col);
		int row = colinfo->elems;
		info->expandToRow(row);
		if (!str_only_space(entry->descrip)) {
			double bl, br, bu, bd;
			g_measure(entry->descrip,&bl,&br,&bu,&bd);
			if (colinfo->size < br) colinfo->size = br;
			if (-bd > info->getRow(row)->descent) info->getRow(row)->descent = -bd;
			// size should not include descent, only ascent of the text
			if (bu > info->getRow(row)->size) info->getRow(row)->size = bu;
			// cout << "key = " << entry->descrip << " bl = " << bl << " br = " << br << " bu = " << bu << " bd = " << bd << endl;
			if (bu/2 < linePos) linePos = bu/2;
		}
		/* Set booleans: kl = lstyle, km = marker, kf = fill */
		if (entry->lstyle[0] == 0 && entry->lwidth > 0) {
			strcpy(entry->lstyle, "1");
		}
		if (entry->lstyle[0] != 0) colinfo->setHasLine(true);
		if (entry->lwidth > 0) colinfo->setHasLine(true);
		if (entry->marker != 0) colinfo->setHasMarker(true);
		if (entry->hasFill()) {
			colinfo->setHasFill(true);
		}
		/* Adjust row height in case of fill */
		if (info->hasFill()) {
			if (rowhi*KEY_FILL_HEI_FY > info->getRow(row)->size) {
				// Row should have at least the height of the fill box
				info->getRow(row)->size = rowhi*KEY_FILL_HEI_FY;
			}
		}
		if (entry->marker != 0) {
			double z = entry->msize;
			if (z == 0) z = khei;
			GLEMeasureBox marksize;
			marksize.measureStart();
			g_move(0.0, 0.0);
			g_marker(entry->marker,z);
			marksize.measureEnd();
			if (info->isCompact() && !info->isNoLines()) {
				double yval = marksize.getYMin();
				double llen = info->getLineLen();
				marksize.updateRange(-llen/2, yval);
				marksize.updateRange(+llen/2, yval);
			}
			if (info->getCol(col)->mleft < -marksize.getXMin()) info->getCol(col)->mleft = -marksize.getXMin();
			if (info->getCol(col)->mright < marksize.getXMax()) info->getCol(col)->mright = marksize.getXMax();
		} else if (info->isCompact() && colinfo->hasLine() && !info->isNoLines()) {
			// compact mode, but no marker in this column!
			double llen = info->getLineLen();
			if (info->getCol(col)->mleft < llen/2) info->getCol(col)->mleft = llen/2;
			if (info->getCol(col)->mright < llen/2) info->getCol(col)->mright = llen/2;
			colinfo->setHasMarker(true);
		}
		info->getCol(col)->elems++;
	}
	/* Set linePos based to half of the fill block */
	if (info->hasFill()) {
		linePos = rowhi*KEY_FILL_HEI_FY/2;
	}
	if (!info->hasLinePos()) {
		// Baseline for text
		info->setLinePos(linePos);
	}
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		g_restore_device(old_device);
		measure_key_v35(info, &orig);
	} else {
		measure_key_v_recent(info, &orig);
		g_restore_device(old_device);
	}
	g_set_bounds(&save_bounds);
	g_set_color(old_color);
	g_set_fill(old_fill);
	g_set_hei(save_hei);
}

void draw_key_after_measure(KeyInfo* info) {
	if (info->getNbEntries() == 0 || info->isDisabled()) {
		return;
	}
	double save_hei;
	g_get_hei(&save_hei);
	GLERC<GLEColor> old_color(g_get_color());
	GLERC<GLEColor> old_fill(g_get_fill());
	double ox = info->getRect()->getXMin();
	double oy = info->getRect()->getYMin();
	if (!info->getNoBox() && !info->getBackgroundColor()->isTransparent()) {
		g_set_fill(info->getBackgroundColor());
		g_box_fill(info->getRect());
	}
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		do_draw_key_v35(ox, oy, info);
	} else {
		do_draw_key(ox+info->getComputedMargins()->getX(), oy+info->getComputedMargins()->getY()+info->getExtraY(), false, info);
		int prev_col = 0;
		for (int i = 0; i < info->getNbEntries(); i++) {
			if (prev_col != info->getEntry(i)->column) {
				prev_col = info->getEntry(i)->column;
				if (i >= 1 && info->getEntry(i - 1)->sepstyle != -1) {
					char msep[9];
					// should implement g_set_line_style that takes int
					sprintf(msep, "%d", info->getEntry(i - 1)->sepstyle);
					g_set_line_style(msep);
					double xsep = ox+info->getComputedMargins()->getX()+info->getCol(prev_col)->offs-info->getColDist()/2;
					g_move(xsep, oy);
					g_line(xsep, info->getRect()->getYMax());
					g_set_line_style("1");
				}
			}
		}
	}
	if (!info->getNoBox()) g_box_stroke(info->getRect());
	g_set_fill(old_fill);
	g_set_color(old_color);
	g_set_hei(save_hei);
}

void do_draw_key(double ox, double oy, bool notxt, KeyInfo* info) {
	double savelw;
	double cx, cy;
	/* Draw all labels */
	int row = 0;
	int prev_col = 0;
	double khei = info->getHei();
	double rowhi = info->getBase();
	g_set_hei(khei);
	for (int i = 0; i < info->getNbEntries(); i++) {
		KeyEntry* entry = info->getEntry(i);
		if (prev_col != entry->column) {
			row = 0;
			prev_col = entry->column;
		}
		KeyRCInfo* col_info = info->getCol(prev_col);
		cx = ox+col_info->offs;
		cy = oy+info->getRow(row)->offs;
		g_move(cx, cy);
		g_update_bounds(cx, cy);
		if (!entry->color.isNull()) {
			g_set_color(entry->color);
		}
		if (col_info->hasMarker()) {
			g_rmove(col_info->mleft, info->getLinePos());
			g_get_line_width(&savelw);
			g_set_line_width(entry->lwidth);
			if (col_info->hasLine() && info->isCompact() && !info->isNoLines() && entry->lstyle[0] != 0) {
				g_set_line_style(entry->lstyle);
				g_rmove(-info->getLineLen()/2, 0.0);
				g_rline(info->getLineLen(), 0.0);
				g_rmove(-info->getLineLen()/2, 0.0);
				g_set_line_style("1");
			}
			if (entry->marker != 0) {
				double z = entry->msize;
				if (z == 0) z = khei;
				g_marker(entry->marker, z);
			}
			g_set_line_width(savelw);         
			g_rmove(col_info->mright+info->getDist(),-info->getLinePos());
		}
		if (col_info->hasLine() && !info->isCompact() && !info->isNoLines()) {
			g_set_line_style(entry->lstyle);
			g_get_line_width(&savelw);
			g_set_line_width(entry->lwidth);
			g_rmove(0.0,info->getLinePos());
			if (entry->lstyle[0]==0) g_rmove(info->getLineLen(),0.0);
			else g_rline(info->getLineLen(),0.0);
			g_rmove(info->getDist(),-info->getLinePos());
			g_set_line_style("1");
			g_set_line_width(savelw);
		}
		if (!entry->color.isNull()) {
			g_set_color(info->getDefaultColor());
		}
		if (col_info->hasFill()) {
			if (entry->hasFill()) {
				g_set_fill(entry->fill);
				g_get_xy(&cx,&cy);
				g_box_fill(cx,cy,cx+rowhi*KEY_FILL_HEI_FX, cy+rowhi*KEY_FILL_HEI_FY);
				GLERC<GLEColor> save_color(g_get_color());
				if (info->hasBoxColor()) {
					GLERC<GLEColor> boxcolor = info->getBoxColor();
					if (!boxcolor->isTransparent()) {
						g_set_color(boxcolor);
						g_box_stroke(cx,cy,cx+rowhi*KEY_FILL_HEI_FX, cy+rowhi*KEY_FILL_HEI_FY);
						g_set_color(save_color);
					}
				} else {
					g_box_stroke(cx,cy,cx+rowhi*KEY_FILL_HEI_FX, cy+rowhi*KEY_FILL_HEI_FY);
				}
			}
			g_rmove(KEY_FILL_HEI_FX*rowhi + info->getDist(), 0.0);
		}
		g_get_xy(&cx,&cy);
		if (!notxt) {
			g_set_just(JUST_LEFT);
			if (entry->descrip != "") {
				g_set_color(entry->textcolor);
				g_text((char*)entry->descrip.c_str());
				if (!entry->textcolor.isNull()) {
					g_set_color(info->getDefaultColor());
				}
			}
		} else {
			g_update_bounds(cx+col_info->size, cy+info->getRow(row)->size);
		}
		row++;
	}
}

KeyEntry::KeyEntry(int col) {
	column = col;
	sepstyle = -1;
	sepdist = 0.0;
	lstyle[0] = 0;
	lwidth = 0.0;
	color = 0;
	marker = 0;
	msize = 0.0;
	fill = new GLEColor();
	fill->setTransparent(true);
}

KeyEntry::~KeyEntry() {
}

bool KeyEntry::hasFill() const {
	return !fill.isNull() && !fill->isTransparent();
}
