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

#include "all.h"
#include "gle-interface/gle-base.h"
#include "tokens/stokenizer.h"
#include "justify.h"
#include "core.h"
#include "axis.h"
#include "gprint.h"
#include "d_interface.h"
#include "numberformat.h"
#include "cutils.h"

#define GRAPHDEF extern
#include "mem_limits.h"
#include "token.h"
#include "graph.h"

double start_subtick(double dsubticks, double dticks, GLEAxis* ax);
void nice_ticks(double *dticks, double *gmin,double *gmax, double *t1,double *tn,int minset, int maxset);
void numtrim(char **d,char *s, double dticks);
void nice_log_ticks(double *start, double *last, double gmin, double gmax) throw (ParserError);

double fnloglen(double v, GLEAxis *ax);
double fnlogx(double v, GLEAxis *ax);

void draw_axis_titles(GLEAxis *ax, double h, double ox, double oy, GLEMeasureBox* measure);
void draw_axis_titles_v35(GLEAxis *ax, double h, double ox, double oy, double dticks, double llen);

extern double graph_x1;
extern double graph_x2;
extern double graph_y1;
extern double graph_y2;

void add_tex_labels(string* title)
{
	if (g_get_tex_labels() && str_i_str(*title, "\\tex{") == -1) {
		title->insert(0, "\\tex{");
		title->append("}");
	}
}

double axis_range_dist_perc(double v1, double v2, GLERange* range, bool log) {
	if (log) {
		double lwidth = log10(range->getMax()) - log10(range->getMin());
		v1 = log10(v1)/lwidth;
		v2 = log10(v2)/lwidth;
	} else {
		v1 /= range->getWidth();
		v2 /= range->getWidth();
	}
	return fabs(v1 - v2);
}

double fnloglen(double v, GLEAxis *ax) {
	return (v - log10(ax->getMin())) / (log10(ax->getMax()) - log10(ax->getMin())) * ax->length;
}

double fnAxisX(double v, GLEAxis *ax) {
	if (ax->negate) {
		v = ax->getMax() - (v - ax->getMin());
	}
	if (ax->log) {
		return fnloglen(log10(v), ax);
	} else {
		return (v - ax->getMin()) / (ax->getMax() - ax->getMin()) * ax->length;
	}
}

bool axis_value_equal(double v1, double v2, GLEAxis* ax) {
	if (ax->log) {
		if (v2 == 0) {
			return fabs(v1) < 0.001;
		} else {
			return fabs(v1 - v2) / v2 < 0.001;
		}
	} else {
		return fabs(v1 - v2) < ax->dsubticks/100.0;
	}
}

bool axis_is_pos(double pos, int* cnt, double del, vector<double>& vec) {
	if (*cnt < (int)vec.size()) {
		while (*cnt < (int)vec.size() && pos > vec[*cnt]+del/100.0) {
			(*cnt)++;
		}
		if (*cnt < (int)vec.size() && fabs(pos-vec[*cnt]) < del/100.0) {
			return true;
		}
	}
	return false;
}

bool axis_is_pos_perc(double pos, int* cnt, double perc, vector<double>& vec) {
	if (*cnt < (int)vec.size()) {
		while (*cnt < (int)vec.size() && pos > vec[*cnt]*(1+perc)) {
			(*cnt)++;
		}
		if (*cnt < (int)vec.size()) {
			if (vec[*cnt] != 0.0) {
				if (fabs((pos-vec[*cnt])/vec[*cnt]) < perc) return true;
			} else {
				if (fabs(pos-vec[*cnt]) < perc) return true;
			}
		}
	}
	return false;
}

void axis_draw_tick(GLEAxis *ax, double fi, int* tick1_cnt, int* tick2_cnt, double ox, double oy, double t) {
	bool has_tick1 = t > 0.0 || ax->ticks_both;
	bool has_tick2 = t < 0.0 || ax->ticks_both;
	has_tick1 &= !ax->isNoTick1(fi, tick1_cnt, ax->dsubticks);
	has_tick2 &= !ax->isNoTick2(fi, tick2_cnt, ax->dsubticks);
	if (!has_tick1 && !has_tick2) return;
	double inv = axis_ticks_neg(ax->type) ? -1.0 : 1.0;
	double from = has_tick2 ? -fabs(t)*inv : 0.0;
	double to = has_tick1 ? fabs(t)*inv : 0.0;
	if (axis_horizontal(ax->type)) {
		g_move(ox + fnAxisX(fi, ax), oy + from);
		g_line(ox + fnAxisX(fi, ax), oy + to);
	} else {
		g_move(ox + from, oy + fnAxisX(fi, ax));
		g_line(ox + to, oy + fnAxisX(fi, ax));
	}
}

void axis_draw_tick_log(GLEAxis *ax, double fi, int* tick1_cnt, int* tick2_cnt, double ox, double oy, double t) {
	bool has_tick1 = t > 0.0 || ax->ticks_both;
	bool has_tick2 = t < 0.0 || ax->ticks_both;
	has_tick1 &= !ax->isNoTick1Perc(fi, tick1_cnt);
	has_tick2 &= !ax->isNoTick2Perc(fi, tick2_cnt);
	if (!has_tick1 && !has_tick2) return;
	double inv = axis_ticks_neg(ax->type) ? -1.0 : 1.0;
	double from = has_tick2 ? -fabs(t)*inv : 0.0;
	double to = has_tick1 ? fabs(t)*inv : 0.0;
	if (axis_horizontal(ax->type)) {
		g_move(ox + fnAxisX(fi, ax), oy + from);
		g_line(ox + fnAxisX(fi, ax), oy + to);
	} else {
		g_move(ox + from, oy + fnAxisX(fi, ax));
		g_line(ox + to, oy + fnAxisX(fi, ax));
	}
}

bool inAxisRange(double value, GLEAxis* ax) {
	if (value >= ax->getMin() && value <= ax->getMax()) {
		return true;
	}
	if (axis_value_equal(value, ax->getMin(), ax)) {
		return true;
	}
	if (axis_value_equal(value, ax->getMax(), ax)) {
		return true;
	}
	return false;
}

std::vector<double> getLogSubPlaces(double pos, double gmin, double gmax, int lgset) {
   std::vector<double> result;
   if (lgset == GLE_AXIS_LOG_1 || lgset == GLE_AXIS_LOG_25 || lgset == GLE_AXIS_LOG_25B) {
      for (int i = 2; i <= 9; i++) {
         if (lgset == GLE_AXIS_LOG_1 || i == 2 || i == 5) {
            double pos_sub = pos * i;
            if (pos_sub >= gmin && pos_sub <= gmax) {
               result.push_back(pos_sub);
            }
         }
      }
   }
   return result;
}

