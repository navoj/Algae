/*
 * debug.h -- Symbolic debugger for Algae.
 *
 * Provides breakpoints, stepping, and variable inspection.
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

#ifndef DEBUG_ALGAE_H
#define DEBUG_ALGAE_H 1

#include "entity.h"

/* Maximum number of breakpoints */
#define MAX_BREAKPOINTS 100

typedef struct {
  int active;
  char *file;
  int line;
} BREAKPOINT;

/* Debugger state */
extern int debugger_enabled;
extern int debugger_stepping;     /* single-step mode */
extern int debugger_next;         /* step-over mode (don't enter calls) */
extern int debugger_call_depth;   /* current function call depth */

/* Initialize/cleanup */
extern void PROTO (debugger_init, (void));

/* Called from execute.c at each OP_LINE */
extern void PROTO (debugger_check, (const char *file, int line));

/* Builtins */
extern ENTITY * PROTO (bi_breakpoint, (int n, ENTITY *file, ENTITY *line_arg));
extern ENTITY * PROTO (bi_dbclear, (int n, ENTITY *file, ENTITY *line_arg));
extern ENTITY * PROTO (bi_dbstep, (int n, ENTITY *mode));
extern ENTITY * PROTO (bi_dbcont, (void));
extern ENTITY * PROTO (bi_dbstack, (void));
extern ENTITY * PROTO (bi_dbstatus, (void));

#endif /* DEBUG_ALGAE_H */
