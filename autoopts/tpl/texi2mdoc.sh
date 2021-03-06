#! /bin/sh

## texi2mdoc.sh -- script to convert texi-isms to mdoc-isms
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

# /bin/sh on Solaris is too horrible for words
#
case "$0" in
/bin/sh ) test -x /usr/xpg4/bin/sh && exec /usr/xpg4/bin/sh ${1+"$@"} ;;
esac

parent_pid=$$
prog=`basename $0 .sh`

die() {
    echo "$prog error:  $*" >&2
    kill -TERM $parent_pid
    sleep 1
    kill -9 $parent_pid
    sleep 1
    exit 1
}

do_example() {
    echo '.Bd -literal -offset indent'
    res=0

    while :
    do
        IFS='' read -r line || die "incomplete example"
        case "$line" in
        '@end '*example ) break ;;
        esac

        do_line
    done
    echo '.Ed'
    return $res
}

do_noindent() {
    return 0
}

do_enumerate() {
    echo '.Bl -enum -compact'

    while :
    do
        IFS='' read -r line || die "incomplete enumerate"
        case "$line" in
        '@end '*enumerate ) break ;;
        esac

        do_line
    done
    echo '.El'

    return $res
}

do_end() {
    die "Improper ending:  $line"
}

do_table() {
    echo '.Bl -tag -width 8n'

    while :
    do
        IFS='' read -r line || die "incomplete table"
        case "$line" in
        '@end '*table ) break ;;
        esac

        do_line
    done
    echo '.El'

    return $res
}

do_itemize() {
    echo '.Bl -bullet -compact'

    while :
    do
        IFS='' read -r line || die "incomplete itemize"
        case "$line" in
        '@end '*itemize ) break ;;
        esac

        do_line
    done
    echo '.El'

    return $res
}

do_item() {
    printf '%s\n' "$line" | sed 's/@item/.It/'
}

do_line() {
    case "${line}" in
    '@subheading'* ) printf '%s\n' "$line" | sed 's/@subheading/.SS /' ;;
    '@*' ) echo .br ;;
    '@sp') echo echo "${line}" | sed 's/@sp/.sp/' ;;
    ''   ) echo .sp ;;
    '@'[{}]* ) printf '%s\n' "${line}" | sed 's/@\([{}]\)/\1/g' ;;
    '@'* )
        typ=`printf '%s\n' "$line" | egrep '@[a-z]*\{'`
        test ${#typ} -gt 0 && printf '%s\n' "$line" && return 0
        typ=`printf '%s\n' "$line" | sed 's/@ *//;s/[^a-z].*//'`
        eval do_${typ} || die "do_${typ} failed"
        ;;

    * )
        printf '%s\n' "$line"
        ;;
    esac
    return 0
}


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

fixfont="${sedcmd}"'
	s;@pxref{\([^}]*\)};see: \1;g
	s;@xref{\([^}]*\)};see: \1;g
	s/@\([{@}]\)/\1/g
	s,^[@$]\*$,.br,
	s/\*\([a-zA-Z0-9:~=_ -]*\)\*/\\fB\1\\fP/g
	s/``\([a-zA-Z0-9:~+=_ -]*\)'\'\''/\\(lq\1\\(rq/g
	s/\([^\\]\)-/\1\\-/g
	s/\([^\\]\)-/\1\\-/g
	/^\.Bl /s/ \\-/ -/g
	/^\.Bd /s/ \\-/ -/g
	/^\.in /s/ \\-/ -/g
	s#%BACKSLASH%#\\#g'"
	s/^'/\\\\'/
	/^\$/d"
readonly fixfont

{
    while IFS='' read -r line
    do
        do_line
    done
} | sed "${fixfont}"

exit 0
