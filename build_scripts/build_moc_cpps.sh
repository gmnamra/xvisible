#!/bin/bash

# arguments srcroot 

usage="-s srcroot -l Logfile"

while getopts ":s:l" options; do
  case $options in
    s ) SRCROOT=$OPTARG;;
    h ) echo $usage;;
    \? ) echo $usage
         exit 1;;
    * ) echo $usage
          exit 1;;
  esac
done

# Create the packages
set -e
trap "echo 'build of moc files might have failed ( like error is expected '" ERR

SCRIPTS_FOLDER="${SRCROOT}/build_scripts"
MOCPROFILE=visible_qt_moc.pro

echo "src root = ${SRCROOT}"

MOC_FOLDER="{$SRCROOT}/src/visible/moc/"
echo "moc src root = ${MOC_FOLDER}"

pushd ${SCRIPTS_FOLDER}
echo "In $PWD "
if [ -e "${MACPROFILE}" ]; 
echo " mac.pro file exists "
then

qmake visible_qt_moc.pro 
make > moc_log 2>&1

`ls moc_*.cpp` $MOC_FOLDER moc_log 2>&1

popd

echo "Out $PWD "

fi

