# Algae Builtin Interface Developer Guide

This document explains how to write C extensions (builtins) for Algae
using the dynamic linking capability provided by the `builtin()` function.

## Overview

Algae includes a `builtin(object_file, symbol_name)` function that loads
a shared object (`.so`) at runtime and registers a compiled C function
as an Algae builtin.  This guide documents the interface and conventions
needed to write such extensions.

## Prerequisites

You will need the Algae header files.  The key headers are:

| Header         | Purpose                                      |
|----------------|----------------------------------------------|
| `algae.h`      | Master header: types, macros, includes       |
| `entity.h`     | ENTITY, CLASS, TYPE enums and base struct     |
| `scalar.h`     | Scalar creation/manipulation                 |
| `vector.h`     | Vector creation/manipulation                 |
| `matrix.h`     | Matrix creation/manipulation                 |
| `table.h`      | Table (associative array) operations         |
| `function.h`   | FUNCTION, BUILTIN structures                 |
| `builtin.h`    | Builtin registration table                   |
| `cast.h`       | Type casting functions                       |
| `dense.h`      | Dense storage conversion                     |
| `exception.h`  | Exception handling macros                    |
| `mem.h`        | Memory allocation macros (MALLOC, FREE, etc) |
| `message.h`    | Error/warning functions (fail, warn, etc)    |

## Type System

### Entity Classes

Every Algae value is an `ENTITY *`:

```c
typedef enum { scalar, table, vector, matrix, function,
               undefined_class, NUM_CLASS } CLASS;
```

### Scalar Types

```c
typedef enum { integer, real, complex, character,
               undefined_type, NUM_TYPE } TYPE;
```

### The ENTITY Structure

```c
struct entity {
    int ref_count;   /* Reference count for memory management */
    CLASS class;     /* scalar, vector, matrix, table, function */
};
```

All entity types (SCALAR, VECTOR, MATRIX, TABLE, FUNCTION) begin with
this header, so `ENTITY *` can safely point to any of them.

## Writing a Builtin Function

### Function Signature

A builtin function receives its arguments as `ENTITY *` pointers and
returns an `ENTITY *`:

**Fixed argument count:**
```c
ENTITY *my_func(ENTITY *arg1, ENTITY *arg2);
```

**Variable argument count (min != max):**
```c
ENTITY *my_func(int n, ENTITY *arg1, ENTITY *arg2, ENTITY *arg3);
```
where `n` is the actual number of arguments passed.  Unused args
beyond `n` are `NULL`.

**Variadic (max > 5):**
```c
ENTITY *my_func(int n, ENTITY **args);
```

### Memory Management Rules

1. **Ownership transfer**: Arguments are *given* to your function.
   You must either consume (use/modify) them or delete them.

2. **Use `EAT(p)`** to consume a reference:
   ```c
   ENTITY *result = some_operation(EAT(arg));
   /* arg is now consumed; don't use or delete it */
   ```

3. **Delete unused args**: If you don't use an argument, call
   `delete_entity(arg)`.

4. **Return a new entity** with `ref_count == 1`.

### Creating Return Values

```c
/* Scalars */
ENTITY *int_to_scalar(int i);
ENTITY *real_to_scalar(REAL r);
ENTITY *complex_to_scalar(COMPLEX c);
ENTITY *char_to_scalar(char *s);   /* s must be malloc'd */

/* Vectors */
ENTITY *form_vector(int ne, TYPE type, DENSITY density);
ENTITY *gift_wrap_vector(int ne, TYPE type, void *data);

/* Tables */
ENTITY *make_table(void);
ENTITY *replace_in_table(TABLE *t, ENTITY *val, char *name);

/* Return NULL for "no value" */
```

### Working with Arguments

```c
/* Convert entity to scalar (any class -> scalar) */
ENTITY *scalar_entity(ENTITY *p);

/* Convert entity to vector */
ENTITY *vector_entity(ENTITY *ip);

/* Type casting */
ENTITY *cast_scalar(SCALAR *s, TYPE target_type);
ENTITY *cast_vector(VECTOR *v, TYPE target_type);

/* Extract values */
int entity_to_int(ENTITY *e);        /* Consumes the entity */
char *entity_to_string(ENTITY *e);   /* Consumes; returns malloc'd string */

/* Access scalar fields */
((SCALAR *)p)->v.integer
((SCALAR *)p)->v.real
((SCALAR *)p)->v.complex.real
((SCALAR *)p)->v.complex.imag
((SCALAR *)p)->v.character

/* Access vector fields */
((VECTOR *)v)->ne             /* number of elements */
((VECTOR *)v)->type           /* integer, real, complex, character */
((VECTOR *)v)->a.integer[i]   /* element access */
((VECTOR *)v)->a.real[i]
((VECTOR *)v)->a.complex[i]
```

### Error Handling

Use the exception mechanism:

```c
#include "exception.h"

WITH_HANDLING {
    /* ... code that might fail ... */
    if (something_bad) {
        fail("Error message: %s", details);
        raise_exception();
    }
}
ON_EXCEPTION {
    /* cleanup code */
    delete_entity(arg);
}
END_EXCEPTION;
```

### Example: A Complete Builtin

```c
/* myfunc.c -- Example Algae builtin */
#include "algae.h"
#include "entity.h"
#include "scalar.h"
#include "vector.h"
#include "cast.h"

/*
 * double_it(x) -- Multiply each element of x by 2.
 */
ENTITY *
double_it(ENTITY *p)
{
    VECTOR *v;
    int i;

    /* Convert to vector of reals */
    v = (VECTOR *) cast_vector(
        (VECTOR *) vector_entity(EAT(p)), real);

    /* Get a mutable copy */
    v = (VECTOR *) dup_vector(EAT(v));

    /* Double each element */
    for (i = 0; i < v->ne; i++)
        v->a.real[i] *= 2.0;

    return ENT(v);
}
```

Compile as a shared object:
```sh
gcc -shared -fPIC -I/path/to/algae/src -o myfunc.so myfunc.c
```

Load in Algae:
```
builtin("./myfunc.so", "double_it")
double_it(1:5)
```

## Registration Table Format

When adding a builtin to the compiled-in table (`builtin.h`):

```c
{"name", {min_args, max_args, arg_flags, BI_CAST(function_ptr)}}
```

- `min_args`, `max_args`: Argument count range
- `arg_flags`: Bitmask where bit `i` = 1 means arg `i` may be NULL
  (e.g., `07` = octal, args 0,1,2 may be NULL)
- When `min_args == max_args`, dispatch is by fixed-arg casting
- When `min_args != max_args` and `max_args <= 5`, dispatch passes
  `(int n, ENTITY *a0, ..., ENTITY *aN)`
- When `max_args > 5`, dispatch passes `(int n, ENTITY **args)`

## Header Organization

The headers are organized by entity type and operation:

- **Structure headers** (`*_st.h`): Define the C struct layout
- **Operation headers** (`*.h`): Declare functions for that type
- **`builtin.h`**: Central registration of all builtins
- **`algae.h`**: Global configuration, typedefs, and includes
- **`entity.h`**: Base entity framework (CLASS, TYPE, etc.)

For external plugins, include `algae.h` and the specific headers
needed for your types.  A minimal set is:

```c
#include "algae.h"
#include "entity.h"
#include "scalar.h"
```

## Help System Integration

After loading your builtin, you can document it:

```
my_fn = builtin("./myfunc.so", "double_it")
my_fn = docstring(my_fn, "double_it(x) -- Multiply each element by 2.")
help("double_it")
```
