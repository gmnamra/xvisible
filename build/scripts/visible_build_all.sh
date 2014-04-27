#!/bin/bash

if [ ! "${1}" ]; then
    echo "usage: $0 xcode_project_path"
    exit
fi

XCODE_PROJECT_PATH=$1
XCODE_DIRECTORY="`dirname "$1"`"
XCODE_PROJECT_NAME="`basename -s .xcodeproj "${XCODE_PROJECT_PATH}"`"
PROJECT_RESOURCE_DIR=resources
PROJECT_RESOURCE_DIR=$PROJECT_RESOURCE_DIR/$XCODE_PROJECT_NAME
echo -e $PROJECT_RESOURCE_DIR

# xcodebuild needs to run from the project's directory so we'll move there
# and stay for the duration of this script
cd "${XCODE_DIRECTORY}"

# Check if svn is installed
if [ `which svn` ]; then
    echo "svn is installed on this computer"

    # If git is installed, then require that the code be committed
    if [ "`svn status -s 2>&1 | egrep '^\?\?|^ M|^A |^ D|^fatal:'`" ] ; then
        echo "Code is not committed into git. Commit into git before deployment."
        exit
    fi
    echo "Repository up-to-date."
fi

# !! Update: Changed from using the Perl Cocoa bridge to using PlistBuddy !!
# Use the perl to Objective-C bridge to get the version from the Info.plist
# You could easily use the python or ruby bridges to do the same thing

$file = "'"$${PROJECT_RESOURCE_DIR}/{XCODE_PROJECT_NAME}"'-Info.plist";
echo -e $file
$plist = NSDictionary->dictionaryWithContentsOfFile_($file);
$value = $plist->objectForKey_("CFBundleVersion");
print $value->description()->UTF8String() . "\n";' | perl`"

# Use PlistBuddy instead of the perl to Cocoa bridge
#CURRENT_VERSION="`/usr/libexec/PlistBuddy -c 'Print CFBundleVersion' \
#    "${XCODE_PROJECT_NAME}-Info.plist"`"

# Report the current version
echo "Current version is ${CURRENT_VERSION}"

# Prompt for a new version
read -p "Please enter the new version:
" NEW_VERSION

# !! Update: Changed from using the Perl Cocoa bridge to using PlistBuddy !!
# Use the bridge again to write the updated version back to the Info.plist
echo 'use Foundation;
$version = "'$NEW_VERSION'";
$file = "'"${XCODE_PROJECT_NAME}"'-Info.plist";
$plist = NSDictionary->dictionaryWithContentsOfFile_($file);
$plist->setObject_forKey_($version, "CFBundleVersion");
$plist->writeToFile_atomically_($file, "YES");' | perl

# Use PlistBuddy instead of the perl to Cocoa bridge
#/usr/libexec/PlistBuddy -c "Set CFBundleVersion ${NEW_VERSION}" \
#    "${XCODE_PROJECT_NAME}-Info.plist"

# Commit the updated Info.plist
if [ `which svn` ]; then
    svn ci -m "Updated Info.plist to version ${NEW_VERSION}" \
        "${XCODE_PROJECT_NAME}-Info.plist"
fi

# Clean the Release build for all targets we are interested in
#xcodebuild -configuration Release -target "${XCODE_PROJECT_NAME}" clean

# Build the Release build
#if [ "`xcodebuild -configuration Release -target "${XCODE_PROJECT_NAME}" build \
#     | egrep ' error:'`" ] ; then
#    echo "Build failed."
#    exit
#fi

# Tag the repository now that we have a successful build
# git tag "version-${NEW_VERSION}"

