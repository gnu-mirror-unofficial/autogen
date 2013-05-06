[= AutoGen5 template -*- Mode: texinfo -*-

texi

#  Documentation template
#
#  This file is part of AutoOpts, a companion to AutoGen.
#  AutoOpts is free software.
#  AutoOpts is Copyright (C) 1992-2013 by Bruce Korb - all rights reserved
#
#  AutoOpts is available under any one of two licenses.  The license
#  in use must be one of these two and the choice is under the control
#  of the user of the license.
#
#   The GNU Lesser General Public License, version 3 or later
#      See the files "COPYING.lgplv3" and "COPYING.gplv3"
#
#   The Modified Berkeley Software Distribution License
#      See the file "COPYING.mbsd"
#
#  These files have the following sha256 sums:
#
#  8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
#  4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
#  13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd

=][=

INVOKE initialization   =][=
INVOKE emit-description =]

This [=(string-downcase doc-level)=] was generated by @strong{AutoGen},
using the @code{agtexi-cmd} template and the option descriptions for the [=(.
coded-prog-name)=] program.[= (name-copyright) =]

@menu
[=
  (out-push-new) (out-suspend "menu")
  (out-push-new)        =][=

INVOKE emit-usage-opt   =][=

;;  FOR all options, ...
;;
(define opt-name       "")
(define extra-ct       0)
(define extra-text     "")
(define optname-from "A-Z_^")
(define optname-to   "a-z--")
(define invalid-doc   "* INVALID *")
(if (exist? "preserve-case") (begin
   (set! optname-from "_^")
   (set! optname-to   "--") ))
(if (and have-doc-options (not (exist? "flag[].documentation"))) (begin
    (ag-fprintf "menu" menu-entry-fmt
                "base-options:: " "Base options")
    (print-node opt-name "Base options")
)   )

=][=#

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

FOR flag                        =][=

  (set! opt-name (string-tr! (get "name") optname-from optname-to))
  (if (exist? "documentation")
      (begin
          (set! label-str (string-append opt-name " options"))
          (ag-fprintf "menu" menu-entry-fmt
              (string-append opt-name ":: ") label-str)
          (print-node opt-name label-str)
          (ag-fprintf 0 "\n%s." (get "descrip"))
          (set! tmp-str (get "documentation"))
          (if (> (string-length tmp-str) 1)
              (ag-fprintf 0 "\n%s" tmp-str))
      )
      (begin
        (set! tmp-str (get "doc" invalid-doc))
        (if (< 0 (string-length tmp-str)) (begin
          (set! label-str (string-append opt-name " option"
                (if (exist? "value")
                    (string-append " (-" (get "value") ")")
                    "" )  ))
          (if have-doc-options
            (ag-fprintf 0 opt-node-fmt opt-name label-str)
            (begin
              (ag-fprintf "menu" menu-entry-fmt
                  (string-append opt-name ":: ") label-str)
              (print-node opt-name label-str)
            )
          )
          (ag-fprintf 0 "\n@cindex %s-%s" down-prog-name opt-name)
        ) )
      )
  )                             =][=

  IF (and (not (exist? "documentation"))
          (< 0 (string-length tmp-str)) )
    =][=
    IF (exist? "aliases")       =][=
      INVOKE emit-aliases       =][=
    ELSE                        =][=
      INVOKE emit-opt-text      =][=
    ENDIF                       =][=
  ENDIF                         =][=

ENDFOR flag                     =][=

IF
   (define home-rc-files (> (count "homerc") 0))
   (if (and (not home-rc-files) (exist? "homerc"))
       (set! home-rc-files (> (string-length (get "homerc")) 0)) )
   (define environ-init  (exist? "environrc"))
   (or home-rc-files environ-init)
   =][=

   INVOKE emit-presets          =][=

ENDIF                           =][=

INVOKE emit-exit-status         =][=
INVOKE emit-doc-sections        =][=

(out-suspend "opt-desc")
(out-resume "menu")
(emit (out-pop #t))
(emit "@end menu\n")
(out-resume "opt-desc")
(emit (out-pop #t))
(define post-proc-cmd (get "doc-sub-cmd" "sed -f %s %s"))

(if do-post-proc (begin
    (out-pop)
    (shellf post-proc-cmd post-proc-file raw-doc-file)
)   )

=][=#

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-doc-sections        =][=

FOR doc-section                 =][=

  IF  (define opt-name (string-capitalize! (get "ds-type")))
      (or (== opt-name "Exit Status")
          (== opt-name "Description")
          (exist? "omit-texi")) =][=
    CONTINUE                    =][=
  ENDIF                         =][=

  (define section-file (string-append tmp-dir "/"
                       (string-substitute opt-name " " "-")))
  (if (not (access? section-file R_OK)) (begin
      (ag-fprintf "menu" menu-entry-fmt (string-append opt-name "::") opt-name)
      (set! label-str (string-append
            down-prog-name " " (string-capitalize opt-name)))
      (out-push-new section-file)
      (print-node opt-name label-str)
    )   (begin
      (out-push-add section-file)
      (emit "\n")
  ) )

  (define cvt-fn (get "ds-format" "texi"))
  (if (not (== cvt-fn "texi"))
      (divert-convert cvt-fn) ) =][=
  (emit (string-append "\n" (get "ds-text") "\n"))
  (convert-divert)
  (out-pop)

=][=
ENDFOR  doc-section             =][=

FOR doc-section                 =][=
  IF  (define opt-name (string-capitalize! (get "ds-type")))
      (define section-file (string-append tmp-dir "/"
                           (string-substitute opt-name " " "-")))
      (access? section-file R_OK)
    =][=
    (shellf "cat %1$s ; rm -f %1$s" section-file)
    =][=
  ENDIF accessible section file =][=
ENDFOR  doc-section             =][=

ENDDEF emit-doc-sections

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-description         =][=

(if (exist? "explain")
    (emit (string-append "\n" (get "explain") "\n")) )
(set! tmp-str (get "option-format" "texi"))
(divert-convert tmp-str)

=][=

IF (match-value? == "doc-section.ds-type" "DESCRIPTION") =][=

  FOR doc-section   =][=
    IF (== (get "ds-type") "DESCRIPTION") =][=
       (define cvt-fn (get "ds-format" "texi"))
       (if (not (== cvt-fn "texi"))
           (divert-convert cvt-fn) )
       (emit (string-append "\n" (get "ds-text") "\n"))
       =][=
       BREAK        =][=

    ENDIF           =][=
  ENDFOR            =][=

ELSE                =][=

(join "\n\n"
    (if (exist? "prog-info-descrip")
        (stack  "prog-info-descrip")
        (if (exist? "prog-man-descrip")
            (stack  "prog-man-descrip")
            (if (exist? "prog-descrip")
                (stack  "prog-descrip")
                (stack  "detail")
)   )   )   )       =][=

ENDIF               =][=

(convert-divert)    =][=

ENDDEF emit-description

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-exit-status         =][=
 (ag-fprintf "menu" menu-entry-fmt "exit status::" "exit status")
 (print-node "exit status" (string-append program-name " exit status")) =]

One of the following exit values will be returned:
@table @samp
@item 0 (EXIT_[=
  (set! tmp-str (get "exit-name[0]" "SUCCESS"))
  (string-upcase (string->c-name! tmp-str))
  =])
[=
  (define need-ex-noinput  (exist? "homerc"))
  (define need-ex-software #t)
  (get "exit-desc[0]" "Successful program execution.")=]
@item 1 (EXIT_[=

  (set! tmp-str (get "exit-name[1]" "FAILURE"))
  (string-upcase (string->c-name! tmp-str))=])
[= (get "exit-desc[1]"
        "The operation failed or the command syntax was not valid.") =][=

FOR exit-desc (for-from 2)   =][=
  (if (= (for-index) 66)
      (set! need-ex-noinput  #f)
      (if (= (for-index) 70)
          (set! need-ex-software #f) ))
  (set! tmp-str (get (sprintf "exit-name[%d]" (for-index)) "* unnamed *"))
  (sprintf "\n@item %d (EXIT_%s)\n%s" (for-index)
    (string-upcase (string->c-name! tmp-str))
    (get (sprintf "exit-desc[%d]" (for-index))))
  =][=
ENDFOR exit-desc                        =][=

(if need-ex-noinput
    (emit "\n@item 66 (EX_NOINPUT)
A specified configuration file could not be loaded."))

(if need-ex-noinput
    (emit "\n@item 70 (EX_SOFTWARE)
libopts had an internal operational error.  Please report
it to autogen-users@@lists.sourceforge.net.  Thank you."))
=]
@end table[=

ENDDEF emit-exit-status

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-aliases             =]

This is an alias for the @code{[= aliases =]} option,
[= (sprintf "@pxref{%1$s %2$s, the %2$s option documentation}.\n"
      down-prog-name (get "aliases")) =][=

ENDDEF emit-aliases

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-opt-text            =]

This is the ``[=(string-downcase! (get "descrip"))=]'' option.[=
    IF (exist? "arg-type")     =]
This option takes an [= (if (exist? "arg-optional") "optional " "")
 =]argument [= arg-type =][=
(if (exist? "arg-name") (string-append " @file{"
    (string-substitute (get "arg-name") "@" "@@") "}"))
 =].[=
    ENDIF           =][=

    (set! extra-ct 0)
    (out-push-new)  =][=

    IF (exist? "min") =]@item
is required to appear on the command line.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "max") =]@item
may appear [=
      IF % max (== "%s" "NOLIMIT")
         =]an unlimited number of times[=
      ELSE
         =]up to [=max=] times[=
      ENDIF=].
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "enabled") =]@item
is enabled by default.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "ifdef") =]@item
must be compiled in by defining @code{[=(get "ifdef")
      =]} during the compilation.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF =][=

    IF (exist? "ifndef") =]@item
must be compiled in by @strong{un}-defining @code{[=(get "ifndef")
      =]} during the compilation.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "no_preset") =]@item
may not be preset with environment variables or configuration (rc/ini) files.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "equivalence") =]@item
is a member of the [=equivalence=] class of options.
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "flags_must") =]@item
must appear in combination with the following options:
[=    FOR flags_must ", " =][=flags_must=][=
      ENDFOR=].
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF (exist? "flags_cant") =]@item
must not appear in combination with any of the following options:
[=    FOR flags_cant ", " =][=flags_cant=][=
      ENDFOR=].
