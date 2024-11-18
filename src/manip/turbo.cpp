#include <stdlib.h>
#include <stdio.h>
#if ( ! ( defined _WIN32 || defined EMXOS2 ))       /* a.r. */
   	#include "vaxconio.h"
#endif
//#include "int32.h"
#define MANIP

//#define true (!false)
//#define false 0
#if ( ! ( defined _WIN32 || defined EMXOS2 ))       /* a.r. */
getch()
{
        return tt_inkey();
}
#endif
int32 coreleft(void)
{
        return 4000000l;
}
#ifndef _WIN32  /* a.r. */
strlwr(char *s)
{
        char *ss=s;
        while (*s!=0) {*s = tolower(*s); s++;}
        return ss;
}
#endif
char *strupr(char *s)
{
        char *ss=s;
        while (*s!=0) {*s = toupper(*s); s++;}
        return ss;
}
#if (!(defined _WIN32 || defined EMXOS2 ))      /* a.r. */
unlink(char *filename)
{
        delete(filename);
}

#include <descrip.h>
int find_context;
static char wildstr[200];
static $DESCRIPTOR(str1,"");
static $DESCRIPTOR(str2,"");
static char rfile[200];
findfirst(char *wild, struct ffblk *ffblk, int zz)
{
        int st;
        char *s;
        strcpy(wildstr,wild);
        strcat(wildstr,";");
        str1.dsc$a_pointer = wildstr;  str1.dsc$w_length = strlen(wildstr);
        str2.dsc$a_pointer = &rfile[0];  str2.dsc$w_length = 132;
        st = LIB$FIND_FILE(&str1,&str2,&find_context);
        if ((st & 1) != 1) {
                st = LIB$FIND_FILE_END(&find_context);
                if ((st & 1) != 1) LIB$SIGNAL(st);
                return true;
        }
        ffblk->ff_name = &rfile[0]; rfile[str2.dsc$w_length] = 0;
        s = strchr(ffblk->ff_name,' ');
        *s = 0;
        trim_file(ffblk->ff_name);
        return false;
}
findnext(struct ffblk *ffblk)
{
        int st;
        char *s;
        st = LIB$FIND_FILE(&str1,&str2,&find_context);
        if ((st & 1) != 1) {
                st = LIB$FIND_FILE_END(&find_context);
                if ((st & 1) != 1) LIB$SIGNAL(st);
                return true;
        }
        ffblk->ff_name = &rfile[0]; rfile[str2.dsc$w_length] = 0;
        s = strchr(ffblk->ff_name,' ');
        if (s!=NULL) *s = 0;
        trim_file(ffblk->ff_name);
        return false;
}
#endif /* _WIN32 + EMXOS2 */
trim_file(char *s)
{
        char *t;
        t = strchr(s,']');
        if (t!=NULL) memcpy(s,t+1,strlen(t));
        t = strchr(s,';');
        if (s!=NULL) *t = 0;
}

#if (defined _WIN32 || defined EMXOS2 )       /* a.r. */

char *getsymbol(char *s)
{
                static char ss[100];
                ss[0] = 0;
                if ( getenv(s) != NULL) strcpy(ss,getenv(s));
                return ss;
}

#else

char *getsymbol(char *sym)
{
        static char mystr[200],*s;
        int r;
        short teklen=80;
        $DESCRIPTOR(symname,sym);
        $DESCRIPTOR(tekval,mystr);
        #ifndef MANIP /* who knows ... */
        mystr[0] = 0;
        #endif
        symname.dsc$w_length = strlen(sym);
        tekval.dsc$w_length = 80;
        r = lib$get_symbol(&symname,&tekval,&teklen,&1);
        mystr[teklen] = 0;
loop1:        s = strchr(mystr,'^');
        if (s!=NULL) {
                *(s+1) = *(s+1) - 64;
                memmove(s,s+1,strlen(s+1)+1);
                goto loop1;
        }
        return mystr;
}
#endif

