#! /bin/sh

## texi2man.sh -- script to convert texi-isms to man page isms
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2020 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following sha256 sums:
##
##  8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
##  4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
##  13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd

## This "library" converts texi-isms into man-isms.  It gets included
## by the man page template at the point where texi-isms might start appearing
## and then "emit-man-text" is invoked when all the text has been assembled.
##
## Display the command line prototype,
## based only on the argument processing type.
##
## And run the entire output through "sed" to convert texi-isms

nl='
'
sedcmd=
bracket='{\([^}]*\)}'
replB='%BACKSLASH%fB\1%BACKSLASH%fP'
replI='%BACKSLASH%fI\1%BACKSLASH%fP'

for f in code command var env dvn samp option strong
do
    sedcmd="${sedcmd}s;@${f}${bracket};${replB};g${nl}"
done

for f in i file emph kbd key abbr acronym email indicateurl
do
    sedcmd="${sedcmd}s;@${f}${bracket};${replI};g${nl}"
done

sed \
 -e "${sedcmd}" \
 -e 's;@pxref{\([^}]*\)};see: \1;g' \
 -e 's;@xref{\([^}]*\)};see: \1;g' \
 -e 's/@\([{}]\)/\1/g' \
 -e 's,^\$\*$,.br,' \
 -e '/@ *example/,/@ *end *example/s/^/    /' \
 -e 's/^ *@ *example/.nf/' \
 -e 's/^ *@ *end *example/.fi/' \
 -e  '/^ *@ *noindent/d' \
 -e  '/^ *@ *enumerate/d' \
 -e 's/^ *@ *end *enumerate/.br/' \
 -e  '/^ *@ *table/d' \
 -e 's/^ *@ *end *table/.br/' \
 -e 's/^@item \(.*\)/.sp\
.IR "\1"/' \
 -e 's/^@item/.sp 1/' \
 -e 's/\*\([a-zA-Z0-9:~=_ -]*\)\*/%BACKSLASH%fB\1%BACKSLASH%fP/g' \
 -e 's/``\([a-zA-Z0-9:~+=_ -]*\)'"''"'/\\(lq\1\\(rq/g' \
 -e "s/^'/\\'/" \
 -e 's/^@\*/.br/' \
 -e 's/^@sp/.sp/' \
 -e 's/ -/ \\-/g' \
 -e 's@^\.in \\-@.in -@' \
 -e 's#%BACKSLASH%#\\#g'
