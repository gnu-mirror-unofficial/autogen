
/**
 * @file cright-update.c
 *
 *  cright-update Copyright (C) 1992-2018 by Bruce Korb - all rights reserved
 *
 * cright-update is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * cright-update is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define  _GNU_SOURCE   1
#define  _XOPEN_SOURCE 600

#include <sys/stat.h>
#include <sys/types.h>

#include "opts.h"
#define  option_data cright_updateOptions

#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define  CRIGHT_UPDATE 1
#include "cright-cmap.h"
#include "collapse-fsm.h"

#ifndef NUL
#define NUL '\0'
#endif

#ifndef NL
#define NL  '\n'
#endif

#define SQUOT '\''
#define DQUOT '"'
#define BACKS '\\'

#define RIDICULOUS_YEAR_LEN 8192

static char * scan_buffer  = NULL;
static char * compress_buf = NULL;

static char const was_updt[]   = "updated";
static char const not_updt[]   = "up-to-date";
static char const needs_updt[] = "out-of-date";
static char const bad_date[]   = "bad date";
static char const no_match[]   = "no cright";
static char const report_fmt[] = "%-11s %s\n";

char * last_year = NULL;
unsigned long first_yr = 0;

static int const regex_flags =
    REG_EXTENDED | REG_ICASE | REG_NEWLINE;

/**
 * find the line prefix before the current copyright mark.
 * When collecting year ranges, these get skipped over.
 *
 * @param ftext  the text of the file
 * @param off    the offset of the "Copyright (C)" mark
 * @returns an allocated "prefix" string
 */
static char *
find_prefix(char const * ftext, regoff_t off)
{
    char const * const cright_mark = ftext + off;
    char const * scan;
    char const * end = SPN_SPACY_NAME_BACK(ftext, cright_mark);

    /*
     * "end" has trimmed off any trailing names by backing up over
     * name-looking sequences, but then going back forward over
     * leading horizontal white space.
     */
    scan = end = SPN_HORIZ_WHITE_CHARS(end);

    /*
     * Find the start of the line.  Make sure "end" follows any "dnl".
     */
    while ((scan > ftext) && (scan[-1] != NL))    scan--;

    /*
     * "end" now points to the the end of the candidate prefix
     * and "scan" points to the start.  If they point to the same place
     * and if the copyright mark is beyond the beginning of the line,
     * then we have to decide if the string at the start of the line is
     * a name and the prefix is zero length, or if the name-like thing
     * is actually a comment indicator.  Presume a comment if the "name"
     * is either "rem" or "dnl" ("remark" or "Delete to New Line").
     */
    if (  (end == scan)
       && ((strncmp(scan, "dnl", 3) == 0) || (strncmp(scan, "rem", 3) == 0))
       && IS_HORIZ_WHITE_CHAR(scan[4])
       && (end < cright_mark)) {

        end += 4;
        end = SPN_HORIZ_WHITE_CHARS(end);
        if (end > cright_mark)
            end = cright_mark;
    }

    {
        size_t len = end - scan;
        char * pz  = malloc(len + 1);
        if (pz == NULL)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "copyright prefix");
        if (len > 0)
            memcpy(pz, scan, len);
        pz[len] = NUL;

        return pz;
    }
}

/**
 * Emit new copyright years.  newlines ane prefixes are inserted as needed.
 *
 * @param fp        output file pointer
 * @param list      the list of new years
 * @param cur_col   our current column number
 * @param pfx       the line separation prefix
 * @param plen      the length of the line prefix
 * @param txt       where to store the resumption point
 * @param tlen      the current remaining length
 */
