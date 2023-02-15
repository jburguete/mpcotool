# Header
AC_PREREQ([2.69])
AC_INIT([mpcotool], [4.6.1], [https://github.com/jburguete/mpcotool])

# Checks for operative systems
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
win=0
native=1
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
	native=0
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
if test $win = 1; then
	native=0
	so=.dll
	AC_CHECK_TOOL(WINDRES, windres)
	AC_SUBST(EXE, [.exe])
	AC_SUBST(WIN, [-win])
else
	so=.so
fi
AC_SUBST(SO, [$so])
AC_MSG_NOTICE([prefix=$PREFIX])

# Checks for programs
AC_PROG_CC([mpicc gcc-12 gcc12 gcc-11 gcc11 gcc-10 gcc10 egcc gcc clang])
AC_MSG_NOTICE([Compiler: $CC])
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
if test "`command -v gmake`"; then
	AC_SUBST(MAKE, [gmake])
else
	AC_SUBST(MAKE, [make])
fi
if test $CC = "clang"; then
	AC_CHECK_PROGS([LLVM_PROFDATA], [llvm-profdata llvm-profdata-15], 0)
	if test $LLVM_PROFDATA = 0; then
		AC_MSG_ERROR([No llvm-profdata])
	fi
	AC_MSG_NOTICE([llvm-profdata: $LLVM_PROFDATA])
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

# Checks for compiler flags
compiler=$CC

# Checking -march=native compiler flag
arch=0
if test $native = 1; then
	AC_ARG_WITH([native], AS_HELP_STRING([--with-native],
		[Build using -march=native compiler flag]))
	if test "x$with_native" = "xyes"; then
		CC="$CC -march=native"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [arch=1], [])
		CC=$compiler
	fi
fi
if test $arch = 0; then
	AC_MSG_NOTICE([checking for -march=native... no])
else
	AC_MSG_NOTICE([checking for -march=native... yes])
	AC_SUBST(ARCH, "-march=native")
fi

# Checking -flto compiler flag
CC="$CC -flto"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [lto=1], [lto=0])
CC=$compiler
if test $lto = 0; then
	AC_MSG_NOTICE([checking for -flto... no])
else
	AC_MSG_NOTICE([checking for -flto... yes])
	AC_SUBST(LTO, "-flto")
fi

# Checking PGO optimiztion
AC_ARG_WITH([pgo], AS_HELP_STRING([--with-pgo], [Build using PGO]))
pgo=0
if test "x$with_pgo" = "xyes"; then
	CC="$CC -fprofile-generate"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [pgo=1])
	CC=$compiler
	if test "$pgo" = "0"; then
		CC="$CC -fprofile-instr-generate"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [pgo=1])
		CC=$compiler
	fi
fi
if test "$pgo" = 0; then
		AC_MSG_NOTICE("checking for PGO... no")
else
		AC_MSG_NOTICE("checking for PGO... yes")
fi
AC_SUBST(PGO, [$pgo])

# Set precision
AC_ARG_WITH([precision], AS_HELP_STRING([--with-precision],
	[Build with precision degree (1-5), default=2]))
if test "x$with_precision" = "x1"; then
	precision=1
elif test "x$with_precision" = "x3"; then
	precision=3
elif test "x$with_precision" = "x4"; then
	precision=4
elif test "x$with_precision" = "x5"; then
	precision=5
else
	precision=2
fi
AC_SUBST(PRECISION, "-DJBM_PRECISION=$precision")
AC_SUBST(SUFFIX, [-$precision])

# Checks for header files
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ALLOCA
AC_CHECK_HEADERS(
	[stdlib.h string.h unistd.h pthread.h locale.h sys/param.h libintl.h])
if test -f "genetic/genetic.h"; then
	AC_MSG_NOTICE([Genetic header installed])
else
	AC_MSG_ERROR([Genetic header not installed])
fi
if test -f "jb/src/jb_def.h"; then
	AC_MSG_NOTICE([JB header installed])
else
	AC_MSG_ERROR([JB header not installed])
fi

# Checks for libraries
genetic=libgenetic$so
if test -x "$genetic"; then
	AC_MSG_NOTICE([Genetic library installed])
else
	AC_MSG_ERROR([Genetic library not installed])
fi
libjb=libjb-$precision$so
if test -x "$libjb"; then
	AC_MSG_NOTICE([$libjb: OK])
else
	AC_MSG_ERROR([$libjb: not installed])
fi
libjbm=libjbm-$precision$so
if test -x "$libjbm"; then
	AC_MSG_NOTICE([$libjbm: OK])
else
	AC_MSG_ERROR([$libjbm: not installed])
fi
libjbjson=libjbjson-$precision$so
if test -x "$libjbjson"; then
	AC_MSG_NOTICE([$libjbjson: OK])
else
	AC_MSG_ERROR([$libjbjson: not installed])
fi
libjbxml=libjbxml-$precision$so
if test -x "$libjbxml"; then
	AC_MSG_NOTICE([$libjbxml: OK])
else
	AC_MSG_ERROR([$libjbxml: not installed])
fi
libjbbin=libjbbin-$precision$so
if test -x "$libjbbin"; then
	AC_MSG_NOTICE([$libjbbin: OK])
else
	AC_MSG_ERROR([$libjbbin: not installed])
fi
AC_SEARCH_LIBS([sqrt], [m])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([JSON], [json-glib-1.0])
PKG_CHECK_MODULES([GTOP], [libgtop-2.0], AC_DEFINE([HAVE_GTOP], [1]),
	AC_MSG_NOTICE([No libgtop]))
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])

# Checks for GTK+ interface
AC_ARG_WITH([gtk], AS_HELP_STRING([--with-gtk],
	[Build using a version of GTK (3 (--with-gtk=3) or 4 (--with-gtk=4))]))
gtk4=0
gtk3=0
gtk=0
if test "x$with_gtk" = "x4"; then
	PKG_CHECK_MODULES([GTK], [gtk4], [gtk4=1], [gtk4=0])
	gtk3=1
elif test "x$with_gtk" = "x3"; then
	gtk3=1
fi
if test $gtk4 = 1; then
	gtk=1
	AC_SUBST(GTK4, "-DGTK4=1")
elif test $gtk3 = 1; then
	PKG_CHECK_MODULES([GTK], [gtk+-3.0],
		[AC_MSG_NOTICE([With GTK])],
		[AC_MSG_NOTICE([Without GTK])])
	gtk=1
	AC_SUBST(GTK4, "-DGTK4=0")
fi
if test $gtk = 1; then
	libjbwin=libjbwin-$precision$so
	if test -x "$libjbwin"; then
		AC_MSG_NOTICE([$libjbwin: OK])
	else
		AC_MSG_ERROR([$libjbwin: not installed])
	fi
	PKG_CHECK_MODULES([FREETYPE], [freetype2])
	AC_SUBST(MPCOTOOL, 1)
else
	AC_SUBST(MPCOTOOL, 0)
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_CHECK_FUNCS([pow setlocale sqrt])

# Final output
mk=Makefile.in
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile:$mk ../tests/test1/Makefile ../tests/test2/Makefile
	../tests/test3/Makefile ../tests/test4/Makefile ../tests/test5/Makefile
	../tests/testAckley/Makefile ../tests/testSphere/Makefile
	../tests/testBooth/Makefile ../tests/testRosenbrock/Makefile
	../tests/testEasom/Makefile ../tests/testBeale/Makefile])
AC_OUTPUT
