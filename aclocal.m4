
dnl  Custom autoconf macros.
dnl  -----------------------

dnl LOCAL_CHECK_CC_PROTOS(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_CHECK_CC_PROTOS,
[cat > conftest.$ac_ext <<EOF
dnl This sometimes fails to find confdefs.h, for some reason.
dnl [#]line __oline__ "[$]0"
[#]line __oline__ "configure"
#include "confdefs.h"
int main(int argc, char *argv[]);
int t(void);
int main(int argc, char *argv[]) { return t(); }
int t(void) { return 0; }
EOF
if eval $ac_compile; then
  ifelse([$1], , :, [rm -rf conftest*
  $1])
ifelse([$2], , , [else
  rm -rf conftest*
  $2
])dnl
fi
rm -f conftest*]
)

dnl LOCAL_TRY_COMPILE_F77(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_TRY_COMPILE_F77,
[cat > conftest.f <<EOF
      SUBROUTINE T(I)
      INTEGER I
      I=I**I
      RETURN
      END
EOF
if eval $f77_compile; then
  ifelse([$1], , :, [rm -rf conftest*
  $1])
ifelse([$2], , , [else
  rm -rf conftest*
  $2
])dnl
fi
rm -f conftest*]
)

dnl LOCAL_TRY_MAKE_F77(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_TRY_MAKE_F77,
[cat > conftest.f <<EOF
      SUBROUTINE T(I)
      INTEGER I
      I=I**I
      RETURN
      END
EOF
cat > conftest.mak <<EOF
FC = \$(F77)
conftest.o: conftest.f
EOF
if eval $f77_make; then
  ifelse([$1], , :, [rm -rf conftest*
  $1])
ifelse([$2], , , [else
  rm -rf conftest*
  $2
])dnl
fi
rm -f conftest*]
)

dnl LOCAL_TRY_LINK_F77(ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_TRY_LINK_F77,
[cat > conftest.f <<EOF
      DOUBLE PRECISION X
      X=ERF(2.0)
      WRITE(*,*) X
      STOP
      END
EOF
if eval $f77_link; then
  ifelse([$1], , :, [rm -rf conftest*
  $1])
ifelse([$2], , , [else
  rm -rf conftest*
  $2
])dnl
fi
rm -f conftest*]
)

dnl LOCAL_C_FROM_FORTRAN(C-FUNC, F-FUNC,
dnl                     ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_C_FROM_FORTRAN,
[cat > conftest.c <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
int [$1]() { return 0; }
EOF
cat > conftestf.f <<EOF
      INTEGER I
      I=[$2]()
      WRITE(*,*) I
      STOP
      END
EOF
if eval $f77_c_link; then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
ifelse([$4], , , [else
  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*]
)

dnl LOCAL_FORTRAN_FROM_C(F-FUNC, C-FUNC,
dnl                     ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND])
AC_DEFUN(LOCAL_FORTRAN_FROM_C,
[cat > conftest.c <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
int main()
{
  int i = 2;
  [$2](&i);
  return i;
}
EOF
cat > conftestf.f <<EOF
      SUBROUTINE [$1](I)
      INTEGER I
      I=I**CMPLX(I,I)
      RETURN
      END
EOF
if eval $c_f77_link; then
  ifelse([$3], , :, [rm -rf conftest*
  $3])
ifelse([$4], , , [else
  rm -rf conftest*
  $4
])dnl
fi
rm -f conftest*]
)

AC_DEFUN(LOCAL_SYS_RESTARTABLE_SYSCALLS,
[AC_CACHE_CHECK(for restartable system calls, ac_cv_sys_restartable_syscalls,
[AC_TRY_RUN(
[/* Exit 0 (true) if wait returns something other than -1,
   i.e. the pid of the child, which means that wait was restarted
   after getting the signal.  */
#include <sys/types.h>
#include <signal.h>
#if HAVE_SIGACTION
#define SIGNAL(sig, handler)	{ \
				  struct sigaction act; \
				  act.sa_handler = handler; \
				  sigemptyset (&act.sa_mask); \
				  act.sa_flags = SA_RESTART; \
				  sigaction (sig, &act, NULL); \
				}
#else
#if HAVE_SIGSET
#define SIGNAL(sig, handler)	sigset (sig, handler)
#else
#define SIGNAL(sig, handler)	signal (sig, handler)
#endif
#endif
ucatch (isig) { }
main () {
  int i = fork (), status;
  if (i == 0) { sleep (3); kill (getppid (), SIGINT); sleep (3); exit (0); }
  SIGNAL (SIGINT, ucatch);
  status = wait(&i);
  if (status == -1) wait(&i);
  exit (status == -1);
}
], ac_cv_sys_restartable_syscalls=yes, ac_cv_sys_restartable_syscalls=no)])
if test $ac_cv_sys_restartable_syscalls = yes; then
  AC_DEFINE(HAVE_RESTARTABLE_SYSCALLS)
fi
])

AC_DEFUN(LOCAL_PERSISTENT_SIG_DISP,
[AC_CACHE_CHECK(for persistent signal disposition,
                ac_cv_persistent_sig_disp,
[AC_TRY_RUN([
#include <sys/types.h>
#include <signal.h>
#if HAVE_SIGACTION
#define SIGNAL(sig, handler)	{ \
				  struct sigaction act; \
				  act.sa_handler = handler; \
				  sigemptyset (&act.sa_mask); \
				  act.sa_flags = SA_RESTART; \
				  sigaction (sig, &act, NULL); \
				}
#else
#if HAVE_SIGSET
#define SIGNAL(sig, handler)	sigset (sig, handler)
#else
#define SIGNAL(sig, handler)	signal (sig, handler)
#endif
#endif
int count;
ucatch (isig) {count++;}
main ()
{
  int i = fork ();
  if (!i)
    {
      sleep (3);
      kill (getppid (), SIGINT);
      sleep (1);
      kill (getppid (), SIGINT);
      sleep (3);
      exit (0);
    }
  else
    {
      SIGNAL (SIGINT, ucatch);
      while (wait(&i) == -1) ;
      exit (count != 2);
    }
}
], ac_cv_persistent_sig_disp=yes,
[ac_cv_persistent_sig_disp=no
case "$host" in *-hpux* ) sleep 4 ;; esac])])
if test $ac_cv_persistent_sig_disp = yes; then
  AC_DEFINE(HAVE_RESTARTABLE_SYSCALLS)
fi
])

