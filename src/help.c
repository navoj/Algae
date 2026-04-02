/*
 * help.c -- Help system for Algae.
 *
 * Provides the `help' and `docstring' builtins.
 *
 * help(name)       - Display documentation for a symbol.
 * docstring(fn, s) - Attach a documentation string to a user function.
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

static char rcsid[] =
"$Id: help.c 2026/04/02 algae Exp $";

#include "algae.h"
#include "help.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "matrix.h"
#include "table.h"
#include "function.h"
#include "ptypes.h"
#include "psr.h"
#include "cast.h"
#include "printf.h"

/* The key used to store documentation in a function's `stuff' table. */
#define DOC_KEY "__doc__"

/*
 * Built-in help text for builtin functions.
 * Entries are {name, one-line synopsis}.
 * Add new entries in alphabetical order.
 */
static struct {
  const char *name;
  const char *help;
} builtin_help[] = {
  {"abs",         "abs(x) -- Absolute value."},
  {"acos",        "acos(x) -- Arc cosine."},
  {"acosh",       "acosh(x) -- Inverse hyperbolic cosine."},
  {"asin",        "asin(x) -- Arc sine."},
  {"asinh",       "asinh(x) -- Inverse hyperbolic sine."},
  {"atan",        "atan(x) -- Arc tangent (robust for complex args)."},
  {"atanh",       "atanh(x) -- Inverse hyperbolic tangent (robust for complex args)."},
  {"atan2",       "atan2(y, x) -- Two-argument arc tangent."},
  {"atof",        "atof(s) -- Convert string to floating point."},
  {"backsub",     "backsub(a, b) -- Back substitution."},
  {"band",        "band(x) -- Extract band structure."},
  {"bdiag",       "bdiag(a, nbr, nbc) -- Block diagonal."},
  {"btrans",      "btrans(a, nbr, nbc) -- Block transpose."},
  {"builtin",     "builtin(obj, sym) -- Load a builtin from a shared object."},
  {"cd",          "cd([path]) -- Change directory."},
  {"ceil",        "ceil(x) -- Ceiling function."},
  {"char",        "char(x) -- Convert to character type."},
  {"chol",        "chol(a) -- Cholesky factorization."},
  {"class",       "class([x]) -- Return or display entity class."},
  {"close",       "close(fname) -- Close an open file."},
  {"complement",  "complement(a, b) -- Set complement."},
  {"conj",        "conj(x) -- Complex conjugate."},
  {"cos",         "cos(x) -- Cosine."},
  {"cosh",        "cosh(x) -- Hyperbolic cosine."},
  {"cram",        "cram(shape, v) -- Reshape array."},
  {"dense",       "dense(x) -- Convert to dense storage."},
  {"diag",        "diag(x) -- Extract or form diagonal."},
  {"dice",        "dice(s) -- Split string into characters."},
  {"digits",      "digits([n]) -- Get/set display precision."},
  {"docstring",   "docstring(fn, text) -- Set help text for a user function."},
  {"eig",         "eig(a [,b [,c]]) -- Eigenvalue decomposition."},
  {"equilibrate", "equilibrate(a) -- Equilibrate a matrix."},
  {"erf",         "erf(x) -- Error function."},
  {"erfc",        "erfc(x) -- Complementary error function."},
  {"exception",   "exception() -- Return last exception info."},
  {"exec",        "exec(cmd) -- Execute a string as Algae code."},
  {"exit",        "exit([status]) -- Exit Algae."},
  {"exp",         "exp(x) -- Exponential function."},
  {"exsparse",    "exsparse(x) -- Expand sparse storage."},
  {"factor",      "factor(a) -- LU factorization."},
  {"fft",         "fft(x [,dim]) -- Fast Fourier transform."},
  {"file",        "file() -- Return current file name."},
  {"fill",        "fill(shape, val) -- Create array filled with val."},
  {"filter",      "filter(b, a, x [,z]) -- Digital filter."},
  {"find",        "find(a, b) -- Find elements."},
  {"floor",       "floor(x) -- Floor function."},
  {"form",        "form(shape, v) -- Form array from values."},
  {"fprintf",     "fprintf(file, fmt, ...) -- Formatted output to file."},
  {"fread",       "fread([file [,len [,type]]]) -- Read binary data."},
  {"fseek",       "fseek(file, offset [,whence]) -- Seek in file (whence: \"set\", \"cur\", \"end\")."},
  {"ftell",       "ftell([file]) -- Return current file position."},
  {"full",        "full(x) -- Convert to full storage."},
  {"fwrite",      "fwrite([file,] data [,type]) -- Write binary data."},
  {"get",         "get([file]) -- Read Algae binary entity."},
  {"getenv",      "getenv(name) -- Get environment variable."},
  {"gpskca",      "gpskca(x [,b]) -- Gibbs-Poole-Stockmeyer."},
  {"help",        "help([name]) -- Display help for a function or variable."},
  {"hermitian",   "hermitian(x) -- Extract Hermitian part."},
  {"ifft",        "ifft(x [,dim]) -- Inverse FFT."},
  {"imag",        "imag(x) -- Imaginary part."},
  {"integer",     "integer(x) -- Convert to integer type."},
  {"intersection","intersection(a, b) -- Set intersection."},
  {"iram",        "iram(dim, op, b, params, opts) -- Implicitly restarted Arnoldi."},
  {"isort",       "isort(x) -- Index sort (returns permutation)."},
  {"leastsq",     "leastsq(a, b) -- Least-squares solution."},
  {"line",        "line() -- Return current line number."},
  {"log",         "log(x) -- Natural logarithm."},
  {"lose",        "lose(a, b) -- Remove elements."},
  {"matrix",      "matrix([x]) -- Convert to matrix."},
  {"imax",        "imax(x) -- Index of maximum element."},
  {"members",     "members(t) -- List table members."},
  {"imin",        "imin(x) -- Index of minimum element."},
  {"mksparse",    "mksparse([t]) -- Create sparse matrix from table."},
  {"norm",        "norm(a [,p]) -- Matrix/vector norm."},
  {"print",       "print(x [,file]) -- Print an entity."},
  {"printf",      "printf(fmt, ...) -- Formatted output (supports array args)."},
  {"put",         "put(x [,file]) -- Write Algae binary entity."},
  {"rand",        "rand([shape]) -- Random numbers."},
  {"read",        "read([file]) -- Read text input."},
  {"readnum",     "readnum([file [,n]]) -- Read numeric data."},
  {"real",        "real(x) -- Real part or convert to real."},
  {"round",       "round(x) -- Round to nearest integer."},
  {"scalar",      "scalar([x]) -- Convert to scalar."},
  {"set",         "set([x]) -- Display/set options."},
  {"sin",         "sin(x) -- Sine."},
  {"sinh",        "sinh(x) -- Hyperbolic sine."},
  {"sort",        "sort(x) -- Sort elements."},
  {"source",      "source(file) -- Execute commands from file."},
  {"sparse",      "sparse(x) -- Convert to sparse storage."},
  {"split",       "split(str [,sep]) -- Split string."},
  {"sprintf",     "sprintf(fmt, ...) -- Formatted string (supports array args)."},
  {"sqrt",        "sqrt(x) -- Square root."},
  {"srand",       "srand([seed]) -- Seed random number generator."},
  {"strip",       "strip(f) -- Strip debug info from function."},
  {"substr",      "substr(s, i [,j]) -- Substring."},
  {"svd",         "svd(a [,t]) -- Singular value decomposition."},
  {"symmetric",   "symmetric(x) -- Extract symmetric part."},
  {"system",      "system(cmd) -- Execute shell command."},
  {"tan",         "tan(x) -- Tangent."},
  {"tanh",        "tanh(x) -- Hyperbolic tangent."},
  {"test",        "test([x]) -- Run test suite."},
  {"time",        "time() -- Return elapsed time."},
  {"tolower",     "tolower(s) -- Convert to lowercase."},
  {"toupper",     "toupper(s) -- Convert to uppercase."},
  {"transform",   "transform(a, p) -- Similarity transform."},
  {"tril",        "tril(x [,k]) -- Lower triangular part."},
  {"triu",        "triu(x [,k]) -- Upper triangular part."},
  {"union",       "union(a, b) -- Set union."},
  {"vector",      "vector([x]) -- Convert to vector."},
  {"zero",        "zero([shape]) -- Create zero array."},
  {NULL, NULL}
};

