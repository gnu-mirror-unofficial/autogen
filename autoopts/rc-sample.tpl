[= AutoGen5 Template rc

# Time-stamp:      "2007-07-04 10:16:01 bkorb"

##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is copyright (c) 1992-2007 by Bruce Korb - all rights reserved
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
##  These files have the following md5sums:
##
##  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
##  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd

=]
# [= prog-name =] sample configuration file
#[=

IF (if (not (exist? "homerc"))
       (error "RC file samples only work for rc-optioned programs")  )
   (out-move (string-append "sample-"
                (if (exist? "rcfile") (get "rcfile")
                    (string-append (get "prog-name") "rc")  )
   )         )
   (set-writable)
   (define tmp-txt "")

   (exist? "copyright")
=] [=(sprintf "%s copyright %s %s - all rights reserved"
     (get "prog-name") (get "copyright.date") (get "copyright.owner") ) =][=

  CASE (get "copyright.type") =][=

    =  gpl      =]
#
[=(gpl  (get "prog-name") "# " ) =][=

    = lgpl      =]
#
[=(lgpl (get "prog-name") (get "copyright.owner") "# " ) =][=

    =  bsd      =]
#
[=(bsd  (get "prog-name") (get "copyright.owner") "# " ) =][=

    = note      =]
#
[=(prefix "# " (get "copyright.text"))  =][=

    *           =]
# <<indeterminate license type>>[=

  ESAC =][=

ENDIF "copyright exists"                =][=

FOR flag                                =][=

  IF (not (or (exist? "documentation") (exist? "no-preset")))     =]

# [= name =] -- [= descrip =]
#
[= INVOKE emit-description =]
# Example:
#
#[= name =][=
    IF (exist? "arg-type")
  =]	[= (if (exist? "arg-default") (get "arg-default")
           (if (exist? "arg-name")    (get "arg-name")
               (get "arg-type")  ))     =][=
    ENDIF (exist? "arg-type")           =][=

  ENDIF (not (exist? "documentation"))  =][=

ENDFOR flag

= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

     =][=

DEFINE emit-description =][=
(out-push-new)          =][=

  IF (exist? "min")
=]This option is required to appear.  [=ENDIF=][=

  IF (exist? "max")
=]This option may appear [=
     IF % max (= "%s" "NOLIMIT")
     =]an unlimited number of times[=ELSE
     =]up to [=max=] times[=
     ENDIF=].  [=
  ENDIF=][=

  IF (exist? "enabled")
=]This option is enabled by default.  [=ENDIF=][=

  IF (exist? "no-preset")
=]This option may not be preset with environment variables[= #
=] or in initialization (rc) files.  [=ENDIF=][=

  IF (exist? "default")
=]This option is the default option.  [=ENDIF=][=

  IF (exist? "equivalence")
=]This option is a member of the [=equivalence=] class of options.  [=ENDIF=][=

  IF (exist? "flags-must")
=]This option must appear in combination with the following options:  [=
    FOR flags-must ", " =][=flags-must=][=ENDFOR=].  [=
  ENDIF=][=

  IF (exist? "flags-cant")
=] This option must not appear in combination with any of the following [=#
=]options:  [=
    FOR flags-cant ", " =][=flags-cant=][=ENDFOR=].  [=
  ENDIF     =][=

  IF (~* (get "arg-type") "key|set")
=]This option takes a keyword as its argument[=

         IF (=* (get "arg-type") "set")

=] list.  Each entry turns on or off membership bits.  The bits are set by [=#
=]name or numeric value and cleared by preceding the name or number with an [=#
=]exclamation character ('!').  They can all be cleared with the magic name [=#
=]"none" and they can all be set with "all".  A single option will process [=#
=]a list of these values[=

         ELSE

=].  The argument sets an enumeration value that can [=#
=]be tested by comparing them against the option value macro[=

         ENDIF

=].  The available keywords are:  [=
            (join ", " (stack "keyword")) =].  [=
      ENDIF     =][=

      IF (exist? "arg-default")
=]The default [=(if (exist? "arg-name") (get "arg-name") (get "arg-type"))
=] for this option is:  [= arg-default =].  [=
      ENDIF     =][=

  (set! tmp-txt (out-pop #t))
  (if (> (string-length tmp-txt) 1)
      (string-append

            (shell (string-append "while read line
            do echo ${line} | fold -s -w76 | sed 's/^/# /'
               echo '#'
            done <<'__EndOfText__'\n" tmp-txt "\n__EndOfText__" ))

            "\n#\n")
      "" ) =][=

  IF (exist? "doc") =][= (prefix "# " (get "doc")) =][=
  ELSE =]# This option has not been fully documented.[=
  ENDIF =][=

ENDDEF emit-description

=]
