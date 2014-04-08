#!/bin/bash 



# arguments srcroot 

usage() { echo "Usage: $0 [-r root . is ok ] [-t <string>] [-v if present just print and not execute" 1>&2; exit 1; }

while getopts ":r:t:v:" options; do
echo $options
  case $options in
    r ) ROOT=$OPTARG;;
    t ) TOKEN=$OPTARG;;
    v ) VERBOSE=$OPTARG;;
    h ) echo $usage;;
    \? ) echo $usage
         exit 1;;
    * ) echo $usage
          exit 1;;
  esac
done


if [[ $VERBOSE == "verbose" ]];
then 
    echo $ROOT
    echo $TOKEN
    echo $VERBOSE
fi

CURRENT=Versions/Current
fixed=0;
for frm in $(find "$ROOT" -name '*.framework' );
 do 
  fname=${frm/%.framework/};     # 
  jname=${fname#*.\/};           # get the framework executable
  fmdir="$frm/${CURRENT}";       # current version
  pushd ${fmdir}                 # goto current version
  counter=0;

# pick using our token, remove name and offset, remove anything between paranthesis
  if [[ $VERBOSE == "verbose" ]];
  then 
      echo $PWD;
      echo   "otool -l ${fname} | grep $TOKEN | grep ')' | sed -e 's/name//g' | sed -e 's/offset//g' |  sed -e 's/([^()]*)//g'"
  else
      oouts=("`otool -l ${fname} | grep $TOKEN | grep ')' | sed -e 's/name//g' | sed -e 's/offset//g' |  sed -e 's/([^()]*)//g'`");
      echo ${#oouts[@]};
  fi
  
  for oout in $oouts
  do 
      counter+=1
      pieces=(${oout//// })                 # split otool output line
      dname=${pieces[${#pieces[@]} - 1]}    # get the framework name
      if [[ $VERBOSE == "verbose" ]];
          then
          echo "${dname} -----  ${jname}" 
      fi

      if [[ $dname == $jname ]];
      then 
          if [[ $VERBOSE == "verbose" ]]; then
              echo  "install_name_tool -id $jname.framework/${CURRENT}/${jname} $PWD/$jname"
              ( ( fixed++ ) );
          else
              install_name_tool -id $jname.framework/${CURRENT}/${jname} $PWD/$jname
          fi
      else
          if [[ $VERBOSE == "verbose" ]]; then
              echo "install_name_tool -change ${oout}  $dname.framework/${CURRENT}/${dname} $PWD/$jname"
              ( ( fixed++ ) );
          else
              install_name_tool -change ${oout} $dname.framework/${CURRENT}/${dname} $PWD/$jname
          fi
      fi
  done;
  popd
  echo $fixed;

done


function abspath
 {
    if [[ -d "$1" ]]
    then
        pushd "$1" >/dev/null
        pwd
        popd >/dev/null
    elif [[ -e $1 ]]
    then
        pushd "$(dirname "$1")" >/dev/null
        echo "$(pwd)/$(basename "$1")"
        popd >/dev/null
    else
        echo "$1" does not exist! >&2
        return 127
    fi
}

