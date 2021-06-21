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
#include "color.h"
#include "edt.h"

#define WHITE 15
#define BLACK 0
#define BEGINDEF extern
#define BCOLOR BLUE
#define FCOLOR MAGENTA
#define HBCOLOR h_bcolor
#define HVCOLOR h_fcolor
#define VCOLOR WHITE

int h_bcolor;
int h_fcolor;
int getsize(void);
char *tabtospace(char *s);
void printdash(int i);
int gle_redraw(void);
int text_showerror(void);
char *strfile(char *s);

int vdelete(int i);
int vinsert(int y, char *s);
void m_ifsend(char *s);
void m_sendline(void);
int minit_extras(void);
int minit_extras(void);
void initmenu(void);
int mystrlen(const char *s);
int pick_file(char *d, const char *w);
void setvstr(char **d, const char *s);
int text_inkey(void);

struct menu_struct { int x; int y; char *title; int width; int typ; int typ2;
        char *val; char *help; };
typedef struct menu_struct menutype;
struct pmenu_struct {int ci; menutype *menu; };
typedef struct pmenu_struct pmenutype;
pmenutype pmenu[17];
void setmenu(menutype *m,int x, int y, int typ, int typ2, int width, const char *title, const char *val, const char *help);
bool do_menu(pmenutype *p);
int extractmenu(void);
void add_unrecognized(char *s);
int fillinmenu(int nbegin);
int nunrec;

#define  PTXT 0
enum {MEND,MTEXT,MFILE,MTOGON,MTOGOFF,MSUB,MRETURN,MNULL};
enum {SNORM,SROL,SNEXTS,SNEXT,SSTR,STOG,SS,SSIZE,SNOBOX,SAMOVE,SSOFF,SSNULL};

void manip_refresh_menu(struct menu_struct *m);
void menu_hilight(struct menu_struct *m);
void menu_norm(struct menu_struct *m);

/* menu's
        topmenu
                title,xtitle,ytitle min max dist on off, ticks dticks
                (2) let d = exp
                Data files
                Dn marker ,color, lwidth, lstyle, err,
        xnamesmenu
        ynamesmenu
        fillmenu
        barmenu
        axismenu's
*/
int read_default(char *result, char *ques, char *dflt) {
        char s[200];
        int r = 0;
        strcpy(s,ques); strcat(s," ["); strcat(s,dflt); strcat(s,"] ");
/*        r = read_input(result,s);*/
        if (strlen(result)==0) strcpy(result,dflt);
        return r;
}
extern int changed;

void addline(char *s) {
        changed = true;
}

const char *mark_names[] = {
        "dot", "square", "fcircle", "club", "diamond", "triangle", "snake", "otimes", "odot", ""};

int m_gstart,m_gend;
/*

        Getkey/command
                arrowkeys normal
                left and right arrow if editing
                delete
                insert normal character
                enter, change to new menu
                escape exit.
                F1, help key
*/
char mbuff[255];

extern int iserr;
int hcx,cx,hcy;        /* HIlighted current x,y */

