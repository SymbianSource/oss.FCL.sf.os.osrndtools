// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Trace Data parser
//

#ifndef __TRACEDATAPARSER_H__
#define __TRACEDATAPARSER_H__

#include <e32base.h>
#include <opensystemtrace.h>
#include <bautils.h> 

// Constants
const TUint32 KHeaderVersion            = 0x01;
const TUint32 KProtocolIdAscii          = 0x02;
const TUint32 KProtocolIdSimpleTrace    = 0x03;
const TUint32 KXtiHeaderVersion         = 0x1D;
const TUint32 KXtiProtocolIdSimpleTrace = 0x08;
const TInt KXtiTraceTypeIndex           = 20;
const TInt KXtiLengthIndex              = 5;
const TInt KMinimumPrimingTracesInBatch = 2;
const TInt KMinMilliSecondsBatchGap     = 10;
const TInt KExpectedPrintfMaxLength     = 32;
const TInt KFileBufferSize              = 1024*10;
const TInt KMaxNumberBufferLength       = 8;
const TInt KMinSizeOstHeader            = 3;
const TInt KMinSizeOstBinaryHeader      = 8;
const TInt KMinSizeOstAsciiHeader       = 8;
const TInt KMinSizeBinaryHeader         = 12;
const TInt KMinSizeXtiHeader            = 20;

// Literals
_LIT8(KDroppedTrace,            "* Dropped Trace");
_LIT8(KFastMutexNameSystemLock, "System Lock");
_LIT8(KFastMutexNameMsgLock,    "MsgLock");
_LIT8(KFastMutexNameObjLock,    "ObjLock");
_LIT8(KFastMutexNameLogonLock,  "LogonLock");
_LIT8(KCodeSegsName1,           "EKern.exe");
_LIT8(KCodeSegsName2,           "EFile.exe");


// Class containing trace header settings
class TTraceHeaderSettings
{
public:
    TUint8          iCategory;
    TUint8          iSubCategory;
    TComponentId    iComponentID;
    TUint32         iTraceWord;
    TUint32         iTimestamp;
    TInt            iLengthOfPayload;
    TUint8          iHeaderFlags;
    TInt            iMultiPartType;
    TBool           iPrintfTrace;
    TBool           iFromTestWriter;
    TUint32         iMultiPartTraceID;
};

/*
 * Trace Data Parser class of static parsing functions
 * 
 * The following trace types are supported:
 *     OST traces (ascii and binary)
 *     XTIv3 traces (ascii and binary)
 *     Traces from the Test Writer (ascii and binary)
 */
class TTraceDataParser
{
public:

