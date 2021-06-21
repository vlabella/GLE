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

class GLESurfaceBlockBase : public GLEBlockWithSimpleKeywords {
public:
	GLESurfaceBlockBase();
	virtual ~GLESurfaceBlockBase();

	virtual GLEBlockInstance* beginExecuteBlockImpl(GLESourceLine& sline, int *pcode, int *cp);
};

typedef char MARKER[12];
typedef char COLOR[12];
typedef char LSTYLE[12];

struct GLEAxis3D {
	int type;	/* 0=xaxis,yaxis, 1=zaxis. */
	float min, max, step, hei, dist, ticklen;
	int minset,maxset;
	COLOR color;
	int on;
	char* title;
	COLOR title_color;
	float title_hei,title_dist;
	int nofirst,nolast;
};

struct surface_struct {
	float sizez, sizex, sizey, title_hei, title_dist;
	float screenx, screeny;
	char* title;
	COLOR zcolour;
	COLOR title_color;
	int maxh;	/* dimension for height array, about 1000 is good */
	int npnts;    	/* data points for markers, droplines etc */
	float *pntxyz;

	int nx, ny;
	float *z;
	float zmin, zmax;

	struct GLEAxis3D xaxis;	/* The axes */
	struct GLEAxis3D yaxis;
	struct GLEAxis3D zaxis;

	int back_hidden,right_hidden,base_hidden;
	COLOR back_lstyle,back_color;		/* grids on back,base,right */
	float back_ystep,back_zstep;
	COLOR base_color,base_lstyle;
	float base_xstep,base_ystep;
	COLOR right_color,right_lstyle;
	float right_xstep,right_zstep;

	int cube_hidden_on;		/* Cube hidden lines not removed */
	int cube_on,cube_front_on;
	COLOR cube_color;
	LSTYLE cube_lstyle;

	float eye_x, eye_y, vdist;
	float xrotate, yrotate, zrotate;
	int skirt_on;
	int xlines_on,ylines_on;
	int hidden_on,top_on,bot_on;
	COLOR top_color,bot_color;
	LSTYLE top_lstyle,bot_lstyle;

	int droplines,droplines_hidden;
	LSTYLE droplines_lstyle;
	COLOR droplines_color;
	int riselines,riselines_hidden;
	LSTYLE riselines_lstyle;
	COLOR riselines_color;

	MARKER marker;
	COLOR marker_color;
	float marker_hei;

	int ctop_hidden;		/* Contouring */
	float ctop_from,ctop_to,ctop_step;
	int cbase_hidden;
	float cbase_from,cbase_to,cbase_step;
} ;