static void
emit_years(FILE * fp, char const * list, int cur_col, char const * pfx,
           size_t plen, char ** txt)
{
    cright_update_exit_code_t res = CRIGHT_UPDATE_EXIT_SUCCESS;

    char * pe;
    bool   need_sp = false;

    if (OPT_VALUE_WIDTH == 0) {
        fputs(list, fp);
        return;
    }

    while (pe = strchr(list, ' '),
           pe != NULL) {
        size_t len = pe - list;
        cur_col += len + 1;

        if (cur_col > OPT_VALUE_WIDTH) {
            fprintf(fp, "\n%s", pfx);
            cur_col = plen + len;
            need_sp = false;
        }

        if (need_sp)
            putc(' ', fp);

        fwrite(list, len, 1, fp);
        need_sp = true;
        list    = SPN_WHITESPACE_CHARS(list + len + 1);
    }

    if (need_sp)
        putc(' ', fp);
    fputs(list, fp);

    {
        char * rest = *txt;
        pe = strchr(rest, NL);

        /*
         * See if we need a line split.  If not, we just resume.
         */
        if (pe != NULL) {
            size_t len = pe - rest;
            if (cur_col + len > OPT_VALUE_WIDTH)
                fprintf(fp, "\n%s", pfx);
        }

        *txt   = (char *)rest;
    }
}

/**
 * check copyright ownership.  The --owner string must match, though
 * the "by " clause is skipped both in ownership and input text.
 *
 * @param[in] scan  current scan point
 * @param[in] pfx   the copyright statement prefix
 *                  (ownership may be on the next line)
 * @param[in] p_len prefix length
 *
 * @returns true (it is okay) or false (not).
 */
static bool
ownership_ok(char const * scan, char const * pfx, size_t p_len)
{
    char const * owner = OPT_ARG(OWNER);

    /*
     * IF we are ignoring ownership, ...
     */
    if ((owner == NULL) || (*owner == NUL)) {
        if (HAVE_OPT(VERBOSE))
            fputs("ownership ignored\n", stderr);
        return true;
    }

    {
        static char const by[] = "by ";
        static int  const byln = sizeof(by) - 1;
        if (strncmp(owner, by, byln) == 0)
            owner = SPN_HORIZ_WHITE_CHARS(owner + byln);

        if (strncmp(scan, by, byln) == 0)
            scan = SPN_HORIZ_WHITE_CHARS(scan + byln);
    }

    do  {
        /*
         * If there is a space character in the ownership, then there
         * must be one in the scan, too.  If not, then neither may have
         * a space character.  The number of space characters is ignored.
         */
        if (IS_WHITESPACE_CHAR(*owner)) {
            owner = SPN_HORIZ_WHITE_CHARS(owner);
            if (*owner == NUL)
                break;
            if (! IS_WHITESPACE_CHAR(*scan))
                return false;

            scan = SPN_HORIZ_WHITE_CHARS(scan);

            /*
             * If the input text has a newline, skip it and the required
             * following prefix and any white space after that.
             * At that point, the owner name must continue.  No newline.
             */
            if (*scan == NL) {
               if (strncmp(scan + 1, pfx, p_len) != 0)
                   return false;
               scan = SPN_HORIZ_WHITE_CHARS(scan + p_len + 1);
               if (*scan == NL)
                   return false;
            }
        }

        /*
         * The owner character is not white space and not NUL,
         * the input text character must match.
         */
        if (*(owner++) != *(scan++))
            return false;
    } while (*owner != NUL);
    return true;
}

/**
 * Find the next token in the year list string.  Used in the FSM.
 *
 * @param[in,out] plist  pointer to pointer scanning the year list
 * @param[in,out] val    previous year value we replace with current year
 *
 * @returns the token type found
 */
te_cyr_event
get_next_token(char ** plist, unsigned long * val)
{
    char * p = SPN_WHITESPACE_CHARS(*plist);
    *plist = p + 1;

    switch (*p) {
    case ',':
    {
        unsigned long v;
        errno = 0;
        v = strtoul(*plist, NULL, 10);
        if ((errno != 0) || (v < 1900) || (v > 3000))
            return CYR_EV_INVALID;
        return (v == (*val + 1)) ? CYR_EV_COMMA_SEQ : CYR_EV_COMMA;
    }
    case '-': return CYR_EV_HYPHEN;
    case NUL: return CYR_EV_END;
    default:  return CYR_EV_INVALID;
    case '0' ... '9':
        break;
    }

    /*
     * We have a year, so parse it.
     */
    errno = 0;
    *val  = strtoul(p, &p, 10);
    if (errno != 0) return CYR_EV_INVALID;
    if ((*val >= 70) && (*val <= 99)) {
        fprintf(stderr, "found two digit year:  %lu\n", *val);
        *val += 1900;
    } else if ((*val < 1900) || (*val > 3000))
        return CYR_EV_INVALID;
    *plist = SPN_WHITESPACE_CHARS(p);

    if (first_yr == 0)
        first_yr = *val;
    return CYR_EV_YEAR;
}

