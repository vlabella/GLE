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

#include "all.h"
#include "edt.h"

#define WLEN 4
#define WEND 24
#define BCOLOR BLUE
#define FCOLOR MAGENTA

#include "../gle/file_io.h"

/* function prototypes */

/*
manip /com=a.a

        manip_refresh screen.
        edit cell mode,
        command mode.
        arrow keys scroll screen.

        data cell:range
        cell:range = expression
        expressions,           c,r,x,y,c1..cn,r1..rn,sum(range),
                        c(x-1,y) = c(x,y)

*/
/* global stuff */
int noscreenio;
int gle_debug;
int moving_x,moving_y;
int data_x;
int data_y;
int changed;
extern int islogging;
extern int isating;
/* local stuff */
int iserr,dont_clear;
int scr_frow=1,scr_fcol=1;
int scr_width=76,scr_colwidth=8;
int scr_ncol;
int scr_nrow=18;
int curx,cury;
int max_x=0,max_y=0;         /* change to 1,1 */

int scr_blackwhite;
int exit_manip;
char file_name[80];
int in_recover,single_step;
extern char strmiss[];

void init_gle_top(char **argv);

int main(int argc, char **argv) {
        int i;
        char ans[90];
        int cmd;
        static char atfile[80];

	init_gle_top(argv);
	manip_scr_init(NULL);

        if (strmiss[0]==0) strcpy(strmiss,"-");

        for (i=1;i<argc;i++) {
                gle_strupr( argv[i] );
                if (strncmp( argv[i] , "-RECOVER",2)==0) in_recover = true;
                else if (strncmp( argv[i] , "-STEP",3)==0) single_step = true ;
                else if (strncmp( argv[i] , "-SIZE",4)==0) {
                        data_setsize(atoi(argv[i+1]),atoi(argv[i+2]));
                        i+=2;
                }
                else if (strncmp( argv[i] , "-COMMANDS",2)==0) strcpy(atfile,argv[++i]);
                else if (strncmp( argv[i] , "-DEBUG",2)==0)                 ;
                else if (strncmp( argv[i] , "-FLOAT",2)==0)  set_usefloat();
                else if (strncmp( argv[i] , "-SINGLE",4)==0)  set_usefloat();
                else if (isalnum(*argv[i])) strcpy(file_name, argv[i]);
                else {
                        printf("Unrecognized parameter {%s} \n",argv[i]);
                        printf("Usage: MANIP infile.dat -recover -step -commands c.log -single -size x y\n");
                        exit(0);
                }
        }
        init_logging(file_name);


        if (atfile[0]!=0) at_open(atfile);
        gle_strlwr(file_name);
        if (strlen(file_name)>0) cmd_load(file_name,"",2);
xx1:
        if (strlen(file_name)==0) strcpy(file_name,"mtest.dat");
        set_colwidth(10);
        changed = false;
        for (;exit_manip==false;) {
                top_line();
                hi_cell(curx,cury);
                mjl_flush();
                read_command(&cmd,ans,"% ");
                if (cmd==eescape || cmd==equit) break;
                if (cmd==0)         do_command(ans);
                else if (cmd==eload) {
                        pick_file(ans,"*.dat");
                        manip_refresh();
                        clear_data();
                        cmd_load(ans,"",2);
                } else if (cmd==ehelp) {
                        do_help("MANIP","");
                        manip_refresh();
                } else if (cmd==esave) {
                        text_save();
                        manip_refresh();
                } else if (cmd==eload) {
                        manip_refresh();
                } else           do_arrow(cmd);
        }
        if (cmd==eescape) if (text_changed()) { exit_manip = false; goto xx1;}
        window_exit();
        if (islogging) log_close();
        scr_end();
	return 0;
}

bool text_changed() {
        int c;
        if (!changed) return false;
        for (;;) {
                fner("Save in {%s} ? (Y,N)",file_name);
                c = text_inkey();
                fner_clear();
                if (c==eescape) return true;
                if (tolower(c)=='n') return false;
                if (tolower(c)=='y') {text_save(); return false;}
        }
}

