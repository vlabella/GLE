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

#include "all.h"
#include "tokens/stokenizer.h"
#include "core.h"
#include "file_io.h"
#include "texinterface.h"
#include "cutils.h"
#include "gprint.h"
#include "cmdline.h"
#include "config.h"
#include "mem_limits.h"
#include "token.h"
#include "var.h"
#include "keyword.h"
#include "run.h"
#include "gle-poppler.h"

#define BEGINDEF extern
#include "begin.h"

#include <time.h>

void decode_utf8_basic(string& sc);

extern ConfigCollection g_Config;
extern CmdLineObj g_CmdLine;

class FourDoubleList {
protected:
	double m_List[4];
	int m_Pos;
public:
	FourDoubleList();
	double get(int idx);
	void add(double value);
};

FourDoubleList::FourDoubleList() {
	m_Pos = 0;
}

double FourDoubleList::get(int idx) {
	return m_List[(m_Pos + idx) % 4];
}

void FourDoubleList::add(double value) {
	m_List[m_Pos] = value;
	m_Pos = (m_Pos + 1) % 4;
}

TeXInterface TeXInterface::m_Instance;

TeXInterface::TeXInterface() {
	m_Enabled = true;
}

TeXInterface::~TeXInterface() {
	cleanUpObjects();
	cleanUpHash();
	for (int i = 0; i < getNbFontSizes(); i++) {
		delete getFontSize(i);
	}
}

TeXObject* TeXInterface::draw(const char* str) throw(ParserError) {
	TeXObjectInfo info;
	return draw(str, info, 1);
}

TeXObject* TeXInterface::draw(const std::string& str, GLERectangle* box) throw(ParserError) {
	TeXObjectInfo info;
	return draw(str.c_str(), info, 1, box);
}
TeXObject* TeXInterface::drawUTF8(const char* str, GLERectangle* box) throw(ParserError) {
	TeXObjectInfo info;
	string utf8 = str;
	decode_utf8_basic(utf8);
	return draw(utf8.c_str(), info, 1, box);
}

TeXObject* TeXInterface::draw(const char* str, int nblines, GLERectangle* box) throw(ParserError) {
	TeXObjectInfo info;
	return draw(str, info, nblines, box);
}

TeXObject* TeXInterface::draw(const char* str, TeXObjectInfo& info, int nblines, GLERectangle* box) throw(ParserError) {
	/* Load hash */
	tryLoadHash();
	/* Get object from hash */
	string obj_str = str;
	/* replace \'' by \" so that umlauts work in LaTeX expressions */
	str_replace_all(obj_str, "\\''", "\\\"");
	scaleObject(obj_str, 0.0);
	TeXHashObject* hobj = getHashObject(obj_str);
	hobj->setNbLines(nblines);
	hobj->setUsed(true);
	/* Construct object and add to list */
	return drawObj(hobj, info, box);
}

void TeXInterface::scaleObject(string& obj_str, double hei) {
	int scaleMode = getScaleMode();
	if (scaleMode != TEX_SCALE_MODE_NONE) {
		TeXPreambleInfo* preamble = getCurrentPreamble();
		if (!preamble->hasFontSizes()) checkTeXFontSizes();
		if (hei == 0) g_get_hei(&hei);
		if (scaleMode == TEX_SCALE_MODE_FIXED) {
			int best_size = preamble->getBestSizeFixed(hei);
			if (best_size != -1) {
				string prefix = "{\\" + getFontSize(best_size)->getName() + " ";
				obj_str = prefix + obj_str + "}";
			}
		} else {
			int best_size = preamble->getBestSizeScaled(hei);
			if (best_size != -1) {
				double scale = hei / preamble->getFontSize(best_size);
				stringstream sstr;
				sstr << "\\scalebox{" << scale << "}{{\\";
				sstr << getFontSize(best_size)->getName();
				sstr << " " << obj_str << "}}";
				obj_str = sstr.str();
			}
		}
	}
}

TeXObject* TeXInterface::drawObj(TeXHashObject* hobj, TeXObjectInfo& info, GLERectangle* box) throw(ParserError) {
	/* Throw exception if disabled*/
	if (!isEnabled()) {
		g_throw_parser_error("safe mode - TeX subsystem has been disabled");
	}
	/* Update unknown fields of info structure */
	info.initializeAll();
	/* Check if dimensions available */
	double width = 1.0, height = 0.5, baseline = 0.1;
	if (hobj->hasDimensions()) {
		width = hobj->getWidth();
		height = hobj->getHeight();
		baseline = hobj->getBaseline();
		// cout << "OBJ: " << width << "x" << height << " + " << baseline << endl;
	}
	/* Handle justify */
	double xp = info.getXp();
	double yp = info.getYp();
	int just = info.getJustify();
	g_dotjust(&xp, &yp, 0.0, width, height, 0.0, just);
	if ((just & 0x100) != 0) {
		yp -= baseline;
	}
	/* Update bounding box */
	g_update_bounds(xp, yp+height);
	g_update_bounds(xp+width, yp);
	if (box != NULL) {
		box->setXMin(xp); box->setXMax(xp+width);
		box->setYMin(yp); box->setYMax(yp+height);
	}
	/* Create object */
	if ((info.getFlags() & TEX_OBJ_INF_DONT_PRINT) == 0 && !g_is_dummy_device()) {
		TeXObject* obj = new TeXObject();
		obj->setObject(hobj);
		obj->setXY(xp, yp);
		m_TeXObjects.push_back(obj);
		obj->setColor(info.getColor());
		/* Apply input transformation */
		double devx, devy;
		g_dev(xp, yp, &devx, &devy);
		obj->setDeviceXY(devx/PS_POINTS_PER_INCH*CM_PER_INCH, devy/PS_POINTS_PER_INCH*CM_PER_INCH);
		/* Find out if text should be rotated */
		double angle = g_get_angle_deg();
		if (fabs(angle) > 1e-6) {
			obj->setAngle(angle);
		}
		/* Return object */
		return obj;
	} else {
		return NULL;
	}
}

void TeXInterface::checkObjectDimensions() {
	GLEDevice* psdev = g_get_device_ptr();
	double x0 = 0.0;
	double y0 = 0.0;
	double x1 = (double)psdev->getBoundingBox()->getX()/PS_POINTS_PER_INCH*CM_PER_INCH;
	double y1 = (double)psdev->getBoundingBox()->getY()/PS_POINTS_PER_INCH*CM_PER_INCH;
	for (vector<TeXObject*>::size_type i = 0; i < m_TeXObjects.size(); i++) {
		TeXObject* obj = m_TeXObjects[i];
		TeXHashObject* hobj = obj->getObject();
		if (hobj != NULL && hobj->hasDimensions()) {
			double cos_alpha = cos(obj->getAngle()*GLE_PI/180.0);
			double sin_alpha = sin(obj->getAngle()*GLE_PI/180.0);
			double ox0 = obj->getDXp();
			double oy0 = obj->getDYp();
			double ox1 = ox0 + hobj->getWidth()*cos_alpha;
			double oy1 = oy0 + hobj->getWidth()*sin_alpha;
			double ox2 = ox1 - hobj->getHeight()*sin_alpha;
			double oy2 = oy1 + hobj->getHeight()*cos_alpha;
			double ox3 = ox0 - hobj->getHeight()*sin_alpha;
			double oy3 = oy0 + hobj->getHeight()*cos_alpha;
			if (ox0 < x0 || ox0 > x1 || oy0 < y0 || oy0 > y1 ||
			    ox1 < x0 || ox1 > x1 || oy1 < y0 || oy1 > y1 ||
			    ox2 < x0 || ox2 > x1 || oy2 < y0 || oy2 > y1 ||
			    ox3 < x0 || ox3 > x1 || oy3 < y0 || oy3 > y1) {
			    	string error = "TeX object '";
				hobj->addFirstLine(&error);
				error += "' outside bounding box";
				g_message(error);
			}
		}
	}
}

