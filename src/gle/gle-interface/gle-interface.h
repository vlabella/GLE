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

#ifndef __GLE_INTERFACE__
#define __GLE_INTERFACE__

#undef DLLEXPORT
#undef DLLIMPORT

#ifndef __WIN32__
#ifdef Q_OS_WIN32
#define __WIN32__
#endif
#endif

#if defined(Q_OS_WIN32) && !defined(_MSC_VER)
#define INCLUDE_GLE_DLL
#endif

#if defined(__WIN32__) && defined(INCLUDE_GLE_DLL)
	#define DLLEXPORT __declspec( dllexport )
	#define DLLIMPORT __declspec( dllimport )
	#ifndef _MSC_VER
	#define DLLCLASSEXPORT __attribute__ ((dllexport))
	#define DLLCLASSIMPORT __attribute__ ((dllimport))
	#else
	// msvc does not have __attribute__
	#define DLLCLASSEXPORT __declspec( dllexport )
	#define DLLCLASSIMPORT __declspec( dllimport )
	#endif
#else
	#define DLLEXPORT
	#define DLLIMPORT
	#define DLLCLASSEXPORT
	#define DLLCLASSIMPORT
#endif

#ifdef INCLUDE_GLE_DLL
#ifndef _MSC_VER
#define DLLFCT DLLIMPORT
#else
// in msvc  class function delcared as ddlimport or export do not need to be declared dllepxort or dllimport
#define DLLFCT
#endif
#define DLLCLASS DLLCLASSIMPORT
#else
#ifndef _MSC_VER
#define DLLFCT DLLEXPORT
#else
// in msvc  class function delcared as ddlimport or export do not need to be declared dllepxort or dllimport
#define DLLFCT
#endif
#define DLLCLASS DLLCLASSEXPORT
#endif

#include <string>
#include <vector>

using namespace std;

#include "gle-base.h"
#include "gle-const.h"

// what do we do with these?
#include "../tokens/RefCount.h"
#include "../tokens/StringKeyHash.h"
#include "../tokens/Tokenizer.h"

#define GDO_FLAG_DELETED  1
#define GDO_FLAG_MODIFIED 2

typedef void (*gle_write_func)(void* closure, char* data, int length);

class GLEInterface;
class GLEScript;
class GLEFont;
class GLEColor;
class GLEDrawObject;
class GLETextDO;
class GLEStringSettings;
class GLEOutputStream;
class GLEProperty;
class GLEPropertyStore;
class GLEPropertyStoreModel;
class GLEGlobalConfig;
class CmdLineObj;
class StringIntHash;
class IntIntHash;
class GLEPoint;
class GLESub;
class GLERun;
class GLEParser;
class GLEPolish;
class GLEPcodeIndexed;
class GLERangeSet;

unsigned char float_to_color_comp(double value);

extern "C" {

	// Note: (currently) GLEInterface is a singleton class there is at most one instance
	DLLFCT GLEInterface* GLEGetInterfacePointer();

	// GLEInterface* iface = GLEGetInterfacePointer();
	// GLEGlobalConfig* conf = GLEGetInterfacePointer()->getConfig();

	DLLEXPORT int GLEMain(int argc, char **argv);
};

enum GLEJustify {
	GLEJustifyCC   = 0x011,  GLEJustifyTL     = 0x002,  GLEJustifyTC    = 0x012,
	GLEJustifyTR   = 0x022,  GLEJustifyBL     = 0x000,  GLEJustifyBC    = 0x010,
	GLEJustifyBR   = 0x020,  GLEJustifyLC     = 0x001,  GLEJustifyRC    = 0x021,
	GLEJustifyLeft = 0x100,  GLEJustifyCenter = 0x110,  GLEJustifyRight = 0x120,
	GLEJustifyCirc = 0x1011, GLEJusitfyBox    = 0x5011, GLEJustifyVert  = 0x2000,
	GLEJustifyHorz = 0x3000

// add LH CH RH BV CV TV
};

class GLERange {
protected:
	double m_Min, m_Max;
public:
	GLERange();
	~GLERange();
	void setMinMax(double min, double max);
	void copy(GLERange* other);
	void copyHas(GLERangeSet* other);
	void initRange();
	bool isMinValid() const;
	bool isMaxValid() const;
	void updateRange(double value);
	void clip(double* value);
	bool contains(double value);
	ostream& printRange(ostream& out) const;
	inline double getMin() { return m_Min; }
	inline double getMax() { return m_Max; }
	inline double getWidth() { return m_Max - m_Min; }
	inline void setMin(double min) { m_Min = min; }
	inline void setMax(double max) { m_Max = max; }
	inline bool valid() { return m_Min <= m_Max; }
	inline bool validNotEmpty() { return m_Min < m_Max; }
	inline bool invalid() { return m_Min > m_Max; }
	inline bool invalidOrEmpty() { return m_Min >= m_Max; }
	inline void updateRange(double value, bool miss) { if (!miss) updateRange(value); }
};

inline ostream& operator<<(ostream& os, const GLERange& range) {
	return range.printRange(os);
}

class GLERangeSet : public GLERange {
protected:
	bool m_MinSet, m_MaxSet;
public:
	GLERangeSet();
	~GLERangeSet();
	void setMinSet(double min);
	void setMaxSet(double max);
	void setMinMaxSet(double min, double max);
	void setMinIfNotSet(double min);
	void setMaxIfNotSet(double max);
	void resetSet();
	void copySet(GLERangeSet* other);
	void copyIfNotSet(GLERange* other);
	void initRangeIfNotSet();
	inline bool hasMin() { return m_MinSet; }
	inline bool hasMax() { return m_MaxSet; }
	inline bool hasBoth() { return m_MinSet && m_MaxSet; }
	inline bool hasOne() { return m_MinSet || m_MaxSet; }
};

