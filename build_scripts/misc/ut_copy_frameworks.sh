#!/bin/bash

GUTPATH=${CONFIGURATION_BUILD_DIR}/${PRODUCT_NAME}
echo $GUTPATH
FRAMEWORK_DIR=@executable_path/Frameworks
CURRENT=Versions/Current

# get current rpath for vfi386
CURNAME=`otool -L $GUTPATH | grep ${VF_NAME/%.framework/} | sed -e 's/([^()]*)//g'`
echo $CURNAME
# switch to using Frameworks of off build_dir
install_name_tool -change $CURNAME "${FRAMEWORK_DIR}/$VF_NAME.framework/$CURRENT/$VF_NAME" $GUTPATH 

# get current rpath for gtest
CURNAME=`otool -L $GUTPATH | grep gtest | sed -e 's/([^()]*)//g'`
echo $CURNAME
# switch to using Frameworks of off build_dir
install_name_tool -change $CURNAME "${FRAMEWORK_DIR}/gtest.framework/$CURRENT/gtest" $GUTPATH