int TeXInterface::createObj(const char* str, double hei) {
	/* Load hash */
	tryLoadHash();
	/* Create object given string */
	string obj_str = str;
	/* replace \'' by \" so that umlauts work in LaTeX expressions */
	str_replace_all(obj_str, "\\''", "\\\"");
	scaleObject(obj_str, hei);
	int result = getHashObjectIndex(obj_str);
	m_TeXHash[result]->setUsed(true);
	return result;
}

void TeXInterface::addHashObject(TeXHashObject* obj) {
	m_TeXHash.push_back(obj);
}

int TeXInterface::getHashObjectIndex(const string& line) {
	for (vector<TeXHashObject*>::size_type i = 0; i < m_TeXHash.size(); i++) {
		if (m_TeXHash[i]->getLine() == line) {
			return i;
		}
	}
	TeXHashObject* hobj = new TeXHashObject(line);
	addHashObject(hobj);
	m_HashModified = 1;
	return m_TeXHash.size()-1;
}

TeXHashObject* TeXInterface::getHashObject(const string& line) {
	return m_TeXHash[getHashObjectIndex(line)];
}

TeXHashObject* TeXInterface::getHashObject(int idx) {
	return m_TeXHash.getHashObject(idx);
}

void TeXInterface::initialize(GLEFileLocation* dotfile, GLEFileLocation* oname) {
	//cout << "TeXInterface::initialize('" << dotfile << "', '" << oname << "')" << endl;
	/* cleanUp */
	cleanUpObjects();
	cleanUpHash();
	/* Initialize flags */
	m_HashLoaded = TEX_INTERFACE_HASH_LOADED_NONE;
	m_HashModified = 0;
	updateNames(dotfile, oname);
	/* Get font sizes */
	initTeXFontScales();
}

void TeXInterface::updateNames(GLEFileLocation* dotfile, GLEFileLocation* oname) {
	m_MainOutputName.copy(oname);
	/* File name without extension */
	if (dotfile->getFullPath() == "") {
		m_HashName = "";
		m_DotDir = GLETempDirName();
		m_DotDir += ".gle";
	} else {
		string infile, file;
		GetMainNameExt(dotfile->getFullPath(), ".gle", infile);
		SplitFileName(infile, m_DotDir, file);
		m_DotDir += ".gle";
		m_HashName = m_DotDir;
		m_HashName += DIR_SEP;
		m_HashName += file;
		m_HashName += DIR_SEP;
		m_HashName += file;
		m_HashName += "_tex";
	}
}

void TeXInterface::updateOutName(GLEFileLocation* oname) {
	m_MainOutputName.copy(oname);
}

void TeXInterface::reset() {
	resetPreamble();
	cleanUpObjects();
	setScaleMode(TEX_SCALE_MODE_FIXED);
	m_HashModified = 0;
	/* Remove unused objects */
	for (int i = m_TeXHash.size()-1; i >= 0; i--) {
		TeXHashObject* hobj = m_TeXHash[i];
		if (!hobj->isUsed()) {
			delete hobj;
			m_TeXHash.erase(m_TeXHash.begin() + i);
		}
	}
}

void TeXInterface::resetPreamble() {
	m_Preambles.selectDefault();
}

void TeXInterface::cleanUpObjects() {
	for (vector<TeXObject*>::size_type i = 0; i < m_TeXObjects.size(); i++) {
		delete m_TeXObjects[i];
	}
	m_TeXObjects.clear();
}

void TeXInterface::cleanUpHash() {
	m_TeXHash.cleanUp();
}

void TeXInterface::tryLoadHash() {
	if (m_HashLoaded != TEX_INTERFACE_HASH_LOADED_FULL && m_HashName != "") {
		if (m_HashLoaded != TEX_INTERFACE_HASH_LOADED_PARTIAL) {
			loadTeXLines();
		}
		m_TeXHash.loadTeXPS(m_HashName);
		m_HashModified = 0;
		m_HashLoaded = TEX_INTERFACE_HASH_LOADED_FULL;
	}
}

void TeXInterface::loadTeXLines() {
	string strfile = m_HashName;
	strfile += ".texlines";
	ifstream str_file(strfile.c_str());
	if (str_file.is_open()) {
		string line;
		while (!str_file.eof()) {
			int len = ReadFileLine(str_file, line);
			if (len != 0) {
				if (strncmp("tex", line.c_str(), 3)  == 0) {
					line.erase(0, 4);
					TeXHashObject* hobj = new TeXHashObject(line);
					addHashObject(hobj);
				} else {
					line.erase(0, 9);
					string object_string;
					int nblines = atoi(line.c_str());
					for (int i = 0; i < nblines; i++) {
						ReadFileLine(str_file, line);
						if (object_string.length() == 0) {
							object_string = line;
						} else {
							object_string += "\7";
							object_string += line;
						}
					}
					TeXHashObject* hobj = new TeXHashObject(object_string);
					addHashObject(hobj);
				}
			}
		}
		str_file.close();
	}
}

void TeXInterface::writeInc(ostream& out, const char* prefix) {
	/* First output include for GLE .eps/.pdf file */
	out << "\\setlength{\\unitlength}{1cm}%" << endl;
	// out << "\\begingroup\\makeatletter\\ifx\\SetFigFont\\undefined%" << endl;
	// out << "\\gdef\\SetFigFont#1#2#3#4#5{%" << endl;
	// out << "   \\reset@font\\fontsize{#1}{#2pt}%" << endl;
	// out << "   \\fontfamily{#3}\\fontseries{#4}\\fontshape{#5}%" << endl;
	// out << "   \\selectfont}%" << endl;
	// out << "\\fi\\endgroup%" << endl;
	/* Get page size */
	double width, height, pic_a, pic_b;
	if (g_is_fullpage()) {
		g_get_pagesize(&width, &height);
		pic_a = width;
		pic_b = height;
	} else {
		g_get_usersize(&width, &height);
		pic_a = width + 0.075;
		pic_b = height + 0.075;
	}
	double pic_c = 0;
	double pic_d = 0;
	/* Output picture commands */
	out << "\\noindent{}\\begin{picture}(" << pic_a << "," << pic_b << ")";
	out <<                 "(" << pic_c << "," << pic_d << ")%" << endl;
	out << "\\put(0,0)";
	// double offs = -CM_PER_INCH/72;
	// out << "\\put(" << offs << "," << offs << ")";
	string incname;
	SplitFileNameNoDir(m_MainOutputName.getFullPath(), incname);
	FileNameDotToUnderscore(incname); /* includegraphics does not handle "." in filenames */
	out << "{\\includegraphics{" << prefix << incname << "_inc}}" << endl;
	for (vector<TeXObject*>::size_type i = 0; i < m_TeXObjects.size(); i++) {
		TeXObject* obj = m_TeXObjects[i];
		obj->output(out);
	}
	out << "\\end{picture}%" << endl;
}

