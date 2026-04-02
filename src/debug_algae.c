/*
 * debug_algae.c -- Symbolic debugger for Algae.
 *
 * Provides breakpoints, single-stepping, and variable inspection
 * for Algae user code.
 *
 * Usage:
 *   breakpoint("file.a", 10)     -- Set a breakpoint
 *   dbclear("file.a", 10)        -- Clear a breakpoint
 *   dbclear()                    -- Clear all breakpoints
 *   dbstep()                     -- Enable single-step mode
 *   dbstep("over")               -- Step over function calls
 *   dbcont()                     -- Continue execution
 *   dbstack()                    -- Show call stack info
 *   dbstatus()                   -- Show all breakpoints
 *
 * When a breakpoint is hit or single-stepping, the debugger
 * enters an interactive prompt where you can inspect variables
 * using normal Algae expressions.
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: debug_algae.c 2026/04/02 algae Exp $";

#include <stdio.h>
#include <string.h>
#include "algae.h"
#include "debug_algae.h"
#include "entity.h"
#include "scalar.h"
#include "table.h"
#include "ptypes.h"
#include "psr.h"
#include "cast.h"
#include "printf.h"

/* --- Debugger state --- */

int debugger_enabled = 0;
int debugger_stepping = 0;
int debugger_next = 0;
int debugger_call_depth = 0;
static int debugger_stop_depth = 0;  /* for "step over" */

static BREAKPOINT breakpoints[MAX_BREAKPOINTS];
static int num_breakpoints = 0;

void
debugger_init (void)
{
  int i;
  for (i = 0; i < MAX_BREAKPOINTS; i++)
    {
      breakpoints[i].active = 0;
      breakpoints[i].file = NULL;
      breakpoints[i].line = 0;
    }
  num_breakpoints = 0;
  debugger_enabled = 0;
  debugger_stepping = 0;
  debugger_next = 0;
  debugger_call_depth = 0;
}

static int
is_breakpoint (const char *file, int line)
{
  int i;
  for (i = 0; i < num_breakpoints; i++)
    {
      if (breakpoints[i].active &&
          breakpoints[i].line == line &&
          breakpoints[i].file &&
          file &&
          strcmp (breakpoints[i].file, file) == 0)
        return 1;
    }
  return 0;
}

/*
 * debugger_check -- Called from execute.c at each OP_LINE instruction.
 * Checks if we should stop (breakpoint hit or stepping).
 */