std::string g_format_label(double value, double dticks, GLENumberFormat* format) {
	std::string result;
	double absValue = fabs(value);
	if (absValue < 0.00001 * dticks) {
		// otherwise place for "zero" may be -8.2556e-25 or some other small number
		value = 0;
	}
	if (format != NULL) {
		format->format(value, &result);
	} else {
		char cbuff[100];
		char* num_trim_str = NULL;
		if (value == 0 || (absValue > 1e-5 && absValue < 1e6)) {
			sprintf(cbuff, "%f", value);
		} else {
			sprintf(cbuff, "%e", value);
		}
		numtrim(&num_trim_str, cbuff, dticks);
		if (num_trim_str != NULL) {
			result = num_trim_str;
			myfree(num_trim_str);
		}
	}
	return result;
}

void draw_axis(GLEAxis *ax, GLERectangle* box, DrawAxisPart drawPart) {
	double x,y,gmin,gmax,dticks,tick1,tickn;
	int savecap;
	double h,dist;
	double ox,oy;

	// Fix for font problems by Jan Struyf
	g_resetfont();

	bool xax = axis_horizontal(ax->type);
	if (ax->off) return;
	g_get_xy(&ox,&oy);

   bool drawsubticks = true;
	bool drawticks = true;
	bool drawrest = true;

	if (ax->hasGrid()) {
      if (drawPart == DRAW_AXIS_GRID_SUBTICKS) {
         drawticks = false;
         drawrest = false;
      } else if (drawPart == DRAW_AXIS_GRID_TICKS) {
         drawsubticks = false;
         drawrest = false;
      } else {
         CUtilsAssert(drawPart == DRAW_AXIS_ALL);
         drawsubticks = false;
         drawticks = false;
      }
	} else {
		if (drawPart != DRAW_AXIS_ALL) return;
	}
	
/*----------------------------- Generate the places for labels to go */
	gmin = ax->getMin();
	gmax = ax->getMax();
	int lgset = ax->lgset;

   vector<double> subplaces;	
	int lgset_def = GLE_AXIS_LOG_OFF;
	if (ax->log) {
		h = ax->label_hei;
		if (h==0) h = ax->base;
		nice_log_ticks(&tick1, &tickn, gmin, gmax);
		/* find suitable value for dticks */
		if (lgset == GLE_AXIS_LOG_DEFAULT || lgset == GLE_AXIS_LOG_OFF) {
			dticks = (tickn - tick1) / ax->length * 3 * h;
			if (dticks >= 4) {
				dticks = ((int)dticks / 5) * 5.0;
			} else if (dticks >= 2) {
				dticks = 2;
			}
		} else {
			dticks = 1.0;
		}
		/* nticks explicitly given */
		if (ax->nticks != 0) {
			dticks = (tickn - tick1)/(ax->nticks - 1);
		}
		/* dticks explicitly given */
		if (ax->dticks != 0) {
			dticks = ax->dticks;
			if (!ax->subticks_off) {
				/* values smaller than 0.9 translate to lgset */
				if (dticks < 0.2) lgset_def = GLE_AXIS_LOG_1;
				else if (dticks < 0.9) lgset_def = GLE_AXIS_LOG_25;
			}
		}
		/* round dticks to integer */
		if (dticks < 1.0) {
			dticks = 1.0;
		} else {
			dticks = floor(dticks+0.001);
		}
		/* define default value for lg_set */
		if (dticks == 1.0 && lgset_def == GLE_AXIS_LOG_OFF && !ax->subticks_off) {
			/* dist between 10^0 and 10^1 */
			dist = fnloglen(2, ax) - fnloglen(1, ax);
			if (dist > ax->base*22.0) lgset_def = GLE_AXIS_LOG_1;
			else if (dist > ax->base*4.5) lgset_def = GLE_AXIS_LOG_25;
		}
		/* now set value of lgset if not already given */
		if (lgset == GLE_AXIS_LOG_DEFAULT) {
			lgset = lgset_def;
		}
		if (ax->getNbPlaces() == 0) {
			for (double pw10 = tick1; pw10 <= tickn; pw10 += dticks) {
				double pos = pow(10.0, pw10);
				if (equals_rel(gmin, pos)) {
					/* first tick */
					if (!ax->nofirst) {
						ax->addPlace(pos);
					}
				} else if (equals_rel(gmax, pos)) {
					/* last tick */
					if (!ax->nolast) {
						ax->addPlace(pos);
					}
				} else {
					/* add if in window */
					if (pos >= gmin && pos <= gmax) {
						ax->addPlace(pos);
					}
				}
				/* also label subticks? */
            std::vector<double> crSubPlaces(getLogSubPlaces(pos, gmin, gmax, lgset));
            for (std::vector<double>::iterator i(crSubPlaces.begin()); i != crSubPlaces.end(); ++i) {
               ax->addPlace(*i);
            }
			}
		}
		for (double pw10 = tick1; pw10 <= tickn; pw10 += dticks) {
         std::vector<double> crSubPlaces(getLogSubPlaces(pow(10.0, pw10), gmin, gmax, lgset));
         for (std::vector<double>::iterator i(crSubPlaces.begin()); i != crSubPlaces.end(); ++i) {
            subplaces.push_back(*i);
         }
      }
	} else {
		dticks = 0;
		GLERangeSet* range = ax->getRange();
		if (ax->nticks != 0) dticks = (gmax - gmin)/ax->nticks;
		if (ax->dticks != 0) dticks = ax->dticks;
		if (ax->has_ftick) {
			tick1 = ax->ftick;
			if (ax->dticks == 0) {
				// Compute suitable value for dticks
				double dummy = 0.0;
				nice_ticks(&dticks, &gmin, &gmax, &dummy, &tickn, range->hasMin(), range->hasMax());
			}
			if (ax->nticks != 0) {
				tickn = tick1 + (ax->nticks-1) * dticks;
			} else {
				tickn = tick1 + floor((gmax-tick1)/dticks) * dticks;
			}
		} else {
			nice_ticks(&dticks, &gmin, &gmax, &tick1, &tickn, range->hasMin(), range->hasMax());
		}
		if (ax->nofirst) tick1 = tick1 + dticks;
		if (ax->nolast) tickn = tickn - dticks;
		tickn = tickn + dticks/100.0;
		if (ax->getNbPlaces() == 0) {
			/* haven't been set, so generate positions. */
			double prev_tick = GLE_INF;
			double end_tick = (tickn+dticks/100.0);
			for (double fi = tick1; fi <= end_tick; fi += dticks) {
				if (fi - prev_tick == 0.0) {
					/* dticks too small for precision of double */
					ax->subticks_off = true;
					break;
				}
				ax->addPlace(fi);
				prev_tick = fi;
			}
		}
	}

/*------------------------------  Now draw the ticks	(subticks first) */

	// ticks length
	double tlen = ax->base*g_get_fconst(GLEC_TICKSSCALE);
	if (ax->ticks_length!=0) tlen = ax->ticks_length;

	// subticks length and distance
	double stlen = tlen/2;
	if (ax->subticks_length!=0) stlen = ax->subticks_length;

	// draw subticks
	if (ax->log) {
		if (!ax->subticks_off && drawsubticks) {
			g_gsave();
			g_set_color(ax->subticks_color);
			g_set_line_width(ax->subticks_lwidth);
			g_set_line_style(ax->subticks_lstyle);
			bool draw_log_subticks = false;
			if (ax->has_subticks_onoff) {
				// if "subticks = on" explicitly given -> draw log subticks if dticks <= 1.0
				draw_log_subticks = (dticks <= 1.0);
			} else {
				// if "subticks = on" not given -> draw log subticks based on default lgset
				draw_log_subticks = (lgset_def != GLE_AXIS_LOG_OFF);
			}
			if (lgset != GLE_AXIS_LOG_OFF) {
				// if log sub-labels on then also draw log subticks (if subtick "on" or "undefined")
				draw_log_subticks = true;
			}
			int place_cnt = 0;
			int tick1_cnt = 0;
			int tick2_cnt = 0;
			for (double pw10 = tick1; pw10 <= tickn; pw10 += 1.0) {
				double pos = pow(10.0, pw10);
				if (pos > gmin && pos < gmax && !ax->isPlaceRel(pos, &place_cnt)) {
					axis_draw_tick_log(ax, pos, &tick1_cnt, &tick2_cnt, ox, oy, stlen);
				}
				if (draw_log_subticks) {
					for (int i = 2; i <= 9; i++) {
						double pos_sub = pos * i;
						if (pos_sub > gmin && pos_sub < gmax) {
							axis_draw_tick_log(ax, pos_sub, &tick1_cnt, &tick2_cnt, ox, oy, stlen);
						}
					}
				}
			}
			g_grestore();
		}
	} else {
		double dsubticks = 0;
		if (ax->nsubticks != 0) dsubticks = dticks/(ax->nsubticks+1);
		if (ax->dsubticks != 0) dsubticks = ax->dsubticks;
		if (dsubticks == 0) dsubticks = dticks / 2.0;
		ax->dsubticks = dsubticks;
		if (!ax->subticks_off && drawsubticks) {
			double firstSubtick = start_subtick(dsubticks, dticks, ax);
			double subtickLimit = ax->getMax() + dsubticks/100.0;
			int place_cnt = 0;
			int tick1_cnt = 0;
			int tick2_cnt = 0;
			g_gsave();
			g_set_color(ax->subticks_color);
			g_set_line_width(ax->subticks_lwidth);
			g_set_line_style(ax->subticks_lstyle);
			for (double fi = firstSubtick; fi <= subtickLimit; fi += dsubticks) {
				if (inAxisRange(fi, ax) && !ax->isPlace(fi, &place_cnt, dsubticks)) {
					axis_draw_tick(ax, fi, &tick1_cnt, &tick2_cnt, ox, oy, stlen);
				}
			}
			g_grestore();
		}
	}

/*------------------------------  Now the main ticks */

	if (!ax->ticks_off && drawticks) {
		g_gsave();
		g_set_color(ax->ticks_color);
		g_set_line_width(ax->ticks_lwidth);
		g_set_line_style(ax->ticks_lstyle);
		int tick1_cnt = 0;
		int tick2_cnt = 0;
      int subplace_cnt = 0;
		if (ax->log) {
			/* Draw log ticks */
			for (int i = 0; i < ax->getNbPlaces(); i++) {
				double fi = ax->places[i];
				if (!axis_is_pos_perc(fi, &subplace_cnt, 1e-6, subplaces)) {
					/* only draw major ticks */
					axis_draw_tick_log(ax, fi, &tick1_cnt, &tick2_cnt, ox, oy, tlen);
				}
			}
		} else {
			for (int i = 0; i < ax->getNbPlaces(); i++) {
				double fi = ax->places[i];
				axis_draw_tick(ax, fi, &tick1_cnt, &tick2_cnt, ox, oy, tlen);
			}
		}
		g_grestore();
	}

/*-----------------------------  Ok lets draw the side now */
/*-----------------------------  Note: after the ticks - this gives better result if ticks are different color */

	if (!ax->side_off && drawrest) {
		// FIXME: is cap not saved by gsave??
		g_get_line_cap(&savecap);
		g_gsave();
		g_set_color(ax->side_color);
		g_set_line_width(ax->side_lwidth);
		g_set_line_style(ax->side_lstyle);
		g_set_line_cap(1);
		g_get_xy(&x,&y);
		if (xax) g_line(x+ax->length,y);
		else g_line(x,y+ax->length);
		g_grestore();
		g_set_line_cap(savecap);
	}

/*------------------------------  Now draw the labels */

	double real_llen = 0;
	if (!ax->ticks_off) {
		if (tlen < 0) real_llen = -tlen;
		if (ax->ticks_both) real_llen = +tlen;
	}
	h = ax->label_hei;
	if (h == 0) h = ax->base;
	double llen = real_llen + h*g_get_fconst(GLEC_ALABELDIST);
	if (ax->label_dist != 0) llen = ax->label_dist;
	if (ax->getNbNames() == 0) {
		char cbuff[100];
		GLENumberFormat* format = ax->format == "" ? NULL : new GLENumberFormat(ax->format);
		if (ax->log) {
			int subplace_cnt = 0;
			for (int i = 0; i < ax->getNbPlaces(); i++) {
				double fi = ax->places[i];
				int n = (int) floor(.0001 + fi/pow(10.0,floor(log10(fi))));
				if (!axis_is_pos_perc(fi, &subplace_cnt, 1e-6, subplaces)) {
					n = (int) floor(log10(fi)+0.5);
					if (format != NULL) {
						format->format(pow(10.0, n), ax->getNamePtr(i));
					} else {
						if (n == 0) {
							sprintf(cbuff, "1");
						} else if (n == 1) {
							sprintf(cbuff, "10");
						} else {
							const char* fmt = g_get_tex_labels() ? "$10^{%d}$" : "10^{%d}";
							sprintf(cbuff, fmt, n);
						}
						ax->setName(i, cbuff);
					}
				} else {
					if (ax->lgset == GLE_AXIS_LOG_25B && format != NULL) {
						format->format(fi, ax->getNamePtr(i));
					} else {
						n = (int) floor(0.0001 + fi/(pow(10.0,floor(log10(fi)))));
						double lsize = 0.7 * h * g_get_fconst(GLEC_ALABELSCALE);
						const char* fmt = g_get_tex_labels() ? "{\\sethei{%g}\\tex{%d}}" : "{\\sethei{%g}%d}";
						sprintf(cbuff, fmt, lsize, n);
						ax->setName(i, cbuff);
					}
				}
			}
		} else {
			if (ax->getNamesDataSet() != -1) {
				// Get axis labels from a data set
				ax->getLabelsFromDataSet(ax->getNamesDataSet());
			} else {
				for (int i = 0; i < ax->getNbPlaces(); i++) {
					ax->setName(i, g_format_label(ax->places[i], dticks, format));
				}
			}
		}
		if (format != NULL) delete format;
	}
	g_set_color(ax->label_color);
	g_set_line_width(ax->label_lwidth);
	g_set_line_style(ax->label_lstyle);
	g_set_font(ax->label_font);
	g_set_hei(h*g_get_fconst(GLEC_ALABELSCALE));
	double maxd = 0, maxh = 0, maxw = 0;
	GLEMeasureBox measure;
	measure.measureStart();
	// If there are no labels, use g_set_bounds to initialize "measure"
	double bl,br,bu,bd;
	init_measure_by_axis(ax, ox, oy, real_llen);
	if (!ax->label_off && drawrest) {
		int nb_names = ax->getNbNamedPlaces();
		for (int i = 0; i < nb_names; i++) {
			string name = ax->names[i];
			add_tex_labels(&name);
			g_measure(name, &bl, &br, &bu, &bd);
			if (bu > maxh) maxh = bu;
			if (-bd > maxd) maxd = -bd;
			if (br-bl > maxw) maxw = br-bl;
		}
		int place_cnt = 0;
		double angle = ax->getLabelAngle();
		for (int i = 0; i < nb_names; i++) {
			double fi = ax->places[i];
			string name = ax->names[i];
			add_tex_labels(&name);
			if (!ax->isNoPlaceLogOrReg(fi, &place_cnt, dticks) && name != "") {
				fi = fnAxisX(fi, ax);
				if (ax->log) fi = fnAxisX(ax->places[i], ax);
				g_measure(name, &bl, &br, &bu, &bd);
				switch (ax->type) {
					case GLE_AXIS_X:
					case GLE_AXIS_X0:
						if (angle == 0.0) {
							g_move(ox+fi+ax->shift, oy-llen-maxh);
							g_jtext(JUST_CENTRE);
						} else {
							g_gsave();
							g_move(ox+fi+ax->shift, oy-llen);
							g_rotate(angle);
							if (angle == 90.0) g_jtext(JUST_RC);
							else if (angle > 0.0) g_jtext(JUST_TR);
							else g_jtext(JUST_TL);
							g_grestore();
						}
						break;
					case GLE_AXIS_Y:
					case GLE_AXIS_Y0:
						if (angle == 0.0) {
							if (ax->label_align == JUST_RIGHT) {
								g_move(ox-llen, oy+fi+ax->shift);
								g_jtext(JUST_RC);
							} else {
								g_move(ox-llen-maxw, oy+fi+ax->shift);
								g_jtext(JUST_LC);
							}
						} else {
							g_gsave();
							if (angle == 90.0) {
								g_move(ox-llen-maxd, oy+fi+ax->shift);
								g_rotate(90);
								g_jtext(JUST_CENTRE);
							} else if (angle == -90.0) {
								g_move(ox-llen-maxh+maxd, oy+fi+ax->shift);
								g_rotate(-90);
								g_jtext(JUST_CENTRE);
							} else {
								g_move(ox-llen, oy+fi+ax->shift);
								g_rotate(angle);
								if (angle > 0.0) g_jtext(JUST_BR);
								else g_jtext(JUST_TR);
							}
							g_grestore();
						}
						break;
					case GLE_AXIS_X2:
						if (angle == 0.0) {
							g_move(ox+fi+ax->shift, oy+llen+maxd);
							g_jtext(JUST_CENTRE);
						} else {
							g_gsave();
							g_move(ox+fi+ax->shift, oy+llen);
							g_rotate(angle);
							if (angle == 90.0) g_jtext(JUST_LC);
							else if (angle > 0.0) g_jtext(JUST_BL);
							else g_jtext(JUST_BR);
							g_grestore();
						}
						break;
					case GLE_AXIS_Y2:
						if (angle == 0.0) {
							g_move(ox+llen, oy+fi+ax->shift);
							g_jtext(JUST_LC);
						} else {
							g_gsave();
							if (angle == 90.0) {
								g_move(ox+llen+maxh-maxd, oy+fi+ax->shift);
								g_rotate(90);
								g_jtext(JUST_CENTRE);
							} else if (angle == -90.0) {
								g_move(ox+llen, oy+fi+ax->shift);
								g_rotate(-90);
								g_jtext(JUST_CENTRE);
							} else {
								g_move(ox+llen, oy+fi+ax->shift);
								g_rotate(angle);
								if (angle > 0.0) g_jtext(JUST_TL);
								else g_jtext(JUST_BL);
							}
							g_grestore();
						}
						break;
				}
			}
		}
	}
	measure.measureEnd();
	// g_box_stroke(measure.getX1(), measure.getY1(), measure.getX2(), measure.getY2(), false);

/*---------------------------------- Now the axis title. */

	if (ax->title_off || ax->title == "" || !drawrest) {
		g_update_bounds_box(box);
		return;
	}
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		draw_axis_titles_v35(ax, h, ox, oy, dticks, llen);
	} else {
		draw_axis_titles(ax, h, ox, oy, &measure);
	}
	g_update_bounds_box(box);
}