void TeXInterface::createInc(const string& prefix) {
	// .inc files can also be created if there are no TeX objects
	// this is for (a.o. Makefile) consistency - see bug #2165591
	string inc_name = m_MainOutputName.getFullPath();
	inc_name += ".inc";
	ofstream inc_file(inc_name.c_str());
	writeInc(inc_file, prefix.c_str());
	inc_file.close();
}

void TeXInterface::createPreamble(ostream& tex_file) {
	ConfigSection* tex = g_Config.getSection(GLE_CONFIG_TEX);
	CmdLineArgSet* texsys =	(CmdLineArgSet*)tex->getOptionValue(GLE_TEX_SYSTEM);
	tex_file << getDocumentClass() << endl;
	if (texsys->hasValue(GLE_TEX_SYSTEM_VTEX)) {
		tex_file << "\\usepackage{graphics}" << endl;
	} else {
		tex_file << "\\usepackage[dvips]{graphics}" << endl;
	}
	for (int i = 0; i < getNbPreamble(); i++) {
		tex_file << getPreamble(i) << endl;
	}
}

void TeXInterface::createTeX(bool usegeom) {
	if (m_TeXObjects.size() != 0) {
		int m_type;
		double m_width, m_height, pic_a, pic_b;
		if (g_is_fullpage()) {
			g_get_pagesize(&m_width, &m_height, &m_type);
			pic_a = m_width;
			pic_b = m_height;
		} else {
			g_get_usersize(&m_width, &m_height);
			pic_a = m_width + 0.075;
			pic_b = m_height + 0.075;
			m_type = GLE_PAPER_UNKNOWN;
		}
		string tex_name = m_MainOutputName.getFullPath();
		tex_name += ".tex";
		if (GLEFileExists(tex_name)) {
			g_throw_parser_error("GLE needs to create a temporary file '", tex_name.c_str(), "', but this file already exists");
		}
		ofstream tex_file(tex_name.c_str());
		createPreamble(tex_file);
		tex_file << "\\usepackage{color}" << endl;
		if (usegeom) {
			tex_file << "\\usepackage{geometry}" << endl;
			tex_file << "\\geometry{%" << endl;
			tex_file << "  paperwidth=" << pic_a << "cm," << endl;
			tex_file << "  paperheight=" << pic_b << "cm," << endl;
			tex_file << "  left=0in," << endl;
			tex_file << "  right=0in," << endl;
			tex_file << "  top=0in," << endl;
			tex_file << "  bottom=0in" << endl;
			tex_file << "}" << endl;
		}
		tex_file << "\\pagestyle{empty}" << endl;
		tex_file << "\\begin{document}" << endl;
		writeInc(tex_file, "");
		tex_file << "\\end{document}" << endl;
		tex_file.close();
	}
}

int TeXInterface::tryCreateHash() {
	if (m_HashModified != 0 && m_TeXObjects.size() != 0) {
		createHiddenDir();
		saveTeXLines();
		m_TeXHash.saveTeXPS(m_HashName, this);
		if (!createTeXPS()) {
			/* error! */
			return 2;
		}
		m_HashLoaded = TEX_INTERFACE_HASH_LOADED_PARTIAL;
		return 1;
	} else {
		return 0;
	}
}

void TeXInterface::removeDotFiles() {
	string dir;
	GetDirName(m_HashName, dir);
	DeleteFileWithExt(m_HashName, ".aux");
	DeleteFileWithExt(m_HashName, ".log");
	DeleteFileWithExt(m_HashName, ".tex");
	DeleteFileWithExt(m_HashName, ".dvi");
	DeleteFileWithExt(m_HashName, ".ps");
	DeleteFileWithExt(m_HashName, ".texlines");
	TryDeleteDir(dir);
}

void TeXInterface::createHiddenDir() {
	string dir;
	GetDirName(m_HashName, dir);
	EnsureMkDir(dir);
}

void TeXInterface::saveTeXLines() {
	string strfile = m_HashName;
	strfile += ".texlines";
	ofstream str_file(strfile.c_str());
	for (vector<TeXHashObject*>::size_type i = 0; i < m_TeXHash.size(); i++) {
		TeXHashObject* obj = m_TeXHash[i];
		if (obj->isUsed()) obj->outputLog(str_file);
	}
	str_file.close();
}

bool TeXInterface::createTeXPS() {
	return createTeXPS(m_HashName);
}

bool TeXInterface::createTeXPS(const string& filestem) {
	string dir, file;
	SplitFileName(filestem, dir, file);
	if (!run_latex(dir, file)) return false;
	return run_dvips(filestem, false);
}

void TeXInterface::initTeXFontScales() {
	addSize(new TeXSize("tiny"));
	addSize(new TeXSize("scriptsize"));
	addSize(new TeXSize("footnotesize"));
	addSize(new TeXSize("small"));
	addSize(new TeXSize("normalsize"));
	addSize(new TeXSize("large"));
	addSize(new TeXSize("Large"));
	addSize(new TeXSize("LARGE"));
	addSize(new TeXSize("huge"));
	addSize(new TeXSize("Huge"));
}

void TeXInterface::checkTeXFontSizes() {
	TeXPreambleInfo* preamble = getCurrentPreamble();
	if (preamble->hasFontSizes()) return;
	/* Create .gle subdirectory */
	string dir = m_DotDir;
	EnsureMkDir(dir);
	dir += DIR_SEP;
	dir += "texpreamble";
	/* Try to load from file */
	m_Preambles.load(dir, this);
	if (preamble->hasFontSizes()) return;
	/* Font sizes not found, recompute */
	TeXHash tex_hash;
	for (int i = 0; i < getNbFontSizes(); i++) {
		string obj_name;
		TeXSize* size = getFontSize(i);
		size->createObject(&obj_name);
		TeXHashObject* obj = new TeXHashObject(obj_name);
		tex_hash.push_back(obj);
		obj->setUsed(true);
	}
	/* Get sizes of these fonts */
	tex_hash.saveTeXPS(dir, this);
	createTeXPS(dir);
	tex_hash.loadTeXPS(dir);
	/* Save this info in the current preamble */
	retrieveTeXFontSizes(tex_hash, preamble);
	/* Save font sizes */
	m_Preambles.save(dir);
}