class DLLCLASS GLERectangle {
protected:
	double m_XMin, m_YMin, m_XMax, m_YMax;
public:
	GLERectangle();
	GLERectangle(double xmin, double ymin, double xmax, double ymax);
	GLERectangle(GLERectangle* other);
	~GLERectangle();
	inline double getXMin() const { return m_XMin; }
	inline double getXMax() const { return m_XMax; }
	inline double getYMin() const { return m_YMin; }
	inline double getYMax() const { return m_YMax; }
	inline void setXMin(double v) { m_XMin = v; }
	inline void setXMax(double v) { m_XMax = v; }
	inline void setYMin(double v) { m_YMin = v; }
	inline void setYMax(double v) { m_YMax = v; }
	inline double getWidth() const { return m_XMax - m_XMin; }
	inline double getHeight() const { return m_YMax - m_YMin; }
	inline double getXMid() const { return (m_XMin+m_XMax)/2; }
	inline double getYMid() const { return (m_YMin+m_YMax)/2; }
	inline bool isValid() const { return m_XMin <= m_XMax; }
	inline bool isPoint() const { return m_XMin == m_XMax && m_YMin == m_YMax; }
	void reset();
	void normalize();
	void copy(GLERectangle* other);
	void copy(GLEPoint* point);
	void getDimensions(double* xmin, double* ymin, double* xmax, double* ymax);
	void setDimensions(double xmin, double ymin, double xmax, double ymax);
	void translate(double x, double y);
	void translate(GLEPoint* p);
	void scale(double s);
	void subtractXFrom(double x);
	void subtractYFrom(double y);
	void grow(double d);
	bool contains(double x, double y);
	void initRange();
	void updateRange(double x, double y);
	void updateRange(GLEPoint* pt);
	void addToRangeX(GLERange* range);
	void addToRangeY(GLERange* range);
	ostream& print(ostream& out) const;
	void toPoint(GLEJustify just, GLEPoint* pt);
};

inline ostream& operator<<(ostream& os, const GLERectangle& rect) {
	return rect.print(os);
}

class DLLCLASS GLEPoint {
public:
	double m_X, m_Y;
public:
	GLEPoint();
	GLEPoint(double x, double y);
	GLEPoint(const GLEPoint& p);
	~GLEPoint();
	inline double getX() const { return m_X; }
	inline double getY() const { return m_Y; }
	inline void setXY(double x, double y) { m_X = x; m_Y = y; }
	inline void setX(double x) { m_X = x; }
	inline void setY(double y) { m_Y = y; }
	inline void set(const GLEPoint& p) { m_X = p.m_X; m_Y = p.m_Y; }
	inline void dotScalar(double s) { m_X *= s; m_Y *= s; }
	inline void addScalar(const GLEPoint& p, double s) { m_X += s*p.m_X; m_Y += s*p.m_Y; }
	inline void subtractFrom(const GLEPoint* p) { m_X = p->m_X - m_X; m_Y = p->m_Y - m_Y; }
	inline void add(const GLEPoint& p) { m_X += p.m_X; m_Y += p.m_Y; }
	inline void add(double x, double y) { m_X += x; m_Y += y; }
	inline void invert() { m_X *= -1; m_Y *= -1; }
	inline bool approx(const GLEPoint& p) { return approx(p.getX(), p.getY()); }
	DLLFCT void add(double s, const GLEPoint& p);
	DLLFCT double distance(const GLEPoint& p) const;
	DLLFCT double norm() const;
	DLLFCT double normSq() const;
	DLLFCT void normalize();
	DLLFCT bool approx(double x, double y);
	DLLFCT void swap(GLEPoint& other);
	DLLFCT ostream& write(ostream& os) const;
};

inline ostream& operator<<(ostream& os, const GLEPoint& pt) {
	return pt.write(os);
}

#include "gle-shapemath.h"

class DLLCLASS GLEPoint3D {
public:
	double m_C[3];
public:
	GLEPoint3D();
	GLEPoint3D(double x, double y, double z);
	GLEPoint3D(const GLEPoint3D& p);
	inline double get(int i) const { return m_C[i]; }
	inline void set(int i, double v) { m_C[i] = v; }
	inline void set(double x, double y, double z) { m_C[0] = x; m_C[1] = y; m_C[2] = z; }
	DLLFCT void dotScalar(double s);
	DLLFCT void add(const GLEPoint3D& p);
	DLLFCT void subtract(const GLEPoint3D& p);
	DLLFCT void addScalar(double s1, double s2, const GLEPoint3D& p);
	DLLFCT double norm() const;
	DLLFCT void normalize();
	void ortho3DUnit(const GLEPoint3D& p, GLEPoint3D* r);
	DLLFCT ostream& write(ostream& os) const;
};

class DLLCLASS GLEMatrix {
public:
	double* m_C;
	int m_Rows, m_Cols;
public:
	GLEMatrix(int rows, int cols);
	GLEMatrix(const GLEMatrix& p);
	~GLEMatrix();
	void dot(const GLEPoint3D& p, GLEPoint3D* r) const;
	void setVertVector(int row, int col, const GLEPoint3D& p);
	DLLFCT ostream& write(ostream& os) const;
};

class DLLCLASS GLEProjection {
public:
	GLEPoint3D m_Eye, m_Reference, m_VVector;
public:
	GLEProjection();
	inline GLEPoint3D* getEye() { return &m_Eye; }
	inline GLEPoint3D* getReference() { return &m_Reference; }
	inline GLEPoint3D* getV() { return &m_VVector; }
	void invToReference(GLEMatrix* mtrx);
	DLLFCT void zoom(double factor);
	DLLFCT void rotate(double angle, bool horiz);
	DLLFCT void reference(const GLEPoint3D& ref);
	DLLFCT void adjustV(double angle);
};

class DLLCLASS GLELinearEquation {
protected:
	double m_A, m_B;
public:
	GLELinearEquation();
	~GLELinearEquation();
	void DLLFCT fit(double x0, double y0, double x1, double y1);
	inline double intersect(GLELinearEquation* o) { return (o->m_B - m_B)/(m_A - o->m_A); }
	inline void fitB(double x0, double y0) { m_B = y0 - m_A*x0; }
	inline void setA(double a) { m_A = a; }
	inline void setB(double b) { m_B = b; }
	inline double apply(double x) const { return m_A*x + m_B; }
	inline double getA() const { return m_A; }
	inline double getB() const { return m_B; }
};

class DLLCLASS GLELineSegment {
private:
	GLEPoint m_p1;
	GLEPoint m_p2;

public:
	GLELineSegment(const GLEPoint& p1, const GLEPoint& p2);
	GLELineSegment(double x1, double y1, double x2, double y2);
	const GLEPoint& getP1() const { return m_p1; }
	const GLEPoint& getP2() const { return m_p2; }
};

#include "gle-datatype.h"

enum GLEFontStyle {
	GLEFontStyleRoman, GLEFontStyleBold, GLEFontStyleItalic, GLEFontStyleBoldItalic
};

