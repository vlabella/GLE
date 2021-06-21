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
#include "cutils.h"
#include "keyword.h"

/*------------------------------------------------------------------*/
/*      Find the KEY WORD *cp, and return it's number               */
/*------------------------------------------------------------------*/
/*                                                                  */
/* You can add a function, you MUST place it in alphabetical order  */
/* and give it the next unused index number                         */
/*            *** 50 is RESERVED for assignment ***                 */
/* when adding you need to edit pass.cpp and run.cpp                */
/* CURRENT MAX IS 89 next command number is 90                      */
/*------------------------------------------------------------------*/

#define NKEYS (sizeof mkeywfn / sizeof(struct mkeyw))
struct mkeyw { const char *word; int index;  } mkeywfn[] = {
	{ "!",                           GLE_KW_COMMENT                 },
	{ "ABOUND",                      GLE_KW_ABOUND                  },
	{ "ALINE",                       GLE_KW_ALINE                   },
	{ "AMOVE",                       GLE_KW_AMOVE                   },
	{ "ARC",                         GLE_KW_ARC                     },
	{ "ARCTO",                       GLE_KW_ARCTO                   },
	{ "ASETPOS",                     GLE_KW_ASETPOS                 },
	{ "Assignment",                  GLE_KW_ASSIGNMENT              },
	{ "BEGIN",                       GLE_KW_BEGIN                   },
	{ "BEZIER",                      GLE_KW_BEZIER                  },
	{ "BIGFILE",                     GLE_KW_BIGFILE                 },
	{ "BITMAP",                      GLE_KW_BITMAP                  },
	{ "BITMAP_INFO",                 GLE_KW_BITMAP_INFO             },
	{ "BOX",                         GLE_KW_BOX                     },
	{ "CALL",                        GLE_KW_CALL                    },
	{ "CIRCLE",                      GLE_KW_CIRCLE                  },
	{ "CLOSEPATH",                   GLE_KW_CLOSEPATH               },
	{ "COLORMAP",                    GLE_KW_COLORMAP                },
	{ "COMPATIBILITY",               GLE_KW_COMPATIBILITY           },
	{ "CURVE",                       GLE_KW_CURVE                   },
	{ "DECLARE",                     GLE_KW_DECLARESUB              },
	{ "DEFAULT",                     GLE_KW_DEFAULT                 },
	{ "DEFCOLOR",                    GLE_KW_DEFCOLOR                },
	{ "DEFINE",                      GLE_KW_DEFINE                  },
	{ "DEFMARKER",                   GLE_KW_DEFMARKER               },
	{ "DFONT",                       GLE_KW_DFONT                   },
	{ "DRAW",                        GLE_KW_DRAW                    },
	{ "ELLIPSE",                     GLE_KW_ELLIPSE                 },
	{ "ELLIPTICAL_ARC",              GLE_KW_ELLIPTICAL_ARC          },
	{ "ELLIPTICAL_NARC",             GLE_KW_ELLIPTICAL_NARC         },
	{ "ELSE",                        GLE_KW_ELSE                    },
	{ "END",                         GLE_KW_END                     },
	{ "FCLOSE",                      GLE_KW_FCLOSE                  },
	{ "FGETLINE",                    GLE_KW_FGETLINE                },
	{ "FILL",                        GLE_KW_FILL                    },
	{ "FOPEN",                       GLE_KW_FOPEN                   },
	{ "FOR",                         GLE_KW_FOR                     },
	{ "FREAD",                       GLE_KW_FREAD                   },
	{ "FREADLN",                     GLE_KW_FREADLN                 },
	{ "FTOKENIZER",                  GLE_KW_FTOKENIZER              },
	{ "FWRITE",                      GLE_KW_FWRITE                  },
	{ "FWRITELN",                    GLE_KW_FWRITELN                },
	{ "GOTO",                        GLE_KW_GOTO                    },
	{ "GRESTORE",                    GLE_KW_GRESTORE                },
	{ "GSAVE",                       GLE_KW_GSAVE                   },
	{ "ICON",                        GLE_KW_ICON                    },
	{ "IF",                          GLE_KW_IF                      },
	{ "INCLUDE",                     GLE_KW_INCLUDE                 },
	{ "INPUT",                       GLE_KW_INPUT                   },
	{ "JOIN",                        GLE_KW_JOIN                    },
	{ "MARGINS",                     GLE_KW_MARGINS                 },
	{ "MARKER",                      GLE_KW_MARKER                  },
	{ "MOVE",                        GLE_KW_MOVE                    },
	{ "NAME",                        GLE_KW_NAME                    },
	{ "NARC",                        GLE_KW_NARC                    },
	{ "NEWPATH",                     GLE_KW_NEWPATH                 },
	{ "NEXT",                        GLE_KW_NEXT                    },
	{ "ORIENTATION",                 GLE_KW_ORIENTATION             },
	{ "PAPERSIZE",                   GLE_KW_PAPERSIZE               },
	{ "PIE",                         GLE_KW_PIE                     },
	{ "POSTSCRIPT",                  GLE_KW_POSTSCRIPT              },
	{ "PRINT",                       GLE_KW_PRINT                   },
	{ "PSBBTWEAK",                   GLE_KW_PSBBTWEAK               },
	{ "PSCOMMENT",                   GLE_KW_PSCOMMENT               },
	{ "RBEZIER",                     GLE_KW_RBEZIER                 },
	{ "REGION",                      GLE_KW_REGION                  },
	{ "RESTOREDEFAULTS",             GLE_KW_RESTOREDEFAULTS         },
	{ "RETURN",                      GLE_KW_RETURN                  },
	{ "REVERSE",                     GLE_KW_REVERSE                 },
	{ "RLINE",                       GLE_KW_RLINE                   },
	{ "RMOVE",                       GLE_KW_RMOVE                   },
	{ "ROT",                         GLE_KW_ROT                     },
	{ "ROTATE",                      GLE_KW_ROTATE                  },
	{ "RSETPOS",                     GLE_KW_RSETPOS                 },
	{ "SAVE",                        GLE_KW_SAVE                    },
	{ "SCALE",                       GLE_KW_SCALE                   },
	{ "SET",                         GLE_KW_SET                     },
	{ "SIZE",                        GLE_KW_SIZE                    },
	{ "SLEEP",                       GLE_KW_SLEEP                   },
	{ "STROKE",                      GLE_KW_STROKE                  },
	{ "SUB",                         GLE_KW_SUB                     },
	{ "TEX",                         GLE_KW_TEX                     },
	{ "TEXT",                        GLE_KW_TEXT                    },
	{ "TEXTDEF",                     GLE_KW_TEXTDEF                 },
	{ "TIFF",                        GLE_KW_TIFF                    },
	{ "TRAN",                        GLE_KW_TRAN                    },
	{ "TRANSLATE",                   GLE_KW_TRANSLATE               },
	{ "UNTIL",                       GLE_KW_UNTIL                   },
	{ "WHILE",                       GLE_KW_WHILE                   },
	{ "WRITE",                       GLE_KW_WRITE                   }
};

