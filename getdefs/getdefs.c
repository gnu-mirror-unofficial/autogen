/*  -*- Mode: C -*-
 *
 *  $Id: getdefs.c,v 2.26 2001/05/09 05:25:59 bkorb Exp $
 *
 *    getdefs copyright 1999-2001 Bruce Korb
 *
 *  Author:            Bruce Korb <bkorb@gnu.org>
 *  Maintainer:        Bruce Korb <bkorb@gnu.org>
 *  Created:           Mon Jun 30 15:35:12 1997
 *  Last Modified:     $Date: 2001/05/09 05:25:59 $
 *            by:      Bruce Korb <bkorb@gnu.org>
 */

#define DEFINE
#include "getdefs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Forward procedure pointers
 */
typedef int (compar_func)(const void *, const void *);
compar_func compar_text;
FILE* startAutogen( void );
char* loadFile( char* pzFname );
void  processFile( char* pzFile );
void  sortEntries( void );
void  validateOptions( void );
void  printEntries( FILE* defFp );
void  doPreamble( FILE* outFp );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   MAIN
 */
    int
main( int    argc,
      char** argv )
{
    FILE* outFp;

    optionProcess( &getdefsOptions, argc, argv );
    validateOptions();

    outFp = startAutogen();

    doPreamble( outFp );

    /*
     *  Process each input file
     */
    {
        int    ct  = STACKCT_OPT(  INPUT );
        char** ppz = STACKLST_OPT( INPUT );
        do  {
            processFile( *ppz++ );
        } while (--ct > 0);
    }

    if (ENABLED_OPT( ORDERING ) && (blkUseCt > 1))
        qsort( (void*)papzBlocks, blkUseCt, sizeof( char* ),
               &compar_text );

    printEntries( outFp );
    fclose( outFp );

    /*
     *  IF output is to a file
     *  THEN set the permissions and modification times
     */
    if (  (WHICH_IDX_AUTOGEN == INDEX_OPT_OUTPUT)
       && (outFp != stdout) )  {
        struct utimbuf tbuf;
        tbuf.actime  = time( (time_t*)NULL );
        tbuf.modtime = modtime + 1;
        utime( OPT_ARG( OUTPUT ), &tbuf );
        chmod( OPT_ARG( OUTPUT ), S_IRUSR|S_IRGRP|S_IROTH);
    }

    /*
     *  IF we are keeping a database of indexes
     *     AND we have augmented the contents,
     *  THEN append the new entries to the file.
     */
    if ((pzIndexText != (char*)NULL) && (pzEndIndex != pzIndexEOF)) {
        FILE* fp = fopen( OPT_ARG( ORDERING ), "a" FOPEN_BINARY_FLAG );
        fputs( pzIndexEOF, fp );
        fclose( fp );
    }

    if (agPid != -1) {
        int  status;
        waitpid( agPid, &status, 0 );
        if (WIFEXITED( status )) {
            status = WEXITSTATUS( status );
            if (status != EXIT_SUCCESS) {
                fprintf( stderr, "ERROR:  %s exited with status %d\n",
                         pzAutogen, status );
            }
            return status;
        }

        if (WIFSIGNALED( status )) {
            status = WTERMSIG( status );
            fprintf( stderr, "ERROR:  %s exited due to %d signal (%s)\n",
                     pzAutogen, status, strsignal( status ));
        }
        else
            fprintf( stderr, "ERROR:  %s exited due to unknown reason %d\n",
                     pzAutogen, status );

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


    void
validateOptions( void )
{
    /*
     *  Our default pattern is to accept all names following
     *  the '/' '*' '=' character sequence
     */
    if ((! HAVE_OPT( DEFS_TO_GET )) || (*OPT_ARG( DEFS_TO_GET ) == NUL)) {
        pzDefPat = "/\\*=([a-z][a-z0-9_]*|\\*)[ \t]+[a-z]";

    } else {
        char*  pz  = OPT_ARG( DEFS_TO_GET );
        size_t len = strlen( pz ) + 16;

        pzDefPat = (char*)malloc( len );
        if (pzDefPat == (char*)NULL) {
            fprintf( stderr, zMallocErr, len, "definition pattern" );
            exit( EXIT_FAILURE );
        }

        /*
         *  IF a pattern has been supplied, enclose it with
         *  the '/' '*' '=' part of the pattern.
         */
        snprintf( pzDefPat, len, "/\\*=(%s)", pz );
    }

    /*
     *  Compile the regular expression that we are to search for
     *  to find each new definition in the source files.
     */
    {
        char zRER[ MAXNAMELEN ];
        static const char zReErr[] =
            "Regex error %d (%s):  Cannot compile reg expr:\n\t%s\n";

        int rerr = regcomp( &define_re, pzDefPat, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &define_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, zRER, pzDefPat );
            exit( EXIT_FAILURE );
        }

        rerr = regcomp( &attrib_re, zAttribRe, REG_EXTENDED | REG_ICASE );
        if (rerr != 0) {
            regerror( rerr, &attrib_re, zRER, sizeof( zRER ));
            fprintf( stderr, zReErr, rerr, zRER, zAttribRe );
            exit( EXIT_FAILURE );
        }
    }

    /*
     *  Prepare each sub-block entry so we can parse easily later.
     */
    if (HAVE_OPT( SUBBLOCK )) {
        tSCC    zNoList[] = "ERROR:  block attr must have name list:\n"
                            "\t%s\n";
        int     ct  = STACKCT_OPT(  SUBBLOCK );
        char**  ppz = STACKLST_OPT( SUBBLOCK );
        char*   pz;
        char*   p;

        /*
         *  FOR each SUBBLOCK argument,
         *  DO  condense each name list to be a list of names
         *      separated by a single space and NUL terminated.
         */
        do  {
            /*
             *  Make sure we find the '=' separator
             */
            pz = strchr( *ppz++, '=' );
            if (pz == (char*)NULL) {
                fprintf( stderr, zNoList, ppz[-1] );
                USAGE( EXIT_FAILURE );
            }

            /*
             *  NUL the equal char
             */
            *pz++ = NUL;
            p = pz;

            /*
             *  Make sure at least one attribute name is defined
             */
            while (isspace( *pz )) pz++;
            if (*pz == NUL) {
                fprintf( stderr, zNoList, ppz[-1] );
                USAGE( EXIT_FAILURE );
            }

            for (;;) {
                /*
                 *  Attribute names must start with an alpha
                 */
                if (! isalpha( *pz )) {
                    fprintf( stderr, "ERROR:  attribute names must start "
                             "with an alphabetic character:\n\t%s\n",
                             ppz[-1] );
                    USAGE( EXIT_FAILURE );
                }

                /*
                 *  Copy the name.  (maybe.  "p" and "pz" may be equal)
                 */
                while (isalnum( *pz ) || (*pz == '_'))
                    *p++ = *pz++;

                /*
                 *  Skip over one comma (optional) and any white space
                 */
                while (isspace( *pz )) pz++;
                if (*pz == ',')
                    pz++;

                while (isspace( *pz )) pz++;
                if (*pz == NUL)
                    break;
                /*
                 *  The final string contains only one space
                 */
                *p++ = ' ';
            }

            *p = NUL;
        } while (--ct > 0);
    }

    /*
     *  Make sure each of the input files is findable.
     *  Also, while we are at it, compute the output file mod time
     *  based on the mod time of the most recent file.
     */
    {
        int    ct  = STACKCT_OPT(  INPUT );
        char** ppz = STACKLST_OPT( INPUT );
        struct stat stb;

        do  {
            if (stat( *ppz++, &stb ) != 0)
                break;
            if (! S_ISREG( stb.st_mode )) {
                errno = EINVAL;
                break;
            }
            stb.st_mtime += 60;
            if (stb.st_mtime > modtime)
                modtime = stb.st_mtime;
        } while (--ct > 0);
        if (ct > 0) {
            fprintf( stderr, "Error %d (%s) stat-ing %s for text file\n",
                     errno, strerror( errno ), ppz[-1] );
            USAGE( EXIT_FAILURE );
        }
    }

    /*
     *  IF the output is to have order AND it is to be based on a file,
     *  THEN load the contents of that file.
     *       IF we cannot load the file,
     *       THEN it must be new or empty.  Allocate several K to start.
     */
    if (   HAVE_OPT( ORDERING )
       && (OPT_ARG( ORDERING ) != (char*)NULL)) {
        pzIndexText = loadFile( OPT_ARG( ORDERING ));
        if (pzIndexText == (char*)NULL) {
            pzIndexText = pzEndIndex  = pzIndexEOF =
                (char*)malloc( 0x4000 );
            indexAlloc = 0x4000;
            *pzIndexText = NUL;
        } else {
            pzEndIndex  = pzIndexEOF = pzIndexText + strlen( pzIndexText );
            indexAlloc = (pzEndIndex - pzIndexText) + 1;
        }

        /*
         *  We map the name entries to a connonical form.
         *  By default, everything is mapped to lower case already.
         *  This call will map these three characters to '_'.
         */
        strequate( "_-^" );
    }

    if (HAVE_OPT( SRCFILE )) {
        if (OPT_ARG( SRCFILE ) == (char*)NULL)
            OPT_ARG( SRCFILE ) = "srcfile";
    }

    if (HAVE_OPT( LINENUM )) {
        if (OPT_ARG( LINENUM ) == (char*)NULL)
            OPT_ARG( LINENUM ) = "linenum";
    }
}


    void
doPreamble( FILE* outFp )
{
    char* pzName;

    /*
     *  Emit the "autogen definitions xxx;" line
     */
    fprintf( outFp, zAgDef, zTemplName );

    if (HAVE_OPT( FILELIST )) {
        tSCC   zFmt[] = "%-12s = '%s';\n";

        int    ct  = STACKCT_OPT(  INPUT );
        char** ppz = STACKLST_OPT( INPUT );

        pzName = OPT_ARG( FILELIST );

        if (pzName == (char*)NULL)
            pzName = "infile";

        do  {
            fprintf( outFp, zFmt, pzName, *ppz++ );
        } while (--ct > 0);

        if (HAVE_OPT( COPY )) {
            ct  = STACKCT_OPT(  COPY );
            ppz = STACKLST_OPT( COPY );
            do  {
                fprintf( outFp, zFmt, pzName, *ppz++ );
            } while (--ct > 0);
        }
        fputc( '\n', outFp );
    }

    /*
     *  IF there are COPY files to be included,
     *  THEN emit the '#include' directives
     */
    if (HAVE_OPT( COPY )) {
        int    ct  = STACKCT_OPT(  COPY );
        char** ppz = STACKLST_OPT( COPY );
        do  {
            fprintf( outFp, "#include %s\n", *ppz++ );
        } while (--ct > 0);
        fputc( '\n', outFp );
    }

    /*
     *  IF there are global assignments, then emit them
     *  (these do not get sorted, so we write directly now.)
     */
    if (HAVE_OPT( ASSIGN )) {
        int    ct  = STACKCT_OPT(  ASSIGN );
        char** ppz = STACKLST_OPT( ASSIGN );
        do  {
            fprintf( outFp, "%s;\n", *ppz++ );
        } while (--ct > 0);
        fputc( '\n', outFp );
    }
}


/*
 *  compar_text
 *
 *  merely returns the relative ordering of two input strings.
 *  The arguments are pointers to pointers to NUL-terminated strings.
 *  IF the definiton was mal-formed, an error message was printed
 *  earlier.  When we get here, we wil fail to find the "zNameTag"
 *  string and EXIT_FAILURE.
 */
    int
compar_text( const void* p1, const void* p2 )
{
    char* pz1 = strstr( *(char**)p1, zNameTag );
    char* pe1;
    char* pz2 = strstr( *(char**)p2, zNameTag );
    char* pe2;
    int   res;

    static const char zBogus[] = "Bogus definition:\n%s\n";

    if (pz1 == (char*)NULL) {
        if (strncmp( *(char**)p1, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return -1;

        fprintf( stderr, zBogus, *(char**)p1 );
        exit( EXIT_FAILURE );
    }

    if (pz2 == (char*)NULL) {
        if (strncmp( *(char**)p2, zGlobal, sizeof( zGlobal )-1 ) == 0)
            return 1;

        fprintf( stderr, zBogus, *(char**)p2 );
        exit( EXIT_FAILURE );
    }

    pz1 += sizeof( zNameTag )-1;
    pe1 = strchr( pz1, '\'' );

    if (pe1 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p1 );
        exit( EXIT_FAILURE );
    }

    pz2 += sizeof( zNameTag )-1;
    pe2 = strchr( pz2, '\'' );

    if (pe2 == (char*)NULL) {
        fprintf( stderr, zBogus, *(char**)p2 );
        exit( EXIT_FAILURE );
    }

    *pe1 = *pe2 = NUL;

    /*
     *  We know ordering is enabled because we only get called when
     *  it is enabled.  If the option was also specified, then
     *  we sort without case sensitivity (and we compare '-', '_'
     *  and '^' as being equal as well).  Otherwise, we do a
     *  strict string comparison.
     */
    if (HAVE_OPT( ORDERING ))
         res = streqvcmp( pz1, pz2 );
    else res = strcmp( pz1, pz2 );
    *pe1 = *pe2 = '\'';
    return res;
}



/*
 *  compressDef
 *
 *  Compress the definition text.  Each input line has some prefix
 *  stuff to ensure it is a comment as seen by the normal processor
 *  of the file.  In "C", the entire block is surrounded by the
 *  '/'-'*' and '*'-'/' pairs.  In shell, every line would start
 *  with a hash character ('#').  Etc.  To make life easy, we require
 *  that every line be prefixed with something that matches the
 *  pattern:
 *
 *        "^[^*]*\*"
 *
 *  and any line that does not is ignored.  So, here we strip off
 *  that prefix before we go ahead and try to parse it.
 */
    void
compressDef( char* pz )
{
    char* pzStrt = pz;
    char* pzDest = pz;
    char* pzSrc  = pz+1;
    int   nlCt;

    /*
     *  Search until we find a line that contains an asterisk
     *  and is followed by something other than whitespace.
     */
    for (;;) {
        nlCt =  0;

        /*
         *  Skip over leading space
         */
        while (isspace( *pzSrc )) {
            if (*pzSrc == '\n')
                nlCt++;
            pzSrc++;
        }

        /*
         *  IF no new-lines were found, then we found text start
         */
        if (nlCt == 0)
            break;

        /*
         *  Skip over the first asterisk we find
         *  Then, skip over leading space.
         */
        pzSrc = strchr( pzSrc, '*' );
        if (pzSrc == (char*)NULL) {
            *pzStrt = NUL;
            return;
        }

        if (pzDest == pzStrt)
            *pzDest++ = '\n';

        /*
         *  Skip over sequential asterisks
         */
        while (*pzSrc == '*') pzSrc++;
    }

    /*
     *  FOR as long as we still have more text, ...
     */
    for (;;) {
        /*
         *  IF we passed over one or more newlines while looking for
         *  an asterisk, then insert one extra newline into the output
         */
        if (nlCt > 0) {
            *pzDest++ = '\n';
            nlCt =  0;
        }

        /*
         *  FOR all the data on the current input line, ...
         */
        for (;;) {
            /*
             *  Move the source to destination until we find
             *  either a new-line or a NUL.
             */
            switch (*pzDest++ = *pzSrc++) {
            case '\n':
                if (*pzSrc != NUL)
                    goto lineDone;

            case NUL:
                pzDest--;
                goto compressDone;

            default:
                ;
            }
        } lineDone:;

        /*
         *  Trim trailing white space off the end of the line.
         */
        if ((pzDest[-2] == ' ') || (pzDest[-2] == '\t')) {
            do  {
                pzDest--;
            } while ((pzDest[-2] == ' ') || (pzDest[-2] == '\t'));
            pzDest[-1] = '\n';
        }

        /*
         *  We found a new-line.  Skip forward to an asterisk.
         */
    foundNewline:
        while (*pzSrc != '*') {
            if (*pzSrc == NUL)
                goto compressDone;
            if (*pzSrc == '\n')
                nlCt++;
            pzSrc++;
        }

        /*
         *  Skip over the asterisk we found and all the ones that follow
         */
        while (*pzSrc == '*')     pzSrc++;
        while (isspace( *pzSrc )) {
            /*
             *  IF we stumble into another newline,
             *  THEN we go back to look for an asterisk.
             */
            if (*pzSrc == '\n')
                goto foundNewline;
            pzSrc++;
        }
    } compressDone:;

    /*
     *  Trim off all the trailing white space, including newlines
     */
    while ((pzDest > pzStrt) && isspace( pzDest[-1] )) pzDest--;
    *pzDest = NUL;
}


/*
 *  The text is quoted, so copy it as is, ensuring that escaped
 *  characters are not used to end the quoted text.
 */
    char*
emitQuote( char** ppzText, char* pzOut )
{
    char*  pzText = *ppzText;
    char   svch   = (*pzOut++ = *pzText++);

    for (;;) {
        switch (*pzOut++ = *pzText++) {

        case '\\':
            if ((*pzOut++ = *pzText++) != NUL)
                break;

        case NUL:
            pzText--;
            pzOut[-1] = svch;
            svch = NUL;
            /* FALLTHROUGH */

        case '"':
        case '\'':
            if (pzOut[-1] == svch)
                goto quoteDone;

            break;
        }
    }

quoteDone:
    *ppzText = pzText;
    *pzOut++ = ';';
    return pzOut;
}


/*
 *  Emit a string in a fashion that autogen will be able to
 *  correctly reconstruct it.
 */
    char*
emitSubblockString( char** ppzText, char sepChar, char* pzOut )
{
    char*  pzText  = *ppzText;
    char*  pcComma;
    char*  pcEnd;

    /*
     *  Skip leading space
     */
    while (isspace( *pzText )) pzText++;

    /*
     *  IF the text is already quoted,
     *  THEN call the quoted text emitting routine
     */
    if ((*pzText == '"') || (*pzText == '\'')) {
        *ppzText = pzText;
        return emitQuote( ppzText, pzOut );
    }

    /*
     *  Look for the character that separates this entry text
     *  from the entry text for the next attribute.  Leave 'pcComma'
     *  pointing to the character _before_ the character where we
     *  are to resume our text scan.  (i.e. at the comma, or the
     *  last character in the string)
     */
    pcComma = strchr( pzText, sepChar );
    if (pcComma == (char*)NULL) {
        pcEnd = pzText + strlen( pzText );
        pcComma = pcEnd-1;
    } else {
        pcEnd = pcComma;
    }

    /*
     *  Clean off trailing white space.
     */
    while ((pcEnd > pzText) && isspace( pcEnd[-1] )) pcEnd--;

    /*
     *  Copy the text, surrounded by single quotes
     */
    *pzOut++ = '\'';
    {
        char svch = *pcEnd;
        *pcEnd = NUL;
        for (;;) {
            char ch = *pzText++;
            switch (ch) {
            case '\'':
                *pzOut++ = '\\';
            default:
                *pzOut++ = ch;
                break;
            case NUL:
                goto copyDone;
            }
        } copyDone: ;

        pzText = pcComma+1;
        *pcEnd = svch;
    }

    *pzOut++ = '\''; *pzOut++ = ';';
    *ppzText = pzText;
    return pzOut;
}



    char*
emitSubblock( char* pzDefList, char* pzText, char* pzOut )
{
    tSCC  zStart[]  = " = {";
    tSCC  zAttr[]   = "\n        ";
    tSCC  zEnd[]    = "\n    };\n";
    char  sepChar   = ',';
    int   FirstAttr = 1;

    /*
     *  Advance past subblock name to the entry name list
     */
    pzDefList += strlen( pzDefList ) + 1;
    strcpy( pzOut, zStart );
    pzOut += sizeof( zStart ) - 1;

    /*
     *  See if there is an alternate separator character.
     *  It must be a punctuation character that is not also
     *  a quote character.
     */
    if (ispunct( *pzText ) && (*pzText != '"') && (*pzText != '\''))
        sepChar = *(pzText++);

    /*
     *  Loop for as long as we have text entries and subblock
     *  attribute names, ...
     */
    do  {
        /*
         *  IF the first character is the separator,
         *  THEN this entry is skipped.
         */
        if (*pzText == sepChar) {
            pzText++;
            for (;;) {
                switch (*++pzDefList) {
                case ' ':
                    pzDefList++;
                case NUL:
                    goto def_list_skip_done;
                }
            } def_list_skip_done:;
            continue;
        }

        /*
         *  Skip leading white space in the attribute and check for done.
         */
        while (isspace( *pzText )) pzText++;
        if (*pzText == NUL) {
            /*
             *  IF there were no definitions, THEN emit one anyway
             */
            if (FirstAttr) {
                strcpy( pzOut, zAttr );
                pzOut += sizeof( zAttr );
                for (;;) {
                    *pzOut++ = *pzDefList++;
                    switch (*pzDefList) {
                    case ' ':
                    case NUL:
                        goto single_def_entry_done;
                    }
                } single_def_entry_done:;
                *pzOut++ = ';';
            }
            break;
        }

        /*
         *  Copy out the attribute name
         */
        strcpy( pzOut, zAttr );
        pzOut += sizeof( zAttr )-1;
        FirstAttr = 0;

        for (;;) {
            *pzOut++ = *pzDefList++;
            switch (*pzDefList) {
            case ' ':
                pzDefList++;
            case NUL:
                goto def_name_copied;
            }
        } def_name_copied:;

        /*
         *  IF there are no data for this attribute,
         *  THEN we emit an empty definition.
         */
        if (*pzText == sepChar) {
            *pzOut++ = ';';
            pzText++;
            continue;
        }

        /*
         *  Copy out the assignment operator and emit the string
         */
        *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = ' ';
        pzOut = emitSubblockString( &pzText, sepChar, pzOut );

    } while (isalpha( *pzDefList ));
    strcpy( pzOut, zEnd );
    return pzOut + sizeof( zEnd ) - 1;
}


    char*
emitDefinition( char* pzDef, char* pzOut )
{
    char   sep_char;
    char   zEntryName[ MAXNAMELEN ];

    /*
     *  Indent attribute definitions four spaces
     */
    *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' '; *pzOut++ = ' ';

    if (! HAVE_OPT( SUBBLOCK )) {
        while (*pzDef != MARK_CHAR)  *pzOut++ = *pzDef++;
        compressDef( pzDef );

    } else {
        int    ct  = STACKCT_OPT(  SUBBLOCK );
        char** ppz = STACKLST_OPT( SUBBLOCK );
        char*  p   = zEntryName;

        while (*pzDef != MARK_CHAR)
            *p++ = *pzOut++ = *pzDef++;

        *p = NUL;
        compressDef( pzDef );

        do  {
            p = *ppz++;
            if (strcmp( p, zEntryName ) == 0)
                return emitSubblock( p, pzDef, pzOut );
        } while (--ct > 0);
    }

    if (isspace( *pzDef ))
         sep_char = *pzDef++;
    else sep_char = ' ';

    switch (*pzDef) {
    case NUL:
        *pzOut++ = ';'; *pzOut++ = '\n';
        break;

    case '"':
    case '\'':
    case '{':
        /*
         *  Quoted entries or subblocks do their own stringification
         *  sprintf is safe because we are copying strings around
         *  and *always* making the result smaller than the original
         */
        pzOut += sprintf( pzOut, " =%c%s;\n", sep_char, pzDef );
        break;

    default:
        *pzOut++ = ' '; *pzOut++ = '='; *pzOut++ = sep_char;
        *pzOut++ = '\'';

        for (;;) {
            switch (*pzOut++ = *pzDef++) {
            case '\\':
            case '\'':
                pzOut[-1] = '\\';
                *pzOut++  = '\'';
                break;

            case NUL:
                goto unquotedDone;
            }
        } unquotedDone:;
        pzOut[-1] = '\''; *pzOut++ = ';'; *pzOut++ = '\n';
        break;
    }
    return pzOut;
}



    char*
assignIndex( char*  pzOut,  char*  pzDef )
{
    char*  pzMatch;
    size_t len = strlen( pzDef );
    long   idx;

    /*
     *  Make the source text all lower case and map
     *  '-', '^' and '_' characters to '_'.
     */
    strtransform( pzDef, pzDef );

    /*
     * IF there is already an entry,
     * THEN put the index into the output.
     */
    pzMatch = strstr( pzIndexText, pzDef );
    if (pzMatch != (char*)NULL) {
        pzMatch += len;
        while (isspace( *pzMatch )) pzMatch++;
        while ((*pzOut++ = *pzMatch++) != ']') ;
        return pzOut;
    }

    /*
     *  We have a new entry.  Make sure we have room for it
     *  in our in-memory string
     */
    if (((pzEndIndex - pzIndexText) + len + 64 ) > indexAlloc) {
        char* pz;
        indexAlloc +=  0x1FFF;
        indexAlloc &= ~0x0FFF;
        pz = (char*)realloc( (void*)pzIndexText, indexAlloc );
        if (pz == (char*)NULL) {
            fputs( "Realloc of index text failed\n", stderr );
            exit( EXIT_FAILURE );
        }

        /*
         *  IF the allocation moved,
         *  THEN adjust all our pointers.
         */
        if (pz != pzIndexText) {
            pzIndexEOF  = pz + (pzIndexEOF - pzIndexText);
            pzEndIndex  = pz + (pzEndIndex - pzIndexText);
            pzIndexText = pz;
        }
    }

    /*
     *  IF there are no data in our text database,
     *  THEN default to a zero index.
     */
    if (pzEndIndex == pzIndexText)
        idx = 0;
    else do {
        char* pz = strrchr( pzDef, ' ' );
        *pz = NUL;
        len = strlen( pzDef );

        /*
         *  Find the last entry for the current category of entries
         */
        pzMatch = strstr( pzIndexText, pzDef );
        if (pzMatch == (char*)NULL) {
            /*
             *  No entries for this category.  Use zero for an index.
             */
            idx = 0;
            *pz = ' ';
            break;
        }

        for (;;) {
            char* pzn = strstr( pzMatch + len, pzDef );
            if (pzn == (char*)NULL)
                break;
            pzMatch = pzn;
        }

        /*
         *  Skip forward to the '[' character and convert the
         *  number that follows to a long.
         */
        *pz = ' ';
        pzMatch = strchr( pzMatch + len, '[' );
        idx = strtol( pzMatch+1, (char**)NULL, 0 )+1;
    } while (0);

    /*
     *  Add the new entry to our text database and
     *  place a copy of the value into our output.
     */
    pzEndIndex += sprintf( pzEndIndex, "%-40s  [%d]\n", pzDef, idx );
    pzOut += sprintf( pzOut, "[%d]", idx );

    return pzOut;
}



    tSuccess
buildPreamble(
    char**   ppzDef,
    char**   ppzOut,
    char*    pzFile,
    int      line )
{
    char* pzDef = *ppzDef;
    char* pzOut = *ppzOut;

    char  zDefText[ MAXNAMELEN ];
    char* pzDefText = zDefText;
    char  zNameText[ MAXNAMELEN ];
    char* pzNameText = zNameText;
    char* pzMembership;

    char* pzIfText   = (char*)NULL;

    /*
     *  Copy out the name of the entry type
     */
    *pzDefText++ = '`';
    while (isalnum( *pzDef ) || (*pzDef == '_') || (*pzDef == '.'))
        *pzDefText++ = *pzDef++;

    *pzDefText = NUL;

    pzDef += strspn( pzDef, "* \t" );

    /*
     *  Copy out the name for this entry of the above entry type.
     */
    while (isalnum( *pzDef ) || (*pzDef == '_'))
        *pzNameText++ = *pzDef++;
    *pzNameText = NUL;

    if (  (zDefText[1]  == NUL)
       || (zNameText[0] == NUL) )  {
        fprintf( stderr, zNoData, pzFile, line );
        return FAILURE;
    }

    pzDef += strspn( pzDef, " \t" );

    /*
     *  IF these names are followed by a comma and an "if" clause,
     *  THEN we emit the definition with "#if..."/"#endif" around it
     */
    if (*pzDef == ',') {
        pzDef += strspn( pzDef+1, " \t" )+1;
        if ((pzDef[0] == 'i') && (pzDef[1] == 'f'))
            pzIfText = pzDef;
    }

    pzDef = strchr( pzDef, '\n' );
    if (pzDef == (char*)NULL) {
        fprintf( stderr, zNoData, pzFile, line );
        return FAILURE;
    }

    *pzDef = NUL;

    /*
     *  Now start the output.  First, the "#line" directive,
     *  then any "#ifdef..." line and finally put the
     *  entry type name into the output.
     */
    pzOut += sprintf( pzOut, zLineId, line, pzFile );
    if (pzIfText != (char*)NULL)
        pzOut += sprintf( pzOut, "#%s\n", pzIfText );
    {
        char*  pz = zDefText+1;
        while (isalnum( *pz ) || (*pz == '_'))
            *pzOut++ = *pz++;
        if (*pz == '.') {
            pzMembership = pz+1;
            *pz = NUL;
        } else
            pzMembership = (char*)NULL;
    }

    /*
     *  IF we are indexing the entries,
     *  THEN build the string by which we are indexing
     *       and insert the index into the output.
     */
    if (pzIndexText != (char*)NULL) {
        sprintf( pzDefText, "  %s'", zNameText );
        pzOut = assignIndex( pzOut, zDefText );
    }

    /*
     *  Now insert the name with a consistent name string prefix
     *  that we use to locate the sort key later.
     */
    pzOut += sprintf( pzOut, "%s%s';\n", zNameTag, zNameText );
    if (pzMembership != (char*)NULL) {
        strcpy( pzOut, zMemberLine );
        pzOut += sizeof( zMemberLine )-1;
        while (isalnum( *pzMembership ) || (*pzMembership == '_'))
            *pzOut++ = *pzMembership++;
        *pzOut++ = ';';
        *pzOut++ = '\n';
    }

    *ppzOut = pzOut;
    *ppzDef = pzDef;
    *pzDef  = '\n';  /* restore the newline.  Used in pattern match */

    /*
     *  Returning "PROBLEM" means the caller must emit the "#endif\n"
     *  at the end of the definition.
     */
    return (pzIfText != (char*)NULL) ? PROBLEM : SUCCESS;
}


    void
buildDefinition(
    char*    pzDef,
    char*    pzFile,
    int      line,
    char*    pzOut )
{
    tSCC zSrcFile[] = "    %s = '%s';\n";
    tSCC zLineNum[] = "    %s = '%d';\n";

    ag_bool    these_are_global_defs = (*pzDef == '*');
    tSuccess   preamble;
    int        re_res;
    char*      pzNextDef = (char*)NULL;
    regmatch_t match[2];

    if (these_are_global_defs) {
        strcpy( pzOut, zGlobal );
        pzOut += sizeof( zGlobal )-1;
        pzOut += sprintf( pzOut, zLineId, line, pzFile );

        pzDef = strchr( pzDef, '\n' );
        if (pzDef != (char*)NULL)
            pzDef++;
        preamble = PROBLEM;

    } else {
        preamble = buildPreamble( &pzDef, &pzOut, pzFile, line );
        if (FAILED( preamble )) {
            *pzOut = NUL;
            return;
        }
    }

    /*
     *  FOR each attribute for this entry, ...
     */
    for (;;) {
        /*
         *  Find the next attribute regular expression
         */
        re_res = regexec( &attrib_re, pzDef, COUNT( match ), match, 0 );
        switch (re_res) {
        case 0:
            /*
             *  NUL-terminate the current attribute.
             *  Set the "next" pointer to the start of the next attribute name.
             */
            pzDef[ match[0].rm_so ] = NUL;
            if (pzNextDef != (char*)NULL)
                pzOut = emitDefinition( pzNextDef, pzOut );
            pzNextDef = pzDef = pzDef + match[1].rm_so;
            break;

        case REG_NOMATCH:
            /*
             *  No more attributes.
             */
            if (pzNextDef == (char*)NULL) {
                *pzOut++ = '\n'; *pzOut++ = '#';
                sprintf( pzOut,  zNoData, pzFile, line );
                fputs( pzOut, stderr );
                pzOut += strlen( pzOut );
                return;
            }

            pzOut = emitDefinition( pzNextDef, pzOut );
            goto eachAttrDone;
            break;

        default:
        {
            char zRER[ MAXNAMELEN ];
            tSCC zErr[] = "error %d (%s) finding `%s' in\n%s\n\n";
            regerror( re_res, &attrib_re, zRER, sizeof( zRER ));
            *pzOut++ = '\n';
            *pzOut++ = '#';
            sprintf( pzOut, zErr, re_res, zRER, zAttribRe, pzDef );
            fprintf( stderr, "getdefs:  %s", zErr );
            return;
        }
        }
    } eachAttrDone:;

    if (these_are_global_defs) {
        *pzOut = NUL;
        return;
    }

    if (HAVE_OPT( COMMON_ASSIGN )) {
        int    ct  = STACKCT_OPT(  ASSIGN );
        char** ppz = STACKLST_OPT( ASSIGN );
        do  {
            pzOut += sprintf( pzOut, "    %s;\n", *ppz++ );
        } while (--ct > 0);
    }

    if (HAVE_OPT( SRCFILE ))
        pzOut += sprintf( pzOut, zSrcFile, OPT_ARG( SRCFILE ), pzFile );

    if (HAVE_OPT( LINENUM ))
        pzOut += sprintf( pzOut, zLineNum, OPT_ARG( LINENUM ), line );

    /*
     *  IF the preamble had a problem, it is because it could not
     *  emit the final "#endif\n" directive.  Do that now.
     */
    if (HADGLITCH( preamble ))
         strcpy( pzOut, "};\n#endif\n" );
    else strcpy( pzOut, "};\n" );
}



    void
processFile( char* pzFile )
{
    char* pzText = loadFile( pzFile ); /* full text */
    char* pzScan;  /* Scanning Pointer  */
    char* pzDef;   /* Def block start   */
    char* pzNext;  /* start next search */
    char* pzDta;   /* data value        */
    int   lineNo = 1;
    char* pzOut;
    regmatch_t  matches[MAX_SUBMATCH+1];

    if (pzText == (char*)NULL) {
        fprintf( stderr, "Error %d (%s) read opening %s\n",
                 errno, strerror( errno ), pzFile );
        exit( EXIT_FAILURE );
    }

    pzNext = pzText;

    while ( pzScan = pzNext,
            regexec( &define_re, pzScan, COUNT(matches), matches, 0 ) == 0) {

        static const char zNoEnd[] =
            "Error:  definition in %s at line %d has no end\n";
        static const char zNoSubexp[] =
            "Warning: entry type not found on line %d in %s:\n\t%s\n";

        int  linesInDef = 0;

        /*
         *  Make sure there is a subexpression match!!
         */
        if (matches[1].rm_so == -1) {
            char* pz;
            char  ch;

            pzDef = pzScan + matches[0].rm_so;
            if (strlen( pzDef ) > 30) {
                pz  = pzDef + 30;
                ch  = *pz;
                *pz = NUL;
            } else
                pz = (char*)NULL;

            fprintf( stderr, zNoSubexp, pzFile, lineNo, pzDef );
            if (pz != (char*)NULL)
                *pz = ch;
            continue;
        }

        pzDef = pzScan + matches[1].rm_so;
        pzNext = strstr( pzDef, "=*/" );
        if (pzNext == (char*)NULL) {
            fprintf( stderr, zNoEnd, pzFile, lineNo );
            exit( EXIT_FAILURE );
        }
        *pzNext = NUL;
        pzNext += 3;
        /*
         *  Count the number of lines skipped to the start of the def.
         */
        for (;;) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan++ == (char*)NULL)
                break;
            if (pzScan >= pzDef)
                break;
            lineNo++;
        }

        pzOut = pzDta = (char*)malloc( 2 * strlen( pzDef ) + 8000);

        /*
         *  Count the number of lines in the definition itself.
         *  It will find and stop on the "=* /\n" line.
         */
        pzScan = pzDef;
        for (;;) {
            pzScan = strchr( pzScan, '\n' );
            if (pzScan++ == (char*)NULL)
                break;
            linesInDef++;
        }

        /*
         *  OK.  We are done figuring out where the boundaries of the
         *  definition are and where we will resume our processing.
         */
        buildDefinition( pzDef, pzFile, lineNo, pzOut );
        pzDta   = (char*)realloc( (void*)pzDta, strlen( pzDta ) + 1 );
        lineNo += linesInDef;

        if (++blkUseCt > blkAllocCt) {
            blkAllocCt += 32;
            papzBlocks = (char**)realloc( (void*)papzBlocks,
                                          blkAllocCt * sizeof( char* ));
            if (papzBlocks == (char**)NULL) {
                fprintf( stderr, "Realloc error for %d pointers\n",
                         blkAllocCt );
                exit( EXIT_FAILURE );
            }
        }
        papzBlocks[ blkUseCt-1 ] = pzDta;
    }

    if (lineNo == 1)
        fprintf( stderr, "Warning:  no copies of pattern `%s' were found in "
                 "%s\n", pzDefPat, pzFile );

    free( (void*)pzText );
}


    void
