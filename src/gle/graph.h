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

#include "gle-block.h"
#include "axis.h"
#include "file_io.h"
#include "bitmap/img2ps.h"

class GLEGraphBlockBase;
class GLEGraphDrawCommand;
class GLEGraphBlockData;
class GLEGraphBlockInstance;

const int GLE_GRAPH_LAYER_UNDEFINED    = -1;
const int GLE_GRAPH_LAYER_GRID         = 200;
const int GLE_GRAPH_LAYER_FILL         = 350;
const int GLE_GRAPH_LAYER_BAR          = 350;
const int GLE_GRAPH_LAYER_AXIS         = 500;
const int GLE_GRAPH_LAYER_LINE         = 700;
const int GLE_GRAPH_LAYER_ERROR_BAR    = 700;
const int GLE_GRAPH_LAYER_MARKER       = 700;
const int GLE_GRAPH_LAYER_DRAW_COMMAND = 700;

class GLEInternalClassDefinitions : public GLERefCountObject
{
public:
	GLEInternalClassDefinitions();

	inline GLEClassDefinition* getKeySeparator() { return m_keySeparator.get(); }
	inline GLEClassDefinition* getDrawCommand() { return m_drawCommand.get(); }
	inline GLEClassDefinition* getFill() { return m_fill.get(); }
	inline GLEClassDefinition* getBar() { return m_bar.get(); }

public:
	GLERC<GLEClassDefinition> m_keySeparator;
	GLERC<GLEClassDefinition> m_drawCommand;
	GLERC<GLEClassDefinition> m_fill;
	GLERC<GLEClassDefinition> m_bar;
};

class GLEGraphDataSetOrder : public GLERefCountObject
{
public:
	GLEGraphDataSetOrder(GLEGraphBlockData* data);

	void addDataSet(int dataSetID);
	void addObject(GLEDataObject* object);
	inline GLEArrayImpl* getArray() { return m_order.get(); }
	inline GLEGraphBlockData* getData() { return m_data; }

private:
	GLEGraphBlockData* m_data;
	GLERC<GLEArrayImpl> m_order;
	std::set<int> m_isIn;
};

class GLEGraphBlockData
{
public:
	GLEGraphBlockData(GLEGraphBlockBase* graphBlockBase);

	inline GLEGraphBlockBase* getGraphBlockBase() { return m_graphBlockBase; }
	inline GLEGraphDataSetOrder* getOrder() { return m_order.get(); }

private:
	GLEGraphBlockBase* m_graphBlockBase;
	GLERC<GLEGraphDataSetOrder> m_order;
};

class GLEGraphPart
{
public:
	GLEGraphPart();
	virtual ~GLEGraphPart();

	virtual std::set<int> getLayers() = 0;
	virtual void drawLayer(int layer);
	virtual void addToOrder(GLEGraphDataSetOrder* order);
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);
};

class GLEGraphPartGrid : public GLEGraphPart
{
public:
	GLEGraphPartGrid();
	virtual ~GLEGraphPartGrid();

	virtual std::set<int> getLayers();
	virtual void drawLayer(int layer);
   void drawLayerPart(DrawAxisPart axisPart);
};

class GLEGraphPartFills : public GLEGraphPart
{
public:
	GLEGraphPartFills();
	virtual ~GLEGraphPartFills();

	virtual std::set<int> getLayers();
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	bool shouldDraw(int n);
	void drawFill(int n);
};

class GLEGraphPartBars : public GLEGraphPart
{
public:
	GLEGraphPartBars();
	virtual ~GLEGraphPartBars();

	virtual std::set<int> getLayers();
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	bool shouldDraw(int n);
	void drawBar(int b);
};

class GLEGraphPartAxis : public GLEGraphPart
{
public:
	GLEGraphPartAxis();
	virtual ~GLEGraphPartAxis();

	virtual std::set<int> getLayers();
	virtual void drawLayer(int layer);

	void setBox(GLERectangle* box);

private:
	GLERectangle* m_box;
};

class GLEGraphPartLines : public GLEGraphPart
{
public:
	GLEGraphPartLines();
	virtual ~GLEGraphPartLines();

	virtual std::set<int> getLayers();
	virtual void addToOrder(GLEGraphDataSetOrder* order);
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	bool shouldDraw(int dn);
	void drawLine(int dn);
};

class GLEGraphPartErrorBars : public GLEGraphPart
{
public:
	GLEGraphPartErrorBars();
	virtual ~GLEGraphPartErrorBars();

