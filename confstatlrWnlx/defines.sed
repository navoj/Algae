/^[ 	]*#[ 	]*define/!b
t clr
: clr
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_UCONTEXT_H[ 	].*$,\1#\2HAVE_UCONTEXT_H 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_RINT_DECL[ 	].*$,\1#\2HAVE_RINT_DECL 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_DRAND48_DECL[ 	].*$,\1#\2HAVE_DRAND48_DECL 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STRCHR[ 	].*$,\1#\2HAVE_STRCHR 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_MEMCPY[ 	].*$,\1#\2HAVE_MEMCPY 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STRERROR[ 	].*$,\1#\2HAVE_STRERROR 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_STRTOD[ 	].*$,\1#\2HAVE_STRTOD 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_DIV[ 	].*$,\1#\2HAVE_DIV 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SETITIMER[ 	].*$,\1#\2HAVE_SETITIMER 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_MTRACE[ 	].*$,\1#\2HAVE_MTRACE 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_ABS[ 	].*$,\1#\2HAVE_ABS 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_RINT[ 	].*$,\1#\2HAVE_RINT 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_RANDOM[ 	].*$,\1#\2HAVE_RANDOM 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_DRAND48[ 	].*$,\1#\2HAVE_DRAND48 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SIGSET[ 	].*$,\1#\2HAVE_SIGSET 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SIGACTION[ 	].*$,\1#\2HAVE_SIGACTION 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_FINITE[ 	].*$,\1#\2HAVE_FINITE 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_ACOSH[ 	].*$,\1#\2HAVE_ACOSH 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_ASINH[ 	].*$,\1#\2HAVE_ASINH 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_ATANH[ 	].*$,\1#\2HAVE_ATANH 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SNPRINTF[ 	].*$,\1#\2HAVE_SNPRINTF 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_VSNPRINTF[ 	].*$,\1#\2HAVE_VSNPRINTF 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_FEENABLEEXCEPT[ 	].*$,\1#\2HAVE_FEENABLEEXCEPT 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_MATHERR[ 	].*$,\1#\2HAVE_MATHERR 0,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_DLOPEN[ 	].*$,\1#\2HAVE_DLOPEN 1,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)RETSIGTYPE[ 	].*$,\1#\2RETSIGTYPE void,;t
s,^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)HAVE_SA_RESTART[ 	].*$,\1#\2HAVE_SA_RESTART 1,;t
