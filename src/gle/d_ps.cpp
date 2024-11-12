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

/*--------------------------------------------------------------*/
/*  Post Script Driver, for GLE                                 */
/*--------------------------------------------------------------*/

#include "all.h"
#include "file_io.h"
#include "bitmap/img2ps.h"
#include "core.h"
#include "d_interface.h"
#include "gprint.h"
#include "cutils.h"
#include "op_def.h"
#include "cmdline.h"
#include "config.h"
#include "mem_limits.h"
#ifdef _WIN32
#include <time.h>
#endif

using namespace std;

//
// -- constants
//
#define dbg if ((gle_debug & 64)>0)

//
// ellipse function only for PS
//
static char ellipse_fcn[] = "\
/ellipsedict 8 dict def\n ellipsedict /mtrx matrix put \n\
/ellipse\n\
        { ellipsedict begin\n\
          /endangle exch def\n\
          /startangle exch def\n\
          /yrad exch def\n\
          /xrad exch def\n\
          /y exch def\n\
          /x exch def\n\
          \n\
          /savematrix mtrx currentmatrix def\n\
          x y translate\n\
          xrad yrad scale\n\
          0 0 1 startangle endangle arc\n\
          savematrix setmatrix\n\
          end\n\
        } def\n\
/ellipsen\n\
        { ellipsedict begin\n\
		/endangle exch def\n\
		  /startangle exch def\n\
          /yrad exch def\n\
          /xrad exch def\n\
          /y exch def\n\
          /x exch def\n\
          \n\
          /savematrix mtrx currentmatrix def\n\
          x y translate\n\
          xrad yrad scale\n\
          0 0 1 startangle endangle arcn\n\
          savematrix setmatrix\n\
          end\n\
        } def\n\
";
//
// -- global variables can we get rid of these?
//
extern bool control_d;
extern struct gmodel g;
extern bool GS_PREVIEW;
extern int gle_debug;
extern ConfigCollection g_Config;

int MAX_VECTOR = MAXIMUM_PS_VECTOR; /* Can't send POSTSCRIPT too complex a path */

int setMaxPSVector(int newMax) {
	int oldValue = MAX_VECTOR;
	MAX_VECTOR = newMax;
	return oldValue;
}

//
// -- function prototypes
//
char *font_getname(int i);
void AddExtension(string& fname, const string& ext);

void d_tidyup() {
}

PSGLEDevice::PSGLEDevice(bool eps) : GLEDevice() {
	m_IsEps = eps;
	m_IsPageSize = false;
	first_ellipse = 1;
	ps_nvec = 0;
	m_Out = NULL;
	m_OutputFile = NULL;
	m_OutputBuffer = NULL;
	m_FillMethod = GLE_FILL_METHOD_DEFAULT;
	m_currentFill = g_get_fill_clear();
}

PSGLEDevice::~PSGLEDevice() {
}

void PSGLEDevice::dfont(char *c) {
	/* only used for the DFONT driver which builds fonts */
}

void PSGLEDevice::message(char *s) {
	printf("%s\n",s);
}

void PSGLEDevice::devcmd(const char *s) {
	out() << s;
}

void PSGLEDevice::source(const char *s) {
	dbg out() << "%% SOURCE, " << s;
}

std::string PSGLEDevice::get_type() {
	std::vector<std::string> temp(g_create_device_string());
	temp.push_back("FILLPATH");
	temp.push_back("POSTSCRIPT");
	return str_join(temp);
}

void PSGLEDevice::set_path(int onoff) {
}

void PSGLEDevice::newpath() {
	out() << " newpath ";
	ps_nvec = 0;
}

void PSGLEDevice::pscomment(char* ss) {
	comments.push_back(ss);
}

FILE* PSGLEDevice::get_file_pointer(void) {
	return psfile;
}

void PSGLEDevice::opendev(double width, double height, GLEFileLocation* outputfile, const string& inputfile) {
	first_ellipse = 1;
	m_OutputName.copy(outputfile);
	m_OutputName.addExtension(g_device_to_ext(getDeviceType()));
#ifdef ENABLE_GS_PREVIEW
	setRecordingEnabled(true);
#endif
	if (isRecordingEnabled()) {
		startRecording();
	} else {
		m_Out = m_OutputFile = new ofstream(m_OutputName.getFullPath().c_str(), ios::out | ios::binary);
		if (!m_OutputFile->is_open()) {
			g_throw_parser_error("failed to create PostScript file '", m_OutputName.getFullPath().c_str(), "'");
		}
	}
	if (!isEps()) {
		if (control_d) out() << (char)4 << endl;
		out() << "%!PS-Adobe-2.0" << endl;
	} else {
		out() << "%!PS-Adobe-2.0 EPSF-2.0" << endl;
	}
	time_t t;
	t = time(0);
	// Make sure to use proper Adobe DSC comments
	// Windows Explorer can show these in the document properties of a PDF created by GLE
	string vers_nosnap = g_get_version_nosnapshot();
	out() << "%%Creator: GLE " << vers_nosnap << " <glx.sourceforge.net>" << endl;
	out() << "%%CreationDate: " << ctime(&t);
	out() << "%%Title: " << inputfile << endl;
	for (vector<string>::size_type i = 0; i < comments.size(); i++) {
		out() << "%% " << comments[i] << endl;
	}
	comments.clear();
	int int_bb_x = 0, int_bb_y = 0;
	computeBoundingBox(width, height, &int_bb_x, &int_bb_y);
	out() << "%%BoundingBox: 0 0 " << int_bb_x << " " << int_bb_y << endl;
	out() << "%%HiResBoundingBox: 0 0 " << m_boundingBox.getX() << " " << m_boundingBox.getY() << endl;
	out() << "%%EndComments" << endl;
	out() << "%%EndProlog" << endl;
	if (isOutputPageSize()) {
		// Currently not used
		out() << "<< /PageSize [" << int_bb_x << " " << int_bb_y << "] >> setpagedevice" << endl;
	}
	initialPS();
}

