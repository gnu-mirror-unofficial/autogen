[= autogen template -*-texinfo-*-
#
#  Documentation template
#  $Id: auto_gen.tpl,v 1.3 1998/06/22 03:10:19 bkorb Exp $
#
texi=autogen.texi =]
\input texinfo
@c %**start of header
@setfilename [=prog_name _down=].info
@settitle [=prog_name _Cap=]
@setchapternewpage off
@c %**end of header

@ignore
[=_eval "" _DNE=]
@end ignore

@set EDITION [=version=]
@set VERSION [=version=]
@set UPDATED [=_EVAL 'date "+%B %Y"' _shell =]

@dircategory GNU programming tools
@direntry
* [=prog_name=]: ([=prog_name=]).         [=prog_title=]
@end direntry

@ifinfo
This file documents [=package=] Version @value{VERSION}

[=prog_name _cap=] copyright @copyright{} [=copyright=] Bruce Korb

[=prog_name _cap "" _gpl=]

@ignore
Permission is granted to process this file through TeX and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph
@end ignore
@end ifinfo


@titlepage
@title [=prog_title=]
@subtitle For version @value{VERSION}, @value{UPDATED}
@author Bruce Korb

@page
@vskip 0pt plus 1filll
[=prog_name _cap copyright _get owner _get
       "#3$%s copyright %s %s" _printf=]
@sp 2
This is the first edition of the GNU [=prog_name _Cap=] documentation,
@sp 2
Published by Bruce Korb, 910 Redwood Dr., Santa Cruz, CA  95060

[=prog_name _cap "" _gpl=]
@end titlepage

@ifinfo
@node Top, Introduction, , (dir)
@top [=prog_title=]
@comment  node-name,  next,  previous,  up

This file documents the [=prog_title=] package for creating an arbitrary
text files containing repetitive text with varying substitutions.
This edition documents version @value{VERSION}, @value{UPDATED}.

@menu
* Introduction::         [=prog_name _cap=]'s purpose
* Generalities::         General ideas
* Example Usage::        A Simple Example
* Definitions File::     Macro values used for control and replacements
* Template File::        Output specification
* Invocation::           Running the program
* Autoopts::             Automatic option processor generation
* Future::               Some ideas for the future
* Concept Index::        General index
* Function Index::	 Function index
@end menu

@end ifinfo

@node Introduction
@chapter Introduction
@cindex Introduction

[=prog_name _cap=] is a tool for automatically generating arbitrary text
files that contain repetitive text with varying substitutions.
This is particularly useful if you have several types of repetitive
text that all need to be kept in sync with each other.

One application for this would be, for example, processing
program options.  Processing options requires a minimum of four
different constructs be kept in proper order in different places in
your program.  You need minimally:

@enumerate
@item
the flag character in the flag string
@item
code to process the flag when it is encountered
@item
a global variable (typically something like:  @samp{int sflg = 0;})
@item
and please do not forget a line in the usage text :-)
@end enumerate

@noindent
You will need more things besides this if you choose to implement
long option names, rc/ini file processing, and environment variables.

All of these things can be kept in sync mechanically,
with the proper templates and this program.
In fact, I have already done so and @code{[=prog_name=]} already
uses the AutoOpt facility.

@node Generalities
@chapter General ideas

