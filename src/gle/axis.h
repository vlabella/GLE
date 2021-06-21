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

#ifndef GLE_HAS_AXIS_H
#define GLE_HAS_AXIS_H
 
#define GLE_AXIS_NONE -1
#define GLE_AXIS_MAX 7
#define GLE_AXIS_X   1
#define GLE_AXIS_Y   2
#define GLE_AXIS_X2  3
#define GLE_AXIS_Y2  4
#define GLE_AXIS_X0  5
#define GLE_AXIS_Y0  6
#define GLE_AXIS_T   7
#define GLE_AXIS_ALL 8

#define GLE_AXIS_LOG_DEFAULT  0
#define GLE_AXIS_LOG_OFF      1
#define GLE_AXIS_LOG_25B      2
#define GLE_AXIS_LOG_25       3
#define GLE_AXIS_LOG_1        4
#define GLE_AXIS_LOG_N1       5

bool axis_ticks_neg(int axis);
bool axis_is_max(int axis);
bool axis_horizontal(int axis);
int axis_get_orth(int axis, int which);
int axis_type(const char *s);
int axis_type_check(const char *s) throw (ParserError);
const char* axis_type_name(int type);
bool axis_is_pos(double pos, int* cnt, double del, vector<double>& vec);
bool axis_is_pos_perc(double pos, int* cnt, double perc, vector<double>& vec);
double axis_range_dist_perc(double v1, double v2, GLERange* range, bool log);

void roundrange(GLERange* range, bool extend, bool tozero, double dticks);

class GLEDataSetDimension;

class GLEAxisQuantileScale : public GLERefCountObject {
protected:
	double m_QLower, m_QUpper, m_QLowerFactor, m_QUpperFactor;
public:
	GLEAxisQuantileScale();
	~GLEAxisQuantileScale();
	inline double getLowerQuantile() { return m_QLower; }
	inline double getUpperQuantile() { return m_QUpper; }
	inline double getLowerQuantileFactor() { return m_QLowerFactor; }
	inline double getUpperQuantileFactor() { return m_QUpperFactor; }
	inline void setLowerQuantile(double v) { m_QLower = v; }
	inline void setUpperQuantile(double v) { m_QUpper = v; }
	inline void setLowerQuantileFactor(double v) { m_QLowerFactor = v; }
	inline void setUpperQuantileFactor(double v) { m_QUpperFactor = v; }
};

