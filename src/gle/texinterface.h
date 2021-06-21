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

/*
 * (c) 2004 Jan Struyf
 *
 */

#ifndef INCLUDE_TEXINTERFACE
#define INCLUDE_TEXINTERFACE

#define FONT_HEI_FACTOR 1.46199

#define TEX_SCALE_MODE_NONE   0
#define TEX_SCALE_MODE_FIXED  1
#define TEX_SCALE_MODE_SCALE  2

class GLERectangle;
class TeXInterface;

class TeXHashObject {
protected:
	string m_Line;
	bool m_Used;
	int m_HasDimensions, m_NbLines;
	double m_Width, m_Height, m_Baseline;
public:
	TeXHashObject(const string& line);
	void outputMeasure(ostream& os);
	void outputLog(ostream& os);
	void outputLines(ostream& os);
	void addFirstLine(string* str);
	void setDimension(double width, double height, double baseline);
	inline const string& getLine() { return m_Line; }
	inline int hasDimensions() { return m_HasDimensions; }
	inline bool isUsed() { return m_Used; }
	inline void setUsed(bool used) { m_Used = used; }
	inline double getWidth() { return m_Width; }
	inline double getHeight() { return m_Height; }
	inline double getBaseline() { return m_Baseline; }
	inline int getNbLines() { return m_NbLines; }
	inline void setNbLines(int nb) { m_NbLines = nb; }
};

#define TEX_OBJ_INF_HAS_COLOR    1
#define TEX_OBJ_INF_HAS_JUSTIFY  2
#define TEX_OBJ_INF_HAS_POSITION 4
#define TEX_OBJ_INF_DONT_PRINT   8

class TeXObjectInfo {
protected:
	int m_Status;
	GLERC<GLEColor> m_Color;
	int m_Just;
	double m_Xp, m_Yp;
public:
	TeXObjectInfo();
	void setJustify(int just);
	void setPosition(double xp, double yp);
	void setColor(const GLERC<GLEColor>& color);
	void initializeAll();
	inline void setFlag(int flag) { m_Status |= flag; }
	inline int getFlags() { return m_Status; }
	inline int getJustify() { return m_Just; }
	inline double getXp() { return m_Xp; }
	inline double getYp() { return m_Yp; }
	inline GLEColor* getColor() { return m_Color.get(); }
};

class TeXObject {
protected:
	double m_Xp, m_Yp, m_DXp, m_DYp, m_Angle;
	TeXHashObject* m_Object;
	GLERC<GLEColor> m_Color;
public:
	TeXObject();
	void output(ostream& os);
	int isBlack();
	void getDimensions(double* x1, double *y1, double *x2, double *y2);
	inline void setAngle(double angle) { m_Angle = angle; }
	inline double getAngle() { return m_Angle; }
	inline void setXY(double x, double y) { m_Xp = x; m_Yp = y; }
	inline void setDeviceXY(double x, double y) { m_DXp = x; m_DYp = y; }
	inline void setObject(TeXHashObject* obj) { m_Object = obj; }
	inline 	TeXHashObject* getObject() { return m_Object; }
	inline double getWidth() { return m_Object->getWidth(); }
	inline double getHeight() { return m_Object->getHeight(); }
	inline double getDXp() { return m_DXp; }
	inline double getDYp() { return m_DYp; }
	inline const string& getLine() { return m_Object->getLine(); }
	inline int hasObject() { return m_Object != NULL; }
	inline void setColor(const GLERC<GLEColor>& color) { m_Color = color; }
	inline GLEColor* getColor() { return m_Color.get(); }
};

class TeXSize {
protected:
	string m_Name;
public:
	TeXSize(const char* name);
	void createObject(string* name);
	inline const string& getName() { return m_Name; }
};

class TeXPreambleKey {
protected:
	string m_DocumentClass;
	vector<string> m_Preamble;
public:
	bool equals(const TeXPreambleKey* key) const;
	void copyFrom(const TeXPreambleKey* other);
	inline void clear() { m_Preamble.clear(); }
	inline void setDocumentClass(const string& line) { m_DocumentClass = line; }
	inline const string& getDocumentClass() const { return m_DocumentClass; }
	inline int getNbPreamble() const { return m_Preamble.size(); }
	inline const string& getPreamble(int i) const { return m_Preamble[i]; }
	inline void addPreamble(const string& str) { m_Preamble.push_back(str); }
};

class TeXPreambleInfo : public TeXPreambleKey {
	bool m_HasFontSizes;
	vector<double> m_FontSizes;
public:
	TeXPreambleInfo();
	void setFontSize(int font, double size);
	double getFontSize(int font);
	void save(ostream& os);
	void load(istream& is, TeXInterface* iface);
	int getBestSizeScaled(double hei);
	int getBestSizeFixed(double hei);
	inline int getNbFonts() { return m_FontSizes.size(); }
	inline bool hasFontSizes() { return m_HasFontSizes; }
	inline void setHasFontSizes(bool hasf) { m_HasFontSizes = hasf; }
};