bool do_menu(pmenutype *ppmenu) {
        menutype *cm,*cmi,*si=NULL,*mi;
        int citem,emode=false;
        int c;
        cm = ppmenu->menu;
        citem = ppmenu->ci;
        cmi = cm+citem;
        manip_refresh_menu(cm);
        menu_hilight(cmi);
        for (;;) {
         c = text_inkey();
         if (iserr) {fner_clear(); window(1,1,80,24);}
         switch (c) {
           case eescape: /* ESCAPE */
           case equit: /* control c */
                return true;
           case eleft: /* left */
                if (emode) {
                        if (cx>0) cx--;
                        gotoxy(hcx+cx,hcy);
                } else {
                        for (si=NULL,mi=cm;mi->typ!=0;mi++) {
                                if (mi->y==cmi->y && mi->x < cmi->x) {
                                	if (si!=NULL) {
                                        if (mi->x > si->x) si = mi;
                                	} else {
                                		si = mi;
                                	}
                                }
                        }
                }
                break;
           case eright: /* right */
                if (emode) {
                        if (cx < (int)strlen(cmi->val)) cx++;
                        gotoxy(hcx+cx,hcy);
                } else {
                        for (si=NULL,mi=cm;mi->typ!=0;mi++) {
                                if (mi->y==cmi->y && mi->x > cmi->x) {
                                	if (si!=NULL) {
                                        if (mi->x < si->x)
                                                si = mi;
                                	} else {
                                		si = mi;
                                	}
                                }
                        }
                }
                break;
           case eup: /* arrow up */
                emode = false;
                for (si=NULL,mi=cm;mi->typ!=0;mi++) {
                        if (mi->typ<MNULL) {
                         if (si!=NULL) if (mi->y==si->y)
                           if (abs(mi->x - cmi->x) <  abs(si->x - cmi->x))
                                si = mi;
                         if (mi->y < cmi->y) {
                        	 if (si!=NULL) {
                        		 if (mi->y > si->y) si = mi;
                        	 } else {
                        		 si = mi;
                        	 }
                         }
                        }
                }
                break;
           case edown: /* arrow down */
                emode = false;
                for (si=NULL,mi=cm;mi->typ!=0;mi++) {
                        if (mi->typ<MNULL) {
                         if (si!=NULL) if (mi->y==si->y)
                           if (abs(mi->x - cmi->x) <  abs(si->x - cmi->x))
                                si = mi;
                         if (mi->y > cmi->y) {
                        	 if (si!=NULL) {
                        		 if (mi->y < si->y) si = mi;
                        	 } else {
                        		 si = mi;
                        	 }
                         }
                       }
                }
                break;
          case ehelp: /* f1 help */
                do_help(cmi->help);
                manip_refresh_menu(cm);
                menu_hilight(cmi);
                break;
          case ereturn: /* carriage return */
                if (emode==true && cmi->typ==MFILE) {
                        emode = false;
                        break;
                }
                emode = false;
                switch (cmi->typ) {
                  case MTEXT:
/*                        if (emode) emode = false; else emode = true; */
                        break;
                  case MTOGON:
                        cmi->typ = MTOGOFF;
                        menu_hilight(cmi);
                        break;
                  case MTOGOFF:
                        cmi->typ = MTOGON;
                        menu_hilight(cmi);
                        break;
                  case MRETURN:
                        goto exit_menu;
                  case MFILE:
                        if (mystrlen(cmi->val)==0) cmi->val = (char*)calloc(20,1);
                        pick_file(cmi->val,"*.dat");
                        manip_refresh_menu(cm);
                        menu_hilight(cmi);
                        break;
                  case MSUB: /* SUB MENU */
                        if (cmi->typ2>99) do_menu(&pmenu[cmi->typ2 - 100]);
                        manip_refresh_menu(cm);
                        menu_hilight(cmi);
                }

                for (si=NULL,mi=cm;mi->typ!=0;mi++) {
                        if (mi->y==cmi->y && mi->x > cmi->x) {
                        if (si!=NULL) {
                            	if (mi->x < si->x) si = mi;
                        	} else {
                        		si = mi;
                        	}
                        }
                }

                break;
          case edelete: /* delete */
                emode = true;
                if (cmi->val==NULL) break;
                if (strlen(cmi->val)==0) break;
                if (cx<1) break;
                ncpy(mbuff,cmi->val,cx-1);
                strcat(mbuff,cmi->val + cx);
                setvstr(&cmi->val,mbuff);
                cx--;
                gotoxy(hcx+cx,hcy);
                cputs(cmi->val + cx);
                putch(DASHCHAR);
                gotoxy(hcx+cx,hcy);
                break;
          case edrawit:
                break;
          case eshowerror:
                break;
          case esave: /* save file */
                break;
          default: /* normal key */
                if (c<26  && c!=9) {fner("Key has no affect"); break;}
                if (c>200)  fner("Unimplemented command");
                else {
/*         if (cmi->typ!=MTEXT  && cmi->typ!=MFILE) goto doreturn; */
                        emode = true;
                        if (cmi->val==NULL) setvstr(&cmi->val,"");
                        ncpy(mbuff,cmi->val,cx);
                        mbuff[cx] = c; mbuff[cx+1] = 0;
                        strcat(mbuff,cmi->val + cx);
                        setvstr(&cmi->val,mbuff);
                        gotoxy(hcx+cx,hcy);
                        cputs(cmi->val + cx);
                        cx++;
                        gotoxy(hcx+cx,hcy);
                }
                break;
         }
         if (si!=NULL) {
                menu_norm(cmi);
                cmi = si;
                menu_hilight(cmi);
                si = NULL;
         }
        }
exit_menu:;
        /* save current point */
        { int i;
                for (i=0,mi=cm;mi->typ!=0;mi++,i++) {
                  if (mi==cmi) ppmenu->ci = i;
                }
        }
        return false; /* normal exit */
}

