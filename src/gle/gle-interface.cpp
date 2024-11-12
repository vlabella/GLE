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

#include <fstream>
#include <set>
#include <string>

#include "all.h"
#include "cutils.h"
#include "file_io.h"
#include "core.h"
#include "cmdline.h"
#include "config.h"
#include "core.h"
#include "gle-interface/gle-interface.h"
#include "tokens/stokenizer.h"
#include "texinterface.h"
#include "drawit.h"
#include "justify.h"
#include "var.h"
#include "sub.h"
#include "mem_limits.h"
#include "token.h"
#include "glearray.h"
#include "polish.h"
#include "pass.h"
#include "keyword.h"
#include "run.h"
#include "gle-poppler.h"

using namespace std;

// TODO make these available through the interface
extern CmdLineObj g_CmdLine;
extern ConfigCollection g_Config;
extern string GLE_WORKING_DIR;
extern string GLE_TOP_DIR;

void init_option_args(CmdLineObj& cmdline);
void load_one_file_sub(GLEScript* script, CmdLineObj& cmdline, size_t* exit_code);
GLERC<GLEScript> load_gle_code_sub(const char* name, CmdLineObj& cmdline);

void GLEScaleSimpleLineProperties(double scale, bool dir, GLEPropertyStore* prop);
void GLEScaleArrowProperties(double scale, bool dir, GLEPropertyStore* prop);

GLEInterface* g_GLEInterface = NULL;

GLEInterface* GLEGetInterfacePointer() {
	if (g_GLEInterface == NULL) {
		GLEGlobalConfig* config = new GLEGlobalConfig();
		config->setCmdLine(&g_CmdLine);
		config->setRCFile(&g_Config);
		g_GLEInterface = new GLEInterface();
		g_GLEInterface->setConfig(config);
	}
	return g_GLEInterface;
}

struct GLEFileLocationCompare {
	bool operator()(const GLEFileLocation& s1, const GLEFileLocation& s2) const {
		if (s1.getExt() == s2.getExt()) {
			if (s1.getName() == s2.getName()) {
				return s1.getFullPath() < s2.getFullPath();
			} else {
				return s1.getName() < s2.getName();
			}
		} else {
			if (str_i_equals(s1.getExt(), "GLE")) return true;
			if (str_i_equals(s2.getExt(), "GLE")) return false;
			return s1.getExt() < s2.getExt();
		}
	}
};

typedef set<GLEFileLocation, GLEFileLocationCompare> GLEFileLocationMapData;

class GLEFileLocationMap {
protected:
	GLEFileLocationMapData m_Data;
public:
	GLEFileLocationMap();
	~GLEFileLocationMap();
	vector<GLEFileLocation> getFiles();
	inline void addFile(const GLEFileLocation& f1) { m_Data.insert(f1); }
	inline void clear() { m_Data.clear(); }
};

GLEFileLocationMap::GLEFileLocationMap() {
}

GLEFileLocationMap::~GLEFileLocationMap() {
}

vector<GLEFileLocation> GLEFileLocationMap::getFiles() {
	vector<GLEFileLocation> res;
	for (GLEFileLocationMapData::iterator it = m_Data.begin(); it != m_Data.end(); it++) {
		res.push_back(*it);
	}
	return res;
}

GLEInterface::GLEInterface():
m_Script(NULL),
m_Output(NULL),
m_Config(NULL),
m_FontHash(NULL),
m_FontIndexHash(NULL),
m_InitialPS(NULL),
m_FileInfoMap(NULL){
	m_Output = new GLEOutputStream();
	m_MakeDrawObjs = false;
	m_CommitMode = false;
	m_FontHash = new StringIntHash();
	m_FontIndexHash = new IntIntHash();
	m_FileInfoMap = new GLEFileLocationMap();

	// Initialize text property store model
	m_TextModel = new GLEPropertyStoreModel();
	m_TextModel->add(new GLEPropertyFont("Font"));
	m_TextModel->add(new GLEPropertyHei("Font size"));
	m_TextModel->add(new GLEPropertyColor("Text color"));
	m_TextModel->add(new GLEPropertyJustify("Text justify"));

	m_LineModel = new GLEPropertyStoreModel();
	m_LineModel->add(new GLEPropertyLWidth("Line width"));
	m_LineModel->add(new GLEPropertyColor("Line color"));
	m_LineModel->add(new GLEPropertyLStyle("Line style"));
	GLEPropertyNominal* linecap = new GLEPropertyNominal("Line cap", GLEPropertyTypeInt, GLEDOPropertyLineCap);
	linecap->addValue("butt", GLELineCapButt);
	linecap->addValue("round", GLELineCapRound);
	linecap->addValue("square", GLELineCapSquare);
	m_LineModel->add(linecap);

	m_LineModel->add(new GLEPropertyArrowSize("Arrow size"));
	m_LineModel->add(new GLEPropertyArrowAngle("Arrow angle"));
	GLEPropertyNominal* arrowstyle = new GLEPropertyNominal("Arrow style", GLEPropertyTypeInt, GLEDOPropertyArrowStyle);
	arrowstyle->addValue("simple", GLEArrowStyleSimple);
	arrowstyle->addValue("filled", GLEArrowStyleFilled);
	arrowstyle->addValue("empty", GLEArrowStyleEmpty);
	m_LineModel->add(arrowstyle);
	GLEPropertyNominal* arrowtip = new GLEPropertyNominal("Arrow tip", GLEPropertyTypeInt, GLEDOPropertyArrowTip);
	arrowtip->addValue("round", GLEArrowTipRound);
	arrowtip->addValue("sharp", GLEArrowTipSharp);
	m_LineModel->add(arrowtip);

	m_ShapeModel = new GLEPropertyStoreModel();
	m_ShapeModel->add(new GLEPropertyLWidth("Line width"));
	m_ShapeModel->add(new GLEPropertyColor("Line color"));
	m_ShapeModel->add(new GLEPropertyLStyle("Line style"));
	m_ShapeModel->add(new GLEPropertyFillColor("Fill color"));

	m_InitialPS = NULL;
	m_Config = NULL;
}

GLEInterface::~GLEInterface() {
	if(m_FontHash != NULL) delete m_FontHash;
	if (m_FontIndexHash != NULL) delete m_FontIndexHash;
	if (m_Output != NULL) delete m_Output;
	if (m_Config != NULL) delete m_Config;
	if (m_FileInfoMap != NULL) delete m_FileInfoMap;
}

bool GLEInterface::initializeGLE(const char* appname, int argc, char **argv) {
	try {
      gle_glib_init(argc, argv);
		// Initialize code in core.cpp
		g_init();
		// Load configuration file
		init_config(&g_Config);
		init_option_args(g_CmdLine);
		return do_load_config(appname, argv, g_CmdLine, g_Config);
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
	}
	return false;
}

string GLEInterface::getGLEVersion() {
	string version;
	g_get_version(&version);
	return version;
}

string GLEInterface::getGLEBuildDate() {
	string date;
	g_get_build_date(&date);
	return date;
}

const char* GLEInterface::getGLETop() {
	return GLE_TOP_DIR.c_str();
}

void GLEInterface::setOutputStream(GLEOutputStream* output) {
	m_Output = output;
}

GLERC<GLEScript> GLEInterface::newGLEFile(const char* glecode, const char* tmpfile) {
	GLERC<GLEScript> script;
	try {
		string in_name = tmpfile;
		script = new GLEScript();
		GLEFileLocation* loc = script->getLocation();
		loc->fromFileNameDir(in_name, GLE_WORKING_DIR);
		GLESourceFile* file = script->getSource()->getMainFile();
		char_separator sep("\n");
		tokenizer<char_separator> tokens(glecode, sep);
		while (tokens.has_more()) {
			string line = tokens.next_token();
			str_trim_both(line);
			GLESourceLine* sline = file->addLine();
			sline->setCode(line);
		}
		file->trim(1);
		script->getSource()->initFromMain();
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
	}
	return script;
}

GLERC<GLEScript> GLEInterface::loadGLEFile(const char* glefile) {
	try {
		if (m_FileInfoMap != NULL) m_FileInfoMap->clear();
		g_set_compatibility(GLE_COMPAT_MOST_RECENT);
		return load_gle_code_sub(glefile, g_CmdLine);
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
		return NULL;
	}
}

void GLEInterface::saveGLEFile(GLEScript* script, const char* glefile) {
	ofstream file(glefile);
	GLESourceFile* main = script->getSource()->getMainFile();
	// save all code lines
	for (int i = 0; i < main->getNbLines(); i++) {
		GLESourceLine* line = main->getLine(i);
		file << line->getPrefix() << line->getCode() << endl;
	}
	file << endl;
	file.close();
	// update location
	main->getLocation()->fromFileNameCrDir(glefile);
}

void GLEInterface::showGLEFile(GLEScript* script) {
	cout << "Script:" << endl;
	GLESourceFile* file = script->getSource()->getMainFile();
	for (int i = 0; i < file->getNbLines(); i++) {
		GLESourceLine* line = file->getLine(i);
		cout << line->getCode() << endl;
	}
}

void GLEInterface::setCompatibilityMode(const char* version) {
	CmdLineArgString* arg = (CmdLineArgString*)g_CmdLine.createOption(GLE_OPT_COMPAT)->getArg(0);
	arg->setValue(version);
	g_set_compatibility(version);
}

void GLEInterface::renderGLE(GLEScript* script, const char* outfile, int device, bool toMemory) {
	m_Script = script;
	if (script == NULL) {
		cerr << "GLEInterface::renderGLE(): script == NULL" << endl;
		return;
	}
	script->cleanUp();
	try {
		size_t exit_code;
		// Select dry-run (and output to memory)
		CmdLineOption* noSave = g_CmdLine.createOption(GLE_OPT_NOSAVE);
		noSave->setHasOption(toMemory);
		// Set the device
		CmdLineArgSet* dev = (CmdLineArgSet*)g_CmdLine.createOption(GLE_OPT_DEVICE)->getArg(0);
		dev->reset();
		dev->addValue(device);
		// Set the output file name
		CmdLineArgString* o_file = (CmdLineArgString*)g_CmdLine.createOption(GLE_OPT_OUTPUT)->getArg(0);
		o_file->setValue(outfile);
		if (isMakeDrawObjects()) script->clear();
		load_one_file_sub(script, g_CmdLine, &exit_code);
		m_Output->setExitCode(get_nb_errors());
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
		m_Output->setExitCode(1);
	}
}

