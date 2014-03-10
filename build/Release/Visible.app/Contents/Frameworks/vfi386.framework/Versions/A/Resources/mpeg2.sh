#!/bin/bash

package=mpeg2dec
version=-0.4.1
progname=$0

. ../scripts/untarAbuild.bash

opt_run_untar $force_untar $auto_untar $package $version ".tar"
opt_run_configure $force_config $auto_config $package $version

opt_run_make $compile   $package $version
##opt_run_make $install   $package $version "install"
opt_run_make $clean     $package $version "clean"
opt_run_make $distclean $package $version "distclean"

opt_run_tarclean $tarclean $package $version
