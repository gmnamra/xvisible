#!/bin/bash

# Parse arguments
PN="${PRODUCT_NAME}"
DYLIB="${PN}.framework/Versions/A/${PN}"
applicationExecutable="${CONFIGURATION_BUILD_DIR}/${DYLIB}"
echo "${applicationExecutable}"
newInstallName="@executable_path/../Frameworks/"
echo "install_name_tool -id ${newInstallName}/${FULL_PRODUCT_NAME} ${applicationExecutable}"
install_name_tool -id "${newInstallName}" "${applicationExecutable}"
product="${CONFIGURATION_BUILD_DIR}/${FULL_PRODUCT_NAME}"


