/*  -*- Mode: C -*-
 *
 *  expFormat.c
 *  $Id: expFormat.c,v 3.19 2003/05/26 03:14:59 bkorb Exp $
 *  This module implements formatting expression functions.
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

static const char zGpl[] =
"%2$s%1$s is free software.\n%2$s\n"
"%2$sYou may redistribute it and/or modify it under the terms of the\n"
"%2$sGNU General Public License, as published by the Free Software\n"
"%2$sFoundation; either version 2, or (at your option) any later version.\n"
"%2$s\n"
"%2$s%1$s is distributed in the hope that it will be useful,\n"
"%2$sbut WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"%2$sMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"%2$sSee the GNU General Public License for more details.\n"
"%2$s\n"
"%2$sYou should have received a copy of the GNU General Public License\n"
"%2$salong with %1$s.  See the file \"COPYING\".  If not,\n"
"%2$swrite to:  The Free Software Foundation, Inc.,\n"
"%2$s           59 Temple Place - Suite 330,\n"
"%2$s           Boston,  MA  02111-1307, USA.";

static const char zLgpl[] =
"%2$s%1$s is free software; you can redistribute it and/or\n"
"%2$smodify it under the terms of the GNU Lesser General Public\n"
"%2$sLicense as published by the Free Software Foundation; either\n"
"%2$sversion 2.1 of the License, or (at your option) any later version.\n"
"%2$s\n"
"%2$s%1$s is distributed in the hope that it will be useful,\n"
"%2$sbut WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"%2$sMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"%2$sLesser General Public License for more details.\n"
"%2$s\n"
"%2$sYou should have received a copy of the GNU Lesser General Public\n"
"%2$sLicense along with %1$s; if not, write to:\n"
"%2$s           The Free Software Foundation, Inc.,\n"
"%2$s           59 Temple Place - Suite 330,\n"
"%2$s           Boston,  MA  02111-1307, USA.";

static const char zBsd[] =
"%2$s%1$s is free software copyrighted by %3$s.\n%2$s\n"
"%2$sRedistribution and use in source and binary forms, with or without\n"
"%2$smodification, are permitted provided that the following conditions\n"
"%2$sare met:\n"
"%2$s1. Redistributions of source code must retain the above copyright\n"
"%2$s   notice, this list of conditions and the following disclaimer.\n"
"%2$s2. Redistributions in binary form must reproduce the above copyright\n"
"%2$s   notice, this list of conditions and the following disclaimer in the\n"
"%2$s   documentation and/or other materials provided with the distribution.\n"
"%2$s3. Neither the name ``%3$s'' nor the name of any other\n"
"%2$s   contributor may be used to endorse or promote products derived\n"
"%2$s   from this software without specific prior written permission.\n"
"%2$s\n"
"%2$s%1$s IS PROVIDED BY %3$s ``AS IS'' AND ANY EXPRESS\n"
"%2$sOR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"%2$sWARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
"%2$sARE DISCLAIMED.  IN NO EVENT SHALL %3$s OR ANY OTHER CONTRIBUTORS\n"
"%2$sBE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n"
"%2$sCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n"
"%2$sSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR\n"
"%2$sBUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"
"%2$sWHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR\n"
"%2$sOTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF\n"
"%2$sADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

static const char zDne1[] =
"%s -*- buffer-read-only: t -*- vi: set ro:\n"
"%s\n";

static const char zDne[] = "%6$s"
"%1$sDO NOT EDIT THIS FILE   (%2$s)\n"
"%1$s\n"
"%1$sIt has been AutoGen-ed  %3$s\n"
"%1$sFrom the definitions    %4$s\n"
"%1$sand the template file   %5$s";

static const char zDne2[] = "%6$s"
"%1$sEDIT THIS FILE WITH CAUTION  (%2$s)\n"
"%1$s\n"
"%1$sIt has been AutoGen-ed  %3$s\n"
"%1$sFrom the definitions    %4$s\n"
"%1$sand the template file   %5$s";

tSCC zOwnLen[]  = "owner length";
tSCC zPfxLen[]  = "prefix length";
tSCC zProgLen[] = "program name length";
tSCC zPfxMsg[]  = "%s may not exceed %d chars\n";

tSCC zFmtAlloc[] = "asprintf allocation";

/*=gfunc dne
 *
 * what:  '"Do Not Edit" warning'
 *
 * exparg: prefix,       string for starting each output line
 * exparg: first_prefix, for the first output line, opt
 * exparg: optpfx,       shifted prefix, opt
 *
 * doc:  Generate a "DO NOT EDIT" or "EDIT WITH CARE" warning string.
 *       Which depends on whether or not the @code{--writable} command line
 *       option was set.  The first argument is a per-line string prefix.
 *       The optional second argument is a prefix for the first-line and,
 *       in read-only mode, activates the editor hints.
 *       @*
 *       @example
 *       -*- buffer-read-only: t -*- vi: set ro:
 *       @end example
 *       @noindent
 *       The warning string also includes information about the template used
 *       to construct the file and the definitions used in its instantiation.
 *
 *       The optional third argument is used when the first argument is actually
 *       an invocation option and the prefix arguments get shifted.
 *       The first argument must be, specifically, "@code{-d}".  That is used
 *       to signify that the date stamp should not be inserted into the output.
=*/
SCM
ag_scm_dne( SCM prefix, SCM first, SCM opt )
{
    int      noDate  = 0;
    char     zScribble[ 128 ];
    char*    pzRes;
    tCC*     pzFirst = zNil;
    SCM      res;
    tCC*     pzPrefix;

    /*
     *  Check for the ``-d'' option
     */
    if (gh_string_p( prefix )) {
        int len = SCM_LENGTH( prefix );
        if ((len == 2) && (strncmp( SCM_CHARS( prefix ), "-d", 2 ) == 0)) {
            noDate = 1;
            prefix = first;
            first  = opt;
        }
    }

    if (SCM_LENGTH( prefix ) > 128 )
        AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));
    pzPrefix = ag_scm2zchars( prefix, "prefix" );

    /*
     *  IF we also have a 'first' prefix string,
     *  THEN we set it to something other than ``zNil'' and deallocate later.
     */
    if (gh_string_p( first )) {
        int len = SCM_LENGTH( first );
        if (len >= 128)
            AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));

        pzFirst = aprf( ENABLED_OPT( WRITABLE ) ? "%s\n" : zDne1,
                        ag_scm2zchars( first, "first-prefix" ), pzPrefix );
    }

    if (noDate) {
        zScribble[0] = NUL;
    } else {
        time_t    curTime = time( NULL );
        struct tm*  pTime = localtime( &curTime );
        strftime( zScribble, 128, "%A %B %e, %Y at %r %Z", pTime );
    }

    {
        const char* pz;
        if (! ENABLED_OPT( DEFINITIONS ))
            pz = "<<no definitions>>";

        else if (*pzOopsPrefix != NUL)
            pz = "<<CGI-definitions>>";

        else {
            pz = OPT_ARG( DEFINITIONS );
            if (strcmp( pz, "-" ) == 0)
                pz = "stdin";
        }

        pzRes = aprf( ENABLED_OPT( WRITABLE ) ? zDne2 : zDne,
                      pzPrefix, pCurFp->pzOutName, zScribble,
                      pz, pzTemplFileName, pzFirst );
    }

    if (pzRes == NULL)
        AG_ABEND( "Allocating Do-Not-Edit string" );

    res = gh_str02scm( pzRes );

    /*
     *  Deallocate any temporary buffers.  pzFirst either points to
     *  the zNil string, or to an allocated buffer.
     */
    AGFREE( (void*)pzRes );
    if (pzFirst != zNil)
        AGFREE( (void*)pzFirst );

    return res;
}