void TeXInterface::retrieveTeXFontSizes(TeXHash& tex_hash, TeXPreambleInfo* preamble) {
	for (int i = 0; i < getNbFontSizes(); i++) {
		string obj_name;
		TeXSize* size = getFontSize(i);
		size->createObject(&obj_name);
		TeXHashObject* obj = tex_hash.getHashObjectOrNULL(obj_name);
		if (obj == NULL || !obj->hasDimensions()) {
			cout << ">>> error: did not get size for TeX font!" << endl;
		} else {
			// Round values by converting them to a string
			// -> Results in same rounding as if values are read from .pinfo file!
			stringstream str_value;
			double size_value = obj->getHeight() * FONT_HEI_FACTOR;
			str_value << size_value;
			str_value >> size_value;
			preamble->setFontSize(i, size_value);
			// cout << "font: " << size->getName() << " size: " << size_value << endl;
		}
	}
	preamble->setHasFontSizes(true);
}

bool TeXPreambleKey::equals(const TeXPreambleKey* key) const {
	if (getDocumentClass() != key->getDocumentClass()) return false;
	int nb = getNbPreamble();
	if (nb != key->getNbPreamble()) return false;
	for (int i = 0; i < nb; i++) {
		if (getPreamble(i) != key->getPreamble(i)) return false;
	}
	return true;
}

void TeXPreambleKey::copyFrom(const TeXPreambleKey* other) {
	setDocumentClass(other->getDocumentClass());
	int nb = other->getNbPreamble();
	for (int i = 0; i < nb; i++) {
		addPreamble(other->getPreamble(i));
	}
}

TeXPreambleInfo::TeXPreambleInfo() {
	m_HasFontSizes = false;
}

void TeXPreambleInfo::setFontSize(int font, double size) {
	while ((int)m_FontSizes.size() <= font) {
		m_FontSizes.push_back(0.0);
	}
	m_FontSizes[font] = size;
}

double TeXPreambleInfo::getFontSize(int font) {
	return font >= (int)m_FontSizes.size() ? 1.0 : m_FontSizes[font];
}

void TeXPreambleInfo::save(ostream& os) {
	int nb = getNbPreamble();
	os << "preamble: " << nb << endl;
	os << getDocumentClass() << endl;
	for (int i = 0; i < nb; i++) {
		os << getPreamble(i) << endl;
	}
	for (int i = 0; i < getNbFonts(); i++) {
		if (i != 0) os << " ";
		os << getFontSize(i);
	}
	os << endl;
}

void TeXPreambleInfo::load(istream& is, TeXInterface* iface) {
	for (int i = 0; i < iface->getNbFontSizes(); i++) {
		double size = 0.0;
		is >> size;
		setFontSize(i, size);
	}
	setHasFontSizes(true);
}

int TeXPreambleInfo::getBestSizeFixed(double hei) {
	int best_size = -1;
	double best_dist = GLE_INF;
	for (int i = 0; i < getNbFonts(); i++) {
		double dist = fabs(hei - getFontSize(i));
		if (dist < best_dist) {
			best_dist = dist;
			best_size = i;
		}
	}
	return best_size;
}

int TeXPreambleInfo::getBestSizeScaled(double hei) {
	for (int i = 0; i < getNbFonts(); i++) {
		double size_i = getFontSize(i);
		if (size_i >= hei) {
			return i;
		}
	}
	return getNbFonts()-1;
}

TeXPreambleInfoList::TeXPreambleInfoList() {
	m_Current = new TeXPreambleInfo();
	m_Current->setDocumentClass("\\documentclass{article}");
	addPreamble(m_Current);
}

TeXPreambleInfoList::~TeXPreambleInfoList() {
	for (int i = 0; i < getNbPreambles(); i++) {
		delete getPreamble(i);
	}
}

TeXPreambleInfo* TeXPreambleInfoList::findOrAddPreamble(const TeXPreambleKey* pre_key) {
	for (int i = 0; i < getNbPreambles(); i++) {
		TeXPreambleInfo* info = getPreamble(i);
		if (pre_key->equals(info)) return info;
	}
	TeXPreambleInfo* info = new TeXPreambleInfo();
	info->copyFrom(pre_key);
	addPreamble(info);
	return info;
}

void TeXPreambleInfoList::save(const string& filestem) {
	string file = filestem + ".pinfo";
	ofstream pream_file(file.c_str());
	for (int i = 0; i < getNbPreambles(); i++) {
		if (getPreamble(i)->hasFontSizes()) {
			getPreamble(i)->save(pream_file);
		}
	}
	pream_file.close();
}

void TeXPreambleInfoList::load(const string& filestem, TeXInterface* iface) {
	string file = filestem + ".pinfo";
	ifstream pream_file(file.c_str());
	if (pream_file.is_open()) {
		string line;
		TeXPreambleKey pre_key;
		while (pream_file.good()) {
			int len = ReadFileLine(pream_file, line);
			if (len != 0) {
				if (strncmp("preamble:", line.c_str(), 9)  == 0) {
					line.erase(0, 10);
					int nblines = atoi(line.c_str());
					ReadFileLine(pream_file, line);
					pre_key.clear();
					pre_key.setDocumentClass(line);
					for (int i = 0; i < nblines; i++) {
						ReadFileLine(pream_file, line);
						pre_key.addPreamble(line);
					}
					TeXPreambleInfo* info = findOrAddPreamble(&pre_key);
					info->load(pream_file, iface);
				} else {
					/* error in file format? */
					return;
				}
			}
		}
	}
	pream_file.close();
}

TeXHash::TeXHash() {
}

TeXHash::~TeXHash() {
	cleanUp();
}

void TeXHash::cleanUp() {
	for (unsigned int i = 0; i < size(); i++) {
		delete get(i);
	}
	clear();
}

TeXHashObject* TeXHash::getHashObject(int idx) {
	if (idx >= (int)size()) return NULL;
	return get(idx);
}

TeXHashObject* TeXHash::getHashObjectOrNULL(const string& line) {
	for (vector<TeXHashObject*>::size_type i = 0; i < size(); i++) {
		if (get(i)->getLine() == line) {
			return get(i);
		}
	}
	return NULL;
}

void TeXHash::saveTeXPS(const string& filestem, TeXInterface* iface) {
	string tex = filestem;
	tex += ".tex";
	ofstream hash_file(tex.c_str());
	/* Output preamble */
	iface->createPreamble(hash_file);
	hash_file << "\\pagestyle{empty}" << endl;
	hash_file << "\\begin{document}" << endl;
	/* First object is for calibrating */
	hash_file << "\\newpage" << endl;
	hash_file << "\\noindent{}\\rule{1cm}{0.025cm}\\framebox{\\rule{1cm}{1cm}}" << endl << endl;
	/* Output other objects */
	for (vector<TeXHashObject*>::size_type i = 0; i < size(); i++) {
		TeXHashObject* obj = get(i);
		if (obj->isUsed()) obj->outputMeasure(hash_file);
	}
	hash_file << "\\end{document}" << endl;
	hash_file.close();
}

