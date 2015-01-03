#!/bin/bash 


VERBOSE=n
function usage { echo "Usage: $0 [-r target ] [-s old  location <string>] [-g new location <string>]" 1>&2; exit 1; }

elementContainsElement ()
 {
  local e
  for e in "${@:2}"; do  [[ "$e" == *"$1"* ]] && return 0; done
  return 1
}


while getopts ":r:s:g:v:h" options; do
  case $options in
    r ) ROOT=$OPTARG;;
    s ) OLD=$OPTARG;;
    g ) NEW=$OPTARG;;
    v ) VERBOSE=$OPTARG;;
    h ) usage;;
    * ) usage;;
  esac
done

if [ $VERBOSE = y ];
 then
    echo "r: $ROOT"
    echo "s: $OLD"
    echo "g: $NEW"
    echo "v: $VERBOSE"
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
        echo "install_name_tool -change ${oout}  ${NEW}/$dname ${ROOT}"
    fi

    ## Change on the exercutable
    install_name_tool -change ${oout}  ${NEW}$dname  ${ROOT}

    ## If there is a Frameworks dir on top, fix it there

  done;