static const char *
find_builtin_help (const char *name)
{
  int i;
  for (i = 0; builtin_help[i].name; i++)
    if (strcmp (builtin_help[i].name, name) == 0)
      return builtin_help[i].help;
  return NULL;
}

ENTITY *
bi_help (int n, ENTITY *name_arg)
{
  /*
   * help()      -- List all available builtins.
   * help(name)  -- Show documentation for the named symbol.
   *
   * For user functions with a docstring, shows the user-provided doc.
   * For builtins, shows the built-in synopsis.
   * For variables, shows the class and type.
   */

  if (!name_arg)
    {
      /* No argument: list all builtin names */
      int i;
      xfprintf (stdout, "Available builtin functions:\n\n");
      for (i = 0; builtin_help[i].name; i++)
        {
          xfprintf (stdout, "  %s\n", builtin_help[i].help);
        }
      xfprintf (stdout, "\nUse help(\"name\") for details on a specific function.\n");
      return NULL;
    }

  WITH_HANDLING
    {
      char *name;
      SYMTAB *stp;

      name_arg = (ENTITY *) scalar_entity (EAT (name_arg));
      if (((SCALAR *)name_arg)->type != character)
        {
          fail ("help: Argument must be a string.");
          raise_exception ();
        }
      name = ((SCALAR *)name_arg)->v.character;

      /* Look up the name in the symbol table */
      stp = find (dup_char (name), NO_CREATE);

      if (stp && stp->stval.datum && stp->stval.datum->type == D_ENTITY)
        {
          ENTITY *e = E_PTR (stp->stval.datum);
          if (e && e->class == function)
            {
              FUNCTION *fp = (FUNCTION *) e;

              /* Check for user-supplied docstring */
              if (fp->stuff)
                {
                  ENTITY *doc = search_in_table (
                    (TABLE *) copy_table (fp->stuff), DOC_KEY);
                  if (doc)
                    {
                      if (doc->class == scalar &&
                          ((SCALAR *)doc)->type == character)
                        {
                          xfprintf (stdout, "%s:\n  %s\n",
                                    name, ((SCALAR *)doc)->v.character);
                        }
                      delete_entity (doc);
                      delete_entity (name_arg);
                      name_arg = NULL;
                      goto done_help;
                    }
                }

              /* Fall back to builtin help */
              if (fp->ilk == builtin)
                {
                  const char *h = find_builtin_help (name);
                  if (h)
                    xfprintf (stdout, "%s\n", h);
                  else
                    xfprintf (stdout, "%s: builtin function (no documentation available).\n", name);
                }
              else
                {
                  UF_BLOCK *ufp = (UF_BLOCK *) fp->funct_p;
                  xfprintf (stdout, "%s: user function (%d argument%s).\n",
                            name, ufp->cargs, PLURAL (ufp->cargs));
                  xfprintf (stdout, "  Use docstring(%s, \"...\") to add documentation.\n", name);
                }
            }
          else if (e)
            {
              xfprintf (stdout, "%s: %s", name, class_string[e->class]);
              if (e->class == scalar)
                xfprintf (stdout, " (%s)", type_string[((SCALAR *)e)->type]);
              else if (e->class == vector)
                xfprintf (stdout, " (%s, %d element%s)",
                          type_string[((VECTOR *)e)->type],
                          ((VECTOR *)e)->ne,
                          PLURAL (((VECTOR *)e)->ne));
              else if (e->class == matrix)
                xfprintf (stdout, " (%s, %dx%d)",
                          type_string[((MATRIX *)e)->type],
                          ((MATRIX *)e)->nr,
                          ((MATRIX *)e)->nc);
              xfprintf (stdout, ".\n");
            }
          else
            {
              xfprintf (stdout, "%s: defined but NULL.\n", name);
            }
        }
      else
        {
          /* Not in symbol table -- check builtin help */
          const char *h = find_builtin_help (name);
          if (h)
            xfprintf (stdout, "%s\n", h);
          else
            xfprintf (stdout, "%s: not found.\n", name);
        }
    done_help: ;
    }
  ON_EXCEPTION
    {
      delete_entity (name_arg);
    }
  END_EXCEPTION;

  if (name_arg) delete_entity (name_arg);
  return NULL;
}

