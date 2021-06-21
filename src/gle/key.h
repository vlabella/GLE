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

#ifndef INC_KEY_H
#define INC_KEY_H

class GLEKeyBlockBase : public GLEBlockWithSimpleKeywords {
public:
	GLEKeyBlockBase();
	virtual ~GLEKeyBlockBase();

	virtual GLEBlockInstance* beginExecuteBlockImpl(GLESourceLine& sline, int *pcode, int *cp);
};

class KeyEntry {
public:
	char lstyle[9];
	GLERC<GLEColor> color;
	GLERC<GLEColor> textcolor;
	GLERC<GLEColor> fill;
	int marker;
	int column;
	double msize,lwidth;
	string descrip;
	int sepstyle;
	double sepdist;
public:
	KeyEntry(int col);
	~KeyEntry();
	bool hasFill() const;
};

class KeyRCInfo {
public:
	double size;
	double offs;
	double descent;
	double mleft;
	double mright;
	int elems;
	bool m_Line, m_Marker, m_Fill;
public:
	KeyRCInfo();
	KeyRCInfo(const KeyRCInfo& other);
	inline bool hasLine() { return m_Line; }
	inline bool hasMarker() { return m_Marker; }
	inline bool hasFill() { return m_Fill; }
	inline void setHasLine(bool val) { m_Line = val; }
	inline void setHasMarker(bool val) { m_Marker = val; }
	inline void setHasFill(bool val) { m_Fill = val; }
};

class KeyInfo {
protected:
	int m_MaxRow;
	GLERC<GLEColor> m_Color;
	GLERC<GLEColor> m_BoxColor;
	GLERC<GLEColor> m_BackgroundColor;
	vector<KeyRCInfo> m_ColInfo;
	vector<KeyRCInfo> m_RowInfo;
	vector<KeyEntry*> m_entries;
	double m_Hei, m_Base, m_LinePos, m_LineLen, m_ExtraY;
	double m_MarginX, m_MarginY, m_TotHei, m_ColDist, m_Dist;
	double m_OffsX, m_OffsY;
	bool m_PosOrJust, m_Absolute, m_HasOffset, m_HasBoxColor, m_NoBox, m_Fill;
	bool m_Compact, m_NoLines, m_Disabled;
	char m_Justify[34];
	GLEPoint m_ComputedMargins;
	GLERectangle m_Rect;
	int m_col;

public:
	KeyInfo();
	~KeyInfo();
	KeyRCInfo* expandToCol(int col);
	void expandToRow(int row);
	void setOffsetX(double x);
	void setOffsetY(double y);
	void setBoxColor(const GLERC<GLEColor>& col);
	void initPosition();
	KeyEntry* createEntry();
	KeyEntry* lastEntry();
	inline void addColumn() { m_col++; }
	inline GLERC<GLEColor> getBackgroundColor() { return m_BackgroundColor; }
	inline void setBackgroundColor(const GLERC<GLEColor>& col) { m_BackgroundColor = col; }
	inline bool hasFill() { return m_Fill; }
	inline void setHasFill(bool val) { m_Fill = val; }
	inline double getOffsetX() { return m_OffsX; }
	inline double getOffsetY() { return m_OffsY; }
	inline bool isPosOrJust() { return m_PosOrJust; }
	inline void setPosOrJust(bool pj) { m_PosOrJust = pj; }
	inline bool isAbsolute() { return m_Absolute; }
	inline void setAbsolute(bool abs) { m_Absolute = abs; }
	inline bool hasOffset() { return m_HasOffset; }
	inline char* getJustify() { return m_Justify; }
	inline int getMaxRow() { return m_MaxRow; }
	inline void setMaxRow(int row) { m_MaxRow = row; }
	inline KeyRCInfo* getCol(int i) { return &m_ColInfo[i]; }
	inline int getNbCols() { return m_ColInfo.size(); }
	inline KeyRCInfo* getRow(int i) { return &m_RowInfo[i]; }
	inline int getNbRows() { return m_RowInfo.size(); }
	inline GLERC<GLEColor> getDefaultColor() { return m_Color; }
	inline void setDefaultColor(const GLERC<GLEColor>& col) { m_Color = col; }
	inline double getHei() { return m_Hei; }
	inline bool hasHei() { return m_Hei != 0.0; }
	inline void setHei(double hei) { m_Hei = hei; }
	inline double getBase() { return m_Base; }
	inline bool hasBase() { return m_Base != 0.0; }
	inline void setBase(double base) { m_Base = base; }
	inline bool getNoBox() { return m_NoBox; }
	inline void setNoBox(bool nb) { m_NoBox = nb; }
	inline double getMarginX() { return m_MarginX; }
	inline double getMarginY() { return m_MarginY; }
	inline void setMarginXY(double mx, double my) { m_MarginX = mx; m_MarginY = my; }
	inline bool hasMargins() { return m_MarginX > -1e20; }
	inline double getTotalHei() { return m_TotHei; }
	inline void setTotalHei(double hei) { m_TotHei = hei; }
	inline double getColDist() { return m_ColDist; }
	inline void setColDist(double dist) { m_ColDist = dist; }
	inline bool hasColDist() { return m_ColDist > -1e20; }
	inline double getDist() { return m_Dist; }
	inline void setDist(double dist) { m_Dist = dist; }
	inline bool hasDist() { return m_Dist >= 0; }
	inline bool hasLinePos() { return m_LinePos >= 0; }
	inline double getLinePos() { return m_LinePos; }
	inline void setLinePos(double pos) { m_LinePos = pos; }
	inline bool hasLineLen() { return m_LineLen >= 0; }
	inline double getLineLen() { return m_LineLen; }
	inline void setLineLen(double Len) { m_LineLen = Len; }
	inline GLERC<GLEColor> getBoxColor() { return m_BoxColor; }
	inline bool hasBoxColor() { return m_HasBoxColor; }
	inline bool isCompact() { return m_Compact; }
	inline bool isNoLines() { return m_NoLines; }
	inline void setCompact(bool val) { m_Compact = val; }
	inline void setNoLines(bool val) { m_NoLines = val; }
	inline GLERectangle* getRect() { return &m_Rect; }
	inline GLEPoint* getComputedMargins() { return &m_ComputedMargins; }
	inline int getNbEntries() { return m_entries.size(); }
	inline double getExtraY() { return m_ExtraY; }
	inline void setExtraY(double v) { m_ExtraY = v; }
	inline void setDisabled(bool dis) { m_Disabled = dis; }
	inline bool isDisabled() { return m_Disabled; }
	inline KeyEntry* getEntry(int i) { return m_entries[i]; }
};

void draw_key(KeyInfo* info);
void measure_key(KeyInfo* info);
void draw_key_after_measure(KeyInfo* info);

void do_draw_key(double ox, double oy, bool notxt, KeyInfo* info);


#endif