void init_measure_by_axis(GLEAxis* ax, double ox, double oy, double llen) {
	// If there are no labels, use g_set_bounds to initialize "measure"
	switch (ax->type) {
		case GLE_AXIS_X:
		case GLE_AXIS_X0:
			g_update_bounds(ox+ax->shift, oy-llen);
			break;
		case GLE_AXIS_Y:
		case GLE_AXIS_Y0:
			g_update_bounds(ox-llen, oy+ax->shift);
			break;
		case GLE_AXIS_X2:
		case GLE_AXIS_T:
			g_update_bounds(ox+ax->shift, oy+llen);
			break;
		case GLE_AXIS_Y2:
			g_update_bounds(ox+llen, oy+ax->shift);
			break;
	}
}

void draw_axis_titles(GLEAxis *ax, double h, double ox, double oy, GLEMeasureBox* measure) {
	double bl,br,bu,bd;
	g_gsave();
	double th = h * g_get_fconst(GLEC_ATITLESCALE);
	if (ax->title_scale != 0) th = th*ax->title_scale;
	if (ax->title_hei != 0) th = ax->title_hei;
	g_set_color(ax->title_color);
	g_set_font(ax->title_font);
	g_set_hei(th);
	double tdist = ax->title_dist;
	if (ax->title_adist >= 0.0) {
		// absolute distance from axis side
		// update measure without taking into account labels or ticks!
		measure->measureStart();
		init_measure_by_axis(ax, ox, oy, 0.0);
		measure->measureEndIgnore();
		tdist = ax->title_adist;
		// also align to baseline and not to bottom of text
		ax->alignBase = true;
	}
	if (tdist == 0) tdist = h * g_get_fconst(GLEC_ATITLEDIST);
	string title_str(ax->title);
	add_tex_labels(&title_str);
	g_measure(title_str,&bl,&br,&bu,&bd);
	switch (ax->type) {
		case GLE_AXIS_X:
		case GLE_AXIS_X0:
			g_move(ox+ax->length/2, measure->getYMin() - tdist);
			g_jtext(JUST_TC);
			break;
		case GLE_AXIS_Y:
		case GLE_AXIS_Y0:
			g_move(measure->getXMin() - tdist,oy + ax->length/2);
			g_rotate(90.0);
			if (ax->isAlignBase()) g_jtext(JUST_CENTER);
			else g_jtext(JUST_BC);
			break;
		case GLE_AXIS_X2:
		case GLE_AXIS_T:
			g_move(ox+ax->length/2, measure->getYMax() + tdist);
			// *should* be center - otherwise titles of different graphs don't align!
			if (ax->isAlignBase()) g_jtext(JUST_CENTER);
			else g_jtext(JUST_BC);
			break;
		case GLE_AXIS_Y2:
			g_move(measure->getXMax() + tdist,oy + ax->length/2);
			if (ax->title_rot) {
				g_rotate(-90.0);
				if (ax->isAlignBase()) g_jtext(JUST_CENTER);
				else g_jtext(JUST_BC);
			} else {
				g_rotate(90.0);
				g_jtext(JUST_TC);
			}
			break;
	}
	g_grestore();
}