/**
 * skip over a sequence of years
 */
static char *
scan_past_years(char * scan, char const * pfx, size_t p_len)
{
    while (IS_YEAR_LIST_CHAR(*scan)) {
        if (! IS_WHITESPACE_CHAR(*scan)) {
            scan++;
            continue;

        } else if (*scan == NL) {
            /*
             *  Newline.  Skip over the newline, the line prefix and
             *  any remaining white space on the line.  Another newline,
             *  and we bail out.  We must find something on this new line.
             */
            char * p = scan;
            while ((*p == NL) && (strncmp(p + 1, pfx, p_len) == 0)) {
                p += p_len + 1;
                p = SPN_WHITESPACE_CHARS(p);
            }
            if (*p == NL)
                break;
            scan = p;

        } else {
            scan = SPN_HORIZ_WHITE_CHARS(scan);
        }
    }

    return scan;
}

/**
 * Pull out the copyright years.  When newlines are encountered,
 * a "prefix" also gets skipped in the process.
 *
 * @param ftext       address of caller's text scan pointer
 * @param col         place to tell caller what column he's in
 * @param pfx         the per-line prefix to strip
 * @param p_len       the length of the prefix
 *
 * @returns the allocated buffer with extracted years.
 * It will always have 8 + strlen(prefix) extra bytes at the end
 */
static char *
extract_years(char ** ftext, int * col, char const * pfx,
              size_t p_len)
{
    char * scanin = *ftext;
    char * resbuf, * scanout, * bfend;
    bool   was_sp;

    {   // figure out what column we are in.  Assume no tabs.
        int cc = 0;
        char * p = scanin;
        while (*--p != NL)  cc++;
        *col = cc;
    }

    {   // skip over white space, both locally and for caller
        char * next  = SPN_WHITESPACE_CHARS(scanin);
        long   delta = next - scanin;
        if (delta > 0)
            *ftext  = scanin = next;
    }

    {
        char * end  = scan_past_years(scanin, pfx, p_len);
        size_t blen = (end - scanin) + p_len + 8;
        resbuf = scanout = malloc(blen);
        if (scanout == NULL)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "year buffer");
        bfend = scanout + blen;
    }

    /*
     *  Copy over the copyright years
     */
    was_sp = false;
    while (IS_YEAR_LIST_CHAR(*scanin)) {
        if (! IS_WHITESPACE_CHAR(*scanin)) {
            *(scanout++) = *(scanin++);
            was_sp   = false;

        } else if (*scanin == NL) {
            /*
             *  Newline.  Skip over the newline, the line prefix and
             *  any remaining white space on the line.  Another newline,
             *  and we bail out.  We must find something on this new line.
             */
            char * p = scanin;
            while ((*p == NL) && (strncmp(p + 1, pfx, p_len) == 0)) {
                p += p_len + 1;
                p = SPN_WHITESPACE_CHARS(p);
            }
            if (*p == NL)
                break;

            if (! was_sp) {
                *(scanout++) = ' ';
                was_sp   = true;
            }
            scanin = p;

        } else {
            scanin = SPN_HORIZ_WHITE_CHARS(scanin);
            if (! was_sp) {
                *(scanout++) = ' ';
                was_sp   = true;
            }
        }

        if (scanout >= bfend)
            return NULL;
    }

    /*
     * If we are worrying about ownership, it should be next in the
     * input stream.  Check that it matches.
     */
    if (! ownership_ok(scanin, pfx, p_len)) {
        fprintf(stderr, "ownership failed: %s\n", OPT_ARG(OWNER));
        return NULL;
    }

    /*
     * back up over any trailing year separation characters
     */
    *ftext   = SPN_YEAR_SEP_BACK(*ftext, scanin);
    scanout  = SPN_YEAR_SEP_BACK(resbuf, scanout);
    *scanout = NUL;

    /*
     * IF --join-years was not specified or if --no-join-years was specified,
     * THEN do not compress the year list.
     */
    if ((! HAVE_OPT(JOIN_YEARS)) || (! ENABLED_OPT(JOIN_YEARS)))
        return resbuf;

    return (cyr_run_fsm(resbuf) == CYR_ST_DONE) ? resbuf : NULL;
}