void PSGLEDevice::initialPS() {
	out() << "gsave" << endl;
	out() << "/f {findfont exch scalefont setfont} bind def" << endl;
	out() << "/s {show} bind def" << endl;
	out() << "/ps {true charpath} bind def" << endl;
	out() << "/l {lineto} bind def" << endl;
	out() << "/m {newpath moveto} bind def" << endl;
	out() << "matrix currentmatrix /originmat exch def" << endl;
	out() << "/umatrix {originmat matrix concatmatrix setmatrix} def" << endl;
	// Measure distance in cm
	g_scale(PS_POINTS_PER_INCH/CM_PER_INCH, PS_POINTS_PER_INCH/CM_PER_INCH);
	if (!g_is_fullpage()) {
		// Bounding box tweak
		g_translate(1.0*CM_PER_INCH/72, 1.0*CM_PER_INCH/72);
	}
}

void PSGLEDevice::closedev() {
	g_flush();
	out() << "showpage" << endl;
	out() << "grestore" << endl;
	out() << "%%Trailer" << endl;
	if (!isEps() && control_d) out() << (char)4 << endl;
	#ifdef ENABLE_GS_PREVIEW
		// GS_PREVIEW a.r. 2005
		// - purpose: *FAST* previewing while writing GLE code,
		//   don't write huge eps files to disk, no hassle with resizing ghostview,
		//   gsview or gv windows.
		// - add to .Xresources: Ghostscript*geometry:   -5+0
		//   to place gs window in upper right corner like  gle -d x11
		// - may use environment variable or .glerc file for additional gs options
		//   based on $DISPLAY, if you work on multiple systems
		if (GS_PREVIEW) {
			ostringstream GScmd;
			double width, height;
			int gsPixelWidth, gsPixelHeight, gsPixelRes;
			g_get_pagesize(&width, &height);
			displayGeometry(width, height, &gsPixelWidth, &gsPixelHeight, &gsPixelRes);
			ConfigSection* tools = g_Config.getSection(GLE_CONFIG_TOOLS);
			string gs_cmd = ((CmdLineArgString*)tools->getOptionValue(GLE_TOOL_GHOSTSCRIPT_CMD))->getValue();
			str_try_add_quote(gs_cmd);
			GScmd << gs_cmd;
			GScmd << " -sDEVICE=x11 -dTextAlphaBits=4 -dGraphicsAlphaBits=2 -dMaxBitmap=5000000 ";
			GScmd << "-dNOPLATFONTS -dTTYPAUSE -g" << gsPixelWidth << "x" << gsPixelHeight << " ";
			GScmd << "-r" << gsPixelRes << "x" << gsPixelRes << " -dDELAYSAFER ";
			GScmd << "-c '<< /PermitFileReading [ (/dev/tty)] >> setuserparams .locksafe' -dSAFER -q -_";
			// cerr << endl << "Calling " << GScmd.str() << endl;
			// NOTE: popen may not exist (or may not work as expected) on Windows and OS/2 !
			FILE* psfile = popen(GScmd.str().c_str(), "w");
			if (psfile == NULL) {
				cerr << "GLE PS: popen ghostscript failed: " << GScmd.str() << endl;
				exit(1);
			}
			fprintf(psfile, "%s\n", m_OutputBuffer->str().c_str());
			pclose(psfile);
		}
	#endif
	if (m_OutputFile != NULL) {
		m_OutputFile->close();
		delete m_OutputFile;
		m_OutputFile = NULL;
	}
	if (g_verbosity() > 0) {
		string mainname;
		if (isEps()) {
			GetMainNameExt(m_OutputName.getName(), ".eps", mainname);
			cerr << "[" << mainname << "][.eps]";
		} else {
			GetMainNameExt(m_OutputName.getName(), ".ps", mainname);
			cerr << "[" << mainname << "][.ps]";
		}
		g_set_console_output(false);
	}
}

void PSGLEDevice::set_line_cap(int i) {
	/*  lcap, 0= butt, 1=round, 2=projecting square */
	if (!g.inpath) g_flush();
	out() << i << " setlinecap" << endl;
}

void PSGLEDevice::set_line_join(int i) {
	if (!g.inpath) g_flush();
	out() << i << " setlinejoin" << endl;
}

void PSGLEDevice::set_line_miterlimit(double d) {
	if (!g.inpath) g_flush();
	out() << d << " setmiterlimit" << endl;
}

void PSGLEDevice::set_line_width(double w) {
	if (w==0) w = 0.02;
	if (w<.0002) w = 0;
	if (!g.inpath) g_flush();
	out() << w << " setlinewidth" << endl;
}

void PSGLEDevice::set_line_styled(double dd) {
}

void PSGLEDevice::set_line_style(const char *s) {
	/* should deal with [] for solid lines */
	static const char *defline[] = {"","","12","41","14","92","1282","9229","4114","54","73","7337","6261","2514"};
	char ob[200];
	if (!g.inpath) g_flush();
	strcpy(ob,"[");
	if (strlen(s) == 1) {
		int index = s[0] - '0';
		if (index < 0 || index > (int)(sizeof(defline)/sizeof(const char*))) {
			ostringstream err;
			err << "illegal line style '" << s << "'";
			g_throw_parser_error(err.str());
		}
		s = defline[index];
	}
	int l = strlen(s);
	for (i=0;i<l;i++) {
		sprintf(ob+strlen(ob),"%g ",(*(s+i)-'0')*g.lstyled);
	}
	strcat(ob,"]");
	out() << ob << " 0 setdash" << endl;
}