/*=gfunc error
 *
 * what:  display message and exit
 *
 * exparg: @message@message to display before exiting@@
 * doc:
 *
 *  The argument is a string that printed out as part of an error
 *  message.  The message is formed from the formatting string:
 *
 *  @example
 *  DEFINITIONS ERROR in %s line %d for %s:  %s\n
 *  @end example
 *
 *  The first three arguments to this format are provided by the
 *  routine and are:  The name of the template file, the line within
 *  the template where the error was found, and the current output
 *  file name.
 *
 *  After displaying the message, the current output file is removed
 *  and autogen exits with the EXIT_FAILURE error code.  IF, however,
 *  the argument begins with the number 0 (zero), or the string is the
 *  empty string, then processing continues with the next suffix.
=*/
SCM
ag_scm_error( SCM res )
{
    tSCC      zFmt[]    = "DEFINITIONS %s in %s line %d for %s:\n\t%s\n";
    tSCC      zErr[]    = "ERROR";
    tSCC      zWarn[]   = "Warning";
    tSCC      zBadMsg[] = "??? indecipherable error message ???";
    tCC*      pzMsg;
    tSuccess  abort = FAILURE;
    char      zNum[16];
    int       msgLen;

    switch (gh_type_e( res )) {
    case GH_TYPE_BOOLEAN:
        if (SCM_FALSEP( res ))
            abort = PROBLEM;
        pzMsg = zNil;
        break;

    case GH_TYPE_NUMBER:
    {
        unsigned long val = gh_scm2ulong( res );
        if (val == 0)
            abort = PROBLEM;
        snprintf( zNum, sizeof( zNum ), "%d", (int)val );
        pzMsg = zNum;
        break;
    }

    case GH_TYPE_CHAR:
        zNum[0] = gh_scm2char( res );
        if ((zNum[0] == NUL) || (zNum[0] == '0'))
            abort = PROBLEM;
        zNum[1] = NUL;
        pzMsg = zNum;
        break;

    case GH_TYPE_STRING:
        pzMsg  = ag_scm2zchars( res, "error string" );
        msgLen = SCM_LENGTH( res );
        while (isspace( *pzMsg ) && (--msgLen > 0)) pzMsg++;

        /*
         *  IF the message starts with the number zero,
         *    OR the message is the empty string,
         *  THEN this is just a warning that is ignored
         */
        if (msgLen <= 0)
            abort = PROBLEM;
        else if (isdigit( *pzMsg ) && (strtol( pzMsg, NULL, 0 ) == 0))
            abort = PROBLEM;
        break;

    default:
        pzMsg = zBadMsg;
    }

    /*
     *  IF there is a message,
     *  THEN print it.
     */
    if (*pzMsg != NUL) {
        char* pz = aprf( zFmt, (abort != PROBLEM) ? zErr : zWarn,
                         pCurTemplate->pzFileName, pCurMacro->lineNo,
                         pCurFp->pzOutName, pzMsg );
        if (abort != PROBLEM)
            AG_ABEND( pz );
        fputs( pz, pfTrace );
        AGFREE( (void*)pz );
    }

    longjmp( fileAbort, abort );
    /* NOTREACHED */
    return SCM_UNDEFINED;
}


