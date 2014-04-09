


ROOTBIN="$1"
NEWLNK="@executable_path/../Frameworks/";
CURRENT=Versions/Current



oouts=("`otool -l ${ROOTBIN} | grep Qt | sed -e 's/([^()]*)//g'`");

for oout in $oouts
  do 
    pieces=(${oout//// })                 # split otool output line
    dname=${pieces[${#pieces[@]} - 1]}    # get the framework name
    echo "install_name_tool -change ${oout} $dname.framework/${CURRENT}/${dname} ${NEWLNK}/${oout} ${ROOTBIN}"
    install_name_tool -change ${oout}  ${NEWLNK}/${oout} ${ROOTBIN}

  done;
end