void PSGLEDevice::fill() {
	out() << "gsave" << endl;
	ddfill();
	out() << "grestore" << endl;
}

void PSGLEDevice::ddfill(GLERectangle* bounds) {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	if (cur_fill.b[B_F] == 255) {
		return; /* clear fill, do nothing */
	} else if (cur_fill.b[B_F] == 2) {
		shade(bounds);
		return;
	}
	set_fill();			/*because color and fill are the same*/
	out() << "fill" << endl;
	set_color();
}

void PSGLEDevice::shadeGLE() {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	double step1 = cur_fill.b[B_B]/160.0;
	double step2 = cur_fill.b[B_G]/160.0;
	if (step1 > 0) {
		out() << -40.0 << " " << step1 << " " << 40.0 << " { /x exch def" << endl;
		out() << "x 0 moveto 40 x add 40 lineto stroke" << endl;
		out() << "} for" << endl;
	}
	if (step2 > 0) {
		out() << 0.0 << " " << step2 << " " << 80.0 << " { /x exch def" << endl;
		out() << "x 0 moveto -40 x add 40 lineto stroke" << endl;
		out() << "} for" << endl;
	}
}

void PSGLEDevice::shadeBoundedIfThenElse1(GLERectangle* bounds, double step1) {
	// if x1+p*s > y1 then
	out() <<  bounds->getXMax() << " p " << step1 << " mul add " << bounds->getYMax() <<  " gt" << endl;
	// aline y1-p*s y1
	out() << "{" << bounds->getYMax() << " dup p " << step1 << " mul sub exch lineto stroke}" << endl;
	// aline x1 x1+p*s
	out() << "{" << bounds->getXMax() << " dup p " << step1 << " mul add lineto stroke} ifelse" << endl;
}

void PSGLEDevice::shadeBoundedIfThenElse2(GLERectangle* bounds, double step2) {
	// if p*s-y1 > x0 then
	out() <<  "p " << step2 << " mul " << bounds->getYMax() << " sub " << bounds->getXMin() <<  " gt" << endl;
	// aline p*s-y1 y1
	out() << "{" << bounds->getYMax() << " dup p " << step2 << " mul exch sub exch lineto stroke}" << endl;
	// aline x0 p*s-x0
	out() << "{" << bounds->getXMin() << " dup p " << step2 << " mul exch sub lineto stroke} ifelse" << endl;
}

void PSGLEDevice::shadeBounded(GLERectangle* bounds) {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	double step1 = cur_fill.b[B_B]/160.0;
	double step2 = cur_fill.b[B_G]/160.0;
	out() << "2 setlinecap" << endl;
	if (step1 > 0) {
		int p0 = (int)ceil((bounds->getYMax()-bounds->getXMin())/step1-1e-6);
		if (bounds->getXMin() + p0*step1 > bounds->getYMax()) p0--;
		int p1 = (int)floor((bounds->getYMin()-bounds->getXMin())/step1+1e-6);
		if (bounds->getXMin() + p1*step1 < bounds->getYMin()) p1++;
		int p2 = (int)floor((bounds->getYMin()-bounds->getXMax())/step1+1e-6);
		if (bounds->getXMax() + p2*step1 < bounds->getYMin()) p2++;
		// for (int p = p0; p > p1; p--)
		out() << p0 << " -1 " << (p1+1) << " { /p exch def" << endl;
		// amove x0 x0+p*s
		out() << bounds->getXMin() << " dup p " << step1 << " mul add moveto" << endl;
		shadeBoundedIfThenElse1(bounds, step1);
		out() << "} for" << endl;
		// for (int p = p1; p >= p2; p--)
		out() << p1 << " -1 " << p2 << " { /p exch def" << endl;
		// amove y0-p*s y0
		out() << bounds->getYMin() << " dup p " << step1 << " mul sub exch moveto" << endl;
		shadeBoundedIfThenElse1(bounds, step1);
		out() << "} for" << endl;
	}
	if (step2 > 0) {
		int p0 = (int)ceil((bounds->getYMax()+bounds->getXMax())/step2-1e-6);
		if (p0*step2 - bounds->getXMin() > bounds->getYMax()) p0--;
		int p1 = (int)floor((bounds->getYMin()+bounds->getXMax())/step2+1e-6);
		if (p1*step2 - bounds->getXMax() < bounds->getYMin()) p1++;
		int p2 = (int)floor((bounds->getYMin()+bounds->getXMin())/step2+1e-6);
		if (p2*step2 - bounds->getXMax() < bounds->getYMin()) p2++;
		// for (int p = p0; p > p1; p--)
		out() << p0 << " -1 " << (p1+1) << " { /p exch def" << endl;
		// amove x0 x0+p*s
		out() << bounds->getXMax() << " dup p " << step2 << " mul exch sub moveto" << endl;
		shadeBoundedIfThenElse2(bounds, step2);
		out() << "} for" << endl;
		// for (int p = p1; p >= p2; p--)
		out() << p1 << " -1 " << p2 << " { /p exch def" << endl;
		// amove y0-p*s y0
		out() << bounds->getYMin() << " dup p " << step2 << " mul exch sub exch moveto" << endl;
		shadeBoundedIfThenElse2(bounds, step2);
		out() << "} for" << endl;
	}
}

