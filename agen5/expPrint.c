
/*
 *  $Id: expPrint.c,v 3.18 2003/05/26 03:14:59 bkorb Exp $
 *
 *  The following code is necessary because the user can give us
 *  a printf format requiring a string pointer yet fail to provide
 *  a valid pointer, thus it will fault.  This code protects
 *  against the fault so an error message can be emitted instead of
 *  a core dump :-)
 */

/*
 *  AutoGen copyright 1992-2003 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

STATIC sigjmp_buf printJumpEnv;
STATIC void   printFault( int sig );
STATIC ssize_t safePrintf( char** pzBuf, char* pzFmt, void** argV );
tSCC pzFormatName[] = "format";

STATIC void
printFault( int sig )
{
    siglongjmp( printJumpEnv, sig );
}


STATIC ssize_t
safePrintf( char** ppzBuf, char* pzFmt, void** argV )
{
    tSCC zBadArgs[]  = "Bad args to sprintf";
    tSCC zBadFmt[]   = "%s ERROR:  %s processing printf format:\n\t%s\n";

    struct sigaction  saSave1;
    struct sigaction  saSave2;

    {
        struct sigaction  sa;
        sa.sa_handler = printFault;
        sa.sa_flags   = 0;
        sigemptyset( &sa.sa_mask );

        sigaction( SIGBUS,  &sa, &saSave1 );
        sigaction( SIGSEGV, &sa, &saSave2 );
    }

    /*
     *  IF the sprintfv call below is going to address fault,
     *  THEN ...
     */
    {
        int faultType = sigsetjmp( printJumpEnv, 0 );
        if (faultType != 0) {
#ifndef HAVE_STRSIGNAL
            extern char* strsignal(int signo);
#endif
            /*
             *  IF the fprintf command in the then clause has not failed yet,
             *  THEN perform that fprintf
             */
            if (sigsetjmp( printJumpEnv, 0 ) == 0)
                fprintf(pfTrace, zBadFmt, pzProg, strsignal( faultType ), pzFmt);

            /*
             *  The "sprintfv" command below faulted, so we exit
             */
            AG_ABEND( zBadArgs );
        }
    }

    {
        size_t printSize = asprintfv( ppzBuf, pzFmt, (void*)argV );
        if ((printSize & ~0xFFFFFF) != 0)
            AG_ABEND( aprf( "asprintfv returned 0x%08X\n", printSize ));

        sigaction( SIGBUS,  &saSave1, NULL );
        sigaction( SIGSEGV, &saSave2, NULL );
        return printSize;
    }
}


EXPORT SCM
run_printf( char* pzFmt, int len, SCM alist )
{
    SCM     res;
    void*   args[8];
    void**  arglist;
    void**  argp;

    if (len < 8)
        arglist = argp = args;
    else
        arglist =
        argp    = (void**)malloc( (len+1) * sizeof(void));

    while (len-- > 0) {
        SCM  car = SCM_CAR( alist );
        alist = SCM_CDR( alist );
        switch (gh_type_e( car )) {
        default:
        case GH_TYPE_UNDEFINED:
            *(argp++) = (void*)"???";
            break;

        case GH_TYPE_BOOLEAN:
            *(argp++) = (void*)((car == SCM_BOOL_F) ? "#f" : "#t");
            break;

        case GH_TYPE_CHAR:
            *(argp++) = (void*)(int)gh_scm2char( car );
            break;

        case GH_TYPE_PAIR:
            *(argp++) = (void*)"..";
            break;

        case GH_TYPE_NUMBER:
            *(argp++) = (void*)gh_scm2ulong( car );
            break;

        case GH_TYPE_SYMBOL:
        case GH_TYPE_STRING:
            *(argp++) = (void*)ag_scm2zchars( car, "printf str" );
            break;

        case GH_TYPE_PROCEDURE:
            *(argp++) = (void*)"(*)()";
            break;

        case GH_TYPE_VECTOR:
        case GH_TYPE_LIST:
            *(argp++) = (void*)"...";
            break;
        }
    }

    /*
     *  Do the formatting and allocate a new SCM to hold the result.
     *  Free up any allocations made by ``gh_scm2newstr''
     */
    {
        char*   pzBuf;
        ssize_t bfSize = safePrintf( &pzBuf, pzFmt, arglist );
        res = gh_str2scm( pzBuf, bfSize );
        free( pzBuf );
    }

    if (arglist != args)
        AGFREE( (void*)arglist );

    return res;
}


