
/*
 *  xml2ag.c
 *  $Id: xml2ag.c,v 1.6 2002/06/28 02:48:05 bkorb Exp $
 *  This is the main routine for xml2ag.
 */

/*
 *  xml2ag copyright 1992-2002 Bruce Korb
 *
 *  xml2ag is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  xml2ag is distributed in the hope that it will be useful,
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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "xmlopts.h"

tSCC zConflict[] =
    "the file name operand conflicts with the definitions option.\n";

tSCC zTextFmt[] =
    "text = '%s';\n";


static const char* typeName[] = {
    "0 - inval",
    "ELEMENT_NODE",
    "ATTRIBUTE_NODE",
    "TEXT_NODE",
    "CDATA_SECTION_NODE",
    "ENTITY_REF_NODE",
    "ENTITY_NODE",
    "PI_NODE",
    "COMMENT_NODE",
    "DOCUMENT_NODE",
    "DOCUMENT_TYPE_NODE",
    "DOCUMENT_FRAG_NODE",
    "NOTATION_NODE",
    "HTML_DOCUMENT_NODE",
    "DTD_NODE",
    "ELEMENT_DECL",
    "ATTRIBUTE_DECL",
    "ENTITY_DECL",
    "NAMESPACE_DECL",
    "XINCLUDE_START",
    "XINCLUDE_END",
    "DOCB_DOCUMENT_NODE" };

int    level = 0;

#undef STATIC
#ifdef DEBUG
# define STATIC
#else
# define STATIC static
#endif

FILE* outFp;

/* = = = START-STATIC-FORWARD = = = */
STATIC char*
loadFile( FILE* fp, size_t* pzSize );

STATIC void
emitIndentation( void );

STATIC char*
trim( const char* pzSrc, size_t* pSz );

STATIC xmlNodePtr
printHeader( xmlDocPtr pDoc );

STATIC void
printAttrs( xmlAttrPtr pAttr );

STATIC int
printNode( xmlNodePtr pNode );

STATIC void
printChildren( xmlNodePtr pNode );

/* = = = END-STATIC-FORWARD = = = */
#define TRIM(s,psz) trim( (const char*)(s), (size_t*)(psz) )


int
main( int argc, char** argv )
{
	xmlDocPtr pDoc;
    char*     pzFile = NULL;

	{
		int ct = optionProcess( &xml2agOptions, argc, argv );
		argc -= ct;
		argv += ct;

        switch (argc) {
        case 1:
            if (strcmp( *argv, "-" ) != 0) {
                if (HAVE_OPT( DEFINITIONS )) {
                    fprintf( stderr, zConflict );
                    USAGE( EXIT_FAILURE );
                }
                pzFile = *argv;
                break;
            }
        case 0:
            if (   HAVE_OPT( DEFINITIONS )
               && (strcmp( OPT_ARG( DEFINITIONS ), "-" ) != 0) )

                pzFile = OPT_ARG( DEFINITIONS );
            break;

        default:
            fprintf( stderr, "only one argument allowed\n" );
            return EXIT_FAILURE;
        }
	}

    if (! HAVE_OPT( OUTPUT ))
        forkAutogen( pzFile );
    else
        outFp = stdout;

	if (pzFile != NULL) {
        fprintf( outFp, "/* Parsing file %s */\n", pzFile );
		pDoc = xmlParseFile( pzFile );
    }
	else {
		size_t sz;
		char*  pz = loadFile( stdin, &sz );
		pDoc = xmlParseMemory( pz, sz );
        fprintf( outFp, "/* Parsed from stdin */\n" );
	}

    {
        xmlNodePtr pRoot = printHeader( pDoc );
        printAttrs( pRoot->properties );
        switch (pDoc->type) {
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            printChildren( pRoot->children );
            break;
        default:
            fprintf( outFp, "/* type %d doc is not DOCUMENT or HTML_DOCUMENT */\n",
                    pDoc->type );
        }
    }

    xmlCleanupParser();
	return 0;
}


