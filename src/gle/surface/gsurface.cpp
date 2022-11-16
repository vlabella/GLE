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

/*
 * Trying to add surface support back into GLE
 * Jan Struyf 2005
 */

#include "../all.h"
#include "../tokens/Tokenizer.h"
#include "../core.h"
#include "../file_io.h"
#include "../texinterface.h"
#include "../cutils.h"
#include "../cmdline.h"
#include "../config.h"
#include "../mem_limits.h"
#include "../token.h"
#include "../gprint.h"
#include "../cutils.h"
#include "../gle-block.h"

#define BEGINDEF extern
#include "../begin.h"

#include "gsurface.h"

extern int gle_debug;

FILE *df;

void hide(float *z,int nx, int ny, float minz, float maxz, struct surface_struct *sff);
void hide_enddefaults(void);

void text_expand(int x);
void hide_defaults();
void pass_line();
void pass_title();
void pass_cube();
void pass_top();
void pass_bot();
void pass_marker();
void pass_droplines();
void pass_riselines();
void pass_back();
void pass_right();
void pass_base();
void pass_axis();
void pass_anytitle();
void pass_zclip();
void pass_points(string);

void getstr(char *s);
float getf(void);
int geton(void);
//void pass_data(int *nx, int *ny, float *zmin, float *zmax);
void pass_data(bool force_zdata);
void pass_zdata(string,int *nx, int *ny, double *zmin, double *zmax);
bool alloc_zdata(int nx, int ny);
extern int trace_on,this_line;
char input_file[80];
static int xsample,ysample;

static struct surface_struct sf;
static double zmin = 10e10,zmax = -10e10;
static float *z;
static int nx,ny;
static double dxmin,dymin,dxmax,dymax;
double zclipmin,zclipmax;
int zclipminset,zclipmaxset;
int ct;

surface_struct* GLEInterface::getSurface() {
	return &sf;
}

void clean_surface() {
	sf.z = NULL;
}

class GLESurfaceBlockInstance : public GLEBlockInstance {
public:
	GLESurfaceBlockInstance(GLESurfaceBlockBase* parent);
	virtual ~GLESurfaceBlockInstance();
	virtual void executeLine(GLESourceLine& sline);
	virtual void endExecuteBlock();

private:
	GLEPoint m_origin;
};

GLESurfaceBlockInstance::GLESurfaceBlockInstance(GLESurfaceBlockBase* parent):
	GLEBlockInstance(parent)
{
	g_get_xy(&m_origin);
	hide_defaults();
}

GLESurfaceBlockInstance::~GLESurfaceBlockInstance() {
}

void GLESurfaceBlockInstance::executeLine(GLESourceLine& sline) {
	token_space();
	begin_init();
	int st = begin_token(sline, srclin, tk, &ntk, outbuff, true);
	if (!st) {
		return;
	}
	ct = 1;
	pass_line();
}

void GLESurfaceBlockInstance::endExecuteBlock() {
	if (nx==0 || ny==0) {
		// Create fake dataset for now...
		nx = ny = 2;
		alloc_zdata(nx,ny);
		z[0] = z[1] = z[2] = z[3] = -GLE_INF;
	}
	if (zclipminset || zclipmaxset) {
		for (int li = 0; li< nx*ny; li++) {
			if (zclipminset) if (z[li]<zclipmin) z[li] = zclipmin;
			if (zclipmaxset) if (z[li]>zclipmax) z[li] = zclipmax;
		}
		if (zclipminset) zmin = zclipmin;
		if (zclipmaxset) zmax = zclipmax;
	}
	hide_enddefaults();
	if (sf.zaxis.min != sf.zaxis.max) {
		zmin = sf.zaxis.min;
		zmax = sf.zaxis.max;
	}
	sf.nx = nx;
	sf.ny = ny;
	sf.z = z;
	sf.zmin = zmin;
	sf.zmax = zmax;
	hide(z,nx,ny,zmin,zmax,&sf);
	g_move(m_origin);
}