class GLEFont : public GLEDataObject {
protected:
	string m_Name;
	string m_FullName;
	GLERC<GLEFont> m_Bold;
	GLERC<GLEFont> m_Italic;
	GLERC<GLEFont> m_BoldItalic;
	GLEFont* m_Parent;
	int m_Index;
	int m_Number;
public:
	GLEFont();
	~GLEFont();
	// Return the name of the font
	inline const string& getName() { return m_Name; }
	inline const string& getFullName() { return m_FullName; }
	inline const char* getNameC() { return m_Name.c_str(); }
	inline const char* getFullNameC() { return m_FullName.c_str(); }
	inline void setName(const string& name) { m_Name = name; }
	inline void setFullName(const string& name) { m_FullName = name; }
	inline bool hasStyle(GLEFontStyle style) { return getStyle(style) != NULL; }
	DLLFCT GLEFontStyle checkStyle(GLEFont* child);
	DLLFCT GLEFont* getStyle(GLEFontStyle style);
	void setStyle(GLEFontStyle style, GLEFont* font);
	inline int getIndex() { return m_Index; }
	inline void setIndex(int value) { m_Index = value; }
	inline int getNumber() { return m_Number; }
	inline void setNumber(int value) { m_Number = value; }
	inline GLEFont* getBaseFont() { return m_Parent != 0 ? m_Parent : this; }
	inline void setParent(GLEFont* parent) { m_Parent = parent; }
};

enum GLEFillType {
	GLE_FILL_TYPE_PATTERN
};

class GLEFillBase : public GLERefCountObject {
public:
	GLEFillBase();
	virtual ~GLEFillBase();
	virtual GLEFillType getFillType() = 0;
	virtual GLEFillBase* clone() = 0;
};

class GLEPatternFill : public GLEFillBase {
public:
	GLEPatternFill(int fillDescr);
	virtual ~GLEPatternFill();
	virtual GLEFillType getFillType();
	virtual GLEFillBase* clone();

	inline void setFillDescription(int fillDescription) { m_fillDescription = fillDescription; }
	inline int getFillDescription() const { return m_fillDescription; }
	inline void setBackground(GLEColor* color) { m_background = color; }
	inline GLEColor* getBackground() const { return m_background.get(); }

private:
	int m_fillDescription;
	GLERC<GLEColor> m_background;
};

class DLLCLASS GLEColor : public GLEDataObject {
protected:
	bool m_Transparent;
	double m_Red, m_Green, m_Blue, m_Alpha;
	string* m_Name;
	GLERC<GLEFillBase> m_Fill;
public:
	GLEColor();
	GLEColor(double r, double g, double b);
	GLEColor(double r, double g, double b, double a);
	explicit GLEColor(double gray);
	~GLEColor();
	virtual int getType() const;
	virtual bool equals(GLEDataObject* obj) const;
	virtual void print(ostream& out) const;
	bool equalsApprox(GLEColor* other);
	DLLFCT void setRGB(double r, double g, double b);
	DLLFCT void setRGBA(double r, double g, double b, double a);
	DLLFCT void setRGB255(int r, int g, int b);
    DLLFCT void setRGB255(double r, double g, double b);
    void setRGBA255(double r, double g, double b, double a);
	void setGray(double gray);
	DLLFCT const char* getName();
	void setName(const string& name);
	void setName(const string* name);
	void setHexValue(unsigned int v);
	void setDoubleEncoding(double v);
	double getDoubleEncoding();
	void setHexValueGLE(unsigned int hexValue);
	unsigned int getHexValueGLE();
	double getGray();
	GLEColor* clone();
	inline double getRed() const { return m_Red; }
	inline double getGreen() const { return m_Green; }
	inline double getBlue() const { return m_Blue; }
	inline double getAlpha() const { return m_Alpha; }
	inline bool hasAlpha() const { return float_to_color_comp(m_Alpha) != 255; }
	inline unsigned char getRedI() const { return float_to_color_comp(m_Red); }
	inline unsigned char getGreenI() const { return float_to_color_comp(m_Green); }
	inline unsigned char getBlueI() const { return float_to_color_comp(m_Blue); }
	inline unsigned char getAlphaI() const { return float_to_color_comp(m_Alpha); }
	inline void setRed(double v) { m_Red = v; }
	inline void setGreen(double v) { m_Green = v; }
	inline void setBlue(double v) { m_Blue = v; }
	inline bool isTransparent() const { return m_Transparent; }
	inline void setTransparent(bool transp) { m_Transparent = transp; }
	inline bool isFill() const { return !m_Fill.isNull(); }
	inline GLEFillBase* getFill() { return m_Fill.get(); }
	inline void setFill(GLEFillBase* fill) { m_Fill = fill; }
	inline std::string* getNameS() { return m_Name; }
};

GLERC<GLEColor> color_or_fill_from_int(int hexValue);
void update_color_foreground(GLEColor* updateMe, GLEColor* color);
void update_color_foreground_and_pattern(GLEColor* updateMe, GLEColor* color);
void update_color_fill_pattern(GLEColor* updateMe, GLEPatternFill* fill);
void update_color_fill_background(GLEColor* updateMe, GLEColor* color);
GLERC<GLEColor> get_fill_background(GLEColor* fill);
GLERC<GLEColor> get_fill_foreground(GLEColor* fill);

// Each drawable object has a reference to a property store
// To, e.g., get the line width of a given object, call:
//
//       double lwidth = obj->getProperties()->getRealProperty(GDOPLineWidth);
//
// It is also possible to enumerate all properties of an object:
//
//       GLEPropertyStoreModel* model = obj->getProperties()->getModel();
//       for (int i = 0; i < model->getNumberOfProperties(); i++) {
//             GLEProperty* prop = model->getProperty(i);
//             // do something with prop, e.g., get name, type, ...
//       }
//
// All objects of the same type share the same model
//
// Should two objects with the same properties share the same GLEPropertyStore object?
// (For efficiency reasons?) Then we have to decide how to set property values.
//
// Or should there be a kind of invisible "set property object"
// Similar to the "set" command in GLE scripts ?

enum GLEPropertyType {
	GLEPropertyTypeInt, GLEPropertyTypeBool, GLEPropertyTypeReal, GLEPropertyTypeString, GLEPropertyTypeColor, GLEPropertyTypeFont
};

enum GLEPropertyID {
	GLEDOPropertyColor, GLEDOPropertyFillColor, GLEDOPropertyJustify,
	GLEDOPropertyLineWidth, GLEDOPropertyLineStyle, GLEDOPropertyLineCap,
	GLEDOPropertyFont, GLEDOPropertyFontSize, GLEDOPropertyArrowSize,
	GLEDOPropertyArrowAngle, GLEDOPropertyArrowStyle, GLEDOPropertyArrowTip,
	GLEDOPropertyUserArg
};