#define CHUNK_SZ  4096
STATIC char*
loadFile( FILE* fp, size_t* pzSize )
{
	size_t  asz = CHUNK_SZ;
	size_t  usz = 0;
	char*   mem = malloc( asz );

	for (;;) {

		if ((usz + CHUNK_SZ) > asz) {
			asz += CHUNK_SZ;
			mem = realloc( mem, asz );
		}

        if (mem == NULL) {
            fprintf( stderr, "Cannot allocate %d byte bufer\n", asz );
            exit( EXIT_FAILURE );
        }

		{
			size_t rdct = fread( mem + usz, 1, CHUNK_SZ, fp );
			usz += rdct;
			if (rdct < CHUNK_SZ)
				break;
		}
	}

	*pzSize = usz;
	return mem;
}


STATIC void
emitIndentation( void )
{
    int indent = level * 2;
    while (--indent >= 0) fputc( ' ', outFp );
}


STATIC char*
trim( const char* pzSrc, size_t* pSz )
{
    static char z[ 4096 * 16 ];
    size_t len;
    const char* pzEnd;

    if (pzSrc == NULL) {
        z[0] = '\0';
        if (pSz != NULL) *pSz = 0;
        return z;
    }

    while (isspace( *pzSrc ))  pzSrc++;
    pzEnd = pzSrc + strlen( pzSrc );
    while ((pzEnd > pzSrc) && isspace( pzEnd[-1] ))  pzEnd--;

    if (pzEnd <= pzSrc) {
        z[0] = '\0';
        if (pSz != NULL) *pSz = 0;
    } else {
        char* pzDest = z;
        for (;;) {
            switch (*(pzDest++) = *(pzSrc++)) {
            case '\'': strcpy( (pzDest++)-1, "\\'" ); break;
            case '\\': *(pzDest++) = '\\'; break;
            }
            if (pzSrc == pzEnd)
                break;
        }

        *pzDest = '\0';
        if (pSz != NULL) *pSz = pzDest - &(z[0]);
    }

    return z;
}

STATIC xmlNodePtr
printHeader( xmlDocPtr pDoc )
{
    tSCC zDef[] = "AutoGen Definitions %s%s;\n";
    char* pzSfx = ".tpl";

    xmlNodePtr pRootNode = xmlDocGetRootElement( pDoc );
    xmlChar*   pTpl = NULL;
    xmlChar*   pzTpl;

    if (pRootNode == NULL) {
        fprintf( stderr, "Root node not found\n" );
        exit( EXIT_FAILURE );
    }

    if (HAVE_OPT( OVERRIDE_TPL )) {
        if (strchr( OPT_ARG( OVERRIDE_TPL ), '.' ) != NULL)
            pzSfx = "";
        pzTpl = (xmlChar*)OPT_ARG( OVERRIDE_TPL );
    }
    else {
        pTpl = xmlGetProp( pRootNode, (xmlChar*)"template" );
        if (pTpl == NULL) {
            fprintf( stderr, "No template was specified.\n" );
            exit( EXIT_FAILURE );
        }

        pzTpl = pTpl;
        if (strchr( pzTpl, '.' ) != NULL)
            pzSfx = "";
    }

    fprintf( outFp, zDef, pzTpl, pzSfx );
    if (pTpl != NULL)
        free( pTpl );

    if (pDoc->name != NULL)
        fprintf( outFp, "XML-name = '%s';\n", TRIM( pDoc->name, NULL ));

    if (pDoc->version != NULL)
        fprintf( outFp, "XML-version = '%s';\n", TRIM( pDoc->version, NULL ));

    if (pDoc->encoding != NULL)
        fprintf( outFp, "XML-encoding = '%s';\n", TRIM( pDoc->encoding, NULL ));

    if (pDoc->URL != NULL)
        fprintf( outFp, "XML-URL = '%s';\n", TRIM( pDoc->URL, NULL ));

    if (pDoc->standalone)
        fputs( "XML-standalone = true;\n", outFp );

    return pRootNode;
}