void setvstr(char **d, const char *s) {
        if ((*d) != 0) myfree(*d);
        *d = sdup(s);
}

int mystrlen(const char *s) {
        if (s==NULL) return 0;
        else return strlen(s);
}

void menu_hilight(struct menu_struct *mm) {
        int j;
        struct menu_struct *m=mm;
        scr_menuhi();
        hcx = m->x + mystrlen(m->title);
        hcy = m->y;
        gotoxy(hcx,hcy);
        if (m->typ==MTOGON || m->typ==MTOGOFF) {
                if (m->val==NULL) m->val = (char*)malloc(4);
                if (m->typ==MTOGON) strcpy(m->val,"ON ");
                if (m->typ==MTOGOFF) strcpy(m->val,"OFF");
        }
        if (m->val==NULL) setvstr(&m->val,"");
        cputs(m->val);
        j = m->width - mystrlen(m->val);
        printdash(j);
        cx = mystrlen(m->val);
        gotoxy(hcx+cx,hcy);
}

void printdash(int j) {
        int i;
        for (i=0;i<j;i++) mbuff[i] = DASHCHAR;
        mbuff[j] = 0;
        if (j>0) cputs(mbuff);
}

void menu_norm(struct menu_struct *mm) {
        int j;
        struct menu_struct *m=mm;
        gotoxy(m->x + mystrlen(m->title),m->y);
        if (m->typ==MTOGON  || m->typ==MTOGOFF) {
                if (m->val==NULL) m->val = (char*)malloc(4);
                if (m->typ==MTOGON) strcpy(m->val,"ON ");
                if (m->typ==MTOGOFF) strcpy(m->val,"OFF");
        }
        if (m->val != NULL) {
                scr_menuval();
                if (m->width>0) cputs(m->val);
                scr_menubg();
        }
        j = m->width - mystrlen(m->val);
        printdash(j);
}

void manip_refresh_menu(struct menu_struct *mm) {
        struct menu_struct *m=mm;
        window(1,1,80,24);
        scr_menubg();
        clrscr();
        for (;m->typ!=0;m++) {
                if (m->title != NULL) if (strlen(m->title)>0) {
                        gotoxy(m->x,m->y);
                        cputs(m->title);
                }
                menu_norm(m);
        }
}