void text_save() {
        cmd_save(file_name,"","",2);
        changed = false;
}

void fix_cur() {
        if (curx<1) curx = 1;
        if (cury<1) cury = 1;
}

void shift_window() {
        int doit=false;
        fix_cur();
        if (curx<scr_fcol || curx >= scr_fcol + scr_ncol) {
                scr_fcol = curx - scr_ncol/2;
                if (scr_fcol<1) scr_fcol = 1;
                doit = true;
        }
        if (cury<scr_frow || cury >= scr_frow + scr_nrow) {
                scr_frow = cury - scr_nrow/2;
                if (scr_frow<1) scr_frow = 1;
                doit = true;
        }
        if (doit) manip_refresh();
}

void do_arrow(int k) {
        char buff[280];
        show_cellwide(curx,cury);
        moving_x = moving_y = 0;
        switch (k) {
        case eup:        cury--; moving_y = -1;  break;
        case edown:     cury++; moving_y = 1; break;
        case eright:        curx++; moving_x = 1; break;
        case eleft:        curx--; moving_x = -1; break;
        case edelline:        cury+=12; break;
        case esearch:         cury-=12; break;
        case ebigright: curx+=6; break;
        case ebigleft:         curx-=6; break;
        }
        fix_cur();
        sprintf(buff,"goto %d %d ",curx,cury);
        if (islogging) log_write(buff);
        shift_window();
        hi_cell(curx,cury);
}

void set_newxy(int x, int y) {
        show_cellwide(curx,cury);
        curx = x;
        cury = y;
        shift_window();
        hi_cell(curx,cury);
}

void window_norm() {
        window(1,1,80,25);
}

void window_exit() {
        window(1,1,80,25);
        gotoxy(1,25);
        scr_norm();
        printf("\n");clreol();
}

void set_colwidth(int n) {
        scr_colwidth = n;
        scr_ncol = scr_width/scr_colwidth;
        shift_window();
        manip_refresh();
}

void set_ncol(int n) {
        scr_ncol = n;
        scr_colwidth = scr_width/scr_ncol;
        shift_window();
        manip_refresh();
}

void top_line() {
        gotoxy(1,1); clreol();
        #ifdef MANIPCURPOS /* a.r.: to see, which cell is edited, if -DNOATTRIB */
        wprintf("Free=%ld  Current Cell( c = %d, r = %d ) MANIP 3.3  File={%s}"
                ,coreleft(),curx,cury,file_name);
        #else
        wprintf("Free=%ld  Used(%d,%d) MANIP 3.3    Current file={%s}"
                ,coreleft(),max_x,max_y,file_name);
        #endif
}

void manip_refresh() {
        int i,j;

        fix_cur();
        window_norm();
        scr_norm();
        clrscr();
        top_line();
        fner_clear();

        scr_menuhi();
        for (i=1;i<=scr_ncol;i++) {
                gotocell(i+scr_fcol-1,scr_frow-1);
                wprintf("c%d",i+scr_fcol-1);
        }
        for (i=1;i<=scr_nrow;i++) {
                gotocell(scr_fcol-1,i+scr_frow-1);
                wprintf("r%d",i+scr_frow-1);
        }
        scr_norm();

        for (j=0;j<scr_nrow;j++) {
         for (i=0;i<scr_ncol;i++) {
                show_cell(i+scr_fcol,j+scr_frow);
         }
        }
        iserr = true;
        fner_clear();
        hi_cell(curx,cury);
}

void hi_cell(int x,int y) {
        scr_menuhi();
        show_cellwide(x,y);
        scr_norm();
}

void gotocell(int x,int y) {
        if (x==scr_fcol-1) gotoxy( (x-scr_fcol+1)*scr_colwidth+1,(y-scr_frow)+3);
        else gotoxy( (x-scr_fcol)*scr_colwidth+6,(y-scr_frow)+3);
}

void show_ifcell(int x, int y) {
        if (x>=scr_fcol && x<(scr_fcol+scr_ncol)) {
        if (y>=scr_frow && y<(scr_frow+scr_nrow)) {
                show_cellwide(x,y);
        }
        }
}

