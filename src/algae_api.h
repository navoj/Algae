/*
 * algae_api.h -- Public API header for Algae extension writers.
 *
 * Include this single header when writing shared-object builtins
 * for the Algae builtin() dynamic linking facility.
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

#ifndef ALGAE_API_H
#define ALGAE_API_H 1

#include "algae.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "cast.h"
#include "dense.h"
#include "exception.h"
#include "message.h"
#include "mem.h"
#include "file_io.h"

/*
 * Quick reference for extension writers:
 *
 * Creating scalars:
 *   ENTITY *int_to_scalar(int i);
 *   ENTITY *real_to_scalar(REAL r);
 *   ENTITY *complex_to_scalar(COMPLEX c);
 *   ENTITY *char_to_scalar(char *s);      // s must be malloc'd
 *
 * Creating vectors:
 *   ENTITY *form_vector(int ne, TYPE type, DENSITY density);
 *   ENTITY *gift_wrap_vector(int ne, TYPE type, void *data);
 *
 * Creating tables:
 *   ENTITY *make_table(void);
 *   ENTITY *replace_in_table(TABLE *t, ENTITY *val, char *name);
 *
 * Conversions:
 *   ENTITY *scalar_entity(ENTITY *p);     // any -> scalar
 *   ENTITY *vector_entity(ENTITY *p);     // any -> vector
 *   ENTITY *cast_scalar(SCALAR *s, TYPE target);
 *   ENTITY *cast_vector(VECTOR *v, TYPE target);
 *   int entity_to_int(ENTITY *e);         // consumes entity
 *   char *entity_to_string(ENTITY *e);    // consumes; returns malloc'd
 *
 * Memory:
 *   EAT(p)                - consume ownership
 *   delete_entity(p)      - free when done
 *   dup_scalar(s)         - get mutable copy
 *   dup_vector(v)         - get mutable copy
 *   copy_entity(e)        - increment ref count
 *   MALLOC(n), FREE(p)    - memory allocation
 *
 * Errors:
 *   fail("message", ...); raise_exception();
 *   WITH_HANDLING { ... } ON_EXCEPTION { ... } END_EXCEPTION;
 */

#endif /* ALGAE_API_H */
