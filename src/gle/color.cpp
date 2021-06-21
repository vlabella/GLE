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

#include <fstream>

#include "all.h"
#include "cutils.h"
#include "gle-interface/gle-interface.h"
#include "color.h"

GLEColorList* g_ColorList = NULL;

GLEColorList* GLEGetColorList() {
	if (g_ColorList == NULL) {
		g_ColorList = new GLEColorList();
		g_ColorList->defineDefaultColors();
	}
	return g_ColorList;
}

GLEColorList::GLEColorList() {
}

GLEColorList::~GLEColorList() {
}

void GLEColorList::reset() {
	m_Colors.clear();
	m_ColorHash.clear();
	m_OldColors.clear();
	m_OldColorHash.clear();
	defineDefaultColors();
}

GLEColor* GLEColorList::get(const string& name) {
	int idx = m_ColorHash.try_get(name);
	if (idx != -1) {
		return m_Colors.get(idx);
	}
	int old_idx = m_OldColorHash.try_get(name);
	if (old_idx != -1) {
		return m_OldColors.get(old_idx);
	}
	return NULL;
}

GLEColor* GLEColorList::getSafeDefaultBlack(const string& name) {
	GLEColor* res = get(name);
	if (res == NULL) res = getColor(GLE_COLOR_BLACK);
	return res;
}

void GLEColorList::defineColor(const char* name, unsigned int value) {
	string key = name;
	defineColor(key, value);
}

void GLEColorList::defineColor(const string& name, unsigned int value) {
	GLEColor* color = new GLEColor();
	color->setHexValue(value);
	defineColor(name, color);
}

void GLEColorList::defineColor(const string& name, GLEColor* color) {
	color->setName(name);
	int idx = m_ColorHash.try_get(name);
	if (idx != -1) {
		m_Colors[idx] = color;
	} else {
		int new_pos = m_Colors.size();
		m_Colors.add(color);
		m_ColorHash.add_item(name, new_pos);
	}
}

void GLEColorList::defineOldColor(const char* name, unsigned int value) {
	string key = name;
	defineOldColor(key, value);
}

void GLEColorList::defineOldColor(const string& name, unsigned int value) {
	GLEColor* color = new GLEColor();
	color->setHexValue(value);
	color->setName(name);
	int idx = m_OldColorHash.try_get(name);
	if (idx != -1) {
		m_OldColors[idx] = color;
	} else {
		int new_pos = m_OldColors.size();
		m_OldColors.add(color);
		m_OldColorHash.add_item(name, new_pos);
	}
}

void GLEColorList::defineDefaultColors() {
	defineGrays();
	defineSVGColors();
	defineOldGLEColors();
}

void GLEColorList::defineGrays() {
	defineColor("GRAY1", GLE_COLOR_GRAY1);
	defineColor("GRAY5", GLE_COLOR_GRAY5);
	defineColor("GRAY10", GLE_COLOR_GRAY10);
	defineColor("GRAY20", GLE_COLOR_GRAY20);
	defineColor("GRAY30", GLE_COLOR_GRAY30);
	defineColor("GRAY40", GLE_COLOR_GRAY40);
	defineColor("GRAY50", GLE_COLOR_GRAY50);
	defineColor("GRAY60", GLE_COLOR_GRAY60);
	defineColor("GRAY70", GLE_COLOR_GRAY70);
	defineColor("GRAY80", GLE_COLOR_GRAY80);
	defineColor("GRAY90", GLE_COLOR_GRAY90);
}