/**
 * Change copyright years.  The input buffer contains the old list and
 * is allocated to be large enough that it can always contain the final
 * result.
 *
 * @param[in,out] yrbuf in: the original copyright years,
 *                      without any newlines or prefixes.
 *                      out: the updated list.
 * @returns the cright-update exit code (SUCCESS, BAD_YEAR or NO_UPDATE).
 */
static cright_update_exit_code_t
change_years(char * yrbuf)
{
    /*
     * We have a valid list of years.  If we have a range that ends with last
     * year, replace it with the current year.  Otherwise, append the new year.
     */
    static char const no_room[] = "OOPS: no room for new year\n";
    char * pz = yrbuf + strlen(yrbuf);

    /*
     * If we don't have any years, then just supply the current one.
     */
    if (pz < yrbuf + 4) {
        strcpy(yrbuf, OPT_ARG(NEW_YEAR));
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }

    if (WHICH_OPT_JOIN_YEARS == VALUE_OPT_COLLAPSE_YEARS) {
        char yr_rng[16];
        int cmp_val;
        snprintf(yr_rng, 16, "%4.4lu-%s", first_yr, OPT_ARG(NEW_YEAR));
        cmp_val = strncmp(yr_rng, yr_rng+5, 4);
        if (cmp_val == 0)
            yr_rng[4] = NUL;
        else if (cmp_val > 0) {
            fprintf(stderr, "start year follows current year:  "
                    "%4.4lu > %s\n", first_yr, OPT_ARG(NEW_YEAR));
            return CRIGHT_UPDATE_EXIT_BAD_YEAR;
        }
        if (strcmp(yr_rng, yrbuf) == 0)
            goto already_done;
        strcpy(yrbuf, yr_rng);
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }

    /*
     * If the new year is already there, skip this one.
     */
    if (strcmp(pz - 4, OPT_ARG(NEW_YEAR)) == 0) {
    already_done:
        if (HAVE_OPT(VERBOSE))
            fputs("already updated\n", stderr);
        return CRIGHT_UPDATE_EXIT_NO_UPDATE;
    }

    /*
     * IF --no-join-years was specified, append year with a comma
     * If a year was missed, then append the year with a comma
     */
    if (  ! ENABLED_OPT(JOIN_YEARS)
       || (strcmp(pz - 4, last_year) != 0)) {
        *(pz++) = ',';
        *(pz++) = ' ';
        strcpy(pz, OPT_ARG(NEW_YEAR));
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }

    /*
     * If last year was the end of a range, replace the year
     */
    if ((pz >= yrbuf + 5) && (pz[-5] == '-')) {
        strcpy(pz - 4, OPT_ARG(NEW_YEAR));
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }

    /*
     * Last year was start of new year range
     */
    *(pz++) = '-';
    strcpy(pz, OPT_ARG(NEW_YEAR));
    return CRIGHT_UPDATE_EXIT_SUCCESS;
}

/**
 * Fix up copyright years (maybe).  The "Copyright (C)" RE has been found,
 * as has the "prefix" that precedes it.  The years get pulled out in a
 * canonical form and that is scanned to see if an update is needed.
 * If so, the update is performed and we will return SUCCESS.
 *
 * @param ftext pointer to scan pointer in file text
 * @param pfx   the prefix to scan over for each line
 * @param p_len the length of per-line prefix
 * @param fp    the output file pointer
 *
 * @returns the cright-update exit code (SUCCESS or other)
 */
