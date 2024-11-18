#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "conio.h"

#ifndef EMXOS2                                  /* a.r. */
        #include "bios.h"
#else
        #include <sys/types.h>
        #include <conio.h>
        #include <sys/kbdscan.h>
        #include <sys/video.h>
#endif

#include "edt.h"
#define true (!false)
#define false 0
int fner(char *s);
int scr_refresh(void);
int scr_getch(void);

#ifdef MANIP
int mjl_key(int c);
#endif

#ifdef EMXOS2                                   /* a.r. */
static int last_key = -1;
static int os2getch (void)
{
  int c;

  if (last_key >= 0)
        {
          c = last_key;
          last_key = -1;
        }
  else
        c = _read_kbd (0, 1, 0);
  return (c);
}
int kbhit()
{
        return _read_kbd(0,0,0);
}
delay(int i){;}
#endif

#ifndef MANIP
char *function_bar()
{
        return "F1-Help  F2-Save  F3-Load  F4-Saveas  F5-Errs  F9-Graph  F10-Drawit ^Z Quit";
}
#else
char *function_bar()
{
        return "F1=Help  F2=Save  F3=Load    Pageup=Recall command     ^Z Quit";
}
#endif


#ifndef SURFACE
extern int in_recover,single_step;
#endif

struct keymatch {int m; int val;};
/* commands using hi 8 bits of bios, e.g. arrow keys */
struct keymatch kmatch1[] = {
        0x0e, edelete,      /* DOS ? */
        0x4d, eright,
        0x4b, eleft,
        0x48, eup,
        0x53, edelright,
        0x50, edown,
        0x4a, edelline,     /* DOS ? */
        0x3b, ehelp,        /* F1   */
        0x3c, esave,        /* F2   */
        0x3d, eload,        /* F3   */
        0x3e, esaveas,      /* F4   */
        0x3f, eshowerror,   /* F5   */
        0x43, egraphmenu,   /* F9   */
        0x44, edrawit,      /* F10  */
        0x49, epageup,
        0x51, epagedown,
        0x47, ebol,
        0x4f, eeol,
#ifdef EMXOS2                                   /* a.r. */
 #ifndef MANIP
        0x2d, eescape,      /* Alt-x */
 #else
        0x73, ebigleft,     /* ^left  */
        0x74, ebigright,    /* ^right */
        0x91, edelline,     /* ^up    */
        0x8d, esearch,      /* ^down  */
 #endif
#endif
        0,0
};
/* Normal key and ^ commands  commands */
struct keymatch kmatch2[] = {
        0x0d, ereturn,      /* RETURN */
        0x03, equit,        /* ^c   */
        0x04, eword,        /* ^d   */
        0x05, eshowerror,   /* ^e   */
#ifndef MANIP
        0x06, efast,        /*      */
#else
        0x02, ebackline,    /* ^b   */
#endif
#ifdef EMXOS2                                           /* a.r. */
        0x08, edelete,      /* Backspace, ^h */
#else
        0x08, ehelp,        /* ^h   */
#endif
        0x13, eshell,       /* ^s   */
        0x14, etrace,       /* ^t   */
        0x0c, efindnext,    /* ^l   */
        0x15, eundelline,   /* ^u   */
        0x19, edelline,     /* ^y   */
        0x1a, eescape,      /* ^z   */
        0x1b, eescape,      /* ESC  */
        0,0
};
/* Control K commands */
struct keymatch kmatch3[] = {
        'b', eselect,
        'v', emove,
        'k', emark,
        'c', ecopy,
        'y', ecut,
        'u', epaste,
        'p', epaste,
        'r', eblockread,
        'w', eblockwrite,
        'm', egraphmenu,
        'l', eload,
        'd', edrawit,
        's', esave,
        'x', equit,
        0,0
};
/* Control Q commands */
struct keymatch kmatch4[] = {
        'f', esearch,
        'c', eendoffile,
        'r', etopoffile,
        0,0
};