	virtual std::set<int> getLayers();
	virtual void addToOrder(GLEGraphDataSetOrder* order);
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	bool shouldDraw(int dn);
	void drawErrorBars(int dn);
};

class GLEGraphPartMarkers : public GLEGraphPart
{
public:
	GLEGraphPartMarkers();
	virtual ~GLEGraphPartMarkers();

	virtual std::set<int> getLayers();
	virtual void addToOrder(GLEGraphDataSetOrder* order);
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	bool shouldDraw(int dn);
	void drawMarkers(int dn);
};

class GLEGraphDrawCommands : public GLEGraphPart
{
public:
	GLEGraphDrawCommands();
	virtual ~GLEGraphDrawCommands();

	virtual std::set<int> getLayers();
	virtual void drawLayerObject(int layer, GLEMemoryCell* object);

	void doDrawCommand(GLESourceLine& sline, GLEGraphBlockInstance* graphBlock);

private:
	GLEVectorAutoDelete<GLEGraphDrawCommand> m_drawCommands;
};

class GLEGraphBlockInstance : public GLEBlockInstance {
public:
	GLEGraphBlockInstance(GLEGraphBlockBase* parent);
	virtual ~GLEGraphBlockInstance();

	virtual void executeLine(GLESourceLine& sline);
	virtual void endExecuteBlock();

	void doDrawCommand(GLESourceLine& sline);

	int getLayer() const;
	int getLayerWithDefault(int defaultLayer) const;
	void setLayer(int layer);

	void drawParts();

	GLEGraphPartAxis* getAxis();
	void setData(GLEGraphBlockData* data);
	GLEGraphBlockData* getData();
	GLEGraphBlockBase* getGraphBlockBase();

private:
	GLEGraphBlockBase* m_graphBlockBase;
	int m_layer;
	GLEGraphBlockData* m_data;
	GLEGraphDrawCommands* m_drawCommands;
	GLEGraphPartAxis* m_axis;
	GLEVectorAutoDelete<GLEGraphPart> m_graphParts;
};

class GLEGraphBlockBase : public GLEBlockBase {
public:
	GLEGraphBlockBase();
	virtual ~GLEGraphBlockBase();

	virtual GLEBlockInstance* beginExecuteBlockImpl(GLESourceLine& sline, int *pcode, int *cp);
	virtual bool checkLine(GLESourceLine& sline);

	inline GLEInternalClassDefinitions* getClassDefinitions() { return m_classDefinitions.get(); }

private:
	GLERC<GLEInternalClassDefinitions> m_classDefinitions;
};

#define BEGINDEF extern

#define GLE_GRAPH_LM_PLAIN    0
#define GLE_GRAPH_LM_STEPS    1
#define GLE_GRAPH_LM_FSTEPS   2
#define GLE_GRAPH_LM_HIST     3
#define GLE_GRAPH_LM_IMPULSES 4
#define GLE_GRAPH_LM_BAR      5

/* for key command and gx(), gy() */

#define dbg if ((gle_debug & 64)>0)
extern int gle_debug;

class KeyInfo;
class GLELet;

void graph_init(void);
void graph_free(void);
void iffree(void *p, const char *s);
void setrange(double x, double y, int m);
void gdraw_key(KeyInfo* info);
void copy_default(int d);
void do_dataset(int d, GLEGraphBlockInstance* graphBlock) throw(ParserError);
void do_each_dataset_settings();
void fill_vec(double x1, double y1, double x2, double y2, vector<double>* vec);
void do_smooth(void);
void window_set(bool showError) throw(ParserError);
void reset_axis_ranges();
bool should_autorange_based_on_lets();
void deleteLet(GLELet* let);
GLELet* parseLet(GLESourceLine& sline) throw(ParserError);
GLELet* parseLet(const string& letFct, int codeLine) throw(ParserError);
void doLet(GLELet* let, bool nofirst) throw(ParserError);
void request(void);
/*int draw_axis(void *axis);*/
void bar_reset();
void doskip(char *s,int *ct);
void store_window_bounds_to_vars();
void do_dataset_key(int d);
void do_bigfile_compatibility() throw(ParserError);
void ensureDataSetCreated(int d);

#define kw(ss) if (str_i_equals(tk[ct],ss))
#define true (!false)
#define false 0

void var_find_dn(int *idx, int *vara, int *nd);
char *un_quote(char *ct);

