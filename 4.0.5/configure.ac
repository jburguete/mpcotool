AC_PREREQ([2.67])
AC_INIT([mpcotool], [4.0.5], [https://github.com/jburguete/mpcotool])

# Checks for operative systems
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
win=0
if test $host = "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	AC_SUBST(PREFIX, "/mingw64")
	AC_SUBST(ICON, "logo.o")
	AC_SUBST(WINCFLAGS, "-mwindows")
	AC_SUBST(WIN,"win64")
	AC_SUBST(SED, [sed])
	win=1
elif test $host = "i686-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	AC_SUBST(PREFIX, "/mingw32")
	AC_SUBST(ICON, "logo.o")
	AC_SUBST(WINCFLAGS, "-mwindows")
	AC_SUBST(WIN,"win32")
	AC_SUBST(SED, [sed])
	win=1
elif test `uname -s` = "FreeBSD"; then
	AC_MSG_NOTICE([FreeBSD])
	AC_SUBST(PREFIX, [/usr/local])
	AC_SUBST(SED, [gsed])
elif test `uname -s` = "NetBSD"; then
	AC_MSG_NOTICE([NetBSD])
	AC_SUBST(PREFIX, [/usr/pkg])
	AC_SUBST(SED, [gsed])
elif test `uname -s` = "OpenBSD"; then
	AC_MSG_NOTICE([OpenBSD])
	AC_SUBST(PREFIX, [/usr/local])
	AC_SUBST(SED, [gsed])
	openbsd=1
elif test `uname -s` = "DragonFly"; then
	AC_MSG_NOTICE([DragonFly BSD])
	AC_SUBST(PREFIX, [/usr/local])
	AC_SUBST(SED, [gsed])
elif test `uname -s` = "SunOS"; then
	AC_MSG_NOTICE([SunOS])
	AC_SUBST(gnu,[/usr/gnu/bin/])
else
	AC_SUBST(PREFIX, [/usr])
	AC_SUBST(SED, [sed])
fi
AC_MSG_NOTICE([prefix=$PREFIX])

# Checks for programs
AC_PROG_CC([mpicc egcc gcc clang])
AC_MSG_NOTICE([Compiler: $CC])
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
if test "x$win" = "x1"; then
	AC_CHECK_TOOL(WINDRES, windres)
	AC_SUBST(SO, [.dll])
	AC_SUBST(EXE, [.exe])
	AC_SUBST(WIN, [-win])
else
	AC_SUBST(SO, [.so])
fi

# Checking MPI
AC_ARG_WITH([mpi], AS_HELP_STRING([--with-mpi], [Build with MPI]))
if test "x$with_mpi" = "xyes"; then
	if test "x$CC" = "xmpicc"; then
		AC_MSG_NOTICE([checking for MPI... yes])
		AC_DEFINE([HAVE_MPI], [1])
	else
		AC_MSG_NOTICE([checking for MPI... no])
	fi
fi

# Checks for libraries
if test -f "genetic/genetic.h"; then
	AC_MSG_NOTICE([Genetic header installed])
else
	AC_MSG_ERROR([Genetic header not installed])
fi
if test "x$win" = "x0"; then
	genetic="libgenetic.so"
else
	genetic="libgenetic.dll"
fi
if test -x "$genetic"; then
	AC_MSG_NOTICE([Genetic library installed])
else
	AC_MSG_ERROR([Genetic library not installed])
fi
AC_SEARCH_LIBS([sqrt], [m])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([JSON], [json-glib-1.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])

# Checks fot GTK+ interface
AC_ARG_WITH([gtk], AS_HELP_STRING([--with-gtk], [Build with GTK+ interface]))
if test "x$with_gtk" = "xyes"; then
	PKG_CHECK_MODULES([GTK], [gtk+-3.0])
	AC_SUBST(MPCOTOOL, "mpcotool")
fi

# Checks for header files
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ALLOCA
AC_CHECK_HEADERS(
	[stdlib.h string.h unistd.h pthread.h locale.h sys/param.h libintl.h])

# Checking -march=native compiler flag
AC_ARG_WITH([native], AS_HELP_STRING([--with-native],
	[Build with native optimization]))
if test [["x$with_native" = "xyes"] -a ["x$win" = "x0"]]; then
	compiler=$CC
	CC="$CC -march=native"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [arch=1],
       		[arch=0])
	CC=$compiler
	if test "x$arch" = "x0"; then
		AC_MSG_NOTICE([checking for -march=native... no])
		AC_SUBST(ARCH, "")
	else
		AC_MSG_NOTICE([checking for -march=native... yes])
		AC_SUBST(ARCH, "-march=native")
	fi
fi

# Checking -flto compiler flag
compiler=$CC
CC="$CC -flto"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [lto=1], [lto=0])
CC=$compiler
if test "x$lto" = "x0"; then
	AC_MSG_NOTICE([checking for -flto... no])
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE([checking for -flto... yes])
	AC_SUBST(LTO, "-flto")
fi

# Checking -std compiler flag
compiler=$CC
CC="$CC -std=gnu11"
AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [std=1], [std=0])
CC=$compiler
if test "x$std" = "x0"; then
	AC_MSG_NOTICE("checking for -std=gnu11... no")
	CC="$CC -std=gnu99"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [std=1],
		[std=0])
	CC=$compiler
	if test "x$std" = "x0"; then
		AC_MSG_NOTICE("checking for -std=gnu99... no")
		AC_SUBST(STD, "")
	else
		AC_MSG_NOTICE("checking for -std=gnu99... yes")
		AC_SUBST(STD, "-std=gnu99")
	fi
else
	AC_MSG_NOTICE("checking for -std=gnu11... yes")
	AC_SUBST(STD, "-std=gnu11")
fi

# Checking PGO optimiztion
AC_ARG_WITH([pgo],
	AS_HELP_STRING([--with-pgo], [Build using PGO]))
if test "x$with_pgo" = "xyes"; then
	CC="$CC -fprofile-generate"
	AC_TRY_LINK([], [int main(int argc, char **argv){return 0;}], [pgo=1],
		[pgo=0])
	CC=$compiler
	if test "x$pgo" = "x0"; then
		AC_MSG_NOTICE("checking for PGO... no")
		AC_SUBST(PGO, "0")
	else
		AC_MSG_NOTICE("checking for PGO... yes")
		AC_SUBST(PGO, "1")
	fi
fi

# Checking static library
AC_ARG_WITH([static],
	AS_HELP_STRING([--with-static], [Build static library]))
if test "x$with_static" = "xyes"; then
	mk=Makefile-static.in
else
	mk=Makefile.in
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_CHECK_FUNCS([pow setlocale sqrt])

# Final output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile:$mk ../tests/test1/Makefile ../tests/test2/Makefile
	../tests/test3/Makefile ../tests/test4/Makefile ../tests/test5/Makefile
	../tests/testAckley/Makefile ../tests/testSphere/Makefile
	../tests/testBooth/Makefile ../tests/testRosenbrock/Makefile
	../tests/testEasom/Makefile ../tests/testBeale/Makefile])
AC_OUTPUT
