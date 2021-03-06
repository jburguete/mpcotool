#!/bin/bash
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/README.md
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/configure.ac
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/Doxyfile
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/interface.c
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" $1.$2.$3/locales/*/*/*.po
gsed -i "s/"$1"\."$2"\."$3"/"$4"\."$5"\."$6"/g" */*.tex
git mv $1.$2.$3 $4.$5.$6
ln -sf $4.$5.$6/README.md
rm mpcotool
ln -s $4.$5.$6 mpcotool