void
debugger_check (const char *file, int line)
{
  int should_stop = 0;

  if (!debugger_enabled) return;

  /* Check breakpoints */
  if (is_breakpoint (file, line))
    should_stop = 1;

  /* Check stepping modes */
  if (debugger_stepping)
    should_stop = 1;
  else if (debugger_next && debugger_call_depth <= debugger_stop_depth)
    should_stop = 1;

  if (!should_stop) return;

  /* We're stopping -- enter debug prompt */
  debugger_stepping = 0;
  debugger_next = 0;

  xfprintf (stdout, "\n");
  if (file)
    xfprintf (stdout, "Stopped at %s, line %d.\n", file, line);
  else
    xfprintf (stdout, "Stopped at line %d.\n", line);
  xfprintf (stdout, "Debug commands: dbstep(), dbcont(), dbstack(), dbstatus()\n");
  xfprintf (stdout, "Type any Algae expression to inspect variables.\n");

  /* Enter interactive debug loop */
  {
    char buf[1024];
    while (1)
      {
        xfprintf (stdout, "debug> ");
        fflush (stdout);

        if (!fgets (buf, sizeof(buf), stdin))
          {
            /* EOF -- continue execution */
            break;
          }

        /* Strip trailing newline */
        {
          int len = strlen (buf);
          if (len > 0 && buf[len-1] == '\n')
            buf[len-1] = '\0';
        }

        /* Check for debugger commands */
        if (strcmp (buf, "c") == 0 || strcmp (buf, "cont") == 0 ||
            strcmp (buf, "dbcont()") == 0 || strcmp (buf, "dbcont") == 0)
          {
            break;
          }
        else if (strcmp (buf, "s") == 0 || strcmp (buf, "step") == 0 ||
                 strcmp (buf, "dbstep()") == 0 || strcmp (buf, "dbstep") == 0)
          {
            debugger_stepping = 1;
            break;
          }
        else if (strcmp (buf, "n") == 0 || strcmp (buf, "next") == 0 ||
                 strncmp (buf, "dbstep(\"over\")", 14) == 0)
          {
            debugger_next = 1;
            debugger_stop_depth = debugger_call_depth;
            break;
          }
        else if (strcmp (buf, "q") == 0 || strcmp (buf, "quit") == 0)
          {
            debugger_enabled = 0;
            debugger_stepping = 0;
            debugger_next = 0;
            xfprintf (stdout, "Debugger disabled.\n");
            break;
          }
        else if (strcmp (buf, "bt") == 0 || strcmp (buf, "where") == 0 ||
                 strcmp (buf, "dbstack()") == 0 || strcmp (buf, "dbstack") == 0)
          {
            if (file)
              xfprintf (stdout, "  %s:%d (depth %d)\n", file, line,
                        debugger_call_depth);
            else
              xfprintf (stdout, "  <interactive>:%d (depth %d)\n", line,
                        debugger_call_depth);
          }
        else if (strcmp (buf, "bl") == 0 || strcmp (buf, "list") == 0 ||
                 strcmp (buf, "dbstatus()") == 0 || strcmp (buf, "dbstatus") == 0)
          {
            int i, count = 0;
            for (i = 0; i < num_breakpoints; i++)
              if (breakpoints[i].active)
                {
                  xfprintf (stdout, "  Breakpoint %d: %s:%d\n",
                            i+1, breakpoints[i].file, breakpoints[i].line);
                  count++;
                }
            if (count == 0)
              xfprintf (stdout, "  No breakpoints set.\n");
          }
        else if (strcmp (buf, "h") == 0 || strcmp (buf, "help") == 0 ||
                 strcmp (buf, "?") == 0)
          {
            xfprintf (stdout, "Debug commands:\n");
            xfprintf (stdout, "  s, step     - single step\n");
            xfprintf (stdout, "  n, next     - step over function calls\n");
            xfprintf (stdout, "  c, cont     - continue execution\n");
            xfprintf (stdout, "  bt, where   - show call stack\n");
            xfprintf (stdout, "  bl, list    - list breakpoints\n");
            xfprintf (stdout, "  q, quit     - disable debugger and continue\n");
            xfprintf (stdout, "  h, help, ?  - this help\n");
            xfprintf (stdout, "  <expr>      - evaluate Algae expression\n");
          }
        else if (buf[0] != '\0')
          {
            /* Try to evaluate as an Algae expression */
            SYMTAB *stp = find (dup_char (buf), NO_CREATE);
            if (stp && stp->stval.datum)
              {
                DATUM d;
                copy_datum (&d, stp->stval.datum);
                if (d.type == D_ENTITY && E_PTR(&d))
                  {
                    ENTITY *e = E_PTR(&d);
                    /* Use the print machinery */
                    xfprintf (stdout, "  %s = ", buf);
                    if (e->class == scalar)
                      {
                        SCALAR *sc = (SCALAR *)e;
                        switch (sc->type)
                          {
                          case integer:
                            xfprintf (stdout, "%d\n", sc->v.integer);
                            break;
                          case real:
                            xfprintf (stdout, "%.6g\n", sc->v.real);
                            break;
                          case character:
                            xfprintf (stdout, "\"%s\"\n", sc->v.character);
                            break;
                          case complex:
                            xfprintf (stdout, "(%.6g, %.6g)\n",
                                      sc->v.complex.real, sc->v.complex.imag);
                            break;
                          default:
                            xfprintf (stdout, "<%s>\n", type_string[sc->type]);
                            break;
                          }
                      }
                    else
                      xfprintf (stdout, "<%s>\n", class_string[e->class]);
                  }
                else if (d.type == D_INT)
                  xfprintf (stdout, "  %s = %d\n", buf, IVAL(&d));
                else if (d.type == D_REAL)
                  xfprintf (stdout, "  %s = %.6g\n", buf, RVAL(&d));
                else
                  xfprintf (stdout, "  %s: (null)\n", buf);
                delete_datum (&d);
              }
            else
              xfprintf (stdout, "  %s: undefined\n", buf);
          }
      }
  }
}

/* --- Builtin functions --- */

ENTITY *
bi_breakpoint (int n, ENTITY *file_arg, ENTITY *line_arg)
{
  /*
   * breakpoint(file, line) -- Set a breakpoint.
   * breakpoint(line)       -- Set on current file.
   */

  char *file;
  int line;
  int i;

  WITH_HANDLING
    {
      if (n == 1)
        {
          /* Single arg: just a line number, use current file */
          line = entity_to_int (EAT (file_arg));
          file = curr_file ? dup_char (curr_file) : dup_char ("<stdin>");
        }
      else
        {
          file_arg = (ENTITY *) scalar_entity (EAT (file_arg));
          if (((SCALAR *)file_arg)->type != character)
            {
              fail ("breakpoint: First argument must be a file name string.");
              raise_exception ();
            }
          file = dup_char (((SCALAR *)file_arg)->v.character);
          delete_entity (file_arg);
          file_arg = NULL;

          line = entity_to_int (EAT (line_arg));
        }

      /* Find a free slot */
      for (i = 0; i < MAX_BREAKPOINTS; i++)
        {
          if (!breakpoints[i].active)
            {
              breakpoints[i].active = 1;
              breakpoints[i].file = file;
              breakpoints[i].line = line;
              if (i >= num_breakpoints)
                num_breakpoints = i + 1;
              debugger_enabled = 1;
              xfprintf (stdout, "Breakpoint %d set at %s:%d\n",
                        i+1, file, line);
              goto bp_done;
            }
        }
      fail ("breakpoint: Maximum number of breakpoints (%d) reached.",
            MAX_BREAKPOINTS);
      FREE_CHAR (file);
      raise_exception ();
    bp_done: ;
    }
  ON_EXCEPTION
    {
      delete_entity (file_arg);
      delete_entity (line_arg);
    }
  END_EXCEPTION;

  return int_to_scalar (1);
}