@cindex m4
The goal is to try to simplify the process of maintaining
repetitive program text.  If there is a single block of text
that needs repetitive substitutions, @code{#define}
macros frequently fill the bill.  Sometimes, though, they are
not quite powerful enough and you will need to use something
like @code{m4}.  When appropriate, those approaches are much
simpler than [=prog_name _down=].

@cindex design goals
[=prog_name _cap=] has been designed for
addressing the problem when there are the same or similar
substitutions required in multiple blocks of repeating text.
@code{#define}s do not always work because they are inflexible
and even sometimes obscure.  @code{m4} is often inadequate
because the entire substitution lists must be repeated in
the proper contexts throughout the base text (template).

[=prog_name _cap=] addresses these problems by separating
the substitution text (macro definitions) from the template
text.  The template text makes references to the definitions
in order to compute text to insert into the template and
to compute which part of the template to process at each step.

@table @samp
@item template text
This is the template used to create the output files.
It contains text that is copied to the output verbatim and
text between escape markers (denoted at the beginning of the file).
The text between markers is used to generate replacement text,
control the output or to control the processing of the template.

@item macro definitions
This file is named on the @code{[=prog_name=]} invocation line.
It identifies the template that the definitions are to be used with
and it contains text macro and group macro definitions.
@end table

@node Example Usage
@chapter A Simple Example
@cindex example, simple
@cindex simple example

Assume you have a list of names and you wish to keep them in an array
and you want an enumerated index for each entry and that it is too
much trouble to do by hand:

@noindent
You have list.h:

@example
#define IDX_ALPHA    0
#define IDX_BETA     1
#define IDX_OMEGA    2

extern const char* az_name_list[ 3 ];
#endif
@end example

@noindent
and you have list.c:

@example
#include "list.h"
const char* az_name_list[ 3 ] = @{
        "some alpha stuff",
        "more beta stuff",
        "dumb omega stuff" @};
@end example

To do this, you need a template or two that can be expanded
into the files you want.  In this program, we use a single
template that is capable of multiple output files.

Assuming we have chosen our markers to be '[#' and '#]' to
start and end our macros, we have a template something like this
(a full description of this is in the next chapter, "Complete Syntax"):

@example
[#[=prog_name=] template h c #]
[#_IF _SFX h = #]
[#_FOR list#]
#define IDX_[#list_element _up#]  [#_eval _index#][# /list #]

extern const char* az_name_list[ [#_eval list _hival 1 + #] ];
#endif[#

_ELIF _SFX c = #]
#include "list.h"
const char* az_name_list[ [#_eval list _hival 1 + #] ] = @{[#
_FOR list ,#]
        "[#list_info#]"[#
/list #] @};[#

_ENDIF header/program #]
@end example

@noindent
and along with this must be included the macro definitions:

@example
[=prog_name=] definitions list;
list = @{ list_element = alpha;
         list_info    = "some alpha stuff"; @};
list = @{ list_element = beta;
         list_info    = "more beta stuff"; @};
list = @{ list_element = omega;
         list_info    = "dumb omega stuff"; @};
@end example

@noindent
Furthermore, if we ever need a name/enumeration mapping again,
we can always write a new set of definitions for the old template :-).

@node Definitions File
@chapter Macro values used for control and substitution.
@cindex definitions file

This file consists of an identity statement that identifies it as a
[=prog_name=] file, followed by block and text macro definitions.
Intermingled may be C-style comments and C preprocessing directives.
All C preprocessing directives are identified, but many
@strong{especially @code{if}} are ignored.

@menu
* Identification::  The first definition
* Definitions::     Text macros and block macros
* Directives::      Controlling what gets processed
* Comments::        Documenting your definitions
* Example::         What it all looks like
* Full Syntax::     YACC Language Grammar
@end menu

@node Identification
@section The first definition
@cindex identification

The first definition in this file is used to identify it as a
[=prog_name=] file.  It consists of the two keywords,
@samp{[=prog_name=]} and @samp{definitions} followed by the default
template name and a terminating semi-colon @code{;}.

@cindex template, file name
@cindex .tpl, file name
@cindex tpl, file name

@noindent
The template file is determined from the template name in the following way:

@enumerate
@item
The template name is appended, if needed, with the string, ".tpl".
@item
@cindex Templ-Dirs
The file is looked for in the current directory.
@item
The file is looked for in the directories named
in the @code{Templ-Dirs} options on the command line,
in the order specified.
@item
The file is looked for in the @code{[=prog_name=]} executable's directory
@item
If the file is not found in any of these locations,
a message is printed and @code{[=prog_name=]} exits.
@end enumerate

For a @code{foobar} template, your identification definition will look
like this:

@example
[=prog_name _cap=] Definitions foobar;
@end example

@noindent
Note that, other than the name @code{foobar}, the words @samp{[=prog_name=]}
and @samp{definitions} are searched for without case sensitivity.
Most lookups in this program are case insensitive.

@node Definitions
@section Text macros and block macros
@cindex macros
@cindex text macros
@cindex block macros

Any macro may appear multiple times in the definition file.
If there is more than one instance, the @strong{only} way
to expand all of the copies of it is by using the @code{_FOR}
text function on the macro, as described in the next chapter.

There are two kinds of macro definitions, @samp{text} and @samp{block}.
Macros are defined thus:

@example
block_name '=' '@{' definition-list '@}' ';'

text_name '=' string ';'

no_text_name ';'
@end example

@noindent
The macro names may be a simple name taking the next available index,
or may specify an index by name or number.  For example:

@example
mac_name
mac_name[2]
mac_name[ DEF_NAME ]
@end example
@noindent
@code{DEF_NAME} must be defined to have a numeric value.
If you do specify an index, you must take care not to cause conflicts.

@noindent
@code{No_text_name} is a text definition with a shorthand empty string
value.

@noindent
@code{definition-list} is a list of definitions that may or may not
contain nested block definitions.  Any such definitions may @strong{only}
be expanded within the a @code{FOR} block iterating over the
containing block macro.

@cindex text macros, format
The string values for the text macros may be specified in one of four
quoting rules:

@table @samp
@item with a double quote @code{"}
@cindex string, double quote
The string follows the
C-style escaping (@code{\\}, @code{\n}, @code{\f}, @code{\v}, etc., plus
octal character numbers specified as @code{\ooo}.  The difference
from "C" is that the string may span multiple lines.

@item with a single quote "'"
@cindex string, single quote
This is similar to the shell single-quote string.  However, escapes
@code{\\} are honored before another escape, single quotes @code{'}
and hash characters @code{#}.  This latter is done specifically
to disambiguate lines starting with a hash character inside
of a quoted string.  In other words,

@example
foo = '
#endif
';
@end example

could be misinterpreted by the definitions parser, whereas
this would not:

@example
foo = '
\#endif
';
@end example

@item with a back quote @code{`}
@cindex string, shell output
This is treated identically with the double quote, except that the
resulting string is written to a shell server process and the macro
takes on the value of the output string.

NB:  The text in interpreted by a server shell.  There may be
left over state from previous @code{`} processing and it may
leave state for subsequent processing.  However, a @code{cd}
to the original directory is always issued before the new
command is issued.

A definition utilizing a backquote may not be joined with any other text.

@item without surrounding quotes
The string must not contain any of the characters special to the
definition text.  E.g. @code{;}, @code{"}, @code{'}, @code{`}, @code{=},
@code{@{}, @code{@}}, @code{[}, @code{]}, @code{#} or any
white space character.  Basically, if the string looks like it is a
normal file name or variable name, and it is not one of two keywords
(@samp{[=prog_name=]} or @samp{definitions}) then it is OK to not quote it.
@end table

If the single or double quote characters is used, then you
also have the option, a la ANSI-C syntax, of implicitly
concatenating a series of them together, with intervening
white space ignored.

NOTE: You @strong{cannot} use directives to alter the string
content.  That is,

@example
str = "foo"
#ifdef LATER
      "bar"
#endif
      ;
@end example

@noindent
will result in a syntax error.  However,

@example
str = "foo
#ifdef LATER
      bar
#endif\n";
@end example

@noindent
@strong{Will} work.  It will enclose the @samp{#ifdef LATER}
and @samp{#endif} in the string.  But it may also wreak
havoc with the definition processing directives.  The hash
characters in the first column should be disambiguated with
an escape @code{\\} or join them with previous lines:
@code{"foo\n#ifdef LATER...}.

@node Directives
@section Controlling what gets processed
@cindex directives

Directives can @strong{only} be reliably processed if the '#' character
is the first character on a line.  Also, if you want a '#' as the first
character of a line in one of your string assignments, you should either
escape it by preceeding it with a backslash @samp{\}, or by embedding
it in the string as in @samp{"\n#"}.

All of the normal C preprocessing directives are recognized,
plus an additional @code{#shell} @code{#endshell} pair.
One minor difference though is that [=prog_name=]
directives must have the hash character @code{#} in column 1.
Another difference is that several of them are ignored.  They are:
[=
_FOR agdirect_func =][=
  _IF dummy _exist
    =]@samp{#[=name _down=]}, [=
  _ENDIF =][=
/agdirect_func=] and @samp{#if}.
Note that when ignoring the @code{#if} directive, all intervening
text through its matching @code{#endif} is also ignored,
including the @code{#else} clause.

The [=prog_name=] directives that affect the processing of
definitions are:

@table @code[=
_FOR agdirect_func "\n"=][=
  _IF text _exist
    =]
@item #[=name _down =][=
    _IF arg _exist =] [=arg=][=
    _ENDIF=]
@cindex #[=name _down=]
@cindex [=name _down=] directive
[=text=][=
  _ENDIF=][=
/agdirect_func=]
@end table

@node Comments
@section Documenting your definitions
@cindex comments

The definitions file may contain C-style comments.

@example
/*
 *  This is a comment.
 *  It continues for several lines and suppresses all processing
 *  until the closing characters '*' and '/' appear together.
#include is ignored.
 */
@end example

@node Example
@section What it all looks like

@noindent
This is an extended example:

@example
[=prog_name=] definitions @samp{template-name};
/*
 *  This is a comment that describes what these
 *  definitions are all about.
 */
globel = "value for a global text macro.";

/*
 *  Include a standard set of definitions
 */
#include standards.def

a_block = @{
    a_field;
    /*
     *  You might want to document sub-block macros, too
     */
    a_subblock = @{
        sub_name  = first;
        sub_field = "sub value.";
    @};

#ifdef FEATURE
    /*
     *  This definition is applicable only if FEATURE
     *  has been defined during the processing of the definitions.
     */
    a_subblock = @{
        sub_name  = second;
    @};
#endif

@};

a_block = @{
    a_field = here;
@};
@end example

@node    Full Syntax
@section YACC Language Grammar

Extracted fromt the agParse.y source file:

@example
[=_eval
  "sed -n -e'/^definitions/,$p' $top_srcdir/src/agParse.y |
  sed -e's/{/@{/g' -e's/}/@}/g' "
  _shell=]
@end example

@node Template File
@chapter Output specification
@cindex template file
@cindex .tpl file
@cindex tpl file

The template is really composed of two parts.
The first part consists of a pseudo macro invocation and commentary.
It is followed by the template proper.

@cindex pseudo macro
@cindex macro, pseudo
This pseudo macro is special.  It is used to identify the file as a
[=prog_name=] template file, fixing the starting and ending marks for
the macro invocations in the rest of the file, and specifying the list
of suffixes to be generated by the template.

@menu
* template id::      Format of the pseudo macro
@end menu

Auto-genning a file consists of copying text from the template proper
to the output file until a start macro marker is found.  The text from
the start marker to the end marker constitutes the macro text.  If it
starts with a hash mark @code{#}, then the macro is a comment.  If it
starts with an underscore @code{_}, then it is an explicit function
invocation.  These functions are listed below.  If it starts with an
alphabetic character, then an implicit function will be invoked.  See
the discussion below.  If it begins with any other character (white
space being ignored), then it is in error and processing will stop.

The following macro functions are currently supported.  The
@code{eval} functionality is also used by many of the
functions before examining their argument lists:

@menu[=
_FOR agfunc_func =][=
  _IF unnamed _exist ! =]
* [=name #:: + "#%-16s" _printf=] [=
    _IF deprecated _exist
      =][=deprecated=][=
    _ELSE
      =][=what=][=
    _ENDIF =][=
  _ENDIF =][=
/agfunc_func=]
@end menu

If the macro text begins with an alphabetic character, the
macro function invoked is selected by default.  The default
depends on the type of macro named by the first token in the
macro.  @xref{Definitions File} chapter, for detailed
information on how macro types are determined.

@table @samp
@item block macros
The @code{FOR} function is invoked over the text that starts
after the closing @code{end} mark and ends just before the
opening @code{mark} marker containing a slash @code{/} and
the block macro name.

@item text macros
The @code{EVAL} function is invoked, including all the
processing arguments.  If there are multiple copies of the
text macro, the one with the highest index is selected.  If
you wish to retrieve all the defined values for a text macro
named @code{txt}, you would need to code:

@example
[#_FOR txt#][#txt#][#/txt#]
@end example
@noindent
Inside the domain of the @code{_FOR} function, only the
@code{txt} value for the current index is visible.

@item undefined macros
It is treated as a comment; no function is invoked.
@end table

@noindent
A @code{template block} is template text that does not have any
incomplete @code{case}, @code{for} or @code{if} macro functions.

@node template id
@section Format of the pseudo macro
@cindex template id

The starting macro marker must be the first non-white space characters
encountered in the file.  The marker consists of all the contiguous
ASCII punctuation characters found there.  With optional intervening
white space, this marker must be immediately followed by the keywords,
"[=prog_name=]" and "template".  Capitalization of these words is not
important.  This is followed by zero, one or more suffix specifications.

Suffix specifications consist of a sequence of POSIX compliant file name
characters and, optionally, an equal sign and a file name formatting
string.  Two string arguments are allowed for that string: the base name
of the definition file and the current suffix (that being the text to
the left of the equal sign).  (Note: "POSIX compliant file name
characters" consist of alphanumerics plus the period (@code{.}), hyphen
(@code{-}) and underscore (@code{_}) characters.)
If there are no suffix specifications, then the generated file will
be written to the stdout file descriptor.

The pseudo macro ends with an end macro marker.  Like the starting macro
marker, it consists of a contiguous sequence of arbitrary punctuation
characters.  However, additionally, it may not begin with any of the
POSIX file name characters (@code{.}, @code{-} or @code{_}) and it may
not contain the starting macro.

This pseudo macro may appear on one or several lines of text.
Intermixed may be comment lines (completely blank or starting with the
hash character @code{#} in column 1), and file content markers (text
between @code{-*-} pairs on a single line).  This may be used to
establish editing "modes" for the file.  These are ignored by
[=prog_name=].

The template proper starts after the pseudo-macro.
The starting character is the first character that meets one
of the following conditions:

@enumerate
@item
It is the first character of a start-macro marker, or
@item
the first non-whitespace character, or
@item
the first character after a new-line.
@end enumerate

So, assuming we want to use @code{[#} and @code{#]} as the start and
end macro markers, and we wish to produce a @file{.c} and a @file{.h}
file, then the first macro invocation will look something like this:

@example
[#[=prog_name=] template -*- Mode: emacs-mode-of-choice -*-

h=chk-%s.h

# it is important that the end macro mark appear after
# useful text.  Otherwise, the '#]' by itself would be
# seen as a comment and ignored.

c #]
@end example

Note:  It is generally a good idea to use some sort of opening
bracket in the starting macro and closing bracket in the ending
macro  (e.g. @code{@{}, @code{(}, @code{[}, or even @code{<}
in the starting macro).  It helps both visually and with editors
capable of finding a balancing parenthesis.

[=

#  FOR each defined function,
      this code will insert the extracted documentation =][=

_FOR agfunc_func =][=
  _IF unnamed _exist ! =]

@node [=name=]
@section [=
    _IF deprecated _exist
      =][=deprecated=][=
    _ELSE
      =][=what=][=
    _ENDIF =]
@findex [=name _up=][=
    _FOR cindex =]
@cindex [=cindex=][=
    /cindex=]

[=desc=][=
    _IF table _exist =][=
      _CASE table _get =][=
      _ agexpr_func =]
@table @samp[=
         _FOR agexpr_func =]
@findex [=name _up=]
@item [=name _up=]
[=descrip=]
[=
         /agexpr_func
=]
@end table[=
      _ESAC =][=
    _ENDIF =][=

  _ENDIF "unnamed does not exist" =][=
/agfunc_func=]

@node Invocation
@chapter Running the program
@cindex invocation

@code{[=prog_name _cap=]} accepts the following options,
as shown in this AutoOpt generated usage text:

@example
[=_eval "#../src/" prog_name _get + "# --help 2>&1 |" +
 "sed -e 's/{/@{/' -e 's/}/@}/'" + _shell=]
@end example

@node Autoopts
@chapter Automatic option processor generation
@cindex autoopts

@code{[=prog_name _cap=]} comes with option processing code generated by
@code{autoopts}.  Autoopts is an @code{[=prog_name _down=]} template and a C
library which, when combined with a program specific definitions file,
generates some C code which can be compiled and linked into an application to
process that program's command line options.

At first, it might seem that if @code{[=prog_name _down=]} depends on the
@code{autoopts} library before it can be linked, and that if @code{autoopts}
requires an installed @code{[=prog_nane _down=]} to combine the
@code{autoopts} template with the @code{[=prog_name=]} options definition
file, that bootstrapping is impossible.  @code{[=prog_name _cap=]} comes with
an @file{opts.c} and @file{opts.h} generated from the @code{autoopts}
template, so @code{[=prog_name _down=]}  is not required to build and install
the distribution.

@ifinfo
At the moment @code{[=prog_name _down=]} @strong{does} require a @sc{POSIX}
regular expression library, such as @sc{GNU} regex.  Version 5 will probably
require the @sc{GNU} guile library too.  Other @code{[=prog_name _down=]}
based packages are free to assume the presence of all of these; the
interdependencies of these packages are thus:

@ignore
TODO: Figure out how to include an eps diagram in the printed manual.

@end ignore
@example
                                  .--------(autogetopts)
                                  V              |
                autoopts<---->autogen--------.   |
                   |              |          |   |
                   |              V          |   |
                   |           libguile--.   |   |
                   |                     V   V   V
                   `------------------->POSIX regex
@end example
@end ifinfo


@node Future
@chapter Some ideas for the future
@cindex futures

Here are some things that might happen in the @strong{way} distant
future.  It is conceivable because their processing is mostly done
by doing what autogen was designed to do, but it is done inside of
miserably complex perl, shell, sed and awk scripts.

@itemize @bullet

@item
fix automake :-)

@item
fix autoconf :-)
@end itemize

@node Concept Index
@unnumbered Concept Index

@printindex cp

@node Function Index
@unnumbered Function Index

@printindex fn

@contents
@bye