/*=gfunc gpl
 *
 * what:  GNU General Public License
 * general_use:
 *
 * exparg: prog-name, name of the program under the GPL
 * exparg: prefix, String for starting each output line
 *
 * doc:
 *
 *  Emit a string that contains the GNU General Public License.
 *  It takes two arguments:
 *  @code{prefix} contains the string to start each output line, and
 *  @code{prog_name} contains the name of the program the copyright is
 *  about.
 *
=*/
SCM
ag_scm_gpl( SCM prog_name, SCM prefix )
{
    char*   pzPfx  = ag_scm2zchars( prefix, "GPL line prefix" );
    char*   pzPrg  = ag_scm2zchars( prog_name, "program name" );
    char*   pzRes;
    SCM     res;

    /*
     *  Get the addresses of the program name and prefix strings.
     *  Make sure they are reasonably sized (<256 for program name
     *  and <128 for a line prefix).  Copy them to the scratch buffer.
     */
    if (SCM_LENGTH( prog_name ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zProgLen, 256 ));

    if (SCM_LENGTH( prefix ) >= 128)
        AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));

    /*
     *  Allocate-sprintf the result string, then put it in a new SCM.
     */
    pzRes = aprf( zGpl, pzPrg, pzPfx );

    res = gh_str02scm( pzRes );
    AGFREE( (void*)pzRes );
    return res;
}