#define skipspace doskip(tk[ct],&ct)
//#define tok(n)  (*tk)[n]
#define tok(n)  tk[n]
#define next_exp (get_next_exp(tk,ntk,&ct))
#define next_font ((ct+=1),pass_font(tk[ct]))
#define next_marker ((ct+=1),pass_marker(tk[ct]))
#define next_color ((ct+=1),pass_color_var(tk[ct]))
#define next_fill ((ct+=1),pass_color_var(tk[ct]))
#define next_str(s)  (ct+=1,skipspace,strcpy(s,tk[ct]))
#define next_str_cpp(s)  (ct+=1,skipspace,s=tk[ct])
#define next_vstr(s)  (ct+=1,skipspace,mystrcpy(&s,tk[ct]))
#define next_vquote(s) (ct+=1,skipspace,mystrcpy(&s,un_quote(tk[ct])))
#define next_vquote_cpp(s) (ct+=1,skipspace,pass_file_name(tk[ct],s))
#define next_quote(s) (ct+=1,skipspace,strcpy(s,un_quote(tk[ct])))

class fill_data {
public:
	fill_data();
	
public:
	int layer;
	int da, db;	/* fill from, too */
	int type; 	/* 1= x1,d1, 2=d1,x2, 3=d1,d2, 4=d1 */
	GLERC<GLEColor> color;
	double xmin, ymin, xmax, ymax;
};

class GLEDataSet;

void draw_vec(double x1, double y1, double x2, double y2, GLEDataSet* ds);
void draw_mark(double x1, double y1, int i, double sz, double dval, GLEDataSet* ds) throw (ParserError);

/* range of dataset dimension is initialized in window_set */
/* can be different from axis range because "xmin" / "xmax" / "ymin" / "ymax" settings of dn */

class GLEDataSetDimension {
protected:
	int m_Axis, m_Index;
	GLERangeSet m_Range;
	GLEDataSet* m_Data;
public:
	GLEDataSetDimension();
	~GLEDataSetDimension();
	void copy(GLEDataSetDimension* other);
	int getDataDimensionIndex();
	inline GLERangeSet* getRange() { return &m_Range; }
	inline int getAxis() { return m_Axis; }
	inline void setAxis(int axis) { m_Axis = axis; }
	inline int getIndex() { return m_Index; }
	inline void setIndex(int idx) { m_Index = idx; }
	inline void setDataSet(GLEDataSet* set) { m_Data = set; }
	inline GLEDataSet* getDataSet() { return m_Data; }
};

class GLEDataPairs : public GLERefCountObject {
protected:
	vector<double> m_X;
	vector<double> m_Y;
	vector<int> m_M;
public:
	GLEDataPairs();
	GLEDataPairs(double* x, double* y, int* m, int np);
	GLEDataPairs(GLEDataSet* dataSet);
	virtual ~GLEDataPairs();
	void copy(GLEDataSet* dataSet);
	void copyDimension(GLEDataSet* dataSet, unsigned int dim);
	void resize(int np);
	void set(double* x, double* y, int* m, int np);
	void set(unsigned int i, double x, double y, int m);
	void add(double x, double y, int m);
	void noMissing();
	void noNaN();
	void transformLog(bool xlog, bool ylog);
	void untransformLog(bool xlog, bool ylog);
	void noLogZero(bool xlog, bool ylog);
	vector<double>* getDimension(unsigned int i);
	double getMinXInterval();
	inline unsigned int size() const { return m_X.size(); }
	inline double getX(int i) const { return m_X[i]; }
	inline double getY(int i) const { return m_Y[i]; }
	inline int getM(int i) const { return m_M[i]; }
	inline double* getX() { return &m_X[0]; }
	inline double* getY() { return &m_Y[0]; }
	inline int* getM() { return &m_M[0]; }

public:
	static void validate(GLEDataSet* data, unsigned int minDim);
	static double getDataPoint(GLEMemoryCell* element, int datasetID, unsigned int dimension, unsigned int arrayIdx);

private:
	void copyDimensionImpl(GLEArrayImpl* data, unsigned int np, int datasetID, unsigned int dim);
};

class GLEAxis;