    IMPORT_C static TUint32 Swap(TUint32 x);
    IMPORT_C static TUint32 ReadUint32FromBuf(TUint8*& aBuf, TBool aFromTestWriter = EFalse);
    IMPORT_C static TUint16 ReadUint16FromBuf(TUint8*& aBuf);

    
/*
 * Validate the trace data in buffer
 *
 * @param aBuffer the buffer containing the trace data
 * @param aExpectedNum expected number of traces
 * @param aComponentID component ID of traces
 * @param aGroupID group ID of traces
 * @param aData expected trace data
 * @param aTraceWord trace word of traces
 * @param aIdentical indicates that the payload of each trace packet should be identical
 * @param aBufferMode The mode of the trace buffer.. 0=straight, 1=circular.
 * @param aOrdinalData indicates whether the payload data reflects its position in the stream of trace packets
 * @param aNumTraces number of traces written to circular buffer
 * @param aMaxTraces maximum number of traces circular buffer can hold
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt ValidatePayload(TDesC8&        aBuffer,
                                         TInt           aExpectedNum,
                                         TComponentId   aComponentID,
                                         TGroupId       aGroupID, 
                                         TUint32        aData, 
                                         TUint32        aTraceWord,
                                         TBool          aIdentical      =EFalse, 
                                         TInt           aBufferMode     =0, 
                                         TBool          aOrdinalData    =EFalse,
                                         TInt           aNumTraces      =0,
                                         TInt           aMaxTraces      =0);

/*
 * Validate the single trace data in buffer
 * 
 * @param aBuffer the buffer containing the trace data
 * @param aTracePresent determines if trace data should be present or not
 * @param aGroupID expected GID
 * @param aComponentID expected CID
 * @param aData expected trace data
 * @param aPrintfTrace determines if trace data should be printf data or not
 * @param aMissingTrace determines if trace data should indicate missing data or not
 * @param aExpectedPrintfTrace expected printf trace data
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt ValidatePayload(TDesC8&        aBuffer, 
                                         TBool          aTracePresent, 
                                         TGroupId       aGroupID, 
                                         TComponentId   aComponentID, 
                                         TUint32        aData,
                                         TBool          aPrintfTrace         =EFalse,
                                         TBool          aMissingTrace        =EFalse,
                                         TDesC8*        aExpectedPrintfTrace =NULL);

/*
 * Parse trace data for kernel priming data
 * 
 * Note that this function can only verify priming data if the group ID is EThreadIdentification, EFastMutex or ECodeSegs,
 * otherwise KErrNotSupported is returned if aVerifyData is ETrue
 *
 * @param aBuffer Buffer containing the trace data
 * @param aGroupId Group ID of traces to parse
 * @param aNumPrimingTraces Output parameter set to number of priming traces found for given group ID
 * @param aNumTraces Output parameter set to total number of traces found for given group ID
 * @param aVerifyData Flag indicating if priming data should be verified
 * @param aVerificationData1 Optional first data required for verifying priming data
 * @param aVerificationData1 Optional second data required for verifying priming data
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt ParsePrimingDataL(TDesC8&       aBuffer,
                                           TGroupId      aGroupId,
                                           TInt&         aNumPrimingTraces,
                                           TInt&         aNumTraces,
                                           TBool         aVerifyData         = EFalse, 
                                           TUint32       aVerificationData1  = 0, 
                                           TUint32       aVerificationData2  = 0);

/*
 * Parse trace data for kernel priming data
 * 
 * Note that this function can only verify priming data if the group ID is EThreadIdentification, EFastMutex or ECodeSegs,
 * otherwise KErrNotSupported is returned if aVerifyData is ETrue
 *
 * @param aFilePath Full path of file containing the trace data
 * @param aFs File system object
 * @param aGroupId Group ID of traces to parse
 * @param aNumPrimingTraces Output parameter set to number of priming traces found for given group ID
 * @param aNumTraces Output parameter set to total number of traces found for given group ID
 * @param aVerifyData Flag indicating if priming data should be verified
 * @param aVerificationData1 Optional first data required for verifying priming data
 * @param aVerificationData1 Optional second data required for verifying priming data
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt ParsePrimingDataL(const TDesC&  aFilePath,
                                           RFs&          aFs,
                                           TGroupId      aGroupId,
                                           TInt&         aNumPrimingTraces,
                                           TInt&         aNumTraces,
                                           TBool         aVerifyData         = EFalse, 
                                           TUint32       aVerificationData1  = 0, 
                                           TUint32       aVerificationData2  = 0);

/*
 * Parse trace data for Printf data, returning the number of occurances of a given string
 *
 * @param aBuffer Buffer containing the trace data
 * @param aFindString String to search for
 * @param aNumFound Output parameter set to number of occurances of given string
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt DataHasPrintfString(TDesC8&        aBuffer,
                                             const TDesC8&  aFindString,
                                             TInt&          aNumFound);

    
/*
 * Parse trace data file for Printf data, returning the number of occurances of a given string
 *
 * @param aFilePath Full path of file containing the trace data
 * @param aFs File system object
 * @param aFindString String to search for
 * @param aNumFound Output parameter set to number of occurances of given string
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt DataHasPrintfStringL(const TDesC&   aFilePath,
                                              RFs&           aFs,
                                              const TDesC8&  aFindString,
                                              TInt&          aNumFound);

    
/*
 * Parse trace data file for a sequence of traces
 *
 * @param aFilePath Full path of file containing the trace data
 * @param aFs File system object
 * @param aLastNumberFound Output parameter set to number in last occurances of given string
 * @param aNumDroppedTraces Output parameter set to number of dropped traces
 * @param aFindPrintfPattern Pattern that indicates where in the printf string the number will be, using an asterisk
 *                           This is only used if aGroupId = BTrace::ERDebugPrintf
 * @param aGroupId Group ID of traces to parse
 * @param aComponentID Component ID of traces to parse
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt DataHasTraceSequenceL(const TDesC&   aFilePath,
                                               RFs&           aFs,
                                               TInt&          aLastNumberFound,
                                               TInt&          aNumDroppedTraces, 
                                               TDesC8*        aFindPrintfPattern,
                                               TGroupId       aGroupId = BTrace::ERDebugPrintf,
                                               TComponentId   aComponentID = 0);

/*
 * Get Printf string from single trace data
 *
 * @param aBuffer Buffer containing the trace data
 * @param aPrintfString Output parameter set to the print string in the trace data
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt GetPrintfFromTrace(TDesC8& aBuffer, TDes8& aPrintfString);

/*
 * Parse trace data for multipart data, returning the number of traces found and total size of trace data
 * The data is expected to contain a sequence of integers, starting at 0
 *
 * @param aBuffer Buffer containing the trace data
 * @param aGroupId Group ID of traces to parse
 * @param aComponentID Component ID of traces to parse
 * @param aMultipartDataSize Output parameter set to total size of data in multipart trace
 * @param aNumMultipartTraces Output parameter set to number of traces that make up multipart trace
 * @return Standard Symbian error code
 */
    IMPORT_C static TInt ValidateMultipartTraces(TDesC8&        aBuffer,
                                                 TGroupId       aGroupID, 
                                                 TComponentId   aComponentID, 
                                                 TInt&          aMultipartDataSize, 
                                                 TInt&          aNumMultipartTraces);

private:

