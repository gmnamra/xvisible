

ROOTBIN="$1"
NEWLNK="@executable_path/../Frameworks/"
CURRENT=Versions/Current



oouts=("`otool -l ${ROOTBIN} | grep Qt | sed -e 's/([^()]*)//g'`");

for frm in $oouts
  do 
    fname=${frm/%.framework/};     # 
    jname=${fname#*.\/};           # get the framework executable
    echo "$fname ---- $jname"
  done
done