void PSGLEDevice::shadePostScript() {
	colortyp cur_fill;
	cur_fill.l = m_currentFill->getHexValueGLE();
	int step1 = cur_fill.b[B_B];
	int step2 = cur_fill.b[B_G];
	out() << "<< /PatternType 1" << endl;
	out() << "/PaintType 1" << endl;
	out() << "/TilingType 1" << endl;
	int xstep = max(step1, step2);
	int ystep = max(step1, step2);
	out() << "/BBox [0 0 " << xstep << " " << ystep << "]" << endl;
	out() << "/XStep " << xstep << endl;
	out() << "/YStep " << ystep << endl;
	out() << "/PaintProc" << endl;
	out() << "{ pop" << endl;
	out() << "0 setlinecap" << endl;
	out() << "0 setlinejoin" << endl;
	GLERC<GLEColor> background(get_fill_background(m_currentFill.get()));
	if (!background->isTransparent()) {
		set_color_impl(background);
		out() << "-1 -1 " << (xstep+1) << " " << (ystep+1) << " rectfill" << endl;
	}
	GLERC<GLEColor> foreground(get_fill_foreground(m_currentFill.get()));
	set_color_impl(foreground);
	out() << (int)cur_fill.b[B_R] << " setlinewidth" << endl;
	if (step1 > 0) {
		out() << "0 0 moveto" << endl;
		out() << xstep << " " << ystep << " l" << endl;
		out() << "stroke" << endl;
		if (step2 == 0) {
			out() << xstep/2 << " " << -ystep/2 << " moveto" << endl;
			out() << 3*xstep/2 << " " << ystep/2 << " l" << endl;
			out() << "stroke" << endl;
			out() << -xstep/2 << " " << ystep/2 << " moveto" << endl;
			out() << xstep/2 << " " << 3*ystep/2 << " l" << endl;
			out() << "stroke" << endl;
		}
	}
	if (step2 > 0) {
		out() << "0 " << ystep << " moveto" << endl;
		out() << xstep << " 0 l" << endl;
		out() << "stroke" << endl;
		if (step1 == 0) {
			out() << -xstep/2 << " " << ystep/2 << " moveto" << endl;
			out() << xstep/2 << " " << -ystep/2 << " l" << endl;
			out() << "stroke" << endl;
			out() << xstep/2 << " " << 3*ystep/2 << " moveto" << endl;
			out() << 3*xstep/2 << " " << ystep/2 << " l" << endl;
			out() << "stroke" << endl;
		}
	}
	out() << "} bind" << endl;
	out() << ">>" << endl;
	out() << "[" << 1.0/160 << " 0 0 " << 1.0/160 << " 1 1]" << endl;
	out() << "makepattern" << endl;
	out() << "/Pattern setcolorspace" << endl;
	out() << "setpattern fill" << endl;
	set_color();
}

void PSGLEDevice::shade(GLERectangle* bounds) {
	if (m_FillMethod == GLE_FILL_METHOD_GLE ||
	   (m_FillMethod == GLE_FILL_METHOD_DEFAULT && bounds != NULL)) {
		GLERC<GLEColor> background(get_fill_background(m_currentFill.get()));
		if (!background->isTransparent()) {
			out() << "gsave" << endl;
			set_color_impl(background);
			out() << "fill" << endl;
			out() << "grestore" << endl;
		}
		// Implemented by using path as clip and then painting strokes over the clip
		out() << "gsave" << endl;
		out() << "clip" << endl;
		out() << "newpath" << endl;
		GLERC<GLEColor> foreground(get_fill_foreground(m_currentFill.get()));
		set_color_impl(foreground);
		colortyp cur_fill;
		cur_fill.l = m_currentFill->getHexValueGLE();
		out() << (double)(cur_fill.b[B_R]/160.0) << " setlinewidth" << endl;
		if (m_FillMethod == GLE_FILL_METHOD_DEFAULT && bounds != NULL) {
			shadeBounded(bounds);
		} else {
			shadeGLE();
		}
		out() << "grestore" << endl;
	} else {
		shadePostScript();
	}
}

void PSGLEDevice::fill_ary(int nwk,double *wkx,double *wky) {
	out() << "gsave" << endl;
	out() << "newpath" << endl;
	out() << wkx[0] << " " << wky[0] << " moveto" << endl;
	for (int i = 1; i < nwk; i++) {
		out() << wkx[i] << " " << wky[i] << " l" << endl;
	}
	set_fill();
	out() << "fill" << endl;
	set_color();
	out() << "grestore" << endl;
}

void PSGLEDevice::line_ary(int nwk,double *wkx,double *wky) {
	out() << "gsave" << endl;
	out() << "newpath" << endl;
	out() << wkx[0] << " " << wky[0] << " moveto" << endl;
	for (int i = 1; i < nwk; i++) {
		out() << wkx[i] << " " << wky[i] << " l" << endl;
	}
	out() << "stroke" << endl;
	out() << "grestore" << endl;
}

void PSGLEDevice::stroke() {
	out() << "gsave" << endl;
	out() << "stroke" << endl;
	out() << "grestore" << endl;
}

void PSGLEDevice::clip() {
	out() << "clip" << endl;
}

void PSGLEDevice::set_matrix(double newmat[3][3]) {
	out() << "[";
	out() << newmat[0][0] << " " << newmat[1][0] << " " << newmat[0][1] << " ";
	out() << newmat[1][1] << " " << newmat[0][2] << " " << newmat[1][2] << "] umatrix" << endl;
}

void PSGLEDevice::move(double zx,double zy) {
	if (g.inpath) {
		out() << zx << " " << zy << " moveto" << endl;
	} else {
		ps_nvec++;
		out() << zx << " " << zy << " m" << endl;
	}
}

void PSGLEDevice::reverse() {
	out() << "reversepath" << endl;
}

void PSGLEDevice::closepath() {
	out() << "closepath" << endl;
}

void PSGLEDevice::line(double zx,double zy) {
	dbg gprint("in d_line  g.curx,y  %g %g ",g.curx,g.cury);
	if (g.xinline==false) {
		move(g.curx,g.cury);
	}
	ps_nvec++;
	if (MAX_VECTOR != -1 && ps_nvec > MAX_VECTOR) {
		//gprint("Warning, complex path, if filling fails then try /nomaxpath \n");
		ps_nvec = 0; g_flush(); move(g.curx,g.cury);
	}
	out() << zx << " " << zy << " l" << endl;
}

