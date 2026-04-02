/*
 * fread.h -- Prototypes for `fread.c'.
 *
 * Copyright (C) 1994-97  K. Scott Hunziker.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

/* $Id: fread.h,v 1.2 1997/03/28 21:37:48 ksh Exp $ */

#ifndef FREAD_H
#define FREAD_H	1

#include "entity.h"

/* The types and qualifiers are enumerated here, with `dt_default' last. */

typedef enum
{
  dt_char,
  dt_int,
  dt_float,
  dt_double,
  dt_long,
  dt_short,
  dt_signed,
  dt_unsigned,
  dt_big,
  dt_little,
  dt_ieee,
  dt_default
} DATA_TYPE;

struct data_type
{
  DATA_TYPE base;
  DATA_TYPE sgn;
  DATA_TYPE size;
  DATA_TYPE endian;
  DATA_TYPE ieee;
};

extern ENTITY * PROTO (bi_fread,
                       (int n, ENTITY *fname, ENTITY *length, ENTITY *type));
extern ENTITY * PROTO (bi_fwrite,
                       (int n, ENTITY *fname, ENTITY *v, ENTITY *type));
extern ENTITY * PROTO (bi_fseek,
                       (int n, ENTITY *fname, ENTITY *offset, ENTITY *whence));
extern ENTITY * PROTO (bi_ftell,
                       (int n, ENTITY *fname));

#endif /* FREAD_H */
