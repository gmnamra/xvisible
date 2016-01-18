#!/bin/bash

echo "recording build number..."                                                                                          
plist=${INFOPLIST_FILE}                                                                                                   

buildnum=`git rev-parse --short HEAD`                                                                                     
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $buildnum" "${plist}"                                                    

echo "Committing the build number..."                                                                                     
cd ${PROJECT_DIR};git add ${INFOPLIST_FILE}                                                                               
cd ${PROJECT_DIR};git commit -m "Bumped the build number."                                                                
cd ${PROJECT_DIR};git push -u origin master                                                                               
echo "Build number committed."                                                                                            


