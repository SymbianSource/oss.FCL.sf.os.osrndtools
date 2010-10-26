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
#!/user/bin/python
import os
import sys
import shutil
import glob
import stat

print "TraceCompiler Test Bench version 1.1\n"

# List of files to be generated. NOTE! Files must be in same order than reference files
generated_files = []
# TraceCompilerTester
generated_files.append(r".\TraceCompilerTester\traces\BigNumbersTraces.h")
generated_files.append(r".\TraceCompilerTester\traces\fixed_id.definitions")
generated_files.append(r".\TraceCompilerTester\traces\TraceCompilerTesterBasicTraces.h")
generated_files.append(r".\TraceCompilerTester\traces\TraceCompilerTesterCommon1Traces.h")
generated_files.append(r".\TraceCompilerTester\traces\TraceCompilerTesterCommon2Traces.h")
generated_files.append(r".\TraceCompilerTester\traces_TraceCompilerTesterAdvanced\BigNumbersTraces.h")
generated_files.append(r".\TraceCompilerTester\traces_TraceCompilerTesterAdvanced\fixed_id.definitions")
generated_files.append(r".\TraceCompilerTester\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterAdvancedTraces.h")
generated_files.append(r".\TraceCompilerTester\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterCommon1Traces.h")
generated_files.append(r".\TraceCompilerTester\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterCommon2Traces.h")
generated_files.append(r".\epoc32\ost_dictionaries\TraceCompilerTesterAdvanced_0x20011111_Dictionary.xml")
generated_files.append(r".\epoc32\ost_dictionaries\TraceCompilerTesterBasic_0x20011112_Dictionary.xml")
generated_files.append(r".\epoc32\include\platform\symbiantraces\autogen\TraceCompilerTesterAdvanced_0x20011111_TraceDefinitions.h")
generated_files.append(r".\epoc32\include\platform\symbiantraces\autogen\TraceCompilerTesterBasic_0x20011112_TraceDefinitions.h")
# HelloWorld
generated_files.append(r".\HelloWorld\traces\fixed_id.definitions")
generated_files.append(r".\HelloWorld\traces\HelloWorldTraces.h")
generated_files.append(r".\epoc32\ost_dictionaries\HelloWorld_0xe9fbe6ee_Dictionary.xml")
generated_files.append(r".\epoc32\include\platform\symbiantraces\autogen\HelloWorld_0xe9fbe6ee_TraceDefinitions.h")

# List of reference files. NOTE! Files must be in same order than generated files
reference_files = []
# TraceCompilerTester
reference_files.append(r".\TraceCompiler_reference_files\traces\BigNumbersTraces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces\fixed_id.definitions")
reference_files.append(r".\TraceCompiler_reference_files\traces\TraceCompilerTesterBasicTraces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces\TraceCompilerTesterCommon1Traces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces\TraceCompilerTesterCommon2Traces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces_TraceCompilerTesterAdvanced\BigNumbersTraces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces_TraceCompilerTesterAdvanced\fixed_id.definitions")
reference_files.append(r".\TraceCompiler_reference_files\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterAdvancedTraces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterCommon1Traces.h")
reference_files.append(r".\TraceCompiler_reference_files\traces_TraceCompilerTesterAdvanced\TraceCompilerTesterCommon2Traces.h")
reference_files.append(r".\TraceCompiler_reference_files\TraceCompilerTesterAdvanced_0x20011111_Dictionary.xml")
reference_files.append(r".\TraceCompiler_reference_files\TraceCompilerTesterBasic_0x20011112_Dictionary.xml")
reference_files.append(r".\TraceCompiler_reference_files\TraceCompilerTesterAdvanced_0x20011111_TraceDefinitions.h")
reference_files.append(r".\TraceCompiler_reference_files\TraceCompilerTesterBasic_0x20011112_TraceDefinitions.h")
# HelloWorld
reference_files.append(r".\HelloWorld_reference_files\traces\fixed_id.definitions")
reference_files.append(r".\HelloWorld_reference_files\traces\HelloWorldTraces.h")
reference_files.append(r".\HelloWorld_reference_files\HelloWorld_0xe9fbe6ee_Dictionary.xml")
reference_files.append(r".\HelloWorld_reference_files\HelloWorld_0xe9fbe6ee_TraceDefinitions.h")