void handleNewProperties(GLEGlobalSource* source, GLEPropertyStore* props) {
	vector<GLEProperty*> changed;
	GLEPropertyStoreModel* model = props->getModel();
	for (int i = 0; i < model->getNumberOfProperties(); i++) {
		GLEProperty* prop = model->getProperty(i);
		if (!prop->isEqualToState(props)) {
			prop->updateState(props);
			changed.push_back(prop);
		}
	}
	if (changed.size() != 0) {
		ostringstream ss;
		ss << "set";
		for (vector<GLEProperty*>::size_type i = 0; i < changed.size(); i++) {
			GLEProperty* prop = changed[i];
			prop->createSetCommandGLECode(ss, props->getPropertyValue(prop));
		}
		source->addLine(ss.str());
	}
}

void GLEInterface::commitChangesGLE(GLEScript* script) {
	m_Script = script;
	if (script == NULL) {
		cerr << "GLEInterface::commitChangesGLE(): script == NULL" << endl;
		return;
	}
	try {
		setCommitMode(true);
		setMakeDrawObjects(true);
		GLEDevice* old_device = g_set_dummy_device();
		TeXInterface* interface = TeXInterface::getInstance();
		GLEFileLocation output;
		output.createIllegal();
		interface->initialize(script->getLocation(), &output);
		interface->reset();
		script->resetObjectIterator();
		DrawIt(m_Script, &output, &g_CmdLine);
		for (int i = 0; i < script->getNumberNewObjects(); i++) {
			string code;
			GLEDrawObject* obj = script->getNewObject(i);
			if (!obj->hasFlag(GDO_FLAG_DELETED)) {
				obj->createGLECode(code);
				/* check if needs amove? */
				GLEPoint amove;
				bool needs_amove = false;
				if (obj->needsAMove(amove)) {
					/* Check if not already at this point */
					GLEPoint crpt;
					g_get_xy(&crpt);
					if (!crpt.approx(amove)) {
						needs_amove = true;
						script->getSource()->addLine("");
					}
				}
				/* add line to code */
				handleNewProperties(script->getSource(), obj->getProperties());
				/* amove command in front of object? */
				if (needs_amove) {
					ostringstream amovec;
					amovec << "amove " << amove.getX() << " " << amove.getY();
					script->getSource()->addLine(amovec.str());
				}
				script->getSource()->addLine(code);
				/* compile and execute line of code */
				obj->updateBoundingBox();
				script->addObject(obj);
			}
		}
		script->getSource()->performUpdates();
		script->clearNewObjects();
		script->removeDeletedObjects();
		interface->tryCreateHash();
		g_restore_device(old_device);
		setMakeDrawObjects(false);
		setCommitMode(false);
	} catch (ParserError& err) {
		string err_str;
		err.toString(err_str);
		str_uppercase_initial_capital(err_str);
		g_message(err_str);
		m_Output->setExitCode(1);
	}
}

CmdLineObj* GLEInterface::getCmdLine() {
	return m_Config->getCmdLine();
}

int GLEInterface::getNumberOfColors() {
	return GLEGetColorList()->getNbColors();
}

GLEColor* GLEInterface::getColor(int i) {
	return GLEGetColorList()->getColor(i);
}

void GLEInterface::addFont(GLEFont* font) {
	font->setNumber(m_Fonts.size());
	m_Fonts.add(font);
	addSubFont(font);
}

void GLEInterface::addSubFont(GLEFont* font) {
	m_FontHash->add_item(font->getName(), m_AllFonts.size());
	m_FontIndexHash->add_item(font->getIndex(), m_AllFonts.size());
	m_AllFonts.add(font);
}

// Return the number of available fonts
int GLEInterface::getNumberOfFonts() {
	return m_Fonts.size();
}

// Return the i-the font
GLEFont* GLEInterface::getFont(int i) {
	if (i < 0 || i >= (int)m_Fonts.size()) return m_Fonts.get(0);
	return m_Fonts.get(i);
}

// Return the font matching a given name
GLEFont* GLEInterface::getFont(const string& name) {
	int idx = m_FontHash->try_get(name);
	if (idx == -1) return NULL;
	else return m_AllFonts.get(idx);
}

GLEFont* GLEInterface::getFont(const char* name) {
	return getFont(string(name));
}

GLEFont* GLEInterface::getFontIndex(int font) {
	int index = m_FontIndexHash->try_get(font);
	if (index == -1) {
		return 0;
	} else {
		return m_AllFonts.get(index);
	}
}

const char* GLEInterface::getInitialPostScript() {
	if (m_InitialPS == NULL) {
		GLESaveRestore saved_state;
		g_select_device(GLE_DEVICE_EPS);
		PSGLEDevice* dev = (PSGLEDevice*)g_get_device_ptr();
		dev->startRecording();
		saved_state.save();
		g_clear();
		dev->startRecording();
		dev->initialPS();
		m_InitialPS = new string();
		dev->getRecordedBytes(m_InitialPS);
		saved_state.restore();
	}
	return m_InitialPS->c_str();
}

const char* GLEInterface::getTerminatePostScript() {
	return "showpage\ngrestore\n";
}

void GLEInterface::renderText(GLETextDO* text, GLEPropertyStore* prop) {
	GLESaveRestore saved_state;
	g_select_device(GLE_DEVICE_EPS);
	PSGLEDevice* dev = (PSGLEDevice*)g_get_device_ptr();
	dev->startRecording();
	saved_state.save();
	g_clear();
	g_resetfont();
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	dev->startRecording();
	// Select the right color
	GLEColor* color = prop->getColorProperty(GLEDOPropertyColor);
	g_set_color(color);
	// Select the right size
	double hei = prop->getRealProperty(GLEDOPropertyFontSize);
	g_set_hei(hei);
	g_set_font_width(-1);
	g_set_line_style("1");
	g_set_line_width(0.02);
	// Select the font
	GLEFont* font = prop->getFontProperty(GLEDOPropertyFont);
	if (font == NULL) {
		font = getFont("rm");
	}
	g_set_font(font->getIndex());
	char* str = (char*)text->getTextC();
	// Calculate the text's size
	double x1, x2, y1, y2;
	g_measure(str, &x1, &x2, &y2, &y1);
	text->initBB(x2-x1, y2-y1, -y1);
// DRAW BOUNDING BOX
/*	g_set_color(0X01FF0000);
	g_set_line_width(0.005);
	g_move(0.0,0.0);
	g_line(x2-x1, 0.0);
	g_line(x2-x1, y2-y1);
	g_line(0.0, y2-y1);
	g_line(0.0, 0.0);
	dev->flush();
	g_set_color(0X01000000);*/
	// Draw the string
	g_move(0.0, 0.0);
	g_jtext(JUST_BL);
	// Return the resulting PostScript code
	dev->getRecordedBytes(text->getPostScriptPtr());
/*
	string code;
	code += getInitialPostScript();
	code += (*text->getPostScriptPtr());
	code += getTerminatePostScript();
	cout << "PS:" << endl << code << endl; */
	saved_state.restore();
}

GLETextDO* GLEInterface::renderText(const char* str, GLEPropertyStore* prop) {
	GLEPoint orig;
	GLETextDO* text = new GLETextDO(orig, str);
	renderText(text, prop);
	return text;
}

bool GLEInterface::isDeviceSupported(int device) {
	switch (device) {
		case GLE_DEVICE_PS:
		case GLE_DEVICE_EPS:
		case GLE_DEVICE_DUMMY:
			return true;
#ifdef HAVE_CAIRO
        case GLE_DEVICE_CAIRO_PDF:
		case GLE_DEVICE_SVG:
        case GLE_DEVICE_CAIRO_SVG:
			return true;
#ifdef _WIN32
        case GLE_DEVICE_EMF:
			return true;
#endif
#endif
#ifdef HAVE_X11
		case GLE_DEVICE_X11:
			return true;
#endif
		default:
			return false;
	}
}

const char* GLEInterface::getDeviceFilenameExtension(int device) {
	switch (device) {
		case GLE_DEVICE_PDF:  return "pdf";
		case GLE_DEVICE_PS:   return "ps";
		case GLE_DEVICE_EPS:  return "eps";
		case GLE_DEVICE_SVG:  return "svg";
		case GLE_DEVICE_EMF:  return "emf";
		case GLE_DEVICE_PNG:  return "png";
		case GLE_DEVICE_JPEG: return "jpg";
	}
	return "unk";
}

void GLEInterface::clearAllCmdLine() {
	getCmdLine()->clearAll();
}

void GLEInterface::setCmdLineOption(const char* name) {
	getCmdLine()->setHasOption(string(name));
}

void GLEInterface::setCmdLineOptionString(const char* name, const char* value, int arg) {
	getCmdLine()->setOptionString(string(name), string(value), arg);
}

bool GLEInterface::hasCmdLineOptionString(const char* name) {
	return getCmdLine()->hasOption(string(name));
}

string GLEInterface::getGhostScriptLocation() {
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
#ifdef _WIN32
	string name = "gsdll32.dll";
	#if _WIN64
		// VS builds
		name = "gsdll64.dll";	
	#endif
	#if __GNUC__
		// gcc builds
		#if __x86_64__ || __ppc64__
			name = "gsdll64.dll";	
		#endif
	#endif
	string result = ((CmdLineArgString*)tools->getOptionValue(GLE_TOOL_GHOSTSCRIPT_CMD))->getValue();
	if (IsAbsPath(result)) {
		StripPathComponents(&result, 1);
		AddDirSep(result);
	} else {
		result = "";
	}
	result += name;
	return result;
#else
	return get_tool_path(GLE_TOOL_GHOSTSCRIPT_LIB, tools);
#endif
}

