#! /bin/bash

echo -n "Finding revision in "
pwd
revnum=`/usr/bin/svnversion . | cut -f '2' -d ':'`
# Now write the constant declaration to the file:
echo "#define SVN_VERSION \"$revnum\"" > svn_version.h
echo "Wrote revision $revnum to svn_version.h"
