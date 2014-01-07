#! /bin/sh


#-------------
# Loop over movies. Create one job per movie.
#-------------
##Generate signatures
run_main_content (){
    echo "content dir is $1";
    echo "tmp dir is $2";
    echo "Removing existing signature files.."
    rm "$1/*.rfysig"
    echo "Done"    
    criteria=".mpg";

    for filename in $( ls -sS $1/*$criteria* | sort )
    do
        foo=$(echo $filename | sed -e "s/mpg/rfysig/")
	if [ -d "$filename" ]; then
	echo "-content $filename -sigout $foo -tmpDir2Use $2 "
	vsigmatch -content $filename -sigout $foo -tmpDir2Use $2
	fi
    done 
}

#-------------
# Loop over clips. match to content
#-------------
##Generate signatures && Match against a dir of existing signatures

check_clips ()
{
    local firstclipdir
    local secondclipdir
    local contentdir
    criteria=".mpg";
    echo "mainSignature Dir is $1"
    echo "clip dir is $2"
    echo "tmp dir is $3";

    for filename in $( ls -sS $2/*$criteria* | sort)
    do
        foo=$(echo $filename | sed -e "s/mpg/rfysig/")	
	echo $filename
	echo $foo
	bar=$(echo $filename | sed -e "s/mpg/rfymat/")	
	vsigmatch -content $filename -sigout $foo -sample 10 -search $1 -tmpDir2Use $3 -searchrep $bar
    done 
}


############################################ M A I N ##################

testdir=`pwd`
tmpDir=$testdir/tmp

# First run the main content

mainContentDir=$testdir/content
clipContentDir=$testdir/clipdir

cd "../bin/vsigmatch.app/Contents/MacOS/"
binDir=`pwd`
echo " Bin Dir is $binDir "

run_main_content $mainContentDir $tmpDir

echo "Finished Main Content "

# Now run the first clip directory

echo "Starting $clipContentDir "

check_clips $mainContentDir $clipContentDir $tmpDir

echo "Finished $clipContentDir "
echo "Starting $clip2ContentDir "

check_clips $mainContentDir $clip2ContentDir $tmpDir

echo "Finished $clip2ContentDir "

exit 0

