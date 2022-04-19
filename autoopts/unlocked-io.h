/*  -*- buffer-read-only: t -*- vi: set ro:
 *
 * DO NOT EDIT THIS FILE   (unlocked-io.m4)
 *
 * It has been derived from /usr/include/stdio.h
 * using the script config/mk-unlocked-io.sh
 *
 * This file is part of AutoOpts, a companion to AutoGen.
 * AutoOpts is free software.
 * AutoOpts is Copyright (C) 1992-2020 by Bruce Korb - all rights reserved
 *
 * Automated Options (AutoOpts) Copyright (C) 1992-2020 by Bruce Korb
 *
 * AutoOpts is available under any one of two licenses.  The license
 * in use must be one of these two and the choice is under the control
 * of the user of the license.
 *
 *  The GNU Lesser General Public License, version 3 or later
 *     See the files "COPYING.lgplv3" and "COPYING.gplv3"
 *
 *  The Modified Berkeley Software Distribution License
 *     See the file "COPYING.mbsd"
 *
 * These files have the following sha256 sums:
 *
 * 8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
 * 4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
 * 13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd
 */
#ifndef UNLOCKED_IO_HEADER_GUARD
#define UNLOCKED_IO_HEADER_GUARD 1

#if HAVE_DECL_FFLUSH_UNLOCKED
#  undef  fflush
#  define fflush(_z) fflush_unlocked (_z)
#else
#  define fflush_unlocked(_z) fflush (_z)
#endif

#if HAVE_DECL_GETC_UNLOCKED
#  undef  getc
#  define getc(_z) getc_unlocked (_z)
#else
#  define getc_unlocked(_z) getc (_z)
#endif

#if HAVE_DECL_GETCHAR_UNLOCKED
#  undef  getchar
#  define getchar() getchar_unlocked ()
#else
#  define getchar_unlocked() getchar ()
#endif

#if HAVE_DECL_FGETC_UNLOCKED
#  undef  fgetc
#  define fgetc(_z) fgetc_unlocked (_z)
#else
#  define fgetc_unlocked(_z) fgetc (_z)
#endif

#if HAVE_DECL_FPUTC_UNLOCKED
#  undef  fputc
#  define fputc(_y,_z) fputc_unlocked (_y,_z)
#else
#  define fputc_unlocked(_y,_z) fputc (_y,_z)
#endif

#if HAVE_DECL_PUTC_UNLOCKED
#  undef  putc
#  define putc(_y,_z) putc_unlocked (_y,_z)
#else
#  define putc_unlocked(_y,_z) putc (_y,_z)
#endif

#if HAVE_DECL_PUTCHAR_UNLOCKED
#  undef  putchar
#  define putchar(_z) putchar_unlocked (_z)
#else
#  define putchar_unlocked(_z) putchar (_z)
#endif

#if HAVE_DECL_FGETS_UNLOCKED
#  undef  fgets
#  define fgets(_x,_y,_z) fgets_unlocked (_x,_y,_z)
#else
#  define fgets_unlocked(_x,_y,_z) fgets (_x,_y,_z)
#endif

#if HAVE_DECL_FPUTS_UNLOCKED
#  undef  fputs
#  define fputs(_y,_z) fputs_unlocked (_y,_z)
#else
#  define fputs_unlocked(_y,_z) fputs (_y,_z)
#endif

#if HAVE_DECL_FREAD_UNLOCKED
#  undef  fread
#  define fread(_w,_x,_y,_z) fread_unlocked (_w,_x,_y,_z)
#else
#  define fread_unlocked(_w,_x,_y,_z) fread (_w,_x,_y,_z)
#endif

#if HAVE_DECL_FWRITE_UNLOCKED
#  undef  fwrite
#  define fwrite(_w,_x,_y,_z) fwrite_unlocked (_w,_x,_y,_z)
#else
#  define fwrite_unlocked(_w,_x,_y,_z) fwrite (_w,_x,_y,_z)
#endif

#if HAVE_DECL_CLEARERR_UNLOCKED
#  undef  clearerr
#  define clearerr(_z) clearerr_unlocked (_z)
#else
#  define clearerr_unlocked(_z) clearerr (_z)
#endif

#if HAVE_DECL_FEOF_UNLOCKED
#  undef  feof
#  define feof(_z) feof_unlocked (_z)
#else
#  define feof_unlocked(_z) feof (_z)
#endif

#if HAVE_DECL_FERROR_UNLOCKED
#  undef  ferror
#  define ferror(_z) ferror_unlocked (_z)
#else
#  define ferror_unlocked(_z) ferror (_z)
#endif

#if HAVE_DECL_FILENO_UNLOCKED
#  undef  fileno
#  define fileno(_z) fileno_unlocked (_z)
#else
#  define fileno_unlocked(_z) fileno (_z)
#endif
#endif // UNLOCKED_IO_HEADER_GUARD