void draw_axis_titles_v35(GLEAxis *ax, double h, double ox, double oy, double dticks, double llen) {
	double bl,br,bu,bd,x,y;
	double ty = 0;
	double tx = ox-.3*h;
	double maxd = 0;
	if (ax->type == GLE_AXIS_Y2) tx = ox + .3*h;
	if (ax->type == GLE_AXIS_X  || ax->type == GLE_AXIS_X0) ty = oy-llen-.3*h;
	if (ax->type == GLE_AXIS_X2 || ax->type == GLE_AXIS_T) ty = oy+llen;
	if (!ax->label_off) {
		int nb_names = ax->getNbNamedPlaces();
		for (int i = 0; i < nb_names; i++) {
			string name(ax->names[i]);
			add_tex_labels(&name);
			g_measure(name,&bl,&br,&bu,&bd);
			if (bd>maxd) maxd = bd;
		}
		int place_cnt = 0;
		for (int i = 0; i < nb_names; i++) {
			double fi = ax->places[i];
			string name = ax->names[i];
			add_tex_labels(&name);
			if (!ax->isNoPlaceLogOrReg(fi, &place_cnt, dticks) && name != "") {
				fi = fnAxisX(fi, ax);
				if (ax->log) fi = fnAxisX(ax->places[i], ax);
				g_measure(name, &bl, &br, &bu, &bd);
				switch (ax->type) {
					case GLE_AXIS_X:
					case GLE_AXIS_X0:
						y = oy-llen-bu+bd-.3*h;
						if (y<ty) ty = y;
						break;
					case GLE_AXIS_Y:
					case GLE_AXIS_Y0:
						x = ox-br+bl-llen-.3*h;
						if (x<tx) tx = x;
						break;
					case GLE_AXIS_X2:
					case GLE_AXIS_T:
						y = oy+llen+maxd+bu;
						if (y>ty) ty = y;
						break;
					case GLE_AXIS_Y2:
						x = ox+br-bl+llen+.3*h;
						if (x>tx) tx = x;
						break;
				}
				if (bd>maxd) maxd = bd;
			}
		}
	}
	g_gsave();
	double th = h * 1.3;
	if (ax->title_scale!=0) th = th*ax->title_scale;
	if (ax->title_hei!=0) th = ax->title_hei;
	g_set_color(ax->title_color);
	g_set_font(ax->title_font);
	g_set_hei(th);
	g_measure(ax->title,&bl,&br,&bu,&bd);
	switch (ax->type) {
		case GLE_AXIS_X:
		case GLE_AXIS_X0:
			g_move(ox+ax->length/2,ty - ax->title_dist);
			g_jtext(JUST_TC);
			break;
		case GLE_AXIS_Y:
		case GLE_AXIS_Y0:
			g_move(tx - ax->title_dist,oy + ax->length/2);
			g_rotate(90.0);
			g_jtext(JUST_BC);
			g_rotate(-90.0);
			break;
		case GLE_AXIS_X2:
		case GLE_AXIS_T:
			g_move(ox+ax->length/2,ty + ax->title_dist);
			g_jtext(JUST_BC);
			break;
		case GLE_AXIS_Y2: /*  y2axis   */
			g_move(tx + ax->title_dist,oy + ax->length/2);
			if (ax->title_rot) {
				g_rotate(-90.0);
				g_jtext(JUST_BC);
				g_rotate(90.0);
			} else {
				g_rotate(90.0);
				g_jtext(JUST_TC);
				g_rotate(-90.0);
			}
			break;
	}
	g_grestore();
}