string GLEInterface::getToolLocation(const char* name) {
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
	for (int j = 0; j < tools->getNbOptions(); j++) {
		CmdLineOption* opt = tools->getOption(j);
		if (str_i_equals(opt->getName(), name)) {
			return get_tool_path(j, tools);
		}
	}
	return string("");
}

string GLEInterface::getUserConfigLocation() {
	string location;
	#if defined(__unix__) || defined(__APPLE__) || defined (__OS2__)
		GLEGetEnv("HOME", location);
	#endif
	#ifdef _WIN32	
		GLEGetEnv("APPDATA", location);
		if (location != "") {
			AddDirSep(location);
			location += "glx.sourceforge.net";
		}
	#endif
	if (location != "") {
		AddDirSep(location);
		location += ".glerc";
	}
	return location;
}

string GLEInterface::getManualLocation() {
	string loc;
#ifdef GLEDOC_CD
	if (GLEAddRelPathAndFileTry(GLE_TOP_DIR, GLEDOC_CD, GLEDOC_REL, "gle-manual.pdf", loc)) {
		return loc;
	}
	if (GLEAddRelPathAndFileTry(GLE_TOP_DIR, GLEDOC_CD, GLEDOC_REL, "gle-manual.pdf.gz", loc)) {
		return loc;
	}
#endif
	if (GLEAddRelPathAndFileTry(GLE_TOP_DIR, 0, "doc", "gle-manual.pdf", loc)) {
		return loc;
	}
	if (GLEAddRelPathAndFileTry(GLE_TOP_DIR, 0, "doc", "gle-manual.pdf.gz", loc)) {
		return loc;
	}
#ifdef GLEDOC_ABS
	if (GLEAddRelPathAndFileTry(GLEDOC_ABS, 0, NULL, "gle-manual.pdf", loc)) {
		return loc;
	}
	if (GLEAddRelPathAndFileTry(GLEDOC_ABS, 0, NULL, "gle-manual.pdf.gz", loc)) {
		return loc;
	}
#endif
	return loc;
}

void GLEInterface::findDependencies(const char* root) {
	string loc = root;
	do_find_deps_sub(this, loc);
}

void GLEInterface::saveRCFile() {
	do_save_config();
}

void GLEInterface::addFileInfo(const GLEFileLocation& f1) {
	if (m_FileInfoMap != NULL) m_FileInfoMap->addFile(f1);
}

vector<GLEFileLocation> GLEInterface::getFileInfos() {
	if (m_FileInfoMap != NULL) {
		return m_FileInfoMap->getFiles();
	} else {
		return vector<GLEFileLocation>();
	}
}

void GLEInterface::initTextProperties(GLEPropertyStore* prop) {
	int font;
	double fontsize;
	g_get_hei(&fontsize);
	prop->setRealProperty(GLEDOPropertyFontSize, fontsize);
	g_get_font(&font);
	prop->setFontProperty(GLEDOPropertyFont, getFontIndex(font));
}

extern GLEGlobalSource* g_Source;
void output_error(ParserError& err);
void g_set_error_line(int lin);

void GLEInterface::evalString(const char* str, GLEScript* script) {
	try {
		g_set_error_line(-1);
		g_select_device(GLE_DEVICE_DUMMY);
		if (script == NULL) {
			g_Source = NULL;
			g_clear();
			sub_clear(false);
			clear_run();
			f_init();
			gle_set_constants();
		}
		GLEPolish polish;
		polish.initTokenizer();
		string value;
		GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
		polish.evalString(stk.get(), str, &value, true);
		g_message_first_newline(false);
		g_message(value);
	} catch (ParserError& err) {
		output_error(err);
	}
}

string GLEInterface::getTempFile() {
	return GLETempName();
}

int GLEInterface::copyFile(const string& from, const string& to, string* err) {
	return GLECopyFile(from, to, err);
}

void GLEInterface::convertPDFToImage(char* pdfData,
                                     int pdfLength,
                                     double resolution,
                                     int device,
                                     int options,
                                     gle_write_func writeFunc,
                                     void* closure)
{
	gle_convert_pdf_to_image(pdfData,
			                 pdfLength,
			                 resolution,
			                 device,
			                 options,
			                 writeFunc,
			                 closure);
}

bool GLEReadFileOrGZIPTxt(const std::string& name, std::string* result);

bool GLEInterface::readFileOrGZIPTxt(const char* name, std::string* result) {
   return GLEReadFileOrGZIPTxt(std::string(name), result);
}

GLEFileLocation::GLEFileLocation() {
	m_Flags = 0;
}

GLEFileLocation::GLEFileLocation(const GLEFileLocation& other) {
	m_Flags = other.getFlags();
	m_Name = other.getName();
	m_Ext = other.getExt();
	m_Directory = other.getDirectory();
	m_FullPath = other.getFullPath();
}

GLEFileLocation::GLEFileLocation(const char* file) {
	m_Name = file;
	m_FullPath = file;
	m_Flags = GLE_FILELOCATION_IS_LOCAL;
	GetExtension(m_Name, m_Ext);
}

GLEFileLocation::~GLEFileLocation() {
}

void GLEFileLocation::fromAbsolutePath(const string& path) {
	SplitFileName(path, m_Directory, m_Name);
	GetExtension(m_Name, m_Ext);
	setFullPath(path);
}

void GLEFileLocation::fromRelativePath(const string& dirname, const string& fname) {
	// don't modify fname, store it with setName(), also if it is a relative path [this is intentional]
	GLEGetFullPath(dirname, fname, m_FullPath);
	// note that fname can also contain part of the path
	GetDirName(m_FullPath, m_Directory);
	GetExtension(fname, m_Ext);
	setName(fname);
}

void GLEFileLocation::fromFileNameCrDir(const string& fname) {
	if (IsAbsPath(fname)) {
		fromAbsolutePath(fname);
	} else {
		string dirname;
		GLEGetCrDir(&dirname);
		fromRelativePath(dirname, fname);
	}
}

void GLEFileLocation::fromFileNameDir(const string& fname, const string& dirname) {
	if (IsAbsPath(fname)) {
		fromAbsolutePath(fname);
	} else {
		fromRelativePath(dirname, fname);
	}
}

void GLEFileLocation::createStdin() {
	m_Name = "stdin";
	m_Flags = GLE_FILELOCATION_IS_STDIN;
}

void GLEFileLocation::createStdout() {
	m_Name = "stdout";
	m_Flags = GLE_FILELOCATION_IS_STDOUT;
}

void GLEFileLocation::createIllegal() {
	m_Name = "illegal";
	m_Flags = GLE_FILELOCATION_IS_ILLEGAL;
}

void GLEFileLocation::initDirectory() {
	GetDirName(m_FullPath, m_Directory);
}

void GLEFileLocation::copy(const GLEFileLocation* other) {
	m_Flags = other->getFlags();
	m_Name = other->getName();
	m_Ext = other->getExt();
	m_Directory = other->getDirectory();
	m_FullPath = other->getFullPath();
}

void GLEFileLocation::addExtension(const char* ext) {
	const char* myExt = ext[0] == '.' ? (ext + 1) : ext;
	m_Ext = myExt;
	m_FullPath += ".";
	m_FullPath += myExt;
	if ((m_Flags & (GLE_FILELOCATION_IS_STDIN | GLE_FILELOCATION_IS_STDOUT | GLE_FILELOCATION_IS_ILLEGAL)) == 0) {
		m_Name += ".";
		m_Name += myExt;
	}
}

bool GLEFileLocation::isStream() {
	return (m_Flags & (GLE_FILELOCATION_IS_STDIN | GLE_FILELOCATION_IS_STDOUT)) != 0;
}

string GLEFileLocation::getFileName() {
	string result;
	SplitFileNameNoDir(m_FullPath, result);
	return result;
}

string GLEFileLocation::getMainName() {
	string result;
	SplitFileNameNoDir(m_FullPath, result);
	GetMainName(result, result);
	return result;
}

GLEErrorMessage::GLEErrorMessage() {
	m_Line = m_Column = -1;
	m_Delta = 0;
}

GLEErrorMessage::~GLEErrorMessage() {
}

GLEOutputStream::GLEOutputStream() {
	m_ExitCode = 0;
}

GLEOutputStream::~GLEOutputStream() {
}

void GLEOutputStream::println() {
	println("");
}

void GLEOutputStream::println(const char* str) {
	cerr << str << endl;
}

void GLEOutputStream::printflush(const char* str) {
	cerr << str;
	cerr.flush();
}

void GLEOutputStream::error(GLEErrorMessage* msg) {
	const char* file = msg->getFile();
	const char* abbrev = msg->getLineAbbrev();
	ostringstream output;
	output << endl;
	output << ">> " << file << " (" << msg->getLine() << ")";
	if (abbrev[0] != 0) {
		output << " |" << abbrev << "|";
	}
	if (msg->getColumn() != -1) {
		char number[50];
		output << endl;
		output << ">> ";
		sprintf(number, "%d", msg->getLine());
		int nbspc = strlen(file) + strlen(number) + 4 + msg->getColumn() - msg->getDelta();
		for (int i = 0; i < nbspc; i++) {
			output << " ";
		}
		output << "^";
	}
	output << msg->getErrorMsg();
	g_message(output.str().c_str());
}

GLEDrawObject::GLEDrawObject() {
	m_Flag = 0;
	m_Properties = NULL;
}

GLEDrawObject::~GLEDrawObject() {
	if (m_Properties != NULL) delete m_Properties;
}

void GLEDrawObject::initProperties(GLEInterface* iface) {
}