void TeXHash::loadTeXPS(const string& filestem) {
	int objindex = -1;
	double adjWidth = 0.0, adjHeight = 0.0, adjBaseline = 0.0;
	string ps_name = filestem;
	ps_name += ".ps";
	StreamTokenizerMax tokens(ps_name,' ', 50);
	while (tokens.hasMoreTokens()) {
		const char* token = tokens.nextToken();
		if (str_i_equals(token, "%%PAGE:")) {
			int found = 0;
			FourDoubleList list;
			double unitsPerCm = 0.0, width = 0.0, height = 0.0, baseline = 0.0, adjustY = 0.0;
			while (found < 3 && tokens.hasMoreTokens()) {
				token = tokens.nextToken();
				if (str_i_equals(token, "v")) {
					double d2 = list.get(1);
					double d3 = list.get(2);
					double d4 = list.get(3);
					switch (found) {
						case 0:	/* Units/cm and baseline */
							unitsPerCm = d3;
							adjustY = d2;
							break;
						case 1:	/* Width */
							width = d3;
							break;
						case 2:	/* Height and baseline */
							height = d4;
							baseline = d2-adjustY;
							break;

					}
					// cout << "v = " << d1 << " " << d2 << " " << d3 << " " << d4 << endl;
					found++;
				} else {
					char* pos;
					double value = strtod(token, &pos);
					list.add(value);
				}
			}
			if (found == 3 && unitsPerCm != 0.0) {
				width /= unitsPerCm; height /= unitsPerCm; baseline /= unitsPerCm;
				if (objindex == -1) {
					/* Reference object is 1cm x 1cm square, no baseline */
					adjWidth = width-1.0;
					adjHeight = height-1.0;
					adjBaseline = baseline;
				} else {
					width -= adjWidth; height -= adjHeight;	baseline -= adjBaseline;
					TeXHashObject* hobj = getHashObject(objindex);
					if (hobj != NULL) {
						// cout << width << "x" << height << " + " << baseline << " " << hobj->getLine() << endl;
						hobj->setDimension(width, height, baseline);
					}
				}
				// cout << "width = " << width << " height = " << height << endl;
			}
			objindex++;
		}
	}

	tokens.close();
}

TeXSize::TeXSize(const char* name) {
	m_Name = name;
}

void TeXSize::createObject(string* name) {
	*name =  "{\\";
	*name += getName();
	*name += " H}";
}

TeXObject::TeXObject() {
	m_Xp = 0.0;
	m_Yp = 0.0;
	m_DXp = 0.0;
	m_DYp = 0.0;
	m_Angle = 0.0;
	m_Object = NULL;
}

void TeXObject::output(ostream& os) {
	if (!hasObject()) {
		return;
	}
	int closeb = 1;
	double angle = m_Angle;
	double pic_x = m_DXp;
	double pic_y = m_DYp;
	os << "\\put(" << pic_x << "," << pic_y << "){";
	if (angle != 0.0) {
		os << "\\rotatebox{" << angle << "}{";
		closeb++;
	}
	os << "\\makebox(0,0)[lb]{";
	if (!isBlack()) {
		GLERC<GLEColor> color(getColor());
		os << "\\color[rgb]{" << color->getRed() << "," << color->getGreen() << "," << color->getBlue() << "}";
	}
	getObject()->outputLines(os);
	for (int i = 0; i < closeb; i++) {
		os << "}";
	}
	os << "}" << endl;
}

void TeXObject::getDimensions(double* x1, double *y1, double *x2, double *y2) {
	*x1 = m_Xp;
	*y1 = m_Yp;
	*x2 = m_Xp + getWidth();
	*y2 = m_Yp + getHeight();
}

int TeXObject::isBlack() {
	GLEColor* color = getColor();
	return color == 0 || color->getHexValueGLE() == GLE_COLOR_BLACK;
}

TeXHashObject::TeXHashObject(const string& line) : m_Line(line) {
	m_Width = 10.0;
	m_Height = 10.0;
	m_Baseline = 0.0;
	m_HasDimensions = 0;
	m_Used = 0;
	m_NbLines = 0;
}

void TeXHashObject::outputLines(ostream& os) {
	if (getNbLines() <= 1) {
		os << getLine();
	} else {
		char_separator sep("\7");
		tokenizer<char_separator> tok(getLine(), sep);
		os << "%" << endl;
		int cnt = 0;
		while (tok.has_more()) {
			if (cnt != 0) os << endl;
			os << tok.next_token();
			cnt++;
		}
	}
}

void TeXHashObject::addFirstLine(string* str) {
	if (getNbLines() <= 1) {
		(*str) += getLine();
	} else {
		char_separator sep("\7");
		tokenizer<char_separator> tok(getLine(), sep);
		if (tok.has_more()) (*str) += tok.next_token();
	}
}

void TeXHashObject::outputMeasure(ostream& os) {
	os << "\\newpage" << endl;
	os << "\\noindent{}\\rule{1cm}{0.025cm}\\framebox{";
	outputLines(os);
	os << "}" << endl << endl;
}

void TeXHashObject::outputLog(ostream& os) {
	if (getNbLines() <= 1) {
		os << "tex " << getLine() << endl;
	} else {
		char_separator sep("\7");
		tokenizer<char_separator> tok(getLine(), sep);
		os << "multitex " << getNbLines() << endl;
		while (tok.has_more()) {
			os << tok.next_token() << endl;
		}
	}
}

void TeXHashObject::setDimension(double width, double height, double baseline) {
	m_Width = width;
	m_Height = height;
	m_Baseline = baseline;
	m_HasDimensions = 1;
}

TeXObjectInfo::TeXObjectInfo() {
	m_Status = 0;
}

void TeXObjectInfo::setJustify(int just) {
	m_Just = just;
	m_Status |= TEX_OBJ_INF_HAS_JUSTIFY;
}

void TeXObjectInfo::setColor(const GLERC<GLEColor>& color) {
	m_Color = color;
	m_Status |= TEX_OBJ_INF_HAS_COLOR;
}

void TeXObjectInfo::setPosition(double xp, double yp) {
	m_Xp = xp; m_Yp = yp;
	m_Status |= TEX_OBJ_INF_HAS_POSITION;
}

void TeXObjectInfo::initializeAll() {
	if ((m_Status & TEX_OBJ_INF_HAS_POSITION) == 0) {
		g_get_xy(&m_Xp, &m_Yp);
	}
	if ((m_Status & TEX_OBJ_INF_HAS_JUSTIFY) == 0) {
		g_get_just(&m_Just);
	}
	if ((m_Status & TEX_OBJ_INF_HAS_COLOR) == 0) {
		m_Color = g_get_color();
	}
}