class GLEDataSet {
public:
	int id;
	int nomiss;
	unsigned int np; /* NUMBER OF POINTS */
	int autoscale;
	bool axisscale;
	bool inverted;
	char lstyle[9];
	string key_name;
	char *bigfile;
	GLERC<GLEColor> key_fill;
	GLERC<GLEColor> color;
	double errwidth;
	string errup;
	string errdown;
	double herrwidth;
	string herrup;
	string herrdown;
	double msize,mdist,lwidth;
	vector<string>* yv_str;
	int marker;
	int smooth;
	int smoothm;
	int svg_smooth;           /* Savitski Golay filtering true=on */
	int svg_poly;             /* the type of polynomial 2,3,4,5...*/
	int svg_points;           /* the number of points 5,7,9,11.... */
	int svg_iter;             /* numb or time to do svg smoothing */
	int deresolve;            /* Only plot every N points: true = on */
	bool deresolve_avg;       /* dresolve + average points */
	int line_mode;
	int mdata;
	double mscale;
	bool line;
	double rx1,ry1,rx2,ry2;
	int layer_line;
	int layer_marker;
	int layer_error;
	GLEDataSetDimension dims[2];
	GLEArrayImpl m_data;
	GLEArrayImpl m_dataBackup;
public:
	GLEDataSet(int identifier);
	~GLEDataSet();
	void copy(GLEDataSet* other);
	void backup();
	void restore();
	void initBackup();
	void clearAll();
	bool undefined();
	GLEDataSetDimension* getDimXInv();
	GLEDataSetDimension* getDimYInv();
	GLEAxis* getAxis(int i);
	void clip(double *x, double *y);
	bool contains(double x, double y);
	bool contains(const GLEPoint& p);
	void checkRanges() throw(ParserError);
	void copyRangeIfRequired(int dimension);
	vector<int> getMissingValues();
	void validateDimensions();
	void validateNbPoints(unsigned int expectedNb, const char* descr = NULL);
	GLEArrayImpl* getDimData(unsigned int dim);
	void fromData(const vector<double>& xp, const vector<double>& yp, const vector<int>& miss);
	inline GLEDataSetDimension* getDim(int i) { return &dims[i]; }
	inline GLEArrayImpl* getData() { return &m_data; }
	inline GLEArrayImpl* getDataBackup() { return &m_dataBackup; }
};

#define GLE_DIM_X 0
#define GLE_DIM_Y 1

class bar_struct {
public:
	int ngrp;
	int from[20];
	int to[20];
	double width,dist;
	double lwidth[20];
	char lstyle[20][9];
	GLERC<GLEColor> fill[20];
	GLERC<GLEColor> color[20];
	GLERC<GLEColor> side[20];
	GLERC<GLEColor> top[20];
	int notop;
	double x3d,y3d;
	bool horiz;
	string style[20];
	int layer;
	bar_struct();
};

#ifdef __TURBOC__
#define MAXTEMP 2000
#else
#define MAXTEMP 10000
#endif

#ifdef GRAPHDEF
#else
#define GRAPHDEF
#endif

#define MAX_NB_FILL 100
#define MAX_NB_BAR 100

GRAPHDEF double graph_x1,graph_y1,graph_x2,graph_y2;  /* in cm */
GRAPHDEF double graph_xmin,graph_ymin,graph_xmax,graph_ymax; /* graph units */
GRAPHDEF char ebuff[400];
GRAPHDEF int etype,eplen;
GRAPHDEF double xbl,ybl;
GRAPHDEF double xlength,ylength;
GRAPHDEF double g_xsize,g_ysize,g_hscale,g_vscale,g_fontsz;
GRAPHDEF double last_vecx,last_vecy;
GRAPHDEF int ndata,g_nobox,g_center;
GRAPHDEF bool g_auto_s_h, g_auto_s_v;
GRAPHDEF bool g_math;
GRAPHDEF double sizex,sizey;
GRAPHDEF double vscale,hscale;
GRAPHDEF double g_discontinuityThreshold;
GRAPHDEF struct fill_data *fd[MAX_NB_FILL];
GRAPHDEF int nfd;
GRAPHDEF int gntmp;

GRAPHDEF GLEDataSet *dp[MAX_NB_DATA];

GRAPHDEF bar_struct *br[MAX_NB_BAR];
void vinit_axis(int i);
void vinit_title_axis();
void draw_bar(double x, double yf, double yt, double wd, bar_struct* barset, int di, GLEDataSet* toDataSet) throw(ParserError);
void get_dataset_ranges();
void set_bar_axis_places();
int get_dataset_identifier(const std::string& ds, bool def = false) throw(ParserError);
int get_dataset_identifier(const string& ds, GLEParser* parser, bool def) throw(ParserError);

