echo $BUILD_SCRIPTS_DIR

# better to fetch the old location. For now this is what we do know. 
OLD_LOC=/usr/local/opencv-2.4.8/build/lib/Release/
NEW=@executable_path/../Frameworks/

cd $BUILD_DIR/$CONFIGURATION/$EXECUTABLE_FOLDER_PATH

echo `$BUILD_SCRIPTS_DIR/reset_linked.sh -r "$EXECUTABLE_NAME" -s $OLD_LOC -g $NEW -v y`


#    export EXECUTABLE_NAME=XcVisible
#    export EXECUTABLE_PATH=XcVisible.app/Contents/MacOS/XcVisible

install_name_tool -change @executable_path/../Frameworks/vfi386 @executable_path/../Frameworks/vfi386.framework/Versions/Current/vfi386 $EXECUTABLE_NAME

cd $BUILD_DIR/$CONFIGURATION/$FRAMEWORKS_FOLDER_PATH

for ofile in `ls libopencv_*.*`; do ../../../../../scripts/replace_install_ids.sh -r $ofile -s /usr/local/opencv-2.4.8/build/lib/Release/  -g @executable_path/../Frameworks/ -v y; done

FWK_EXECUTABLE_NAME=vfi386.framework/Versions/Current/vfi386

echo `$BUILD_SCRIPTS_DIR/reset_linked.sh -r "$FWK_EXECUTABLE_NAME" -s $OLD_LOC -g $NEW -v y`