static cright_update_exit_code_t
fixup(char ** ftext, char const * pfx, size_t p_len, FILE * fp)
{
    int    col;
    char * scan = extract_years(ftext, &col, pfx, p_len);
    cright_update_exit_code_t res;

    if (scan == NULL) {
        if (HAVE_OPT(VERBOSE))
            fputs("no years found in copyright\n", stderr);
        return CRIGHT_UPDATE_EXIT_BAD_YEAR;
    }

    if (HAVE_OPT(DIFF)) {
        char * ori = strdup(scan);
        res = change_years(scan);
        if (res == CRIGHT_UPDATE_EXIT_SUCCESS)
            printf("=== replacing:\n\t%s\n\nwith:\n\t%s\n===\n", ori, scan);
        free(ori);
    } else
        res = change_years(scan);

    if (res == CRIGHT_UPDATE_EXIT_SUCCESS)
        emit_years(fp, scan, col, pfx, p_len, ftext);
    return res;
}

/**
 * See if a file is a C or C++ file, based only on the suffix.
 * "C" suffixes are ".c" and ".h", with or without "xx" or "++" following.
 * Upper and lower case versions are accepted, but all letters must be
 * all one case.
 *
 * @param[in] fname  the file name to test
 * @returns true or false, depending.
 */
static bool
is_c_file_name(char const * fname)
{
    /*
     * Find the "." introducing the suffix.  (The last period.)
     */
    fname = strrchr(fname, '.');
    if (fname == NULL)
        return false;

    switch (fname[1]) {
    case 'h':
    case 'c':
        if (fname[2] == NUL)
            return true;
        if (strcmp(fname + 2, "pp") == 0)
            return true;
        if (strcmp(fname + 2, "xx") == 0)
            return true;
        return strcmp(fname + 2, "++") == 0;

    case 'H':
    case 'C':
        if (fname[2] == NUL)
            return true;
        if (strcmp(fname + 2, "XX") == 0)
            return true;
        return strcmp(fname + 2, "++") == 0;

    default:
        return false;
    }
}

/**
 * Move the result file into place.  Adjust the time stamp on the output
 * file to match the original.  We ought not have really changed anything.
 * Also, if there is a --backup option specified, then save the original
 * file to the same name, but with the backup string appended.
 *
 * @param fname the file we are updating
 * @param tname the temporary name we opened for output
 */
static void
mv_file(char const * fname, char const * tname)
{
    static mode_t const mode_mask = S_IRWXU | S_IRWXG | S_IRWXO;

    {
        struct utimbuf utbf = { .actime   = time(NULL) };
        struct stat sb;

        if (stat(fname, &sb) != 0)
            fserr(CRIGHT_UPDATE_EXIT_FSERR, "stat", fname);

        if (chmod(tname, sb.st_mode & mode_mask) != 0)
            fserr(CRIGHT_UPDATE_EXIT_FSERR, "chmod", fname);

        utbf.modtime = sb.st_mtime;
        utime(tname, &utbf);
    }

    if (HAVE_OPT(BACKUP)) {
        size_t len = strlen(fname) + strlen(OPT_ARG(BACKUP)) + 2;
        char * p   = malloc(len);
        if (p == NULL)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "file name");
        sprintf(p, "%s%s", fname, OPT_ARG(BACKUP));
        if (rename(fname, p) != 0)
            fserr(CRIGHT_UPDATE_EXIT_FSERR, "renaming backup file", fname);
        free(p);
    }

    if (rename(tname, fname) != 0)
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "renaming temp file", tname);
}

/**
 * fopen a temp file
 */
static FILE *
open_tfile(char const * fname, char ** tname)
{
    char const fmt[] = "%s-XXXXXX";
    char * nm = *tname = malloc(strlen(fname) + sizeof(fmt));
    FILE * res;

    if (nm == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "file name");

    sprintf(nm, fmt, fname);
    {
        int fd = mkstemp(nm);
        if (fd < 0)
            fserr(CRIGHT_UPDATE_EXIT_FSERR, "mkstemp", nm);

        res = fdopen(fd, "w");
    }
    if (res == NULL)
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "fdopen", nm);

    return res;
}