void do_help(const char *k1, const char *k2) {
        FILE *hfile;
        int yy;
        char hbuff[90];
        const char *hfilename="manip.hlp";
        char fk1[20],fk2[20];
        scr_menuval();
        clrscr();
        if (strcmp(k2," ")==0) k2 = "";
        gotoxy(1,1);
	wprintf("Manip help on topic {%s},  sub topic {%s}",k1,k2);
        scr_menubg();
        hfile = fopen(gledir(hfilename).c_str(),"r");
        if (hfile==NULL) {
		gotoxy(1,3);
		wprintf("Unable to open {%s} \n",gledir(hfilename).c_str());
		text_inkey();
		return;
	}
        for (;!feof(hfile);) {
                if (fgets(hbuff,90,hfile)!=NULL) {
                        if (hbuff[0]=='3') {
                                strtok(hbuff," \n");
                                strcpy(fk1,strtok(NULL," \n"));
                                if (strcmp(fk1,k1)==0 && strlen(k2)==0)
                                        goto help_type;
                        }
                        if (hbuff[0]=='4') {
                                strtok(hbuff," \n");
                                strcpy(fk2,strtok(NULL," \n"));
                                if (strlen(k2)>0)
                                if (strcmp(fk1,k1)==0 &&
                                        strcmp(fk2,k2)==0) goto help_type;
                        }
                }
        }
        fclose(hfile);
        scr_menuval();
        gotoxy(1,9);
        wprintf("help text not found sorry ??? \n");
        text_inkey();
        return;
help_type:;
        yy = 3;
        for (;!feof(hfile);) {
                if (fgets(hbuff,90,hfile)!=NULL) {
                        if (isdigit(hbuff[0])) {
                                goto end_help;
                        }
                        gotoxy(1,yy++); cputs(tabtospace(hbuff));
                }
                if (yy==22) {
                        scr_menuval();
                        gotoxy(2,24); wprintf("Press any key for next screen of help");
                        text_inkey();  clrscr();
                        gotoxy(1,1); wprintf("Graph help on topic {%s},  sub topic {%s}",k1,k2);
                        scr_menubg();  yy = 3;
                }
        }
end_help:;
        scr_menuval();
        gotoxy(1,yy+1); wprintf("Press any key to continue");
        text_inkey();
        fclose(hfile);
}

void add_unrecognized(char *s) {
        if (nunrec>6) return;
}

void vunquote(char **ss) {
        char *s = *ss;
        if (*s=='\"') {
                s[strlen(s)-1] = 0;
                *ss = sdup(s+1);
                myfree(s);
        }
}

int initmenudone;
menutype *m_pnt[20];
menutype *m_end[20];
char *m_val[20];
char m_tog[50];

void initmenu() {
        menutype *mi;
        int nt=0,z;
        static int nv;

        if (!initmenudone) {
          initmenudone = true;
          for (z=0;z<=10;z++) {
            for (mi=pmenu[z].menu; mi->typ!=0; mi++) {
                if (mi->typ==MTOGON || mi->typ==MTOGOFF) {
                        m_tog[nt++] = mi->typ;
                }
                if (mi->val!=NULL) {
                        m_val[nv] = sdup(mi->val);
                        mi->val = sdup(mi->val);
                        m_pnt[nv++] = mi;
                }
            }
          }
        } else {
          for (z=0;z<=10;z++) {
            for (mi=pmenu[z].menu; mi->typ!=0; mi++) {
                if (mi->typ==MTOGON || mi->typ==MTOGOFF) {
                        mi->typ = m_tog[nt++];
                } else {
                        if (mi->val!=0) {
                                myfree(mi->val);
                        }
                        mi->val = 0;
                }
            }
          }
          for (z=0;z<nv;z++) {
                mi = m_pnt[z];
                mi->val = sdup(m_val[z]);
          }
        }
}

char msend[200];
#define mss msend+strlen(msend)
char cs[80]="xyz";
int gstartend;

void m_sendline() {
        msend[0] = 0;
        strcpy(cs,"xyz");
}

void m_ifsend(char *s) {
        if (strcmp(s,cs)!=0) {
          m_sendline();
          sprintf(msend,"\t%s ",s);
          strcpy(cs,s);
        }
}

void setmenu(menutype *m,int x, int y, int typ, int typ2, int width, const char *title, const char *val, const char *help) {
        m->x = x; m->y = y; m->typ = typ; m->typ2 = typ2; m->width = width;
        m->title = sdup(title);
        if (val!=0) m->val = sdup(val);
        m->help = sdup(help);
}

void init_menucolor() {
                h_bcolor = WHITE; h_fcolor = BLACK;
}