bool GLEDrawObject::needsAMove(GLEPoint& pt) {
	return false;
}

void GLEDrawObject::createGLECode(string& code) {
	code = "";
}

void GLEDrawObject::getPSBoundingBox(GLERectangle* box) {
}

const char* GLEDrawObject::getPostScriptCode() {
	return "";
}

void GLEDrawObject::updateBoundingBox() {
}

void GLEDrawObject::applyTransformation(bool dir) {
}

void GLEDrawObject::applyTransformationPt(GLEPoint* pt, bool dir) {
	if (dir) {
		double devx, devy;
		g_dev(pt->getX(), pt->getY(), &devx, &devy);
		pt->setXY((devx-1.0)/PS_POINTS_PER_INCH*CM_PER_INCH, (devy-1.0)/PS_POINTS_PER_INCH*CM_PER_INCH);
	} else {
		double xp, yp;
		g_undev(pt->getX()*PS_POINTS_PER_INCH/CM_PER_INCH+1.0, pt->getY()*PS_POINTS_PER_INCH/CM_PER_INCH+1.0, &xp, &yp);
		pt->setXY(xp, yp);
	}
}

GLEDrawObject* GLEDrawObject::clone() {
	return NULL;
}

GLEDrawObject* GLEDrawObject::deepClone() {
	GLEDrawObject* res = clone();
	GLEPropertyStore* store = getProperties();
	if (store != NULL) {
		res->setProperties(store->clone());
	}
	return res;
}

bool GLEDrawObject::approx(GLEDrawObject* other) {
	return false;
}

void GLEDrawObject::draw() {
}

bool GLEDrawObject::modified() {
	return true;
}

void GLEDrawObject::setProperties(GLEPropertyStore* store) {
	if (m_Properties != NULL) delete m_Properties;
	m_Properties = store;
}

GLEObjectDOConstructor::GLEObjectDOConstructor(GLESub* sub) {
	m_Sub = sub;
	m_NbExtra = sub->getNbParam();
	int first = 0;
	if (sub->getNbParam() >= 2 && str_i_equals(sub->getParamNameShort(0), "width") && str_i_equals(sub->getParamNameShort(1), "height")) {
		// First two arguments are width and height
		m_CanScale = true;
		add(new GLEProperty(sub->getParamNameShort(0).c_str(), GLEPropertyTypeReal, GLEDOPropertyUserArg));
		add(new GLEProperty(sub->getParamNameShort(1).c_str(), GLEPropertyTypeReal, GLEDOPropertyUserArg));
		first += 2;
	}
	for (int i = first; i < sub->getNbParam(); i++) {
		string arg = sub->getParamNameShort(i);
		add(new GLEProperty(arg.c_str(), GLEPropertyTypeString, GLEDOPropertyUserArg));
	}
	add(new GLEPropertyColor("Color"));
	add(new GLEPropertyFillColor("Fill color"));
	add(new GLEPropertyLWidth("Line width"));
	add(new GLEPropertyLStyle("Line style"));
	GLEPropertyNominal* linecap = new GLEPropertyNominal("Line cap", GLEPropertyTypeInt, GLEDOPropertyLineCap);
	linecap->addValue("butt", GLELineCapButt);
	linecap->addValue("round", GLELineCapRound);
	linecap->addValue("square", GLELineCapSquare);
	add(linecap);
	add(new GLEPropertyFont("Font"));
	add(new GLEPropertyHei("Font size"));
}

GLEObjectDOConstructor::~GLEObjectDOConstructor() {
}

void GLEObjectDOConstructor::scale(GLEDrawObject* obj, double sx, double sy) {
	GLEArrayImpl* props = obj->getProperties()->getArray();
	props->setDouble(0, sx);
	props->setDouble(1, sy);
}

const string& GLEObjectDOConstructor::getName() {
	return m_Sub->getName();
}

GLEScript* GLEObjectDOConstructor::getScript() {
	return m_Sub->getScript();
}

void output_error(ParserError& err);
void eval(GLEArrayImpl* stk, GLEPcodeList* pclist, int *pcode, int *cp, double *oval, GLEString **ostr, int *otyp);

GLEObjectDO* GLEObjectDOConstructor::constructObject() {
	GLEObjectDO* obj = new GLEObjectDO(this);
	obj->initProperties(m_Sub->getScript()->getGLEInterface());
	GLEArrayImpl* arr = obj->getProperties()->getArray();
	int first = 0;
	if (isSupportScale()) {
		// First two arguments are width and height
		arr->setDouble(0, atof(m_Sub->getDefault(0).c_str()));
		arr->setDouble(1, atof(m_Sub->getDefault(1).c_str()));
		first += 2;
	}
	for (int i = first; i < m_Sub->getNbParam(); i++) {
		string arg = m_Sub->getDefault(i);
		arr->setObject(i, new GLEString(arg));
	}
	obj->render();
	return obj;
}

GLEScript::GLEScript() {
	m_Run = NULL;
	m_Parser = NULL;
	m_Polish = NULL;
	m_Pcode = NULL;
}

GLEScript::~GLEScript() {
	cleanUp();
}

void GLEScript::cleanUp() {
	if (m_Run != NULL) delete m_Run;
	if (m_Parser != NULL) delete m_Parser;
	if (m_Polish != NULL) delete m_Polish;
	if (m_Pcode != NULL) {
		delete m_Pcode->getPcodeList();
		delete m_Pcode;
	}
	m_Run = NULL;
	m_Parser = NULL;
	m_Polish = NULL;
	m_Pcode = NULL;
	m_PostScriptCode.resize(0);
}

GLEDrawObject* GLEScript::newGLEObject(GLEDrawObjectType type) {
	GLEDrawObject* obj = NULL;
	switch (type) {
		case GDOLine:
			obj = new GLELineDO();
			obj->initProperties(GLEGetInterfacePointer());
			break;
		case GDOEllipse:
			obj = new GLEEllipseDO();
			obj->initProperties(GLEGetInterfacePointer());
			break;
		case GDOArc:
			obj = new GLEArcDO();
			obj->initProperties(GLEGetInterfacePointer());
			break;
		case GDOText:
			obj = new GLETextDO();
			((GLETextDO*)obj)->setModified(true);
			obj->initProperties(GLEGetInterfacePointer());
			break;
		default:
			break;
	}
	m_NewObjs.add(obj);
	return obj;
}

void GLEScript::cancelObject(GLEDrawObject* obj) {
	int pos = m_NewObjs.size()-1;
	while (pos > 0) {
		if (m_NewObjs[pos].get() == obj) {
			m_NewObjs.erase(m_NewObjs.begin()+pos);
			break;
		}
		pos--;
	}
}

GLEDrawObject* GLEScript::nextObject() {
	if (m_CurrObject < getNumberObjects()) {
		return getObject(m_CurrObject++);
	} else {
		return NULL;
	}
}

void GLEScript::clearNewObjects() {
	m_NewObjs.clear();
}

GLEDrawObjectType GLEScript::getType() {
	return GDOScript;
}

void GLEScript::updateObjectDOConstructors() {
	m_File.clearObjectDOConstructors();
	// update all
	GLESubMap* subs = getParser()->getSubroutines();
	for (int i = 0; i < subs->size(); i++) {
		GLESub* sub = subs->get(i);
		sub->setScript(this);
		bool okSub = true;
		if (!sub->isObject()) {
			okSub = false;
		} else {
			for (int j = 0; j < sub->getNbParam(); j++) {
				// Should have default value for each parameter
				if (sub->getDefault(j).length() == 0) okSub = false;
			}
		}
		if (okSub) {
			int line = sub->getStart();
			GLESourceFile* file = m_File.getLine(line)->getSource();
			GLEObjectDOConstructor* cons = sub->getObjectDOConstructor();
			file->addObjectDOConstructor(cons);
		}
	}
}

const char* GLEScript::getPostScriptCode()
{
	return m_PostScriptCode.c_str();
}

string* GLEScript::getRecordedBytesBuffer(int device)
{
	if (device == GLE_DEVICE_EPS) {
		return &m_PostScriptCode;
	} else if (device == GLE_DEVICE_PDF) {
		return &m_PDFCode;
	} else {
		CUtilsAssert(false);
		return 0;
	}
}

GLEHasArrowBase::GLEHasArrowBase():
	m_Arrow(GLEHasArrowNone)
{
}

GLELineDO::GLELineDO() {
}

GLELineDO::GLELineDO(const GLEPoint& p1, const GLEPoint& p2) : m_P1(p1), m_P2(p2) {
}

GLELineDO::GLELineDO(double x1, double y1, double x2, double y2) : m_P1(x1,y1), m_P2(x2,y2) {
}

GLELineDO::~GLELineDO() {
}

GLEDrawObjectType GLELineDO::getType() {
	return GDOLine;
}

void GLELineDO::initProperties(GLEInterface* iface) {
	m_Properties = new GLEPropertyStore(iface->getLinePropertyStoreModel());
	GLEInitLineProperties(m_Properties);
	GLEInitArrowProperties(m_Properties);
}

bool GLELineDO::needsAMove(GLEPoint& pt) {
	pt.set(getP1());
	return true;
}

namespace {

	void addArrowToCode(ostream& str, GLEHasArrow arrow) {
		if (arrow == (GLEHasArrowStart | GLEHasArrowEnd)) {
			str << " arrow both";
		} else if (arrow == GLEHasArrowStart) {
			str << " arrow start";
		} else if (arrow == GLEHasArrowEnd) {
			str << " arrow end";
		}
	}

}

void GLELineDO::createGLECode(string& code) {
	ostringstream str;
	str << "aline " << getP2().getX() << " " << getP2().getY();
	addArrowToCode(str, getArrow());
	code = str.str();
}

void GLELineDO::updateBoundingBox() {
	g_update_bounds(getP1().getX(), getP1().getY());
	g_update_bounds(getP2().getX(), getP2().getY());
	g_move(getP2().getX(), getP2().getY());
}