void GLEColorList::defineSVGColors() {
	defineColor("ALICEBLUE", GLE_COLOR_ALICEBLUE);
	defineColor("ANTIQUEWHITE", GLE_COLOR_ANTIQUEWHITE);
	defineColor("AQUA", GLE_COLOR_AQUA);
	defineColor("AQUAMARINE", GLE_COLOR_AQUAMARINE);
	defineColor("AZURE", GLE_COLOR_AZURE);
	defineColor("BEIGE", GLE_COLOR_BEIGE);
	defineColor("BISQUE", GLE_COLOR_BISQUE);
	defineColor("BLACK", GLE_COLOR_BLACK);
	defineColor("BLANCHEDALMOND", GLE_COLOR_BLANCHEDALMOND);
	defineColor("BLUE", GLE_COLOR_BLUE);
	defineColor("BLUEVIOLET", GLE_COLOR_BLUEVIOLET);
	defineColor("BROWN", GLE_COLOR_BROWN);
	defineColor("BURLYWOOD", GLE_COLOR_BURLYWOOD);
	defineColor("CADETBLUE", GLE_COLOR_CADETBLUE);
	defineColor("CHARTREUSE", GLE_COLOR_CHARTREUSE);
	defineColor("CHOCOLATE", GLE_COLOR_CHOCOLATE);
	defineColor("CORAL", GLE_COLOR_CORAL);
	defineColor("CORNFLOWERBLUE", GLE_COLOR_CORNFLOWERBLUE);
	defineColor("CORNSILK", GLE_COLOR_CORNSILK);
	defineColor("CRIMSON", GLE_COLOR_CRIMSON);
	defineColor("CYAN", GLE_COLOR_CYAN);
	defineColor("DARKBLUE", GLE_COLOR_DARKBLUE);
	defineColor("DARKCYAN", GLE_COLOR_DARKCYAN);
	defineColor("DARKGOLDENROD", GLE_COLOR_DARKGOLDENROD);
	defineColor("DARKGRAY", GLE_COLOR_DARKGRAY);
	defineColor("DARKGREEN", GLE_COLOR_DARKGREEN);
	defineColor("DARKKHAKI", GLE_COLOR_DARKKHAKI);
	defineColor("DARKMAGENTA", GLE_COLOR_DARKMAGENTA);
	defineColor("DARKOLIVEGREEN", GLE_COLOR_DARKOLIVEGREEN);
	defineColor("DARKORANGE", GLE_COLOR_DARKORANGE);
	defineColor("DARKORCHID", GLE_COLOR_DARKORCHID);
	defineColor("DARKRED", GLE_COLOR_DARKRED);
	defineColor("DARKSALMON", GLE_COLOR_DARKSALMON);
	defineColor("DARKSEAGREEN", GLE_COLOR_DARKSEAGREEN);
	defineColor("DARKSLATEBLUE", GLE_COLOR_DARKSLATEBLUE);
	defineColor("DARKSLATEGRAY", GLE_COLOR_DARKSLATEGRAY);
	defineColor("DARKTURQUOISE", GLE_COLOR_DARKTURQUOISE);
	defineColor("DARKVIOLET", GLE_COLOR_DARKVIOLET);
	defineColor("DEEPPINK", GLE_COLOR_DEEPPINK);
	defineColor("DEEPSKYBLUE", GLE_COLOR_DEEPSKYBLUE);
	defineColor("DIMGRAY", GLE_COLOR_DIMGRAY);
	defineColor("DODGERBLUE", GLE_COLOR_DODGERBLUE);
	defineColor("FIREBRICK", GLE_COLOR_FIREBRICK);
	defineColor("FLORALWHITE", GLE_COLOR_FLORALWHITE);
	defineColor("FORESTGREEN", GLE_COLOR_FORESTGREEN);
	defineColor("FUCHSIA", GLE_COLOR_FUCHSIA);
	defineColor("GAINSBORO", GLE_COLOR_GAINSBORO);
	defineColor("GHOSTWHITE", GLE_COLOR_GHOSTWHITE);
	defineColor("GOLD", GLE_COLOR_GOLD);
	defineColor("GOLDENROD", GLE_COLOR_GOLDENROD);
	defineColor("GRAY", GLE_COLOR_GRAY);
	defineColor("GREEN", GLE_COLOR_GREEN);
	defineColor("GREENYELLOW", GLE_COLOR_GREENYELLOW);
	defineColor("HONEYDEW", GLE_COLOR_HONEYDEW);
	defineColor("HOTPINK", GLE_COLOR_HOTPINK);
	defineColor("INDIANRED", GLE_COLOR_INDIANRED);
	defineColor("INDIGO", GLE_COLOR_INDIGO);
	defineColor("IVORY", GLE_COLOR_IVORY);
	defineColor("KHAKI", GLE_COLOR_KHAKI);
	defineColor("LAVENDER", GLE_COLOR_LAVENDER);
	defineColor("LAVENDERBLUSH", GLE_COLOR_LAVENDERBLUSH);
	defineColor("LAWNGREEN", GLE_COLOR_LAWNGREEN);
	defineColor("LEMONCHIFFON", GLE_COLOR_LEMONCHIFFON);
	defineColor("LIGHTBLUE", GLE_COLOR_LIGHTBLUE);
	defineColor("LIGHTCORAL", GLE_COLOR_LIGHTCORAL);
	defineColor("LIGHTCYAN", GLE_COLOR_LIGHTCYAN);
	defineColor("LIGHTGOLDENRODYELLOW", GLE_COLOR_LIGHTGOLDENRODYELLOW);
	defineColor("LIGHTGRAY", GLE_COLOR_LIGHTGRAY);
	defineColor("LIGHTGREEN", GLE_COLOR_LIGHTGREEN);
	defineColor("LIGHTPINK", GLE_COLOR_LIGHTPINK);
	defineColor("LIGHTSALMON", GLE_COLOR_LIGHTSALMON);
	defineColor("LIGHTSEAGREEN", GLE_COLOR_LIGHTSEAGREEN);
	defineColor("LIGHTSKYBLUE", GLE_COLOR_LIGHTSKYBLUE);
	defineColor("LIGHTSLATEGRAY", GLE_COLOR_LIGHTSLATEGRAY);
	defineColor("LIGHTSTEELBLUE", GLE_COLOR_LIGHTSTEELBLUE);
	defineColor("LIGHTYELLOW", GLE_COLOR_LIGHTYELLOW);
	defineColor("LIME", GLE_COLOR_LIME);
	defineColor("LIMEGREEN", GLE_COLOR_LIMEGREEN);
	defineColor("LINEN", GLE_COLOR_LINEN);
	defineColor("MAGENTA", GLE_COLOR_MAGENTA);
	defineColor("MAROON", GLE_COLOR_MAROON);
	defineColor("MEDIUMAQUAMARINE", GLE_COLOR_MEDIUMAQUAMARINE);
	defineColor("MEDIUMBLUE", GLE_COLOR_MEDIUMBLUE);
	defineColor("MEDIUMORCHID", GLE_COLOR_MEDIUMORCHID);
	defineColor("MEDIUMPURPLE", GLE_COLOR_MEDIUMPURPLE);
	defineColor("MEDIUMSEAGREEN", GLE_COLOR_MEDIUMSEAGREEN);
	defineColor("MEDIUMSLATEBLUE", GLE_COLOR_MEDIUMSLATEBLUE);
	defineColor("MEDIUMSPRINGGREEN", GLE_COLOR_MEDIUMSPRINGGREEN);
	defineColor("MEDIUMTURQUOISE", GLE_COLOR_MEDIUMTURQUOISE);
	defineColor("MEDIUMVIOLETRED", GLE_COLOR_MEDIUMVIOLETRED);
	defineColor("MIDNIGHTBLUE", GLE_COLOR_MIDNIGHTBLUE);
	defineColor("MINTCREAM", GLE_COLOR_MINTCREAM);
	defineColor("MISTYROSE", GLE_COLOR_MISTYROSE);
	defineColor("MOCCASIN", GLE_COLOR_MOCCASIN);
	defineColor("NAVAJOWHITE", GLE_COLOR_NAVAJOWHITE);
	defineColor("NAVY", GLE_COLOR_NAVY);
	defineColor("OLDLACE", GLE_COLOR_OLDLACE);
	defineColor("OLIVE", GLE_COLOR_OLIVE);
	defineColor("OLIVEDRAB", GLE_COLOR_OLIVEDRAB);
	defineColor("ORANGE", GLE_COLOR_ORANGE);
	defineColor("ORANGERED", GLE_COLOR_ORANGERED);
	defineColor("ORCHID", GLE_COLOR_ORCHID);
	defineColor("PALEGOLDENROD", GLE_COLOR_PALEGOLDENROD);
	defineColor("PALEGREEN", GLE_COLOR_PALEGREEN);
	defineColor("PALETURQUOISE", GLE_COLOR_PALETURQUOISE);
	defineColor("PALEVIOLETRED", GLE_COLOR_PALEVIOLETRED);
	defineColor("PAPAYAWHIP", GLE_COLOR_PAPAYAWHIP);
	defineColor("PEACHPUFF", GLE_COLOR_PEACHPUFF);
	defineColor("PERU", GLE_COLOR_PERU);
	defineColor("PINK", GLE_COLOR_PINK);
	defineColor("PLUM", GLE_COLOR_PLUM);
	defineColor("POWDERBLUE", GLE_COLOR_POWDERBLUE);
	defineColor("PURPLE", GLE_COLOR_PURPLE);
	defineColor("RED", GLE_COLOR_RED);
	defineColor("ROSYBROWN", GLE_COLOR_ROSYBROWN);
	defineColor("ROYALBLUE", GLE_COLOR_ROYALBLUE);
	defineColor("SADDLEBROWN", GLE_COLOR_SADDLEBROWN);
	defineColor("SALMON", GLE_COLOR_SALMON);
	defineColor("SANDYBROWN", GLE_COLOR_SANDYBROWN);
	defineColor("SEAGREEN", GLE_COLOR_SEAGREEN);
	defineColor("SEASHELL", GLE_COLOR_SEASHELL);
	defineColor("SIENNA", GLE_COLOR_SIENNA);
	defineColor("SILVER", GLE_COLOR_SILVER);
	defineColor("SKYBLUE", GLE_COLOR_SKYBLUE);
	defineColor("SLATEBLUE", GLE_COLOR_SLATEBLUE);
	defineColor("SLATEGRAY", GLE_COLOR_SLATEGRAY);
	defineColor("SNOW", GLE_COLOR_SNOW);
	defineColor("SPRINGGREEN", GLE_COLOR_SPRINGGREEN);
	defineColor("STEELBLUE", GLE_COLOR_STEELBLUE);
	defineColor("TAN", GLE_COLOR_TAN);
	defineColor("TEAL", GLE_COLOR_TEAL);
	defineColor("THISTLE", GLE_COLOR_THISTLE);
	defineColor("TOMATO", GLE_COLOR_TOMATO);
	defineColor("TURQUOISE", GLE_COLOR_TURQUOISE);
	defineColor("VIOLET", GLE_COLOR_VIOLET);
	defineColor("WHEAT", GLE_COLOR_WHEAT);
	defineColor("WHITE", GLE_COLOR_WHITE);
	defineColor("WHITESMOKE", GLE_COLOR_WHITESMOKE);
	defineColor("YELLOW", GLE_COLOR_YELLOW);
	defineColor("YELLOWGREEN", GLE_COLOR_YELLOWGREEN);
}

