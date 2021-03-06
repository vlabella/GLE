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

#ifndef INC_COLOR_H
#define INC_COLOR_H

typedef union {
	int l;
	unsigned char b[4];
} colortyp;

typedef struct {
	double red, green, blue;
} rgb01;

#ifdef WORDS_BIGENDIAN
	#define B_F 0
	#define B_R 1
	#define B_G 2
	#define B_B 3
#else
	#define B_F 3
	#define B_R 2
	#define B_G 1
	#define B_B 0
#endif

#define GLE_FILL_CLEAR  0XFF000000

// SVG colors
#define GLE_COLOR_ALICEBLUE 0x01F0F8FF
#define GLE_COLOR_ANTIQUEWHITE 0x01FAEBD7
#define GLE_COLOR_AQUA 0x0100FFFF
#define GLE_COLOR_AQUAMARINE 0x017FFFD4
#define GLE_COLOR_AZURE 0x01F0FFFF
#define GLE_COLOR_BEIGE 0x01F5F5DC
#define GLE_COLOR_BISQUE 0x01FFE4C4
#define GLE_COLOR_BLACK 0x01000000
#define GLE_COLOR_BLANCHEDALMOND 0x01FFEBCD
#define GLE_COLOR_BLUE 0x010000FF
#define GLE_COLOR_BLUEVIOLET 0x018A2BE2
#define GLE_COLOR_BROWN 0x01A52A2A
#define GLE_COLOR_BURLYWOOD 0x01DEB887
#define GLE_COLOR_CADETBLUE 0x015F9EA0
#define GLE_COLOR_CHARTREUSE 0x017FFF00
#define GLE_COLOR_CHOCOLATE 0x01D2691E
#define GLE_COLOR_CORAL 0x01FF7F50
#define GLE_COLOR_CORNFLOWERBLUE 0x016495ED
#define GLE_COLOR_CORNSILK 0x01FFF8DC
#define GLE_COLOR_CRIMSON 0x01DC143C
#define GLE_COLOR_CYAN 0x0100FFFF
#define GLE_COLOR_DARKBLUE 0x0100008B
#define GLE_COLOR_DARKCYAN 0x01008B8B
#define GLE_COLOR_DARKGOLDENROD 0x01B8860B
#define GLE_COLOR_DARKGRAY 0x01A9A9A9
#define GLE_COLOR_DARKGREEN 0x01006400
#define GLE_COLOR_DARKKHAKI 0x01BDB76B
#define GLE_COLOR_DARKMAGENTA 0x018B008B
#define GLE_COLOR_DARKOLIVEGREEN 0x01556B2F
#define GLE_COLOR_DARKORANGE 0x01FF8C00
#define GLE_COLOR_DARKORCHID 0x019932CC
#define GLE_COLOR_DARKRED 0x018B0000
#define GLE_COLOR_DARKSALMON 0x01E9967A
#define GLE_COLOR_DARKSEAGREEN 0x018FBC8F
#define GLE_COLOR_DARKSLATEBLUE 0x01483D8B
#define GLE_COLOR_DARKSLATEGRAY 0x012F4F4F
#define GLE_COLOR_DARKTURQUOISE 0x0100CED1
#define GLE_COLOR_DARKVIOLET 0x019400D3
#define GLE_COLOR_DEEPPINK 0x01FF1493
#define GLE_COLOR_DEEPSKYBLUE 0x0100BFFF
#define GLE_COLOR_DIMGRAY 0x01696969
#define GLE_COLOR_DODGERBLUE 0x011E90FF
#define GLE_COLOR_FIREBRICK 0x01B22222
#define GLE_COLOR_FLORALWHITE 0x01FFFAF0
#define GLE_COLOR_FORESTGREEN 0x01228B22
#define GLE_COLOR_FUCHSIA 0x01FF00FF
#define GLE_COLOR_GAINSBORO 0x01DCDCDC
#define GLE_COLOR_GHOSTWHITE 0x01F8F8FF
#define GLE_COLOR_GOLD 0x01FFD700
#define GLE_COLOR_GOLDENROD 0x01DAA520
#define GLE_COLOR_GRAY 0x01808080
#define GLE_COLOR_GREEN 0x01008000
#define GLE_COLOR_GREENYELLOW 0x01ADFF2F
#define GLE_COLOR_HONEYDEW 0x01F0FFF0
#define GLE_COLOR_HOTPINK 0x01FF69B4
#define GLE_COLOR_INDIANRED 0x01CD5C5C
#define GLE_COLOR_INDIGO 0x014B0082
#define GLE_COLOR_IVORY 0x01FFFFF0
#define GLE_COLOR_KHAKI 0x01F0E68C
#define GLE_COLOR_LAVENDER 0x01E6E6FA
#define GLE_COLOR_LAVENDERBLUSH 0x01FFF0F5
#define GLE_COLOR_LAWNGREEN 0x017CFC00
#define GLE_COLOR_LEMONCHIFFON 0x01FFFACD
#define GLE_COLOR_LIGHTBLUE 0x01ADD8E6
#define GLE_COLOR_LIGHTCORAL 0x01F08080
#define GLE_COLOR_LIGHTCYAN 0x01E0FFFF
#define GLE_COLOR_LIGHTGOLDENRODYELLOW 0x01FAFAD2
#define GLE_COLOR_LIGHTGRAY 0x01D3D3D3
#define GLE_COLOR_LIGHTGREEN 0x0190EE90
#define GLE_COLOR_LIGHTPINK 0x01FFB6C1
#define GLE_COLOR_LIGHTSALMON 0x01FFA07A
#define GLE_COLOR_LIGHTSEAGREEN 0x0120B2AA
#define GLE_COLOR_LIGHTSKYBLUE 0x0187CEFA
#define GLE_COLOR_LIGHTSLATEGRAY 0x01778899
#define GLE_COLOR_LIGHTSTEELBLUE 0x01B0C4DE
#define GLE_COLOR_LIGHTYELLOW 0x01FFFFE0
#define GLE_COLOR_LIME 0x0100FF00
#define GLE_COLOR_LIMEGREEN 0x0132CD32
#define GLE_COLOR_LINEN 0x01FAF0E6
#define GLE_COLOR_MAGENTA 0x01FF00FF
#define GLE_COLOR_MAROON 0x01800000
#define GLE_COLOR_MEDIUMAQUAMARINE 0x0166CDAA
#define GLE_COLOR_MEDIUMBLUE 0x010000CD
#define GLE_COLOR_MEDIUMORCHID 0x01BA55D3
#define GLE_COLOR_MEDIUMPURPLE 0x019370DB
#define GLE_COLOR_MEDIUMSEAGREEN 0x013CB371
#define GLE_COLOR_MEDIUMSLATEBLUE 0x017B68EE
#define GLE_COLOR_MEDIUMSPRINGGREEN 0x0100FA9A
#define GLE_COLOR_MEDIUMTURQUOISE 0x0148D1CC
#define GLE_COLOR_MEDIUMVIOLETRED 0x01C71585
#define GLE_COLOR_MIDNIGHTBLUE 0x01191970
#define GLE_COLOR_MINTCREAM 0x01F5FFFA
#define GLE_COLOR_MISTYROSE 0x01FFE4E1
#define GLE_COLOR_MOCCASIN 0x01FFE4B5
#define GLE_COLOR_NAVAJOWHITE 0x01FFDEAD
#define GLE_COLOR_NAVY 0x01000080
#define GLE_COLOR_OLDLACE 0x01FDF5E6
#define GLE_COLOR_OLIVE 0x01808000
#define GLE_COLOR_OLIVEDRAB 0x016B8E23
#define GLE_COLOR_ORANGE 0x01FFA500
#define GLE_COLOR_ORANGERED 0x01FF4500
#define GLE_COLOR_ORCHID 0x01DA70D6
#define GLE_COLOR_PALEGOLDENROD 0x01EEE8AA
#define GLE_COLOR_PALEGREEN 0x0198FB98
#define GLE_COLOR_PALETURQUOISE 0x01AFEEEE
#define GLE_COLOR_PALEVIOLETRED 0x01DB7093
#define GLE_COLOR_PAPAYAWHIP 0x01FFEFD5
#define GLE_COLOR_PEACHPUFF 0x01FFDAB9
#define GLE_COLOR_PERU 0x01CD853F
#define GLE_COLOR_PINK 0x01FFC0CB
#define GLE_COLOR_PLUM 0x01DDA0DD
#define GLE_COLOR_POWDERBLUE 0x01B0E0E6
#define GLE_COLOR_PURPLE 0x01800080
#define GLE_COLOR_RED 0x01FF0000
#define GLE_COLOR_ROSYBROWN 0x01BC8F8F
#define GLE_COLOR_ROYALBLUE 0x014169E1
#define GLE_COLOR_SADDLEBROWN 0x018B4513
#define GLE_COLOR_SALMON 0x01FA8072
#define GLE_COLOR_SANDYBROWN 0x01F4A460
#define GLE_COLOR_SEAGREEN 0x012E8B57
#define GLE_COLOR_SEASHELL 0x01FFF5EE
#define GLE_COLOR_SIENNA 0x01A0522D
#define GLE_COLOR_SILVER 0x01C0C0C0
#define GLE_COLOR_SKYBLUE 0x0187CEEB
#define GLE_COLOR_SLATEBLUE 0x016A5ACD
#define GLE_COLOR_SLATEGRAY 0x01708090
#define GLE_COLOR_SNOW 0x01FFFAFA
#define GLE_COLOR_SPRINGGREEN 0x0100FF7F
#define GLE_COLOR_STEELBLUE 0x014682B4
#define GLE_COLOR_TAN 0x01D2B48C
#define GLE_COLOR_TEAL 0x01008080
#define GLE_COLOR_THISTLE 0x01D8BFD8
#define GLE_COLOR_TOMATO 0x01FF6347
#define GLE_COLOR_TURQUOISE 0x0140E0D0
#define GLE_COLOR_VIOLET 0x01EE82EE
#define GLE_COLOR_WHEAT 0x01F5DEB3
#define GLE_COLOR_WHITE 0x01FFFFFF
#define GLE_COLOR_WHITESMOKE 0x01F5F5F5
#define GLE_COLOR_YELLOW 0x01FFFF00
#define GLE_COLOR_YELLOWGREEN 0x019ACD32

// Gray values
#define GLE_COLOR_GRAY1 0x01FDFDFD
#define GLE_COLOR_GRAY5 0x01F0F0F0
#define GLE_COLOR_GRAY10 0x01C8C8C8
#define GLE_COLOR_GRAY20 0x01AFAFAF
#define GLE_COLOR_GRAY30 0x01969696
#define GLE_COLOR_GRAY40 0x017D7D7D
#define GLE_COLOR_GRAY50 0x01646464
#define GLE_COLOR_GRAY60 0x014B4B4B
#define GLE_COLOR_GRAY70 0x01323232
#define GLE_COLOR_GRAY80 0x01191919
#define GLE_COLOR_GRAY90 0x01060606

#endif