void nice_log_ticks(double *start, double *last, double gmin, double gmax) throw (ParserError) {
	if (gmin <= 0 || gmax <= 0) {
		stringstream err;
		err << "illegal range for log axis: min = ";
		err << gmin << " max = " << gmax;
		g_throw_parser_error(err.str());
	}
	/* range in powers of 10 */
	/* add one extra power on both sides! */
	*start = floor(log10(gmin)-1e-6);
	if (equals_rel(gmin, pow(10.0, *start+1))) {
		(*start)++;
	}
	*last =  ceil(log10(gmax)+1e-6);
	if (equals_rel(gmax, pow(10.0, *last-1))) {
		(*last)--;
	}
}

double compute_dticks(GLERange* range) {
	if (range->getMax() <= range->getMin()) {
		return 0.0;
	}
	double delta = range->getMax() - range->getMin();
	double st = delta/10;
	double expnt = floor(log10(st));
	double n = st/pow(10.0, expnt);
	int ni = 1;
	if (n > 5) {
		ni = 10;
	} else if (n > 2) {
		ni = 5;
	} else if (n > 1) {
		ni = 2;
	} else {
		ni = 1;
	}
	return ni * pow(10.0, expnt);
}

bool auto_collapse_range(GLERange* range, double dticks) {
	// detect underflow in precision
	// because this could lead to infinite loop in axis drawing code
	double max_end = max(fabs(range->getMin()), fabs(range->getMax()));
	if (max_end == 0.0) {
		range->setMin(0.0); range->setMax(0.0);
		return true;
	}
	if (dticks / max_end < CUTILS_REL_PREC_FINE) {
		double mean = (range->getMin() + range->getMax())/2.0;
		range->setMin(mean); range->setMax(mean);
		return true;
	} else {
		return false;
	}
}

