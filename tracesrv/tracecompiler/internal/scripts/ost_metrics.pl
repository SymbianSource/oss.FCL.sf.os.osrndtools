#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#
#!perl -w
# 
# 

#*********************************************************************
# ost_metrics.pl
# *********************************************************************
#
# VERSION     : 8      Draft         Feb-15-2010      Adrian Issott
# REASION     : Minor update to output style and added warnings about 
#               the tool being unsupported
#
# VERSION     : 7      Draft         Sep-16-2009      Adrian Issott
# REASION     : Added support for restricting output to specific
#               components
#
# VERSION     : 6      Draft         Sep-15-2009      Adrian Issott
# REASION     : Added support for older environments with dictionaries
#               under epoc32\include\internal\symbiantraces\dictionary
#
# VERSION     : 5      Draft         Sep-11-2009      Adrian Issott
# REASION     : Added trace name listing
#               Added counting of trace / group ID with lower case names
#               Made the output of detailed inforamtion optional 
#               Added support for epocroots other than \
#               Added warning about assuming epocroot=\ when it's not set
#               Fixed illegal division by zero issue when no components were found
#
# VERSION     : 4      Draft         Sep-11-2008      Esa Karvanen
# REASION     : Skip duplicate UID's when parsing
#
# VERSION     : 3      Draft         May-07-2008      Esa Karvanen
# REASION     : Changed to print amount of traces and stuff
#
# VERSION     : 2      Draft         May-06-2008      Esa Karvanen
# REASION     : Changed to calculate unique groups names
#
# VERSION     : 1      Draft         Mar-28-2008      Teemu Piiroinen
# REASION     : Initial version

use strict;
use warnings;
use env;

use Getopt::Long;
use Pod::Usage;

warn "\n";
warn "Warning: this script is not supported and the Dynamic Analysis Tools team\n";
warn "does NOT promise to fix any bugs or add any functionality to it.\n";
warn "\n";

my %options;
GetOptions('h|help|?'         => \$options{showHelp},
           'm|manual'         => \$options{showManual},
           'r|restrictTo=s'   => \$options{restrictTo},
           'c|byComponent'    => \$options{byComponent},
           't|showTraceNames' => \$options{showTraceNames},
           'l|lowerCaseNames' => \$options{lowerCaseNames},
  ) or pod2usage(-Verbose => 0);
pod2usage(-Verbose => 1) if ($options{showHelp});
pod2usage(-Verbose => 2) if ($options{showManual});

$options{byComponent} = 1 if $options{showTraceNames};

if (defined $options{restrictTo}) {
  eval "'' =~ /$options{restrictTo}/";
  die "Invalid restrictTo pattern ($options{restrictTo}): $@\n" if $@;    
} else {
  $options{restrictTo} = '.*';
}

my $component_count = 0;

my $group_count = 0;
my $total_group_count = 0;

my $trace_count = 0;
my $component_trace_count = 0;
my $total_trace_count = 0;
my @allgroups = ();
my %hashgroups = ();
my %uids;

my $lowerCaseGroupNames = 0;
my $lowerCaseTraceNames = 0;

my $epocroot;
if (not defined $ENV{EPOCROOT}) {
	warn "Warning: EPOCROOT not set so assuming it's \\\n";
	$epocroot = '\\';
} else {
	$epocroot = $ENV{EPOCROOT};
}
die "Expected the EPOCROOT ($epocroot) to be a directory\n" unless -d $epocroot;

my $ost_dictionaries_dir = $epocroot.'epoc32\ost_dictionaries';
print "Looking for dictionaries in $ost_dictionaries_dir ...\n";

if (not -d $ost_dictionaries_dir) {
   $ost_dictionaries_dir = $epocroot.'epoc32\include\internal\symbiantraces\dictionary';
   print "Looking for dictionaries in $ost_dictionaries_dir ...\n";
}
die "Couldn't find the OST dictionaries directory\n" unless -d $ost_dictionaries_dir;

print "Found the OST dictionaries directory\n";
print "\n";

my $dir_cmd = "dir $ost_dictionaries_dir\\*.xml /S 	/B 2>NUL";
my $dir_return = qx($dir_cmd);
my $current_group;

foreach my $file (split("\n", $dir_return))
{
	next unless $file =~ /$options{restrictTo}/i;
	
	# OST Autogen headers contain "_0x" in their file name
	if ($file =~ /_0x(.+?)_/i)
	{
		# Skip duplicate UID
		if (exists($uids{$1}))
		{
			warn "Warning: skipping duplicate UID in file " . $file . "\n";

		}
		# This UID for the first time, parse the file
		else
		{
			$uids{$1} = 0;
			parseFile($file);
		}
	}
	# Also calculate BTraceHooksDictionary
	elsif ($file =~ m/(BTraceHooksDictionary\.xml)/i)
	{
		parseFile($file);
	}
}

# Remove duplicates from the groups list
my %seen = ();
my @uniq_groups = ();
foreach my $item (@allgroups) {
  push(@uniq_groups, $item) unless $seen{$item}++;
}


### OUTPUT TOTALS ###


my @groupNames = sort keys %hashgroups; 
my $uniq_groups = scalar @groupNames;

my $avg_groups = ($component_count > 0 ? sprintf "%.2f",($total_group_count / $component_count) : 0);

print "\n\n--------------TOTALS--------------\n\n";
print "Component count: $component_count\n";
print "Unique group count: $uniq_groups\n";
print "Average groups per component: $avg_groups\n";
print "Trace count: $total_trace_count\n";

if ($options{lowerCaseNames}) {
	for my $groupName (@groupNames) {
		if (uc($groupName) ne $groupName) {
			$lowerCaseGroupNames++;
		} 
	
	}
	
	print "\n";
	print "Lower case Group ID names count: $lowerCaseGroupNames\n";
	print "Lower case Trace ID names count: $lowerCaseTraceNames\n";
}


