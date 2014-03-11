#!/bin/bash

SCRIPTS_FOLDER="${PWD}"
MOCPROFILE=visible_qt_moc.pro

if [ ! -d "build_scripts" ];
 then SRCROOT=`dirname $SCRIPTS_FOLDER`
fi

echo "src root = ${SRCROOT}"

MOC_FOLDER="$SRCROOT/src/visible/moc/"
echo "moc src root = ${SRCROOT}"

pushd ${SCRIPTS_FOLDER}
echo "In $PWD "
if [ -e "${MACPROFILE}" ]; 
echo " mac.pro file exists "
then

output=`qmake visible_qt_moc.pro`
echo "In $PWD $output "
output=`make`
echo "make: $output "
output=cp `ls moc_*.cpp` $MOC_FOLDER
echo "cp: $output "
popd
echo "Out $PWD "

fi

