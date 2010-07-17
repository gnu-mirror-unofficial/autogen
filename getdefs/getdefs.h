/*  -*- Mode: C -*-
 *
 *    getdefs Copyright (c) 1999-2010 by Bruce Korb - all rights reserved
 *
 *  Time-stamp:        "2010-07-16 15:20:23 bkorb"
 *  Author:            Bruce Korb <bkorb@gnu.org>
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2010 by Bruce Korb - all rights reserved
 *
 *  AutoGen is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GETDEFS_HEADER
#define GETDEFS_HEADER

#include "config.h"
#include "compat/compat.h"
#include <sys/wait.h>
#include <utime.h>
#include <stdarg.h>

#include REGEX_HEADER

#include "opts.h"

/*
 *  Procedure success codes
 *
 *  USAGE:  define procedures to return "tSuccess".  Test their results
 *          with the SUCCEEDED, FAILED and HADGLITCH macros.
 *
 *  Microsoft sticks its nose into user space here, so for Windows' sake,
 *  make sure all of these are undefined.
 */
#undef  SUCCESS
#undef  FAILURE
#undef  PROBLEM
#undef  SUCCEEDED
#undef  SUCCESSFUL
#undef  FAILED
#undef  HADGLITCH

#define SUCCESS  ((tSuccess) 0)
#define FAILURE  ((tSuccess)-1)
#define PROBLEM  ((tSuccess) 1)

typedef int tSuccess;

#define SUCCEEDED( p )     ((p) == SUCCESS)
#define SUCCESSFUL( p )    SUCCEEDED( p )
#define FAILED( p )        ((p) <  SUCCESS)
#define HADGLITCH( p )     ((p) >  SUCCESS)

#define EXPORT

#define MAXNAMELEN 256

#define MAX_SUBMATCH   1
#define COUNT(a)       (sizeof(a)/sizeof(a[0]))

#define MARK_CHAR ':'

#ifndef STR
#  define __STR(s)  #s
#  define STR(s)    __STR(s)
#endif

#define AG_NAME_CHAR(c) (zUserNameCh[(unsigned)(c)] & 2)
#define USER_NAME_CH(c) (zUserNameCh[(unsigned)(c)] & 1)
char zUserNameCh[ 256 ] = { '\0' };

/*
 *  Index database string pointers.
 */
char*    pzIndexText = NULL; /* all the text    */
char*    pzEndIndex  = NULL; /* end of current  */
char*    pzIndexEOF  = NULL; /* end of file     */
size_t   indexAlloc  = 0;    /* allocation size */

/*
 *  Name of program to process output (normally ``autogen'')
 */
tCC*     pzAutogen   = "autogen";

/*
 *  const global strings
 */
#define DEF_STRING(n,s) tCC n[] = s
DEF_STRING( zGlobal,     "\n/* GLOBALDEFS */\n" );
DEF_STRING( zLineId,     "\n#line %d \"%s\"\n" );
DEF_STRING( zMallocErr,  "Error:  could not allocate %d bytes for %s\n" );
DEF_STRING( zAttribRe,   "\n[^*\n]*\\*[ \t]*([a-z][a-z0-9_-]*):");
DEF_STRING( zNameTag,    " = {\n    name    = '" );
DEF_STRING( zMemberLine, "    member  = " );
DEF_STRING( zNoData,     "error no data for definition in file %s line %d\n" );
DEF_STRING( zAgDef,      "autogen definitions %s;\n");
DEF_STRING( zDne,
            "/*  -*- buffer-read-only: t -*- vi: set ro:\n *\n"
            " *\n *  DO NOT EDIT THIS FILE   (%s)\n *\n"
            " *  It has been extracted by getdefs from the following files:\n"
            " *\n" );

/*
 *  ptr to zero (NUL) terminated definition pattern string.
 *
 *  The pattern we look for starts with the three characters
 *  '/', '*' and '=' and is followed by two names:
 *  the name of a group and the name of the entry within the group.
 *
 *  The patterns we accept for output may specify a particular group,
 *  certain members within certain groups or all members of all groups
 */
tCC*    pzDefPat   = NULL;
regex_t define_re;
regex_t attrib_re;

/*
 *  The output file pointer.  It may be "stdout".
 *  It gets closed when we are done.
 */
FILE*  evtFp       = (FILE*)NULL;

/*
 *  The output file modification time.  Only used if we
 *  have specified a real file for output (not stdout).
 */
time_t modtime     = 0;

/*
 *  The array of pointers to the output blocks.
 *  We build them first, then sort them, then print them out.
 */
char**  papzBlocks = (char**)NULL;
size_t  blkUseCt   = 0;
size_t  blkAllocCt = 0;
pid_t   agPid      = -1;

#define LOCAL static
#endif /* GETDEFS_HEADER */

/* emacs
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of getdefs/getdefs.h */