[=    (set! extra-ct (+ extra-ct 1)) =][=
    ENDIF=][=

    IF  (~* (get "arg-type") "key|set") =]@item
This option takes a keyword as its argument[=

      CASE arg-type   =][=
      =* key          =][= (set! extra-ct (+ extra-ct 1)) =].
The argument sets an enumeration value that can be tested by comparing[=

      =* set          =][= (set! extra-ct (+ extra-ct 1)) =] list.
Each entry turns on or off membership bits.  These bits can be tested
with bit tests against[=
      ESAC arg-type   =] the option value macro ([=
(string-upcase (string-append
(if (exist? "prefix") (string-append (get "prefix") "_") "")
"OPT_VALUE_" (get "name")  )) =]).
The available keywords are:
@example
[= (shell (string-append
   "${CLexe:-columns} -I4 --spread=1 -W50 <<\\" heredoc-marker
   (join "\n" (stack "keyword") "\n")
   heredoc-marker
   )  ) =]
@end example
[=

      IF (=* (get "arg-type") "key") =]
or their numeric equivalent.[=
      ENDIF =][=

    ENDIF key/set arg =][=

    IF (> extra-ct 0) =][=
      (set! extra-text (out-pop #t)) =]

@noindent
This option has some usage constraints.  It:
@itemize @bullet
[=(. extra-text)
=]@end itemize
[=  ELSE  =][=
      (out-pop) =][=
    ENDIF =][=

?% doc "\n%s" "\nThis option has no @samp{doc} documentation." =][=
  IF (exist? "deprecated") =]

@strong{NOTE}@strong{: THIS OPTION IS DEPRECATED}[=

  ENDIF     =][=

ENDDEF emit-opt-text

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE set-home-rc-vars          =][=
  CASE homerc                    =][=
  ==*  '$@'                      =][=
       (set! explain-pkgdatadir #t)
       (set! cfg-file-name (string-substitute (get "homerc")
          "$@" "$(pkgdatadir)")) =][=

  ==   '.'                       =][=
       (set! cfg-file-name "$PWD")
       (set! env-var-list (string-append env-var-list "PWD, "))
       =][=

  ==*  './'                      =][=
       (set! explain-pkgdatadir #t)
       (set! env-var-list  (string-append env-var-list "PWD, "))
       (set! cfg-file-name (string-append "$PWD" (substring (get "homerc") 1)))
       =][=

  ~~*  '\$[A-Za-z]'              =][=
       (set! cfg-file-name (get "homerc"))
       (set! env-var-list (string-append env-var-list
             (shellf "echo '%s' | sed 's/^.//;s#/.*##'" cfg-file-name)
             ", " ))
       =][=

  == "" =][= (set! cfg-file-name "") =][=

  *                              =][=
       (set! cfg-file-name (get "homerc"))  =][=
  ESAC                           =][=

ENDDEF set-home-rc-vars

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-multiple-rc         \=]
[=
  (define explain-pkgdatadir #f)
  (define env-var-list       "")
  rc-count =] places for configuration files:
@itemize @bullet[=
FOR homerc                       =][=
  INVOKE set-home-rc-vars        =][=
  (if (> (string-length cfg-file-name) 0)
      (sprintf "\n@item\n%s"  cfg-file-name ))
  =][=

ENDFOR homerc                    =]
@end itemize[=
 (if explain-pkgdatadir (ag-fprintf 0
"\nThe value for @code{$(pkgdatadir)} is recorded at package configure time
and replaced by @file{libopts} when @file{%s} runs." program-name))

(if (> (string-length env-var-list) 1)
    (shell (string-append
"list='@code{'`echo '" env-var-list "' | \
  sed -e 's#, $##' \
      -e 's#, #}, @code{#g' \
      -e 's#, \\([^ ][^ ]*\\)$#, and \\1#'`\\}
echo
echo 'The environment variables' ${list}
echo 'are expanded and replaced when @file{" program-name "} runs.'"
))  ) =]
For any of these that are plain files, they are simply processed.
For any that are directories, then a file named @file{[=
 (if (exist? "rcfile") (get "rcfile")
     (string-append "." program-name "rc"))=]} is searched for
within that directory and processed.
[=

ENDDEF emit-multiple-rc

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-one-rc-dir              =][=
  (define env-var-list       "")
  (define explain-pkgdatadir #f)    =][=
  INVOKE set-home-rc-vars

=]@file{[=(. cfg-file-name) =]} for configuration (option) data.[=
  IF (. explain-pkgdatadir)         =]
The value for @code{$(pkgdatadir)} is recorded at package configure time
and replaced by @file{libopts} when @file{[=prog-name=]} runs.
[=ENDIF=][=
(if (> (string-length env-var-list) 1)
    (sprintf
"\nThe environment variable @code{%s} is expanded and replaced when
the program runs" env-var-list)) =]
If this is a plain file, it is simply processed.
If it is a directory, then a file named @file{[=
(if (exist? "rcfile") (get "rcfile")
     (string-append "." program-name "rc"))
=]} is searched for within that directory.[=

ENDDEF emit-one-rc-dir

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-rc-file-info                =]

@noindent
@code{libopts} will search in [=

    IF (define rc-count (count "homerc"))
       (define cfg-file-name "")
       (> rc-count 1)           =][=

       INVOKE emit-multiple-rc  =][=

    ELSE                        =][=
       INVOKE emit-one-rc-dir   =][=
    ENDIF (> rc-count 1)

=]

Configuration files may be in a wide variety of formats.
The basic format is an option name followed by a value (argument) on the
same line.  Values may be separated from the option name with a colon,
equal sign or simply white space.  Values may be continued across multiple
lines by escaping the newline with a backslash.

Multiple programs may also share the same initialization file.
Common options are collected at the top, followed by program specific
segments.  The segments are separated by lines like:
@example
[[=(. UP-PROG-NAME)=]]
@end example
@noindent
or by
@example
<?program [= prog-name =]>
@end example
@noindent
Do not mix these styles within one configuration file.

Compound values and carefully constructed string values may also be
specified using XML syntax:
@example
<option-name>
   <sub-opt>...&lt;...&gt;...</sub-opt>
</option-name>
@end example
@noindent
yielding an @code{option-name.sub-opt} string value of
@example
"...<...>..."
@end example
@code{AutoOpts} does not track suboptions.  You simply note that it is a
hierarchicly valued option.  @code{AutoOpts} does provide a means for searching
the associated name/value pair list (see: optionFindValue).[=

ENDDEF emit-rc-file-info

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-presets                     =]

[=
  (ag-fprintf "menu" menu-entry-fmt "config::"
           (string-append "presetting/configuring " down-prog-name) )

 (print-node "config"
      (string-append "presetting/configuring " program-name) ) =]

Any option that is not marked as @i{not presettable} may be preset by
loading values from [=

IF

   (if home-rc-files (emit
       "configuration (\"rc\" or \"ini\") files"))

   environ-init

  =][=
  (if home-rc-files (emit ", and values from "))
  =]environment variables named @code{[=(. UP-PROG-NAME)=]} and @code{[=
(. UP-PROG-NAME)=]_<OPTION_NAME>}.  @code{<OPTION_NAME>} must be one of
the options listed above in upper case and segmented with underscores.
The @code{[=(. UP-PROG-NAME)=]} variable will be tokenized and parsed like
the command line.  The remaining variables are tested for existence and their
values are treated like option arguments[=
  ENDIF  have environment inits         =].
[=

  IF (. home-rc-files)                  =][=
     INVOKE emit-rc-file-info           =][=
  ENDIF home-rc-files                   =]

The command line options relating to configuration and/or usage help are:
[=

IF (exist? "version")                   =]
@[= (. head-level) =] version[= (flag-string "version-value" "v") =]

Print the program version to standard out, optionally with licensing
information, then exit 0.  The optional argument specifies how much licensing
detail to provide.  The default is to print [=
(if (exist? "gnu-usage")
    "the license name with the version"
    "just the version")
=].  The licensing infomation may be selected with an option argument.
Only the first letter of the argument is examined:

@table @samp
@item version
Only print the version.[=
(if (not (exist? "gnu-usage")) "  This is the default.")=]
@item copyright
Name the copyright usage licensing terms.[=
(if (exist? "gnu-usage") "  This is the default.")=]
@item verbose
Print the full copyright usage licensing terms.
@end table
[=
ENDIF version                           =][=

IF (exist? "usage-opt")                 =]
@[= (. head-level) =] usage[= (flag-string "usage-value" "u") =]

Print abbreviated usage to standard out, then exit 0.
[=
ENDIF usage-opt                         =][=

IF (exist? "vendor-opt")                =]
@[= (. head-level) =] vendor-option (-W)

Options that do not have flag values specified must be specified with
@code{-W} and the long option name.  That long name is the argument to
this option.  Any options so named that require an argument must have
that argument attached to the option name either with quoting or an
equal sign.
[=
ENDIF vendor-opt                        =][=

IF (exist? "resettable")                =]
@[= (. head-level) =] reset-option[= (flag-string "reset-value" "R") =]

Resets the specified option to the compiled-in initial state.
This will undo anything that may have been set by configuration files.
The option argument may be either the option flag character or its long name.
[=
ENDIF resettable                        =][=

IF (exist? "home-rc")                   =][=
  IF (exist? "disable-save")            =]
@[= (. head-level) =] save-opts[= (flag-string "save-opts-value" ">") =]

Saves the final, configured option state to the specified file (the optional
option argument).  If no file is specified, then it will be saved to the
highest priority (last named) @file{rc-file} in the search list.
The command will exit after updating this file.
[=
  ENDIF disable-save                    =][=

  IF (exist? "disable-load")            =]
@[= (. head-level) =] load-opts[= (flag-string "load-opts-value" "<") =]

Loads the named configuration file.
[=
  ENDIF disable-load                    =][=
ENDIF home-rc                           =][=

ENDDEF emit-presets

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE header                          \=]
\input texinfo
@c -*-texinfo-*-
@c %**start of header
@setfilename [= (string-append down-prog-name ".info") =]
@settitle [= (sprintf (if (exist? "package") "%2$s - %1$s" "%s")
             (get "package")  (get "prog-title")) =]
@c %**end of header
@setchapternewpage off
@titlepage
@sp 10
@comment The title is printed in a large font.
@center @titlefont{Sample Title}

@c The following two commands start the copyright page.
@page
@vskip 0pt plus 1filll
[= (name-copyright) =][=
IF (exist? "copyright.type") =]
[= (license-full (get "copyright.type") program-name ""
    (get "copyright.owner" (get "copyright.author" ""))
    (get "copyright.date") ) =][=
ENDIF =]
@end titlepage
@node Top, [= prog-name =] usage, , (dir)
@top [= prog-title =]
[=

ENDDEF header

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE emit-usage-opt                   =][=

  (define label-str (string-append
          program-name " help/usage (@option{" help-opt "})"))
  (ag-fprintf "menu" menu-entry-fmt "usage::" label-str)
  (sprintf node-fmt "usage" label-str) =]
@cindex [=(. down-prog-name)=] help

This is the automatically generated usage text for [= prog-name =].

The text printed is the same whether selected with the @code{help} option
(@option{[= (. help-opt) =]}) or the @code{more-help} option (@option{[=
(. more-help-opt) =]}).  @code{more-help} will print
the usage text by passing it through a pager program.
@code{more-help} is disabled on platforms without a working
@code{fork(2)} function.  The @code{PAGER} environment variable is
used to select the program, defaulting to @file{more}.  Both will exit
with a status code of 0.

@exampleindent 0
@example
[= (out-push-new) =]
prog_name=[= (. program-name) =]
PROG=./${prog_name}
test -f ${PROG} || {
  PROG=`echo $PROG | tr '[A-Z]' '[a-z]'`
  test -f ${PROG} || PROG=`echo $PROG | tr x_ x-`
}
if [ ! -f ${PROG} ]
then
  if [= (string-append program-name " " help-opt) =] > /dev/null 2>&1
  then PROG=`command -v ${prog_name}`
  else PROG="echo ${prog_name} is unavailable - no "
  fi
fi
${PROG} [=(. help-opt)=] 2>&1 | \
    sed -e "s/Usage:  lt-${prog_name} /Usage:  ${prog_name} /" \
        -e 's/@/@@/g;s/{/@{/g;s/}/@}/g' \
        -e 's/	/        /g'
[= (shell (out-pop #t))         =]
@end example
@exampleindent 4
[=

ENDDEF emit-usage-opt

@c = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =][=

DEFINE initialization                   =][=

  ;;# START-BUILDTREE-ISMS
  ;;
  (shell "CLexe=${AGexe%/agen5/*}/columns/columns
  test -x \"${CLexe}\" || {
    CLexe=${AGexe%/autogen}/columns
    test -x \"${CLexe}\" || die 'columns program is not findable'
  }")

=][= # END-BUILDTREE-ISMS

  (shell "CLexe=${AGexe%/autogen}/columns")

# END-INSTALL-ONLY-CODE =][=

  ;; divert-convert  divert text for conversion to .texi format
  ;; convert-divert  convert the diversion done with divert-convert
  ;;
  (define divert-convert (lambda (diversion-type) (begin
     (set! was-diverted
           (not (or (== diversion-type "texi") (== diversion-type ""))))
     (if was-diverted   (begin
         (set! cvt-script
               (find-file (string-append diversion-type "2texi")))
         (if (not (defined? 'cvt-script))
             (error (sprintf "unknown source format type: %s" diversion-type)) )
         (out-push-new) ))  )))

  (define heredoc-marker "_Unlikely_Here_Doc_Marker_\n")
  (define convert-divert (lambda ()
     (if was-diverted (shell (string-append
         cvt-script "<<\\" heredoc-marker (out-pop #t) "\n" heredoc-marker
  )) )))

  (define was-diverted   #f)
  (define diversion-type "")
  (define cvt-script     "")
  (define tmp-str        "")

  (define name-copyright (lambda ()
      (if (exist? "copyright")
          (string-append "\nThis software is released under "
             (license-name (get "copyright.type" "an unknown copyright"))
             "." ) ) ))

  (make-tmp-dir)
  (define program-name      (get "prog-name"))
  (define down-prog-name    (string-downcase program-name))
  (define UP-PROG-NAME      (string-upcase   program-name))
  (shellf "export AG_DEF_PROG_NAME=%s" program-name)
  (define doc-level         (getenv "LEVEL"))
  (if (not (string? doc-level))
      (set! doc-level "section"))
  (define file-name         (string-append down-prog-name ".texi"))
  (define coded-prog-name   (string-append "@code{" down-prog-name "}"))

  (define replace-prog-name (lambda (nm)
     (string-substitute (get nm) down-prog-name coded-prog-name )  ))

  (define have-doc-options  (exist? "flag.documentation"))
  (define print-menu        #t)
  (define do-doc-nodes      #f)
  (define menu-entry-fmt    (string-append
                            "* " down-prog-name " %-24s %s\n"))
  (define emit-menu-entry   (lambda (is-doc) (not is-doc)))
  (if have-doc-options
      (set! emit-menu-entry (lambda (is-doc) is-doc))  )
  (define chk-flag-val      (exist? "flag.value"))
  (define flag-string       (lambda (v-nm v-df) (if (not chk-flag-val) ""
     (string-append " (-"
        (if (exist? v-nm) (get v-nm) v-df)
        ")")  )))

  (define help-opt "")
  (if (exist? "long-opts")
      (set! help-opt "--help")
  (if (not (exist? "flag.value"))
      (set! help-opt "help")
  (if (not (exist? "help-value"))
      (set! help-opt "-?")
      (begin
         (set! tmp-str (get "help-value"))
         (if (> (string-length tmp-str) 0)
             (set! help-opt (string-append "-" tmp-str))
             (set! help-opt "--help")
      )  )
  )))

  (define more-help-opt "")
  (if (exist? "long-opts")
      (set! more-help-opt "--more-help")
  (if (not (exist? "flag.value"))
      (set! more-help-opt "more-help")
  (if (not (exist? "more-help-value"))
      (set! more-help-opt "-!")
      (begin
         (set! tmp-str (get "more-help-value"))
         (if (> (string-length tmp-str) 0)
             (set! help-opt (string-append "-" tmp-str))
             (set! help-opt "--more-help")
      )  )
  )))

  =][=

  CASE (. doc-level)    =][=
    == document         =][= INVOKE header =][=
       (define sub-level  "chapter")
       (define head-level "heading")       =][=
    == chapter          =][=
       (define sub-level  "section")
       (define head-level "subheading")    =][=
    == section          =][=
       (define sub-level "subsection")
       (define head-level "subsubheading") =][=
    == subsection       =][=
       (define sub-level "subsubsection")
       (define head-level "subsubheading") =][=

    * =][=(error (sprintf "invalid doc level: %s\n" doc-level)) =][=

  ESAC doc level        =][=

  (define node-fmt (string-append
     "\n@node " down-prog-name " %s\n@" sub-level " %s"))
  (define print-node        (lambda (a b) (ag-fprintf 0 node-fmt a b) ))

  (define opt-node-fmt (if have-doc-options
     (string-append "\n@" head-level
        " %2$s.\n@anchor{" down-prog-name " %1$s}")
     node-fmt
  ))

  (define exit-sts-fmt "\n\n@node %1$s %2$s\n@%3$s %1$s %2$s\n")
  =][=

  IF (not (== doc-level "document"))    =][=
     (set! file-name (string-append "invoke-" file-name))
       \=]
@node [= prog-name      =] Invocation
@[=(. doc-level)        =] Invoking [= prog-name =]
@pindex [= prog-name    =]
@cindex [= prog-title   =][=

FOR concept =]
@cindex [= concept      =][=
ENDFOR                  =][=

  ENDIF document component

=]
@ignore
[=

(out-move file-name)
(out-push-new (string-substitute (out-name) ".texi" ".menu"))

(ag-fprintf 0 "* %-32s Invoking %s\n"
    (string-append program-name " Invocation::")
    program-name )

(out-pop)
(dne "# " "# ")

=]
@end ignore
[=
(define rep-string      "")
(define do-post-proc    #f)
(define post-proc-file  "")
(define raw-doc-file    "")
(define post-proc-commands "")  =][=

FOR doc-sub                     =][=
  (define field-name (get "sub-type" "texi"))
  (if (~~ "texi" field-name) (begin
      (set! do-post-proc #t)
      (set! field-name (get "sub-name"))
      (set! rep-string (string-append "<<" field-name ">>"))
      (set! post-proc-commands (string-append post-proc-commands
            (string-substitute (get "sub-text") rep-string (get field-name))
            "\n" ))
  )   )                         =][=

ENDFOR  doc-sub                 =][=

(if (> (string-length post-proc-commands) 1) (begin
    (set! post-proc-file (string-append tmp-dir "/post-proc-cmds"))
    (out-push-new post-proc-file)
    (emit post-proc-commands)
    (out-pop)
    (set! raw-doc-file (string-append tmp-dir "/raw-doc"))
    (out-push-new raw-doc-file)
)   )

=][=

ENDDEF initialization

@c agtexi-cmd.tpl ends here =]