bool read_eps_and_adjust_bounding_box(const string& name, GLEScript* script) {
	int b1 = 0, b2 = 0, b3 = 0, b4 = 0;
	string fileName(name + ".eps");
	vector<string> lines;
	if (!GLEReadFile(fileName, &lines)) {
		return false;
	}
	unsigned int pos = 0;
	ostringstream out;
	while (pos < lines.size()) {
		string line(lines[pos++]);
		if (g_parse_ps_boundingbox(line, &b1, &b2, &b3, &b4))
		{
			time_t t;
			t = time(0);
			GLEPoint bb(script->getBoundingBox());
			// Make sure to use proper Adobe DSC comments
			// Windows Explorer can show these in the document properties of a PDF created by GLE
			string vers_nosnap = g_get_version_nosnapshot();
			out << "%%Creator: GLE " << vers_nosnap << " <www.gle-graphics.org>" << endl;
			out << "%%CreationDate: " << ctime(&t);
			out << "%%Title: " << script->getLocation()->getName() << endl;
			int b3New = (int)ceil(b1 + bb.getX() + 1e-6);
			int b4New = (int)ceil(b2 + bb.getY() + 1e-6);
			out << "%%BoundingBox: " << b1 << " " << b2 << " " << b3New << " " << b4New << endl;
			script->setBoundingBoxOrigin(b1, b2);
			script->setBoundingBox(b3New - b1 + 1, b4New - b2 + 1);
		}
		else if (str_starts_with_trim(line, "%%HiResBoundingBox") != -1 ||
		         str_starts_with_trim(line, "%%Creator") != -1 ||
		         str_starts_with_trim(line, "%%CreationDate") != -1 ||
		         str_starts_with_trim(line, "%%Title") != -1)
		{
			// Forget about these
		}
		else if (str_starts_with_trim(line, "%%EndComments") != -1)
		{
			out << line << endl;
			break;
		}
		else
		{
			out << line << endl;
		}
	}
	while (pos < lines.size()) {
		string line(lines[pos++]);
		out << line << endl;
	}
	string* buffer = script->getRecordedBytesBuffer(GLE_DEVICE_EPS);
	*buffer = out.str();
	return true;
}

bool create_bitmap_file_ghostscript(GLEFileLocation* fname, int device, int dpi, int options, GLEScript* script) {
	ostringstream gsargs;
	gsargs << "-q -DNOPLATFONTS -dTextAlphaBits=4 -dGraphicsAlphaBits=4 -dBATCH -dNOPAUSE -r";
	gsargs << dpi;
	string* bytesPDF = script->getRecordedBytesBuffer(GLE_DEVICE_PDF);
	if (bytesPDF->empty()) {
		GLEPoint bb(script->getBoundingBox());
		int img_wd = GLEBBoxToPixels(dpi, bb.getX());
		int img_hi = GLEBBoxToPixels(dpi, bb.getY());
		gsargs << " -g" << img_wd << "x" << img_hi;
	}
	string gs_opts = g_CmdLine.getOptionString(GLE_OPT_GSOPTIONS);
	if (gs_opts != "") {
		str_replace_all(gs_opts, "\\", "");
		gsargs << " " << gs_opts;
	}
	bool bw = (options & GLE_OUTPUT_OPTION_GRAYSCALE) != 0;
	bool transp = (options & GLE_OUTPUT_OPTION_TRANSPARENT) != 0;
	gsargs << " -sDEVICE=";
	switch (device) {
		case GLE_DEVICE_PNG:
			if (bw) {
				gsargs << "pnggray";
			} else {
				gsargs << (transp ? "pngalpha" : "png16m");
			}
			break;
		case GLE_DEVICE_JPEG:
			gsargs << (bw ? "jpeggray" : "jpeg");
			break;
	}
	string outputfile;
	if (fname->isStdout()) {
		gsargs << " -sOutputFile=-";
	} else {
		outputfile = fname->getFullPath();
		switch (device) {
			case GLE_DEVICE_PNG: outputfile += ".png"; break;
			case GLE_DEVICE_JPEG: outputfile += ".jpg"; break;
		}
		gsargs << " -sOutputFile=\"" << outputfile << "\"";
	}
	gsargs << " -";
	string* bytesEPS = script->getRecordedBytesBuffer(GLE_DEVICE_EPS);
	if (!bytesPDF->empty()) {
		stringstream pdfCode;
		pdfCode.write(bytesPDF->data(), bytesPDF->size());
		return run_ghostscript(gsargs.str(), outputfile, !fname->isStdout(), &pdfCode);
	} else {
		stringstream postscript;
		GLEPoint origin(script->getBoundingBoxOrigin());
		postscript << (-origin.getX()) << " " << (-origin.getY()) << " translate" << endl;
		postscript.write(bytesEPS->data(), bytesEPS->size());
		return run_ghostscript(gsargs.str(), outputfile, !fname->isStdout(), &postscript);
	}
}

bool create_bitmap_file(GLEFileLocation* fname, int device, int dpi, int options, GLEScript* script) {
#ifdef HAVE_POPPLER
	bool supportsBitmapType = g_bitmap_supports_type(g_device_to_bitmap_type(device));
	string* bytesPDF = script->getRecordedBytesBuffer(GLE_DEVICE_PDF);
	if (supportsBitmapType && !bytesPDF->empty()) {
		std::string myFName = fname->getFullPath();
		myFName += g_device_to_ext(device);
		if (g_verbosity() >= 5) {
			g_message(std::string("[Poppler PDF conversion: ") + myFName + "]");
		}
		gle_convert_pdf_to_image_file((char*)bytesPDF->c_str(), (int)bytesPDF->size(), dpi, device, options, myFName.c_str());
		return true;
	}
#endif // HAVE_POPPLER
	return create_bitmap_file_ghostscript(fname, device, dpi, options, script);
}

bool create_pdf_file_ghostscript(GLEFileLocation* fname, int dpi, GLEScript* script) {
	ostringstream gsargs;
	gsargs << "-q";
	int compr_mode = g_get_pdf_image_format();
	switch (compr_mode) {
		case PDF_IMG_COMPR_AUTO:
			gsargs << " -dAutoFilterColorImages=true";
			gsargs << " -dAutoFilterGrayImages=true";
			gsargs << " -dEncodeColorImages=true";
			gsargs << " -dEncodeGrayImages=true";
			gsargs << " -dEncodeMonoImages=false"; break;
		case PDF_IMG_COMPR_ZIP:
			gsargs << " -dAutoFilterColorImages=false";
			gsargs << " -dAutoFilterGrayImages=false";
			gsargs << " -dEncodeColorImages=true";
			gsargs << " -dEncodeGrayImages=true";
			gsargs << " -dEncodeMonoImages=true";
			gsargs << " -dColorImageFilter=/FlateEncode";
			gsargs << " -dGrayImageFilter=/FlateEncode";
			gsargs << " -dMonoImageFilter=/FlateEncode"; break;
		case PDF_IMG_COMPR_JPEG:
			gsargs << " -dAutoFilterColorImages=false";
			gsargs << " -dAutoFilterGrayImages=false";
			gsargs << " -dEncodeColorImages=true";
			gsargs << " -dEncodeGrayImages=true";
			gsargs << " -dEncodeMonoImages=true";
			gsargs << " -dColorImageFilter=/DCTEncode";
			gsargs << " -dGrayImageFilter=/DCTEncode";
			gsargs << " -dMonoImageFilter=/FlateEncode"; break;
		case PDF_IMG_COMPR_PS:
			gsargs << " -dAutoFilterColorImages=false";
			gsargs << " -dAutoFilterGrayImages=false";
			gsargs << " -dEncodeColorImages=false";
			gsargs << " -dEncodeGrayImages=false";
			gsargs << " -dEncodeMonoImages=false"; break;
	}
	gsargs << " -dBATCH -dNOPAUSE -r" << dpi;
	GLEPoint bb(script->getBoundingBox());
	GLEPoint origin(script->getBoundingBoxOrigin());
	int img_wd = GLEBBoxToPixels(dpi, bb.getX());
	int img_hi = GLEBBoxToPixels(dpi, bb.getY());
	gsargs << " -g" << img_wd << "x" << img_hi;
	gsargs << " -sDEVICE=pdfwrite";
	gsargs << " -dPDFSETTINGS=/prepress -dMaxSubsetPct=100 -dSubsetFonts=true";
	gsargs << " -dEmbedAllFonts=true -dAutoRotatePages=/None";
	string outputfile;
	if (fname->isStdout()) {
		gsargs << " -sOutputFile=-";
	} else {
		outputfile = fname->getFullPath() + ".pdf";
		gsargs << " -sOutputFile=\"" << outputfile << "\"";
	}
	gsargs << " -";
	stringstream postscript;
	string* bytes = script->getRecordedBytesBuffer(GLE_DEVICE_EPS);
	postscript << (-origin.getX()) << " " << (-origin.getY()) << " translate" << endl;
	postscript.write(bytes->data(), bytes->size());
	return run_ghostscript(gsargs.str(), outputfile, !fname->isStdout(), &postscript);
}