/**
 * Fix the copyright years.  We've detected that there is a copyright in
 * the file.  Now we update the years.
 *
 * @param fname file name
 * @param ftext file text
 * @param fsize size of file
 * @param preg  compiled regex for finding copyright notices
 * @param match the regexec() results.
 *
 * @returns the cright-update exit code result.
 */
static cright_update_exit_code_t
doit(char const * fname, char * ftext, size_t fsize,
     regex_t * preg, regmatch_t * match)
{
    char * tname;
    FILE * fp     = open_tfile(fname, &tname);
    int    fixct  = 0;
    int    is_c   = is_c_file_name(fname);
    char * prefix = NULL;
    size_t p_len;
    char const * res_msg;
    char * start = ftext;
    int    reres;

    cright_update_exit_code_t res = CRIGHT_UPDATE_EXIT_SUCCESS;

    /*
     * Fix the dates after the located marker.  This continues
     * if there is a failure or if --all has been specified.
     */
    do  {
        cright_update_exit_code_t fixres;

        regoff_t const eo = match[1].rm_eo;
        fwrite(ftext, eo, 1, fp);
        if (prefix != NULL)
            free(prefix);
        prefix = find_prefix(ftext, match[0].rm_so);
        p_len  = strlen(prefix);
        ftext += eo;
        if (is_c && (prefix[0] == '/') && (prefix[1] == '*'))
            *prefix = ' ';

        fixres = fixup(&ftext, prefix, p_len, fp);

        if (fixres == CRIGHT_UPDATE_EXIT_SUCCESS) {
            fixct++;
            if (! HAVE_OPT(ALL))
                break;

        } else if (res == CRIGHT_UPDATE_EXIT_SUCCESS)
            res = fixres;

        reres = regexec(preg, ftext, 2, match, 0);
    } while (reres == 0);

    /*
     * Write the remainder of the file -- if we're going to keep it.
     */
    if ((fixct > 0) && ! HAVE_OPT(DRY_RUN))
        fwrite(ftext, fsize - (ftext - start), 1, fp);
    if (prefix != NULL)
        free(prefix);
    fclose(fp);

    /*
     * If we didn't change anything, dump the new file and complain
     */
    if (fixct == 0) {
        if (HAVE_OPT(VERBOSE))
            fprintf(stderr, "no fixups applied to %s\n", fname);
        unlink(tname);
        res_msg = (res == CRIGHT_UPDATE_EXIT_NO_UPDATE) ? not_updt : bad_date;

    /*
     *  On a dry run, say that we'd have done something, but throw it away.
     */
    } else if (HAVE_OPT(DRY_RUN)) {
        unlink(tname);
        res_msg = needs_updt;

    /*
     *  preserve the temp output, replacing the original (maybe).
     *  Say we've done this in the summary.
     */
    } else {
        mv_file(fname, tname);
        res_msg = was_updt;
    }

    free(tname);
    printf(report_fmt, res_msg, fname);
    return res;
}

static void cu_regcomp(regex_t * preg, char const * re, int flags)
{
    int res = regcomp(preg, re, flags);
    if (res == 0)
        return;
    {
        char bf[64];
        (void)regerror(res, preg, bf, sizeof(bf));
        die(CRIGHT_UPDATE_EXIT_REGCOMP,
            "regcomp error %d (%s) -- failed on:  ``%s''\n",
            res, bf, re);
    }
}

static regex_t *
initialize(void)
{
    regex_t * preg = malloc(sizeof(* preg));
    int       reres;

    if (preg == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "regex struct");
    cu_regcomp(preg, OPT_ARG(COPYRIGHT_MARK), regex_flags);

    if (OPT_ARG(NEW_YEAR) == NULL) {
        char *      pz;
        time_t      ctim = time(NULL);
        struct tm * tmp  = localtime(&ctim);

        if (asprintf(&pz, "%4d", tmp->tm_year + 1900) < 4)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "year string");
        SET_OPT_NEW_YEAR(pz);
    }

    {
        char * pz;
        int yr = strtoul(OPT_ARG(NEW_YEAR), &pz, 10);
        if ((yr < 1900) || (yr > 2200) || (*pz != NUL))
            die(CRIGHT_UPDATE_EXIT_BAD_YEAR, "invalid year specified: %s\n",
                OPT_ARG(NEW_YEAR));

        asprintf(&last_year, "%4d", yr - 1);
    }

    return preg;
}

