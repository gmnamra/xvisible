#! /bin/bash

usage="-d Directory -p XcodeProject -t Target -a Action -l Logfile"

while getopts ":d:p:t:a:l" options; do
  case $options in
    d ) dirname=$OPTARG;;
    p ) project=$OPTARG;;
    t ) target=$OPTARG;;
    a ) action=$OPTARG;;
    l ) _logfile=$OPTARG;;    
    h ) echo $usage;;
    \? ) echo $usage
         exit 1;;
    * ) echo $usage
          exit 1;;

  esac
done

echo '===== BUILDING FILIE FOR DEPLOYMENT ====='
echo 'Script:'$0
if [ ! -d $dirname ]; then dirname = $pwd; fi
    cd $dirname
    project=$project".xcodeproj"
    echo 'Project Name: '$project
    if [ ! -d $project ]; then exit 0; fi

    echo "    xcodebuild -project $project -target $target $action  2>&1 & "
    xcodebuild -project $project -target $target $action  2>&1 &

    
##    if [! -d "$project/build" ]; then exist 0; fi

            cd build
            echo '===== CREATING ARCHIVE ====='
## tar -czf Filie.tgz Filie.app

            echo '===== UPLOADING ARCHIVE ====='
## curl --upload-file Filie.tgz ftp://home-up.t-online.de/

            echo '===== FINISHED ====='
