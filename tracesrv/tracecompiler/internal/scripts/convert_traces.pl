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
# convert_traces.pl
# *********************************************************************
#
# VERSION     : 6      Draft         15-Feb-2010       Adrian Issott
# REASON      : Added warnings about the tool being unsupported
#
# VERSION     : 5      Draft         21-Jan-2010       Adrian Issott
# REASON      : Fixed issue with own macro support + added removal of 
#               _L(...) around trace format strings
#
# VERSION     : 4      Draft         11-Nov-2009       Esa Karvanen
# REASON      : Initial support for converting OST Function entry /
#               exit traces to printfs.
#
# VERSION     : 3      Draft         20-Aug-2009       Esa Karvanen
# REASON      : Added support to convert OST traces back to printf. 
#               Doesn't work for Function entry / exit traces.
#
# VERSION     : 2      Draft         10-Apr-2008       Esa Karvanen
# REASON      : Printf and own macro support
#
# VERSION     : 1      Draft         16-Oct-2007       Teemu Piiroinen
# REASON      : Initial version

use strict;
use env;
use FindBin '$Bin';

warn "\n";
warn "Warning: this script is not supported and the Dynamic Analysis Tools team\n";
warn "does NOT promise to fix any bugs or add any functionality to it.\n";
warn "\n";

# Should we print debug prints
my $DEBUG = 0;

if (not defined $ARGV[0] or not -f $ARGV[0])
{
	print "Please give path to mmp file as parameter.";
	
	exit;
}

# Get group names
my %map_trace_to_group;
my %map_trace_to_text;
my %map_trace_to_parameters;

my $traces_folder;

# Get source files
my @sources = getSourceFiles();
my @trace_id_names;
my $CASWTraceCount = 0;

my $printfMacro;


# Ask selection from user
my $selection = get_operation();

# Check  if the selection is numeric
if ($selection =~ /^-?\d/)
{
	# Symbian traces
	if ($selection == 1)
	{
		$traces_folder = get_traces_folder();
		convertSymbianTraces(@sources);
	}

	# Kern::Printf
	elsif ($selection == 2)
	{
		$printfMacro = "kern::printf";
		convertPrintf($printfMacro, @sources);
	}

	# RDebug::Printf
	elsif ($selection == 3)
	{
		$printfMacro = "rdebug::printf";
		convertPrintf($printfMacro, @sources);
	}
	
	# User defined MACRO
	elsif ($selection == 4)
	{
		$printfMacro = "rdebug::printf";
		convertPrintf($printfMacro, @sources);
	}
	
	# OST traces to Kern::Printf
	elsif ($selection == 5)
	{
		$printfMacro = "Kern::Printf";
		convertOstToPrintf($printfMacro, @sources);
	}	
	
	# OST traces to RDebug::Printf
	elsif ($selection == 6)
	{
		$printfMacro = "RDebug::Printf";
		convertOstToPrintf($printfMacro, @sources);
	}
	else
	{
		print "Wrong selection!";
	}
}

# Otherwise, it's user defined MACRO
else
{
	$printfMacro = $selection;
	convertPrintf($printfMacro, @sources);
}