#ifndef MANIP
#ifndef EMXOS2                                  /* a.r. */
text_inkey()
{
        int cc,i,c1,c2,ff;
        scr_refresh();
loop1:        cc = bioskey(0);
        ff = bioskey(2);
        c1 = (cc & 0xff00)>>8;
        c2 = (cc & 0xff);
        if (c1==45 && ((ff & 8)>0) ) return eexitnow;
        for (i=0;kmatch1[i].m!=0;i++)
                if (kmatch1[i].m==c1) return kmatch1[i].val;

        switch(c2) {
          default:
            for (i=0;kmatch2[i].m!=0;i++)
                if (kmatch2[i].m==c2) return kmatch2[i].val;
            break;
          case 17:
            fner("^Q  F=Find string,  R=Top of file,  C=End of file");
            cc = bioskey(0);
            c2 = (cc & 0xff);
            if (c2<32) c2 = c2 + 'a' - 1;
            c2 = tolower(c2);
            for (i=0;kmatch4[i].m!=0;i++)
                if (kmatch4[i].m==c2) return kmatch4[i].val;
            fner("Unrecognized Quick movement command");
            goto loop1;
          case 11:
            fner("^K  B=begin block,  P=Paste,  Y=Cut,  K=End block");
            cc = bioskey(0);
            c2 = (cc & 0xff);
            if (c2<32) c2 = c2 + 'a' - 1;
            c2 = tolower(c2);
            for (i=0;kmatch3[i].m!=0;i++)
                if (kmatch3[i].m==c2) return kmatch3[i].val;
            fner("Unrecognized block command");
            goto loop1;
        }
        return c2;
}

#else   /* EMXOS2 */

text_inkey()
{
        int c,i,c2,c3;
        scr_refresh();
loop1:    
          c = os2getch();
          switch (c)
                {
                case 0:                   /* 8 bit keys: Alt-x, Fx, Home... */
                  c2 = os2getch();
                  for (i=0;kmatch1[i].m!=0;i++)
                           if (kmatch1[i].m==c2) {return kmatch1[i].val;}
                  break;


                case 0x11:                              /* Ctrl-q x sequences */
                                fner("^Q  F=Find string,  R=Top of file,  C=End of file");
                                c3 = os2getch();
                                c3 = (c3 & 0xff);
                                if (c3<32) c3 = c3 + 'a' - 1;
                                c3 = tolower(c3);
                                for (i=0;kmatch4[i].m!=0;i++)
                                if (kmatch4[i].m==c3) return kmatch4[i].val;
                                fner("Unrecognized Quick movement command");
                                goto loop1;

                case 0x0b:                              /* Ctrl-k x sequences */
                                fner("^K  B=begin block,  P=Paste,  Y=Cut,  K=End block");
                                c3 = os2getch();
                                c3 = (c3 & 0xff);
                                if (c3<32) c3 = c3 + 'a' - 1;
                                c3 = tolower(c3);
                                for (i=0;kmatch3[i].m!=0;i++)
                                if (kmatch3[i].m==c3) return kmatch3[i].val;
                                fner("Unrecognized block command");
                                goto loop1;

                default:                                /* normal and contol keys */
                        for (i=0;kmatch2[i].m!=0;i++)
                                if (kmatch2[i].m==c) {return kmatch2[i].val;}
                }
return c;            
}
#endif  /* EMXOS2 */

#else /* MANIP follows */

extern int iserr;
FILE *jouptr;
text_inkey()
{
        int c;
        unsigned char ccc;
        static int rdone;
        if (in_recover && !rdone) {
#ifdef unix
                if (jouptr==NULL) jouptr = fopen("manip_.j1","r");
#else
                if (jouptr==NULL) jouptr = fopen("manip_.j1","rb");
#endif
                if (jouptr==NULL) fner("Unable to open/read journal file manip_.j1");
                if (fread(&ccc,1,1,jouptr)!=1) {
                        rdone = true;
                        fclose(jouptr);
                }
                if (single_step) if (getch()!=' ') rdone = true;
                return ccc;
        } else {
                c = xtext_inkey();
                mjl_key(c);
                return c;
        }
}
#ifndef EMXOS2                  /* a.r. */
xtext_inkey()
{
                int cc,i,c1,c2,ff;
                scr_refresh();
loop1:        cc = bioskey(0);
                ff = bioskey(2);
                c1 = (cc & 0xff00)>>8;
                c2 = (cc & 0xff);
/*printf("%d %d ff=%d\n",c1,c2,ff); */
                if (c1==0x48 && ((ff & 2)>0) ) return esearch;
                if (c1==0x50 && ((ff & 2)>0) ) return edelline;
                if (c1==0x4b && ((ff & 2)>0) ) return ebigleft;
                if (c1==0x4d && ((ff & 2)>0) ) return ebigright;
                if (c1==45 && ((ff & 8)>0) ) return eexitnow;
                if (c2==0 || c2==8) {
                                for (i=0;kmatch1[i].m!=0;i++)
                                if (kmatch1[i].m==c1) {return kmatch1[i].val;}
                }
                switch(c2) {
                  default:
                        for (i=0;kmatch2[i].m!=0;i++)
                                if (kmatch2[i].m==c2) return kmatch2[i].val;
                        break;
                  case 17:
                        fner("^Q  F=Find string,  R=Top of file,  C=End of file");
                        cc = bioskey(0);
                        c2 = (cc & 0xff);
                        if (c2<32) c2 = c2 + 'a' - 1;
                        c2 = tolower(c2);
                        for (i=0;kmatch4[i].m!=0;i++)
                                if (kmatch4[i].m==c2) return kmatch4[i].val;
                        fner("Unrecognized Quick movement command");
                        goto loop1;
                  case 11:
                        fner("^K  B=begin block,  P=Paste,  Y=Cut,  K=End block");
                        cc = bioskey(0);
                        c2 = (cc & 0xff);
                        if (c2<32) c2 = c2 + 'a' - 1;
                        c2 = tolower(c2);
                        for (i=0;kmatch3[i].m!=0;i++)
                                if (kmatch3[i].m==c2) return kmatch3[i].val;
                        fner("Unrecognized block command");
                        goto loop1;
                }
                return c2;
}

