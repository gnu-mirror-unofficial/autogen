[= AutoGen5 Template -*- Mode: html -*-

html

# Time-stamp: "2010-07-16 15:21:34 bkorb"
# Version:    "$Revision: 4.13 $
##
##  This file is part of AutoGen.
##  AutoGen Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
##
##  AutoGen is free software: you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by the
##  Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  AutoGen is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##  See the GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with this program.  If not, see <http://www.gnu.org/licenses/>.
=]
<!--#include virtual="/server/header.html" -->
[=(dne "  ==  " "<!-- ")=]

  ***  THEREFORE  *** if you make changes to this file, please
  email the author so it will not be overwritten  :-) "

  -->
<title>[= title =] - GNU Project - Free Software Foundation (FSF)</title>
<meta http-equiv="content-type" content='text/html; charset=utf-8' />
<meta name="generator" content="AutoGen [=(. autogen-version)=]"/>
<!--#include virtual="/server/banner.html" -->
<h3>[= project =] version [= version =] - Table of Contents</h3>

<!-- This document is in XML, and xhtml 1.0 -->
<!-- Please make sure to properly nest your tags -->
<!-- and ensure that your final document validates -->
<!-- consistent with W3C xhtml 1.0 and CSS standards -->
<!-- See validator.w3.org -->

<address>Free Software Foundation</address>
<address>last updated [=`date '+%B %e, %Y'`=]</address>

<p>The manual for the <a href="/software/[=
   (define proj-name (string-downcase! (get "project")))
   (define package   (string-downcase! (get "package")))
   package
   =]">[= project =] project</a> is available in the following formats:</p>[=

(define fnam "")
(define fsiz 0)

(define (compute-size dir sfx)
  (begin
     (set! fnam (string-append dir "/" package sfx))
     (set! fsiz (if (access? fnam R_OK)  (stat:size (stat fnam)) 0 ))
     (shellf "fsiz='%d'
         if test ${fsiz} -lt 4096
         then echo ${fsiz}
         else
           fsiz=`expr ${fsiz} / 1024`
           if test ${fsiz} -lt 2048
           then echo ${fsiz}K
           else
             fsiz=`expr ${fsiz} / 1024`
             echo ${fsiz}M
           fi
         fi" fsiz)
) )

=]
<ul>
  <li>formatted in <a href="html_mono/[= (. package) =].html">HTML ([=
      (compute-size "html_mono" ".html")
      =] characters)</a> entirely on one web page.
  <li>formatted in <a href="html_mono/[= (. package) =].html.gz">HTML ([=
      (compute-size "html_mono" ".html.gz")
      =] gzipped bytes)</a> entirely on one web page.
  <li> formatted in <a href="html_chapter/[= (. package) =]_toc.html">HTML</a>
       with one web page per chapter.
  <li> formatted in <a href="html_chapter/[= (. package)
       =]_chapter_html.tar.gz">HTML ([=
      (compute-size "html_chapter" "_chapter_html.tar.gz")
      =] gzipped tar file)</a> with one web page per chapter.
  <li> formatted in <a href="html_node/[= (. package) =]_toc.html">HTML</a>
       with one web page per node.
  <li> formatted in <a href="html_node/[= (. package)
       =]_node_html.tar.gz">HTML ([=
      (compute-size "html_node" "_node_html.tar.gz")
      =] gzipped tar file)</a> with one web page per node.
  <li>formatted as an <a href="info/[= (. package)
       =].info.gz">Info document ([=
      (compute-size "info" ".info.gz")
      =] bytes gzipped tar file)</a>.
  <li>formatted as <a href="text/[= (. package) =].txt">ASCII text ([=
      (compute-size "text" ".txt")
      =] characters)</a>.
  <li>formatted as <a href="text/[= (. package) =].txt.gz">ASCII text ([=
      (compute-size "text" ".txt.gz")
      =] gzipped bytes)</a>.
  <li>formatted as <a href="dvi/[= (. package) =].dvi.gz">a TeX dvi file ([=
      (compute-size "dvi" ".dvi.gz")
      =] gzipped bytes)</a>.
  <li>formatted as <a href="pdf/[= (. package) =].pdf.gz">a PDF file ([=
      (compute-size "pdf" ".pdf.gz")
      =] gzipped bytes)</a>.
  <li>formatted as <a href="ps/[= (. package) =].ps.gz">a PostScript file ([=
      (compute-size "ps" ".ps.gz")
      =] gzipped bytes)</a>.
  <li>the original <a href="texi/[= (. package) =].texi.gz">Texinfo source ([=
      (compute-size "texi" ".texi.gz")
      =] gzipped bytes)</a>
</ul>

<p>(This page generated by the <a href="http://www.gnu.org/software/autogen"
>autogen program</a> in conjunction with <a
href="http://savannah.gnu.org/cgi-bin/viewcvs/autogen/autogen/pkg/gnudoc.tpl"
>a fairly simple template</a>.)</p>

<div class="copyright">
<p>
Return to the <a href="/home.html">GNU Project home page</a>.
</p>

<p>
Please send FSF &amp; GNU inquiries to
<a href="mailto:gnu@gnu.org"><em>gnu@gnu.org</em></a>.
There are also <a href="/home.html#ContactInfo">other ways to contact</a>
the FSF.
<br />
Please send broken links and other corrections (or suggestions) to
<a href="mailto:webmasters@gnu.org"><em>webmasters@gnu.org</em></a>.
</p>

<p>
Copyright (C) 2010 Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02111, USA
<br />
Verbatim copying and distribution of this entire article is
permitted in any medium, provided this notice is preserved.
</p>

<p>
Updated:
<!-- timestamp start -->
$Date: 2009/01/01 16:49:26 $ $Author: bkorb $
<!-- timestamp end -->
</p>
</div>

</body>
</html>