# Name of report file that include differences between generated files and reference files.
report_file_name = "diff.txt"

def print_instructions():
    print "Common usage : TraceCompiler_Test.py <drive> <location_of_TraceCompiler>"
    print "  where <drive> is a letter of virtual drive which you want to assign"
    print "     a path of the TraceCompiler Test Bench."
    print "  where <location_of_TraceCompiler> is a location of the TraceCompiler"
    print "     that wanted to be test. Location should be the location of"
    print "     ""tracecompiler"" folder."
    print "\nExamples:";
    print "  TraceCompiler_Test.py W: .";
    print "      The TraceCompiler Test Bench uses W:\\ as virtual drive and tests"
    print "      the TraceCompiler that has been copied to root of the test bench.";
    print "  TraceCompiler_Test.py Y X:\\epoc32\\tools";
    print "      The TraceCompiler Test Bench uses Y:\\ as virtual drive and tests"
    print "      the TraceCompiler that can be found from X:\\epoc32\\tools folder.";

try:
    # Get drive name from command line arguments
    drive = sys.argv[1]
            
    # Check that drive name is in valid format
    if drive[0].isalpha() == False or len(drive) > 2 or (len(drive) == 2 and drive[1] != ":") :
        raise TypeError
    elif len(drive) <  2 :
        drive += ":"
        
    # Get location of the TraceCompiler to be tested from command line arguments
    location_of_tracecompiler = sys.argv[2]
    
    # Check if location of the TraceCompiler path valid
    tracecompiler_script = os.path.join(location_of_tracecompiler, r"tracecompiler.pl")
    if not os.path.exists(tracecompiler_script) :
        print "Error: TraceCompiler that should be tested does not exist in defined location"
        sys.exit()
                   
except (IndexError, TypeError) :
    print_instructions()
    sys.exit()
    
# Subst TraceCompiler Test Bench as specific drive
print "##### Subst " + drive + " drive #####"
os.system("subst " + drive + " .")

# Go to substed drive
os.chdir(drive)

# Verify that substed drive includes TraceCompiler Test Bench. Check is needed in case that 
# drive was already substed.
filename = os.path.join(drive, r"\TraceCompiler_Test.py")
if not os.path.exists(filename) :
    print "\nError: Substed drive does not include TraceCompiler test Bench"
    sys.exit()

# Path to folder where TraceCompiler under test will be copied
tracecompiler_under_test_path = os.path.abspath(r"\TraceCompiler_under_test")

# Remove old TraceCompiler under test if that exist
shutil.rmtree(tracecompiler_under_test_path, True)

# Copy TraceCompiler that should be tested to TraceCompiler Test Bench
os.mkdir(tracecompiler_under_test_path)
shutil.copy(tracecompiler_script, os.path.abspath(r"\TraceCompiler_under_test\tracecompiler.pl"))
shutil.copy(os.path.join(location_of_tracecompiler, r"tracecompiler.pm"), 
    os.path.abspath(r"\TraceCompiler_under_test\tracecompiler.pm"))
shutil.copy(os.path.join(location_of_tracecompiler, r"tracecompiler_parse_mmp.pl"), 
    os.path.abspath(r"\TraceCompiler_under_test\tracecompiler_parse_mmp.pl"))
shutil.copytree(os.path.join(location_of_tracecompiler, r"tracecompiler"),
    os.path.abspath(r"\TraceCompiler_under_test\tracecompiler"))
    
# Just in case remove all read-only attributes
for root, dirs, files in os.walk(tracecompiler_under_test_path, topdown=False):
    for fileName in files:
        os.chmod (os.path.join(root, fileName), stat.S_IWRITE)
    for dirName in dirs:
        os.chmod (os.path.join(root, dirName), stat.S_IWRITE)       