printEntries( FILE* fp )
{
    int     ct  = blkUseCt;
    char**  ppz = papzBlocks;

    if (ct == 0)
        exit( EXIT_FAILURE );

    for (;;) {
        char* pz = *(ppz++);
        if (--ct < 0)
            break;
        fputs( pz, fp );
        free( (void*)pz );
        if (ct > 0)
            fputc( '\n', fp );
    }
    free( (void*)papzBlocks );
}


    FILE*
startAutogen( void )
{
    char*  pz;
    FILE*  agFp;

    char zSrch[  MAXPATHLEN ];
    char zBase[  68 ];

    /*
     *  Compute the base name.
     *
     *  If an argument was specified, use that without question.
     *  IF a definition pattern is supplied, and it looks like
     *     a normal name, then use that.
     *  If neither of these work, then use the current directory name.
     */
    if (HAVE_OPT( BASE_NAME ))
        snprintf( zBase, sizeof(zBase), "-b%s", OPT_ARG( BASE_NAME ));

    else {
        /*
         *  IF we have a definition name pattern,
         *  THEN copy the leading part that consists of name-like characters.
         */
        strcpy( zBase, "-b" );
        if (HAVE_OPT( DEFS_TO_GET )) {
            char* pzS = OPT_ARG( DEFS_TO_GET );
            pz = zBase + 2;
            while (isalnum( *pzS ) || (*pzS == '_'))
                *pz++ = *pzS++;
            *pz = NUL;
            if (pz >= zBase + sizeof(zBase)) {
                fputs( "base name length exceeds 64\n", stderr );
                exit( EXIT_FAILURE );
            }
        }

        /*
         *  IF no pattern or it does not look like a name, ...
         */
        if (zBase[2] == NUL) {
            if (getcwd( zSrch, sizeof( zSrch )) == (char*)NULL) {
                fprintf( stderr, "Error %d (%s) on getcwd\n", errno,
                         strerror( errno ));
                exit( EXIT_FAILURE );
            }

            pz = strrchr( zSrch, '/' );
            if (pz == (char*)NULL)
                 pz = zSrch;
            else pz++;
            snprintf( zBase+2, sizeof(zBase)-2, "%s", pz );
        }
    }

    /*
     *  For our template name, we take the argument (if supplied).
     *  If not, then whatever we decided our base name was will also
     *  be our template name.
     */
    if (HAVE_OPT( TEMPLATE )) {
        strcpy( zTemplName, OPT_ARG( TEMPLATE ));
    } else {
        strcpy( zTemplName, zBase+2 );
    }

    /*
     *  Now, what kind of output have we?
     *  If it is a file, open it up and return.
     *  If it is an alternate autogen program,
     *  then set it to whatever the argument said it was.
     *  If the option was not supplied, we default to
     *  whatever we set the "pzAutogen" pointer to above.
     */
    if (HAVE_OPT( AUTOGEN ))
        switch (WHICH_IDX_AUTOGEN) {
        case INDEX_OPT_OUTPUT:
        {
            tSCC   zFileFmt[] = " *      %s\n";
            const char* pzFmt;

            int    ct  = STACKCT_OPT(  INPUT );
            char** ppz = STACKLST_OPT( INPUT );
            FILE*  fp;

            if (strcmp( OPT_ARG( OUTPUT ), "-") == 0)
                return stdout;

            unlink( OPT_ARG( OUTPUT ));
            fp = fopen( OPT_ARG( OUTPUT ), "w" FOPEN_BINARY_FLAG );
            fprintf( fp, zDne, OPT_ARG( OUTPUT ));

            do  {
                fprintf( fp, zFileFmt, *ppz++ );
            } while (--ct > 0);

            fputs( " */\n", fp );
            return fp;
        }

        case INDEX_OPT_AUTOGEN:
            if (! ENABLED_OPT( AUTOGEN ))
                return stdout;

            if (  ( OPT_ARG( AUTOGEN ) != (char*)NULL)
               && (*OPT_ARG( AUTOGEN ) != NUL ))
                pzAutogen = OPT_ARG( AUTOGEN );

            break;
        }

    {
        int  pfd[2];

        if (pipe( pfd ) != 0) {
            fprintf( stderr, "Error %d (%s) creating pipe\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );
        }

        agPid = fork();

        switch (agPid) {
        case 0:
            /*
             *  We are the child.  Close the write end of the pipe
             *  and force STDIN to become the read end.
             */
            close( pfd[1] );
            if (dup2( pfd[0], STDIN_FILENO ) != 0) {
                fprintf( stderr, "Error %d (%s) dup pipe[0]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }
            break;

        case -1:
            fprintf( stderr, "Error %d (%s) on fork()\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );

        default:
            /*
             *  We are the parent.  Close the read end of the pipe
             *  and get a FILE* pointer for the write file descriptor
             */
            close( pfd[0] );
            agFp = fdopen( pfd[1], "w" FOPEN_BINARY_FLAG );
            if (agFp == (FILE*)NULL) {
                fprintf( stderr, "Error %d (%s) fdopening pipe[1]\n",
                         errno, strerror( errno ));
                exit( EXIT_FAILURE );
            }
            return agFp;
        }
    }

    {
        tCC**  paparg;
        tCC**  pparg;
        int    argCt = 5;

        /*
         *  IF we don't have template search directories,
         *  THEN allocate the default arg counter of pointers and
         *       set the program name into it.
         *  ELSE insert each one into the arg list.
         */
        if (! HAVE_OPT( AGARG )) {
            paparg = pparg = (tCC**)malloc( argCt * sizeof( char* ));
            *pparg++ = pzAutogen;

        } else {
            int    ct  = STACKCT_OPT(  AGARG );
            char** ppz = STACKLST_OPT( AGARG );

            argCt += ct;
            paparg = pparg = (tCC**)malloc( argCt * sizeof( char* ));
            *pparg++ = pzAutogen;

            do  {
                *pparg++ = *ppz++;
            } while (--ct > 0);
        }

        *pparg++ = zBase;
        *pparg++ = "--";
        *pparg++ = "-";
        *pparg++ = (char*)NULL;

#ifdef DEBUG
        fputc( '\n', stderr );
        pparg = paparg;
        for (;;) {
            fputs( *pparg++, stderr );
            if (*pparg == (char*)NULL)
                break;
            fputc( ' ', stderr );
        }
        fputc( '\n', stderr );
        fputc( '\n', stderr );
#endif

        execvp( pzAutogen, (char**)paparg );
        fprintf( stderr, "Error %d (%s) exec of %s %s %s %s\n",
                 errno, strerror( errno ),
                 paparg[0], paparg[1], paparg[2], paparg[3] );
        exit( EXIT_FAILURE );
    }

    return (FILE*)NULL;
}


    char*
loadFile( char* pzFname )
{
    FILE*  fp = fopen( pzFname, "r" FOPEN_BINARY_FLAG );
    int    res;
    char*  pzText;
    char*  pzRead;
    size_t rdsz;

    if (fp == (FILE*)NULL)
        return (char*)NULL;
    /*
     *  Find out how much data we need to read.
     *  And make sure we are reading a regular file.
     */
    {
        struct stat stb;
        res = fstat( fileno( fp ), &stb );
        if (res != 0) {
            fprintf( stderr, "error %d (%s) stat-ing %s\n",
                     errno, strerror( errno ), pzFname );
            exit( EXIT_FAILURE );
        }
        if (! S_ISREG( stb.st_mode )) {
            fprintf( stderr, "error file %s is not a regular file\n",
                     pzFname );
            exit( EXIT_FAILURE );
        }
        rdsz = stb.st_size;
        if (rdsz < 16) {
            fprintf( stderr, "Error file %s only contains %d bytes.\n"
                     "\tit cannot contain autogen definitions\n",
                     pzFname, rdsz );
            exit( EXIT_FAILURE );
        }
    }

    /*
     *  Allocate the space we need for the ENTIRE file.
     */
    pzRead = pzText = (char*)malloc( rdsz + 1 );
    if (pzText == (char*)NULL) {
        fprintf( stderr, "Error: could not allocate %d bytes\n",
                 rdsz + 1 );
        exit( EXIT_FAILURE );
    }

    /*
     *  Read as much as we can get until we have read the file.
     */
    do  {
        size_t rdct = fread( (void*)pzRead, 1, rdsz, fp );

        if (rdct == 0) {
            fprintf( stderr, "Error %d (%s) reading file %s\n",
                     errno, strerror( errno ), pzFname );
            exit( EXIT_FAILURE );
        }

        pzRead += rdct;
        rdsz   -= rdct;
    } while (rdsz > 0);

    *pzRead = NUL;
    fclose( fp );
    return pzText;
}