/*=gfunc lgpl
 *
 * what:  GNU Library General Public License
 * general_use:
 *
 * exparg: prog_name, name of the program under the LGPL
 * exparg: owner, Grantor of the LGPL
 * exparg: prefix, String for starting each output line
 *
 * doc:
 *
 *  Emit a string that contains the GNU Library General Public License.
 *  It takes three arguments:  @code{prefix} contains the string to
 *  start each output line.  @code{owner} contains the copyright owner.
 *  @code{prog_name} contains the name of the program the copyright is about.
=*/
SCM
ag_scm_lgpl( SCM prog_name, SCM owner, SCM prefix )
{
    char*   pzPfx   = ag_scm2zchars( prefix, "GPL line prefix" );
    char*   pzPrg  = ag_scm2zchars( prog_name, "program name" );
    char*   pzOwner = ag_scm2zchars( owner, "owner" );
    char*   pzRes;
    SCM     res;

    /*
     *  Get the addresses of the program name prefix and owner strings.
     *  Make sure they are reasonably sized (<256 for program name
     *  and <128 for a line prefix).  Copy them to the scratch buffer.
     */
    if (SCM_LENGTH( prog_name ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zProgLen, 256 ));

    if (SCM_LENGTH( prefix ) >= 128)
        AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));

    if (SCM_LENGTH( owner ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zOwnLen, 256 ));

    /*
     *  Allocate-sprintf the result string, then put it in a new SCM.
     */
    pzRes = aprf( zLgpl, pzPrg, pzPfx, pzOwner );

    res = gh_str02scm( pzRes );
    AGFREE( (void*)pzRes );
    return res;
}


/*=gfunc bsd
 *
 * what:  BSD Public License
 * general_use:
 *
 * exparg: prog_name, name of the program under the BSD
 * exparg: owner, Grantor of the BSD License
 * exparg: prefix, String for starting each output line
 *
 * doc:
 *
 *  Emit a string that contains the Free BSD Public License.
 *  It takes three arguments:
 *  @code{prefix} contains the string to start each output line.
 *  @code{owner} contains the copyright owner.
 *  @code{prog_name} contains the name of the program the copyright is about.
 *
=*/
SCM
ag_scm_bsd( SCM prog_name, SCM owner, SCM prefix )
{
    char*   pzPfx   = ag_scm2zchars( prefix, "GPL line prefix" );
    char*   pzPrg  = ag_scm2zchars( prog_name, "program name" );
    char*   pzOwner = ag_scm2zchars( owner, "owner" );
    char*   pzRes;
    SCM     res;

    if (! (   gh_string_p( prog_name )
           && gh_string_p( owner )
           && gh_string_p( prefix )))
        return SCM_UNDEFINED;

    /*
     *  Get the addresses of the program name prefix and owner strings.
     *  Make sure they are reasonably sized (<256 for program name
     *  and <128 for a line prefix).  Copy them to the scratch buffer.
     */
    if (SCM_LENGTH( prog_name ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zProgLen, 256 ));

    if (SCM_LENGTH( prefix ) >= 128)
        AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));

    if (SCM_LENGTH( owner ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zOwnLen, 256 ));

    /*
     *  Allocate-sprintf the result string, then put it in a new SCM.
     */
    pzRes = aprf( zBsd, pzPrg, pzPfx, pzOwner );

    res = gh_str02scm( pzRes );
    AGFREE( (void*)pzRes );
    return res;
}