void report_latex_errors_parse_error(istream& strm, string* result) {
	string line;
	stringstream err;
	int state = 0;
	while (state != 2 && !strm.eof()) {
		getline(strm, line);
		str_trim_right(line);
		if (state == 1 && line == "") {
			state = 2;
		} else if (state == 0 && line.length() > 2 && line[0] == 'l' && line[1] == '.') {
			state = 1;
			err << line << endl;
		} else if (line != "") {
			err << line << endl;
		}
	}
	*result = err.str();
}

bool report_latex_errors(istream& strm, const string& cmdline) {
	bool has_error = false;
	bool has_cmdline = false;
	if (g_verbosity() >= 5) {
		has_cmdline = true;
	}
	string line, next, prev;
	while (!strm.eof()) {
		getline(strm, line);
		if (line.length() > 1 && line[0] == '!') {
			if (!has_cmdline) {
				ostringstream msg;
				msg << "Error running: " << cmdline;
				g_message(msg.str());
				has_cmdline = true;
			}
			stringstream err;
			err << ">> LaTeX error:" << endl;
			err << line << endl;
			report_latex_errors_parse_error(strm, &next);
			if (!(str_i_equals(line, "! Emergency stop.") && str_i_equals(next, prev))) {
				err << next;
				g_message(err.str());
				inc_nb_errors();
			}
			prev = next;
			has_error = true;
		}
	}
	return has_error;
}

bool post_run_latex(bool result_ok, stringstream& output, const string& cmdline) {
	if (g_verbosity() >= 10) {
		g_message(output.str());
		return result_ok;
	} else {
		if (result_ok) {
			return !report_latex_errors(output, cmdline);
		} else {
			bool has_err = report_latex_errors(output, cmdline);
			if (!has_err) {
				ostringstream msg;
				msg << "Error running: " << cmdline << endl;
				msg << output.str();
				g_message(msg.str());
			}
			return false;
		}
	}
}

bool create_pdf_file_pdflatex(const string& fname, GLEScript* script) {
	string file, dir;
	SplitFileName(fname, dir, file);
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
	string cmdline(get_tool_path(GLE_TOOL_PDFTEX_CMD, tools));
	str_try_add_quote(cmdline);
	const string pdftex_opts(tools->getOptionString(GLE_TOOL_PDFTEX_OPTIONS));
	if (!pdftex_opts.empty()) {
		cmdline += " ";
		cmdline += pdftex_opts;
	}
	cmdline += string(" \"") + file + ".tex\"";
	string outfile = file + ".pdf";
	if (g_verbosity() >= 5) {
		ostringstream msg;
		msg << "[Running: " << cmdline << "]";
		g_message(msg.str());
	}
	stringstream output;
	TryDeleteFile(outfile);
	int result = GLESystem(cmdline, true, true, NULL, &output);
	bool result_ok = (result == GLE_SYSTEM_OK) && GLEFileExists(outfile);
	post_run_latex(result_ok, output, cmdline);
	DeleteFileWithExt(fname, ".aux");
	DeleteFileWithExt(fname, ".log");
	if (result_ok) {
		std::vector<char> pdfData;
		if (GLEReadFileBinary(outfile, &pdfData) && !pdfData.empty()) {
			string* buffer = script->getRecordedBytesBuffer(GLE_DEVICE_PDF);
			*buffer = std::string(&pdfData[0], pdfData.size());
		}
	}
	return result_ok;
}

bool run_latex(const string& dir, const string& file) {
	string crdir;
	if (dir != "") {
		GLEGetCrDir(&crdir);
		if (!GLEChDir(dir)) {
			gprint("Can't find directory: {%s}", dir.c_str());
			return false;
		}
	}
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
	string cmdline(get_tool_path(GLE_TOOL_LATEX_CMD, tools));
	str_try_add_quote(cmdline);
	const string latex_opts(tools->getOptionString(GLE_TOOL_LATEX_OPTIONS));
	if (!latex_opts.empty()) {
		cmdline += " ";
		cmdline += latex_opts;
	}
	cmdline += string(" \"") + file + ".tex\"";
	string outfile = file + ".dvi";
	if (g_verbosity() >= 5) {
		ostringstream msg;
		msg << "[Running: " << cmdline << "]";
		g_message(msg.str());
	}
	stringstream output;
	TryDeleteFile(outfile);
	int result = GLESystem(cmdline, true, true, NULL, &output);
	bool result_ok = (result == GLE_SYSTEM_OK) && GLEFileExists(outfile);
	result_ok = post_run_latex(result_ok, output, cmdline);
	if (crdir.length() != 0) GLEChDir(crdir);
	return result_ok;
}

bool create_eps_file_latex_dvips(const string& fname, GLEScript* script) {
	string file, dir;
	ConfigSection* tex = g_Config.getSection(GLE_CONFIG_TEX);
	CmdLineArgSet* texsys =	(CmdLineArgSet*)tex->getOptionValue(GLE_TEX_SYSTEM);
	SplitFileName(fname, dir, file);
	if (!run_latex(dir, file)) return false;
	if (!run_dvips(fname, true)) return false;
	bool result = read_eps_and_adjust_bounding_box(fname, script);
	DeleteFileWithExt(fname, ".aux");
	if (texsys->hasValue(GLE_TEX_SYSTEM_VTEX)) {
		DeleteFileWithExt(fname, ".ps");
	} else {
		DeleteFileWithExt(fname, ".dvi");
	}
	DeleteFileWithExt(fname, ".log");
	return result;
}