/**
 * Count length required for a raw string.  May be too large for
 * a cooked string, but will not be too short.  Good enough.
 */
static size_t
count_len(char * p)
{
    char * ptr = p;
    size_t len = 1; // for terminating NUL byte
    char   qch = *(p++); // may be single or double quote

    for (;;) {
        char ch = *++p;

        switch (ch) {
        case NUL:
        found_nul:
            die(CRIGHT_UPDATE_EXIT_REGCOMP, "bad regex string:  %s\n", ptr);

        case DQUOT: case SQUOT:
            if (ch != qch)
                break;
            p = SPN_HORIZ_WHITE_CHARS(p+1);
            switch (*p) {
            case NUL:
                goto found_nul;

            case BACKS:
                /*
                 * IF we have an escaped newline, look ahead to the next token.
                 * If that is the same kind of quote, then we glue the strings
                 * together, so keep counting.
                 */
                if (p[1] == NL) {
                    p = SPN_HORIZ_WHITE_CHARS(p+2);
                    if (*p == qch) {
                        p++;
                        continue;
                    }
                }
                // we do not have a backslash/newline followed by more text, so
                /* FALLTHROUGH */

            default:
                return len;
            }
            /* NOTREACHED */

        default:
            len++;
        }
    }

    return len;
}

static regex_t *
assemble_cooked(char * src_p)
{
    size_t    len  = count_len(src_p);  // might be over size
    regex_t * res  = malloc(sizeof(*res) + len);
    char * res_str = (char *)(res + 1);
    char * dst_p   = res_str;

    if (res == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "copyright mark regex");

    for (;;) {
        char ch_buf[4];
        char ch = *++src_p;
        int  ix;

        switch (ch) {
        case DQUOT:
            /*
             * We may be done.  Check for this sequence:
             *    <whitespace><backslash><newline><whitespace><doublequote>
             * If we find it, we continue
             */
            src_p = SPN_HORIZ_WHITE_CHARS(src_p+1);
            if ((src_p[0] == BACKS) && (src_p[1] == NL)) {
                src_p = SPN_HORIZ_WHITE_CHARS(src_p + 2);
                if (*src_p == DQUOT) {
                    src_p++;
                    continue;
                }
            }
            goto done_cooking; // break; break;

        default:
            *(dst_p++) = ch;
            break;

        case BACKS:
            ch = *++src_p;
            switch (ch) {
            case 'a': *(dst_p++) = '\a'; break;
            case 'b': *(dst_p++) = '\b'; break;
            case 't': *(dst_p++) = '\t'; break;
            case 'n': *(dst_p++) = NL;   break;
            case 'v': *(dst_p++) = '\v'; break;
            case 'f': *(dst_p++) = '\f'; break;
            case 'r': *(dst_p++) = '\r'; break;

            case 'x':
                for (ix = 0;;) {
                    if (! IS_HEX_DIGIT_CHAR(src_p[1]))
                        break;

                    ch_buf[ix] = *++src_p;
                    if (++ix >= 2)
                        break;
                }

                ch_buf[ix] = NUL;
                *(dst_p++) = 0xFF & strtoul(ch_buf, 0, 16);
                break;

            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                for (ix = 0;;) {
                    if (! IS_OCT_DIGIT_CHAR(src_p[1]))
                        break;

                    ch_buf[ix] = *++src_p;
                    if (++ix >= 3)
                        break;
                }

                ch_buf[ix] = NUL;
                *(dst_p++) = 0xFF & strtoul(ch_buf, 0, 8);
                break;

            default:
                *(dst_p++) = ch;
            }
        }
    }

 done_cooking:
    *dst_p = NUL; // we're done.
    cu_regcomp(res, res_str, regex_flags);
    return res;
}