/*=gfunc sprintf
 *
 * what:  format a string
 * general_use:
 *
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
=*/
SCM
ag_scm_sprintf( SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );

    if (list_len <= 0)
        return fmt;

    return run_printf( pzFmt, list_len, alist );
}


/*=gfunc printf
 *
 * what:  format to stdout
 * general_use:
 *
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to the standard out port.  The result will NOT appear in your
 *       output.  Use this to print information messages to a template user.
 *       Use ``(sprintf ...)'' to add text to your document.
=*/
SCM
ag_scm_printf( SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );

    gh_display( run_printf( pzFmt, list_len, alist ));
    return SCM_UNDEFINED;
}


/*=gfunc fprintf
 *
 * what:  format to a file
 * general_use:
 *
 * exparg: port, Guile-scheme output port
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to a specified port.  The result will NOT appear in your
 *       output.  Use this to print information messages to a template user.
=*/
SCM
ag_scm_fprintf( SCM port, SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );
    SCM   res      = run_printf( pzFmt, list_len, alist );

    return  scm_display( res, port );
}


/*=gfunc hide_email
 *
 * what:  convert eaddr to javascript
 * general_use:
 *
 * exparg: display, display text
 * exparg: eaddr,   email address
 *
 * doc:    Hides an email address as a java scriptlett.
 *         The 'mailto:' tag and the email address are coded bytes
 *         rather than plain text.  They are also broken up.
=*/
SCM
ag_scm_hide_email( SCM display, SCM eaddr )
{
    tSCC zStrt[]  = "<script language=\"JavaScript\" type=\"text/javascript\">\n"
                    "<!--\n"
                    "var one = 'm&#97;';\n"
                    "var two = 'i&#108;t';\n"
                    "document.write('<a href=\"' + one + two );\n"
                    "document.write('&#111;:";
    tSCC zEnd[]   = "');\n"
        "document.write('\" >%s</a>');\n"
        "//-->\n</script>";
    tSCC zFmt[]   = "&#%d;";
    char*  pzDisp = ag_scm2zchars( display, pzFormatName );
    char*  pzEadr = ag_scm2zchars( eaddr,   pzFormatName );
    SCM    res;

    {
        size_t sz = (strlen( pzEadr ) * sizeof( zFmt ))
            + sizeof( zStrt ) + sizeof( zEnd ) + strlen( pzDisp );

        res = scm_makstr( sz, 0 );
    }

    {
        char* pz = SCM_CHARS( res );

        strcpy( pz, zStrt );
        pz += sizeof( zStrt ) - 1;

        for (;;) {
            if (*pzEadr == NUL)
                break;
            pz += sprintf( pz, zFmt, *(pzEadr++) );
        }

        sprintf( pz, zEnd, pzDisp );
    }

    return res;
}


/*=gfunc   format_arg_count
 *
 * what:   count the args to a format
 * general_use:
 *
 * exparg: format, formatting string
 *
 * doc:    "Sometimes, it is useful to simply be able to figure out how many\n"
 *         "arguments are required by a format string.  For example, if you\n"
 *         "are extracting a format string for the purpose of generating a\n"
 *         "macro to invoke a printf-like function, you can run the\n"
 *         "formatting string through this function to determine how many\n"
 *         "arguments to provide for in the macro. e.g. for this extraction\n"
 *         "text:\n"
 *         "@example\n\n"
 *         " /" "*=fumble bumble\n"
 *         "  * fmt: 'stumble %s: %d\\n'\n"
 *         " =*" "/\n"
 *         "@end example\n\n"
 *         "@noindent\n"
 *         "You may wish to generate a macro:\n"
 *         "@example\n\n"
 *         " #define BUMBLE(a1,a2) printf_like(something,(a1),(a2))\n"
 *         "@end example\n\n"
 *         "@noindent\n"
 *         "You can do this by knowing that the format needs two arguments.\n"
=*/
SCM
ag_scm_format_arg_count( SCM fmt )
{
    char* pzFmt = ag_scm2zchars( fmt, pzFormatName );
    int   ct    = 0;
    for (;;) {
        switch (*(pzFmt++)) {
        case NUL: goto scanDone;
        case '%':
            if (*pzFmt == '%')
                 pzFmt++;
            else ct++;
        }
    } scanDone:;

    return gh_int2scm( ct );
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expPrint.c */
