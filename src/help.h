/*
 * help.h -- Prototypes for help system.
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

#ifndef HELP_H
#define HELP_H	1

#include "entity.h"

extern ENTITY * PROTO (bi_help, (int n, ENTITY *name));
extern ENTITY * PROTO (bi_docstring, (ENTITY *fn, ENTITY *text));

#endif /* HELP_H */