bool create_ps_file_latex_dvips(const string& fname) {
	string file, dir;
	ConfigSection* tex = g_Config.getSection(GLE_CONFIG_TEX);
	CmdLineArgSet* texsys =	(CmdLineArgSet*)tex->getOptionValue(GLE_TEX_SYSTEM);
	SplitFileName(fname, dir, file);
	if (!run_latex(dir, file)) return false;
	if (!run_dvips(fname, false)) return false;
	DeleteFileWithExt(fname, ".aux");
	if (!texsys->hasValue(GLE_TEX_SYSTEM_VTEX)) {
		DeleteFileWithExt(fname, ".dvi");
	}
	DeleteFileWithExt(fname, ".log");
	return true;
}

void post_run_process(bool result_ok, const char* procname, const string& cmdline, const string& output) {
	if (!result_ok || g_verbosity() >= 5) {
		ostringstream msg;
		if (!result_ok) {
			if (procname != NULL) {
				msg << "Error running " << procname << ":" << endl;
				if (g_verbosity() < 5) {
					// If verbosity higher, then we already printed this
					msg << "Running: " << cmdline << endl;
				}
			} else {
				msg << "Error running: " << cmdline << endl;
			}
		}
		msg << output;
		g_message(msg.str());
	}
}

bool run_ghostscript(const string& args, const string& outfile, bool redirout, istream* is) {
	ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
	string cmdline(get_tool_path(GLE_TOOL_GHOSTSCRIPT_CMD, tools));
	str_try_add_quote(cmdline);
	const string gs_opts(tools->getOptionString(GLE_TOOL_GHOSTSCRIPT_OPTIONS));
	if (!gs_opts.empty()) {
		cmdline += " ";
		cmdline += gs_opts;
	}
	cmdline += " ";
	cmdline += args;
	if (g_verbosity() >= 5) {
		ostringstream msg;
		msg << "[Running: " << cmdline << "]";
		g_message(msg.str());
	}
	ostringstream output;
	bool outfile_ok = true;
	int result = GLE_SYSTEM_OK;
	if (outfile != "" && IsAbsPath(outfile)) {
		// Checking if the output file is present is the best way to double check that Ghostscript worked
		TryDeleteFile(outfile);
		result = GLESystem(cmdline, true, redirout, is, &output);
		if (!GLEFileExists(outfile)) {
			outfile_ok = false;
		}
	} else {
		result = GLESystem(cmdline, true, redirout, is, &output);
	}
	string output_s = output.str();
	bool result_ok = (outfile_ok && result == GLE_SYSTEM_OK && str_i_str(output_s, "error:") == -1);
	post_run_process(result_ok, "Ghostscript", cmdline, output_s);
	return (result == GLE_SYSTEM_OK) && outfile_ok;
}

bool run_dvips(const string& file, bool eps) {
	ConfigSection* tex = g_Config.getSection(GLE_CONFIG_TEX);
	CmdLineArgSet* texsys =	(CmdLineArgSet*)tex->getOptionValue(GLE_TEX_SYSTEM);
	if (texsys->hasValue(GLE_TEX_SYSTEM_VTEX)) {
		// VTeX creates directly PS or PDF, no DVI!
		// use ghostscript to convert PS to EPS
		if (eps) {
			string gsargs;
			string outputfile(file + ".eps");
			gsargs += "-dNOPAUSE -sDEVICE=epswrite -sOutputFile=";
			gsargs += outputfile;
			gsargs += " -q -sBATCH \"";
			gsargs += file;
			gsargs += ".ps\"";
			return run_ghostscript(gsargs, outputfile);
		} else {
			return true;
		}
	} else {
		// get dvips command
		ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
		string dvips_cmd(get_tool_path(GLE_TOOL_DVIPS_CMD, tools));
		str_try_add_quote(dvips_cmd);
		// construct dvips arguments
		ostringstream dvipsargs;
		dvipsargs << dvips_cmd;
		const string dvips_opts(tools->getOptionString(GLE_TOOL_DVIPS_OPTIONS));
		if (!dvips_opts.empty()) dvipsargs << " " << dvips_opts;
		if (eps) dvipsargs << " -E";
		const string outfile(file + (eps ? ".eps" : ".ps"));
		dvipsargs << " -o \"" << outfile << "\" \"" << file << ".dvi\"";
		string cmdline = dvipsargs.str();
		if (g_verbosity() >= 5) {
			ostringstream msg;
			msg << "[Running: " << cmdline << "]";
			g_message(msg.str());
		}
		ostringstream output;
		TryDeleteFile(outfile);
		int result = GLESystem(cmdline, true, true, NULL, &output);
		bool result_ok = (result == GLE_SYSTEM_OK) && GLEFileExists(outfile);
		post_run_process(result_ok, NULL, cmdline, output.str());
		return result_ok;
	}
}

void begin_tex_preamble(int *pln, int *pcode, int *cp) {
	TeXInterface* iface = TeXInterface::getInstance();
	iface->resetPreamble();
	// Start with pcode from the next line
	(*pln)++;
	begin_init();
	TeXPreambleKey pre_key;
	pre_key.setDocumentClass(iface->getDocumentClass());
	while (true) {
		int st = begin_token(&pcode,cp,pln,srclin,tk,&ntk,outbuff);
		if (!st) {
			/* exit loop */
			break;
		}
		string mline = srclin;
		str_trim_both(mline);
		if (str_i_str(mline.c_str(), "\\documentclass") != NULL) {
			pre_key.setDocumentClass(mline);
		} else {
			pre_key.addPreamble(mline);
		}
	}
	TeXPreambleInfo* info = iface->getPreambles()->findOrAddPreamble(&pre_key);
	iface->getPreambles()->select(info);
}

void begin_tex(GLERun* run, int *pln, int *pcode, int *cp) {
	// Get optional params
	GLERC<GLEString> name;
	double add = 0.0;
	int ptr = *(pcode + (*cp)); /* add */
	if (ptr) {
		int zzcp = 0;
		add = evalDouble(run->getStack(), run->getPcodeList(), pcode + (*cp) + ptr, &zzcp);
	}
	(*cp) = (*cp) + 1;
	ptr = *(pcode + (*cp)); /* name */
	if (ptr) {
		int zzcp = 0;
		name = evalString(run->getStack(), run->getPcodeList(), pcode + (*cp) + ptr, &zzcp, true);
	}
	// Start with pcode from the next line
	(*pln)++;
	begin_init();
	string text_block;
	int nblines = 0;
	while (true) {
		int st = begin_token(&pcode,cp,pln,srclin,tk,&ntk,outbuff);
		if (!st) {
			/* exit loop */
			break;
		}
		string mline = srclin;
		str_trim_left(mline);
		if (text_block.length() == 0) {
			text_block = mline;
		} else {
			text_block += "\7";
			text_block += mline;
		}
		nblines++;
	}
	GLERectangle box;
	decode_utf8_basic(text_block);
	TeXInterface::getInstance()->draw(text_block.c_str(), nblines, &box);
	// Name object
	if (!name.isNull() && name->length() != 0) {
		double x1, x2, y1, y2;
		box.getDimensions(&x1,&y1,&x2,&y2);
		x1 -= add; x2 += add; y1 -= add; y2 += add;
		run->name_set(name.get(),x1,y1,x2,y2);
	}
}
