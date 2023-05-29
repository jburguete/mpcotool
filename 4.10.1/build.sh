#!/usr/bin/env bash
if [[ `uname -s` = "OpenBSD" ]]; then
	export AUTOCONF_VERSION=2.71
	export AUTOMAKE_VERSION=1.16
elif [[ `uname -s` = "SunOS" ]]; then
	export PATH=/usr/gcc/11/bin:$PATH
fi
if [[ $# != 6 ]]; then
	echo "The syntax is: ./build.sh A B C D E F"
	echo "A: 1 on MPI"
	echo "B: 1 on native"
	echo "C: 1 on PGO"
	echo "D: JB precision (1-5)"
	echo "E: 3 on GTK3, 4 on GTK4, 0 without GUI"
	echo "F: 1 on strip"
	exit 1
fi
if [[ $1 = 1 ]]; then
	a1="--with-mpi"
fi
if [[ $2 = 1 ]]; then
	a2="--with-native"
fi
if [[ $3 = 1 ]]; then
	a3="--with-pgo"
fi
if [[ $4 = 1 || $4 = 2 || $4 = 3 || $4 = 4 || $4 = 5 ]]; then
	a4="--with-precision=$4"
else
	echo "Unknown option"
	exit 2
fi
if [[ $5 = 3 || $5 = 4 ]]; then
	a5="--with-gtk=$5"
elif [[ $5 != 0 ]]; then
	echo "Unknown option"
	exit 2
fi
if [[ $6 = 1 ]]; then
	a6="strip"
fi
aclocal
autoconf
automake --add-missing
./configure $a1 $a2 $a3 $a4 $a5
if test -x "`command -v gmake`"; then
	gmake $a6
else
	make $a6
fi