class GLEAxis {
public:
	int type;	/* 1=xaxis 2=yaxis 3=x2axis 4=y2axis */
	double base;
	double length;
	double shift;
	int label_font;
	double label_hei;
	double label_scale;
	double label_dist;
	int label_align;
	bool log;
	int lgset;
	int nofirst,nolast;
	int nticks,nsubticks;
	bool has_ftick, has_offset, ticks_both;
	double ftick, dticks, dsubticks, offset;
	double ticks_length,ticks_scale,ticks_lwidth;
	char ticks_lstyle[9];
	double subticks_length,subticks_scale,subticks_lwidth;
	char subticks_lstyle[9],label_lstyle[9];
	int off;
	int label_off;
	int side_off,ticks_off,subticks_off;
	bool has_subticks_onoff, has_label_onoff;
	double side_lwidth,label_lwidth;
	char side_lstyle[9];
	int title_font;
	double title_dist, title_adist;
	double title_hei, title_scale;
	int title_rot,title_off;
	GLERC<GLEColor> title_color;	/* 0=normal, 1=rotate 180 */
	GLERC<GLEColor> ticks_color;
	GLERC<GLEColor> side_color;
	GLERC<GLEColor> subticks_color;
	GLERC<GLEColor> label_color;
	int names_ds;
	double label_angle;
	bool grid, gridtop;
	bool alignBase;
	bool roundRange;
	string title;
	vector<string> names;
	vector<double> places;
	vector<double> noticks1;
	vector<double> noticks2;
	vector<double> noplaces;
	int negate;
	string format;
	GLERangeSet m_Range;
	GLERangeSet m_DataRange;
	vector<GLEDataSetDimension*> m_Dims;
	GLERC<GLEAxisQuantileScale> m_QuantileScale;
public:
	GLEAxis();
	~GLEAxis();
	void init(int i);
	string* getNamePtr(int i);
	void setName(int i, const std::string& name);
	void setPlace(int i, double place);
	void clearNoTicks();
	void addNoTick(double pos);
	void insertNoTick(double pos);
	void insertNoTickOrLabel(double pos);
	void insertNoTick(double pos, vector<double>& vec);
	void printNoTicks();
	void getLabelsFromDataSet(int ds);
	int getNbNamedPlaces();
	bool isNoPlaceLogOrReg(double pos, int* place_cnt, double delta);
	void initRange();
	void performRoundRange(GLERange* range, bool extend, bool tozero);
	void roundDataRange(bool extend, bool tozero);
	void makeUpRange(GLEAxis* copy, GLEAxis* orth, bool extend, bool tozero);
	void setColor(const GLERC<GLEColor>& color);
	inline bool isPlace(double pos, int* cnt, double delta) { return axis_is_pos(pos, cnt, delta, places); }
	inline bool isPlaceRel(double pos, int* cnt) { return axis_is_pos_perc(pos, cnt, 1e-6, places); }
	inline bool isNoPlace(double pos, int* cnt, double delta) { return axis_is_pos(pos, cnt, delta, noplaces); }
	inline bool isNoTick1(double pos, int* cnt, double delta) { return axis_is_pos(pos, cnt, delta, noticks1); }
	inline bool isNoTick2(double pos, int* cnt, double delta) { return axis_is_pos(pos, cnt, delta, noticks2); }
	inline bool isPlacePerc(double pos, int* cnt) { return axis_is_pos_perc(pos, cnt, 0.001, places); }
	inline bool isNoPlacePerc(double pos, int* cnt) { return axis_is_pos_perc(pos, cnt, 0.001, noplaces); }
	inline bool isNoTick1Perc(double pos, int* cnt) { return axis_is_pos_perc(pos, cnt, 0.001, noticks1); }
	inline bool isNoTick2Perc(double pos, int* cnt) { return axis_is_pos_perc(pos, cnt, 0.001, noticks2); }
	inline double getPlace(unsigned int i) { return places[i]; }
	inline void addName(const char* name) { names.push_back(string(name)); }
	inline void addPlace(double place) { places.push_back(place); }
	inline void addNoTick1(double pos) { noticks1.push_back(pos); }
	inline void addNoTick2(double pos) { noticks2.push_back(pos); }
	inline void addNoPlace(double pos) { noplaces.push_back(pos); }
	inline void insertNoTick1(double pos) { insertNoTick(pos, noticks1); }
	inline void insertNoTick2(double pos) { insertNoTick(pos, noticks2); }
	inline void insertNoPlace(double pos) { insertNoTick(pos, noplaces); }
	inline bool hasPlaces() { return places.size() != 0; }
	inline bool hasNames() { return names.size() != 0; }
	inline int getNbPlaces() { return places.size(); }
	inline int getNbNames() { return names.size(); }
	inline void setNamesDataSet(int i) { names_ds = i; }
	inline int getNamesDataSet() { return names_ds; }
	inline double getLabelAngle() { return label_angle; }
	inline void setLabelAngle(double angle) { label_angle = angle; }
	inline bool hasGrid() { return grid; }
	inline void setGrid(bool gr) { grid = gr; }
	inline bool hasGridOnTop() { return gridtop; }
	inline void setGridOnTop(bool ontop) { gridtop = ontop; }
	inline void setAlignBase(bool align) { alignBase = align; }
	inline bool isAlignBase() { return alignBase; }
	inline GLERangeSet* getRange() { return &m_Range; }
	inline double getMin() { return m_Range.getMin(); }
	inline double getMax() { return m_Range.getMax(); }
	inline GLERange* getDataRange() { return &m_DataRange; }
	inline int getType() { return type; }
	inline const char* getName() { return axis_type_name(getType()); }
	inline void removeAllDimensions() { m_Dims.clear(); }
	inline void addDimension(GLEDataSetDimension* dim) { m_Dims.push_back(dim); }
	inline int getNbDimensions() { return m_Dims.size(); }
	inline GLEDataSetDimension* getDim(int i) { return m_Dims[i]; }
	inline bool shouldPerformQuantileScale() { return !m_QuantileScale.isNull(); }
	inline GLEAxisQuantileScale* getQuantileScale() { return m_QuantileScale.get(); }
	inline void setQuantileScale(GLEAxisQuantileScale* scale)  { m_QuantileScale = scale; }
private:
	double getLocalAveragePlacesDistance(int i);
};

void init_measure_by_axis(GLEAxis* ax, double ox, double oy, double llen);

enum DrawAxisPart {
   DRAW_AXIS_GRID_SUBTICKS,
   DRAW_AXIS_GRID_TICKS,
   DRAW_AXIS_ALL
};

void draw_axis(GLEAxis *ax, GLERectangle* box, DrawAxisPart drawPart); /* Draws the axis */

#endif // GLE_HAS_AXIS_H