void GLELineDO::applyTransformation(bool dir) {
	applyTransformationPt(&m_P1, dir);
	applyTransformationPt(&m_P2, dir);
	double scale = g_get_avg_scale();
	GLEScaleSimpleLineProperties(scale, dir, getProperties());
	GLEScaleArrowProperties(scale, dir, getProperties());
}

GLEDrawObject* GLELineDO::clone() {
	GLELineDO* cl = new GLELineDO(getP1(), getP2());
	cl->setArrow(getArrow());
	return cl;
}

bool GLELineDO::approx(GLEDrawObject* other) {
	GLELineDO* otherc = (GLELineDO*)other;
	return getP1().approx(otherc->getP1()) && getP2().approx(otherc->getP2()) && getArrow() == otherc->getArrow();
}

GLEEllipseDO::GLEEllipseDO() {
	m_Rx = m_Ry = 0.0;
}

GLEEllipseDO::GLEEllipseDO(double x0, double y0, double rx, double ry) : m_Center(x0, y0) {
	m_Rx = rx;
	m_Ry = ry;
}

GLEEllipseDO::GLEEllipseDO(const GLEPoint& c, double rx, double ry) : m_Center(c) {
	m_Rx = rx;
	m_Ry = ry;
}

GLEEllipseDO::GLEEllipseDO(double x0, double y0, double r) : m_Center(x0, y0) {
	m_Rx = m_Ry = r;
}

GLEEllipseDO::~GLEEllipseDO() {
}

GLEDrawObjectType GLEEllipseDO::getType() {
	return GDOEllipse;
}

void GLEEllipseDO::setRadius(double r) {
	m_Rx = m_Ry = r;
}

GLEPoint GLEEllipseDO::getPoint(GLEJustify just) {
	switch (just) {
		case GLEJustifyRC:
			return GLEPoint(m_Center.getX()+m_Rx, m_Center.getY());
		case GLEJustifyTL:
			return GLEPoint(m_Center.getX()-m_Rx, m_Center.getY()+m_Ry);
		case GLEJustifyBR:
			return GLEPoint(m_Center.getX()+m_Rx, m_Center.getY()-m_Ry);
		default:
			break;
	}
	return GLEPoint();
}

void GLEEllipseDO::initProperties(GLEInterface* iface) {
	m_Properties = new GLEPropertyStore(iface->getShapePropertyStoreModel());
	GLEInitSimpleLineProperties(m_Properties);
	GLEInitShapeFillColor(m_Properties);
}

bool GLEEllipseDO::needsAMove(GLEPoint& pt) {
	pt.set(getCenter());
	return true;
}

void GLEEllipseDO::createGLECode(string& code) {
	ostringstream str;
	if (isCircle()) {
		str << "circle " << m_Rx;
	} else {
		str << "ellipse " << m_Rx << " " << m_Ry;
	}
	code = str.str();
}

void GLEEllipseDO::updateBoundingBox() {
	g_move(getCenter().getX(), getCenter().getY());
	g_update_bounds(getCenter().getX()-m_Rx,getCenter().getY()-m_Ry);
	g_update_bounds(getCenter().getX()+m_Rx,getCenter().getY()+m_Ry);
}

void GLEEllipseDO::applyTransformation(bool dir) {
	applyTransformationPt(&m_Center, dir);
	double sx, sy;
	g_get_scale(&sx, &sy);
	if (dir) {
		m_Rx *= sx;
		m_Ry *= sy;
	} else {
		m_Rx /= sx;
		m_Ry /= sy;
	}
	double scale = (sx+sy)/2.0;
	GLEScaleSimpleLineProperties(scale, dir, getProperties());
}

GLEDrawObject* GLEEllipseDO::clone() {
	return new GLEEllipseDO(m_Center, m_Rx, m_Ry);
}

bool GLEEllipseDO::approx(GLEDrawObject* other) {
	GLEEllipseDO* otherc = (GLEEllipseDO*)other;
	return m_Center.approx(otherc->getCenter()) &&
	       fabs(m_Rx - otherc->getRadiusX()) < 1e-6 &&
	       fabs(m_Ry - otherc->getRadiusY()) < 1e-6;
}

GLEArcDO::GLEArcDO() {
	m_Angle1 = m_Angle2 = 0.0;
}

GLEArcDO::GLEArcDO(double x0, double y0, double r, double a1, double a2) : GLEEllipseDO(x0, y0, r) {
	m_Angle1 = a1; m_Angle2 = a2;
}

GLEArcDO::GLEArcDO(double x0, double y0, double rx, double ry, double a1, double a2) : GLEEllipseDO(x0, y0, rx, ry) {
	m_Angle1 = a1; m_Angle2 = a2;
}

GLEArcDO::~GLEArcDO() {
}

GLEDrawObjectType GLEArcDO::getType() {
	return GDOArc;
}

void GLEArcDO::initProperties(GLEInterface* iface) {
	m_Properties = new GLEPropertyStore(iface->getLinePropertyStoreModel());
	GLEInitLineProperties(m_Properties);
	GLEInitArrowProperties(m_Properties);
}

bool GLEArcDO::needsAMove(GLEPoint& pt) {
	pt.set(getCenter());
	return true;
}

void GLEArcDO::createGLECode(string& code) {
	ostringstream str;
	double angle2 = g_arc_normalized_angle2(m_Angle1, m_Angle2);
	if (isCircle()) {
		str << "arc " << m_Rx << " " << m_Angle1 << " " << angle2;
	} else {

		str << "elliptical_arc " << m_Rx << " " << m_Ry << " " << m_Angle1 << " " << angle2;
	}
	addArrowToCode(str, getArrow());
	code = str.str();
}

void GLEArcDO::updateBoundingBox() {
	g_move(m_Center.getX(), m_Center.getY());
}

GLEDrawObject* GLEArcDO::clone() {
	GLEArcDO* result = new GLEArcDO(m_Center.getX(), m_Center.getY(), m_Rx, m_Ry, m_Angle1, m_Angle2);
	result->setArrow(getArrow());
	return result;
}

bool GLEArcDO::approx(GLEDrawObject* other) {
	GLEArcDO* otherc = (GLEArcDO*)other;
	return GLEEllipseDO::approx(other) &&
	       fabs(m_Angle1 - otherc->getAngle1()) < 1e-6 &&
	       fabs(m_Angle2 - otherc->getAngle2()) < 1e-6 &&
	       getArrow() == otherc->getArrow();
}

void GLEArcDO::normalize() {
	m_Angle2 = g_arc_normalized_angle2(m_Angle1, m_Angle2);
}

GLEPoint& GLEArcDO::getPoint1(GLEPoint& pt) {
	pt.set(m_Center);
	pt.add(m_Rx*cos(m_Angle1*GLE_PI/180.0), m_Ry*sin(m_Angle1*GLE_PI/180.0));
	return pt;
}

GLEPoint& GLEArcDO::getPoint2(GLEPoint& pt) {
	pt.set(m_Center);
	pt.add(m_Rx*cos(m_Angle2*GLE_PI/180.0), m_Ry*sin(m_Angle2*GLE_PI/180.0));
	return pt;
}

GLEPoint& GLEArcDO::getPointMid(GLEPoint& pt) {
	pt.set(m_Center);
	double angle2 = g_arc_normalized_angle2(m_Angle1, m_Angle2);
	double angleMid = (m_Angle1 + angle2)/2;
	pt.add(m_Rx*cos(angleMid*GLE_PI/180.0), m_Ry*sin(angleMid*GLE_PI/180.0));
	return pt;
}

GLETextDO::GLETextDO() : m_Modified(false) {
}

GLETextDO::GLETextDO(GLEPoint& position, const string& text) : m_Position(position), m_Text(text), m_Modified(false) {
}

GLETextDO::~GLETextDO() {
}

GLEDrawObjectType GLETextDO::getType() {
	return GDOText;
}

const char* GLETextDO::getPostScriptCode() {
	return m_PostScript.c_str();
}

void GLETextDO::getPSBoundingBox(GLERectangle* box) {
	box->copy(&m_PSBoundingBox);
}

void GLETextDO::initBB(double width, double height, double baseline) {
	m_PSBoundingBox.setXMin(0.0);
	m_PSBoundingBox.setXMax(width);
	m_PSBoundingBox.setYMin(0.0);
	m_PSBoundingBox.setYMax(height);
	m_BaseLine = baseline;
}

bool GLETextDO::needsAMove(GLEPoint& pt) {
	pt.set(getPosition());
	return true;
}

void GLETextDO::createGLECode(string& code) {
	ostringstream str;
	str << "write \"" << getText() << "\"";
	code = str.str();
}

void GLETextDO::updateBoundingBox() {
	GLEDevice* oldDevice = g_set_dummy_device();
	g_move(getPosition());
	g_text(getText());
	g_restore_device(oldDevice);
	g_move(getPosition());
}

void GLETextDO::applyTransformation(bool dir) {
	applyTransformationPt(&m_Position, dir);
	GLEPropertyStore* props = getProperties();
	if (props != NULL) {
		double scale = g_get_avg_scale();
		if (scale > 0) {
			double hei = props->getRealProperty(GLEDOPropertyFontSize);
			if (dir) {
				hei *= scale;
			} else {
				hei /= scale;
			}
			props->setRealProperty(GLEDOPropertyFontSize, hei);
		}
	}
}

GLEDrawObject* GLETextDO::clone() {
	return new GLETextDO(getPosition(), getText());
}

bool GLETextDO::approx(GLEDrawObject* other) {
	GLETextDO* othert = (GLETextDO*)other;
	return m_Position.approx(othert->getPosition()) &&
	       m_Text == othert->getText();
}

bool GLETextDO::modified() {
	return m_Modified;
}

void GLETextDO::setModified(bool modified) {
	m_Modified = modified;
}

