/*
   logical.h -- Prototypes for `logical.c'.

   Copyright (C) 1994-95  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: logical.h,v 1.1.1.1 1996/04/17 05:56:18 ksh Exp $ */

#ifndef LOGICAL_H
#define LOGICAL_H

#include "entity.h"

int PROTO (eq_integer, (int l, int r));
int PROTO (eq_real, (REAL l, REAL r));
int PROTO (eq_complex, (COMPLEX l, COMPLEX r));
int PROTO (eq_character, (char *l, char *r));

int PROTO (ne_integer, (int l, int r));
int PROTO (ne_real, (REAL l, REAL r));
int PROTO (ne_complex, (COMPLEX l, COMPLEX r));
int PROTO (ne_character, (char *l, char *r));

int PROTO (lt_integer, (int l, int r));
int PROTO (lt_real, (REAL l, REAL r));
int PROTO (lt_character, (char *l, char *r));

int PROTO (lte_integer, (int l, int r));
int PROTO (lte_real, (REAL l, REAL r));
int PROTO (lte_character, (char *l, char *r));

int PROTO (gt_integer, (int l, int r));
int PROTO (gt_real, (REAL l, REAL r));
int PROTO (gt_character, (char *l, char *r));

int PROTO (gte_integer, (int l, int r));
int PROTO (gte_real, (REAL l, REAL r));
int PROTO (gte_character, (char *l, char *r));

int PROTO (and_integer, (int l, int r));
int PROTO (and_real, (REAL l, REAL r));
int PROTO (and_complex, (COMPLEX l, COMPLEX r));
int PROTO (and_character, (char *l, char *r));

int PROTO (or_integer, (int l, int r));
int PROTO (or_real, (REAL l, REAL r));
int PROTO (or_complex, (COMPLEX l, COMPLEX r));
int PROTO (or_character, (char *l, char *r));

extern ENTITY *PROTO (logical_scalar_vector_integer,
		        (SCALAR * l, VECTOR * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_scalar_vector_real,
		        (SCALAR * l, VECTOR * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_scalar_vector_complex,
		  (SCALAR * l, VECTOR * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_scalar_vector_character,
		    (SCALAR * l, VECTOR * r, int (*func) (char *, char *)));
extern ENTITY *PROTO (logical_vector_integer,
		        (VECTOR * l, VECTOR * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_vector_real,
		        (VECTOR * l, VECTOR * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_vector_complex,
		  (VECTOR * l, VECTOR * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_vector_character,
		    (VECTOR * l, VECTOR * r, int (*func) (char *, char *)));
extern ENTITY *PROTO (logical_scalar_matrix_integer,
		        (SCALAR * l, MATRIX * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_scalar_matrix_real,
		        (SCALAR * l, MATRIX * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_scalar_matrix_complex,
		  (SCALAR * l, MATRIX * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_scalar_matrix_character,
		    (SCALAR * l, MATRIX * r, int (*func) (char *, char *)));
extern ENTITY *PROTO (logical_vector_matrix_integer,
		        (VECTOR * l, MATRIX * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_matrix_vector_integer,
		        (MATRIX * l, VECTOR * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_vector_matrix_real,
		        (VECTOR * l, MATRIX * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_matrix_vector_real,
		        (MATRIX * l, VECTOR * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_vector_matrix_complex,
		  (VECTOR * l, MATRIX * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_matrix_vector_complex,
		  (MATRIX * l, VECTOR * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_vector_matrix_character,
		    (VECTOR * l, MATRIX * r, int (*func) (char *, char *)));
extern ENTITY *PROTO (logical_matrix_vector_character,
		    (MATRIX * l, VECTOR * r, int (*func) (char *, char *)));
extern ENTITY *PROTO (logical_matrix_integer,
		        (MATRIX * l, MATRIX * r, int (*func) (int, int)));
extern ENTITY *PROTO (logical_matrix_real,
		        (MATRIX * l, MATRIX * r, int (*func) (REAL, REAL)));
extern ENTITY *PROTO (logical_matrix_complex,
		  (MATRIX * l, MATRIX * r, int (*func) (COMPLEX, COMPLEX)));
extern ENTITY *PROTO (logical_matrix_character,
		    (MATRIX * l, MATRIX * r, int (*func) (char *, char *)));

#endif /* LOGICAL_H */