ENTITY *
bi_dbclear (int n, ENTITY *file_arg, ENTITY *line_arg)
{
  /*
   * dbclear(file, line) -- Clear a specific breakpoint.
   * dbclear()           -- Clear all breakpoints.
   */

  if (n == 0)
    {
      /* Clear all */
      int i;
      for (i = 0; i < num_breakpoints; i++)
        {
          if (breakpoints[i].active)
            {
              breakpoints[i].active = 0;
              if (breakpoints[i].file)
                FREE_CHAR (breakpoints[i].file);
              breakpoints[i].file = NULL;
            }
        }
      num_breakpoints = 0;
      if (!debugger_stepping && !debugger_next)
        debugger_enabled = 0;
      xfprintf (stdout, "All breakpoints cleared.\n");
      return int_to_scalar (0);
    }

  WITH_HANDLING
    {
      char *file;
      int line, i;

      if (n == 1)
        {
          line = entity_to_int (EAT (file_arg));
          file = curr_file ? curr_file : "<stdin>";
        }
      else
        {
          file_arg = (ENTITY *) scalar_entity (EAT (file_arg));
          if (((SCALAR *)file_arg)->type != character)
            {
              fail ("dbclear: First argument must be a file name string.");
              raise_exception ();
            }
          file = ((SCALAR *)file_arg)->v.character;
          line = entity_to_int (EAT (line_arg));
        }

      for (i = 0; i < num_breakpoints; i++)
        {
          if (breakpoints[i].active &&
              breakpoints[i].line == line &&
              breakpoints[i].file &&
              strcmp (breakpoints[i].file, file) == 0)
            {
              breakpoints[i].active = 0;
              FREE_CHAR (breakpoints[i].file);
              breakpoints[i].file = NULL;
              xfprintf (stdout, "Breakpoint %d cleared.\n", i+1);
              goto clear_done;
            }
        }
      xfprintf (stdout, "No breakpoint at %s:%d.\n", file, line);
    clear_done: ;
      if (file_arg) delete_entity (file_arg);
    }
  ON_EXCEPTION
    {
      delete_entity (file_arg);
      delete_entity (line_arg);
    }
  END_EXCEPTION;

  return int_to_scalar (0);
}

ENTITY *
bi_dbstep (int n, ENTITY *mode)
{
  /*
   * dbstep()       -- Enable single-step mode.
   * dbstep("over") -- Step over function calls.
   */

  debugger_enabled = 1;

  if (mode)
    {
      mode = (ENTITY *) scalar_entity (EAT (mode));
      if (((SCALAR *)mode)->type == character &&
          strcmp (((SCALAR *)mode)->v.character, "over") == 0)
        {
          debugger_next = 1;
          debugger_stop_depth = debugger_call_depth;
          delete_entity (mode);
          return int_to_scalar (1);
        }
      delete_entity (mode);
    }

  debugger_stepping = 1;
  return int_to_scalar (1);
}

ENTITY *
bi_dbcont (void)
{
  /*
   * dbcont() -- Continue execution (clear stepping mode).
   */

  debugger_stepping = 0;
  debugger_next = 0;
  return int_to_scalar (0);
}

ENTITY *
bi_dbstack (void)
{
  /*
   * dbstack() -- Show current execution position.
   */

  if (curr_file)
    xfprintf (stdout, "  %s:%d (depth %d)\n", curr_file, curr_line_no,
              debugger_call_depth);
  else
    xfprintf (stdout, "  <interactive>:%d (depth %d)\n", curr_line_no,
              debugger_call_depth);

  return NULL;
}

ENTITY *
bi_dbstatus (void)
{
  /*
   * dbstatus() -- List all active breakpoints.
   */

  int i, count = 0;
  for (i = 0; i < num_breakpoints; i++)
    if (breakpoints[i].active)
      {
        xfprintf (stdout, "  Breakpoint %d: %s:%d\n",
                  i+1, breakpoints[i].file, breakpoints[i].line);
        count++;
      }
  if (count == 0)
    xfprintf (stdout, "  No breakpoints set.\n");
  xfprintf (stdout, "  Debugger: %s\n",
            debugger_enabled ? "enabled" : "disabled");
  xfprintf (stdout, "  Stepping: %s\n",
            debugger_stepping ? "yes" : (debugger_next ? "over" : "no"));

  return int_to_scalar (count);
}
