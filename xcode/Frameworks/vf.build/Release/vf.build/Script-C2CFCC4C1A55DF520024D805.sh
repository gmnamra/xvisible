#!/bin/sh
FWK=$BUILD_DIR/$CONFIGURATION/$EXECUTABLE_PATH
echo $FWK
install_name_tool -id @executable_path/../Frameworks/$EXECUTABLE_NAME $FWK