ENTITY *
bi_docstring (ENTITY *fn_arg, ENTITY *text_arg)
{
  /*
   * docstring(fn, text) -- Attach a documentation string to
   * a user function.  The function's `stuff' table member is
   * used to store the doc string under the key "__doc__".
   *
   * fn:   A function entity.
   * text: A character string.
   *
   * Returns the function.
   */

  WITH_HANDLING
    {
      FUNCTION *fp;
      char *doc;

      if (!fn_arg || fn_arg->class != function)
        {
          fail ("docstring: First argument must be a function.");
          raise_exception ();
        }

      text_arg = (ENTITY *) scalar_entity (EAT (text_arg));
      if (((SCALAR *)text_arg)->type != character)
        {
          fail ("docstring: Second argument must be a string.");
          raise_exception ();
        }

      doc = dup_char (((SCALAR *)text_arg)->v.character);
      delete_entity (text_arg);
      text_arg = NULL;

      /* Get a mutable copy of the function */
      fn_arg = dup_function ((FUNCTION *) fn_arg);
      fp = (FUNCTION *) fn_arg;

      if (!fp->stuff)
        fp->stuff = (TABLE *) make_table ();

      fp->stuff = (TABLE *) replace_in_table (
        fp->stuff,
        (ENTITY *) char_to_scalar (doc),
        dup_char (DOC_KEY));
    }
  ON_EXCEPTION
    {
      delete_entity (fn_arg);
      delete_entity (text_arg);
    }
  END_EXCEPTION;

  return fn_arg;
}