#-------------------------------------------------------
# Convert Symbian traces
#-------------------------------------------------------
sub convertSymbianTraces
{
	debug("\nConvertSymbianTraces starts");
	my (@sources) = @_;
	
	# Go through all found source files
	foreach my $source_file (@sources)
	{
		debug("Source file $source_file");
		print $source_file . "\n";
		
		RemoveLineBreaksFromTracesInFile($source_file, 0);
		
		open FILE, "<$source_file" or die $!;
		
		my $new_file_content;
		
		my $traces_converted = 0;
		my $file_name = "";
		my $last_include_line = 0;
		my $line_number = 0;
	
		if ($source_file =~ /.*\\(.+?)\..+/)
		{
			$file_name = $1;
		}
		
		foreach my $line (<FILE>)
		{
			$line_number += 1;
			
			my $line_converted = 0;
			
			chomp $line;
			
			if ($line =~ /^\s*\#include/)
			{
				$last_include_line = $line_number; 
			}
			
			if ($line =~ /Traces\.h/)
			{
				$line = "// " . $line;
			}
			
			# FA_CORE_SERVICES traces
			if ($line =~ /(BUILD_TRACE|ASSERT_TRACE|ASSERT_ALWAYS_TRACE|API_TRACE|INTERNAL_TRACE|DATA_DUMP_TRACE|ISIMSG_API_TRACE).*;/)
			{
				my $trace_name = $1;		
				
				if ($line =~ /.*?\/\/.*?$trace_name/)
				{
					next;
				}
				
				my $spaces = "";				
				if ($line =~ /^(\s+).*?$trace_name/)
				{
					$spaces = $1;
				}
				
				my $prefix = "";				
				if ($line =~ /^\s*(.*?)$trace_name/)
				{
					$prefix = $1;
				}
				
				$line =~ s/^.*($trace_name.+;).*/$1/;
				
				if ($line =~ /$trace_name\s*\((.+)\)\s*;/)
				{
					$line_converted = 1;
					$traces_converted = 1;
					
					my $parameters = $1;
					
					$new_file_content .= $spaces . "// " . $prefix . $line . "\n";
				
					# Get parameters without format string
					my $parameters_without_format = $parameters;
					if ($parameters =~ /\".*\"(.*)/)
					{
						$parameters_without_format = $1;
					}
					
					# Calculate parameter count
					my $parameter_count = -1;
					foreach my $part (split(",", $parameters_without_format))
					{
						$parameter_count++;
					}
	
					my $new_trace = GetNewCASWTraceName($trace_name, $parameter_count);
					
					if ($trace_name =~ /(BUILD_TRACE|ASSERT_TRACE|ASSERT_ALWAYS_TRACE|API_TRACE|INTERNAL_TRACE)/)
					{
						$new_trace .= "(" . $trace_name . ", TRACE_NAME_" . $CASWTraceCount++ . ", " . $parameters . ");";
					}
					else
					{
						$new_trace .= "(" . $trace_name . ", TRACE_NAME_" . $CASWTraceCount++ . ", ";
						
						if ($parameter_count == 0)
						{
							$new_trace .= "\"\", " . $parameters . ", <data_len>);";
						}
						else
						{
							$new_trace .= $parameters . ", <data_len>);";
						}
					}
					
					$new_file_content .= $spaces . $new_trace . "\n";
				}
			}
			
			# SymbianTraces
			if ($line =~ /SymbianTrace([0-2]|Data[0-1]{0,1}|Thread[0-2]|ThreadData[0-1]{0,1}|IsTraceActive|Pc[0-2]|ThreadPc[0-2]|PcData[0-1]{0,1}|ThreadPcData[0-1]{0,1}|StartControlBlock|EndControlBlock|Ext[1-6]|ExtThread[1-6]).*;/)
			{
				my $trace_name = "SymbianTrace" . $1;
	
				if ($line =~ /.*?\/\/.*?SymbianTrace/)
				{
					next;
				}						
			
				# print $trace_name . "\n";
				
				my $spaces = "";				
				if ($line =~ /^(\s+).*?SymbianTrace/)
				{
					$spaces = $1;
				}
				
				my $prefix = "";				
				if ($line =~ /^\s*(.*?)SymbianTrace/)
				{
					$prefix = $1;
				}
				
				$line =~ s/^.*(SymbianTrace.+;).*/$1/;
				
				#print $line . "\n";
				
				# Get trace id name
				if ($line =~ /\(\s*(.+?)[\s\),]{1,2}(.*?)\)\s*;$/)
				{
					$line_converted = 1;
					$traces_converted = 1;
					
					my $trace_id_name = $1;
					my $rest_of_parameters = $2;
					
					my $new_trace_id_name = $trace_id_name;
					
					$new_file_content .= $spaces . "// " . $prefix . $line . "\n";
									
					#print $trace_id_name . "\n";
					#print $rest_of_parameters . "\n";
					
					if ($trace_name ne "SymbianTraceIsTraceActive")
					{
						while (grep(/$new_trace_id_name/, @trace_id_names))
						{
							# Duplicate trace
							$new_trace_id_name .= "_";
						}
						
						push(@trace_id_names, $new_trace_id_name);
					}
								
					my $trace_text = $map_trace_to_text{$trace_id_name};
					
					my @params = split(",", $rest_of_parameters);
									
					$trace_text .= GetFormatText($trace_id_name, @params);
					
					my $type_string = IsStringType($trace_id_name, @params);
					
					my $group_name = $map_trace_to_group{$trace_id_name};
					if (not defined $group_name or $group_name eq "")
					{
						$group_name = "DEFAULT_GROUP";
						$trace_text = "";
					}
					
					my $new_trace = GetNewSymbianTraceName($trace_name, $type_string);
					$new_trace .= "( " . $group_name . ", " . 
												$new_trace_id_name . ", " .
												"\"" . $trace_text . "\"";
												
					if ($rest_of_parameters ne "")
					{
						 $new_trace .= ", " . $rest_of_parameters;
					}
					
					$new_trace .= ");";
					
					$new_file_content .= $spaces . $new_trace . "\n";
					
					#print $new_trace . "\n\n";
				}
				else
				{
					print STDERR "Cannot find trace id name. From line: $line\n";
				}
			}
			
			if (not $line_converted)
			{
				$new_file_content .= $line . "\n";
			}
			
		}
	
		close FILE;	
		
		if ($traces_converted == 1)
		{		
			SaveNewSourceFile($source_file, $new_file_content, $last_include_line, $file_name, 1);
		}
	}

CreateOSTTraceDefinitionsHeader();
}


#-------------------------------------------------------
# Convert PRINTF traces to OST traces
#-------------------------------------------------------
sub convertPrintf
{
	my ($macro, @sources) = @_;
	debug("\nConvertPrintf starts, macro is: $macro");
	
	# Go through all found source files
	foreach my $source_file (@sources)
	{
		debug("Source file $source_file");
		print $source_file . "\n";
		
		RemoveLineBreaksFromTracesInFile($source_file, 0);
		
		open FILE, "<$source_file" or die $!;
		
		my $new_file_content;
		
		my $traces_converted = 0;
		my $file_name = "";
		my $last_include_line = 0;
		my $line_number = 0;
		my $traceNumber = 0;
	
		if ($source_file =~ /.*\\(.+?)\..+/)
		{
			$file_name = $1;
		}
		
		foreach my $line (<FILE>)
		{
			$line_number += 1;
			
			my $line_converted = 0;
			
			chomp $line;
			
			if ($line =~ /^\s*\#include/)
			{
				$last_include_line = $line_number; 
			}
			
			if ($line =~ /Traces\.h/)
			{
				$line = "// " . $line;
			}
			
			# Printf macro
			if ($line =~ /^\s*$macro.*;/i)
			{		
				my $spaces;				
				my $trace = "";
				if ($line =~ /^(\s*)/)
				{
					$spaces = $1;
				}
				
				$line =~ s/^\s*($macro.+;).*/$1/;
				
				$trace = $line;

				# Remove spaces from the beginning
				$trace =~ s/^\s*//g;

				print("Trace: " . $line . "\n");
				
				if (GetBracketCount($line) % 2 == 0)
				{
					my $param_count = 0;
					my $params = $line;
					
					while($params =~ s/(.*?),/$1/)
					{
						$param_count++;
					}
										
					$line = $spaces . "// " . $trace ."\n";
					$line .= $spaces;
					
					# Remove the macro from the trace
					debug("Trace before removing MACRO $trace");
					$trace =~ /\((.*)\)/;
					$trace = $1;
					debug("Trace after removing MACRO $trace");

    				# Remove use of _L(...)
    				if ($trace =~ s/\b_L\(//g) 
                    {
                        $trace =~ s/(.*")\)/$1/; # want to remove the last ") rather than the first one we come across
                    }

					# Convert
					# 0 parameters
					debug("Convert trace with $param_count parameters");
					if ($param_count == 0)
					{
						$line .= "OstTrace0(DEFAULT_GROUP, DEFAULT_TRACE" . $traceNumber . ", " . $trace . ");";
					}
					# 1 parameter
					elsif	($param_count == 1)
					{
						$line .= "OstTrace1(DEFAULT_GROUP, DEFAULT_TRACE" . $traceNumber . ", " . $trace . ");";
					}
					# More than 1 parameters
					else
					{
					$line .= "OstTraceExt" . $param_count . "(DEFAULT_GROUP, DEFAULT_TRACE" . $traceNumber . ", " . $trace . ");";	
					}
					
					$new_file_content .= $line . "\n";
					$line_converted = 1;
					$traces_converted = 1;
					$traceNumber++;
				}
			}
			else
			{
				$new_file_content .= $line . "\n";
			}
		}
	
		close FILE;	
		
		if ($traces_converted == 1)
		{	
			debug("\n\nSave new source file");
			SaveNewSourceFile($source_file, $new_file_content, $last_include_line, $file_name, 1);
		}
	}

CreateOSTTraceDefinitionsHeader();
	
}


#-------------------------------------------------------
# Get bracket count
#-------------------------------------------------------
sub GetBracketCount
{
	my ($line) = @_;
	
	$line =~ s/\\\(//g;
	$line =~ s/\\\)//g;
	
	my @brackets = ($line =~ /\(|\)/g);
	
	return scalar @brackets;
}

#-------------------------------------------------------
# Save new source file
#-------------------------------------------------------
sub SaveNewSourceFile
{
	my ($source_file, $new_file_content, $last_include_line, $file_name, $add_ost_stuff) = @_;
	
	open FILE, ">$source_file" or die $!;
	
	my @lines = split("\n", $new_file_content);
	
	my $line_number = 0;
	foreach my $line (@lines)
	{	
		$line_number++;
		
		print FILE $line . "\n";
		
		if ($line_number == $last_include_line && $add_ost_stuff == 1)
		{
			print FILE "\#include \"OstTraceDefinitions.h\"\n";
			print FILE "\#ifdef OST_TRACE_COMPILER_IN_USE\n";
			print FILE "\#include \"" . $file_name . "Traces.h\"\n";
			print FILE "\#endif\n\n";
		}
	}
	
	close FILE;
}


#-------------------------------------------------------
# Create OST Trace Definitions header file
#-------------------------------------------------------
sub CreateOSTTraceDefinitionsHeader
{
	debug("\nCreateOSTTraceDefinitionsHeader starts");
	
	# Get path to traces folder
	my $mmp_file = $ARGV[0];
	my $file_path = $mmp_file;
  $file_path =~ s/\\[^\\]+$/\\/i;
  $file_path =~ s/\\\\/\\/g;
  if ($file_path =~ m/\.mmp/i)
  {
  	debug("getGroupTraceData. MMP file doesn't have path. Use current dir.");
  	my $dir = "$Bin";
  	$file_path = $dir;
  	$file_path =~ s/\//\\/g;
  } 
	my $trace_folder_path = concatenatePath($file_path, "..\\traces");
	
	debug("CreateOSTTraceDefinitionsHeader trace_folder_path: $trace_folder_path");
	
	mkdir($trace_folder_path);
	
	
	open FILE, ">$trace_folder_path\\OstTraceDefinitions.h" or die $!;
	
	print FILE "\#ifndef __OSTTRACEDEFINITIONS_H__\n" .
							"\#define __OSTTRACEDEFINITIONS_H__\n" .
							"// OST_TRACE_COMPILER_IN_USE flag has been added by Trace Compiler\n" .
							"// REMOVE BEFORE CHECK-IN TO VERSION CONTROL\n" .
							"//\#define OST_TRACE_COMPILER_IN_USE\n" .
							"\#include <OpenSystemTrace.h>\n" .
							"\#endif\n";

	
	close FILE;
}


#-------------------------------------------------------
# Get format text
#-------------------------------------------------------
sub GetFormatText
{
	my ($trace_id_name, @params) = @_;
	
	my $format_text = "";
	
	foreach my $param (@params)
	{
		if ($param =~ /(\w+)[\s\)]*$/)
		{
			$param = $1;
		}
		
		my $type = $map_trace_to_parameters{$trace_id_name}{$param};
		
		if (not defined $type or $type eq "" or $type eq "-DEC")
		{
			$format_text .= " %d";
		}
		elsif ($type eq "DEC")
		{
			$format_text .= " %u";
		}
		elsif ($type eq "HEX")
		{
			$format_text .= " 0x%x";
		}
		elsif ($type eq "STR")
		{
			$format_text .= " %s";
		}
	}
	
	return $format_text;
}

#-------------------------------------------------------
# Is string type
#-------------------------------------------------------
sub IsStringType
{
	my ($trace_id_name, @params) = @_;
	
	my $type_string = 0;
	
	foreach my $param (@params)
	{
		if ($param =~ /(\w+)[\s\)]*$/)
		{
			$param = $1;
		}
		
		my $type = $map_trace_to_parameters{$trace_id_name}{$param};
		
		if (defined $type and $type eq "STR")
		{
			$type_string = 1;
		}
	}
	
	return $type_string;
}

#-------------------------------------------------------
# Remove linebreaks from traces
#-------------------------------------------------------
sub RemoveLineBreaksFromTracesInFile
{
	my ($file, $convert_back_to_printf) = @_;
	
	my $file_changed = 0;
	my $new_file_content;
	my $previous_line_changed = 0;
	my $convert_macro = "Kern::Printf";
	
	if ($convert_back_to_printf == 1)
	{
		$convert_macro = "OstTrace([01]|Ext|Data)";
	}
	
	open FILE, "<$file" or die $!;
	
	foreach my $line (<FILE>)
	{
		chomp($line);
			
		if ($line =~ /SymbianTrace([0-2]|Data[0-1]{0,1}|Thread[0-2]|ThreadData[0-1]{0,1}|IsTraceActive|Pc[0-2]|ThreadPc[0-2]|PcData[0-1]{0,1}|ThreadPcData[0-1]{0,1}|StartControlBlock|EndControlBlock|Ext[1-6]|ExtThread[1-6])[^;]+$/ or
				$line =~ /$convert_macro[^;]+$/ or
				$line =~ /(BUILD_TRACE|ASSERT_TRACE|ASSERT_ALWAYS_TRACE|API_TRACE|INTERNAL_TRACE|DATA_DUMP_TRACE|ISIMSG_API_TRACE)[^;]+$/)
		{
			# We have a trace in multiple lines
			$new_file_content .= $line;
			
			$file_changed = 1;
			$previous_line_changed = 1;
		}
		else
		{
			if ($previous_line_changed == 1)
			{
				$line =~ s/\s*(.*)/ $1/;
				
				$new_file_content .= $line;
			}
			else
			{
				$new_file_content .= $line . "\n";
			}
			
			if ($previous_line_changed and $line =~ /;/)
			{
				$new_file_content .= "\n";
				$previous_line_changed = 0;
			}
		}
	}
	
	close FILE;
	
	if ($file_changed == 1)
	{
		open FILE, ">$file" or die $!;
	
		print FILE $new_file_content;
		
		close FILE;
	}
}

#-------------------------------------------------------
# Get new CASW trace name
#-------------------------------------------------------
sub GetNewCASWTraceName
{
	my ($old_trace_name, $paramater_count) = @_;
	
	my $new_trace_name;
	
	if ($old_trace_name eq "BUILD_TRACE" or 
			$old_trace_name eq "ASSERT_TRACE" or
			$old_trace_name eq "ASSERT_ALWAYS_TRACE" or
			$old_trace_name eq "API_TRACE" or
			$old_trace_name eq "INTERNAL_TRACE")
	{
		if ($paramater_count <= 0)
		{
			$new_trace_name = "OstTrace0";
		}
		elsif ($paramater_count <= 5)
		{
			$new_trace_name = "OstTraceExt" . $paramater_count;
		}
		else
		{
			$new_trace_name = "// TODO: Cannot convert trace. Too much parameters.";
		}
	}
	elsif ( $old_trace_name eq "DATA_DUMP_TRACE" or 
					$old_trace_name eq "ISIMSG_API_TRACE")
	{
		$new_trace_name = "OstTraceData";
	}
	
	return $new_trace_name;
}

#-------------------------------------------------------
# Get new Symbian trace name
#-------------------------------------------------------
sub GetNewSymbianTraceName
{
	my ($old_trace_name, $type_string) = @_;
	
	my $new_trace_name;
	
	if ($old_trace_name eq "SymbianTrace0" or 
			$old_trace_name eq "SymbianTraceThread0")
	{
		$new_trace_name = "OstTrace0";
	}
	elsif (	not $type_string and 
					( $old_trace_name eq "SymbianTrace1" or 
						$old_trace_name eq "SymbianTraceThread1" or
						$old_trace_name eq "SymbianTraceExt1" or
						$old_trace_name eq "SymbianTraceExtThread1"))
	{
		$new_trace_name = "OstTrace1";
	}
	elsif (	$type_string and 
					( $old_trace_name eq "SymbianTrace1" or 
						$old_trace_name eq "SymbianTraceThread1" or
						$old_trace_name eq "SymbianTraceExt1" or
						$old_trace_name eq "SymbianTraceExtThread1"))
	{
		$new_trace_name = "OstTraceExt1";
	}
	elsif (	$old_trace_name eq "SymbianTrace2" or 
					$old_trace_name eq "SymbianTraceThread2" or
					$old_trace_name eq "SymbianTraceExt2" or
					$old_trace_name eq "SymbianTraceExtThread2")
	{
		$new_trace_name = "OstTraceExt2";
	}
	elsif (	$old_trace_name eq "SymbianTraceExt3" or
					$old_trace_name eq "SymbianTraceExtThread3")
	{
		$new_trace_name = "OstTraceExt3";
	}
	elsif (	$old_trace_name eq "SymbianTraceExt4" or
					$old_trace_name eq "SymbianTraceExtThread4")
	{
		$new_trace_name = "OstTraceExt4";
	}
	elsif (	$old_trace_name eq "SymbianTraceExt5" or
					$old_trace_name eq "SymbianTraceExtThread5")
	{
		$new_trace_name = "OstTraceExt5";
	}
	elsif (	$old_trace_name eq "SymbianTraceExt6" or
					$old_trace_name eq "SymbianTraceExtThread6")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceExt6";
	}
	elsif (	$old_trace_name eq "SymbianTraceExt7" or
					$old_trace_name eq "SymbianTraceExtThread7")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceExt7";
	}
	elsif (	$old_trace_name eq "SymbianTraceData" or 
					$old_trace_name eq "SymbianTraceData0" or 
					$old_trace_name eq "SymbianTraceThreadData" or 
					$old_trace_name eq "SymbianTraceThreadData0")
	{
		$new_trace_name = "OstTraceData";
	}	
	elsif ($old_trace_name eq "SymbianTraceData1")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceData1";
	}
	elsif ($old_trace_name eq "SymbianTraceIsTraceActive")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceIsTraceActive";
	}
	elsif (	$old_trace_name eq "SymbianTracePc0" or
					$old_trace_name eq "SymbianTracePc1" or
					$old_trace_name eq "SymbianTracePc2")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTracePc";
	}
	elsif (	$old_trace_name eq "SymbianTraceThreadPc0" or
					$old_trace_name eq "SymbianTraceThreadPc1" or
					$old_trace_name eq "SymbianTraceThreadPc2")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceThreadPc";
	}
	elsif (	$old_trace_name eq "SymbianTracePcData" or
					$old_trace_name eq "SymbianTracePcData0" or
					$old_trace_name eq "SymbianTracePcData1")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTracePcData";
	}
	elsif (	$old_trace_name eq "SymbianTraceThreadPcData" or
					$old_trace_name eq "SymbianTraceThreadPcData0" or
					$old_trace_name eq "SymbianTraceThreadPcData1")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceThreadPcData";
	}
	elsif (	$old_trace_name eq "SymbianTraceStartControlBlock")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceStartControlBlock";
	}
	elsif (	$old_trace_name eq "SymbianTraceEndControlBlock")
	{
		$new_trace_name = "// TODO: Cannot convert SymbianTraceEndControlBlock";
	}
	
	return $new_trace_name;
}

