/*
   vargs.h -- chooses <varargs.h> or <stdarg.h>

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: vargs.h,v 1.2 1996/05/10 06:00:40 ksh Exp $ */

#if !HAVE_STDARG_H || NO_PROTOS
#include <varargs.h>

#define  VA_ALIST(x)  (va_alist) va_dcl { x ;

#define  VA_START(p,type, param)  va_start(p) ;\
				  param = va_arg(p,type)

#define  VA_ALIST2(a1,a2) (va_alist) va_dcl { a1 ; a2 ;

#define  VA_START2(p,t1,a1,t2,a2)  va_start(p) ;\
				   a1 = va_arg(p,t1);\
				   a2 = va_arg(p,t2)
#else
#include <stdarg.h>

#define  VA_ALIST(x)  (x, ...) {

#define  VA_START(p,type,param)   va_start(p,param)

#define  VA_ALIST2(a1,a2)  (a1,a2,...) {
#define  VA_START2(p,t1,a1,t2,a2)  va_start(p,a2)

#endif
