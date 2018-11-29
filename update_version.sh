#!/bin/bash
sed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/{README.md,configure.ac,Doxyfile,interface.c,locales/*/*/*.po} */*.tex
git mv $1.$2.$3 $4.$5.$6
ln -sf $4.$5.$6/README.md
rm mpcotool
ln -s $4.$5.$6 mpcotool