void show_cell(int x, int y) {
        gotocell(x,y);
        cputs(scell(x,y));
}

void show_cellwide(int x, int y) {
        char buff[280];
        gotocell(x,y);
        strcpy(buff,scell(x,y));
        if ((int)strlen(buff)<(scr_colwidth-1))
                ncpy(buff+strlen(buff),"                ",
                        scr_colwidth-strlen(buff)-1);
        cputs(buff);
}

#if defined(__unix__) || defined(__APPLE__)
void wprintf_do(char *s) {
        //printw("%s",s);
        printf("%s",s);
}

void fner_do(char *output) {
        if (strchr(output,'\n')!=NULL) *strchr(output,'\n') = 0;
        fnerx(output);
}

void printmess_do(char *output) {
        if (strchr(output,'\n')!=NULL) *strchr(output,'\n') = 0;
        fnerxx(output);
}
#endif

/* Prints to the window */
#if defined(__unix__) || defined(__APPLE__)
void wprintf(const char* arg_list, ...) {
#else
void wprintf(va_list arg_list, ...) {
#endif
        va_list arg_ptr;
        char *format;
        char output[200];

         va_start(arg_ptr, arg_list);
         format = (char*)arg_list;
         vsprintf(output, format, arg_ptr);
#ifdef __TURBOC__
        printf(output);
#else
        cputs(output);
#endif
}

void d_tidyup() {
}

void gle_abort(const char *s) {
        fner("ABORT {%s}\n",s);
        exit(1);
}

void fner_clear(void) {
        if (iserr==false) return;
        if (dont_clear) return;
        scr_savexy();
        window_norm();
        gotoxy(1,25);
        scr_grey();
        clreol();
        gotoxy(2,25);
        cputs(function_bar());
        scr_norm();
        iserr = false;
        window_norm();
        scr_restorexy();
}

void fnerx(char *s) {
        if (dont_clear) return;
        scr_savexy();
        iserr = true;
        window_norm();
        gotoxy(1,25);
        scr_inv();
        clreol();
        gotoxy(2,25);
        cputs(s);
        scr_norm();
        scr_restorexy();
        scr_refresh();
}

void fnerxx(char *s) {
        scr_savexy();
        command_scroll_up();
        iserr = true;
        window_norm();
        gotoxy(1,23);
        scr_inv();
        clreol();
        gotoxy(2,23);
        cputs(s);
        scr_norm();
        scr_restorexy();
        scr_refresh();
}

int read_command(int *cmd,char *ans,const char *ques) {
        static char *lastline[22];
        int cl=0;
        int rr,i;
        int direc=0;

        if (isating) {
                if (!at_read(ans)) goto contxx;
                window(1,21,80,WEND);
                gotoxy(1,WLEN);
                scr_inv();
                clreol();
                gotoxy(2,WLEN);
                clreol(); *cmd = 0;
                cputs(ques); cputs(ans);  rr = false;
                goto xxend;
        }
contxx:;
        *ans = 0;
        iserr = true;
        window(1,21,80,WEND);
        gotoxy(1,WLEN);
        scr_inv();
        clreol();
xxxx:
       #ifndef EMXOS2  /* cause EMX has not really a function window()  a.r. */
        gotoxy(2,WLEN);
       #else
        gotoxy(1,24);
       #endif
        clreol();
        cputs(ques);
        rr = read_str(cmd,ans);
        if (*cmd == epageup) {
                if (direc==-1) cl++;
                if (lastline[cl]!=NULL) {
                        strcpy(ans,lastline[cl]);
                        cl++;
                }
                if (cl>18) cl = 0;
                direc = 1;
                goto xxxx;
        }
        if (*cmd == epagedown) {
                if (direc==1) cl--;
                cl--;
                direc = -1;
                if (cl<0) {cl = -1; *ans = 0; goto xxxx;}
                if (lastline[cl]!=NULL) strcpy(ans,lastline[cl]);
                goto xxxx;
        }
xxend:
        gotoxy(1,WLEN);
        if (strlen(ans)>0) {
                if (lastline[20]!=NULL) free(lastline[20]);
                for (i=20;i>0;i--) lastline[i] = lastline[i-1];
                lastline[0] = sdup(ans);
                command_scroll_up();
        }
        scr_norm();
#ifndef EMXOS2          /* a.r. */
        clreol();
#endif
        window_norm();
        return rr;
}

int read_str(int *cmd, char *s) {
        int c,cx=0;
        char mbuff[280];
        *cmd = 0;
        cputs(s); cx = strlen(s);
        for (;;) {
         c = text_inkey();
         if (iserr) fner_clear();
         if (strlen(s)==0) {
                switch (c) {
        case edelline:
        case esearch:
        case ebigright:
        case ebigleft:
                  case eup:
                  case edown:
                  case eleft:
                  case eright:
                  case equit:
                  case eescape:
                  case eload:
                  case esave:
                  case ehelp:
                     *cmd = c;
                     return false;
                }
         }
         switch (c) {
           case ebackline:
                c = epageup;
          case eload:
          case esave:
          case ehelp:
           case epagedown:
           case epageup:
                *cmd = c;
                return false;
           case eescape: /* ESCAPE */
                *cmd = c;
           case equit: /* control c */
                return true;
           case eleft: /* left */
                if (cx <= 0) break;
                cx--;
                scr_left(1);
                break;
           case eright: /* right */
                if (cx >= (int)strlen(s)) break;
                cx++;
                scr_right(1);
                break;
        case edelline:
        case esearch:
        case ebigright:
        case ebigleft:
           case eup: /* arrow up */
           case edown: /* arrow down */
                break;
          case ereturn: /* carriage return */
                return false;
          case edelete: /* delete */
                if (strlen(s)==0) break;
                if (cx<1) break;
                ncpy(mbuff,s,cx-1);
                strcat(mbuff,s + cx);
                strcpy(s,mbuff);
                cx--;
                scr_left(1);
                cputs(s + cx);
                putch(' ');
                scr_left(strlen(s+cx)+1);
                break;
          case eshowerror:
          case edrawit:
                break;
          default: /* normal key */
                if (c<26  && c!=9) {fner("Key has no affect"); break;}
                if (c>200)  fner("Unimplemented command");
                else {
                        ncpy(mbuff,s,cx);
                        mbuff[cx] = c; mbuff[cx+1] = 0;
                        strcat(mbuff,s + cx);
                        strcpy(s,mbuff);
                        cputs(s + cx);
                        cx++;
                        scr_left(strlen(s+cx));
                }
                break;
           }
         }
}

void command_scroll_up() {
        window(1,21,80,WEND);
        gotoxy(1,1);
        delline();
}

extern string GLE_TOP_DIR;

void init_gle_top(char** argv) {
	const char* top = getenv("GLE_TOP");
	if (top == NULL || top[0] == 0) {
		string exe_name;
		bool has_exe_name = GetExeName("manip", argv, exe_name);
		if (has_exe_name) {
			#ifdef GLETOP_CD
				GLE_TOP_DIR = exe_name;
				StripPathComponents(&GLE_TOP_DIR, GLETOP_CD+1);
				AddDirSep(GLE_TOP_DIR);
				GLE_TOP_DIR += GLETOP_REL;
				StripDirSep(GLE_TOP_DIR);
				string hlp_name = GLE_TOP_DIR + DIR_SEP + "manip.hlp";
				if (!GLEFileExists(hlp_name)) {
					GLE_TOP_DIR = exe_name;
					StripPathComponents(&GLE_TOP_DIR, 2);
				}
			#else
				GLE_TOP_DIR = exe_name;
				StripPathComponents(&GLE_TOP_DIR, 2);
			#endif
		} else {
			// The user will see as error message: "$GLE_TOP/some_file" not found.
			GLE_TOP_DIR = "$GLE_TOP";
		}
	} else {
		GLE_TOP_DIR = top;
	}
	StripDirSep(GLE_TOP_DIR);
}

char *gle_top() {
	return (char*)GLE_TOP_DIR.c_str();
}