GLESurfaceBlockBase::GLESurfaceBlockBase():
	GLEBlockWithSimpleKeywords("surface", false)
{
	const char* commands[] = {
		"SIZE", "TITLE", "CUBE", "DATA", "ZDATA", "ROTATE", "EYE",
		"VIEW", "HARRAY", "ZCLIP", "SKIRT", "XLINES", "YLINES",
		"TOP", "UNDERNEATH", "HIDDEN", "MARKER", "POINTS", "DROPLINES",
		"RISELINES", "BASE", "BACK", "RIGHT", "ZCOLOUR", "ZCOLOR", "" };
	for (int i = 0; commands[i][0] != 0; ++i) {
		addKeyWord(commands[i]);
	}
	const char* axis[] = { "X", "Y", "Z", "" };
	for (int i = 0; axis[i][0] != 0; ++i) {
		addKeyWord(std::string(axis[i]) + "AXIS");
		addKeyWord(std::string(axis[i]) + "TITLE");
	}
}

GLESurfaceBlockBase::~GLESurfaceBlockBase() {
}

GLEBlockInstance* GLESurfaceBlockBase::beginExecuteBlockImpl(GLESourceLine& /* sline */, int* /* pcode */, int* /* cp */) {
	return new GLESurfaceBlockInstance(this);
}

#define kw(k) if (str_i_equals(tk[ct],k))

char* getstrv() {
	if (ct >= ntk) {
		gprint("Expecting string \n");
		return NULL;
	}
	string result;
       	pass_file_name(tk[++ct], result);
        return sdup((char*)result.c_str());
}

void getstr(char *s) {
        if (ct>=ntk) { gprint("Expecting Color or Lstyle\n"); return;}
        strncpy(s,tk[++ct],11);
}

float getf() {
        if (ct>=ntk) gprint("Expecting Number\n");
        return atof(tk[++ct]);
}

int geton() {
        if (ct>=ntk) gprint("Expecting ON | OFF\n");
        ct++;
        if (str_i_equals(tk[ct],"ON")) return true;
        if (str_i_equals(tk[ct],"OFF")) return false;
        gprint("Expecting ON | OFF, assuming ON\n");
        return true;
}

void pass_line() {
	if (ntk<1) return;
	kw("SIZE") {sf.screenx = getf(); sf.screeny = getf();}
	else kw("TITLE") pass_title();
	else kw("CUBE") pass_cube();
//        else kw("DATA") { pass_data(&nx,&ny,&zmin,&zmax); }
	else kw("DATA") { pass_data(false); }
	else kw("ZDATA") { pass_data(true); }
	else kw("ROTATE") {sf.xrotate = getf(); sf.yrotate = getf(); sf.zrotate = getf();}
	else kw("EYE") {sf.eye_x = getf(); sf.eye_y = getf(); sf.vdist = getf();}
	else kw("VIEW") {sf.eye_x = getf(); sf.eye_y = getf(); sf.vdist = getf();}
	else kw("HARRAY") sf.maxh = (int)getf();
	else kw("ZCLIP") pass_zclip();
	else kw("SKIRT") sf.skirt_on = geton();
	else kw("XLINES") sf.xlines_on = geton();
	else kw("YLINES") sf.ylines_on = geton();
	else kw("TOP") pass_top();
	else kw("UNDERNEATH") pass_bot();
	else kw("HIDDEN") sf.hidden_on = geton();
	else kw("MARKER") pass_marker();
	else kw("POINTS") pass_data(false);
	else kw("DROPLINES") pass_droplines();
	else kw("RISELINES") pass_riselines();
	else kw("BASE") pass_base();
	else kw("BACK") pass_back();
	else kw("RIGHT") pass_right();
	else kw("ZCOLOUR") getstr(sf.zcolour);
	else kw("ZCOLOR") getstr(sf.zcolour);
	else if (str_i_str(tk[1],"AXIS")!=NULL)  pass_axis();
	else if (str_i_str(tk[1],"TITLE")!=NULL)  pass_anytitle();
	else {
		stringstream err;
		err << "illegal keyword in surface block: '" << tk[ct] << "'";
		g_throw_parser_error(err.str());
	}
	if (ct<ntk) {
		stringstream err;
		err << "extra parameters on end of line: '" << tk[ct] << "'";
		g_throw_parser_error(err.str());
	}
}