int pick_file(char *result, const char *wld) {
        pmenutype ask;
        char wild[80];
        char *f[80];
        char *swap;
        menutype *m,*mi,*mwild;
#ifdef SOMEOS                          /* a.r. */
        struct ffblk ffblk;
#endif
#ifdef DOS
        HDIR            hdir = HDIR_SYSTEM;
        FILEFINDBUF3    FFbufr;
        unsigned long   SearchCnt = 1;
#endif
        int i,x,y,nf,rval,adddir=false;

        strcpy(wild,wld);
        ask.ci = 0;
pick_again:;
        x=0; y=0; nf=0;
        init_menucolor();
#ifdef SOMEOS                          /* a.r. */
        done = findfirst(wild,&ffblk,0);
        for (nf=0;nf<70 && !done;nf++) {
                f[nf] = sdup(ffblk.ff_name);
                done = findnext(&ffblk);
                }
#endif
#ifdef DOS
        done = DosFindFirst(    wild,
                                &hdir,
                                0,
                                &FFbufr,
                                sizeof(FFbufr),
                                &SearchCnt,
                                FIL_STANDARD);
        for (nf=0;nf<70 && !done;nf++) {
                f[nf] = sdup(FFbufr.achName);
                done = DosFindNext( hdir,
                                    &FFbufr,
                                    sizeof(FFbufr),
                                    &SearchCnt);
                }
#endif

sortagain:;
        swap = NULL;
        for (i=0;i<(nf-1);i++) {
                if (strcmp(f[i],f[i+1])>0) {
                        swap = f[i]; f[i] = f[i+1]; f[i+1] = swap;
                }
        }
        if (swap != NULL) goto sortagain;

        m = (menutype*)calloc(3+nf,sizeof(*m));
        ask.menu = m;
        ask.ci = 0;
        if (nf==0) ask.ci = 1;
        if (m==0) wprintf("memory allocation error");
        for (mi=m,i=0;i<nf;i++,mi++) {
                setmenu(mi,x*20+1,y+5,MRETURN,0,18,"",f[i],"");
                x++;
                if (x==4) {x=0; y++; }
        }
        setmenu(mi,5,1,MNULL,0,0,"Use arrow keys to select file then press return","","");
        mi++;
        setmenu(mi,5,3,MRETURN,0,30,"Disk/Dir Specification ",wild,"");
        mwild = mi;
        mi++; mi->typ = 0;
        rval = do_menu(&ask);
        strcpy(result,((ask.menu)[ask.ci]).val);

        if (strcmp(wild,mwild->val)!=0) {
                strcpy(wild,mwild->val);
                if (strstr(wild,"*")==NULL) {
                        if (wild[strlen(wild)-1] == '\\')
                                strcat(wild,"*.dat");
                        else
                                strcat(wild,"\\*.dat");
                }
                adddir = true;
                goto pick_again;
        }

        for (i=0;i<nf;i++) myfree(f[i]);
        myfree(m);

        if (adddir) {
                *(strfile(wild)) = 0;
#if (defined __TURBOC__ || defined DJ)
                if (strstr(wild,":")!=NULL) {
                        setdisk(toupper(*wild)-'A');
                }
                strcpy(buff2,wild+2); strcpy(wild,buff2);
#endif
#ifdef EMXOS2                                                   /* a.r. */
                if (strstr(wild,":")!=NULL) {
                        DosSetDefaultDisk(toupper(*wild)-'A' + 1);
                }
                strcpy(buff2,wild+2); strcpy(wild,buff2);
                if (_chdir2(wild)!=0) ; /* {perror("Cannot change dir"); delay(3000);} */
#else
                if (chdir(wild)!=0) ; /* {perror("Cannot change dir"); delay(3000);} */
#endif
        }

/*        if (adddir) {
                *(strfile(wild)+1) = 0;
                strcpy(result,wild);
                strcat(result,((ask.menu)[ask.ci]).val);
        }
*/
        return rval;
}

char *strfile(char *s) {
        char *e;
        e = s;
        for (;*s!=0;s++) {
                if (*s==']'  ||  *s=='\\' || *s=='/' || *s==':') e = s;
        }
        return e;
}

char *tabtospace(char *s) {
        static char buf[255];
        char *o = &buf[0];
        int p=0,k,df;
        for (;*s!=0;s++) {
                if (*s==9) {
                        df = (p/8)*8+8-p;
                        for (k=1;k<=df;k++) {
                                *o++ = ' ';
                                p++;
                        }
                } else {
                        *o++ = *s;
                        p++;
                }
        }
        *o = 0;
        return &buf[0];
}
