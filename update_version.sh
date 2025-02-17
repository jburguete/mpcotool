#!/bin/bash
sed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/{configure.ac,Doxyfile,interface.c,locales/*/*/*.po} README* */*.tex
git mv $1.$2.$3 $4.$5.$6
rm mpcotool
ln -s $4.$5.$6 mpcotool