# Delete generated files
print "\n##### Delete all generated files #####"
for generated_file in generated_files :
    try :
        os.remove(generated_file)
    except :
        pass
        
# Delete diff.txt
try :
    os.remove(report_file_name)
except :
    pass

# Run TraceCompiler to TraceCompilerTesterBasic component
print "\n##### Run TraceCompiler to TraceCompilerTesterBasic component #####"
os.chdir("TraceCompilerTester\group")
os.system(r"java -classpath ../../TraceCompiler_under_test/tracecompiler com.nokia.tracecompiler.TraceCompiler 20011112 TraceCompilerTesterBasic ../../TraceCompilerTester/group/TraceCompilerTesterBasic.mmp ../../TraceCompilerTester/src/TraceCompilerTesterBasic.cpp ../../TraceCompilerTester/src/TraceCompilerTesterCommon1.cpp ../../TraceCompilerTester/src/TraceCompilerTesterCommon2.cpp")
os.chdir("..\..")

# Run TraceCompiler to TraceCompilerTesterAdvanced component
print "\n##### Run TraceCompiler to TraceCompilerTesterAdvanced component #####"
os.chdir("TraceCompilerTester\group")
os.system(r"java -classpath ../../TraceCompiler_under_test/tracecompiler com.nokia.tracecompiler.TraceCompiler 20011111 TraceCompilerTesterAdvanced ../../TraceCompilerTester/group/TraceCompilerTesterAdvanced.mmp ../../TraceCompilerTester/src/TraceCompilerTesterAdvanced.cpp ../../TraceCompilerTester/src/TraceCompilerTesterCommon1.cpp ../../TraceCompilerTester/src/TraceCompilerTesterCommon2.cpp")
os.chdir("..\..")

# Run TraceCompiler to HelloWorld component
print "\n##### Run TraceCompiler to HelloWorld component #####"
os.chdir("HelloWorld\group")
os.system(r"java -classpath ../../TraceCompiler_under_test/tracecompiler com.nokia.tracecompiler.TraceCompiler E9FBE6EE HelloWorld ../../HelloWorld/group/HelloWorld.mmp ../../HelloWorld/src/HelloWorld.cpp")
os.chdir("..\..")

# Compare generated files to reference files
print "\n##### Compare generated files to reference files #####"
changed_files = []
changes = []
file_index = 0
for generated_file in generated_files :
    gfile = open(generated_file, "r")
    rfile = open(reference_files[file_index], "r")
    filename_added = False

    for gline in gfile :
        rline = rfile.readline()
        if gline != rline :
            if not "// Created by TraceCompiler" in gline :
                valid = True
                
                # In case of dictionary file ignore differences in drive letters
                if generated_file.endswith(".xml") and "<path val=" in gline :
                    if gline[gline.find(r"/"):] == rline[rline.find(r"/"):] :
                        valid = False
                
                if valid :
                    if not filename_added :
                        changed_files.append(generated_file)
                        changes.append(50 * "-")
                        changes.append("\n")
                        changes.append(generated_file)
                        changes.append("\n\n")
                        filename_added = True
                    changes.append(gline)
                    changes.append(" <---> ")
                    changes.append("\n")
                    changes.append(rline)
                    changes.append("\n")
                
    file_index+=1

# Print results
print "\n##### TEST RESULTS #####"
if len(changed_files) == 0 :
    print "\nNO DIFFERENCES\n"  
else :
    print "\a"
    print "DIFFERENCES TO REFERENCE FILES NOTICED!\n"
    
    # Write changes to diff.txt
    report = open(report_file_name, "w")
    report.writelines(changes)
    
    print "There are differences in following files:"
    for changed_file in changed_files :
        print changed_file
    print "\nFor more information, please see the diff.txt."
    print "\nNOTE: If differences are ok, remember to update reference files."  
     
    