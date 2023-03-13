#!/usr/bin/env bash
if [ `uname -o` != 'Msys' ]; then
  ln -sf ../../jb/jb
  ln -sf ../../genetic/genetic
  so='.so'
else
  so='.dll'
fi
ln -sf jb/bin/libjb-3$so
ln -sf jb/bin/libjbm-3$so
ln -sf jb/bin/libjbjson-3$so
ln -sf jb/bin/libjbxml-3$so
ln -sf jb/bin/libjbbin-3$so
ln -sf jb/bin/libjbwin-3$so
ln -sf genetic/libgenetic$so