### OUTPUT GROUP ID SUMMARY ###


# Generate new array where key is the amount of traces and value is the string to be printed
my @group_lines = ();
my $groupNo = 0;
foreach my $key (@groupNames) {
	my $line = "";
	
	# Check how many times this group can be found from all groups array
	my $occurrences = 0;
	foreach my $item (@allgroups) {
		if ($item eq $key) {
			$occurrences = $occurrences + 1;
		}
	}
	
	my $value = $hashgroups{$key};

	my $trace_percents = ($total_trace_count > 0 ? sprintf "%.2f",($value / $total_trace_count) * 100 : 0);	
	my $component_percents = ($component_count > 0 ? sprintf "%.2f",($occurrences / $component_count) * 100 : 0);

    # Add the group name
	my $i = 0;
	$line .= "$key";
    while ((length $key) + $i < 42){
    	$line .= " ";
    	$i = $i + 1;
    }

    # Add the number of traces  
    $i = 0;
    $line .= "$value";
    while ((length $value) + $i < 5){
    	$line .= " ";
  	  $i = $i + 1;
    }

    # Add the number of traces in percents
    $line .= "($trace_percents %)";
    $i = 0;
    while ((length $trace_percents) + $i < 21){
    	$line .= " ";
    	$i = $i + 1;
    }
  
    # Add used by no. of components
    $i = 0;
    $line .= "$occurrences";
    while ((length $occurrences) + $i < 3){
    	$line .= " ";
    	$i = $i + 1;
    }

    # Add used by no. of components in percents 
    $line .= "($component_percents %)\n";
  
    $group_lines[$groupNo][0] = $value;
    $group_lines[$groupNo][1] = $line;
    $groupNo = $groupNo + 1;
}

my @sorted_group_lines = reverse sort{$a->[0] <=> $b->[0]} @group_lines;  #if the $id'th column is numerically

print "\n\n--------------GROUP ID SUMMARY--------------\n\n";
print "GROUP NAME\t\t\t\tAMOUNT OF TRACES\tUSED BY NO. OF COMPONENTS\n\n";

foreach my $line (@sorted_group_lines) {
	print $$line[1];
}


#######################################################################
# Parses file
#######################################################################
sub parseFile
{
	
	my $file = $_[0];
	open FILE, "<$file" or die $!;

			foreach my $line (<FILE>)
			{
				if ($line =~ /<path val=\"(.+?)\"/i)
				{
					print "Path: $1\n" if $options{byComponent};
				}

				if ($line =~ /<component id=\"-?\d+\" name=\"(.+?)\"/i)
				{
					print "Component: $1\n" if $options{byComponent};

					$component_count++;
				}

				if ($line =~ /<group id=\"\d+\" name=\"(.+?)\"/i)
				{
					my $group = $1;
					print "\tGroup: $group\n" if $options{byComponent};
					push(@allgroups, $group);

					if (not defined $hashgroups{$group})
					{
						$hashgroups{$group} = 0;
					}

					$group_count++;
					$total_group_count++;
					$current_group = $group;
					
					print "\tGroup trace names:\n" if $options{showTraceNames};
				}

				if ($line =~ /<\/component>/i)
				{
					if ($options{byComponent}) {
						print "\tGroup count: $group_count\n";
						print "\tComponent trace count: $component_trace_count\n\n\n";
					}

					$group_count = 0;
					$component_trace_count = 0;
				}
				if ($line =~ /<trace data-ref=\"\d+\"(?:\s+name=\"(\w+)\")?/i)
				{
					if (defined $1) {
						print "\t\t$1\n" if $options{showTraceNames};
						if ($options{lowerCaseNames} and uc($1) ne $1) {
							$lowerCaseTraceNames++;
						}
					} 
					$trace_count++;
					$component_trace_count++;
					$total_trace_count++;
				}

				if ($line =~ /<\/group>/i)
				{
					print "\tGroup trace count: $trace_count\n" if $options{byComponent};
					$hashgroups{$current_group} = ($hashgroups{$current_group} + $trace_count);
					$trace_count = 0;
				}
			}

			close FILE;
}

warn "\n";
warn "Warning: this script is not supported and the Dynamic Analysis Tools team\n";
warn "does NOT promise to fix any bugs or add any functionality to it.\n";
warn "\n";

__END__

=head1 NAME

ost_metrics - Prints metrics about OST usage based on the dictionaries in an EPOCROOT

=head1 SYNOPSIS

ost_metrics [options]

=head1 OPTIONS

 -h (--help)             Brief help message.
 -m (--manual)           Full documentation.
 -r (--restrictTo) <re>  Restrict the output to just dictionaries with names 
                         matching the regular expression re.
 -c (--byComponent)      Output detailed information about the trace used in each 
                         component.
 -t (--showTraceNames)   Output detailed information about the trace ID names used.
                         Implies -byComponent
 -l (--lowerCaseNames)   Count the number of lower case names in the dictionaries.
 
=head1 DESCRIPTION

This script reads in the dictionaries found under EPOCROOT\epoc32\ost_dictionaries
and outputs the following information:

=over

=item 1 OST usage breakdown by component if --byComponent is specified

=item 2 OST trace ID usage by component if --showTraceNames is specified

=item 3 Group ID usage in the environment

=item 4 Total usage in the environment

=back

Note that if you don't define EPOCROOT this script assumes you mean "\". 

=head1 SUPPORT

Please note that this script is not supported and the Dynamic Analysis Tools team
does NOT promise to fix any bugs or add any functionality to it.

=head1 COPYRIGHT

Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.

=cut