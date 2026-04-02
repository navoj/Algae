/*
   enum_strng.h -- Strings for enums.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: enum_strng.h,v 1.2 1996/10/25 05:28:28 ksh Exp $ */

#ifndef ENUM_STRNG_H
#define ENUM_STRNG_H	1

/* This should be included only once, in `entity.c'. */
#ifdef ENUM_STRNG_INIT

/* Naturally, these must be in the same order as the associated enums. */

char *class_string[] =
{
  "scalar",
  "table",
  "vector",
  "matrix",
  "function",
  "undefined_class",
};
char bad_class[] = "Can't handle the \"%s\" class.";

char *symmetry_string[] =
{
  "general",
  "symmetric",
  "skew_symmetric",	/* not used */
  "hermitian",
  "skew_hermitian",	/* not used */
  "undefined_symmetry",
};
char bad_symmetry[] = "Can't handle the \"%s\" symmetry.";

char *density_string[] =
{
  "dense",
  "sparse",
  "sparse_upper",
  "undefined_density",
};
char bad_density[] = "Can't handle the \"%s\" density.";

char *order_string[] =
{
  "ordered",
  "not_ordered",
  "undefined_order",
};
char bad_order[] = "Can't handle the \"%s\" order.";

char *type_string[] =
{
  "integer",
  "real",
  "complex",
  "character",
  "undefined_type",
};
char bad_type[] = "Can't handle the \"%s\" type.";
int type_size[] =
{
  sizeof (int),
  sizeof (REAL),
  sizeof (COMPLEX),
  sizeof (char *),
  1,
};

char *ilk_string[] =
{
  "builtin",
  "user",
  "undefined_ilk",
};
char bad_ilk[] = "Can't handle the \"%s\" function ilk.";

#endif /* ENUM_STRNG_INIT */

#endif /* ENUM_STRNG_H */
