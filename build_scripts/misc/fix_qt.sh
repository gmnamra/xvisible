

SRCROOT="$1"
ORGLNK="/usr/local/qt/4.8.3/macx/lib"
NEWLNK="@executable_path/../Frameworks/"
QTLIBLOCATION="${SRCROOT}/usr/local/Qt/4.8.3/lib/i386/lib"
CURRENT=Versions/Current

QTLIBS[0]=QtGui
QTLIBS[1]=QtCore
QTLIBS[2]=QtOpenGL
QTLIBS[3]=Qt3Support
QTLIBS[4]=QtXml
QTLIBS[5]=QtXmlPatterns
QTLIBS[5]=phonon


for i in "${QTLIBS[@]}"
do
  fmdir="${QTLIBLOCATION}/$i.framework/${CURRENT}";
  echo "${fmdir}";
  for file in $(find -f "${fmdir}/")
  do
  if [[ $(file $file | grep 'shared library') ]]
  then
     `install_name_tool -id "$i.framework/${CURRENT}/${i}" "./$i.framework/$i"`
#    oldname=`otool -L ${file} | grep macx | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/' | sed -n 1p`
#    echo "install_name_tool -change $oldname" "${NEWLNK}$i.framework/${CURRENT}/${i}" "$i.framework/${CURRENT}/${i}"
#    echo otool output : `otool -L $file | grep  $ORGLNK | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`    
  fi
  done
done