int binsearchk(const char *word, struct mkeyw tab[], int n);

void cmd_name(int idx, char **cp) {
	static char *kp;
	static char fail[]="Keyword not found";
	if (kp==NULL) kp = (char*) myallocz(80);
	for (unsigned int i=0;i<NKEYS;i++) {
		if (mkeywfn[i].index==idx) {
			strcpy(kp,mkeywfn[i].word);
			*cp = kp;
			return;
		}
	}
	*cp = &fail[0];
}

void find_mkey(string cp, int *idx) {
	if (cp.length() == 0 ) { *idx = 0; return;}
	int i;
	i = binsearchk(cp.c_str(),mkeywfn,NKEYS);
	/* printf("I %d, cmd {%s} \n",i,cp); */
	if (i==-1) { *idx = 0; return;}
	*idx = mkeywfn[i].index;
}

/*------------------------------------------------------------------*/
/* Simple binary search 					    */
/*------------------------------------------------------------------*/

int binsearchk(const char *word, struct mkeyw tab[], int n) {
	int cond,low,high,mid;
	low = 0;
	high = n-1;
	while (low <= high) {
		mid = (low+high) / 2;
		if ((cond = strcmp(word,tab[mid].word)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return mid;
	}
	return -1;
}


/*-------------------------------------------------------------------------*/
/* This is for the tex primitives */

#include "tex.h"
struct mkeyw tkeywfn[] = {
	{ "^",                  tp_sup          },
	{ "_",                  tp_sub          },
	{ "acccmb",             tp_acccmb       },
	{ "accent",             tp_accent       },
	{ "accentxy",           tp_accentxy     },
	{ "char",               tp_char         },
	{ "chardef",            tp_chardef      },
	{ "def",                tp_def          },
	{ "defbegin",           tp_defbegin     },
	{ "delcode",            tp_delcode      },
	{ "delimiter",          tp_delimiter    },
	{ "fontenc",            tp_fontenc      },
	{ "frac",               tp_frac         },
	{ "hfill",              tp_hfill        },
	{ "left",               tp_left         },
	{ "linegap",            tp_linegap      },
	{ "lineskip",           tp_lineskip     },
	{ "mathchar",           tp_mathchar     },
	{ "mathchardef",        tp_mathchardef  },
	{ "mathcode",           tp_mathcode     },
	{ "movexy",             tp_movexy       },
	{ "newline",            tp_newline      },
	{ "nolimits",           tp_nolimits     },
	{ "overbrace",          tp_overbrace    },
	{ "overline",           tp_overline     },
	{ "parskip",            tp_parskip      },
	{ "presave",            tp_presave      },
	{ "right",              tp_right        },
	{ "rule",               tp_rule         },
	{ "setfont",            tp_setfont      },
	{ "sethei",             tp_sethei       },
	{ "setstretch",         tp_setstretch   },
	{ "sfont",              tp_sfont        },
	{ "ssfont",             tp_ssfont       },
	{ "sub",                tp_sub          },
	{ "sup",                tp_sup          },
	{ "tex",                tp_tex          },
	{ "tfont",              tp_tfont        },
	{ "uchr",               tp_uchr         },
	{ "underbrace",         tp_underbrace   },
	{ "underline",          tp_underline    },
	{ "unicode",            tp_unicode      }
};

#define NTKEYS (sizeof tkeywfn / sizeof(struct mkeyw))
int find_primcmd(char *cp) {
	int i;
	i = binsearchk(cp,tkeywfn,NTKEYS);
	if (i==-1) return 0;
	return tkeywfn[i].index;
}