class TeXPreambleInfoList {
protected:
	TeXPreambleInfo* m_Current;
	vector<TeXPreambleInfo*> m_Infos;
public:
	TeXPreambleInfoList();
	~TeXPreambleInfoList();
	void save(const string& filestem);
	void load(const string& filestem, TeXInterface* iface);
	TeXPreambleInfo* findOrAddPreamble(const TeXPreambleKey* pre_key);
	inline int getNbPreambles() { return m_Infos.size(); }
	inline TeXPreambleInfo* getPreamble(int i) { return m_Infos[i]; }
	inline void addPreamble(TeXPreambleInfo* preamble) { m_Infos.push_back(preamble); }
	inline TeXPreambleInfo* getCurrent() { return m_Current; }
	inline void selectDefault() { m_Current = m_Infos[0]; }
	inline void select(TeXPreambleInfo* value) { m_Current = value; }
};

class TeXHash : public vector<TeXHashObject*> {
public:
	TeXHash();
	~TeXHash();
	void cleanUp();
	void loadTeXPS(const string& filestem);
	void saveTeXPS(const string& filestem, TeXInterface* iface);
	TeXHashObject* getHashObject(int idx);
	TeXHashObject* getHashObjectOrNULL(const string& line);
	inline TeXHashObject* get(int i) { return (*this)[i]; }
};

#define TEX_INTERFACE_HASH_LOADED_NONE    0
#define TEX_INTERFACE_HASH_LOADED_PARTIAL 1
#define TEX_INTERFACE_HASH_LOADED_FULL    2

class TeXInterface {
protected:
	static TeXInterface m_Instance;
	vector<TeXObject*> m_TeXObjects;
	TeXHash m_TeXHash;
	vector<TeXSize*> m_FontSizes;
	TeXPreambleInfoList m_Preambles;
	string m_HashName;
	string m_DotDir;
	GLEFileLocation m_MainOutputName;
	int m_ScaleMode;
	int m_HashLoaded, m_HashModified;
	bool m_HasFontSizes;
	bool m_Enabled;
public:
	TeXInterface();
	~TeXInterface();
	TeXObject* draw(const char* str) throw(ParserError);
	TeXObject* drawUTF8(const char* str, GLERectangle* box = NULL) throw(ParserError);
	TeXObject* draw(const std::string& str, GLERectangle* box) throw(ParserError);
	TeXObject* draw(const char* str, int nblines, GLERectangle* box = NULL) throw(ParserError);
	TeXObject* draw(const char* str, TeXObjectInfo& info, int nblines, GLERectangle* box = NULL) throw(ParserError);
	TeXObject* drawObj(TeXHashObject* hobj, TeXObjectInfo& info, GLERectangle* box = NULL) throw(ParserError);
	void scaleObject(string& obj_str, double hei);
	void checkObjectDimensions();
	int createObj(const char* str, double hei);
	void initialize(GLEFileLocation* dotfile, GLEFileLocation* oname);
	void updateNames(GLEFileLocation* dotfile, GLEFileLocation* oname);
	void updateOutName(GLEFileLocation* oname);
	void reset();
	int tryCreateHash();
	void removeDotFiles();
	void createInc(const string& prefix);
	void createTeX(bool usegeom);
	int getHashObjectIndex(const string& line);
	TeXHashObject* getHashObject(const string& line);
	TeXHashObject* getHashObject(int idx);
	void resetPreamble();
	void createPreamble(ostream& tex_file);
	inline void setHasFontSizes(bool has) { m_HasFontSizes = has; }
	inline bool hasFontSizes() { return m_HasFontSizes; }
	inline bool hasObjects() { return m_TeXObjects.size() != 0; }
	inline int getNbPreamble() { return m_Preambles.getCurrent()->getNbPreamble(); }
	inline const string& getPreamble(int i) { return m_Preambles.getCurrent()->getPreamble(i); }
	inline const string& getDocumentClass() { return m_Preambles.getCurrent()->getDocumentClass(); }
	inline TeXPreambleInfoList* getPreambles() { return &m_Preambles; }
	inline void addSize(TeXSize* size) { m_FontSizes.push_back(size); }
	inline int getNbFontSizes() { return m_FontSizes.size(); }
	inline TeXSize* getFontSize(int i) { return m_FontSizes[i]; }
	inline TeXPreambleInfo* getCurrentPreamble() { return m_Preambles.getCurrent(); }
	inline int getScaleMode() { return m_ScaleMode; }
	inline void setScaleMode(int mode) { m_ScaleMode = mode; }
	static inline TeXInterface* getInstance() { return &m_Instance; }
	inline bool isEnabled() { return m_Enabled; }
	inline void setEnabled(bool ena) { m_Enabled = ena; }
protected:
	void writeInc(ostream& out, const char* prefix);
	void tryLoadHash();
	void loadTeXLines();
	void createHiddenDir();
	void saveTeXLines();
	bool createTeXPS();
	bool createTeXPS(const string& filestem);
	void addHashObject(TeXHashObject* obj);
	void cleanUpObjects();
	void cleanUpHash();
	void initTeXFontScales();
	void checkTeXFontSizes();
	void retrieveTeXFontSizes(TeXHash& tex_hash, TeXPreambleInfo* preamble);
};

bool create_eps_file_latex_dvips(const string& fname, GLEScript* script);
bool create_ps_file_latex_dvips(const string& fname);
bool create_pdf_file_pdflatex(const string& fname, GLEScript* script);

bool create_bitmap_file(GLEFileLocation* fname, int device, int dpi, int options, GLEScript* script);
bool create_pdf_file_ghostscript(GLEFileLocation* fname, int dpi, GLEScript* script);

bool run_ghostscript(const string& args, const string& outfile, bool redirout = true, istream* is = NULL);
bool run_latex(const string& dir, const string& file);
bool run_dvips(const string& file, bool eps);

#endif