void PSGLEDevice::clear() {
}

void PSGLEDevice::flush() {
	if (g.inpath) return;
	if (g.xinline) {
		out() << "stroke" << endl;
		ps_nvec = 0;
	}
}

void PSGLEDevice::arcto(dbl x1,dbl y1,dbl x2,dbl y2,dbl rrr) {
	if (g.xinline==false) move(g.curx,g.cury);
	out() << x1 << " " << y1 << " " << x2 << " " << y2 << " " << rrr << " arcto clear ";
	out() << x2 << " " << y2 << " l" << endl;
	g.xinline = true;
}

void PSGLEDevice::arc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
	//double dx,dy;
	double x,y;
	g_get_xy(&x,&y);
	//polar_xy(r,t1,&dx,&dy);
	if (!g.inpath) {
		if (!g.xinline) out() << "newpath ";
	}
	out() << cx << " " << cy << " " << r << " " << t1 << " " << t2 << " arc" << endl;
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void PSGLEDevice::narc(dbl r,dbl t1,dbl t2,dbl cx,dbl cy) {
	double dx,dy;
	double x,y;
	g_get_xy(&x,&y);
	polar_xy(r,t1,&dx,&dy);
	if (!g.inpath) {
		if (!g.xinline) out() << "newpath ";
	}
	out() << cx << " " << cy << " " << r << " " << t1 << " " << t2 << " arcn" << endl;
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void PSGLEDevice::elliptical_arc(dbl rx,dbl ry,dbl t1,dbl t2,dbl cx,dbl cy) {
	double dx,dy;
	double x,y;
	if (first_ellipse) {
		first_ellipse = 0;
		out() << ellipse_fcn << endl;
	}
	g_get_xy(&x,&y);
	polar_xy(rx,ry,t1,&dx,&dy);
	if (!g.inpath) g_move(cx+dx,cy+dy);
	out() << cx << " " << cy << " " << rx << " " << ry << " " << t1 << " " << t2 << " ellipse" << endl;
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void PSGLEDevice::elliptical_narc(dbl rx,dbl ry,dbl t1,dbl t2,dbl cx,dbl cy) {
	double dx,dy;
	double x,y;
	if(first_ellipse) {
		first_ellipse = 0;
		out() << ellipse_fcn << endl;
	}
	g_get_xy(&x,&y);
	polar_xy(rx,ry,t1,&dx,&dy);
	if (!g.inpath) g_move(cx+dx,cy+dy);
	out() << cx << " " << cy << " " << rx << " " << ry << " " << t1 << " " << t2 << " ellipsen" << endl;
	g.xinline = true;
	if (!g.inpath) g_move(x,y);
}

void PSGLEDevice::box_fill(dbl x1, dbl y1, dbl x2, dbl y2) {
	if (g.inpath) {
		xdbox(x1,y1,x2,y2);
	} else {
		g_flush();
		out() << "newpath ";
		GLERectangle rect(x1, y1, x2, y2);
		xdbox(x1,y1,x2,y2);
		ddfill(&rect);
		out() << "newpath" << endl;
	}
}

void PSGLEDevice::box_stroke(dbl x1, dbl y1, dbl x2, dbl y2, bool reverse) {
	if (g.inpath) {
		if (reverse) {
			out() << x1 << " " << y1 << " moveto " << x1 << " " << y2 << " l " <<
			         x2 << " " << y2 << " l " << x2 << " " << y1 << " l closepath" << endl;
		} else {
			xdbox(x1,y1,x2,y2);
		}
	} else {
		g_flush();
		out() << "newpath ";
		xdbox(x1,y1,x2,y2);
		out() << "stroke" << endl;
		ps_nvec = 0;
	}
}

void PSGLEDevice::xdbox(double x1, double y1, double x2, double y2) {
	out() << x1 << " " << y1 << " moveto " << x2 << " " << y1 << " l " << x2 << " " <<
	         y2 << " l " << x1 << " " << y2 << " l closepath" << endl;
}

void PSGLEDevice::circle_stroke(double zr) {
	double x,y;
	g_get_xy(&x,&y);
	if (g.inpath) {
		out() << x << " " << y << " " << zr << " 0 360 arc" << endl;
	} else {
		g_flush();
		out() << "newpath ";
		out() << x << " " << y << " " << zr << " 0 360 arc" << endl;
		out() << "closepath stroke" << endl;
	}
}

void PSGLEDevice::circle_fill(double zr) {
	double x=g.curx,y=g.cury;
	if (g.inpath) {
		out() << x << " " << y << " " << zr << " 0 360 arc" << endl;
	} else {
		g_flush();
		out() << "newpath ";
		out() << x << " " << y << " " << zr << " 0 360 arc" << endl;
		GLERectangle rect(x-zr, y-zr, x+zr, y+zr);
		ddfill(&rect);
		out() << "newpath" << endl;
	}
}

void PSGLEDevice::ellipse_stroke(double rx, double ry) {
	double x,y;
	if (first_ellipse) {
		first_ellipse = 0;
		out() << ellipse_fcn << endl;
	}
	g_get_xy(&x,&y);
	if (g.inpath) {
		out() << x << " " << y << " " << rx << " " << ry << " 0 360 ellipse" << endl;
	} else {
		g_flush();
		out() << "newpath ";
		out() << x << " " << y << " " << rx << " " << ry << " 0 360 ellipse closepath" << endl;
		out() << "closepath stroke" << endl;
	}
}

void PSGLEDevice::ellipse_fill(double rx, double ry) {
	double x=g.curx,y=g.cury;
	if (first_ellipse) {
		first_ellipse = 0;
		out() << ellipse_fcn << endl;
	}
	if (g.inpath) {
		out() << x << " " << y << " " << rx << " " << ry << " 0 360 ellipse" << endl;
	} else {
		g_flush();
		out() << "newpath ";
		out() << x << " " << y << " " << rx << " " << ry << " 0 360 ellipse" << endl;
		GLERectangle rect(x-rx, y-ry, x+rx, y+ry);
		ddfill(&rect);
		out() << "newpath" << endl;
	}
}

void PSGLEDevice::bezier(dbl x1,dbl y1,dbl x2,dbl y2,dbl x3,dbl y3) {
	double x=g.curx,y=g.cury;
	if (g.inpath) {
		if (g.xinline==false) move(g.curx,g.cury);
		out() << x1 << " " << y1 << " " << x2 << " " << y2 << " "
		      << x3 << " " << y3 << " curveto" << endl;
	} else {
		g_flush();
		if (!g.xinline) out() << x << " " << y << " moveto ";
		out() << x1 << " " << y1 << " " << x2 << " " << y2 << " "
		      << x3 << " " << y3 << " curveto" << endl;
	}
	g.xinline = true;
}

void PSGLEDevice::set_color_impl(const GLERC<GLEColor>& color) {
	unsigned int hexValue = color->getHexValueGLE();
	if (hexValue == (unsigned int)GLE_COLOR_WHITE) {
		out() << "1 setgray" << endl;
	} else if (hexValue == (unsigned int)GLE_COLOR_BLACK) {
		out() << "0 setgray" << endl;
	} else {
		if (color->getRed() == color->getGreen() && color->getRed() == color->getBlue()) {
			out() << color->getRed() << " setgray" << endl;
		} else {
			out() << color->getRed() << " " << color->getGreen() << " " << color->getBlue() << " setrgbcolor" << endl;
		}
	}
	if (color->hasAlpha()) {
		g_throw_parser_error("semi-transparency only supported with command line option '-cairo'");
	}
}

void PSGLEDevice::set_color() {
	set_color_impl(m_currentColor);
}

void PSGLEDevice::set_fill() {
	set_color_impl(m_currentFill);
}

void PSGLEDevice::set_fill_method(int m) {
	m_FillMethod = m;
}

void PSGLEDevice::set_color(const GLERC<GLEColor>& color) {
	g_flush();
	m_currentColor = color;
	set_color();
}

void PSGLEDevice::set_fill(const GLERC<GLEColor>& fill) {
	m_currentFill = fill;
}

void PSGLEDevice::beginclip() {
	out() << "gsave" << endl;
}

void PSGLEDevice::endclip() {
	g_flush();
	out() << "grestore" << endl;
	gmodel* state = new gmodel();
	g_get_state(state);
	g_set_state(state);
	delete state;
}

struct psfont_struct {char *sname; char *lname;};
struct psfont_struct psf[70] = { /* leaves room for twenty more from PSFONT.DAT*/
	{ (char*)"PSTR",       (char*)"Times-Roman"                    },
	{ (char*)"PSTI",       (char*)"Times-Italic"                   },
	{ (char*)"PSTB",       (char*)"Times-Bold"                     },
	{ (char*)"PSTBI",      (char*)"Times-BoldItalic"               },
	{ (char*)"RM",         (char*)"Times-Roman"                    },
	{ (char*)"RMI",        (char*)"Times-Italic"                   },
	{ (char*)"RMB",        (char*)"Times-Bold"                     },
	{ (char*)"RMBI",       (char*)"Times-BoldItalic"               },
	{ (char*)"SS",         (char*)"Helvetica"                      },
	{ (char*)"SSB",        (char*)"Helvetica-Bold"                 },
	{ (char*)"SSI",        (char*)"Helvetica-Oblique"              },
	{ (char*)"SSBI",       (char*)"Helvetica-BoldOblique"          },
	{ (char*)"PSH",        (char*)"Helvetica"                      },
	{ (char*)"PSHB",       (char*)"Helvetica-Bold"                 },
	{ (char*)"PSHBO",      (char*)"Helvetica-BoldOblique"          },
	{ (char*)"PSHO",       (char*)"Helvetica-Oblique"              },
	{ (char*)"PSAGB",      (char*)"AvantGarde-Book"                },
	{ (char*)"PSAGBO",     (char*)"AvantGarde-BookOblique"         },
	{ (char*)"PSAGD",      (char*)"AvantGarde-Demi"                },
	{ (char*)"PSAGDO",     (char*)"AvantGarde-DemiOblique"         },
	{ (char*)"PSBD",       (char*)"Bookman-Demi"                   },
	{ (char*)"PSBDI",      (char*)"Bookman-DemiItalic"             },
	{ (char*)"PSBL",       (char*)"Bookman-Light"                  },
	{ (char*)"PSBLI",      (char*)"Bookman-LightItalic"            },
	{ (char*)"PSC",        (char*)"Courier"                        },
	{ (char*)"PSCB",       (char*)"Courier-Bold"                   },
	{ (char*)"PSCBO",      (char*)"Courier-BoldOblique"            },
	{ (char*)"PSCO",       (char*)"Courier-Oblique"                },
	{ (char*)"TT",         (char*)"Courier"                        },
	{ (char*)"TTB",        (char*)"Courier-Bold"                   },
	{ (char*)"TTBI",       (char*)"Courier-BoldOblique"            },
	{ (char*)"TTI",        (char*)"Courier-Oblique"                },
	{ (char*)"PSNCSB",     (char*)"NewCenturySchlbk-Bold"          },
	{ (char*)"PSNCSBI",    (char*)"NewCenturySchlbk-BoldItalic"    },
	{ (char*)"PSNCSI",     (char*)"NewCenturySchlbk-Italic"        },
	{ (char*)"PSNCSR",     (char*)"NewCenturySchlbk-Roman"         },
	{ (char*)"PSPB",       (char*)"Palatino-Bold"                  },
	{ (char*)"PSPBI",      (char*)"Palatino-BoldItalic"            },
	{ (char*)"PSPI",       (char*)"Palatino-Italic"                },
	{ (char*)"PSPR",       (char*)"Palatino-Roman"                 },
	{ (char*)"PSZCMI",     (char*)"ZapfChancery-MediumItalic"      },
	{ (char*)"PSZD",       (char*)"ZapfDingbats"                   },
	{ (char*)"PSSYM",      (char*)"Symbol"                         },
	{ NULL,         NULL                                           }
};

static int d_ps_this_font = 0;
static double d_ps_this_size = 0;

struct ps_glyph_name { const char* name; };
struct ps_glyph_name ps_glyph_name_map[] = {
	{ "Aacute" }, { "Abreve" }, { "Acircumflex" }, { "Adieresis" }, { "Agrave" }, { "Amacron" },
	{ "Aogonek" }, { "Aring" }, { "Atilde" }, { "Cacute" }, { "Ccaron" }, { "Ccedilla" },
	{ "Dcaron" }, { "Dcroat" }, { "Delta" }, { "Eacute" }, { "Ecaron" }, { "Ecircumflex" },
	{ "Edieresis" }, { "Edotaccent" }, { "Egrave" }, { "Emacron" }, { "Eogonek" }, { "Eth" },
	{ "Gbreve" }, { "Gcommaaccent" }, { "Iacute" }, { "Icircumflex" }, { "Idieresis" },
	{ "Idotaccent" }, { "Igrave" }, { "Imacron" }, { "Iogonek" }, { "Kcommaaccent" },
	{ "Lacute" }, { "Lcaron" }, { "Lcommaaccent" }, { "Nacute" }, { "Ncaron" }, { "Ncommaaccent" },
	{ "Ntilde" }, { "Oacute" }, { "Ocircumflex" }, { "Odieresis" }, { "Ograve" }, { "Ohungarumlaut" },
	{ "Omacron" }, { "Otilde" }, { "Racute" }, { "Rcaron" }, { "Rcommaaccent" }, { "Sacute" },
	{ "Scaron" }, { "Scedilla" }, { "Scommaaccent" }, { "Tcaron" }, { "Tcommaaccent" }, { "Thorn" },
	{ "Uacute" }, { "Ucircumflex" }, { "Udieresis" }, { "Ugrave" }, { "Uhungarumlaut" },
	{ "Umacron" }, { "Uogonek" }, { "Uring" }, { "Yacute" }, { "Ydieresis" }, { "Zacute" },
	{ "Zcaron" }, { "Zdotaccent" }, { "aacute" }, { "abreve" }, { "acircumflex" }, { "adieresis" },
	{ "agrave" }, { "amacron" }, { "aogonek" }, { "aring" }, { "atilde" }, { "brokenbar" },
	{ "cacute" }, { "ccaron" }, { "ccedilla" }, { "commaaccent" }, { "copyright" }, { "dcaron" },
	{ "dcroat" }, { "degree" }, { "divide" }, { "eacute" }, { "ecaron" }, { "ecircumflex" },
	{ "edieresis" }, { "edotaccent" }, { "egrave" }, { "emacron" }, { "eogonek" }, { "eth" },
	{ "gbreve" }, { "gcommaaccent" }, { "greaterequal" }, { "iacute" }, { "icircumflex" },
	{ "idieresis" }, { "igrave" }, { "imacron" }, { "iogonek" }, { "kcommaaccent" }, { "lacute" },
	{ "lcaron" }, { "lcommaaccent" }, { "lessequal" }, { "logicalnot" }, { "lozenge" }, { "minus" },
	{ "mu" }, { "multiply" }, { "nacute" }, { "ncaron" }, { "ncommaaccent" }, { "notequal" },
	{ "ntilde" }, { "oacute" }, { "ocircumflex" }, { "odieresis" }, { "ograve" }, { "ohungarumlaut" },
	{ "omacron" }, { "onehalf" }, { "onequarter" }, { "onesuperior" }, { "otilde" }, { "partialdiff" },
	{ "plusminus" }, { "racute" }, { "radical" }, { "rcaron" }, { "rcommaaccent" }, { "registered" },
	{ "sacute" }, { "scaron" }, { "scedilla" }, { "scommaaccent" }, { "summation" }, { "tcaron" },
	{ "tcommaaccent" }, { "thorn" }, { "threequarters" }, { "threesuperior" }, { "trademark" },
	{ "twosuperior" }, { "uacute" }, { "ucircumflex" }, { "udieresis" }, { "ugrave" }, { "uhungarumlaut" },
	{ "umacron" }, { "uogonek" }, { "uring" }, { "yacute" }, { "ydieresis" }, { "zacute" }, { "zcaron" },
	{ "zdotaccent" }
};


void PSGLEDevice::dochar(int font, int cc) {
	read_psfont();
	if (font_get_encoding(font)>2) {
		my_char(font,cc);
		return;
	}
	if (d_ps_this_font!=font || d_ps_this_size!=g.fontsz) {
		if (g.fontsz<0.00001) {
			gprint("Font size is zero, error ********* \n");
			return;
		}
		char *s = font_getname(font);
		for (i=0;;i++) {
			if (psf[i].sname==NULL) break;
			dbg printf("font match  {%s} {%s} \n",s,psf[i].sname);
			if (str_i_equals(psf[i].sname,s)) break;
		}
		if (psf[i].sname==NULL) {
			my_char(font,cc);
			return;
		}
		d_ps_this_font = font;
		d_ps_this_size = g.fontsz;
		out() << g.fontsz << " /" << psf[i].lname << " f" << endl;
	}
	if (cc >= 256) {
		if (cc <= 420) {
			/* FIXME: support g.inpath for these glyphs */
			out() << "/" << ps_glyph_name_map[cc-256].name << " glyphshow" << endl;
		}
	} else {
		if (isalnum(cc) && cc < 127) {
			out() << "(" << (char)cc << ")";
		} else {
			char temp[50];
			sprintf(temp,"(\\%o)",cc);
			out() << temp;
		}
		if (g.inpath) {
			out() << " ps" << endl;
		} else {
			out() << " s" << endl;
		}
	}
}

void PSGLEDevice::resetfont() {
	d_ps_this_font = 0;
	d_ps_this_size = 0;
}

void PSGLEDevice::read_psfont(void) {
	/* add additional ps fonts,  e.g.  pstr = TimesRoman */
	static int init_done;
	FILE *fptr;
	char *s;
	char inbuff[200];
	if (init_done) return;
	init_done = true;

	/* Find last used psf */
	for (i=0;;i++) if (psf[i].sname==NULL) break;

	string fname = fontdir("psfont.dat");
	fptr = fopen(fname.c_str(), "r");
	if (fptr == 0) return; /* if it doesn't exist then don't bother */

	if (fgets(inbuff, 200, fptr) != 0) {
		while (!feof(fptr)) {
			s = strchr(inbuff,'!');
			if (s!=NULL) *s=0;
			s = strtok(inbuff," \t,\n");
			if (s!=NULL) if (*s!='\n') {
				psf[i].sname = sdup(s);
				s = strtok(0," \t,\n");
				psf[i].lname = sdup(s);
				i++;
			}
			if (fgets(inbuff, 200, fptr) == 0) {
				break;
			}
		}
	}
	psf[i].sname = NULL;
	psf[i].lname = NULL;
}

void PSGLEDevice::displayGeometry(double width, double height, int *gsPixelWidth, int *gsPixelHeight, int *gsPixelRes) {
#if ( defined(__unix__) || defined(__APPLE__) ) && defined(ENABLE_GS_PREVIEW)
	Display *dpy = XOpenDisplay((char *) NULL);
	if (dpy == NULL){
		perror("Unable to open Display!");
		exit(1);
	}
	int screenNo = DefaultScreen(dpy);
	int dpyPixelWidth = DisplayWidth(dpy, screenNo);
	int dpyPixelHeight = DisplayHeight(dpy, screenNo);
	XCloseDisplay(dpy);
	double dpyXbyY = (double) dpyPixelWidth / (double) dpyPixelHeight;
	double gleXbyY = width / height;
	if (gleXbyY > dpyXbyY) {
		*gsPixelWidth = (int) (0.9 * dpyPixelWidth);
		*gsPixelRes = (int) (*gsPixelWidth/(width/CM_PER_INCH));
		*gsPixelHeight = (int) ( *gsPixelWidth / gleXbyY);
	} else {
		*gsPixelHeight = (int) (0.9 * dpyPixelHeight);
		*gsPixelRes = (int) (*gsPixelHeight/(height/CM_PER_INCH));
		*gsPixelWidth = (int) (*gsPixelHeight * gleXbyY);
	}
#endif
#if defined(_WIN32) && defined(ENABLE_GS_PREVIEW)
	// Not yet implemented
#endif
#if defined(__OS2__) && defined(ENABLE_GS_PREVIEW)
	// Not yet implemented
#endif
}

int PSGLEDevice::getDeviceType() {
	return m_IsEps ? GLE_DEVICE_EPS : GLE_DEVICE_PS;
}

void PSGLEDevice::psFileASCIILine(const char* prefix, int cnt, char ch, bool nl) {
	out() << prefix;
	for (int i = 0; i < cnt; i++) {
		out() << (char)ch;
	}
	if (nl) out() << endl;
}

void PSGLEDevice::bitmap(GLEBitmap* bitmap, GLEPoint* pos, GLEPoint* scale, int type) {
	/* Store current box */
	GLERectangle save_box;
	g_get_bounds(&save_box);
	/* Generate header in postrscript output */
	if (type != BITMAP_TYPE_USER) {
		string str = string("%% BEGIN image: ") + bitmap->getFName() + "\n";
		psFileASCIILine("%%", str.length()-3, '=', true);
		g_devcmd((char*)str.c_str());
		psFileASCIILine("%%", str.length()-3, '=', true);
	}
	g_devcmd("/GLESTATE save def \n");
	g_devcmd("gsave\n");
	g_devcmd("0 setgray 0 setlinecap 0 setlinewidth 0 setlinejoin\n");
	g_devcmd("10 setmiterlimit [] 0 setdash\n");
	g_gsave();
	/* Set options */
	bitmap->setCompress(0.0);
	bitmap->setASCII85(1);
	/* Get current position	*/
	g_scale(scale->getX(), scale->getY());
	g_translate(pos->getX(), pos->getY());
	/* Convert bitmap to postscript */
	bitmap->toPS(m_Out);
	bitmap->close();
	/* Footer */
	g_devcmd("grestore GLESTATE restore \n");
	g_grestore();
	if (type != BITMAP_TYPE_USER) {
		string str = string("%% END image: ") + bitmap->getFName() + "\n";
		psFileASCIILine("%%", str.length()-3, '=', true);
		g_devcmd((char*)str.c_str());
		psFileASCIILine("%%", str.length()-3, '=', true);
	}
	g_set_bounds(&save_box);
}

void PSGLEDevice::startRecording() {
	if (m_OutputFile != NULL) {
		delete m_OutputFile;
		m_OutputFile = NULL;
	}
	if (m_OutputBuffer != NULL) {
		delete m_OutputBuffer;
	}
	m_OutputBuffer = new ostringstream();
	m_Out = m_OutputBuffer;
}

void PSGLEDevice::getRecordedBytes(string* result) {
	*result = m_OutputBuffer->str();
}