void pass_title() {
        sf.title = getstrv();
        for (ct++;ct<=ntk;ct++) {
                kw("HEI") sf.title_hei = getf();
                else kw("DIST") sf.title_dist = getf();
                else kw("COLOR") getstr(sf.title_color);
                else gprint("Expecting one of HEI, DIST, COLOR , found {%s} \n",tk[ct]);
        }
}

void pass_anytitle() {
        struct GLEAxis3D *ax = NULL;
        if (toupper(*tk[ct])=='X') ax = &sf.xaxis;
        else if (toupper(*tk[ct])=='Y') ax = &sf.yaxis;
        else if (toupper(*tk[ct])=='Z') ax = &sf.zaxis;
        else return;

        ax->title = getstrv();
        for (ct++;ct<=ntk;ct++) {
                kw("HEI") ax->title_hei = getf();
                else kw("DIST") ax->title_dist = getf();
                else kw("COLOR") getstr(ax->title_color);
                else gprint("Expecting one of HEI, DIST, COLOR , found {%s} \n",tk[ct]);
        }
}

void pass_cube() {
        for (ct++;ct<=ntk;ct++) {
                kw("ON") sf.cube_on = true;
                else kw("OFF") sf.cube_on = false;
                else kw("NOFRONT") sf.cube_front_on = false;
                else kw("FRONT") sf.cube_front_on = geton();
                else kw("LSTYLE") getstr(sf.cube_lstyle);
                else kw("COLOR") getstr(sf.cube_color);
                else kw("XLEN") sf.sizex = getf();
                else kw("YLEN") sf.sizey = getf();
                else kw("ZLEN") sf.sizez = getf();
                else gprint("Expecting one of OFF, XLEN, YLEN, ZLEN, FRONT, LSTYLE, COLOR, found {%s} \n",tk[ct]);
        }

}

void pass_zclip() {
        for (ct++;ct<=ntk;ct++) {
                kw("MIN") { zclipmin = getf(); zclipminset = true; }
                else kw("MAX") { zclipmax = getf(); zclipmaxset = true; }
                else gprint("Expecting one of MIN, MAX found {%s} \n",tk[ct]);
        }
}

void pass_back() {
        for (ct++;ct<=ntk;ct++) {
                kw("YSTEP") sf.back_ystep = getf();
                else kw("ZSTEP") sf.back_zstep = getf();
                else kw("LSTYLE") getstr(sf.back_lstyle);
                else kw("COLOR") getstr(sf.back_color);
                else kw("NOHIDDEN") sf.back_hidden = false;
                else gprint("Expecting one of YSTEP, ZSTEP, LSTYLE, COLOR found {%s} \n",tk[ct]);
        }
}

void pass_right() {
        for (ct++;ct<=ntk;ct++) {
                kw("ZSTEP") sf.right_zstep = getf();
                else kw("XSTEP") sf.right_xstep = getf();
                else kw("LSTYLE") getstr(sf.right_lstyle);
                else kw("COLOR") getstr(sf.right_color);
                else kw("NOHIDDEN") sf.right_hidden = false;
                else gprint("Expecting one of ZSTEP, XSTEP, LSTYLE, COLOR found {%s} \n",tk[ct]);
        }
}

void pass_base() {
        for (ct++;ct<=ntk;ct++) {
                kw("XSTEP") sf.base_xstep = getf();
                else kw("YSTEP") sf.base_ystep = getf();
                else kw("LSTYLE") getstr(sf.base_lstyle);
                else kw("COLOR") getstr(sf.base_color);
                else kw("NOHIDDEN") sf.base_hidden = false;
                else gprint("Expecting one of XSTEP, YSTEP, LSTYLE, COLOR found {%s} \n",tk[ct]);
        }
}