#else   /* EMXOS2 --  a.r. */

xtext_inkey()
{
        int c,i,c1,c2,c3,ff, backspaces;
        scr_refresh();
loop1:    
          c = os2getch();
          switch (c)
                {
                case 0:                                 /* 8 bit keys: Alt-x, Fx, Home... */
                  c2 = os2getch();
                  for (i=0;kmatch1[i].m!=0;i++)
                           if (kmatch1[i].m==c2) {return kmatch1[i].val;}
                  break;


                case 0x11:                              /* Ctrl-q x sequences */
                                fner("^Q  F=Find string,  R=Top of file,  C=End of file");
                                c3 = os2getch();
                                c3 = (c3 & 0xff);
                                if (c3<32) c3 = c3 + 'a' - 1;
                                c3 = tolower(c3);
                                for (i=0;kmatch4[i].m!=0;i++)
                                if (kmatch4[i].m==c3) return kmatch4[i].val;
                                fner("Unrecognized Quick movement command");
                                goto loop1;

                case 0x0b:                              /* Ctrl-k x sequences */
                                fner("^K  B=begin block,  P=Paste,  Y=Cut,  K=End block");
                                c3 = os2getch();
                                c3 = (c3 & 0xff);
                                if (c3<32) c3 = c3 + 'a' - 1;
                                c3 = tolower(c3);
                                for (i=0;kmatch3[i].m!=0;i++)
                                if (kmatch3[i].m==c3) return kmatch3[i].val;
                                fner("Unrecognized block command");
                                goto loop1;

                default:                                /* normal and contol keys */
                        for (i=0;kmatch2[i].m!=0;i++)
                                if (kmatch2[i].m==c) {return kmatch2[i].val;}
                }
return c;            
}
#endif  /* EMXOS2 */

init_logging(char *infile)
{
        FILE *jptr;
        unsigned char ccc;
        static char buff[80];
        if (in_recover) {
#ifdef unix
                jouptr = fopen("manip_.j1","r");
#else
                jouptr = fopen("manip_.j1","rb");
#endif
                if (jouptr==NULL) fner("Unable to open/read journal file manip_.j1");
                fread(&ccc,1,1,jouptr);
                fread(buff,1,ccc,jouptr);
                if (strlen(infile)==0) strcpy(infile,buff);
                return;
        }
        unlink("manip_.j3");
        rename("manip_.j2","manip_.j3");
        rename("manip_.j1","manip_.j2");
#ifdef unix
        jptr = fopen("manip_.j1","w");
#else
        jptr = fopen("manip_.j1","wb");
#endif
        fputc(strlen(infile),jptr);
        fwrite(infile,1,strlen(infile),jptr);
        if (jptr==NULL) fner("Unable to open journal file manip_.j1");
        fclose(jptr);
}
unsigned char mjl_buff[80];
mjl_key(int c)
{
        int s;
        if (in_recover) return;
        if (c==0) return;
        s = strlen(mjl_buff);
        mjl_buff[s] = c;
        mjl_buff[s+1] = 0;
        if (s>40) mjl_flush();
}
mjl_flush()
{
        FILE *jptr;
        if (in_recover) return;
        if (strlen(mjl_buff)<7) return;
        jptr = fopen("manip_.j1","ab");
        if (jptr==NULL) fner("Unable to append to journal file manip_.j1");
        fwrite(mjl_buff,1,strlen(mjl_buff),jptr);
        fclose(jptr);
        mjl_buff[0] = 0;
}

#endif