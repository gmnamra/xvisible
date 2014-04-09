#!/bin/bash 


VERBOSE=n
FLIB=.

function usage { echo "Usage: $0 [-r target ] [ [-f framework or lib name <string>] or [-s old rpath<string>] ] [-g new rpath <string>]" 1>&2; exit 1; }

elementContainsElement ()
 {
  local e
  for e in "${@:2}"; do  [[ "$e" == *"$1"* ]] && return 0; done
  return 1
}


while getopts ":r:s:f:g:v:h" options; do
echo $options
  case $options in
    r ) ROOT=$OPTARG;;
    s ) OLD=$OPTARG;;
    f ) FLIB=$OPTARG;;
    g ) NEW=$OPTARG;;
    v ) VERBOSE=$OPTARG;;
    h ) usage;;
    * ) usage;;
  esac
done

if [ $VERBOSE = y ];
 then
    echo $ROOT
    echo $OLD
    echo $NEW
    echo $FLIB
fi

if [ ! $FLIB = "." ];
then
# pick using our token, remove name and offset, remove anything between paranthesis
  oouts=("`otool -l ${ROOT} | grep $FLIB | grep ')' | sed -e 's/name//g' | sed -e 's/offset//g' |  sed -e 's/([^()]*)//g'`");
  if [[ $VERBOSE = y  ]];
  then 
      echo   "otool -l ${ROOT} | grep $FLIB | grep ')' | sed -e 's/name//g' | sed -e 's/offset//g' |  sed -e 's/([^()]*)//g'"
      echo ${#oouts[@]};
  fi

  
  for oout in $oouts
  do 
      pieces=(${oout//// })                 # split otool output line
      dname=${pieces[${#pieces[@]} - 1]}    # get the framework name
      echo "${pieces[@]}"
      echo "${dname} -----  ${FLIB}" 
      echo "install_name_tool -change ${oout} ${NEW} $ROOT"
      install_name_tool -change ${oout} ${NEW} $ROOT
  done

  exit 1;  
fi


oouts=("`otool -L ${ROOT} | grep ${OLD} | sed -e 's/([^()]*)//g'`");

for oout in $oouts
  do 
    pieces=(${oout//// })                 # split otool output line
    dname=${pieces[${#pieces[@]} - 1]}    # get the framework name
    if [ $VERBOSE = y ];
    then
        echo $dname
        echo $oout
        echo "install_name_tool -change ${oout}  ${NEW} ${ROOT}"
    fi
    install_name_tool -change ${oout}  ${NEW}  ${ROOT}

  done;