/*=gfunc license
 *
 * what:  an arbitrary license
 * general_use:
 *
 * exparg: lic_name, file name of the license
 * exparg: prog_name, name of the licensed program or library
 * exparg: owner, Grantor of the License
 * exparg: prefix, String for starting each output line
 *
 * doc:
 *  Emit a string that contains the named license.  The license text
 *  is read from a file named, @code{lic_name}.lic, searching the standard
 *  directories.  The file contents are used as a format argument
 *  to @code{printf}(3), with @code{prog_name} and @code{owner} as
 *  the two string formatting arguments.  Each output line is automatically
 *  prefixed with the string @code{prefix}.
=*/
SCM
ag_scm_license( SCM license, SCM prog_name, SCM owner, SCM prefix )
{
    char*   pzPfx   = ag_scm2zchars( prefix, "GPL line prefix" );
    char*   pzPrg  = ag_scm2zchars( prog_name, "program name" );
    char*   pzOwner = ag_scm2zchars( owner, "owner" );
    static tMapInfo mi = { NULL, 0, 0, NULL };

    char*     pzRes;
    SCM       res;

    if (! gh_string_p( license ))
        return SCM_UNDEFINED;

    {
        char* pzLicense = ag_scm2zchars( license, "license file name" );

        /*
         *  Set the current file name.
         *  If it changes, then unmap the old data
         */
        if (mi.pzFileName == NULL)
            mi.pzFileName = pzLicense;

        else if (strcmp( mi.pzFileName, pzLicense ) != 0) {
            munmap( mi.pData, mi.size );
            close( mi.fd );
            mi.pData = NULL;
            AGFREE( (void*)mi.pzFileName );
            mi.pzFileName = pzLicense;
        }
    }

    /*
     *  Make sure the data are loaded and trim any white space.
     *  If the data pointer is NULL, then we have put a Guile-allocated
     *  string pointer into mi.pzFileName.  "mapDataFile" will insert
     *  an AGSTRDUP-ed string there.
     */
    if (mi.pData == NULL) {
        char* pz;
        tSCC*  apzSfx[] = { "lic", NULL };

        mapDataFile( mi.pzFileName, &mi, apzSfx );

        pz = (char*)mi.pData + mi.size - 1;
        while (isspace( pz[-1] )) pz--;
        *pz = NUL;
    }

    /*
     *  Get the addresses of the program name prefix and owner strings.
     *  Make sure they are reasonably sized (<256 for program name
     *  and <128 for a line prefix).  Copy them to the scratch buffer.
     */
    if (SCM_LENGTH( prog_name ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zProgLen, 256 ));

    if (SCM_LENGTH( prefix ) >= 128)
        AG_ABEND( aprf( zPfxMsg, zPfxLen, 128 ));

    if (SCM_LENGTH( owner ) >= 256)
        AG_ABEND( aprf( zPfxMsg, zOwnLen, 256 ));

    /*
     *  Reformat the string with the given arguments
     */
    pzRes = aprf( (char*)mi.pData, pzPrg, pzOwner );
    {
        int     pfx_size = strlen( pzPfx );
        char*   pzScan   = pzRes;
        char*   pzOut;
        size_t  out_size = pfx_size;

        /*
         *  Figure out how much space we need (text size plus
         *  a prefix size for each newline)
         */
        for (;;) {
            switch (*(pzScan++)) {
            case NUL:
                goto exit_count;
            case '\n':
                out_size += pfx_size;
                /* FALLTHROUGH */
            default:
                out_size++;
            }
        } exit_count:;

        /*
         *  Create our output buffer and insert the first prefix
         */
        res    = scm_makstr( out_size, 0 );
        pzOut  = SCM_CHARS( res );
        strcpy( pzOut, pzPfx );
        pzOut += pfx_size;
        pzScan = pzRes;

        for (;;) {
            switch (*(pzOut++) = *(pzScan++)) {
            case NUL:
                goto exit_copy;

            case '\n':
                strcpy( pzOut, pzPfx );
                pzOut += pfx_size;
                break;

            default:
                break;
            }
        }
    } exit_copy:;

    /*
     *  We allocated a temporary buffer that has all the formatting done,
     *  but need the prefixes on each line.
     */
    AGFREE( (void*)pzRes );

    return res;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expFormat.c */
