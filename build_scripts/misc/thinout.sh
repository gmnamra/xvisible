#!/bin/bash

FATLIB="$1"
echo "Fat lib dir: {$FATLIB}"

for file in $(find -f $FATLIB)
do
  if [[ $(file $file | grep 'shared library') ]]
  then
    echo lipoing: $file;
    lipo $file -thin i386 -output $file;
  fi
done
