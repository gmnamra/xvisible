#!/bin/bash

# works with a file called VF_VERSION in the current directory,
# the contents of which should be a semantic version number
# such as "1.2.3"

# this script will display the current version, automatically
# suggest a "minor" version update, and ask for input to use
# the suggestion, or a newly entered value.

# once the new version number is determined, the script will
# pull a list of changes from git history, prepend this to
# a file called CHANGES (under the title of the new version
# number) and create a GIT tag.

if [ -f VF_VERSION ]; then
    BASE_STRING=`cat VF_VERSION`
    BASE_LIST=(`echo $BASE_STRING | tr '.' ' '`)
    V_MAJOR=${BASE_LIST[0]}
    V_MINOR=${BASE_LIST[1]}
    V_PATCH=${BASE_LIST[2]}
    echo "Current version : $BASE_STRING"
    V_MINOR=$((V_MINOR + 1))
    V_PATCH=0
    SUGGESTED_VF_VERSION="$V_MAJOR.$V_MINOR.$V_PATCH"
    INPUT_STRING=$SUGGESTED_VF_VERSION
    echo $INPUT_STRING > VF_VERSION
    echo "Version $INPUT_STRING:" > tmpfile
    git log -1 >> tmpfile
    echo "" >> tmpfile
    cat CHANGES >> tmpfile
    mv tmpfile CHANGES
    git add CHANGES VF_VERSION
    git commit -m "Version bump to $INPUT_STRING"
#    git tag -a -m "Tagging version $INPUT_STRING" "v$INPUT_STRING"
#    git push origin --tags
else
    echo "Could not find a VF_VERSION file"
        echo "0.1.0" > VF_VERSION
        echo "Version 0.1.0" > CHANGES
        git log --pretty=format:" - %s" >> CHANGES
        echo "" >> CHANGES
        echo "" >> CHANGES
        git add VF_VERSION CHANGES
        git commit -m "Added VF_VERSION and CHANGES files, Version bump to v0.1.0"
#        git tag -a -m "Tagging version 0.1.0" "v0.1.0"
#        git push origin --tags
fi