double graph_bar_pos(double xpos, int bar, int set) throw(ParserError);
void begin_graph(GLEGraphBlockBase* graphBlockBase, GLEGraphBlockInstance* graphBlock) throw (ParserError);
bool execute_graph(GLESourceLine& sline, bool isCommandCheck, GLEGraphBlockInstance* graphBlock);
void begin_key(int *pln, int *pcode, int *cp) throw (ParserError);
void begin_tab(int *pln, int *pcode, int *cp);
void begin_text(int *pln, int *pcode, int *cp, double w, int just);
void draw_key(int nkd, struct offset_struct* koffset, char *kpos,double khei, int knobox);
string dimension2String(unsigned int dimension);

#define DP_CAST (struct data_struct*)
#define BR_CAST (struct bar_struct*)
#define AX_CAST (GLEAxis*)
#define FD_CAST (struct fill_data*)

class GLEZData;

class GLEToView {
public:
	GLEToView();
	virtual ~GLEToView();
	virtual GLEPoint fnXY(const GLEPoint& xy) = 0;
	virtual GLEPoint fnXYInv(const GLEPoint& xy) = 0;
};

class GLEToRectangularView: public GLEToView {
public:
	GLEToRectangularView();
	virtual ~GLEToRectangularView();
	virtual GLEPoint fnXY(const GLEPoint& xy);
	virtual GLEPoint fnXYInv(const GLEPoint& xy);
	void setXRange(double from, double to) { m_xRange.setMinMax(from, to); }
	void setYRange(double from, double to) { m_yRange.setMinMax(from, to); }
	void setOrigin(const GLEPoint& origin) { m_origin.set(origin); }
	void setSize(const GLEPoint& size) { m_size.set(size); }
private:
	GLERange m_xRange;
	GLERange m_yRange;
	GLEPoint m_origin;
	GLEPoint m_size;
};

class GLEToGraphView: public GLEToView {
public:
	GLEToGraphView(GLEAxis* xAxis, GLEAxis* yAxis);
	virtual ~GLEToGraphView();
	virtual GLEPoint fnXY(const GLEPoint& xy);
	virtual GLEPoint fnXYInv(const GLEPoint& xy);
private:
	GLEAxis* m_xAxis;
	GLEAxis* m_yAxis;
};

class GLEColorMap {
public:
	string m_function;
	string m_palette;
	int m_wd, m_hi;
	bool m_color;
	double m_zmin, m_zmax;
	bool m_has_zmin;
	bool m_has_zmax;
	bool m_invert;
	bool m_haspal;
	IpolType m_ipolType;
	GLEZData* m_Data;
public:
	GLEColorMap();
	~GLEColorMap();
	void draw(GLEToView* toView, double x0, double y0, double wd, double hi);
	void setZMin(double val);
	void setZMax(double val);
	void setPalette(const string& pal);
	void readData();
	inline bool hasZMin() { return m_has_zmin; }
	inline bool hasZMax() { return m_has_zmax; }
	inline double getZMin() { return m_zmin; }
	inline double getZMax() { return m_zmax; }
	inline void setFunction(const std::string& f) { m_function = f; }
	inline void setWidth(int wd) { m_wd = wd; }
	inline void setHeight(int hi) { m_hi = hi; }
	inline const string& getFunction() { return m_function; }
	inline int getWidth() { return m_wd; }
	inline int getHeight() { return m_hi; }
	inline bool isColor() { return m_color; }
	inline void setColor(bool color) { m_color = color; }
	inline void setInvert(bool inv) { m_invert = inv; }
	inline bool isInverted() { return m_invert; }
	inline bool hasPalette() { return m_haspal; }
	inline const string& getPaletteFunction() { return m_palette; }
	inline GLEZData* getData() { return m_Data; }
	inline void setIpolType(IpolType type) { m_ipolType = type; }
	inline IpolType getIpolType() const { return m_ipolType; }
};


/**
 * Layers:
 *
 * 	- background (fill)
 * 	- color map
 * 	- grid
 * 	- fills (e.g., fill x1,d1 color gray50)
 *		 struct fill_data {
 *			int da,db;
 *			int type;
 *			int color;
 *			double xmin,ymin,xmax,ymax;
 *		 };
 * 	- bars (e.g., bar d3 from d1 width 0.6 fill gray30)
 *       class bar_struct {
 *	        int ngrp;
 *	        int from[20];
 *	        int to[20];
 *	        double width,dist;
 *          ...
 * 	- axis
 * 	- lines
 * 	- error bars
 * 	- markers
 * 	- key
 */