#-------------------------------------------------------
# Get group names
#-------------------------------------------------------
sub getGroupTraceData
{
	my ($map_trace_to_group, $map_trace_to_text, $map_trace_to_parameters, $traces_folder) = @_;
	
	debug("\n\ngetGroupTraceData starts with: $traces_folder");
	
	# Get path to traces folder
	my $mmp_file = $ARGV[0];
	my $file_path = $mmp_file;
  $file_path =~ s/\\[^\\]+$/\\/i;
  $file_path =~ s/\\\\/\\/g;
  if ($file_path =~ m/\.mmp/i)
  {
  	debug("getGroupTraceData file doesn't have path. Use current dir.");
  	my $dir = "$Bin";
  	$file_path = $dir;
  	$file_path =~ s/\//\\/g;
  } 
  
	my $trace_folder_path = concatenatePath($file_path, $traces_folder);
	
	debug("getGroupTraceData MMP file: $mmp_file");
	debug("getGroupTraceData folder path: $trace_folder_path");
	
	if (not -e $trace_folder_path)
	{
		return;
	}
	
	# Get all header files in traces folder
	my $dir_return = qx("dir $trace_folder_path\\*.h /b");
	my @header_files = split("\n", $dir_return);
	
	my $found_header = 0;
	
	foreach my $header_file (@header_files)
	{
		debug("getGroupTraceData file: $header_file");
		open FILE, "<$trace_folder_path\\$header_file" or die $!;
		
		debug("getGroupTraceData file: Found header!");
		$found_header = 1;
		
		my $trace_id_name;
		my $trace_group_name;
		
		foreach my $line (<FILE>)
		{
			if ($line =~ /\s*TRACE_GROUP_BEGIN\(\s*(.+?)[\s,]/)
			{
				$trace_group_name = $1;
			}
						
			if ($line =~ /VALUE\s*\(\s*(-*)\d\d\s*,.+?,\s*(.+?),.*?,\s*(.+?)[\s\)]/)
			{
				my $sign = $1;
				my $variable = $2;
				my $type = $3;
				
				$map_trace_to_parameters{$trace_id_name}{$variable} = $1 . $type;
				
				#print $variable . " => " . $type . "\n";
			}
			
			if ($line =~ /STRING_REF\s*\(\s*.+?\s*,.+?,\s*(.+?),.*?,\s*.+?[\s\)]/)
			{
				my $variable = $1;
				
				$map_trace_to_parameters{$trace_id_name}{$variable} = "STR";
			}
			
			if ($line =~ /\s*TRACE_GROUP_END/)
			{
				$trace_group_name = "";
			}

			if ($line =~ /TRACE_BEGIN\(\s*(.+?)[\s,]+\"(.*)\"/)
			{
				$trace_id_name = $1;
				my $trace_text = $2;
				
				$trace_text =~ s/\%//g;
				
				if ($trace_group_name eq "")
				{
					print STDERR "Invalid trace definition header file: $header_file\nMost likely bug in this script...";
				}
				else
				{
					#print "$trace_id_name => $trace_group_name\n";
					#print "$trace_id_name => $trace_text\n";
				
					$map_trace_to_group{$trace_id_name} = $trace_group_name;
					$map_trace_to_text{$trace_id_name} = $trace_text;
				}
			}
		}
		
		close FILE;
	}
	
	return $found_header;
}

#-------------------------------------------------------
# Convert OST traces to PRINTF traces
#-------------------------------------------------------
sub convertOstToPrintf
{
	my ($macro, @sources) = @_;
	debug("convertOstToPrintf starts, macro is: $macro");
	
	# Go through all found source files
	foreach my $source_file (@sources)
	{
		debug("Source file $source_file");
		print $source_file . "\n";
		
		RemoveLineBreaksFromTracesInFile($source_file, 1);
		
		open FILE, "<$source_file" or die $!;
		
		my $new_file_content;
		
		my $traces_converted = 0;
		my $file_name = "";
		my $last_include_line = 0;
		my $line_number = 0;
		my $traceNumber = 0;
	
		if ($source_file =~ /.*\\(.+?)\..+/)
		{
			$file_name = $1;
		}
		
		foreach my $line (<FILE>)
		{
			$line_number += 1;
			
			my $line_converted = 0;
			
			chomp $line;
			
			if ($line =~ /^\s*\#include/)
			{
				$last_include_line = $line_number; 
			}
			
			if ($line =~ /Traces\.h/)
			{
				$line = "// " . $line;
			}
			
			# Printf macro
			if ($line =~ /^\s*OstTrace.*;/i)
			{		
				my $spaces;				
				my $trace = "";
				if ($line =~ /^(\s*)/)
				{
					$spaces = $1;
				}
				
				$line =~ s/^\s*(OstTrace.+;).*/$1/;
				
				# Remove spaces from the beginning
				$trace = $line;
				$trace =~ s/^\s*//g;
				
				print("Trace: " . $line . "\n");
				
				if (GetBracketCount($line) % 2 == 0)
				{									
					$line = $spaces . "// " . $trace ."\n";
					$line .= $spaces;
					
					# Remove the macro from the trace
					debug("Trace before removing MACRO $trace");
					
					# Check if Function Entry or Exit trace
					if ($trace =~ /^\s*OstTraceFunction.*;/i)
					{
						$trace =~ /\(\s*(.*?)([,\s\)])/;
						$trace = "\"" . $1 . "\"";		
					}
					
					# Other OST traces
					else
					{
						$trace =~ /\((.*?,)(.*?,)\s?(.*)\)/;
						$trace = $3;
					}
					debug("Trace after removing MACRO $trace");
					
					# Convert
					$line .= $macro . "(" . $trace . ");";
					
					$new_file_content .= $line . "\n";
					$line_converted = 1;
					$traces_converted = 1;
					$traceNumber++;
				}
			}
			else
			{
				$new_file_content .= $line . "\n";
			}
		}
	
		close FILE;	
		
		if ($traces_converted == 1)
		{	
			debug("\n\nSave new source file");
			SaveNewSourceFile($source_file, $new_file_content, $last_include_line, $file_name, 0);
		}
	}	
}

#-------------------------------------------------------
# Get source files
#-------------------------------------------------------
sub getSourceFiles
{
	debug("getSourceFiles starts");
	my @sources;
	
	# Open mmp file
	foreach my $mmp_file (@ARGV)
	{
		debug("getSourceFiles MMP file: $mmp_file");
		my $file_path = $mmp_file;
		$file_path =~ s/\\[^\\]+$/\\/i;
		$file_path =~ s/\\\\/\\/g;
	  if ($file_path =~ m/\.mmp/i)
	  {
	  	debug("getSourceFiles file doesn't have path. Use current dir");
	  	my $dir = "$Bin";
	  	$file_path = $dir;
	  	$file_path =~ s/\//\\/g;
	  } 
	
		# print "File path: $file_path\n";
		
		my $module_name = $mmp_file;
		$module_name =~ s/.*\\([^\.]*).*/$1/;
		
		# print "Module name: $module_name\n";
		
		my $uid = 0;
		my $current_src_path;
		
		if (-e $mmp_file)
		{
			debug("getSourceFiles MMP file exists");
			# Go through lines
			open FILE, "<$mmp_file" or die $!;
			foreach my $line (<FILE>)
			{
				$line =~ s/\//\\/g;
				
				# Find uid
				if ($line =~ /uid.+0x([a-fA-F0-9]+)?/i)
			  {
			  	$uid = $1;
			  	debug("getSourceFiles Found UID: $uid");
			  	# print "Uid: $uid\n";
			  }
			   
			  # Find source path 
			  if ($line =~ /sourcepath\s+([^\s]+)/i)
			  {
			  	my $src_path = $1;
			  	
			  	# Check absolute path
			  	if ($src_path =~ /^[^\\]/)
					{
			  		$current_src_path = concatenatePath($file_path, $src_path . "\\");
			  	}
			  	else
			  	{
			  		$current_src_path = substr($file_path, 0, 2) . $src_path;
			  	}
			  	
			  	debug("getSourceFiles current src path: $current_src_path");
			  	# print "Current src path: $current_src_path\n";
			  }
			  
			  while ($line =~ /source\s+([^\s]+)/i)
			  {
			  	my $src = $1;
			  	
					my $src_path = concatenatePath($current_src_path, $src);
			  	
			  	if (-e $src_path)
			  	{
						push (@sources, $src_path);
			  		debug("getSourceFiles found source: $src_path");
			  		# print "SRC: $src_path\n";
			  	}
			  	
			  	$line =~ s/\Q$src//;
			  }
			}
			close FILE;
			
		}
	}
	
	return @sources;
}

#--------------------------------------------------------------
# Get operation
#--------------------------------------------------------------
sub get_operation
{
	print "\nSelect operation:\n";
	print "Convert Symbian traces to OST traces        (1)\n";
	print "Convert Kern::Printf traces to OST traces   (2)\n";
	print "Convert RDebug::Printf traces to OST traces (3)\n";
	print "Convert own MACRO to OST traces             (4)\n";
	print "Convert OST traces to Kern::Printf traces   (5)\n";
	print "Convert OST traces to RDebug::Printf traces (6)\n";
	  
  my $selection = <STDIN>;
  chomp $selection;
  
  if ($selection != 1 and
  		$selection != 2 and
  		$selection != 3 and
  		$selection != 4 and
  		$selection != 5 and
  		$selection != 6)
  {
  	print STDERR "Invalid input!\n";
  	die;		
	}
	
	if ($selection == 4)
	{
		print "\nGive the macro name (e.g. \"TRACE\" if your maco is like TRACE(\"Text\")\n";
		$selection = <STDIN>;
		chomp $selection;
	}
  
  return $selection;
}

#--------------------------------------------------------------
# Get traces folder
#--------------------------------------------------------------
sub get_traces_folder
{
	my $traces_folder;
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\traces"))
	{
		$traces_folder = "..\\..\\traces";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\traces"))
	{
		$traces_folder = "..\\traces";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\symbian_traces\\autogen"))
	{
		$traces_folder = "..\\..\\symbian_traces\\autogen";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\..\\symbian_traces\\autogen"))
	{
		$traces_folder = "..\\..\\..\\symbian_traces\\autogen";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\..\\..\\symbian_traces\\autogen"))
	{
		$traces_folder = "..\\..\\..\\..\\symbian_traces\\autogen";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\autogen"))
	{
		$traces_folder = "..\\..\\autogen";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\..\\autogen"))
	{
		$traces_folder = "..\\..\\..\\autogen";
	}
	
	if (not defined $traces_folder and getGroupTraceData(\%map_trace_to_group, \%map_trace_to_text, \%map_trace_to_parameters, "..\\..\\..\\..\\autogen"))
	{
		$traces_folder = "..\\..\\..\\..\\autogen";
	}
	
	if (not defined $traces_folder)
	{
		print STDERR "Cannot find traces folder...";
		exit;
	}
	return $traces_folder;
}


#-------------------------------------------------------
# Concatenate path
#-------------------------------------------------------
sub concatenatePath
{
	my ($concatenatePathBase, $concatenatePathFile) = @_;
	
	my $backCount = 0;
	
	# Find how many back references there are and remove them
	while ($concatenatePathFile =~ /\.\.\\/g) 
	{ 
		$backCount++ 
	}
	$concatenatePathFile =~ s/\.\.\\//g;
	
	# If there is \ in the end of the base remove it
	my $lastChar = chop($concatenatePathBase);
	if ($lastChar ne "\\")
	{
		$concatenatePathBase = "$concatenatePathBase$lastChar";
	}
	
	# Remove directories from the end of the path
	$concatenatePathBase = reverse($concatenatePathBase);
	for (my $i=0; $i<$backCount; $i++)
	{
		$concatenatePathBase =~ s/.*?\\//;
	}
	$concatenatePathBase = reverse($concatenatePathBase);
	
	my $concatenatePathFullFilePath = "$concatenatePathBase\\$concatenatePathFile";
	
	return $concatenatePathFullFilePath;	
}


#-------------------------------------------------------
# Debug print
#-------------------------------------------------------
sub debug
{
	my ($print) = @_;
	
	if ($DEBUG)
	{
		print $print . "\n";
	}
}

warn "\n";
warn "Warning: this script is not supported and the Dynamic Analysis Tools team\n";
warn "does NOT promise to fix any bugs or add any functionality to it.\n";
warn "\n";