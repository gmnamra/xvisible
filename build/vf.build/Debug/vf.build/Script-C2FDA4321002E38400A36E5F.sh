#!/bin/sh

# Parse arguments
PN="${PRODUCT_NAME}"
DYLIB="${PN}.framework/Versions/A/${PN}"
applicationExecutable="${CONFIGURATION_BUILD_DIR}/${DYLIB}"
echo "${applicationExecutable}"
newInstallName="@executable_path/../${DYLIB}"
echo "install_name_tool -id ${newInstallName} ${applicationExecutable}"
install_name_tool -id "${newInstallName}" "${applicationExecutable}"

