#!/bin/sh
# Parse arguments
DYLIB="vfi386.framework/Versions/A/vfi386"
applicationExecutable="${CONFIGURATION_BUILD_DIR}/Visible.app/Contents/MacOS/Visible"
echo "${LINENO} ${applicationExecutable}"
frameworksLocation="`dirname ${applicationExecutable}`/../Frameworks"
echo " ${LINENO} Relocating ${frameworksLocation}"
frameworkExecutable="${frameworksLocation}/${DYLIB}"

# Rewrite install_name of framework
newInstallName="@executable_path/../Frameworks/${DYLIB}"
echo "${LINENO}  Relocating ${frameworkExecutable} as ${newInstallName}"
install_name_tool -id "${newInstallName}" "${frameworkExecutable}"

 # Rewrite install_name in the app
echo "${LINENO}  Relocating ${frameworkExecutable} as ${newInstallName} in application"

# We re-search originalInstallName, in application, because might be different.
  originalInstallName=`otool -L ${applicationExecutable}| grep  ${DYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "${LINENO}  install_name_tool -change ${originalInstallName} ${newInstallName} ${applicationExecutable}"
  install_name_tool -change "${originalInstallName}" "${newInstallName}" "${applicationExecutable}"



# Rewrite install_name of framework
SUBDYLIB="ASC_DCAM_DEV.framework/Versions/A/ASC_DCAM_DEV"
SUBframeworkExecutable="${frameworksLocation}/${SUBDYLIB}"
echo "${LINENO}  Relocating ${SUBframeworkExecutable}"
newInstallName="@executable_path/../Frameworks/${SUBDYLIB}"
echo "${LINENO}  Relocating ${SUBframeworkExecutable} as ${newInstallName}"
install_name_tool -id "${newInstallName}" "${SUBframeworkExecutable}"

 # Rewrite install_name in the app
echo "${LINENO}  Relocating ${SUBframeworkExecutable} as ${newInstallName} in application"

# We re-search originalInstallName, in application, because might be different.
  originalInstallName=`otool -L ${applicationExecutable}| grep  ${SUBDYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "${LINENO}  install_name_tool -change ${originalInstallName} ${newInstallName} ${applicationExecutable}"
  install_name_tool -change "${originalInstallName}" "${newInstallName}" "${applicationkExecutable}"

# finally mark DYLIB and SUBDYLIB
 originalInstallName=`otool -L ${frameworksExecutable}| grep  ${SUBDYLIB} | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
  echo "${LINENO}  install_name_tool -change ${originalInstallName} ${newInstallName} ${frameworkExecutable}"

 install_name_tool -change "${originalInstallName}" "${newInstallName}" "${frameworkExecutable}"


