
/**
 *  \file expFormat.c
 *
 *  Time-stamp:        "2010-07-11 12:54:17 bkorb"
 *
 *  This module implements formatting expression functions.
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

static char const zGpl[] =
"%2$s%1$s is free software: you can redistribute it and/or modify it\n"
"%2$sunder the terms of the GNU General Public License as published by the\n"
"%2$sFree Software Foundation, either version 3 of the License, or\n"
"%2$s(at your option) any later version.\n%2$s\n"
"%2$s%1$s is distributed in the hope that it will be useful, but\n"
"%2$sWITHOUT ANY WARRANTY; without even the implied warranty of\n"
"%2$sMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"%2$sSee the GNU General Public License for more details.\n%2$s\n"
"%2$sYou should have received a copy of the GNU General Public License along\n"
"%2$swith this program.  If not, see <http://www.gnu.org/licenses/>.";

static char const zAgpl[] =
"%2$s%1$s is free software: you can redistribute it and/or modify it\n"
"%2$sunder the terms of the GNU General Public License as published by the\n"
"%2$sFree Software Foundation, either version 3 of the License, or\n"
"%2$s(at your option) any later version.\n%2$s\n"
"%2$s%1$s is distributed in the hope that it will be useful, but\n"
"%2$sWITHOUT ANY WARRANTY; without even the implied warranty of\n"
"%2$sMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"%2$sSee the GNU Affero General Public License for more details.\n%2$s\n"
"%2$sYou should have received a copy of the GNU Affero General Public License"
"\n%2$salong with this program.  If not, see <http://www.gnu.org/licenses/>.";

static char const zLgpl[] =
"%2$s%1$s is free software: you can redistribute it and/or modify it\n"
"%2$sunder the terms of the GNU Lesser General Public License as published\n"
"%2$sby the Free Software Foundation, either version 3 of the License, or\n"
"%2$s(at your option) any later version.\n%2$s\n"
"%2$s%1$s is distributed in the hope that it will be useful, but\n"
"%2$sWITHOUT ANY WARRANTY; without even the implied warranty of\n"
"%2$sMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"%2$sSee the GNU Lesser General Public License for more details.\n%2$s\n"
"%2$sYou should have received a copy of the GNU Lesser General Public License"
"\n%2$salong with this program.  If not, see <http://www.gnu.org/licenses/>.";

static char const zBsd[] =
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

static char const zDne1[] =
"%s -*- buffer-read-only: t -*- vi: set ro:\n"
"%s\n";

static char const zDne[] = "%6$s"
"%1$sDO NOT EDIT THIS FILE   (%2$s)\n"
"%1$s\n"
"%1$sIt has been AutoGen-ed%3$s\n"
"%1$sFrom the definitions    %4$s\n"
"%1$sand the template file   %5$s";

static char const zDne2[] = "%6$s"
"%1$sEDIT THIS FILE WITH CAUTION  (%2$s)\n"
"%1$s\n"
"%1$sIt has been AutoGen-ed%3$s\n"
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
 * doc:
 *  Generate a "DO NOT EDIT" or "EDIT WITH CARE" warning string.
 *  Which depends on whether or not the @code{--writable} command line
 *  option was set.
 *
 *  The first argument may be an option:  -d
 *
 *  This will suppress the variable text (date and version information).
 *  If specified, then the "prefix" and "first" arguments are shifted
 *  to the next arguments.
 *
 *  The first argument is a per-line string prefix.  The optional second
 *  argument is a prefix for the first-line and, in read-only mode, activates
 *  the editor hints.
 *  @*
 *  @example
 *  -*- buffer-read-only: t -*- vi: set ro:
 *  @end example
 *  @noindent
 *  The warning string also includes information about the template used
 *  to construct the file and the definitions used in its instantiation.
 *
 *  The optional third argument is used when the first argument is actually an
 *  invocation option and the prefix arguments get shifted.  The first
 *  argument must be, specifically, "@code{-d}".  That is used to signify that
 *  the date stamp should not be inserted into the output.
=*/
SCM
ag_scm_dne(SCM prefix, SCM first, SCM opt)
{
    int      noDate   = 0;
    char     zScribble[ SCRIBBLE_SIZE ];
    char*    pzRes;
    tCC*     pzFirst  = zNil;
    SCM      res;
    size_t   pfxLen   = AG_SCM_STRLEN(prefix);
    tCC*     pzPrefix = ag_scm2zchars(prefix, "dne-prefix");

    /*
     *  Check for the ``-d'' option
     */
    if ((pfxLen == 2) && (strncmp(pzPrefix, "-d", (size_t)2) == 0)) {
        noDate   = 1;
        pfxLen   = AG_SCM_STRLEN(first);
        pzPrefix = ag_scm2zchars(first, "dne-prefix");
        first    = opt;
    }

    if (pfxLen > SCRIBBLE_SIZE )
        AG_ABEND(aprf(zPfxMsg, zPfxLen, SCRIBBLE_SIZE));

    /*
     *  IF we also have a 'first' prefix string,
     *  THEN we set it to something other than ``zNil'' and deallocate later.
     */
    if (AG_SCM_STRING_P(first)) {
        int len = AG_SCM_STRLEN(first);
        if (len >= SCRIBBLE_SIZE)
            AG_ABEND(aprf(zPfxMsg, zPfxLen, SCRIBBLE_SIZE));

        pzFirst = aprf(ENABLED_OPT(WRITABLE) ? "%s\n" : zDne1,
                       ag_scm2zchars(first, "first-prefix"), pzPrefix);
    }

    if (noDate) {
        zScribble[0] = NUL;
    } else {
        static char const tim_fmt[] =
            "  %B %e, %Y at %r by AutoGen " AUTOGEN_VERSION;
        time_t    curTime = time(NULL);
        struct tm*  pTime = localtime(&curTime);
        strftime(zScribble, (size_t)SCRIBBLE_SIZE, tim_fmt, pTime);
    }

    {
        char const* pz;
        tFpStack*   pfp = pCurFp;

        while (pfp->flags & FPF_UNLINK)  pfp = pfp->pPrev;
        if (! ENABLED_OPT(DEFINITIONS))
            pz = "<<no definitions>>";

        else if (*pzOopsPrefix != NUL)
            pz = "<<CGI-definitions>>";

        else {
            pz = OPT_ARG(DEFINITIONS);
            if (strcmp(pz, "-") == 0)
                pz = "stdin";
        }

        pzRes = aprf(ENABLED_OPT(WRITABLE) ? zDne2 : zDne,
                     pzPrefix, pfp->pzOutName, zScribble,
                     pz, pzTemplFileName, pzFirst);
    }

    if (pzRes == NULL)
        AG_ABEND("Allocating Do-Not-Edit string");

    res = AG_SCM_STR02SCM(pzRes);

    /*
     *  Deallocate any temporary buffers.  pzFirst either points to
     *  the zNil string, or to an allocated buffer.
     */
    AGFREE((void*)pzRes);
    if (pzFirst != zNil)
        AGFREE((void*)pzFirst);

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
ag_scm_error(SCM res)
{
    tSCC      zFmt[]    = "DEFINITIONS %s in %s line %d for %s:\n\t%s\n";
    tSCC      zErr[]    = "ERROR";
    tSCC      zWarn[]   = "Warning";
    tSCC      zBadMsg[] = "??? indecipherable error message ???";
    tCC*      pzMsg;
    tSuccess  abrt = FAILURE;
    char      zNum[16];
    int       msgLen;

    switch (gh_type_e(res)) {
    case GH_TYPE_BOOLEAN:
        if (SCM_FALSEP(res))
            abrt = PROBLEM;
        pzMsg = zNil;
        break;

    case GH_TYPE_NUMBER:
    {
        unsigned long val = gh_scm2ulong(res);
        if (val == 0)
            abrt = PROBLEM;
        snprintf(zNum, sizeof(zNum), "%d", (int)val);
        pzMsg = zNum;
        break;
    }

    case GH_TYPE_CHAR:
        zNum[0] = gh_scm2char(res);
        if ((zNum[0] == NUL) || (zNum[0] == '0'))
            abrt = PROBLEM;
        zNum[1] = NUL;
        pzMsg = zNum;
        break;

    case GH_TYPE_STRING:
        pzMsg  = ag_scm2zchars(res, "error string");
        msgLen = AG_SCM_STRLEN(res);
        while (IS_WHITESPACE_CHAR(*pzMsg) && (--msgLen > 0)) pzMsg++;

        /*
         *  IF the message starts with the number zero,
         *    OR the message is the empty string,
         *  THEN this is just a warning that is ignored
         */
        if (msgLen <= 0)
            abrt = PROBLEM;
        else if (IS_DEC_DIGIT_CHAR(*pzMsg) && (strtol(pzMsg, NULL, 0) == 0))
            abrt = PROBLEM;
        break;

    default:
        pzMsg = zBadMsg;
    }

    /*
     *  IF there is a message,
     *  THEN print it.
     */
    if (*pzMsg != NUL) {
        char* pz = aprf(zFmt, (abrt != PROBLEM) ? zErr : zWarn,
                        pCurTemplate->pzTplFile, pCurMacro->lineNo,
                        pCurFp->pzOutName, pzMsg);
        if (abrt != PROBLEM)
            AG_ABEND(pz);
        fputs(pz, pfTrace);
        AGFREE((void*)pz);
    }

    longjmp(fileAbort, abrt);
    /* NOTREACHED */
    return SCM_UNDEFINED;
}

static SCM
mk_license(SCM prog, SCM pfx, char const * lic, char const * owner)
{
    /*
     *  Get the addresses of the program name prefix and owner strings.
     *  Make sure they are reasonably sized (less than
     *  SCRIBBLE_SIZE).  Copy them to the scratch buffer.
     */
    if (AG_SCM_STRLEN(prog) >= SCRIBBLE_SIZE)
        AG_ABEND(aprf(zPfxMsg, zProgLen, SCRIBBLE_SIZE));

    if (AG_SCM_STRLEN(pfx) >= SCRIBBLE_SIZE)
        AG_ABEND(aprf(zPfxMsg, zPfxLen, SCRIBBLE_SIZE));

    {
        char const * pzPfx = ag_scm2zchars(pfx,  "lic prefix");
        char const * pzPrg = ag_scm2zchars(prog, "prog name");
        char const * pzRes = aprf(lic, pzPrg, pzPfx);
        SCM res = AG_SCM_STR02SCM(pzRes);

        AGFREE((void*)pzRes);
        return res;
    }
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
=*/
SCM
ag_scm_gpl(SCM prog_name, SCM prefix)
{
    return mk_license(prog_name, prefix, zGpl, zNil);
}


/*=gfunc agpl
 *
 * what:  GNU Affero General Public License
 * general_use:
 *
 * exparg: prog-name, name of the program under the GPL
 * exparg: prefix, String for starting each output line
 *
 * doc:
 *
 *  Emit a string that contains the GNU Affero General Public License.
 *  It takes two arguments:
 *  @code{prefix} contains the string to start each output line, and
 *  @code{prog_name} contains the name of the program the copyright is
 *  about.
=*/
SCM
ag_scm_agpl(SCM prog_name, SCM prefix)
{
    return mk_license(prog_name, prefix, zAgpl, zNil);
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
ag_scm_lgpl(SCM prog_name, SCM owner, SCM prefix)
{
    char const * own = ag_scm2zchars(owner, "owner");
    return mk_license(prog_name, prefix, zLgpl, own);
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
ag_scm_bsd(SCM prog_name, SCM owner, SCM prefix)
{
    char const * own = ag_scm2zchars(owner, "owner");
    return mk_license(prog_name, prefix, zBsd, own);
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
ag_scm_license(SCM license, SCM prog_name, SCM owner, SCM prefix)
{
    char const * pzPfx   = ag_scm2zchars(prefix,    "GPL line prefix");
    char const * pzPrg   = ag_scm2zchars(prog_name, "program name");
    char const * pzOwner = ag_scm2zchars(owner,     "owner");
    static struct {
        tCC*    pzFN;
        tmap_info_t mi;
    } lic = { NULL, { NULL, 0, 0, 0, 0, 0, 0, 0 }};

    char*     pzRes;

    if (! AG_SCM_STRING_P(license))
        return SCM_UNDEFINED;

    {
        tSCC*  apzSfx[] = { "lic", NULL };
        static char zRealFile[ AG_PATH_MAX ];
        char const * pzLicense = ag_scm2zchars(license, "license file");

        /*
         *  Find the template file somewhere
         */
        if (! SUCCESSFUL(findFile(pzLicense, zRealFile, apzSfx, NULL))) {
            errno = ENOENT;
            AG_CANT("map license file", pzLicense);
        }

        if ((lic.pzFN != NULL) && (strcmp(zRealFile, lic.pzFN) != 0)) {
            text_munmap(&lic.mi);
            AGFREE((void*)lic.pzFN);
            lic.pzFN = NULL;
        }

        if (lic.pzFN == NULL) {
            text_mmap(zRealFile, PROT_READ|PROT_WRITE, MAP_PRIVATE, &lic.mi);
            if (TEXT_MMAP_FAILED_ADDR(lic.mi.txt_data))
                AG_ABEND(aprf("Could not open license file '%s'", pzLicense));

            if (pfDepends != NULL)
                append_source_name(pzLicense);

            AGDUPSTR(lic.pzFN, zRealFile, "license file name");
        }
    }

    /*
     *  Trim trailing white space.
     */
    {
        char* pz = (char*)lic.mi.txt_data + lic.mi.txt_size;
        while (IS_WHITESPACE_CHAR(pz[-1]) && (pz > (char*)lic.mi.txt_data))
            pz--;
        *pz = NUL;
    }

    /*
     *  Get the addresses of the program name prefix and owner strings.
     *  Make sure they are reasonably sized (less than
     *  SCRIBBLE_SIZE).  Copy them to the scratch buffer.
     */
    if (AG_SCM_STRLEN(prog_name) >= SCRIBBLE_SIZE)
        AG_ABEND(aprf(zPfxMsg, zProgLen, SCRIBBLE_SIZE));

    if (AG_SCM_STRLEN(prefix) >= SCRIBBLE_SIZE)
        AG_ABEND(aprf(zPfxMsg, zPfxLen, SCRIBBLE_SIZE));

    if (AG_SCM_STRLEN(owner) >= SCRIBBLE_SIZE)
        AG_ABEND(aprf(zPfxMsg, zOwnLen, SCRIBBLE_SIZE));

    /*
     *  Reformat the string with the given arguments
     */
    pzRes = aprf((char*)lic.mi.txt_data, pzPrg, pzOwner);
    {
        int     pfx_size = strlen(pzPfx);
        char*   pzScan   = pzRes;
        char*   pzOut;
        char*   pzSaveRes;
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
        pzOut = pzSaveRes = ag_scribble(out_size);

        strcpy(pzOut, pzPfx);
        pzOut += pfx_size;
        pzScan = pzRes;

        for (;;) {
            switch (*(pzOut++) = *(pzScan++)) {
            case NUL:
                goto exit_copy;

            case '\n':
                strcpy(pzOut, pzPfx);
                pzOut += pfx_size;
                break;

            default:
                break;
            }
        }
    exit_copy:;

        /*
         *  We allocated a temporary buffer that has all the
         *  formatting done, but need the prefixes on each line.
         */
        AGFREE((void*)pzRes);

        return AG_SCM_STR2SCM(pzSaveRes, (size_t)((pzOut - pzSaveRes) - 1));
    }
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expFormat.c */