void GLETextDO::initProperties(GLEInterface* iface) {
	int justify;
	m_Properties = new GLEPropertyStore(iface->getTextPropertyStoreModel());
	GLEInitColorProperty(m_Properties);
	iface->initTextProperties(m_Properties);
	g_get_just(&justify);
	m_Properties->setIntProperty(GLEDOPropertyJustify, justify);
}

void GLETextDO::render(GLEInterface* iface) {
	iface->renderText(this, getProperties());
}

GLEObjectDO::GLEObjectDO(GLEObjectDOConstructor* cons) {
	m_Cons = cons;
	setRefPointString(GLEString::getEmptyString());
}

GLEObjectDO::~GLEObjectDO() {
}

GLEDrawObjectType GLEObjectDO::getType() {
	return GDOObject;
}

const char* GLEObjectDO::getPostScriptCode() {
	return m_PostScript.c_str();
}

void GLEObjectDO::makePropertyStore() {
	if (m_Properties == NULL) {
		m_Properties = new GLEPropertyStore(m_Cons);
	}
}

void GLEObjectDO::getPSBoundingBox(GLERectangle* box) {
	box->copy(m_ObjRep->getRectangle());
	// Convert from points to cm
	box->translate(-1.0, -1.0);
	box->scale(CM_PER_INCH/PS_POINTS_PER_INCH);
}

bool GLEObjectDO::needsAMove(GLEPoint& pt) {
	pt.set(m_Position);
	return true;
}

void GLEObjectDO::createGLECode(string& code) {
	ostringstream str;
	GLESub* sub = getConstructor()->getSubroutine();
	string subname = sub->getName();
	gle_strlwr(subname);
	if (m_RefPoint.isNull()) {
		str << "draw " << subname;
	} else {
		str << "draw " << subname << "." << *m_RefPoint.get();
	}
	GLEArrayImpl* args = getProperties()->getArray();
	for (int i = 0; i < sub->getNbParam(); i++) {
		str << " ";
		gle_memory_cell_print(args->get(i), str);
	}
	code = str.str();
}

void GLEObjectDO::updateBoundingBox() {
	g_move(getPosition());
}

void GLEObjectDO::applyTransformation(bool dir) {
	applyTransformationPt(&m_Position, dir);
}

GLEDrawObject* GLEObjectDO::clone() {
	GLEObjectDO* res = new GLEObjectDO(m_Cons);
	res->setPosition(m_Position);
	res->setRefPointString(m_RefPoint.get());
	return res;
}

bool GLEObjectDO::approx(GLEDrawObject* other) {
	GLEObjectDO* otherc = (GLEObjectDO*)other;
	if (!otherc->getRefPointString()->equals(getRefPointString())) return false;
	GLEArrayImpl* my_args = getProperties()->getArray();
	GLEArrayImpl* other_args = otherc->getProperties()->getArray();
	GLESub* sub = getConstructor()->getSubroutine();
	for (int i = 0; i < sub->getNbParam(); i++) {
		GLEMemoryCell* a1 = my_args->get(i);
		GLEMemoryCell* a2 = other_args->get(i);
		if (!gle_memory_cell_equals(a1, a2)) return false;
	}
	return getPosition().approx(otherc->getPosition());
}

void GLEObjectDO::initProperties(GLEInterface* iface) {
	makePropertyStore();
	GLEInitLineProperties(m_Properties);
	GLEInitShapeFillColor(m_Properties);
	iface->initTextProperties(m_Properties);
}

void GLEObjectDO::render() {
	GLEObjectRepresention* newobj = new GLEObjectRepresention();
	setObjectRepresentation(newobj);
	GLESub* sub = m_Cons->getSubroutine();
	GLEScript* script = sub->getScript();
	if (script == NULL && sub->getStart() == -1) {
		// Subroutine does not have script attached properly
		newobj->getRectangle()->setXMin(-1.0);
		return;
	}
	GLEInterface* iface = script->getGLEInterface();
	GLESaveRestore saved_state;
	try {
		g_select_device(GLE_DEVICE_EPS);
		PSGLEDevice* dev = (PSGLEDevice*)g_get_device_ptr();
		dev->startRecording();
		saved_state.save();
		g_clear();
		g_resetfont();
		g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
		dev->startRecording();
		GLEPropertyStore* prop = getProperties();
		// Select the right color
		GLEColor* color = prop->getColorProperty(GLEDOPropertyColor);
		g_set_color(color);
		GLEColor* fill = prop->getColorProperty(GLEDOPropertyFillColor);
		g_set_fill(fill);
		// Select the right size
		double hei = prop->getRealProperty(GLEDOPropertyFontSize);
		if (hei == 0.0) g_set_hei(0.3633);
		else g_set_hei(hei);
		g_set_font_width(-1);
		g_set_line_style("1");
		g_set_line_width(prop->getRealProperty(GLEDOPropertyLineWidth));
		// Select the font
		GLEFont* font = prop->getFontProperty(GLEDOPropertyFont);
		if (font == NULL) {
			font = iface->getFont("rm");
		}
		g_set_font(font->getIndex());
		newobj->enableChildObjects();
		GLERun* run = script->getRun();
		run->setDeviceIsOpen(true);
		run->setCRObjectRep(newobj);
		/* draw to measure */
		GLEMeasureBox measure;
		measure.measureStart();
		g_move(0.0, 0.0);
		int cp = 0;
		GLEPcodeList pc_list;
		GLEPcode pcode(&pc_list);
		pcode.addInt(PCODE_EXPR);    /* Expression follows */
		int savelen = pcode.size();  /* Used to set actual length at end */
		pcode.addInt(0);             /* Length of expression */
		GLEPolish* polish = script->getPolish();
		if (polish != NULL) {
			string argi;
			GLEArrayImpl* arr = prop->getArray();
			for (int i = 0; i < sub->getNbParam(); i++) {
				int rtype = sub->getParamTypes()[i];
				if (arr->getType(i) == GLEObjectTypeDouble) {
					pcode.addDoubleExpression(arr->getDouble(i));
				} else {
					GLEString* args = (GLEString*)arr->getObject(i);
					args->toUTF8(argi);
					polish->polish(argi.c_str(), pcode, &rtype);
				}
			}
		}
		pcode.addFunction(sub->getIndex() + LOCAL_START_INDEX);
		pcode.setInt(savelen, pcode.size() - savelen - 1);
		GLERC<GLEArrayImpl> stk(new GLEArrayImpl());
		evalGeneric(stk.get(), &pc_list, (int*)&pcode[0], &cp);
		// g_flush() required to make sure that all line segments are output
		g_flush();
		measure.measureEnd();
		newobj->getRectangle()->copy(&measure);
		// cout << "Rectangle: " << *newobj->getRectangle() << endl;
		g_dev(newobj->getRectangle());
		// cout << "Dev. Rectangle: " << *newobj->getRectangle() << endl;
		run->setCRObjectRep(NULL);
		// Return the resulting PostScript code
		dev->getRecordedBytes(getPostScriptPtr());
		saved_state.restore();
	} catch (ParserError& err) {
		newobj->getRectangle()->setXMin(-1.0);
		output_error(err);
	}
}

void GLEObjectDO::computeReferencePoint(GLEPoint* pt) {
	if (m_RefPoint.isNull()) return;
	try {
		GLEJustify just;
		GLEObjectRepresention* myobj = getObjectRepresentation();
		GLERC<GLEArrayImpl> path(m_RefPoint->split('.'));
		GLEObjectRepresention* obj = GLERun::name_to_object(myobj, path.get(), &just, 0);
		if (obj != NULL) {
			GLERectangle rect;
			rect.copy(obj->getRectangle());
			rect.translate(1.0-myobj->getRectangle()->getXMin(), 1.0-myobj->getRectangle()->getYMin());
			g_undev(&rect);
			rect.toPoint(just, pt);
		} else {
			pt->setXY(0, 0);
		}
	} catch (ParserError& err) {
		output_error(err);
	}
}

GLEComposedObject::GLEComposedObject() {
}

GLEComposedObject::~GLEComposedObject() {
}

void GLEComposedObject::clear() {
	m_Objs.clear();
}

void GLEComposedObject::removeDeletedObjects() {
	int nbDeleted = 0;
	int nbTotal = getNumberObjects();
	for (int i = 0; i < nbTotal; i++) {
		GLEDrawObject* obj = getObject(i);
		if (obj->hasFlag(GDO_FLAG_DELETED)) {
			nbDeleted++;
		}
		if (i+nbDeleted < nbTotal) {
			setObject(i, getObject(i+nbDeleted));
		}
	}
	setNumberObjects(nbTotal - nbDeleted);
}

GLEFont::GLEFont() {
	m_Bold = m_Italic = m_BoldItalic = NULL;
	m_Index = m_Number = 0;
	m_Parent = 0;
}

GLEFont::~GLEFont() {
}

GLEFontStyle GLEFont::checkStyle(GLEFont* child) {
	if (m_Bold.get() == child) {
		return GLEFontStyleBold;
	} else if (m_Italic.get() == child) {
		return GLEFontStyleItalic;
	} else if (m_BoldItalic.get() == child) {
		return GLEFontStyleBoldItalic;
	} else {
		return GLEFontStyleRoman;
	}
}

GLEFont* GLEFont::getStyle(GLEFontStyle style) {
	switch (style) {
		case GLEFontStyleRoman:
			return this;
		case GLEFontStyleBold:
			return m_Bold.get();
		case GLEFontStyleItalic:
			return m_Italic.get();
		case GLEFontStyleBoldItalic:
			return m_BoldItalic.get();
		default:
			return NULL;
	}
}

void GLEFont::setStyle(GLEFontStyle style, GLEFont* font) {
	switch (style) {
		case GLEFontStyleBold:
			m_Bold = font;
			break;
		case GLEFontStyleItalic:
			m_Italic = font;
			break;
		case GLEFontStyleBoldItalic:
			m_BoldItalic = font;
			break;
		default:
			break;
	}
}

GLEFillBase::GLEFillBase() {
}

