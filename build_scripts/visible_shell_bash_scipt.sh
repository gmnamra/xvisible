
## register executable to framework and framework to executable in the application Bundle

## $1 framework executable
## $2 application executable in MacOS folder
## $3 path to frameworks in Frameworks
## $4 core framework executable if != $1

applicationExecutable="${CONFIGURATION_BUILD_DIR}/Visible.app/Contents/MacOS/Visible"
VFDYLIB="vfi386.framework/Versions/A/vfi386"
frameworksLocation="`dirname ${applicationExecutable}`/../Frameworks"
frameworkExecutable="${frameworksLocation}/${VFDYLIB}"

echo " Application Executable: ${applicationExecutable} "
echo " VF Framework Name: ${VFDYLIB} "
echo " Frameworks Location: ${frameworksLocation} "
echo " Framework Executable: ${frameworkExecutable} "


VFLOCATION="${frameworksLocation}/${VFDYLIB}"
CVCORE_ALIAS=libopencv_core.2.2.dylib
CVHG_ALIAS=libopencv_highgui.2.2.dylib
CVIP_ALIAS=libopencv_imgproc.2.2.dylib
CVOD_ALIAS=libopencv_objdetect.2.2.dylib
#  CVF2_ALIAS=libopencv_features2d.2.2.dylib    
# CVCB_ALIAS=libopencv_calib3d.2.2.dylib
# CVFL_ALIAS=libopencv_flann.2.2.dylib
LPLOT_ALIAS=liblightplot2d.1.dylib

CVCORE=libopencv_core.2.2.0.dylib
CVHG=libopencv_highgui.2.2.0.dylib
CVIP=libopencv_imgproc.2.2.0.dylib
CVOD=libopencv_objdetect.2.2.0.dylib
# CVF2=libopencv_features2d.2.2.0.dylib    
# CVCB=libopencv_calib3d.2.2.0.dylib        
# CVFL=libopencv_flann.2.2.0.dylib            
LPLOT=liblightplot2d.1.0.1.dylib

function _log()
{
    echo "LINENO:  $1 $2 "
    #    echo "BASH_LINENO: ${BASH_LINENO[*]}"
}

function _register_in_parent_framework ()
{
    _log "${LINENO}" "install_name_tool -change $1 $2 $3"
    install_name_tool -change $1 $2 $3
}


## i.e. 2.2 to 2.2.x
function _link_framework ()
{
    _log "${LINENO}" "Linking $1 to $2 "
    pushd "$3"
    src="$1"
    dst="$2"
    if [ -e "$dst" ]
    then 
       echo "Link to Canonical Name with Versioned Name $dst exists"
    else
    ln -s "$src" "$dst"
    fi
    popd
}

function _link_this_in
{
    echo "install_name_tool -change $2 "@executable_path/../Frameworks/$2" $1"
    install_name_tool -change $2 "@executable_path/../Frameworks/$2" $1
}

function _crosslink_opencv ()
{
_log "${LINENO} -- $PWD "
pushd ${frameworksLocation}
_log "${LINENO} -- $PWD "
_link_this_in  $CVHG $CVCORE_ALIAS
_link_this_in $CVIP $CVCORE_ALIAS
_link_this_in $CVOD $CVCORE_ALIAS
_log "${LINENO}" " finished register ${CVCORE_ALIAS} "

_link_this_in $CVOD $CVIP_ALIAS
_link_this_in $CVHG $CVIP_ALIAS
_log "${LINENO}" " finished register ${CVIP_ALIAS} "

_link_this_in $CVOD $CVHG_ALIAS
_log "${LINENO}" " finished register ${CVHG_ALIAS} "

_link_this_in $CVOD $CVOD_ALIAS
_log "${LINENO}"  " finished _relocate and register in ${CVOD} "        
popd
_log "${LINENO} -- $PWD "
}


function relocate_and_register_()
{
    _log "${LINENO}"  "arg1 is $1  $2  $3"
    local lastarg=${!#}
    local extraOpArgN=3
        
        echo " Rewrite install_name of framework"
        echo 'step 2:'
        local newInstallName="@executable_path/../Frameworks/$1"
        echo "Relocating ${frameworkExecutable} as ${newInstallName}" 
        install_name_tool -change "$1" "${newInstallName}" "${frameworkExecutable}"
        ## Rewrite install_name in the app
        echo 'step 3:'
        
        echo "Relocating ${frameworkExecutable} as ${newInstallName} in application" 
        ## We re-search originalInstallName, in application, because might be different.
        local originalInstallName=`otool -L $2 | grep  $1 | sed -e 's/^\([[:blank:]]*\)\(.*\)\( (.*\)$/\2/'`
        echo "install_name_tool -change ${originalInstallName} ${newInstallName} $2 "
        install_name_tool -change "${originalInstallName}" "${newInstallName}" "$2"
        
        if [ $# = "$extraOpArgN" ]
        then
        _register_in_parent_framework $1 ${newInstallName} $3
        fi
    }
     
       
    _link_framework $CVCORE $CVCORE_ALIAS $frameworksLocation
    _link_framework $CVIP $CVIP_ALIAS $frameworksLocation
    _link_framework $CVHG $CVHG_ALIAS $frameworksLocation
    _link_framework $CVOD $CVOD_ALIAS $frameworksLocation
    # _link_framework $CVF2 $CVF2_ALIAS $frameworksLocation    
    # _link_framework $CVCB $CVCB_ALIAS $frameworksLocation        
    # _link_framework $CVFL $CVFL_ALIAS $frameworksLocation            
    _link_framework $LPLOT $LPLOT_ALIAS $frameworksLocation    
    _log "${LINENO}"  " finished _link_frameworks "
    relocate_and_register_ $VFDYLIB $applicationExecutable

    relocate_and_register_ $CVCORE_ALIAS $applicationExecutable $VFLOCATION
    relocate_and_register_ $CVHG_ALIAS $applicationExecutable $VFLOCATION
    relocate_and_register_ $CVIP_ALIAS $applicationExecutable $VFLOCATION
    relocate_and_register_ $CVOD_ALIAS $applicationExecutable $VFLOCATION
    #  relocate_and_register_ $CVF2_ALIAS $applicationExecutable $VFLOCATION    
    # relocate_and_register_ $CVCB_ALIAS $applicationExecutable  $VFLOCATION       
    # relocate_and_register_ $CVFL_ALIAS $applicationExecutable  $VFLOCATION           
    relocate_and_register_ $LPLOT_ALIAS $applicationExecutable $VFLOCATION    
    _log "${LINENO}"  " finished _relocate and register in vfi "
    

   _crosslink_opencv 
     
    
