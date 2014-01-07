#!/usr/bin/perl
#
# Script for creating and copying a Reify visible license to the appropriate directory.
# This script must reside in the same directory as executables rfyhostinfo and license-generator.
#
# WARNING: This script is for Reify internal use only. 
#          Do NOT give this script to customers or collaborators.
# 
# Copyright 2003 Reify Corporation. All rights reserved.
# Program version
my $VERSION = '$Revision: 5936 $';

########################################################################
#                                                                      #
# COMMAND LINE PROCESSING                                              #
#                                                                      #
########################################################################
use strict;
use Getopt::Long;


my (%options,
	);
use vars qw($directory)


Getopt::Long::Configure("bundling");
GetOptions( "d:s" => $directory);

$help && Usage();


# Specify license name and location
$license_dir = "/Users/Shared/Library/Reify";
$license_file = "visible.rfylic";
$rfyhostbin = "$directory/rfyhostinfo/rfyhostinfo.app/Contents/MacOS/";

# Get current hostid
$hostid = `$rfyhostbin/rfyhostinfo | grep -v Reify`;
chomp $hostid;

# Remove old license
system("rm -f $license_file");
# Generate new license
$licensebin = "$directory/license-generator/bin/license-generator.app/Contents/MacOS/";
system("$licensebin/license-generator -host $hostid -expiration never");

# Copy license
system("mkdir -p $license_dir");
system("cp -f $license_file $license_dir");

# We are done
print "Copied $license_file to $license_dir\n";