enum GLELineCap {
	GLELineCapButt = 0,
	GLELineCapRound = 1,
	GLELineCapSquare = 2
};

enum GLEArrowStyle {
	GLEArrowStyleSimple = 0,
	GLEArrowStyleFilled = 1,
	GLEArrowStyleEmpty = 2,
	GLEArrowStyleSub = 10
};

enum GLEArrowTip {
	GLEArrowTipRound = 0,
	GLEArrowTipSharp = 1
};

class GLEProperty {
private:
	string m_Name;
	const char* m_SetCmdName;
	GLEPropertyType m_Type;
	GLEPropertyID m_ID;
	int m_Index;
public:
	GLEProperty(const char* name, GLEPropertyType type, GLEPropertyID id);
	GLEProperty(const char* name, const char* cmdname, GLEPropertyType type, GLEPropertyID id);
	virtual ~GLEProperty();
	inline GLEPropertyType getType() { return m_Type; }
	inline GLEPropertyID getID() { return m_ID; }
	inline const string& getName() { return m_Name; }
	inline const char* getNameC() { return m_Name.c_str(); }
	inline void setSetCommandName(const char* cmdname) { m_SetCmdName = cmdname; }
	inline const char* getSetCommandName() { return m_SetCmdName; }
	inline void setIndex(int idx) { m_Index = idx; }
	inline int getIndex() { return m_Index; }
	virtual void getPropertyAsString(string* result, GLEMemoryCell* value);
	virtual void createSetCommandGLECode(ostream& os, GLEMemoryCell* value);
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyNominal : public GLEProperty {
private:
	IntIntHash* m_Value2Name;
	StringIntHash* m_Name2Value;
	vector<string> m_NomValues;
public:
	GLEPropertyNominal(const char* name, GLEPropertyType type, GLEPropertyID id);
	virtual ~GLEPropertyNominal();
	void addValue(const char* name, int value);
	virtual void getPropertyAsString(string* result, GLEMemoryCell* value);
};

class GLEPropertyLWidth : public GLEProperty {
public:
	GLEPropertyLWidth(const char* name);
	virtual ~GLEPropertyLWidth();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyLStyle : public GLEProperty {
public:
	GLEPropertyLStyle(const char* name);
	virtual ~GLEPropertyLStyle();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyColor : public GLEProperty {
public:
	GLEPropertyColor(const char* name);
	virtual ~GLEPropertyColor();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyHei : public GLEProperty {
public:
	GLEPropertyHei(const char* name);
	virtual ~GLEPropertyHei();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyFont : public GLEProperty {
public:
	GLEPropertyFont(const char* name);
	virtual ~GLEPropertyFont();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyJustify : public GLEProperty {
public:
	GLEPropertyJustify(const char* name);
	virtual ~GLEPropertyJustify();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
	virtual void getPropertyAsString(string* result, GLEMemoryCell* value);
};

class GLEPropertyFillColor : public GLEProperty {
public:
	GLEPropertyFillColor(const char* name);
	virtual ~GLEPropertyFillColor();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyArrowSize : public GLEProperty {
public:
	GLEPropertyArrowSize(const char* name);
	virtual ~GLEPropertyArrowSize();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class GLEPropertyArrowAngle : public GLEProperty {
public:
	GLEPropertyArrowAngle(const char* name);
	virtual ~GLEPropertyArrowAngle();
	virtual bool isEqualToState(GLEPropertyStore* store);
	virtual void updateState(GLEPropertyStore* store);
};

class DLLCLASS GLEPropertyStoreModel : public GLERefCountObject {
protected:
	GLEVectorAutoDelete<GLEProperty> m_Properties;
	IntIntHash* m_Hash;
	bool m_CanScale;
	int m_NbExtra;
public:
	GLEPropertyStoreModel();
	virtual ~GLEPropertyStoreModel();
	inline bool isSupportScale() { return m_CanScale; }
	inline int getNumberOfExtraProperties() { return m_NbExtra; }
	inline int getNumberOfProperties() { return m_Properties.size(); }
	inline GLEProperty* getProperty(int idx) { return m_Properties[idx]; }
	inline bool hasProperty(GLEPropertyID id) { return find(id) != -1; }
	DLLFCT int find(GLEPropertyID id);
	void add(GLEProperty* prop);
	virtual void scale(GLEDrawObject* obj, double sx, double sy);
};

class DLLCLASS GLEPropertyStore {
protected:
	GLEArrayImpl m_Values;
	GLERC<GLEPropertyStoreModel> m_Model;
public:
	GLEPropertyStore(GLEPropertyStoreModel* model);
	~GLEPropertyStore();
	inline GLEPropertyStoreModel* getModel() { return m_Model.get(); }
	inline int getIntProperty(GLEPropertyID id) { return m_Values.getInt(m_Model->find(id)); }
	inline bool getBoolProperty(GLEPropertyID id) { return m_Values.getBool(m_Model->find(id)); }
	inline double getRealProperty(GLEPropertyID id) { return m_Values.getDouble(m_Model->find(id)); }
	inline GLEFont* getFontProperty(GLEPropertyID id) { return (GLEFont*)m_Values.getObject(m_Model->find(id)); }
	inline GLEColor* getColorProperty(GLEPropertyID id) { return (GLEColor*)m_Values.getObject(m_Model->find(id)); }
	inline GLEString* getStringProperty(GLEPropertyID id) { return (GLEString*)m_Values.getObject(m_Model->find(id)); }
	inline void setIntProperty(GLEPropertyID id, int value) { m_Values.setInt(m_Model->find(id), value); }
	inline void setBoolProperty(GLEPropertyID id, bool value) { m_Values.setBool(m_Model->find(id), value); }
	inline void setRealProperty(GLEPropertyID id, double value) { m_Values.setDouble(m_Model->find(id), value); }
	inline void setFontProperty(GLEPropertyID id, GLEFont* value) { m_Values.setObject(m_Model->find(id), value); }
	inline void setColorProperty(GLEPropertyID id, GLEColor* value) { m_Values.setObject(m_Model->find(id), value); }
	inline void setStringProperty(GLEPropertyID id, GLEString* value) { m_Values.setObject(m_Model->find(id), value); }
	inline int getNumberOfProperties() { return m_Model->getNumberOfProperties(); }
	inline GLEProperty* getProperty(int idx) { return m_Model->getProperty(idx); }
	inline GLEMemoryCell* getPropertyValue(int idx) { return m_Values.get(idx); }
	inline void setPropertyValue(int idx, const GLEMemoryCell* value) { m_Values.set(idx, value); }
	inline GLEMemoryCell* getPropertyValue(GLEProperty* prop) { return m_Values.get(prop->getIndex()); }
	inline int getIntProperty(GLEProperty* prop) { return m_Values.getInt(prop->getIndex()); }
	inline double getRealProperty(GLEProperty* prop) { return m_Values.getDouble(prop->getIndex()); }
	inline void setRealProperty(GLEProperty* prop, double value) { m_Values.setDouble(prop->getIndex(), value); }
	inline GLEColor* getColorProperty(GLEProperty* prop) { return (GLEColor*)m_Values.getObject(prop->getIndex()); }
	inline GLEFont* getFontProperty(GLEProperty* prop) { return (GLEFont*)m_Values.getObject(prop->getIndex()); }
	inline GLEString* getStringProperty(GLEProperty* prop) { return (GLEString*)m_Values.getObject(prop->getIndex()); }
	inline GLEArrayImpl* getArray() { return &m_Values; }
	void getPropertyAsString(GLEPropertyID id, string* result);
	GLEPropertyStore* clone();
};

class GLEFileLocationMap;

#define GLE_FILELOCATION_IS_STDIN   1
#define GLE_FILELOCATION_IS_STDOUT  2
#define GLE_FILELOCATION_IS_LOCAL   4
#define GLE_FILELOCATION_IS_ILLEGAL 8

class DLLCLASS GLEFileLocation {
protected:
	unsigned int m_Flags;
	string m_Name;
	string m_Ext;
	string m_Directory;
	string m_FullPath;
public:
	GLEFileLocation();
	GLEFileLocation(const char* file);
	GLEFileLocation(const GLEFileLocation& other);
	~GLEFileLocation();
	void fromAbsolutePath(const string& path);
	void fromRelativePath(const string& dirname, const string& fname);
	void fromFileNameCrDir(const string& fname);
	void fromFileNameDir(const string& fname, const string& dirname);
	void createStdin();
	void createStdout();
	void createIllegal();
	void initDirectory();
	void copy(const GLEFileLocation* other);
	void addExtension(const char* ext);
	bool isStream();
	string getFileName();
	string getMainName();
	inline unsigned int getFlags() const { return m_Flags; }
	inline const string& getName() const { return m_Name; }
	inline const string& getExt() const { return m_Ext; }
	inline const string& getDirectory() const { return m_Directory; }
	inline const string& getFullPath() const { return m_FullPath; }
	inline string& getFullPathNC() { return m_FullPath; }
	inline bool isStdin() { return (m_Flags & GLE_FILELOCATION_IS_STDIN) != 0; }
	inline bool isStdout() { return (m_Flags & GLE_FILELOCATION_IS_STDOUT) != 0; }
	inline void setName(const string& v) { m_Name = v; }
	inline void setExt(const string& v) { m_Ext = v; }
	inline void setDirectory(const string& v) { m_Directory = v; }
	inline void setFullPath(const string& v) { m_FullPath = v; }
};

struct surface_struct;

class DLLCLASS GLEInterface {
protected:
	GLEScript* m_Script;
	GLEOutputStream* m_Output;
	GLEGlobalConfig* m_Config;
	bool m_MakeDrawObjs;
	bool m_CommitMode;
	GLERCVector<GLEFont> m_Fonts;
	GLERCVector<GLEFont> m_AllFonts;
	StringIntHash* m_FontHash;
	IntIntHash* m_FontIndexHash;

	GLERC<GLEPropertyStoreModel> m_TextModel;
	GLERC<GLEPropertyStoreModel> m_LineModel;
	GLERC<GLEPropertyStoreModel> m_ShapeModel;

	string* m_InitialPS;
	GLEFileLocationMap* m_FileInfoMap;
public:
	GLEInterface();
	~GLEInterface();

	// Initialize GLE - call once before calling any of the other functions
	DLLFCT bool initializeGLE(const char* appname, int argc, char **argv);

	// Returns the version of GLE
	DLLFCT string getGLEVersion();
	DLLFCT string getGLEBuildDate();

	// Returns the location of GLE_TOP
	DLLFCT const char* getGLETop();

	// ****************************** Methods for changing settings

	// Sets the port used by the QGLE server
	void setSetting(const char* section, const char* name, const char* value);

	// Write settings to "glerc" file ($HOME/.glerc)
	void saveSettings();

	// ****************************** Methods for rendering a GLE file

	// The provided function is called for each block of PostScript code created by GLE
	void setPostScriptCallbackFunction(void* callback);

	// Should GLE make GLEDrawObjects or just regular output
	inline void setMakeDrawObjects(bool make) { m_MakeDrawObjs = make; }
	inline bool isMakeDrawObjects() { return m_MakeDrawObjs; }

	inline GLEScript* getScript() { return m_Script; }

	// Set the output stream for error messages, etc..
	DLLFCT void setOutputStream(GLEOutputStream* output);

	DLLFCT GLERC<GLEScript> newGLEFile(const char* glecode, const char* tmpfile);

	DLLFCT GLERC<GLEScript> loadGLEFile(const char* glefile);

	DLLFCT void saveGLEFile(GLEScript* script, const char* glefile);

	DLLFCT void showGLEFile(GLEScript* script);

	DLLFCT void setCompatibilityMode(const char* version);

	DLLFCT void renderGLE(GLEScript* script, const char* outfile, int device, bool toMemory = false);

	DLLFCT void commitChangesGLE(GLEScript* script);

	// Create a new GLE script "in memory"
	GLEScript* createGLEScript();

	// ****************************** Methods for working with text

	int getNumberOfColors();
	GLEColor* getColor(int i);

	// Return the number of available fonts
	int getNumberOfFonts();

	// Return the i-the font
	GLEFont* getFont(int i);

	// Return the font matching a given name
	GLEFont* getFont(const string& name);

	// Return the font matching a given name
	GLEFont* getFont(const char* name);

	GLEFont* getFontIndex(int font);

	// Return the property store model for a text string
	inline GLEPropertyStoreModel* getTextPropertyStoreModel() { return m_TextModel.get(); }

	DLLFCT const char* getInitialPostScript();

	DLLFCT const char* getTerminatePostScript();

	// Render a given string
	DLLFCT GLETextDO* renderText(const char* strUTF8, GLEPropertyStore* prop);
	void renderText(GLETextDO* text, GLEPropertyStore* prop);

	// Deletes a given object that is no longer required
	void deleteGLEObject(GLEDrawObject* object);

	inline GLEOutputStream* getOutput() { return m_Output; }

	DLLFCT bool isDeviceSupported(int device);

	DLLFCT const char* getDeviceFilenameExtension(int device);

	// Return the property store model for a line
	inline GLEPropertyStoreModel* getLinePropertyStoreModel() { return m_LineModel.get(); }
	inline GLEPropertyStoreModel* getShapePropertyStoreModel() { return m_ShapeModel.get(); }

	DLLFCT void clearAllCmdLine();
	DLLFCT void setCmdLineOption(const char* name);
	DLLFCT void setCmdLineOptionString(const char* name, const char* value, int arg = 0);
	DLLFCT bool hasCmdLineOptionString(const char* name);

	DLLFCT string getGhostScriptLocation();
	DLLFCT string getToolLocation(const char* name);
	DLLFCT string getUserConfigLocation();

	// Returns the location of gle-manual.pdf
	DLLFCT string getManualLocation();

	DLLFCT void findDependencies(const char* root);
	DLLFCT void saveRCFile();

	DLLFCT void addFileInfo(const GLEFileLocation& f1);
	DLLFCT vector<GLEFileLocation> getFileInfos();
	inline bool hasFileInfos() { return m_FileInfoMap != NULL; }

	DLLFCT void evalString(const char* str, GLEScript* script = NULL);

	DLLFCT string getTempFile();
	DLLFCT int copyFile(const string& from, const string& to, string* err = NULL);

	void initTextProperties(GLEPropertyStore* prop);

	// Get information about 3D surface plots
	DLLFCT surface_struct* getSurface();

	DLLFCT void convertPDFToImage(char* pdfData,
                                  int pdfLength,
                                  double resolution,
                                  int device,
                                  int options,
                                  gle_write_func writeFunc,
                                  void* closure);
                                  
   DLLFCT bool readFileOrGZIPTxt(const char* name, std::string* result);

public:
	inline void setCommitMode(bool commit) { m_CommitMode = commit; }
	inline bool isCommitMode() { return m_CommitMode; }
	inline GLEGlobalConfig* getConfig() { return m_Config; }
	inline void setConfig(GLEGlobalConfig* config) { m_Config = config; }
	CmdLineObj* getCmdLine();
public:
	// For GLE internal use only
	void addFont(GLEFont* font);
	void addSubFont(GLEFont* font);
};

class DLLCLASS GLEErrorMessage {
private:
	int m_Line, m_Column, m_Delta;
	string m_File;
	string m_LineAbbrev;
	string m_ErrorMsg;
public:
	GLEErrorMessage();
	~GLEErrorMessage();
	inline int getLine() { return m_Line; }
	inline void setLine(int line) { m_Line = line; }
	inline int getColumn() { return m_Column; }
	inline void setColumn(int col) { m_Column = col; }
	inline int getDelta() { return m_Delta; }
	inline void setDelta(int delta) { m_Delta = delta; }
	inline const char* getFile() { return m_File.c_str(); }
	inline void setFile(const string& val) { m_File = val; }
	inline const char* getLineAbbrev() { return m_LineAbbrev.c_str(); }
	inline void setLineAbbrev(const string& val) { m_LineAbbrev = val; }
	inline const char* getErrorMsg() { return m_ErrorMsg.c_str(); }
	inline void setErrorMsg(const string& val) { m_ErrorMsg = val; }
};

class DLLCLASS GLEOutputStream {
private:
	int m_ExitCode;
public:
	GLEOutputStream();
	virtual ~GLEOutputStream();
	virtual void println();
	virtual void println(const char* str);
	virtual void printflush(const char* str);
	virtual void error(GLEErrorMessage* msg);
	inline int getExitCode() { return m_ExitCode; }
	inline void setExitCode(int code) { m_ExitCode = code; }
};

enum GLEDrawObjectType {
	// Only implement a few to start
	GDOScript, GDOText, GDOLine, GDOEllipse, GDOArc, GDOObject
};

enum GLEHasArrow {
	GLEHasArrowNone = 0,
	GLEHasArrowStart = 1,
	GLEHasArrowEnd = 2,
	GLEHasArrowBoth = 3
};

class DLLCLASS GLEDrawObject : public GLERefCountObject {
protected:
	int m_Flag;
	GLEPropertyStore* m_Properties;
public:
	GLEDrawObject();
	virtual ~GLEDrawObject();

	// Get the exact bounding box of this object (in cm)
	virtual void getPSBoundingBox(GLERectangle* box);

	// Return the PostScript code generated for this object (if any)
	virtual const char* getPostScriptCode();

	// Position of the object in the originating script (if any)
	// This can be used to delete or move the object
	int getSourceLine1();
	int getSourceLine2();

	// What type of object are we dealing with?
	DLLFCT virtual GLEDrawObjectType getType() = 0;

	inline void setFlag(int flag) { m_Flag |= flag; }
	inline bool hasFlag(int flag) { return (m_Flag & flag) != 0; }

	// Return the list of properties associated with this object
	inline GLEPropertyStore* getProperties() { return m_Properties; }
	void setProperties(GLEPropertyStore* store);

	virtual void initProperties(GLEInterface* iface);
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual void applyTransformation(bool dir);
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
	virtual bool modified();
	virtual void draw();

	GLEDrawObject* deepClone();
	void applyTransformationPt(GLEPoint* point, bool dir);

	inline int getIntProperty(GLEPropertyID id) { return m_Properties->getIntProperty(id); }
	inline bool getBoolProperty(GLEPropertyID id) { return m_Properties->getBoolProperty(id); }
	inline double getRealProperty(GLEPropertyID id) { return m_Properties->getRealProperty(id); }
	inline GLEFont* getFontProperty(GLEPropertyID id) { return m_Properties->getFontProperty(id); }
	inline GLEColor* getColorProperty(GLEPropertyID id) { return m_Properties->getColorProperty(id); }
	inline void setIntProperty(GLEPropertyID id, int value) { m_Properties->setIntProperty(id, value); }
	inline void setBoolProperty(GLEPropertyID id, bool value) { m_Properties->setBoolProperty(id, value); }
	inline void setRealProperty(GLEPropertyID id, double value) { m_Properties->setRealProperty(id, value); }
	inline void setFontProperty(GLEPropertyID id, GLEFont* value) { m_Properties->setFontProperty(id, value); }
	inline void setColorProperty(GLEPropertyID id, GLEColor* value) { m_Properties->setColorProperty(id, value); }
	void getPropertyAsString(GLEPropertyID id, string* result) { m_Properties->getPropertyAsString(id, result); }
};

class GLEHasArrowBase {
public:
	GLEHasArrowBase();
	inline GLEHasArrow getArrow() { return m_Arrow; }
	inline void setArrow(GLEHasArrow arrow) { m_Arrow = arrow; }

protected:
	GLEHasArrow m_Arrow;
};

class GLELineDO : public GLEDrawObject, public GLEHasArrowBase {
protected:
	GLEPoint m_P1, m_P2;
public:
	GLELineDO();
	GLELineDO(double x1, double y1, double x2, double y2);
	GLELineDO(const GLEPoint& p1, const GLEPoint& p2);
	virtual ~GLELineDO();
	virtual GLEDrawObjectType getType();
	inline GLEPoint& getP1() { return m_P1; }
	inline GLEPoint& getP2() { return m_P2; }
	inline void setP1(const GLEPoint& p1) { m_P1 = p1; }
	inline void setP2(const GLEPoint& p2) { m_P2 = p2; }
	virtual void initProperties(GLEInterface* iface);
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual void applyTransformation(bool dir);
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
};

class GLEEllipseDO : public GLEDrawObject {
protected:
	GLEPoint m_Center;
	double m_Rx, m_Ry;
public:
	GLEEllipseDO();
	GLEEllipseDO(double x0, double y0, double r);
	GLEEllipseDO(const GLEPoint& c, double rx, double ry);
	GLEEllipseDO(double x0, double y0, double rx, double ry);
	virtual ~GLEEllipseDO();
	virtual GLEDrawObjectType getType();
	inline GLEPoint& getCenter() { return m_Center; }
	inline void setCenter(const GLEPoint& c) { m_Center = c; }
	inline double getRadiusX() { return m_Rx; }
	inline double getRadiusY() { return m_Ry; }
	inline void setRadiusX(double r) { m_Rx = r; }
	inline void setRadiusY(double r) { m_Ry = r; }
	DLLFCT void setRadius(double r);
	inline bool isCircle() { return m_Rx == m_Ry; }
	DLLFCT GLEPoint getPoint(GLEJustify just);
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual void applyTransformation(bool dir);
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
	virtual void initProperties(GLEInterface* iface);
};

class GLEArcDO : public GLEEllipseDO, public GLEHasArrowBase {
protected:
	double m_Angle1, m_Angle2;
public:
	GLEArcDO();
	GLEArcDO(double x0, double y0, double r, double a1, double a2);
	GLEArcDO(double x0, double y0, double rx, double ry, double a1, double a2);
	virtual ~GLEArcDO();
	virtual GLEDrawObjectType getType();
	inline double getAngle1() { return m_Angle1; }
	inline double getAngle2() { return m_Angle2; }
	inline void setAngle1(double a1) { m_Angle1 = a1; }
	inline void setAngle2(double a2) { m_Angle2 = a2; }
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
	virtual void initProperties(GLEInterface* iface);
	DLLFCT GLEPoint& getPoint1(GLEPoint& pt);
	DLLFCT GLEPoint& getPoint2(GLEPoint& pt);
	DLLFCT GLEPoint& getPointMid(GLEPoint& pt);
	DLLFCT void normalize();
};

class GLETextDO : public GLEDrawObject {
protected:
	GLEPoint m_Position;
	string m_Text;
	string m_PostScript;
	GLERectangle m_PSBoundingBox;
	double m_BaseLine;
	bool m_Modified;
public:
	GLETextDO();
	GLETextDO(GLEPoint& position, const string& text);
	virtual ~GLETextDO();
	virtual const char* getPostScriptCode();
	virtual void getPSBoundingBox(GLERectangle* box);
	GLEDrawObjectType getType();
	inline double getBaseLine() { return m_BaseLine; }
	void initBB(double width, double height, double baseline);
	inline string* getPostScriptPtr() { return &m_PostScript; }
	inline const string& getText() { return m_Text; }
	inline const char* getTextC() { return m_Text.c_str(); }
	inline void setText(const string& text) { m_Text = text; }
	inline void setText(const char* text) { m_Text = text; }
	inline GLEPoint& getPosition() { return m_Position; }
	inline void setPosition(const GLEPoint& pos) { m_Position = pos; }
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual void applyTransformation(bool dir);
	virtual void initProperties(GLEInterface* iface);
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
	virtual bool modified();
	DLLFCT void setModified(bool modified);
	DLLFCT void render(GLEInterface* iface);
};

// ...

class DLLCLASS GLEComposedObject : public GLEDrawObject {
private:
	GLERCVector<GLEDrawObject> m_Objs;
public:
	GLEComposedObject();
	virtual ~GLEComposedObject();

	// Return the number of objects defined by this object
	inline int getNumberObjects() { return m_Objs.size(); }
	inline void setNumberObjects(int nb) { m_Objs.resize(nb); }

	// Return given object defined by the script
	inline GLEDrawObject* getObject(int i) { return m_Objs.get(i); }
	inline void setObject(int i, GLEDrawObject* obj) { m_Objs[i] = obj; }

	// Add a new object to this composed object
	inline void addObject(GLEDrawObject* obj) { m_Objs.add(obj); }

	void DLLFCT clear();
	void removeDeletedObjects();
};

class GLEObjectDO;

class DLLCLASS GLEObjectDOConstructor : public GLEPropertyStoreModel {
protected:
	GLESub* m_Sub;
public:
	GLEObjectDOConstructor(GLESub* sub);
	virtual ~GLEObjectDOConstructor();
	DLLFCT const string& getName();
	DLLFCT GLEObjectDO* constructObject();
	DLLFCT GLEScript* getScript();
	inline GLESub* getSubroutine() { return m_Sub; }
	virtual void scale(GLEDrawObject* obj, double sx, double sy);
};

class GLEObjectDO : public GLEDrawObject {
protected:
	GLEPoint m_Position;
	string m_PostScript;
	GLERC<GLEObjectRepresention> m_ObjRep;
	GLERC<GLEString> m_RefPoint;
	GLEObjectDOConstructor* m_Cons;
public:
	GLEObjectDO(GLEObjectDOConstructor* cons);
	virtual ~GLEObjectDO();
	virtual const char* getPostScriptCode();
	virtual void getPSBoundingBox(GLERectangle* box);
	GLEDrawObjectType getType();
	inline string* getPostScriptPtr() { return &m_PostScript; }
	inline GLEPoint& getPosition() { return m_Position; }
	inline void setPosition(const GLEPoint& pos) { m_Position = pos; }
	virtual bool needsAMove(GLEPoint& pt);
	virtual void createGLECode(string& code);
	virtual void updateBoundingBox();
	virtual void applyTransformation(bool dir);
	virtual void initProperties(GLEInterface* iface);
	virtual GLEDrawObject* clone();
	virtual bool approx(GLEDrawObject* other);
	void makePropertyStore();
	DLLFCT void render();
	DLLFCT void computeReferencePoint(GLEPoint* pt);
	inline void setObjectRepresentation(GLEObjectRepresention* rep) { m_ObjRep = rep; }
	inline GLEObjectRepresention* getObjectRepresentation() { return m_ObjRep.get(); }
	inline GLEObjectDOConstructor* getConstructor() { return m_Cons; }
	inline void setRefPointString(GLEString* str) { m_RefPoint = str; }
	inline GLEString* getRefPointString() { return m_RefPoint.get(); }
};

// must come after definition GLEFileLocation and of GLEObjectDOConstructor
#include "gle-sourcefile.h"

class DLLCLASS GLEScript : public GLEComposedObject {
protected:
	GLEPcodeIndexed* m_Pcode;
	GLERun* m_Run;
	GLEParser* m_Parser;
	GLEPolish* m_Polish;
	GLEGlobalSource m_File;
	GLEPoint m_Size, m_BoundingBox, m_BoundingBoxOrigin;
	GLERCVector<GLEDrawObject> m_NewObjs;
	string m_PostScriptCode;
	string m_PDFCode;
	int m_CurrObject;
public:
	GLEScript();
	virtual ~GLEScript();
	void cleanUp();

	// What type of object are we dealing with?
	virtual GLEDrawObjectType getType();

	DLLFCT void updateObjectDOConstructors();

	GLEDrawObject* DLLFCT newGLEObject(GLEDrawObjectType type);
	void DLLFCT cancelObject(GLEDrawObject* obj);

	inline GLEGlobalSource* getSource() { return &m_File; }
	inline GLEFileLocation* getLocation() { return m_File.getLocation(); }

	inline const GLEPoint& getSize() { return m_Size; }
	inline const GLEPoint& getBoundingBox() { return m_BoundingBox; }
	inline const GLEPoint& getBoundingBoxOrigin() { return m_BoundingBoxOrigin; }

	inline void setSize(double wd, double hi) { m_Size.setXY(wd, hi); }
	inline void setBoundingBox(double wd, double hi) { m_BoundingBox.setXY(wd, hi); }
	inline void setBoundingBoxOrigin(double x, double y) { m_BoundingBoxOrigin.setXY(x, y); }

	inline void resetObjectIterator() { m_CurrObject = 0; }
	GLEDrawObject* nextObject();

	inline GLEInterface* getGLEInterface() { return GLEGetInterfacePointer(); }
	inline GLERun* getRun() { return m_Run; }
	inline GLEParser* getParser() { return m_Parser; }
	inline GLEPolish* getPolish() { return m_Polish; }
	inline GLEPcodeIndexed* getPcode() { return m_Pcode; }
	inline void setRun(GLERun* run) { m_Run = run; }
	inline void setParser(GLEParser* parser) { m_Parser = parser; }
	inline void setPolish(GLEPolish* polish) { m_Polish = polish; }
	inline void setPcode(GLEPcodeIndexed* code) { m_Pcode = code; }

	inline int getNumberNewObjects() { return m_NewObjs.size(); }
	inline GLEDrawObject* getNewObject(int i) { return m_NewObjs.get(i); }
	inline void addNewObject(GLEDrawObject* obj) { m_NewObjs.add(obj); }
	void clearNewObjects();
	virtual const char* getPostScriptCode();
	virtual string* getRecordedBytesBuffer(int device);
};

class DLLCLASS GLEColorList {
protected:
	GLERCVector<GLEColor> m_Colors;
	StringIntHash m_ColorHash;
	GLERCVector<GLEColor> m_OldColors;
	StringIntHash m_OldColorHash;
public:
	GLEColorList();
	~GLEColorList();
	void reset();
	void defineColor(const char* name, unsigned int value);
	void defineColor(const string& name, unsigned int value);
	void defineColor(const string& name, GLEColor* color);
	void defineOldColor(const char* name, unsigned int value);
	void defineOldColor(const string& name, unsigned int value);
	GLEColor* get(const string& name);
	DLLFCT GLEColor* getSafeDefaultBlack(const string& name);
	inline int getNbColors() { return m_Colors.size(); }
	inline GLEColor* getColor(int i) { return m_Colors.get(i); }
	void defineDefaultColors();
private:
	void defineGrays();
	void defineSVGColors();
	void defineOldGLEColors();
};

GLEColorList* DLLFCT GLEGetColorList();

class GLEStringSettings {
public:
	void setHeight(double hei);
	void setFontLWidth(double lwidht);
// ...
};

typedef struct {
	double xt, yt;
	double xa, ya;
	double xb, yb;
	double xl, yl;
} GLEArrowPoints;

DLLFCT void GLESetDefaultArrowProperties(double lwd, GLEPropertyStore* prop);
DLLFCT void GLEGetArrowPoints(GLEPoint& pt, double dx, double dy, GLEPropertyStore* props, double fac, GLEArrowPoints* pts);
DLLFCT void GLEGetArrowPointsNoProps(GLEPoint& pt, double dx, double dy, double lwd, double size, double angle, GLEArrowPoints* pts);
void GLEInitColorProperty(GLEPropertyStore* prop);
void GLEInitSimpleLineProperties(GLEPropertyStore* prop);
void GLEInitLineProperties(GLEPropertyStore* prop);
void GLEInitArrowProperties(GLEPropertyStore* prop);
void GLEInitShapeFillColor(GLEPropertyStore* prop);
void GLEInitTextProperties(GLEPropertyStore* prop);

// Implements: (int)floor((double)dpi/72.0*bbox+1);
// Not inline because math.h may not be included in some cpp files, which is required by floor()
DLLFCT int GLEBBoxToPixels(double dpi, double bbox);

#endif
