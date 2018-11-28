#!/bin/bash
sed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/{README.md,configure.ac,Makefile.in,Doxyfile,interface.c,locales/*/*/*.po} */*.tex
git mv $1.$2.$3 $4.$5.$6
ln -sf $4.$5.$6/README.md