GLEFillBase::~GLEFillBase() {
}

GLEPatternFill::GLEPatternFill(int fillDescr) :
	m_fillDescription(fillDescr),
	m_background(new GLEColor())
{
	m_background->setHexValueGLE(GLE_COLOR_WHITE);
}

GLEPatternFill::~GLEPatternFill() {
}

GLEFillType GLEPatternFill::getFillType() {
	return GLE_FILL_TYPE_PATTERN;
}

GLEFillBase* GLEPatternFill::clone() {
	GLEPatternFill* result = new GLEPatternFill(m_fillDescription);
	result->setBackground(m_background->clone());
	return result;
}

GLEColor::GLEColor() :
	m_Name(0)
{
	setGray(0);
}

GLEColor::GLEColor(double r, double g, double b) :
	m_Name(0)
{
	setRGB(r, g, b);
}

GLEColor::GLEColor(double r, double g, double b, double a) :
	m_Name(0)
{
	setRGBA(r, g, b, a);
}

GLEColor::GLEColor(double gray) :
	m_Name(0)
{
	setGray(gray);
}

GLEColor::~GLEColor() {
	if (m_Name != NULL) delete m_Name;
}

GLEColor* GLEColor::clone() {
	GLEColor* result = new GLEColor(m_Red, m_Green, m_Blue, m_Alpha);
	result->setTransparent(isTransparent());
	result->setName(m_Name);
	if (isFill()) {
		result->setFill(getFill()->clone());
	}
	return result;
}

int GLEColor::getType() const {
	return GLEObjectTypeColor;
}

bool GLEColor::equalsApprox(GLEColor* other) {
	return equals_rel_fine(m_Red, other->m_Red) &&
		   equals_rel_fine(m_Green, other->m_Green) &&
		   equals_rel_fine(m_Blue, other->m_Blue) &&
		   equals_rel_fine(m_Alpha, other->m_Alpha) &&
		   m_Transparent == other->m_Transparent;
}

bool GLEColor::equals(GLEDataObject* obj) const {
	if (obj->getType() != GLEObjectTypeColor) return false;
	GLEColor* other = (GLEColor*)obj;
	return m_Red == other->m_Red &&
	       m_Green == other->m_Green &&
	       m_Blue == other->m_Blue &&
	       m_Alpha == other->m_Alpha &&
	       m_Transparent == other->m_Transparent;
}

void GLEColor::print(ostream& out) const {
	if (isTransparent()) {
		out << "clear";
	} else {
		bool found = false;
		// FIXME: don't do this look-up here - do it some place else (in QGLE?)
		// FIXME: AFTER4.2
		GLEColorList* list = GLEGetColorList();
		for (int i = 0; i < list->getNbColors(); i++) {
			GLEColor* def = list->getColor(i);
			if (equals(def)) {
				string name = def->getName();
				if (name != "") {
					gle_strlwr(name);
					out << name;
					found = true;
				}
			}
		}
		if (!found) {
			if (hasAlpha()) {
				out << "rgba255(" << (int)getRedI() << "," << (int)getGreenI() << "," << (int)getBlueI() << "," << (int)getAlphaI() << ")";
			} else {
				out << "rgb255(" << (int)getRedI() << "," << (int)getGreenI() << "," << (int)getBlueI() << ")";
			}
		}
	}
}

const char* GLEColor::getName() {
	return m_Name != NULL ? m_Name->c_str() : "";
}

void GLEColor::setName(const string& name) {
	delete m_Name;
	m_Name = new string(name);
}

void GLEColor::setName(const string* name) {
	delete m_Name;
	if (name != 0) {
		m_Name = new std::string(*name);
	} else {
		m_Name = 0;
	}
}

void GLEColor::setRGB(double r, double g, double b) {
	m_Red = r; m_Green = g; m_Blue = b;
	m_Alpha = 1.0;
	m_Transparent = false;
}

void GLEColor::setRGBA(double r, double g, double b, double a) {
	m_Red = r; m_Green = g; m_Blue = b; m_Alpha = a;
	m_Transparent = false;
}

void GLEColor::setRGB255(int r, int g, int b) {
	setRGB(((double)r) / 255.0, ((double)g) / 255.0, ((double)b) / 255.0);
}

void GLEColor::setRGB255(double r, double g, double b) {
	setRGB(r / 255.0, g / 255.0, b / 255.0);
}