void nice_ticks(double *dticks, double *gmin, double *gmax, double *t1, double *tn, int minset, int maxset) {
/*
	double delta,st,expnt,n;
	int ni;
	delta = *gmax-*gmin;
	if (delta == 0) {
		gprint("Axis range error min=%g max=%g \n",*gmin,*gmax);
		*gmax = *gmin+10;
		delta = 10;
	}
	st = delta/10;
	expnt = floor(log10(st));
	n = st/pow(10.0,expnt);
	if (n>5)
		ni = 10;
	else if (n>2)
		ni = 5;
	else if (n>1)
		ni = 2;
	else
		ni = 1;
	if (*dticks==0) *dticks = ni * pow(10.0,expnt);


	if (*gmin - (delta/1000) <=  floor( *gmin/ *dticks) * *dticks)
		*t1 = *gmin;
	else
		*t1 = (floor(*gmin/ *dticks) * *dticks ) + *dticks;

	*tn = *gmax;
	if (  (floor(.000001+ *gmax / *dticks) * *dticks) <
	  (*gmax - (delta/1000) ) )
		*tn = floor(.00001 + *gmax/ *dticks ) * *dticks;
*/
	if (*gmax <= *gmin) {
		gprint("Axis range error min=%g max=%g \n",*gmin,*gmax);
		*gmax = *gmin + 10;
	}
	GLERange range;
	range.setMinMax(*gmin, *gmax);
	if (*dticks == 0.0) {
		// no dticks parameter given for axis
		*dticks = compute_dticks(&range);
	}
	// Round range to nearest dtick
	range.setMax(ceil(range.getMax()/(*dticks)) * (*dticks));
	range.setMin(floor(range.getMin()/(*dticks)) * (*dticks));
	// cout << "Range0 [" << *gmin << "," << *gmax << "] ticks ["<< range.getMin() << "," << range.getMax() << "] diff: " << ((*gmin)-range.getMin()) << endl;
	// Make sure labels are in axis range
	if ((*gmin)-range.getMin() > CUTILS_REL_PREC_FINE) range.setMin(range.getMin()+(*dticks));
	if (range.getMax()-(*gmax) > CUTILS_REL_PREC_FINE) range.setMax(range.getMax()-(*dticks));
	// cout << "Range1 [" << *gmin << "," << *gmax << "] ticks ["<< range.getMin() << "," << range.getMax() << "]" << endl;
	// Return labels
	*t1 = range.getMin();
	*tn = range.getMax();
}

void roundrange(GLERange* range, bool extend, bool tozero, double dticks) {
	if (range->getMax() <= range->getMin()) {
		return;
	}
	double delta = range->getMax() - range->getMin();
	// auto-extend axis towards zero
	if (tozero && range->getMin() > 0 && range->getMin() <= delta*0.2) {
		range->setMin(0.0);
	}
	if (tozero && range->getMax() < 0 && range->getMax() >= -delta*0.2) {
		range->setMax(0.0);
	}
	if (dticks == 0.0) {
		// no dticks parameter given for axis
		dticks = compute_dticks(range);
	}
	// detect underflow in precision
	if (auto_collapse_range(range, dticks)) {
		return;
	}
	// round range towards multiple of dticks
	if (equals_rel_fine(ceil(range->getMax()/dticks) * dticks, range->getMax())) {
		range->setMax(ceil(range->getMax()/dticks) * dticks);
		// "extend" is true only in bar plots - it makes sure that there is a tick of space
		// left above of the largest bar; otherwise the bar would extend to the x2axis
		if (extend && range->getMax() != 0.0) range->setMax(range->getMax() + dticks);
	} else {
		range->setMax(ceil(range->getMax()/dticks) * dticks);
	}
	if (equals_rel_fine(floor(range->getMin()/dticks) * dticks, range->getMin())) {
		range->setMin(floor(range->getMin()/dticks) * dticks);
		if (extend && range->getMin() != 0.0) range->setMin(range->getMin() - dticks);
	} else {
		range->setMin(floor(range->getMin()/dticks) * dticks);
	}
}

double start_subtick(double dsubticks, double dticks, GLEAxis* ax) {
	double firstTick;
	if (ax->getNbPlaces() > 0) {
		firstTick = ax->getPlace(0);
	} else {
		GLERange range;
		range.copy(ax->getRange());
		roundrange(&range, false, false, dticks);
		firstTick = range.getMin();
	}
	if (firstTick <= ax->getMin()) {
		return firstTick;
	} else {
		double n = ceil((firstTick - ax->getMin()) / dsubticks) + 1;
		return firstTick - n * dsubticks;
	}
}

void numtrime(char *o,char *s) {
	char *e, *f;
	strcpy(o, s);
	e = strchr(s, 'e');
	if (e == NULL) return;
	e--;
	for (; *e=='0'; e--);
	if (*e == '.') {
		e--;
	}
	f = strchr(s, 'e');
	strcpy(e + 1, f);
	strcpy(o, s);
}

void numtrim(char **d,char *s,double dticks) {
	char *o,*nonzero=0;
	if (*d==0)  *d = (char*) myallocz(20);
	o = *d;
	nonzero = 0;
	if (strchr(s,'e')!=NULL) {
		numtrime(o,s);
		return;
	}
	while (*s==' ' && *s!=0) s++;
	while (*s!=0) {
		*(o++) = *(s++);
		if (*s=='.') {
			nonzero = o-1;
			if (dticks!=floor(dticks)) nonzero = o+1;
			while (*s!=0) {
				*(o++) = *(s++);
				if ((*s!='0') && (*s!=0))
					if (o>nonzero) nonzero = o;
			}
		}
	}
	*(o++) = 0;
	if (nonzero!=NULL) *(nonzero+1) = 0;
}

bool axis_is_max(int axis) {
	return axis == GLE_AXIS_X2 || axis == GLE_AXIS_Y2;
}