void GLEColorList::defineOldGLEColors() {
	defineOldColor("BAKERS_CHOCOLATE", 0x5C3317);
	defineOldColor("BLUE_VIOLET", 0x9F5F9F);
	defineOldColor("BRASS", 0xB5A642);
	defineOldColor("BRIGHT_GOLD", 0xD9D919);
	defineOldColor("BRONZE", 0x8C7853);
	defineOldColor("BRONZE_II", 0xA67D3D);
	defineOldColor("BROWN_WEB", 0xA62A2A);
	defineOldColor("CADET_BLUE", 0x5F9F9F);
	defineOldColor("COOL_COPPER", 0xD98719);
	defineOldColor("COPPER", 0xB87333);
	defineOldColor("CORN_FLOWER_BLUE", 0x42426F);
	defineOldColor("DARK_BROWN", 0x5C4033);
	defineOldColor("DARK_GREEN", 0x2F4F2F);
	defineOldColor("DARK_GREEN_COPPER", 0x4A766E);
	defineOldColor("DARK_OLIVE_GREEN", 0x4F4F2F);
	defineOldColor("DARK_ORCHID", 0x9932CD);
	defineOldColor("DARK_PURPLE", 0x871F78);
	defineOldColor("DARK_SLATE_BLUE", 0x6B238E);
	defineOldColor("DARK_SLATE_GRAY", 0x2F4F4F);
	defineOldColor("DARK_SLATE_GREY", 0x2F4F4F);
	defineOldColor("DARK_TAN", 0x97694F);
	defineOldColor("DARK_TURQUOISE", 0x7093DB);
	defineOldColor("DARK_WOOD", 0x855E42);
	defineOldColor("DIM_GRAY", 0x545454);
	defineOldColor("DIM_GREY", 0x545454);
	defineOldColor("DUSTY_ROSE", 0x856363);
	defineOldColor("FELDSPAR", 0xD19275);
	defineOldColor("FOREST_GREEN", 0x228B22);
	defineOldColor("FOREST_GREEN_WEB", 0x238E23);
	defineOldColor("GRAY_WEB", 0xC0C0C0);
	defineOldColor("GREEN_COPPER", 0x527F76);
	defineOldColor("GREEN_YELLOW", 0x93DB70);
	defineOldColor("GREY", 0x7F7F7F);
	defineOldColor("GREY1", 0xFDFDFD);
	defineOldColor("GREY10", 0xC8C8C8);
	defineOldColor("GREY20", 0xAFAFAF);
	defineOldColor("GREY30", 0x969696);
	defineOldColor("GREY40", 0x7D7D7D);
	defineOldColor("GREY5", 0xF0F0F0);
	defineOldColor("GREY50", 0x646464);
	defineOldColor("GREY60", 0x4B4B4B);
	defineOldColor("GREY70", 0x323232);
	defineOldColor("GREY80", 0x191919);
	defineOldColor("GREY90", 0x060606);
	defineOldColor("GREY_WEB", 0xC0C0C0);
	defineOldColor("HUNTER_GREEN", 0x215E21);
	defineOldColor("INDIAN_RED", 0x4E2F2F);
	defineOldColor("LAWN_GREEN", 0x7CFC00);
	defineOldColor("LIGHT_BLUE", 0xC0D9D9);
	defineOldColor("LIGHT_GRAY", 0xA8A8A8);
	defineOldColor("LIGHT_GREY", 0xA8A8A8);
	defineOldColor("LIGHT_STEEL_BLUE", 0x8F8FBD);
	defineOldColor("LIGHT_WOOD", 0xE9C2A6);
	defineOldColor("LIME_GREEN", 0x32CD32);
	defineOldColor("MANDARIAN_ORANGE", 0xE47833);
	defineOldColor("MAROON_WEB", 0x8E236B);
	defineOldColor("MEDIUM_AQUAMARINE", 0x32CD99);
	defineOldColor("MEDIUM_BLUE", 0x3232CD);
	defineOldColor("MEDIUM_FOREST_GREEN", 0x6B8E23);
	defineOldColor("MEDIUM_GOLDENROD", 0xEAEAAE);
	defineOldColor("MEDIUM_ORCHID", 0x9370DB);
	defineOldColor("MEDIUM_SEA_GREEN", 0x426F42);
	defineOldColor("MEDIUM_SLATE_BLUE", 0x7F00FF);
	defineOldColor("MEDIUM_SPRING_GREEN", 0x7FFF00);
	defineOldColor("MEDIUM_TURQUOISE", 0x70DBDB);
	defineOldColor("MEDIUM_VIOLET_RED", 0xDB7093);
	defineOldColor("MEDIUM_WOOD", 0xA68064);
	defineOldColor("MIDNIGHT_BLUE", 0x2F2F4F);
	defineOldColor("NAVY_BLUE", 0x23238E);
	defineOldColor("NEON_BLUE", 0x4D4DFF);
	defineOldColor("NEON_PINK", 0xFF6EC7);
	defineOldColor("NEW_MIDNIGHT_BLUE", 0x00009C);
	defineOldColor("NEW_TAN", 0xEBC79E);
	defineOldColor("OLD_GOLD", 0xCFB53B);
	defineOldColor("ORANGE_RED", 0xFF2400);
	defineOldColor("ORANGE_WEB", 0xFF7F00);
	defineOldColor("PALE_GREEN", 0x8FBC8F);
	defineOldColor("PINK_WEB", 0xBC8F8F);
	defineOldColor("QUARTZ", 0xD9D9F3);
	defineOldColor("RICH_BLUE", 0x5959AB);
	defineOldColor("SCARLET", 0x8C1717);
	defineOldColor("SEA_GREEN", 0x238E68);
	defineOldColor("SEMI_SWEET_CHOCOLATE", 0x6B4226);
	defineOldColor("SILVER_WEB", 0xE6E8FA);
	defineOldColor("SKY_BLUE", 0x3299CC);
	defineOldColor("SLATE_BLUE", 0x007FFF);
	defineOldColor("SPICY_PINK", 0xFF1CAE);
	defineOldColor("SPRING_GREEN", 0x00FF7F);
	defineOldColor("STEEL_BLUE", 0x236B8E);
	defineOldColor("SUMMER_SKY", 0x38B0DE);
	defineOldColor("TAN_COLOR", 0x008080);
	defineOldColor("VERY_DARK_BROWN", 0x5C4033);
	defineOldColor("VERY_LIGHT_GRAY", 0xCDCDCD);
	defineOldColor("VERY_LIGHT_GREY", 0xCDCDCD);
	defineOldColor("VIOLET_RED", 0xCC3299);
	defineOldColor("VIOLET_WEB", 0x4F2F4F);
	defineOldColor("YELLOW_GREEN", 0x99CC32);
}

