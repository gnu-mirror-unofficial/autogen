#!/usr/bin/perl

## man2mdoc.pl -- Convert man tags to mdoc tags
##
## Author:	Harlan Stenn <stenn@ntp.org>
##
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

use strict;

my ($taglist , $optlistold, $paraold, $parafirstval,$List,
    $indentation,$isindentated);

my ($line);

$isindentated = 0;      #this variable is used check the indentation
$indentation = 0;       #this variable is used to set the indentation if given
$List = 0;
$parafirstval = 0;
$paraold = 0;
$taglist = 0;           ### 1 = taglist, 0 = other list types except taglist
$optlistold = 0;        ##  1 = previous list entry was there, 0 = not

while ($line = <STDIN>)
{
    if ($line !~ /^\./ ) {
        if ($line =~ /[\\fB|\\fI]/) {
            MakeMacro($line);
            print "\n";
            next;
        }

        print $line;
        next;
    }

    $line =~ s/^\.//;

    next
        if ($line =~ /\\"/);

    $line = ParseMacro($line);

    print($line)
        if (defined $line);
}

sub ParseMacro
{
    my ($line) = @_;

    my (@words,$retval);
    $retval = '';
    @words =  split(/\s+/,$line);

    while($_ = shift (@words)) {
        if (/^sp$/ || /^ne$/ || /^na$/|| /^rt$/|| /^mk$/|| /^ad$/) {
            last;
        }

        if (/^RS$/) {
            $List = 1;
            # this is to check whether that indentation value is given,
            # if it is not given tha means we have to use default indentation,
            # if it is given we need to check for that value
            #
            $isindentated = scalar(@words);

            if ($isindentated) {
                if ($_ = shift (@words))
                {
                    $indentation = 1;
                    last;
                }
                $indentation = 0;
                last;
            }
            $indentation = 1;
            last;
        }

        if (/^IP$/ && $List) {
            if (!$optlistold) {
                $optlistold = 1;
                $taglist = 1;

                if ($indentation) {
                    $retval .= ".Bl -tag -offset indent -compact\n";
                } else {
                    $retval .= ".Bl -tab -offset 0n -compact\n";
                }
                print $retval;
                $words[0] =~ s/\\fB/ Nm /;
                $words[0] =~ s/\\fI/ Ar /;
                $words[0] =~ s/\\fR/ /g;

                print ".It ".$words[0]."\n";
                last;

            }

            if ($optlistold) {
                $words[0] =~ s/\\fB/ Nm /;
                $words[0] =~ s/\\fI/ Ar /;
                $words[0] =~ s/\\fR/ /g;
                print ".It ".$words[0]."\n";
                last;
            }
        }

        if (/^TP$/ && $List) {
            if (!$optlistold)
            {
                $optlistold = 1;
                $taglist = 1;

                if ($indentation) {
                    $retval .= ".Bl -tag -offset indent -compact\n";
                } else {
                    $retval .= ".Bl -tab -offset 0n -compact\n";
                }
                print $retval;
                $retval = <DATA>;

                $retval =~ s/\\fB/ Nm /;
                $retval =~ s/\\fI/ Ar /;
                $retval =~ s/\\fR/ /g;

                print ".It ".$retval."\n";
                last;
            }

            if ($optlistold) {
                $retval = <DATA>;
                $retval =~ s/\\fB/ Nm /;
                $retval =~ s/\\fI/ Ar /;
                $retval =~ s/\\fR/ /g;
                print ".It ".$retval."\n";
                last;
            }
        }

        if (/^RE$/) {
            $indentation = 0;
            $optlistold = 0;
            $isindentated = 0;

            $optlistold = 0;

        }

        if (/^IP$/ && !$List)
        {
            if (!$optlistold && $words[0] =~ /^\\\(bu$/) {
                $optlistold = 1;
                $retval .= ".Bl -bullet"."\n";
                print $retval;
                print ".It \n";
                last;
            }


            if (!$optlistold && $words[0] =~ /^-$/) {
                $optlistold = 1;
                $retval .= ".Bl -dash \n";
                print $retval;
                print ".It \n";
                last;
            }


            if (!$optlistold && $words[0] =~ /^[1-9]\.$/) {
                $optlistold = 1;
                $retval .= ".Bl -enum \n";
                print $retval;
                print ".It \n";
                last;

            }

            if (!$optlistold && $words[0] !~ /[0-9|-|(br]/) {
                $optlistold = 1;
                $taglist = 1;
                $retval .= ".Bl -tag \n";
                print $retval;
                print ".It ".$words[0]."\n";
                last;
            }

            if (!$optlistold) {
                $optlistold = 1;
                $retval .= ".Bl -item \n";
                print $retval;
                print ".It \n";
                last;

            }

            if ($optlistold) {
                print ".It \n";
                last
            }
        }

        if ($optlistold && ! /^IP$/ ) {
            $optlistold = 0;
            print ".El \n";
        }

        if (/^TP$/) {
            $parafirstval = 1;

            if (!$paraold) {
                $retval .= ".Bl -tag \n";
                print $retval;
                print ".It ";
                $paraold = 1;
                last;
            }

            if ($paraold) {
                print ".It ";
                $paraold = 1;
                last;
            }
        }

        #text bolding (mdoc : .Nm ntpq) (man : .B ntpq )
        if (/^RS$/) {
            $List = 1;
        }

        if (/^B$/) {
            $retval .= ".Nm ".join(' ',@words)."\n";
            print $retval;
        }

        #text bolding ()
        if (/\\fB/) {
            $retval = $_;
            $retval =~  s/[\\fB|\\fP]//g;
            print ".Nm ".$retval."\n";
        }

        if (/\\fI/) {
            $retval = $_;
            $retval =~ s/[\\fI|\\fP]//g;
            print ".Em ".$retval."\n";
        }

        if (/^I$/) {
            $retval .= ".Em ".join(' ',@words)."\n";
            print $retval;
        }

        if (/^PP$/) {
            print "\n";
        }

        if (/^LP$/) {
            print "\n";
        }
    }
}

sub MakeMacro
{
    my (@words);
    @words =  split(/\s+/,$line);
    while($_ = shift (@words))
    {
        if (/\\fB/ or /\\fI/) {
            print "\n";
            $_ =~ s/\\fB/\.Nm /;
            $_ =~ s/\\fI/\.Ar /;
            $_ =~ s/\\fR//g;

            print $_;
            print"\n";
            next;
        }

        print $_." ";
    }
}
