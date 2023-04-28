#!/usr/bin/env bash
if [ `uname -o` != 'Msys' ]; then
  ln -sf ../../jb/jb
  ln -sf ../../genetic/genetic
  for i in 2 3 4; do
    ln -sf ../../../genetic/genetic ../tests/test$i
  done
  if [ `uname -s` != 'Darwin' ]; then
	  bin=bin/
  fi
  so='.so'
else
  bin=bin/
  so='.dll'
fi
ln -sf jb/${bin}libjb-3$so
ln -sf jb/${bin}libjbm-3$so
ln -sf jb/${bin}libjbjson-3$so
ln -sf jb/${bin}libjbxml-3$so
ln -sf jb/${bin}libjbbin-3$so
ln -sf jb/${bin}libjbwin-3$so
ln -sf genetic/libgenetic$so
for i in 2 3 4; do
  ln -sf genetic/libgenetic$so ../tests/test$i
done