static regex_t *
assemble_raw(char * src_p)
{
    size_t    len  = count_len(src_p);
    regex_t * res  = malloc(sizeof(*res) + len);
    char * res_str = (char *)(res + 1);
    char * dst_p   = res_str;

    if (res == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "copyright mark regex");

    for (;;) {
        char ch = *++src_p;
        switch (ch) {
        case SQUOT:
            src_p = SPN_HORIZ_WHITE_CHARS(src_p+1);
            if ((src_p[0] == BACKS) && (src_p[1] == NL)) {
                src_p = SPN_HORIZ_WHITE_CHARS(src_p + 2);
                if (*src_p == SQUOT)
                    continue;
            }
            break;

        default:
            *(dst_p++) = ch;
            continue;
        }
    }

    *dst_p = NUL;
    cu_regcomp(res, res_str, regex_flags);
    return res;
}

static regex_t *
assemble_xml(char * p)
{
    die(CRIGHT_UPDATE_EXIT_REGCOMP, "xml regex-es not supported\n");
}

static regex_t *
assemble_line(char * p)
{
    regex_t * res;
    size_t res_str_size;
    char * q = strchr(p, NL);
    char * res_str;

    if (q == NULL)
        return NULL;
    q = SPN_HORIZ_WHITE_BACK(p, q);
    res_str_size = q - p;
    res = malloc(sizeof(*res) + res_str_size + 1);
    if (res == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "copyright mark regex");
    res_str = (char *)(res + 1);
    memcpy(res_str, p, res_str_size);
    res_str[res_str_size] = NUL;
    cu_regcomp(res, res_str, regex_flags);
    return res;
}

/**
 * parse out and allocate a regex buffer.
 * Ignore leading colon or equal char.
 */
static regex_t *
assemble_regex(char * p)
{
    p = SPN_HORIZ_WHITE_CHARS(p);
    switch (*p) {
    case ':': case '=':
        p = SPN_HORIZ_WHITE_CHARS(p+1);
        break;
    }

    switch (*p) {
    case NL:    return NULL;
    case DQUOT: return assemble_cooked(p);
    case SQUOT: return assemble_raw(p);
    case '<':   return assemble_xml(p);
    default:    return assemble_line(p);
    }

}

/**
 * see if the file text contains an alternate RE to use.
 * @param [in] ftext  the text of that file
 * @param [in] preg   the default RE
 */
static regex_t *
choose_re(char * ftext, regex_t * preg)
{
    static char const mark[] = "--" "copyright-mark";
    char * p = strstr(ftext, mark);
    if (p == NULL)
        return preg;

    if (preg == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "regex struct");

    return assemble_regex(p + sizeof(mark) - 1);
}

/**
 * Main callout procedure.
 * @param [in]     fname  file name to (potentially) update
 * @param [in,out] ftext  the text of that file
 * @param [in]     fsize  the number of bytes of text
 * @returns a cright-update exit code
 */
int
fix_copyright(char const * fname, char * ftext, size_t fsize)
{
    static regex_t * preg = NULL;
    regex_t * re;

    int        reres;
    regmatch_t match[2];
    char * re_text;

    if (preg == NULL)
        preg = initialize();

    re    = choose_re(ftext, preg);
    reres = regexec(re, ftext, 2, match, 0);

    switch (reres) {
    default:
        die(CRIGHT_UPDATE_EXIT_REGEXEC, "regexec failed in %s:  %s\n",
            fname, OPT_ARG(COPYRIGHT_MARK));
        /* NOTREACHED */

    case 0:
        if (match[1].rm_so > 0) {
            reres = doit(fname, ftext, fsize, re, match);
            break;
        }
        /* FALLTHROUGH */

    case REG_NOMATCH:
        printf(report_fmt, no_match, fname);
        reres = CRIGHT_UPDATE_EXIT_SUCCESS;
    }

    if (re != preg) {
        regfree(re);
        free(re);
    }
    return reres;
}