/* toplines lstyle color hidden */
void pass_droplines() {
        sf.droplines = true;
        for (ct++;ct<=ntk;ct++) {
                kw("LSTYLE") getstr(sf.droplines_lstyle);
                else kw("COLOR") getstr(sf.droplines_color);
                else kw("HIDDEN") sf.droplines_hidden = true;
                else gprint("Expecting one of LSTYLE, COLOR , found {%s} \n",tk[ct]);
        }

}

/* toplines lstyle color hidden */
void pass_riselines() {
        sf.riselines = true;
        for (ct++;ct<=ntk;ct++) {
                kw("LSTYLE") getstr(sf.riselines_lstyle);
                else kw("COLOR") getstr(sf.riselines_color);
                else kw("HIDDEN") sf.riselines_hidden = true;
                else gprint("Expecting one of LSTYLE, COLOR , found {%s} \n",tk[ct]);
        }

}

/* top lstyle color off */
void pass_top() {
        for (ct++;ct<=ntk;ct++) {
                kw("LSTYLE") getstr(sf.top_lstyle);
                else kw("COLOR") getstr(sf.top_color);
                else kw("ON") sf.top_on = true;
                else kw("OFF") sf.top_on = false;
                else gprint("Expecting one of OFF, LSTYLE, COLOR , found {%s} \n",tk[ct]);
        }
}

void pass_bot() {
        sf.bot_on = true;
        for (ct++;ct<=ntk;ct++) {
                kw("LSTYLE") getstr(sf.bot_lstyle);
                else kw("COLOR") getstr(sf.bot_color);
                else kw("ON") sf.bot_on = true;
                else kw("OFF") sf.bot_on = false;
                else gprint("Expecting one of ON, OFF, LSTYLE, COLOR , found {%s} \n",tk[ct]);
        }
}

void pass_marker() {
        getstr(sf.marker);
        for (ct++;ct<=ntk;ct++) {
                kw("COLOR") getstr(sf.marker_color);
                else kw("HEI") sf.marker_hei = getf();
                else gprint("Expecting MARKER markername COLOR c HEI h, found {%s} \n",tk[ct]);
        }
}

void pass_axis() {
        struct GLEAxis3D *ax=NULL;
        if (toupper(*tk[ct])=='X') ax = &sf.xaxis;
        if (toupper(*tk[ct])=='Y') ax = &sf.yaxis;
        if (toupper(*tk[ct])=='Z') ax = &sf.zaxis;
        if (ax==NULL) { gprint("Expecting xaxis,yaxis,zaxis,  \n"); return;}

        for (ct++;ct<=ntk;ct++) {
                     kw("MIN") {ax->min = getf(); ax->minset = true;}
                else kw("MAX") {ax->max = getf(); ax->maxset = true;}
                else kw("DTICKS") ax->step = getf();
                else kw("TICKLEN") ax->ticklen = getf();
                else kw("LEN") ax->ticklen = getf();
                else kw("COLOR") getstr(ax->color);
                else kw("STEP") ax->step = getf();
                else kw("HEI") ax->hei = getf();
                else kw("OFF") ax->on = false;
                else kw("ON") ax->on = true;
                else kw("NOFIRST") ax->nofirst = true;
                else kw("NOLAST") ax->nolast = true;
                else gprint("Expecting HEI, DIST, COLOR , TICKLEN, MIN, MAX, STEP, found {%s} \n",tk[ct]);
        }
}

static char buff[2032];

bool alloc_zdata(int nx, int ny) {
        if (z!=NULL) free(z);
        z = (float*)malloc(nx * (ny+1) * sizeof(float));
        if (z==NULL) {
                gprint("Unable to allocate enough memory for datafile\n");
                return true;
        }
        return false;
}

double getkeyval(char *buff,const char *k) {
        char *s;
        s = str_i_str(buff,k);
        if (s!=NULL) return atof(s+strlen(k));
        return 0.0;
}

void pass_data(bool force_zdata) {
	string fname = getstrv();
	if (str_i_ends_with(fname, ".z") || force_zdata) {
		pass_zdata(fname,&nx,&ny,&zmin,&zmax);
	} else {
		pass_points(fname);

	}
}