bool axis_ticks_neg(int axis) {
	return axis == GLE_AXIS_X2 || axis == GLE_AXIS_Y2;
}

bool axis_horizontal(int axis) {
	return axis == GLE_AXIS_X0 || axis == GLE_AXIS_X || axis == GLE_AXIS_X2 || axis == GLE_AXIS_T;
}

int axis_get_orth(int axis, int which) {
	if (axis_horizontal(axis)) {
		switch (which) {
			case 0:  return GLE_AXIS_Y0;
			case 1:  return GLE_AXIS_Y;
			default: return GLE_AXIS_Y2;
		}
	} else {
		switch (which) {
			case 0:  return GLE_AXIS_X0;
			case 1:  return GLE_AXIS_X;
			default: return GLE_AXIS_X2;
		}
	}
}

int axis_type(const char *s) {
	if (str_ni_equals(s,"X0",2)) return GLE_AXIS_X0;
	if (str_ni_equals(s,"Y0",2)) return GLE_AXIS_Y0;
	if (str_ni_equals(s,"X2",2)) return GLE_AXIS_X2;
	if (str_ni_equals(s,"Y2",2)) return GLE_AXIS_Y2;
	if (str_ni_equals(s,"X",1))  return GLE_AXIS_X;
	if (str_ni_equals(s,"Y",1))  return GLE_AXIS_Y;
	return GLE_AXIS_ALL;
}

int axis_type_check(const char *s) throw (ParserError) {
	int type = axis_type(s);
	if (type == GLE_AXIS_ALL) {
		ostringstream err;
		err << "can't infer axis type (x, y, ...) from expression '" << s << "'; ";
		err << "try, e.g., 'x" << s << "'";
		g_throw_parser_error(err.str());
	}
	return type;
}

const char* axis_type_name(int type) {
	switch (type) {
		case GLE_AXIS_X: return "xaxis";
		case GLE_AXIS_Y: return "yaxis";
		case GLE_AXIS_X2: return "x2axis";
		case GLE_AXIS_Y2: return "y2axis";
		case GLE_AXIS_X0: return "x0axis";
		case GLE_AXIS_Y0: return "y0axis";
		default: return "unknown";
	}
}

/*
 * lower and upper quantile used for automatic yscaling.
 * the factors control the amount of extension of the yrange
 * under the lower quantile and above the upper quantile. with
 * lower quantile of 3/40=0.075 and upper quantile of
 * 37/40=0.925 a factor of 3/34=0.0882 would extend linearly,
 * meaning that the graph of a linear function (e. g.: f(x)=x)
 * would exactly fit. a slightly higher value looks fine for
 * many functions.
 */

GLEAxisQuantileScale::GLEAxisQuantileScale() {
	m_QLower = 0.075;
	m_QUpper = 0.925;
	m_QLowerFactor = 0.092;
	m_QUpperFactor = 0.092;
}

GLEAxisQuantileScale::~GLEAxisQuantileScale() {
}

GLEAxis::GLEAxis() : format() {
}

GLEAxis::~GLEAxis() {
}

void GLEAxis::init(int i) {
	format = ""; title = "";
	clearNoTicks();
	names.clear(); places.clear(); noplaces.clear();
	base = 0.0; length = 0.0; shift = 0.0;
	label_align = JUST_RIGHT;
	label_font = 0; label_hei = 0.0; label_scale = 0.0; label_dist = 0.0;
	log = 0; nofirst = 0; nolast = 0;
	nticks = 0; nsubticks = 0; dticks = 0.0; dsubticks = 0.0;
	ticks_length = 0.0; ticks_scale = 0.0; ticks_lstyle[0] = 0;
	subticks_length = 0.0; subticks_scale = 0.0; subticks_lstyle[0] = 0;
	label_lstyle[0] = 0;
	off = 0;
	label_off = (i != GLE_AXIS_X && i != GLE_AXIS_Y);
	side_off = 0; ticks_off = 0; subticks_off = 0;
	side_lstyle[0] = 0;
	title_font = 0; title_dist = 0.0; title_adist = -1.0; title_hei = 0.0; title_scale = 0.0;
	title_rot = 0; title_off = 0; title_color = 0;
	negate = 0;
	names_ds = -1;
	label_angle = 0.0;
	has_subticks_onoff = false;
	has_label_onoff = false;
	setColor(g_get_color());
	side_lwidth = -1; ticks_lwidth = -1; subticks_lwidth = -1; label_lwidth = -1;
	lgset = GLE_AXIS_LOG_DEFAULT;
	has_ftick = false; ftick = 0.0;
	has_offset = false; offset = 0.0;
	ticks_both = false; grid = false; gridtop = false;
	roundRange = true;
	type = i;
	getRange()->initRange();
	getRange()->resetSet();
	m_QuantileScale.clear();
	if (g_get_compatibility() <= GLE_COMPAT_35) {
		alignBase = false;
	} else {
		if (i == GLE_AXIS_Y || i == GLE_AXIS_Y0 || i == GLE_AXIS_Y2) alignBase = false;
		else alignBase = true;
	}
}

string* GLEAxis::getNamePtr(int i) {
	while ((int)names.size() <= i) names.push_back(string());
	return &names[i];
}

void GLEAxis::setName(int i, const std::string& name) {
	while ((int)names.size() <= i) names.push_back(string());
	names[i] = name;
}

void GLEAxis::setPlace(int i, double place) {
	while ((int)places.size() <= i) places.push_back(0.0);
	places[i] = place;
}

void GLEAxis::insertNoTick(double pos, vector<double>& vec) {
	vector<double>::size_type at = 0;
	while (at < vec.size() && vec[at] < pos) {
		at++;
	}
	if (at == vec.size()) vec.push_back(pos);
	else vec.insert(vec.begin()+at, pos);
}

bool GLEAxis::isNoPlaceLogOrReg(double pos, int* place_cnt, double delta) {
	if (log) return axis_is_pos_perc(pos, place_cnt, 0.001, noplaces);
	else return axis_is_pos(pos, place_cnt, delta, noplaces);
}

void GLEAxis::addNoTick(double pos) {
	addNoTick1(pos);
	addNoTick2(pos);
}

void GLEAxis::insertNoTick(double pos) {
	insertNoTick1(pos);
	insertNoTick2(pos);
}

void GLEAxis::insertNoTickOrLabel(double pos) {
	insertNoTick1(pos);
	insertNoTick2(pos);
	insertNoPlace(pos);
}

void GLEAxis::clearNoTicks() {
	noticks1.clear();
	noticks2.clear();
}