    static TBool IsStartOfKernelPrimingBatch(TUint8* aBuffer, TGroupId aGroupId, TUint8 aCategory, TUint8 aSubCategory);
    static TInt IsPotentialPrimingTrace(TUint8 aCategory, TUint8 aSubCategory, TBool& aIsPotentialPrimingTrace);
    static TBool StringsMatch(const TDesC8& aString1, const TDesC8& aString2);
    static TPtr8 ReadTracePrintf(TUint8* aData, TInt aSize);
    static TInt ParseStringForNumber(const TDesC8& aBuffer, const TDesC8& aFindStringPattern, TInt& aNumberFound);

    static TInt ParseHeader(TUint8*&                aData, 
                            TInt                    aSize, 
                            TTraceHeaderSettings&   aTraceHeaderSettings);
    
    static TInt IsInPrimingBatch(TUint8  aCategory, 
                                 TUint8  aSubCategory, 
                                 TUint32 aTimeDifference, 
                                 TBool&  aIsInPrimingBatch, 
                                 TBool&  aFirstTrace, 
                                 TBool&  aStartOfBatch);

    static TInt GetPrimingVerificationDataL(TUint8*  aBuffer, 
                                            TInt     aSize, 
                                            TGroupId aGroupId, 
                                            TUint32& aVerificationData1, 
                                            TUint32& aVerificationData2,
                                            RFile*   aFile = NULL);

    static TInt GetThreadPrimingVerificationDataL(TUint8*  aBuffer, 
                                                  TInt     aSize, 
                                                  TUint32& aThreadAddr, 
                                                  TUint32& aProcessAddr,
                                                  RFile*   aFile = NULL);

    static TInt GetCodeSegsVerificationDataL(TUint8*  aBuffer, 
                                             TInt     aSize, 
                                             TUint32& aSegAddr1, 
                                             TUint32& aSegAddr2,
                                             RFile*   aFile = NULL);

    static TInt VerifyPrimingData(TUint8* aBuffer, 
                                  TInt    aSize, 
                                  TUint8  aCategory, 
                                  TUint8  aSubCategory, 
                                  TInt&   aElementsFound,
                                  TBool   aFromTestWriter, 
                                  TUint32 aVerificationData1, 
                                  TUint32 aVerificationData2);    

    static TInt VerifyThreadPrimingData(TUint8* aBuffer, 
                                        TInt    aSize, 
                                        TUint8  aSubCategory, 
                                        TInt&   aElementsFound,
                                        TBool   aFromTestWriter, 
                                        TUint32 aThreadAddr, 
                                        TUint32 aProcessAddr);    

    static TInt VerifyFastMutexPrimingData(TUint8* aBuffer, 
                                           TInt    aSize, 
                                           TUint8  aSubCategory, 
                                           TInt&   aElementsFound,
                                           TBool   aFromTestWriter);    

    static TInt VerifyCodeSegsPrimingData(TUint8* aBuffer, 
                                          TInt    aSize, 
                                          TUint8  aSubCategory, 
                                          TInt&   aElementsFound,
                                          TBool   aFromTestWriter, 
                                          TUint32 aSegAddr1, 
                                          TUint32 aSegAddr2);    

    static TInt ReadNextChunkFromFile(TDes8&   aFileBuffer, 
                                      RFile&   aFile, 
                                      TInt&    aFilePosition,
                                      TUint8*& aData,
                                      TUint8*& aStartOfData,
                                      TUint8*& aEndOfData);

    static TInt CreateFileBuffer(RBuf8&  aFileBuffer, 
                                 RFile&  aFile);
    
};

#endif // __TRACEDATAPARSER_H__

