#!/bin/bash

if [ ! "${1}" ]; then
    echo "usage: $0 xcode_project_path"
    exit
fi

XCODE_PROJECT_PATH=$1
XCODE_DIRECTORY="`dirname "$1"`"
XCODE_PROJECT_NAME="`basename -s .xcodeproj "${XCODE_PROJECT_PATH}"`"

# xcodebuild needs to run from the project's directory so we'll move there
# and stay for the duration of this script
cd "${XCODE_DIRECTORY}"

# Check if git is installed
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
#CURRENT_VERSION="`echo 'use Foundation;
#$file = "'"${XCODE_PROJECT_NAME}"'-Info.plist";
#$plist = NSDictionary->dictionaryWithContentsOfFile_($file);
#$value = $plist->objectForKey_("CFBundleVersion");
#print $value->description()->UTF8String() . "\n";' | perl`"

# Use PlistBuddy instead of the perl to Cocoa bridge
CURRENT_VERSION="`/usr/libexec/PlistBuddy -c 'Print CFBundleVersion' \
    "${XCODE_PROJECT_NAME}-Info.plist"`"

# Report the current version
echo "Current version is ${CURRENT_VERSION}"

# Prompt for a new version
read -p "Please enter the new version:
" NEW_VERSION

# !! Update: Changed from using the Perl Cocoa bridge to using PlistBuddy !!
# Use the bridge again to write the updated version back to the Info.plist
#echo 'use Foundation;
#$version = "'$NEW_VERSION'";
#$file = "'"${XCODE_PROJECT_NAME}"'-Info.plist";
#$plist = NSDictionary->dictionaryWithContentsOfFile_($file);
#$plist->setObject_forKey_($version, "CFBundleVersion");
#$plist->writeToFile_atomically_($file, "YES");' | perl

# Use PlistBuddy instead of the perl to Cocoa bridge
/usr/libexec/PlistBuddy -c "Set CFBundleVersion ${NEW_VERSION}" \
    "${XCODE_PROJECT_NAME}-Info.plist"

# Commit the updated Info.plist
if [ `which git` ]; then
    git commit -m "Updated Info.plist to version ${NEW_VERSION}" \
        "${XCODE_PROJECT_NAME}-Info.plist"
fi

# Clean the Release build
xcodebuild -configuration Release -target "${XCODE_PROJECT_NAME}" clean

# Build the Release build
if [ "`xcodebuild -configuration Release -target "${XCODE_PROJECT_NAME}" build \
     | egrep ' error:'`" ] ; then
    echo "Build failed."
    exit
fi

# Tag the repository now that we have a successful build
git tag "version-${NEW_VERSION}"

#########
# From this point onwards, the script is all about DMG packaging
#########

# Create a temporary directory to work in
TEMP_DIR="`mktemp -d "${TMPDIR}${XCODE_PROJECT_NAME}.XXXXX"`"

# Create the folder from which we'll make the disk image
DISK_IMAGE_SOURCE_PATH="${TEMP_DIR}/${XCODE_PROJECT_NAME}"
mkdir "${DISK_IMAGE_SOURCE_PATH}"

# Copy the application into the folder
cp -R "build/Release/${XCODE_PROJECT_NAME}.app" \
    "${DISK_IMAGE_SOURCE_PATH}/${XCODE_PROJECT_NAME}.app"

# Make a symlink to the Applications folder
# (so we can prompt the user to install the application)
ln -s "/Applications" "${DISK_IMAGE_SOURCE_PATH}/Applications"

# If a "background.png" file is present in the Xcode project directory,
# we'll use that for the background of the folder.
# An assumption is made in this script that the background image is 400x300px
# If you are using a different sized image, you'll need to adjust the
# placement and sizing parameters in the Applescript below
if [ -e "background.png" ]; then
    cp "background.png" \
        "${DISK_IMAGE_SOURCE_PATH}/background.png"
fi


# Create the read-write version of the disk image from the folder
# Also note the path at which the disk is mounted so we can open the disk
# to adjust its attributes
DISK_IMAGE_READWRITE_PATH="${DISK_IMAGE_SOURCE_PATH}-rw.dmg"
VOLUME_MOUNT_PATH="`hdiutil create -srcfolder "${DISK_IMAGE_SOURCE_PATH}" \
    -format UDRW -attach "${DISK_IMAGE_READWRITE_PATH}" | \
    sed -n 's/.*\(\/Volumes\/.*\)/\1/p'`"


# Now we use Applescript to tell the Finder to open the disk image,
# set the view options to a bare, icon arranged view
# set the background image (if present)
# and set the icon placements
if [ -e "background.png" ]; then
    echo '
    tell application "Finder"
        open ("'"${VOLUME_MOUNT_PATH}"'" as POSIX file)
        set statusbar visible of front window to false
        set toolbar visible of front window to false
        set view_options to the icon view options of front window
        set icon size of view_options to 96
        set arrangement of view_options to not arranged
        set the bounds of front window to {100, 100, 500, 400}
        set app_icon to item "'"${XCODE_PROJECT_NAME}"'" of front window
        set app_folder to item "Applications" of front window
        set background_image to item "background.png" of front window
        set background picture of view_options to item "background.png" of front window
        set position of background_image to {200, 200}
        set position of app_icon to {120, 100}
        set position of app_folder to {280, 100}
        set current view of front window to icon view
    end tell' | osascript
else
    echo '
    tell application "Finder"
        open ("'"${VOLUME_MOUNT_PATH}"'" as POSIX file)
        set statusbar visible of front window to false
        set toolbar visible of front window to false
        set view_options to the icon view options of front window
        set icon size of view_options to 96
        set arrangement of view_options to not arranged
        set the bounds of front window to {100, 100, 500, 400}
        set app_icon to item "'"${XCODE_PROJECT_NAME}"'" of front window
        set app_folder to item "Applications" of front window
        set position of app_icon to {120, 100}
        set position of app_folder to {280, 100}
        set current view of front window to icon view
    end tell' | osascript
fi

# Make the background.png file invisible
SetFile -a V "${VOLUME_MOUNT_PATH}/background.png"

# Eject the disk image so that we can convert it to a compressed format
hdiutil eject "${VOLUME_MOUNT_PATH}"

# Create the final, compressed disk image
hdiutil convert "${DISK_IMAGE_READWRITE_PATH}" -format UDBZ \
    -o "${HOME}/Desktop/${XCODE_PROJECT_NAME}.dmg"

# Remove the temp directory
rm -Rf "${TEMP_DIR}"