/* data test.z [nx ny] */
//void pass_data(int *nx, int *ny, float *zmin, float *zmax) {
void pass_zdata(string fname,int *nx, int *ny, double *zmin, double *zmax) {
        double v;
        int x,y,xx,yy;
        int c,b,mx = 0,my = 0,xcnt,ycnt;
        char *s;

        xx = yy = x = y = 0;
         *nx = 0; *ny = 0;
        for (ct++;ct<=ntk;ct++) {
                kw("NX") *nx = (int)getf();
                else kw("NY") *ny = (int)getf();
                else kw("XSAMPLE") xsample = (int)getf();
                else kw("YSAMPLE") ysample = (int)getf();
                else kw("SAMPLE") {xsample = (int)getf(); ysample = xsample; }
                else gprint("Wanted DATA file.Z  XSAMPLE YSAMPLE SAMPLE NX NY. Found {%s} \n",tk[ct]);
        }
        if (*nx != 0) {
                mx = (*nx - 1)/xsample + 1;
                my = (*ny - 1)/ysample + 1;
        }
        xcnt = xsample; ycnt = ysample;

        if (nx==NULL || ny==0) printf("nx or ny is zero \n");
        if (*nx!=0 && *ny != 0)   if (alloc_zdata(*nx,*ny)) return;

        df = validate_fopen(fname.c_str(), "r", true);
        if (df==NULL) {*nx = 0; *ny = 0; return;}
        for (;!feof(df);) {
          if (fgets(buff,2000,df)!=NULL) {
                if (*nx==0) {
                        *nx  = (int)getkeyval(buff,"NX");
                        *ny  = (int)getkeyval(buff,"NY");
                        dxmin = getkeyval(buff,"XMIN");
                        dymin = getkeyval(buff,"YMIN");
                        dxmax = getkeyval(buff,"XMAX");
                        dymax = getkeyval(buff,"YMAX");
                        if (*nx==0 || *ny==0) {
                                gprint("Expecting ! NX 10 NY 10  in first line of data file \n");
                                return;
                        }
                        mx = (*nx - 1)/xsample + 1;
                        my = (*ny - 1)/ysample + 1;
                        if (alloc_zdata(mx,my)) return;
                        if (fgets(buff,2000,df) == 0) {
                        	return;
                        }
                }
check_again:
                b = strlen(buff);
                c = buff[b-1];
                if (strchr(" \n\t",c)==NULL) { /* we're halfway through a number */
                        buff[b] = getc(df);
                        buff[b+1] = 0;
                        goto check_again;
                }
                s = strchr(buff,'!');
                if (s!=NULL) *s = 0;
                s = strtok(buff," \t\n,");
                for (;s!=NULL;) {
                        v = atof(s);
                        if (isdigit(*s) || *s=='-' || *s=='+' || *s=='.') {
                                if (x>= *nx) {
                                        if (ycnt==ysample) {ycnt = 0; yy++;}
                                        x = 0; y++; ycnt++; xx = 0; xcnt = xsample;
                                }
                                if (y>= *ny) {
                                        gprint("Too much data in data file %ld %d \n",y,*ny);
                                        return;
                                }
                                if (v < *zmin) *zmin = v;
                                if (v > *zmax) *zmax = v;

                                if (xx<mx && xcnt==xsample && ycnt==ysample) {
                                        z[xx + yy * mx] = v;
                                        xx++;
                                        xcnt = 0;
                                }
                                xcnt++;
                                x++;
                        } else gprint("Not a number {%s} \n",s);
                        s = strtok(NULL," \t\n,");
                }
          }
        }
        fclose(df);
        *ny = my;
        *nx = mx;
        return;
}

float *pntxyz;
int npnts;

void pnt_alloc(int size) {
        static int cursize;
        void *d;
        if (size+10<cursize) return;
        size = size*2;
        d = malloc(size*sizeof(float));
        if (d==NULL) {
                gprint("Unable to allocate storage for POINTS data\n");
                gle_abort("memory shortage\n");
        }
        if (cursize>0) memcpy(d,pntxyz,cursize*sizeof(float));
        cursize = size;
        pntxyz = (float*)d;
}

