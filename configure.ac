# To compile on UNIX systems:
# aclocal
# autoconf
# automake --add-missing
# ./configure
# make

AC_PREREQ([2.69])
AC_INIT([calibrator], [0.0.15], [https://github.com/jburguete/calibrator])

# Checks for operative systems
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([System: $build])
AC_MSG_NOTICE([System: $host])
if test $host = "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	CPPFLAGS="$CPPFLAGS -I/mingw/include"
elif test $host = "i686-pc-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	CPPFLAGS="$CPPFLAGS -I/mingw/include"
elif test `uname -s` = "NetBSD"; then
	AC_MSG_NOTICE([NetBSD])
	CPPFLAGS="$CPPFLAGS -DNetBSD"
fi

# Checks for programs.
AC_PROG_CC([mpicc.openmpi mpicc.mpich2 mpicc egcc gcc clang])
AC_MSG_NOTICE([Compiler: $CC])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG

# Checks for libraries.
AC_SEARCH_LIBS([sqrt], [m])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checking MPI
if test $CC = "mpicc.openmpi"; then
	AC_SUBST(MPIDEFINE, "-DHAVE_MPI")
	AC_MSG_NOTICE([Found MPI compiler: $CC])
elif test $CC = "mpicc.mpich2"; then
	AC_SUBST(MPIDEFINE, "-DHAVE_MPI")
	AC_MSG_NOTICE([Found MPI compiler: $CC])
elif test $CC = "mpicc"; then
	AC_SUBST(MPIDEFINE, "-DHAVE_MPI")
	AC_MSG_NOTICE([Found MPI compiler: $CC])
fi

# Checking -march=native compiler flag
compiler=$CC
CC="$CC -march=native"
AC_TRY_LINK([],
	[int main(int argc, char **argv){return 0;}], [arch=1], [arch=0])
CC=$compiler
if test $arch = 0; then
	AC_MSG_NOTICE([checking for -march=native... no])
	AC_SUBST(ARCH, "")
else
	AC_MSG_NOTICE([checking for -march=native... yes])
	AC_SUBST(ARCH, "-march=native")
fi

# Checking -flto compiler flag
compiler=$CC
CC="$CC -flto"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [lto=1], [lto=0])
CC=$compiler
if test $lto = 0; then
	AC_MSG_NOTICE([checking for -flto... no])
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE([checking for -flto... yes])
	AC_SUBST(LTO, "-flto")
fi

# Checking GAUL library
compiler=$CC
CC="$CC -lm -lgaul -lgaul_util -fopenmp"
AC_TRY_LINK([#include "gaul.h"], [int main(int argc, char **argv){return 0;}],
	[gaul=1], [gaul=0])
CC=$compiler
if test $gaul = 0; then
	AC_MSG_NOTICE([checking for GAUL... no])
else
	AC_MSG_NOTICE([checking for GAUL... yes])
	AC_SUBST(GAULDEFINE, "-DHAVE_GAUL")
	AC_SUBST(GAUL_LIBS, "-lm -lgaul -lgaul_util -fopenmp")
fi

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([strtoul])

# Final output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