AC_DEFUN(LOCAL_C_SIGNED,
[AC_CACHE_CHECK(for working signed,
                ac_cv_c_signed,
[AC_TRY_COMPILE(,[signed char *x;], ac_cv_c_signed=yes, ac_cv_c_signed=no)])
if test $ac_cv_c_signed = no; then
  AC_DEFINE(signed, )
fi
])

dnl undefine VARIABLE
dnl LOCAL_UNDEFINE(VARIABLE)
define(LOCAL_UNDEFINE,
[cat >> confdefs.h <<\EOF
[#undef] $1
EOF
])

dnl  May have to look in several places for termcap functions.

AC_DEFUN(LOCAL_CHECK_LIB_TERMCAP,
[
if test "X$algae_cv_termcap_lib" = "X"; then
_algae_needmsg=yes
else
AC_MSG_CHECKING(which library has the termcap functions)
_algae_needmsg=
fi
AC_CACHE_VAL(algae_cv_termcap_lib,
[AC_CHECK_LIB(termcap, tgetent, algae_cv_termcap_lib=termcap,
    [AC_CHECK_LIB(curses, tgetent, algae_cv_termcap_lib=curses,
	[AC_CHECK_LIB(ncurses, tgetent, algae_cv_termcap_lib=ncurses,
	    algae_cv_termcap_lib=no)])])])
if test "X$_algae_needmsg" = "Xyes"; then
  AC_MSG_CHECKING(which library has the termcap functions)
fi
if test $algae_cv_termcap_lib = no; then
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT(using $algae_cv_termcap_lib)
  LIBS="-l$algae_cv_termcap_lib $LIBS"
fi
])