STATIC void
printAttrs( xmlAttrPtr pAttr )
{
    while (pAttr != NULL) {
        char* pzCont = pAttr->children->content;

        emitIndentation();
        fputs( pAttr->name, outFp );
        fputs( " = ", outFp );
        if (pAttr->children->children == NULL)
            fprintf( outFp, "'%s';\n", TRIM( pzCont, NULL ));
        else {
            fputs( "{\n", outFp );
            level++;
            if (pzCont != NULL) {
                emitIndentation();
                fprintf( outFp, zTextFmt, TRIM( pzCont, NULL ));
            }
            printChildren( pAttr->children->children );
            level--;
            emitIndentation();
            fputs( "};\n", outFp );
        }

        pAttr = pAttr->next;
    }
}


STATIC int
printNode( xmlNodePtr pNode )
{
    switch (pNode->type) {
    case XML_ELEMENT_NODE:
    {
        size_t sz;
        char*  pzTxt;
        emitIndentation();
        fputs( pNode->name, outFp );
        pzTxt = TRIM( pNode->content, &sz );

        if (  (pNode->properties == NULL)
           && (pNode->children == NULL)) {

            if (sz == 0)
                 fputs( ";\n", outFp );
            else fprintf( outFp, " = '%s';\n", pzTxt );
            break;
        }

        fputs( " = {\n", outFp );
        level++;
        emitIndentation();
        fprintf( outFp, "content = '%s';\n", pzTxt );
        printAttrs( pNode->properties );
        printChildren( pNode->children );
        level--;
        emitIndentation();
        fputs( "};\n", outFp );
        break;
    }

    case XML_ATTRIBUTE_NODE:
        fputs( "Misplaced attribute\n", outFp );
        exit( EXIT_FAILURE );

    case XML_TEXT_NODE:
    {
        size_t sz;
        char* pzTxt = TRIM( pNode->content, &sz );
        if (sz == 0)
            break;
        emitIndentation();
        fprintf( outFp, zTextFmt, pzTxt );
        break;
    }

    case XML_COMMENT_NODE:
    {
        size_t sz;
        char* pzTxt = TRIM( pNode->content, &sz );
        if (sz == 0)
            break;

        emitIndentation();
        fputs( "/* ", outFp );
        for (;;) {
            char* pz = strstr( pzTxt, "*/" );
            if (pz == NULL)
                break;
            fwrite( pzTxt, (pz - pzTxt) + 1, 1, outFp );
            pzTxt = pz+1;
            fputc( ' ', outFp );
        }
        fprintf( outFp, "%s */\n", pzTxt );
        break;
    }

    case XML_CDATA_SECTION_NODE:
    case XML_ENTITY_REF_NODE:
    case XML_ENTITY_NODE:
    case XML_PI_NODE:

    case XML_DOCUMENT_NODE:
    case XML_HTML_DOCUMENT_NODE:
    case XML_DOCUMENT_TYPE_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_NOTATION_NODE:
    case XML_DTD_NODE:
    case XML_ELEMENT_DECL:
    case XML_ATTRIBUTE_DECL:
    case XML_ENTITY_DECL:
    case XML_NAMESPACE_DECL:
    case XML_XINCLUDE_START:
    case XML_XINCLUDE_END:
        emitIndentation();
        fprintf( outFp, "/* Unsupported XML node type:  %s */\n",
                typeName[ pNode->type ]);
        break;

    default:
        emitIndentation();
        fprintf( outFp, "/* Unknown XML node type %d */\n", pNode->type );
        break;
    }
}


STATIC void
printChildren( xmlNodePtr pNode )
{
    while (pNode != NULL) {
        printNode( pNode );
        pNode = pNode->next;
    }
}

/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * tab-width: 4
 * End:
 * end of autogen.c */