void pass_points(string fname) {
        double v;
        char *s;
        int nd,nc;

        pnt_alloc(30);

        if (ct>ntk) {
                gprint("Expecting POINTS filename.xyz \n");
                return;
        }

        df = validate_fopen(fname.c_str(), "r", true);
        if (df==NULL) return;
        nd = 0;
        for (;!feof(df);) {
          if (fgets(buff,2000,df)!=NULL) {
                s = strchr(buff,'!');
                if (s!=NULL) *s = 0;
                nc = 0;
                s = strtok(buff," \t\n,");
                for (;s!=NULL;) {
                        v = atof(s);
                        pnt_alloc(nd);
                        if (isdigit(*s) || *s=='-' || *s=='+' || *s=='.') {
                                pntxyz[nd++] = v; nc++;
                        } else gprint("Not a number {%s} \n",s);
                        s = strtok(NULL," \t\n,");
                }
                if (nc>0 && nc!=3) {
                        gprint("Expecting 3 columns in data file, found %d (FATAL ERROR) \n",nc);
                }
          }
        }
        fclose(df);
        npnts = nd;
        sf.pntxyz = pntxyz;
        sf.npnts = npnts;
}

/* defaults after data and commands read */
void hide_enddefaults() {
        if (dxmin==dxmax) dxmax = nx-1;
        if (dymin==dymax) dymax = ny-1;
        if (!sf.xaxis.maxset)  sf.xaxis.max = dxmax;
        if (!sf.yaxis.maxset)  sf.yaxis.max = dymax;
        if (!sf.xaxis.minset)  sf.xaxis.min = dxmin;
        if (!sf.yaxis.minset)  sf.yaxis.min = dymin;
        if (!sf.zaxis.minset)  sf.zaxis.min = zmin;
        if (!sf.zaxis.maxset)  sf.zaxis.max = zmax;
        if (sf.zrotate==0 && sf.xrotate==0 && sf.yrotate==0) {
                sf.xrotate = 60;
                sf.yrotate = 50;
                sf.zrotate = 20;        /* not needed as is corrected later*/
        }
        if (sf.eye_x== -1) {
                sf.eye_x = sf.sizex/2.0;
                sf.eye_y = sf.sizex/2.0;
        }
}

void hide_defaults() {
	/* Setup some defaults, */
	memset(&sf,0,sizeof(sf));
	sf.sizey = sf.sizex = 18;
	sf.sizey = sf.sizex = sf.sizez = 18;
	sf.screenx = 18; sf.screeny = 18;
	sf.eye_x = -1;
	sf.zaxis.type = 2;
	sf.yaxis.type = 1;
	sf.xaxis.on = sf.yaxis.on = sf.zaxis.on = true;
	sf.xaxis.min = 0;
	sf.xaxis.max = 10;
	sf.yaxis.min = 0;
	sf.yaxis.max = 10;
	sf.zaxis.min = 0;
	sf.zaxis.max = 10;
	sf.cube_hidden_on = true;
	sf.cube_on = true;
	sf.cube_front_on = false;
	sf.xlines_on = true;
	sf.ylines_on = true;
	sf.hidden_on = true;
	sf.top_on = true;
	sf.bot_on = false;
	sf.base_hidden = sf.right_hidden = sf.back_hidden = true;
	strcpy(sf.title_color, "BLACK");
	strcpy(sf.cube_color, "BLACK");
	strcpy(sf.droplines_color, "BLACK");
	strcpy(sf.riselines_color, "BLACK");
	strcpy(sf.marker_color, "BLACK");
	strcpy(sf.xaxis.color, "BLACK");
	strcpy(sf.xaxis.title_color, "BLACK");
	strcpy(sf.yaxis.color, "BLACK");
	strcpy(sf.yaxis.title_color, "BLACK");
	strcpy(sf.zaxis.color, "BLACK");
	strcpy(sf.zaxis.title_color, "BLACK");
	nx = 0;
	ny = 0;
	xsample = 1;
	ysample = 1;
	zclipmin = 0;
	zclipminset = false;
	zclipmax = 0;
	zclipmaxset = false;
	zmin = GLE_INF;
	zmax = -GLE_INF;
}
