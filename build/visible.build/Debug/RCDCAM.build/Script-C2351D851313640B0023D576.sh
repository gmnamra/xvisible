#!/bin/sh
# Parse arguments
DYLIB="vf.framework/Versions/A/vf"
applicationExecutable="${CONFIGURATION_BUILD_DIR}/Visible.app/Contents/MacOS/RCDCAM"
echo "${applicationExecutable}"
frameworksLocation="`dirname ${applicationExecutable}`/../Frameworks"
echo "Relocating ${frameworksLocation}"
frameworkExecutable="${frameworksLocation}/${DYLIB}"

# Rewrite install_name of framework
newInstallName="@executable_path/../Frameworks/${DYLIB}"
echo "Relocating ${frameworkExecutable} as ${newInstallName}" 
install_name_tool -id "${newInstallName}" "${frameworkExecutable}"

 # Rewrite install_name in the app
echo "Relocating ${frameworkExecutable} as ${newInstallName} in application" 

# We re-search originalInstallName, in application, because might be different.
  originalInstallName=`otool -L ${applicationExecutable}| grep  ${DYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "install_name_tool -change ${originalInstallName} ${newInstallName} ${applicationExecutable}"
  install_name_tool -change "${originalInstallName}" "${newInstallName}" "${applicationExecutable}"



# Rewrite install_name of framework
SUBDYLIB="ASC_DCAM_DEV.framework/Versions/A/ASC_DCAM_DEV"
SUBframeworksLocation="`dirname ${applicationExecutable}`/../Frameworks"
echo "Relocating ${SUBframeworksLocation}"
SUBframeworkExecutable="${SUBframeworksLocation}/${SUBDYLIB}"
newInstallName="@executable_path/../Frameworks/${SUBDYLIB}"
echo "Relocating ${frameworkExecutable} as ${newInstallName}" 
install_name_tool -id "${newInstallName}" "${SUBframeworkExecutable}"

 # Rewrite install_name in the app
echo "Relocating ${SUBframeworkExecutable} as ${newInstallName} in application" 

# We re-search originalInstallName, in application, because might be different.
  originalInstallName=`otool -L ${applicationExecutable}| grep  ${SUBDYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "install_name_tool -change ${originalInstallName} ${newInstallName} ${applicationExecutable}"
  install_name_tool -change "${originalInstallName}" "${newInstallName}" "${applicationkExecutable}"

# finally mark DYLIB and SUBDYLIB
 originalInstallName=`otool -L ${frameworksExecutable}| grep  ${SUBDYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "install_name_tool -change ${originalInstallName} ${newInstallName} ${frameworkExecutable}"

 install_name_tool -change "${originalInstallName}" "${newInstallName}" "${frameworkExecutable}"
