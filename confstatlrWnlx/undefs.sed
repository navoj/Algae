/^[ 	]*#[ 	]*undef/!b
t clr
: clr
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_UCONTEXT_H$,\1#\2define\3HAVE_UCONTEXT_H 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_RINT_DECL$,\1#\2define\3HAVE_RINT_DECL 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_DRAND48_DECL$,\1#\2define\3HAVE_DRAND48_DECL 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STRCHR$,\1#\2define\3HAVE_STRCHR 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_MEMCPY$,\1#\2define\3HAVE_MEMCPY 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STRERROR$,\1#\2define\3HAVE_STRERROR 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_STRTOD$,\1#\2define\3HAVE_STRTOD 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_DIV$,\1#\2define\3HAVE_DIV 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SETITIMER$,\1#\2define\3HAVE_SETITIMER 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_MTRACE$,\1#\2define\3HAVE_MTRACE 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_ABS$,\1#\2define\3HAVE_ABS 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_RINT$,\1#\2define\3HAVE_RINT 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_RANDOM$,\1#\2define\3HAVE_RANDOM 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_DRAND48$,\1#\2define\3HAVE_DRAND48 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SIGSET$,\1#\2define\3HAVE_SIGSET 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SIGACTION$,\1#\2define\3HAVE_SIGACTION 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_FINITE$,\1#\2define\3HAVE_FINITE 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_ACOSH$,\1#\2define\3HAVE_ACOSH 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_ASINH$,\1#\2define\3HAVE_ASINH 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_ATANH$,\1#\2define\3HAVE_ATANH 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SNPRINTF$,\1#\2define\3HAVE_SNPRINTF 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_VSNPRINTF$,\1#\2define\3HAVE_VSNPRINTF 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_FEENABLEEXCEPT$,\1#\2define\3HAVE_FEENABLEEXCEPT 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_MATHERR$,\1#\2define\3HAVE_MATHERR 0,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_DLOPEN$,\1#\2define\3HAVE_DLOPEN 1,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)RETSIGTYPE$,\1#\2define\3RETSIGTYPE void,;t
s,^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)HAVE_SA_RESTART$,\1#\2define\3HAVE_SA_RESTART 1,;t
s,^[ 	]*#[ 	]*undef[ 	][ 	]*[a-zA-Z_][a-zA-Z_0-9]*,/* & */,