void GLEColor::setRGBA255(double r, double g, double b, double a) {
	setRGBA(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

void GLEColor::setGray(double gray) {
	setRGB(gray, gray, gray);
}

void GLEColor::setHexValue(unsigned int v) {
	unsigned int red   = (v >> 16) & 0xFF;
	unsigned int green = (v >> 8) & 0xFF;
	unsigned int blue  = v & 0xFF;
	setRGB(((double)red) / 255.0, ((double)green) / 255.0, ((double)blue) / 255.0);
}

void GLEColor::setDoubleEncoding(double v) {
	union {
		double d;
		int l[2];
	} both;
	both.d = v;
	setHexValueGLE(both.l[0]);
	m_Alpha = double(both.l[1]) / 255.0;
}

double GLEColor::getDoubleEncoding() {
	union {
		double d;
		int l[2];
	} both;
	both.l[0] = getHexValueGLE();
	both.l[1] = getAlphaI();
	return both.d;
}

void GLEColor::setHexValueGLE(unsigned int hexValue) {
	if (hexValue == GLE_FILL_CLEAR) {
		setGray(0);
		m_Fill = 0;
		setTransparent(true);
	} else if ((hexValue & 0X02000000) != 0) {
		setGray(0);
		setFill(new GLEPatternFill(hexValue));
	} else {
		setHexValue(hexValue);
	}
}

unsigned int GLEColor::getHexValueGLE() {
	if (isTransparent()) {
		return GLE_FILL_CLEAR;
	} else if (isFill() && getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		GLEPatternFill* myFill = static_cast<GLEPatternFill*>(getFill());
		return myFill->getFillDescription();
	} else {
		unsigned int red = float_to_color_comp(m_Red);
		unsigned int green = float_to_color_comp(m_Green);
		unsigned int blue = float_to_color_comp(m_Blue);
		return 0x01000000 | (red << 16) | (green << 8) | blue;
	}
}

double GLEColor::getGray() {
	return (getRed() * 3.0 + getGreen() * 2.0 + getBlue()) / 6.0;
}

GLEPropertyStore::GLEPropertyStore(GLEPropertyStoreModel* model) {
	m_Model = model;
	m_Values.ensure(model->getNumberOfProperties());
}

GLEPropertyStore::~GLEPropertyStore() {
}

GLEPropertyStore* GLEPropertyStore::clone() {
	GLEPropertyStore* result = new GLEPropertyStore(getModel());
	for (size_t i = 0; i < m_Values.size(); i++) {
		result->setPropertyValue(i, m_Values.get(i));
	}
	return result;
}

void GLEPropertyStore::getPropertyAsString(GLEPropertyID id, string* result) {
	int idx = m_Model->find(id);
	GLEProperty* prop = m_Model->getProperty(idx);
	prop->getPropertyAsString(result, m_Values.get(idx));
}

GLEPropertyStoreModel::GLEPropertyStoreModel() {
	m_Hash = new IntIntHash();
	m_CanScale = false;
	m_NbExtra = 0;
}

GLEPropertyStoreModel::~GLEPropertyStoreModel() {
	delete m_Hash;
}

int GLEPropertyStoreModel::find(GLEPropertyID id) {
	return m_Hash->try_get(id);
}

void GLEPropertyStoreModel::add(GLEProperty* prop) {
	int id = m_Properties.size();
	m_Properties.push_back(prop);
	prop->setIndex(id);
	m_Hash->add_item(prop->getID(), id);
}

void GLEPropertyStoreModel::scale(GLEDrawObject* obj, double sx, double sy) {
}

GLEProperty::GLEProperty(const char* name, const char* cmdname, GLEPropertyType type, GLEPropertyID id) {
	m_Name = name;
	m_Type = type;
	m_ID = id;
	m_SetCmdName = cmdname;
	m_Index = -1;
}

GLEProperty::GLEProperty(const char* name, GLEPropertyType type, GLEPropertyID id) {
	m_Name = name;
	m_Type = type;
	m_ID = id;
	m_SetCmdName = NULL;
	m_Index = -1;
}

GLEProperty::~GLEProperty() {
}

void GLEProperty::getPropertyAsString(string* result, GLEMemoryCell* value) {
	GLEColor* color = NULL;
	GLEFont* font = NULL;
	GLEString* gstr = NULL;
	ostringstream str;
	switch (m_Type) {
		case GLEPropertyTypeInt:
			str << value->Entry.IntVal;
			break;
		case GLEPropertyTypeBool:
			if (value->Entry.BoolVal) str << "yes";
			else str << "no";
			break;
		case GLEPropertyTypeReal:
			str << value->Entry.DoubleVal;
			break;
		case GLEPropertyTypeString:
			gstr = (GLEString*)value->Entry.ObjectVal;
			str << *gstr;
			break;
		case GLEPropertyTypeColor:
			color = (GLEColor*)value->Entry.ObjectVal;
			color->print(str);
			break;
		case GLEPropertyTypeFont:
			font = (GLEFont*)value->Entry.ObjectVal;
			str << font->getName();
			break;
	}
	*result = str.str();
}

void GLEProperty::createSetCommandGLECode(ostream& os, GLEMemoryCell* value) {
	if (getSetCommandName() != NULL) {
		string str;
		getPropertyAsString(&str, value);
		os << " " << getSetCommandName() << " " << str;
	}
}

bool GLEProperty::isEqualToState(GLEPropertyStore* store) {
	return true;
}

void GLEProperty::updateState(GLEPropertyStore* store) {
}

GLEPropertyNominal::GLEPropertyNominal(const char* name, GLEPropertyType type, GLEPropertyID id) : GLEProperty(name, type, id) {
	m_Value2Name = new IntIntHash();
	m_Name2Value = new StringIntHash();
}

GLEPropertyNominal::~GLEPropertyNominal() {
	delete m_Value2Name;
	delete m_Name2Value;
}

void GLEPropertyNominal::addValue(const char* name, int value) {
	int pos = m_NomValues.size();
	m_NomValues.push_back(name);
	m_Value2Name->add_item(value, pos);
	m_Name2Value->add_item(name, value);
}

void GLEPropertyNominal::getPropertyAsString(string* result, GLEMemoryCell* value) {
	int pos = m_Value2Name->try_get(value->Entry.IntVal);
	if (pos == -1) {
		ostringstream res;
		res << value->Entry.IntVal;
		*result = res.str();
	} else {
		*result = m_NomValues[pos];
	}
}

GLEPropertyLWidth::GLEPropertyLWidth(const char* name) : GLEProperty(name, "lwidth", GLEPropertyTypeReal, GLEDOPropertyLineWidth) {
}

GLEPropertyLWidth::~GLEPropertyLWidth() {
}

bool GLEPropertyLWidth::isEqualToState(GLEPropertyStore* store) {
	double lwidth;
	g_get_line_width(&lwidth);
	return equals_rel_fine(lwidth, store->getRealProperty(this));
}

void GLEPropertyLWidth::updateState(GLEPropertyStore* store) {
	g_set_line_width(store->getRealProperty(this));
}

GLEPropertyLStyle::GLEPropertyLStyle(const char* name) : GLEProperty(name, "lstyle", GLEPropertyTypeString, GLEDOPropertyLineStyle) {
}

GLEPropertyLStyle::~GLEPropertyLStyle() {
}

bool GLEPropertyLStyle::isEqualToState(GLEPropertyStore* store) {
	char lstyle[15];
	g_get_line_style(lstyle);
	GLEString* str = store->getStringProperty(this);
	return str->length() == 0 || str->equalsI(lstyle);
}

void GLEPropertyLStyle::updateState(GLEPropertyStore* store) {
	char lstyle[15];
	store->getStringProperty(this)->toUTF8(lstyle);
	g_set_line_style(lstyle);
}

GLEPropertyColor::GLEPropertyColor(const char* name) : GLEProperty(name, "color", GLEPropertyTypeColor, GLEDOPropertyColor) {
}

GLEPropertyColor::~GLEPropertyColor() {
}

bool GLEPropertyColor::isEqualToState(GLEPropertyStore* store) {
	GLERC<GLEColor> color(g_get_color());
	GLEColor* gle_color = store->getColorProperty(this);
	return color->equalsApprox(gle_color);
}

void GLEPropertyColor::updateState(GLEPropertyStore* store) {
	g_set_color(store->getColorProperty(this));
}

GLEPropertyFillColor::GLEPropertyFillColor(const char* name) : GLEProperty(name, "fill", GLEPropertyTypeColor, GLEDOPropertyFillColor) {
}

GLEPropertyFillColor::~GLEPropertyFillColor() {
}

bool GLEPropertyFillColor::isEqualToState(GLEPropertyStore* store) {
	GLERC<GLEColor> curr_fill(g_get_fill());
	return curr_fill->equalsApprox(store->getColorProperty(this));
}

void GLEPropertyFillColor::updateState(GLEPropertyStore* store) {
	g_set_fill(store->getColorProperty(this));
}

GLEPropertyHei::GLEPropertyHei(const char* name) : GLEProperty(name, "hei", GLEPropertyTypeReal, GLEDOPropertyFontSize) {
}

GLEPropertyHei::~GLEPropertyHei() {
}

bool GLEPropertyHei::isEqualToState(GLEPropertyStore* store) {
	double fontsize;
	g_get_hei(&fontsize);
	return equals_rel_fine(fontsize, store->getRealProperty(this));
}

void GLEPropertyHei::updateState(GLEPropertyStore* store) {
	g_set_hei(store->getRealProperty(this));
}

GLEPropertyFont::GLEPropertyFont(const char* name) : GLEProperty(name, "font", GLEPropertyTypeFont, GLEDOPropertyFont) {
}

GLEPropertyFont::~GLEPropertyFont() {
}

bool GLEPropertyFont::isEqualToState(GLEPropertyStore* store) {
	int font;
	g_get_font(&font);
	GLEFont* gle_font = store->getFontProperty(this);
	return font == gle_font->getIndex();
}

void GLEPropertyFont::updateState(GLEPropertyStore* store) {
	g_set_font(store->getFontProperty(this)->getIndex());
}

GLEPropertyJustify::GLEPropertyJustify(const char* name) : GLEProperty(name, "just", GLEPropertyTypeInt, GLEDOPropertyJustify) {
}

GLEPropertyJustify::~GLEPropertyJustify() {
}

bool GLEPropertyJustify::isEqualToState(GLEPropertyStore* store) {
	int just;
	g_get_just(&just);
	return just == store->getIntProperty(this);
}

void GLEPropertyJustify::updateState(GLEPropertyStore* store) {
	g_set_just(store->getIntProperty(this));
}

void GLEPropertyJustify::getPropertyAsString(string* result, GLEMemoryCell* value) {
	int ival = value->Entry.IntVal;
	switch (ival) {
		case GLEJustifyCC:
			*result = "cc"; break;
		case GLEJustifyTL:
			*result = "tl"; break;
		case GLEJustifyTC:
			*result = "tc"; break;
		case GLEJustifyTR:
			*result = "tr"; break;
		case GLEJustifyBL:
			*result = "bl"; break;
		case GLEJustifyBC:
			*result = "bc"; break;
		case GLEJustifyBR:
			*result = "br"; break;
		case GLEJustifyLC:
			*result = "lc"; break;
		case GLEJustifyRC:
			*result = "rc"; break;
		case GLEJustifyLeft:
			*result = "left"; break;
		case GLEJustifyCenter:
			*result = "center"; break;
		case GLEJustifyRight:
			*result = "right"; break;
		default:
			*result = "?"; break;
	}
}

GLEPropertyArrowSize::GLEPropertyArrowSize(const char* name) : GLEProperty(name, "arrowsize", GLEPropertyTypeReal, GLEDOPropertyArrowSize) {
}

GLEPropertyArrowSize::~GLEPropertyArrowSize() {
}

bool GLEPropertyArrowSize::isEqualToState(GLEPropertyStore* store) {
	GLEArrowProps arrow;
	g_arrowsize(&arrow);
	return equals_rel_fine(arrow.size, store->getRealProperty(this));
}

void GLEPropertyArrowSize::updateState(GLEPropertyStore* store) {
	g_set_arrow_size(store->getRealProperty(this));
}

GLEPropertyArrowAngle::GLEPropertyArrowAngle(const char* name) : GLEProperty(name, "arrowangle", GLEPropertyTypeReal, GLEDOPropertyArrowAngle) {
}

GLEPropertyArrowAngle::~GLEPropertyArrowAngle() {
}

bool GLEPropertyArrowAngle::isEqualToState(GLEPropertyStore* store) {
	GLEArrowProps arrow;
	g_arrowsize(&arrow);
	return equals_rel_fine(arrow.angle, store->getRealProperty(this));
}

void GLEPropertyArrowAngle::updateState(GLEPropertyStore* store) {
	g_set_arrow_angle(store->getRealProperty(this));
}

void GLEInitColorProperty(GLEPropertyStore* prop) {
	prop->setColorProperty(GLEDOPropertyColor, g_get_color()->clone());
}

void GLEInitSimpleLineProperties(GLEPropertyStore* prop) {
	// line width property
	double lwidth;
	g_get_line_width(&lwidth);
	prop->setRealProperty(GLEDOPropertyLineWidth, lwidth);
	// line style
	char lstyle[15];
	g_get_line_style(lstyle);
	prop->setStringProperty(GLEDOPropertyLineStyle, new GLEString(lstyle));
	GLEInitColorProperty(prop);
}

void GLEScaleSimpleLineProperties(double scale, bool dir, GLEPropertyStore* prop) {
	if (prop != NULL && scale > 0) {
		double wd = prop->getRealProperty(GLEDOPropertyLineWidth);
		if (dir) {
			wd *= scale;
		} else {
			wd /= scale;
		}
		prop->setRealProperty(GLEDOPropertyLineWidth, wd);
	}
}

void GLEScaleArrowProperties(double scale, bool dir, GLEPropertyStore* prop) {
	if (prop != NULL && scale > 0) {
		double size = prop->getRealProperty(GLEDOPropertyArrowSize);
		if (dir) {
			size *= scale;
		} else {
			size /= scale;
		}
		prop->setRealProperty(GLEDOPropertyArrowSize, size);
	}
}

void GLEInitLineProperties(GLEPropertyStore* prop) {
	GLEInitSimpleLineProperties(prop);
	// cap
	int cap;
	g_get_line_cap(&cap);
	prop->setIntProperty(GLEDOPropertyLineCap, cap);
}

void GLESetDefaultArrowProperties(double lwd, GLEPropertyStore* prop) {
	if (!g_is_dummy_device()) {
		g_set_dummy_device();
	}
	g_set_line_width(lwd);
	g_set_hei(0.3633);
	GLEInitArrowProperties(prop);
}

void GLEInitArrowProperties(GLEPropertyStore* prop) {
	GLEArrowProps arrow;
	g_arrowsize(&arrow);
	prop->setRealProperty(GLEDOPropertyArrowSize, arrow.size);
	prop->setRealProperty(GLEDOPropertyArrowAngle, arrow.angle);
	prop->setIntProperty(GLEDOPropertyArrowStyle, arrow.style);
	prop->setIntProperty(GLEDOPropertyArrowTip, arrow.tip);
}

void GLEInitShapeFillColor(GLEPropertyStore* prop) {
	GLERC<GLEColor> gcolor(g_get_fill());
	prop->setColorProperty(GLEDOPropertyFillColor, gcolor->clone());
}
