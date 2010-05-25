/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Common declarations/definitions for Analyze Tool.
*
*/



#ifndef __ANALYZETOOLTRACECONSTANTS_H__
#define __ANALYZETOOLTRACECONSTANTS_H__

#include <analyzetool/atcommon.h>

// Trace version information.
const TUint KATTraceVersion = 0x2;

// The default logging mode 
const TATLogOption KDefaultLoggingMode = EATLogToTrace;

// When needed, update the storage data file's version number directly inside
// the _LIT8 macro. Note, if you change this string, also remember to update
// the constant "KVersionStringLength" below.

_LIT8( KDataFileVersion, "DATA_FILE_VERSION 11\r\n" );
_LIT8( KProcessStart, "PROCESS_START %S %x " );
_LIT16( KProcessStart16, "PROCESS_START %S %x " );
_LIT8( KDllLoad, "DLL_LOAD %S %Lx %x %x\r\n" );
_LIT16( KDllLoad16, "DLL_LOAD %S %Lx %x %x\r\n" );
_LIT8( KDllUnload, "DLL_UNLOAD %S %Lx %x %x\r\n" );
_LIT16( KDllUnload16, "DLL_UNLOAD %S %Lx %x %x\r\n" );
_LIT8( KProcessEnd, "PROCESS_END %x " );
_LIT16( KProcessEnd16, "PROCESS_END %x " );
_LIT8( KProcessEndAbnormal, "PROCESS_END %x ABNORMAL " );
_LIT8( KMemoryLeak, "MEM_LEAK " ); // Remember to update value of KMemoryLeak when changing this.
_LIT8( KHandleLeak, "HANDLE_LEAK %S %x\r\n" );
_LIT16( KHandleLeak16, "HANDLE_LEAK %S %x\r\n" );
_LIT8( KErrorOccured, "ERROR_OCCURED %d " );
_LIT8( KLoggingCancelled, "LOGGING_CANCELLED %x\r\n" );
_LIT8( KNewLine, "\r\n" );
_LIT8( KHexaNumber, " %x" );
_LIT8( KSpace, " " );
_LIT8( KUdeb, "UDEB" );
_LIT8( KUrel, "UREL" );
    
// Constants for logging through debug channel
_LIT( KTraceMessage, "PCSS %x %S" );
_LIT( KMemoryAllocHeader, "ALLOCH " );
_LIT( KMemoryAllocFragment, "ALLOCF " );
_LIT( KMemoryFreedHeader, "FREEH " );
_LIT( KMemoryFreedFragment, "FREEF " );
_LIT( KSpaceTrace, " " );
_LIT( KNewLineTrace, "\r\n" );
_LIT( KHexaNumberTrace, " %x" );
_LIT( KProcessEndTrace, "PROCESS_END %x " );
_LIT( KProcessEndAbnormalTrace, "PROCESS_END %x ABNORMAL " );
_LIT( KErrorOccuredTrace, "ERROR_OCCURED %d " );
_LIT( KSubtestStart, "TEST_START " );
_LIT( KSubtestEnd, "TEST_END " );
_LIT8( KEmpty, "" );
_LIT( KOpenSquareBracket, "[" );
_LIT( KCloseSquareBracket, "]" );
_LIT( KUnderLine, "_" );
// File name format
_LIT( KFormat, "%S%S%02d%S");// pad char="0", field width=2

// A string for setting time January 1st, 1970 AD nominal Gregorian
_LIT( KJanuaryFirst1970, "19700000:000000.000000" );

// Module name when it cannot be defined
_LIT8( KUnknownModule, "Unknown" );
_LIT16( KUnknownModule16, "Unknown" );

// Constant time variable used to calculate timestamps for pc side.
const TInt64  KMicroSecondsAt1970 = 62168256000000000;

// The length of the string KDataFileVersion
const TInt KVersionStringLength = 22;

// The length of the string KMemoryLeak
const TInt KMemleakLength = 9;

// The maximum length of one word (32 bits) represented in the hexadecimal text format
// without "0x" prefix
const TInt KHexa32Length = 8;

// The maximum length of one word (32 bits) represented in the decimal text format
const TInt KDec32Length = 10;

// The maximum length of a TInt64 represented in the hexadecimal text format without
// "0x" prefix
const TInt KHexa64Length = 16;

// The length of one space character in text
const TInt KSpaceLength = 1;

// The length of the combination of carriage return and new line characters.
const TInt KNewlineLength = 2;

// The maximum length of the "PROCESS_START..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KProcessStartBufLength = 16 + KMaxProcessName + KSpaceLength + KHexa32Length +
                                        KSpaceLength + KHexa64Length + KHexa32Length + KNewlineLength;

// The maximum length of the "DLL_LOAD..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KDllLoadBufLength = 9 + KMaxLibraryName + KSpaceLength + KHexa64Length + KSpaceLength +
                    KHexa32Length + KSpaceLength + KHexa32Length + KNewlineLength;

// The maximum length of the "DLL_UNLOAD..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KDllUnloadBufLength = 11 + KMaxLibraryName + KHexa64Length + KSpaceLength + KSpaceLength + 
                    KHexa32Length + KSpaceLength + KHexa32Length + KNewlineLength;

// The maximum length of the "PROCESS_END..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KProcessEndBufLength = 12 + KHexa32Length + KSpaceLength +
                                        KHexa64Length + KNewlineLength;
                                        
// The maximum length of the "ERROR_OCCURED..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KErrOccuredBufLength = 14 + KDec32Length + KSpaceLength +
                                        KHexa64Length + KNewlineLength;
                                        
// The maximum length of the "ALLOCH / ALLOCF..." buffer. 
const TInt KMemAllocBufLength = 255;

// The maximum length of the "FREE..." buffer.
const TInt KMemFreedBufLength = 255;

// The maximum length of the "HANDLE_LEAK..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KHandleLeakBufLength = 12 + KMaxLibraryName + KSpaceLength +
                                        KHexa32Length + KNewlineLength;
                                        
// The maximum length of the "TEST_START..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KTestStartBufLength = 11 + KHexa64Length + KSpaceLength +
                                        KATMaxSubtestIdLength + KSpaceLength + KHexa64Length + KNewlineLength;

// The maximum length of the "TEST_END..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KTestEndBufLength = 9 + KHexa64Length + KSpaceLength +
                                        KATMaxSubtestIdLength + KSpaceLength + KHexa64Length + KNewlineLength;

// The maximun length of the "LOGGING_CANCELLED..." buffer. The first number is the
// length of the line tag and one space character (see the descriptor constants above).
const TInt KCancelBufLength = 18 + KHexa64Length + KNewlineLength;

// The maximun length of the "PROCESS_END %x ABNORMAL..." buffer. The first number is length of
// the line tag and one space character (see the descriptor constants above).
const TInt KEndAbnormalBufLength = 22 + KHexa32Length + KHexa64Length + 
                                        KSpaceLength + KNewlineLength;

// The maximun length of the file name extension buffer.
const TInt KExtensionLength = 50;

// The maximun length of the process UID3 buffer.
const TInt KProcessUidLength = 20;

#endif // __ANALYZETOOLTRACECONSTANTS_H__