GLERC<GLEColor> color_or_fill_from_int(int hexValue) {
	GLERC<GLEColor> result(new GLEColor());
	result->setHexValueGLE(hexValue);
	return result;
}

void update_color_foreground(GLEColor* updateMe, GLEColor* color) {
	updateMe->setRGBA(color->getRed(), color->getGreen(), color->getBlue(), color->getAlpha());
	updateMe->setTransparent(color->isTransparent());
	updateMe->setName(color->getNameS());
}

void update_color_foreground_and_pattern(GLEColor* updateMe, GLEColor* color) {
	update_color_foreground(updateMe, color);
	if (color->isFill() && color->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		update_color_fill_pattern(updateMe, static_cast<GLEPatternFill*>(color->getFill()));
	}
}

void update_color_fill_pattern(GLEColor* updateMe, GLEPatternFill* fill) {
	if (updateMe->isFill() && updateMe->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		GLEPatternFill* myFill = static_cast<GLEPatternFill*>(updateMe->getFill());
		myFill->setFillDescription(fill->getFillDescription());
	} else {
		updateMe->setFill(new GLEPatternFill(fill->getFillDescription()));
	}
	updateMe->setTransparent(false);
}

void update_color_fill_background(GLEColor* updateMe, GLEColor* color) {
	if (updateMe->isFill() && updateMe->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		GLEPatternFill* myFill = static_cast<GLEPatternFill*>(updateMe->getFill());
		myFill->setBackground(color);
	} else {
		GLEPatternFill* myFill = new GLEPatternFill(0X02010020);
		myFill->setBackground(color);
		updateMe->setFill(myFill);
	}
	updateMe->setTransparent(false);
}

GLERC<GLEColor> get_fill_background(GLEColor* fill) {
	if (fill->isFill() && fill->getFill()->getFillType() == GLE_FILL_TYPE_PATTERN) {
		GLEPatternFill* myFill = static_cast<GLEPatternFill*>(fill->getFill());
		return myFill->getBackground();
	} else {
		return color_or_fill_from_int(GLE_COLOR_WHITE);
	}
}

GLERC<GLEColor> get_fill_foreground(GLEColor* fill) {
	GLERC<GLEColor> result(new GLEColor());
	update_color_foreground(result.get(), fill);
	return result;
}