void GLEAxis::printNoTicks() {
	cout << "Noticks1:";
	for (vector<double>::size_type i = 0; i < noticks1.size(); i++) {
		cout << " " << noticks1[i];
	}
	cout << endl;
	cout << "Noticks2:";
	for (vector<double>::size_type i = 0; i < noticks2.size(); i++) {
		cout << " " << noticks2[i];
	}
	cout << endl;
	cout << "NoPlaces:";
	for (vector<double>::size_type i = 0; i < noplaces.size(); i++) {
		cout << " " << noplaces[i];
	}
	cout << endl;
}

int GLEAxis::getNbNamedPlaces() {
	int nb = getNbNames();
	if (getNbPlaces() < nb) nb = getNbPlaces();
	return nb;
}

double GLEAxis::getLocalAveragePlacesDistance(int i) {
	int count = 0;
	double distance = 0.0;
	if (i > 0) {
		distance += fabs(places[i] - places[i - 1]);
		count ++;
	}
	if (i < getNbPlaces() - 1) {
		distance += fabs(places[i] - places[i + 1]);
		count ++;
	}
	if (count == 0) {
		return GLE_INF;
	} else {
		return distance / count;
	}
}

void GLEAxis::getLabelsFromDataSet(int ds) {
	GLEDataSet* dataSet = dp[ds];
	if (dataSet == NULL || dataSet->np == 0) {
		return;
	}
	GLEDataPairs data;
	GLEDataPairs::validate(dataSet, 2);
	data.copyDimension(dataSet, 0);
	GLEArrayImpl* yv = static_cast<GLEArrayImpl*>(dataSet->getData()->getObject(1));
	double* xt = data.getX();
	double min_val = xt[0];
	double max_val = xt[dataSet->np - 1];
	double half_spc = (max_val - min_val) / dataSet->np / 2.0;
	min_val -= half_spc;
	max_val += half_spc;
	unsigned int crpos = 0;
	for (int i = 0; i < getNbPlaces(); i++) {
		double fi = places[i];
		*getNamePtr(i) = "";
		if (fi >= min_val && fi <= max_val) {
			// find last position with x-value smaller than fi
			while (crpos < dataSet->np && xt[crpos] < fi) {
				crpos++;
			}
			if (crpos < dataSet->np && crpos >= 0) {
				if (crpos > 0) crpos--;
				unsigned int sel = crpos;
				double dist = fabs(xt[crpos] - fi);
				if (crpos+1 < dataSet->np) {
					if (fabs(xt[crpos+1] - fi) < dist) sel = crpos+1;
				}
				if (crpos >= 1) {
					if (fabs(xt[crpos-1] - fi) < dist) sel = crpos-1;
				}
				if (sel >= 0 && sel < dataSet->np && !data.getM(sel)) {
					bool showLabel = true;
					dist = fabs(xt[sel] - fi);
					if (!log && dist > getLocalAveragePlacesDistance(i) / 2.0) {
						showLabel = false;
					}
					if (showLabel) {
						GLERC<GLEString> str(yv->getString(sel));
						*getNamePtr(i) = str->toUTF8();
					}
				}
			}
		}
	}
}

void GLEAxis::initRange() {
	getDataRange()->initRange();
	getRange()->initRangeIfNotSet();
}

void GLEAxis::performRoundRange(GLERange* range, bool extend, bool tozero) {
	if (range->getMax() <= range->getMin()) {
		return;
	}
	if (!log) {
		if (roundRange) {
			roundrange(range, extend, tozero, dticks);
		} else {
			double dt = compute_dticks(range);
			auto_collapse_range(range, dt);
		}
	}
}

void GLEAxis::roundDataRange(bool extend, bool tozero) {
	if (getRange()->hasBoth()) {
		return;
	}
	GLERange* drange = getDataRange();
	performRoundRange(drange, extend, tozero);
	drange->copyHas(getRange());
	getRange()->copyIfNotSet(drange);
}

void GLEAxis::makeUpRange(GLEAxis* copy, GLEAxis* orth, bool extend, bool tozero) {
	if (getRange()->hasBoth()) {
		return;
	}
	GLERange* drange = getDataRange();
	if (drange->getWidth() == 0.0) {
		// No spread in values
		double x = drange->getMin();
		GLERange* orth_range = orth->getRange();
		if (!log && !orth->log && orth_range->validNotEmpty()) {
			// Set range based on orthogonal axis
			drange->setMin(x - orth_range->getWidth()/2);
			drange->setMax(x + orth_range->getWidth()/2);
			performRoundRange(drange, extend, tozero);
		}
		if (drange->getWidth() == 0.0) {
			// Previous approach was not applicable or did not help
			if (log) {
				// Set 3 log steps
				drange->setMinMax(x/10, x*10);
			} else {
				// Range [0..2x] or [-2x..0]
				double xa = fabs(x);
				if (xa == 0) drange->setMinMax(-1, 1);
				else drange->setMinMax(x - xa, x + xa);
				performRoundRange(drange, extend, tozero);
			}
		}
	}
	if (drange->invalidOrEmpty()) {
		// Note: if both ends found and invalid then error in range specification
		// this should throw error later, so ignore that case
		if (!drange->isMinValid() && !drange->isMaxValid()) {
			// Both ends of axis not found, copy range or use default
			GLERange* copy_range = copy->getRange();
			if (copy_range->validNotEmpty()) {
				drange->copy(copy_range);
				// Copy not only range, but also places (if given)
				if (getNbPlaces() == 0 && copy->getNbPlaces() > 0) {
					for (int i = 0; i < copy->getNbPlaces(); i++) {
						addPlace(copy->getPlace(i));
					}
				}
			} else {
				if (log) drange->setMinMax(1, 1e3);
				else drange->setMinMax(0, 1);
			}
		} else if (drange->isMinValid()) {
			if (log) {
				// Set 3 log steps
				drange->setMax(drange->getMin()*100);
			} else {
				// Range [min..0], [0..1], or [v..10v]
				if (drange->getMin() < 0) drange->setMax(0);
				else if (drange->getMin() == 0) drange->setMax(1);
				else drange->setMax(drange->getMin()*10);
				performRoundRange(drange, extend, tozero);
			}
		} else if (drange->isMaxValid()) {
			if (log) {
				// Set 3 log steps
				drange->setMin(drange->getMax()/100);
			} else {
				// Range [0..max], [-1..0], or [-10v..-v]
				if (drange->getMax() > 0) drange->setMin(0);
				else if (drange->getMax() == 0) drange->setMin(-1);
				else drange->setMin(drange->getMax()*10);
				performRoundRange(drange, extend, tozero);
			}
		}
	}
	getRange()->copyIfNotSet(drange);
}


void GLEAxis::setColor(const GLERC<GLEColor>& color) {
	side_color = color;
	ticks_color = color;
	label_color = color;
	subticks_color = color;
}
