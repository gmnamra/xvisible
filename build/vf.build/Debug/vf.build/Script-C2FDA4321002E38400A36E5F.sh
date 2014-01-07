#!/bin/sh

# Parse arguments
DYLIB="vf.framework/Versions/A/vf"
applicationExecutable="${CONFIGURATION_BUILD_DIR}/${DYLIB}"
echo "${applicationExecutable}"
newInstallName="@executable_path/../Frameworks/${DYLIB}"
echo "install_name_tool -id ${newInstallName} ${applicationExecutable}"
install_name_tool -id "${newInstallName}" "${applicationExecutable}"

