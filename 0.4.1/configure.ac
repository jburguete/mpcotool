# To work in:
#	- Debian 7.7 (Linux, kFreeBSD or Hurd)
#	- FreeBSD 10.0
#	- Microsoft Windows XP 32 bits
#	- Microsoft Windows 7 32 bits
#	- NetBSD 6.1.5
#	- DragonFly BSD 3.8.2
#
# aclocal
# autoconf
# automake --add-missing
# ./configure
# make

# To work in Microsoft Windows (64 bits):
#
# aclocal
# autoconf
# automake --add-missing
# ./configure --host=x86_64-w64-mingw32
# make

# To work in OpenBSD 5.6:
#
# export AUTOCONF_VERSION=2.69 AUTOMAKE_VERSION=1.14
# aclocal
# autoconf
# automake --add-missing
# ./configure
# make

AC_PREREQ([2.69])
AC_INIT([calibrator], [0.4.1], [https://github.com/jburguete/calibrator])

# Checks for operative systems
openbsd=0
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
if test $host = "x86_64-w64-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 64 bits])
	CPPFLAGS="$CPPFLAGS -I/mingw/include"
	AC_SUBST(PREFIX, "/mingw")
elif test $host = "i686-pc-mingw32"; then
	AC_MSG_NOTICE([Microsoft Windows 32 bits])
	CPPFLAGS="$CPPFLAGS -I/mingw/include"
	AC_SUBST(PREFIX, "/mingw")
elif test `uname -s` = "NetBSD"; then
	AC_MSG_NOTICE([NetBSD])
	AC_SUBST(PREFIX,"/usr/pkg")
elif test `uname -s` = "OpenBSD"; then
	AC_MSG_NOTICE([OpenBSD])
	AC_SUBST(PREFIX,"/usr/local")
	openbsd=1
elif test `uname -s` = "DragonFly"; then
	AC_MSG_NOTICE([DragonFly BSD])
	AC_SUBST(PREFIX,"/usr/local")
else
	AC_SUBST(PREFIX, "/usr")
fi
AC_MSG_NOTICE([prefix=$PREFIX])

# Checks for programs
if test $openbsd = 1; then
	AC_MSG_NOTICE([OpenBSD])
	# On OpenBSD mpicc uses very old GCC compiler
	# then is better: egcc `mpicc -showme:compile` `mpicc -showme:link`
	if test -x "$PREFIX/bin/mpicc"; then
		OPENBSD_FLAGS="`mpicc -showme:compile` `mpicc -showme:link`"
	else
		OPENBSD_FLAGS=""
	fi
	AC_MSG_NOTICE([MPI flags: $OPENBSD_FLAGS])
	# Unable to use AC_PROG_CC([egcc gcc clang]) on conditional statements
	# Trying to recover
	if test -x "$PREFIX/bin/egcc"; then
		CC="egcc"
	elif test -x "$PREFIX/bin/gcc"; then
		CC="gcc"
	elif test -x "$PREFIX/bin/clang"; then
		CC="clang"
	elif test -x "/usr/bin/gcc"; then
		CC="gcc"
	else
		AC_MSG_ERROR([No C compiler])
	fi
	CC="$CC $OPENBSD_FLAGS"
	AC_MSG_NOTICE([Compiler: $CC])
else
	# Unable to use AC_PROG_CC([mpicc gcc clang]) on conditional statements
	# Trying to recover
	if test -x "$PREFIX/bin/mpicc"; then
		CC="mpicc"
	elif test -x "$PREFIX/bin/gcc"; then
		CC="gcc"
	elif test -x "$PREFIX/bin/clang"; then
		CC="clang"
	elif test -x "/usr/bin/gcc"; then
		CC="gcc"
	else
		AC_MSG_ERROR([No C compiler])
	fi
fi
AC_MSG_NOTICE([Compiler: $CC])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG

# Checks for libraries
AC_SEARCH_LIBS([sqrt], [m])
PKG_CHECK_MODULES([GSL], [gsl])
PKG_CHECK_MODULES([XML], [libxml-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])

# Checks for header files
AC_CHECK_HEADERS([stdlib.h string.h unistd.h pthread.h])

# Checking MPI
if test -x "$PREFIX/bin/mpicc"; then
	MPI_DEFINE="-DHAVE_MPI=1"
else
	MPI_DEFINE=""
fi
AC_SUBST(MPI_DEFINE, $MPI_DEFINE)

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

# Checks for library functions
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([strtoul])

# Final output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
