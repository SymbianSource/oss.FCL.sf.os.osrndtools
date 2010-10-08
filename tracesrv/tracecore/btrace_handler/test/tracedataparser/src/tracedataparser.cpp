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

#include <e32svr.h>
#include <e32def.h>
#include <e32def_private.h>
#include <e32btrace.h>

#include "tracedataparser.h"
#include "testdatawriternotifier.h"
#include "tracecoreconstants.h"
#include "d32tracebuffer.h"


EXPORT_C TUint32 TTraceDataParser::Swap(TUint32 x)
    {
    return  (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
    }

EXPORT_C TUint32 TTraceDataParser::ReadUint32FromBuf(TUint8*& aBuf, TBool aFromTestWriter)
    {
#ifdef __WINS__
    aFromTestWriter = EFalse;
#endif
    // reads a 4 byte integer of expected endianess
    TUint32 n;
    if (!aFromTestWriter)
        {
        n = *(aBuf+3) + (*(aBuf+2) << 8) + (*(aBuf+1) << 16) + (*aBuf << 24);
        }
    else
        {
        // endianess order is reversed for TestWriter on hw
        n = (*aBuf) + (*(aBuf+1) << 8) + (*(aBuf+2) << 16) + (*(aBuf+3) << 24);    
        }
    aBuf += sizeof(TUint32);
    return n;
    }

EXPORT_C TUint16 TTraceDataParser::ReadUint16FromBuf(TUint8*& aBuf)
    {
    // reads a 2 byte integer of expected endianess
    TUint16 n = (*aBuf << 8)+ (*(aBuf+1));
    aBuf += sizeof(TUint16);
    return n;
    }

TPtr8 TTraceDataParser::ReadTracePrintf(TUint8* aData, TInt aSize)
    {
    TPtr8 printfString(aData, aSize, aSize);
    TInt endPosition = printfString.Locate(TChar(0));
    if (endPosition >= 0)
        {
        printfString = printfString.Left(endPosition);
        }
    else
        {
        printfString.Trim();
        }
    return printfString;
    }

TBool TTraceDataParser::StringsMatch(const TDesC8& aString1, const TDesC8& aString2)
    {
    TInt compareLength = (aString1.Length() > aString2.Length()) ? aString2.Length() : aString1.Length();
    if ( (aString2.Left(compareLength).Compare(aString1) == 0) ||
         (aString1.Left(compareLength).Compare(aString2) == 0) )
        {
        return ETrue;
        }
    return EFalse;
    }

/**
 * Function to string for a number
 * @param aBuffer Trace data descriptor
 * @param aFindStringPattern Pattern that indicates where in the string the number will be, using an asterisk
 * @param aNumberFound If found, the number in the string
 * @return Symbian error code
 */
TInt TTraceDataParser::ParseStringForNumber(const TDesC8& aBuffer, const TDesC8& aFindStringPattern, TInt& aNumberFound)
    {
    TInt err = KErrNotFound;
    TInt posOfNum = aFindStringPattern.Locate('*');
    TInt lengthOfNum = 1 + aBuffer.Length() - aFindStringPattern.Length();
    if (posOfNum >= 0 && lengthOfNum >= 1)
        {
        TBuf<KMaxNumberBufferLength> numberBuffer;
        numberBuffer.Copy(aBuffer.Mid(posOfNum, lengthOfNum));
        TLex lex(numberBuffer);
        err = lex.Val(aNumberFound);
        }
    return err;
    }

/*
 * Validate the trace data in buffer
 *
 * @param aBuffer the buffer containing the trace data
 * @param aExpectedNum expected number of traces
 * @param aComponentID component ID of traces
 * @param aGroupID group ID of traces
 * @param aData first expected trace data
 * @param aTraceWord trace word of traces
 * @param aIdentical indicates that the payload of each trace packet should be identical
 * @param aBufferMode The mode of the trace buffer.. 0=straight, 1=circular.
 * @param aOrdinalData indicates whether the payload data reflects its position in the stream of trace packets
 * @param aNumTraces number of traces written to circular buffer
 * @param aMaxTraces maximum number of traces circular buffer can hold
 * @return Standard Symbian error code
 */
EXPORT_C TInt TTraceDataParser::ValidatePayload(TDesC8&         aBuffer,
                                                TInt            aExpectedNum,
                                                TComponentId    aComponentID, 
                                                TGroupId        aGroupID, 
                                                TUint32         aData, 
                                                TUint32         aTraceWord,
                                                TBool           aIdentical, 
                                                TInt            aBufferMode, 
                                                TBool           aOrdinalData, 
                                                TInt            aNumTraces, 
                                                TInt            aMaxTraces)
    {
    if ((aBuffer.Length() == 0) && (aExpectedNum <= 0))    // size is zero and there should be no traces
        {
        return KErrNone;
        }
    if ((aBuffer.Length() == 0) && (aExpectedNum > 0))     // size is zero and there should be traces
        {
        return KErrCorrupt;
        }
    if (aBuffer.Length() != 0 && (aExpectedNum <= 0))      // size is non-zero and there should be no traces
        {
        return KErrCorrupt;
        }

    TUint32 lastData = aData;
    TBool firstInStream = ETrue;

    // get the current position on the stream of traces so we can determine what the payload value should be.
    TInt overwrites = 0;
    if (aBufferMode == RTraceBuffer::EFreeRunning)
        {
        TInt overwrittenTraces = aNumTraces; // initialise to number of traces we wrote to the buffer for circular test
        while (overwrittenTraces > aMaxTraces)
            {
            overwrittenTraces = overwrittenTraces - aMaxTraces; // aMaxTraces is the max the buffer can hold 
            overwrites++;
            }
        lastData = (aMaxTraces * (overwrites - 1)) + overwrittenTraces + 1; // get where we are in the buffer
        }

    TInt            numberOSTtraces = 0;
    TUint8*         data            = (TUint8*) aBuffer.Ptr();
    TUint8*         endOfData       = data + aBuffer.Size();    
    TUint8          testGID         = aGroupID;
    TUint32         testCID         = aComponentID;
    TUint32         testTraceWord   = 0;

    TTraceHeaderSettings  traceHeaderSettings;

    // Loop through all traces
    while (data < endOfData)
        {
        // Get trace info from header
        TInt err = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (err != KErrNone)
            {
            return err;
            }

        //check the missing flag if we expect it
        if ((firstInStream&&(overwrites>0)))
            {
            if(!(traceHeaderSettings.iHeaderFlags & BTrace::EMissingRecord))
                {
                return KErrCorrupt;
                }
            }

        if (!traceHeaderSettings.iPrintfTrace)
            {
            testTraceWord = ((TUint32) (testGID << GROUPIDSHIFT)) + 1;
            
            // check group ID
            if (traceHeaderSettings.iCategory != testGID)
                {
                return KErrCorrupt;
                }
            // check component ID
            if (traceHeaderSettings.iComponentID != testCID)
                {
                return KErrCorrupt;
                }
            // check trace word
            if (traceHeaderSettings.iTraceWord != testTraceWord)
                {
                return KErrCorrupt;
                }
            
            TBool lastTrace = EFalse;
            if ( (traceHeaderSettings.iMultiPartType == BTrace::EMultipartLast) || (!traceHeaderSettings.iMultiPartType) )
                {
                lastTrace = ETrue;
                }

            // check trace payload
            for (TInt i=0; traceHeaderSettings.iLengthOfPayload>0; i++)
                {
                TUint32 param = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
                // For circular case, we try to estimate to within +/- 2 where we are in payload
                // for straight case we should get it exactly correct.
                if ( ((param != lastData) && (aBufferMode == 0)) || 
                      ( ((param > lastData + 2) || (param < lastData - 2)) && (aBufferMode == RTraceBuffer::EFreeRunning) ) )
                    {
                    return KErrCorrupt;
                    }
                lastData = param; // in case we weren't accurate in circular case.
                traceHeaderSettings.iLengthOfPayload -= 4;
                lastData++;
                }
            lastData = (!aOrdinalData)? aData : lastData;
            if (!aOrdinalData)
                {
                lastData = (lastTrace || aIdentical) ? aData : lastData;
                }

            if (traceHeaderSettings.iMultiPartType || aIdentical || aOrdinalData)
                {
                testGID = aGroupID;
                testCID = aComponentID;
                testTraceWord = aTraceWord;
                }
            else
                {
                testGID++;
                testCID++;
                testTraceWord += (1 << GROUPIDSHIFT);
                }
            numberOSTtraces++;
            }
        else
            {
            // Go to the next trace
            data += traceHeaderSettings.iLengthOfPayload;
            }
        firstInStream = EFalse; //after this, it's defnitely not the first trace in the stream.
        }

    // Check that we're getting the correct number of traces
    //or approximate ammount (+/-1) for circular buffer because
    //the arithmetic to overwrite traces sometimes also skips one
    //extra trace

    if (numberOSTtraces != aExpectedNum)
        {
        if ((aBufferMode==RTraceBuffer::EFreeRunning)
                &&((numberOSTtraces>=aExpectedNum-1)||(numberOSTtraces<=aExpectedNum+1)))
            {
            return KErrNone;
            }
        return KErrCorrupt;
        }

    return KErrNone;
    }

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
EXPORT_C TInt TTraceDataParser::ValidatePayload(TDesC8&        aBuffer, 
                                                TBool          aTracePresent, 
                                                TGroupId       aGroupID, 
                                                TComponentId   aComponentID, 
                                                TUint32        aData,
                                                TBool          aPrintfTrace,
                                                TBool          aMissingTrace,
                                                TDesC8*        aExpectedPrintfTrace)
    {
    if ((aBuffer.Length() == 0) && (!aTracePresent))    // size is zero and there should be no traces
        {
        return KErrNone;
        }
    if ((aBuffer.Length() == 0) && (aTracePresent))     // size is zero and there should be traces
        {
        return KErrCorrupt;
        }
    if (aBuffer.Length() != 0 && (!aTracePresent))      // size is non-zero and there should be no traces
        {
        return KErrCorrupt;
        }

    TUint8*         data            = (TUint8*) aBuffer.Ptr();
    TUint8*         endOfData       = data + aBuffer.Size();    

    TTraceHeaderSettings  traceHeaderSettings;

    // Get trace info from header
    TInt err = ParseHeader(data, endOfData-data, traceHeaderSettings);
    
    if (err != KErrNone)
        {
        return err;
        }

    // check trace type
    if (traceHeaderSettings.iPrintfTrace != aPrintfTrace)
        {
        return KErrCorrupt;
        }

    if (!traceHeaderSettings.iPrintfTrace)
        {
        TUint32 testTraceWord = ((TUint32) (aGroupID << GROUPIDSHIFT)) + 1;
        
        // check group ID
        if (traceHeaderSettings.iCategory != aGroupID)
            {
            return KErrCorrupt;
            }
        // check component ID
        if (traceHeaderSettings.iComponentID != aComponentID)
            {
            return KErrCorrupt;
            }
        // check trace word
        if (traceHeaderSettings.iTraceWord != testTraceWord)
            {
            return KErrCorrupt;
            }
        
        if (aMissingTrace)
            {
            // check missing flag
            if (!(traceHeaderSettings.iHeaderFlags & BTrace::EMissingRecord))
                {
                return KErrCorrupt;
                }
            }
        
        TInt i = 0;
        while (data < endOfData)
            {
            TUint32 param = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
            if (param != aData + i)
                {
                return KErrCorrupt;
                }
            i++;
            }
        }
    else if (!aExpectedPrintfTrace)
        {
        return KErrArgument;
        }
    else
        {
        TBuf8<KExpectedPrintfMaxLength> expectedPrintf;
        
        // In the case of an expected dropped trace, we only check for "* Dropped Trace" because
        // the test writer has completed the request for data when it writes that bit first (right 
        // before the next trace), and when it goes to write the actual trace, it doesn't.
        // If we change this, the "* Dropped Trace" will be overwritten by the actual trace before
        // the test can check it, since the test writer doesn't buffer trace data.
        if (aMissingTrace)
            {
            // check that "* Dropped Trace" is in the buffer
            expectedPrintf = KDroppedTrace;
            }
        else
            {
            // check that "Test Printf Trace" is in the buffer"
            expectedPrintf = *aExpectedPrintfTrace;
            }
        
        expectedPrintf.Append(TChar(0)); // printf handler appends null at the end.

        TPtrC8 actualprintf(data, traceHeaderSettings.iLengthOfPayload);

        // Need to fix endianess for hw
#ifndef __WINS__
        TInt bufpos = 0;
        while (bufpos < (actualprintf.Length()-4))
            {
            // reads a 4 byte integer of expected endianess
            TUint32 n = actualprintf[bufpos] + (actualprintf[bufpos+1] << 8) + (actualprintf[bufpos+2] << 16) + (actualprintf[bufpos+3] << 24);
            bufpos += sizeof(TUint32);
            TUint32 tempData = Swap(n);
            memcpy((TAny*)(actualprintf.Ptr()+bufpos-4),(TAny*)&tempData, sizeof(TUint32));
            }        
#endif //__WINS__
        
        if (expectedPrintf.Compare(actualprintf) != 0)
            {
            return KErrCorrupt;
            }

        // Check if there is another trace to follow
        data += traceHeaderSettings.iLengthOfPayload;
        if (data != endOfData)
            {
            TPtrC8 buffer(data, endOfData - data);
            
            return ValidatePayload(buffer, 
                                   aTracePresent, 
                                   aGroupID, 
                                   aComponentID, 
                                   aData,
                                   aPrintfTrace,
                                   EFalse,
                                   aExpectedPrintfTrace);
            }
        }    
    return KErrNone;
    }

/*
 * Verify the priming data contains all the expected values
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aCategory BTrace category of the trace data
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aElementsFound Parameter to hold current state of expected data found so far
 * @param aVerificationData1 Optional input value - used differently for different categories
 * @param aVerificationData2 Optional input value - used differently for different categories
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::VerifyPrimingData(TUint8* aBuffer, 
                                         TInt    aSize, 
                                         TUint8  aCategory, 
                                         TUint8  aSubCategory, 
                                         TInt&   aElementsFound,
                                         TBool   aFromTestWriter, 
                                         TUint32 aVerificationData1, 
                                         TUint32 aVerificationData2)
    {
    TInt err = KErrNotSupported;
    switch(aCategory)
        {
        case BTrace::EThreadIdentification:
            err = VerifyThreadPrimingData(aBuffer, aSize, aSubCategory, aElementsFound, aFromTestWriter, aVerificationData1, aVerificationData2);
            break;
        case BTrace::EFastMutex:
            err = VerifyFastMutexPrimingData(aBuffer, aSize, aSubCategory, aElementsFound, aFromTestWriter);
            break;
        case BTrace::ECodeSegs:
            err = VerifyCodeSegsPrimingData(aBuffer, aSize, aSubCategory, aElementsFound, aFromTestWriter, aVerificationData1, aVerificationData2);
            break;
        default:
            break;
        }
    return err;    
    }

/*
 * Verify the EThreadIdentification priming data contains all the expected values
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aElementsFound Parameter to hold current state of expected data found so far
 * @param aThreadAddr The address of the thread to search for
 * @param aProcessAddr The address of the process to search for
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::VerifyThreadPrimingData(TUint8* aBuffer, 
                                               TInt    aSize, 
                                               TUint8  aSubCategory, 
                                               TInt&   aElementsFound,
                                               TBool   aFromTestWriter, 
                                               TUint32 aThreadAddr, 
                                               TUint32 aProcessAddr)
    {
    RThread currentThread;    
    RProcess currentProcess;

    TUint32 tempThreadAddr;
    TUint32 tempProcessAddr;
    
    switch(aSubCategory)
        {
        case BTrace::EProcessCreate:
            {
            tempProcessAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            if (tempProcessAddr == aProcessAddr)
                {
                aElementsFound |= 0x000000ff;
                }
            }
            break;
            
        case BTrace::EProcessName:
            {
            tempThreadAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            tempProcessAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            if ( (tempThreadAddr == aThreadAddr) && (tempProcessAddr == aProcessAddr) )
                {
                TBuf8<KMaxName> currentProcessName;
                currentProcessName.Copy(currentProcess.Name());
                TPtr8 processName(aBuffer,(aSize-8),(aSize-8)); //should be name of the process
                processName.Trim();
                if (StringsMatch(currentProcessName, processName))
                    {
                    aElementsFound |= 0x0000ff00;
                    }
                }
            }
            break;
        
        case BTrace::EThreadName:
            {
            tempThreadAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            tempProcessAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            if ( (tempThreadAddr == aThreadAddr) && (tempProcessAddr == aProcessAddr) )
                {
                TBuf8<KMaxName> currentThreadName;
                currentThreadName.Copy(currentThread.Name());
                TPtr8 threadName(aBuffer,(aSize-8),(aSize-8)); //should be name of the thread
                threadName.Trim();
                if (StringsMatch(currentThreadName, threadName))
                    {
                    aElementsFound |= 0x00ff0000;
                    }
                }
            }
            break;
            
        case BTrace::EThreadId:
            {
            tempThreadAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            tempProcessAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
            if ( (tempThreadAddr == aThreadAddr) && (tempProcessAddr == aProcessAddr) )
                {
                TThreadId threadId = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
                if (currentThread.Id() == threadId)
                    {
                    aElementsFound |= 0xff000000;
                    }
                }
            }
            break; 
            
        default:
            break;
        }
    
    TInt err = KErrNotFound;
    if (aElementsFound == 0xffffffff)
        {
        err = KErrNone;
        }
    return err;
    }

/*
 * Verify the EFastMutex priming data contains all the expected values
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aElementsFound Parameter to hold current state of expected data found so far
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::VerifyFastMutexPrimingData(TUint8* aBuffer, 
                                                  TInt    aSize, 
                                                  TUint8  aSubCategory, 
                                                  TInt&   aElementsFound,
                                                  TBool   aFromTestWriter)
    {
    if (aSubCategory == BTrace::EFastMutexName)
        {
        TUint32 fastMutexId = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
        TUint32 unspecified = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
        if (fastMutexId && !unspecified)
            {
            TPtr8 fastMutexName(aBuffer,(aSize-8),(aSize-8)); //should be name of the fast mutex
            fastMutexName.Trim();
            if (StringsMatch(KFastMutexNameSystemLock(), fastMutexName))
                {
                aElementsFound |= 0x000000ff;
                }
            else if (StringsMatch(KFastMutexNameMsgLock(), fastMutexName))
                {
                aElementsFound |= 0x0000ff00;
                }
            else if (StringsMatch(KFastMutexNameObjLock(), fastMutexName))
                {
                aElementsFound |= 0x00ff0000;
                }
            else if (StringsMatch(KFastMutexNameLogonLock(), fastMutexName))
                {
                aElementsFound |= 0xff000000;
                }
            }
        }
    
    TInt err = KErrNotFound;
    if (aElementsFound == 0xffffffff)
        {
        err = KErrNone;
        }
    return err;
    }

/*
 * Verify the ECodeSegs priming data contains all the expected values
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aElementsFound Parameter to hold current state of expected data found so far
 * @param aSegAddr1 The address of the first code seg to search for
 * @param aSegAddr2 The address of the second code seg to search for
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::VerifyCodeSegsPrimingData(TUint8* aBuffer, 
                                                 TInt    aSize, 
                                                 TUint8  aSubCategory, 
                                                 TInt&   aElementsFound,
                                                 TBool   aFromTestWriter, 
                                                 TUint32 aSegAddr1, 
                                                 TUint32 aSegAddr2)
    {
    TUint32 codeSegAddr;
    if (aSubCategory == BTrace::ECodeSegCreated)
        {
        codeSegAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
        if (codeSegAddr == aSegAddr1 || codeSegAddr == aSegAddr2)
            {
            TPtr8 codeSegName(aBuffer,(aSize-4),(aSize-4)); //should be name of the code seg
            codeSegName.Trim();
            if (codeSegAddr == aSegAddr1 && StringsMatch(KCodeSegsName1(), codeSegName))
                {
                aElementsFound |= 0x000000ff;
                }
            else if (codeSegAddr == aSegAddr2 && StringsMatch(KCodeSegsName2(), codeSegName))
                {
                aElementsFound |= 0x0000ff00;
                }
            }
        }
    else if (aSubCategory == BTrace::ECodeSegInfo)
        {
        codeSegAddr = Swap(ReadUint32FromBuf(aBuffer, aFromTestWriter));
        if (codeSegAddr == aSegAddr1)
            {
            aElementsFound |= 0x00ff0000;
            }
        else if (codeSegAddr == aSegAddr2)
            {
            aElementsFound |= 0xff000000;
            }
        }
    
    TInt err = KErrNotFound;
    if (aElementsFound == 0xffffffff)
        {
        err = KErrNone;
        }
    return err;
    }

/*
 * Work out if a trace point is potentially kernel priming data based on category and subcategory
 *
 * @param aCategory BTrace category of the trace data
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aIsPotentialPrimingTrace Output value set if there is no error
 * @return Standard Symbian error code
 */
TBool TTraceDataParser::IsPotentialPrimingTrace(TUint8 aCategory, TUint8 aSubCategory, TBool& aIsPotentialPrimingTrace)
    {
    TInt err = KErrNone;
    
    switch(aCategory)
        {
        case BTrace::EThreadIdentification:
            {
            switch(aSubCategory)
                {
                case BTrace::EProcessCreate:
                case BTrace::EProcessName:
                case BTrace::EThreadName:
                case BTrace::EThreadId:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::ECpuUsage:
            {
            switch(aSubCategory)
                {
                case BTrace::ENewThreadContext:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::EChunks:
            {
            switch(aSubCategory)
                {
                case BTrace::EChunkCreated:
                case BTrace::EChunkOwner:
                case BTrace::EChunkInfo:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::ECodeSegs:
            {
            switch(aSubCategory)
                {
                case BTrace::ECodeSegCreated:
                case BTrace::ECodeSegInfo:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::EPaging:
            {
            switch(aSubCategory)
                {
                case BTrace::EPagingMemoryModel:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::EThreadPriority:
            {
            switch(aSubCategory)
                {
                case BTrace::EProcessPriority:
                case BTrace::EDThreadPriority:
                case BTrace::ENThreadPriority:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::EFastMutex:
            {
            switch(aSubCategory)
                {
                case BTrace::EFastMutexName:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::ESymbianKernelSync:
            {
            switch(aSubCategory)
                {
                case BTrace::EMutexCreate:
                case BTrace::ESemaphoreCreate:
                case BTrace::ECondVarCreate:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        case BTrace::EClientServer:
            {
            switch(aSubCategory)
                {
                case BTrace::EServerCreate:
                case BTrace::ESessionAttach:
                    aIsPotentialPrimingTrace = ETrue;
                    break;
                default:
                    aIsPotentialPrimingTrace = EFalse;
                    break;
                }
            }
            break;
        default:
            err = KErrNotSupported;
            break;
        }
    
    return err;
    }

/*
 * If the next trace is a meta trace true is returned if it is the start of priming data for a given group ID
 *
 * @param aBuffer Buffer containing the trace data
 * @param aGroupId Group ID of kernel priming data
 * @param aCategory BTrace category of the current trace
 * @param aSubCategory BTrace subcategory of the current trace
 * @return True if start of priming data for given group ID, false otherwise
 */
TBool TTraceDataParser::IsStartOfKernelPrimingBatch(TUint8* aBuffer, TGroupId aGroupId, TUint8 aCategory, TUint8 aSubCategory)
    {
    TBool startOfBatch = EFalse;
    if (aCategory == BTrace::EMetaTrace)
        {
        if (aSubCategory == BTrace::EMetaTraceFilterChange)
            {
            TUint8 metaCategory = aBuffer[0];
            TUint8 isActive = aBuffer[1];
            if (metaCategory == aGroupId && isActive)
                {
                startOfBatch = ETrue;
                }
            }        
        }
    return startOfBatch;
    }

/*
 * Use heuristics to work out if current trace is part of kernel priming data
 *
 * @param aCategory BTrace category of the trace data
 * @param aSubCategory BTrace subcategory of the trace data
 * @param aTimeDifference Time difference (in microseconds) since last trace for given category
 * @param aIsInPrimingBatch Output parameter set to true if in priming data
 * @param aFirstTrace Boolean used to indicate if this is first trace (so there is no valid time difference)
 * @param aStartOfBatch Output parameter set to true if start of batch of kernel priming data
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::IsInPrimingBatch(TUint8  aCategory, 
                                        TUint8  aSubCategory, 
                                        TUint32 aTimeDifference, 
                                        TBool&  aIsInPrimingBatch, 
                                        TBool&  aFirstTrace, 
                                        TBool&  aStartOfBatch)
    {
    TBool isPotentialPrimingTrace = EFalse;
    TInt err = IsPotentialPrimingTrace(aCategory, aSubCategory, isPotentialPrimingTrace);
    
    // Look at timestamp to see if this is a new batch of traces
    if (aFirstTrace)
        {
        aFirstTrace = EFalse;
        }
    else if (aTimeDifference > KMinMilliSecondsBatchGap * 1000)
        {
        aStartOfBatch = ETrue;
        aIsInPrimingBatch = EFalse;
        }
    else
        {
        aStartOfBatch = EFalse;
        }
    
    if (isPotentialPrimingTrace && aStartOfBatch)
        {
        aIsInPrimingBatch = ETrue;
        aStartOfBatch = EFalse;
        }
    else if (!isPotentialPrimingTrace)
        {
        aIsInPrimingBatch = EFalse;
        }
    
    return err;
    }

/*
 * Parse the trace data header
 * 
 * The following trace types are supported:
 *     OST traces (ascii and binary)
 *     XTIv3 traces (ascii and binary)
 *     Traces from the Test Writer (ascii and binary)
 *
 * @param aData Buffer containing the trace data
 * @param aSize Size of trace data buffer
 * @param aTraceHeaderSettings Output parameters set to header settings of the trace data
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::ParseHeader(TUint8*&                 aData, 
                                   TInt                     aSize, 
                                   TTraceHeaderSettings&    aTraceHeaderSettings)
    {
    // Check buffer is large enough to contain header
    aSize--;
    if (aSize < 0)
        {
        return KErrOverflow;
        }
    
    TInt err = KErrNone;    
    TUint8* startOfData = aData;
    aTraceHeaderSettings.iMultiPartType = 0;
    aTraceHeaderSettings.iMultiPartTraceID = 0;
    aTraceHeaderSettings.iPrintfTrace = EFalse;
    aTraceHeaderSettings.iHeaderFlags = EFalse;
    aTraceHeaderSettings.iFromTestWriter = EFalse;
    TBool xtiTrace = EFalse;
    TUint8 version = aData[0];                        // Version (1 byte)
    aData++;
    
    if (version == KXtiHeaderVersion)
        {
        // Check buffer is large enough to contain header
        aSize -= KMinSizeXtiHeader;
        if (aSize < 0)
            {
            aData = startOfData;
            return KErrOverflow;
            }
        xtiTrace = ETrue;
        TUint8 xtiTraceType = aData[KXtiTraceTypeIndex-1];
        if (xtiTraceType != KXtiProtocolIdSimpleTrace)
            {
            aTraceHeaderSettings.iPrintfTrace = ETrue;
            aTraceHeaderSettings.iLengthOfPayload = aData[KXtiLengthIndex-1] - 15;
            aData += KMinSizeXtiHeader;
            }
        else
            {
            // Check buffer is large enough to contain header
            aSize -= 1;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            // Jump to start of OST buffer
            aData += KMinSizeXtiHeader + 1;
            version = aData[0];
            aData++;
            }
        }
    
    if (version == KHeaderVersion) // Ost header version byte is correct
        {
        // Check buffer is large enough to contain header
        aSize -= KMinSizeOstHeader;
        if (aSize < 0)
            {
            aData = startOfData;
            return KErrOverflow;
            }
        const TUint8 protocolId = aData[0];                 // Protocol ID (1 byte)
        aData++;
        if (xtiTrace)
            {
            aTraceHeaderSettings.iLengthOfPayload = aData[0];
            aData++;
            aSize++;
            }
        else
            {
            aTraceHeaderSettings.iLengthOfPayload = ReadUint16FromBuf(aData);        // Size (2 bytes)
            }
    
        if (protocolId == KProtocolIdSimpleTrace) // Normal binary data
            {
            // Check buffer is large enough to contain header
            aSize -= KMinSizeOstBinaryHeader;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            ReadUint32FromBuf(aData);                       // Timestamp 1 (4 bytes)
            aTraceHeaderSettings.iTimestamp = ReadUint32FromBuf(aData);          // Timestamp 2 (4 bytes)
            }
        else if (protocolId == KProtocolIdAscii) // Ascii Printf data
            {
            aTraceHeaderSettings.iPrintfTrace = ETrue;
            // Check buffer is large enough to contain header
            aSize -= KMinSizeOstAsciiHeader;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            aData += 8;
            aTraceHeaderSettings.iLengthOfPayload -= 8;
            }
        else // Protocol ID is incorrect
            {
            err = KErrCorrupt;
            }
        }
    else if (version == SYMBIAN_TRACE) // Trace data is binary data from Test Writer
        {
        aTraceHeaderSettings.iFromTestWriter = ETrue;
        }
    else if (version == PRINTF_TRACE) // Trace data is ascii data from Test Writer
        {
        aTraceHeaderSettings.iFromTestWriter = ETrue;
        aTraceHeaderSettings.iPrintfTrace = ETrue;
        // The end of the printf trace data will have null char, so use that to find end of payload
        TPtr8 printfString(aData, aSize, aSize);
        aTraceHeaderSettings.iLengthOfPayload = printfString.Locate(TChar(0));
        if (aTraceHeaderSettings.iLengthOfPayload >= 0)
            {
            aTraceHeaderSettings.iLengthOfPayload++;
            }
        else
            {
            err = KErrCorrupt;
            }
        }
    else if (version != KXtiHeaderVersion)
        {
        err = KErrCorrupt;
        
        if(aTraceHeaderSettings.iLengthOfPayload < 0 )
            {
            err = KErrOverflow;
            }
        }
              
    if (aTraceHeaderSettings.iPrintfTrace)
        {
        aTraceHeaderSettings.iTimestamp = 0;
        aTraceHeaderSettings.iComponentID = 0;
        aTraceHeaderSettings.iCategory = 0;
        aTraceHeaderSettings.iSubCategory = 0;
        aTraceHeaderSettings.iTraceWord = 0;
        }
    else if (err == KErrNone)
        {
        // Check buffer is large enough to contain header
        aSize -= KMinSizeBinaryHeader;
        if (aSize < 0)
            {
            aData = startOfData;
            return KErrOverflow;
            }
        aTraceHeaderSettings.iComponentID = ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);    // ComponentId (4 bytes)
        aTraceHeaderSettings.iTraceWord = ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);      // GroupId (2 bytes). TraceId (2 bytes)
        
        TUint8* startOfPayload = aData;
        TUint8 size;
        
        TBool testWriterHwData = EFalse;
#ifndef __WINS__
        testWriterHwData = aTraceHeaderSettings.iFromTestWriter;
#endif

        // now look at BTrace header and payload.
        if (!testWriterHwData)
            {
            size = aData[BTrace::ESizeIndex];
            aTraceHeaderSettings.iHeaderFlags = aData[BTrace::EFlagsIndex];
            aTraceHeaderSettings.iCategory = aData[BTrace::ECategoryIndex];
            aTraceHeaderSettings.iSubCategory = aData[BTrace::ESubCategoryIndex];
            }
        else
            {
            // endianess order is reversed for TestWriter on hw
            aTraceHeaderSettings.iSubCategory = aData[BTrace::ESizeIndex];
            aTraceHeaderSettings.iCategory = aData[BTrace::EFlagsIndex];
            aTraceHeaderSettings.iHeaderFlags = aData[BTrace::ECategoryIndex];
            size = aData[BTrace::ESubCategoryIndex];
            }
        
        aData+=4;

        //read header extensions
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::EHeader2Present)
            {
            // Check buffer is large enough to contain header
            aSize -= 4;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            TUint32 header2 = Swap(ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter));
            aTraceHeaderSettings.iMultiPartType = header2 & BTrace::EMultipartFlagMask;
            }
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::ETimestampPresent)
            {
            // Check buffer is large enough to contain header
            aSize -= 4;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);
            }
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::ETimestamp2Present)
            {
            // Check buffer is large enough to contain header
            aSize -= 4;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);
            }
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::EContextIdPresent)
            {
            // Check buffer is large enough to contain header
            aSize -= 4;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);
            }
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::EPcPresent)
            {
            // Check buffer is large enough to contain header
            aSize -= 4;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter);
            }
        if(aTraceHeaderSettings.iHeaderFlags & BTrace::EExtraPresent)
            {
            // Check buffer is large enough to contain header
            aSize -=4 ;
            if (aSize < 0)
                {
                aData = startOfData;
                return KErrOverflow;
                }
            aTraceHeaderSettings.iMultiPartTraceID = Swap(ReadUint32FromBuf(aData, aTraceHeaderSettings.iFromTestWriter));
            }

        if (!xtiTrace)
            {
            size = (size+3)&~3;  // make it 4 byte aligned
            }
        if (size == 0)
            {
            // This happens when the trace is larger than maximum size, and we can't use the BTrace size field
            // Instead calculate payload length based on size in OST header
            aTraceHeaderSettings.iLengthOfPayload -= (aData-startOfPayload)+16; // size of rest of payload
            }
        else
            {
            aTraceHeaderSettings.iLengthOfPayload = size-(aData-startOfPayload); // size of rest of payload
            }
        }

    
    // Check buffer is large enough to contain payload
    aSize -= aTraceHeaderSettings.iLengthOfPayload;
    if (aSize < 0)
        {
        aData = startOfData;
        err = KErrOverflow;
        }
    
    return err;
    }

/*
 * Get the data required to verify kernel priming traces for EThreadIdentification
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aThreadAddr Output parameter set to address of current thread
 * @param aProcessAddr Output parameter set to address of current process
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::GetThreadPrimingVerificationDataL(TUint8*     aBuffer, 
                                                         TInt        aSize, 
                                                         TUint32&    aThreadAddr, 
                                                         TUint32&    aProcessAddr,
                                                         RFile*      aFile)
    {
    TInt    err = KErrNone;
    TInt    filePos = 0;
    TUint8* data = NULL;
    TUint8* startOfData = NULL;
    TUint8* endOfData = NULL;    
    RBuf8   fileBuffer;
    
    if (aFile)
        {
        // Create file buffer and read first chunk from file
        err = CreateFileBuffer(fileBuffer, *aFile);
        if (err != KErrNone)
            {
            return err;
            }
        fileBuffer.CleanupClosePushL();
        err = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
        if (err != KErrNone)
            {
            CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
            return err;
            }
        }
    else
        {
        data = aBuffer;
        startOfData = data;
        endOfData = data + aSize;
        }
    
    TInt            headerErr = KErrNone; 
    RThread         currentThread;            
    TUint32         tempThreadAddr;
    TUint32         tempProcessAddr;
    TThreadId       threadid = 0;

    TTraceHeaderSettings  traceHeaderSettings;

    err = KErrGeneral;
    
    while(data < endOfData)
        {
        headerErr = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (aFile && headerErr == KErrOverflow)
            {
            // We don't have all the trace data, so read next chunk from file
            headerErr = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
            if (headerErr == KErrNone)
                {
                continue;
                }
            }
        if (headerErr != KErrNone)
            {
            err = headerErr;
            break;
            }
        if ( (traceHeaderSettings.iCategory == BTrace::EThreadIdentification) && (traceHeaderSettings.iSubCategory == BTrace::EThreadId) )
            {
            tempThreadAddr = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
            tempProcessAddr = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
            threadid = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
            if (currentThread.Id() == threadid)
                {
                aThreadAddr = tempThreadAddr;
                aProcessAddr = tempProcessAddr;
                err = KErrNone;
                break;
                }
            }
        else
            {
            data += traceHeaderSettings.iLengthOfPayload; //go to next trace
            }

        if (aFile && data == endOfData)
            {
            // We might not have all the trace data, so read next chunk from file
            headerErr = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
            if (headerErr != KErrNone)
                {
                err = headerErr;
                break;
                }
            }
        }

    if (aFile)
        {
        CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
        }

    return err;
    }


/*
 * Get the data required to verify kernel priming traces for ECodeSegs
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aSegAddr1 Output parameter set to address of first code seg
 * @param aSegAddr2 Output parameter set to address of second code seg
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::GetCodeSegsVerificationDataL(TUint8*  aBuffer, 
                                                    TInt     aSize, 
                                                    TUint32& aSegAddr1, 
                                                    TUint32& aSegAddr2,
                                                    RFile*   aFile)
    {
    TInt    err = KErrNone;
    TInt    filePos = 0;
    TUint8* data = NULL;
    TUint8* startOfData = NULL;
    TUint8* endOfData = NULL;    
    RBuf8   fileBuffer;
    
    if (aFile)
        {
        // Create file buffer and read first chunk from file
        err = CreateFileBuffer(fileBuffer, *aFile);
        if (err != KErrNone)
            {
            return err;
            }
        fileBuffer.CleanupClosePushL();
        err = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
        if (err != KErrNone)
            {
            CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
            return err;
            }
        }
    else
        {
        data = aBuffer;
        startOfData = data;
        endOfData = data + aSize;
        }
    
    TInt            headerErr = KErrNone; 
    TUint32         tempSegAddr;

    TTraceHeaderSettings  traceHeaderSettings;

    err = KErrGeneral;
    
    while(data < endOfData)
        {
        headerErr = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (aFile && headerErr == KErrOverflow)
            {
            // We don't have all the trace data, so read next chunk from file
            headerErr = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
            if (headerErr == KErrNone)
                {
                continue;
                }
            }
        if (headerErr != KErrNone)
            {
            err = headerErr;
            break;
            }
        if ( (traceHeaderSettings.iCategory == BTrace::ECodeSegs) && (traceHeaderSettings.iSubCategory == BTrace::ECodeSegCreated) )
            {
            tempSegAddr = Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
            traceHeaderSettings.iLengthOfPayload -= 4;
            TPtr8 codeSegName(data,traceHeaderSettings.iLengthOfPayload,traceHeaderSettings.iLengthOfPayload); //should be name of the code seg
            codeSegName.Trim();
            if (StringsMatch(KCodeSegsName1(), codeSegName))
                {
                aSegAddr1 = tempSegAddr;
                }
            if (StringsMatch(KCodeSegsName2(), codeSegName))
                {
                aSegAddr2 = tempSegAddr;
                }
            if (aSegAddr1 != 0 && aSegAddr2 != 0)
                {
                err = KErrNone;
                break;
                }
            }

        data += traceHeaderSettings.iLengthOfPayload; //go to next trace

        if (aFile && data == endOfData)
            {
            // We might not have all the trace data, so read next chunk from file
            headerErr = ReadNextChunkFromFile(fileBuffer, *aFile, filePos, data, startOfData, endOfData);
            if (headerErr != KErrNone)
                {
                err = headerErr;
                break;
                }
            }
        }

    if (aFile)
        {
        CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
        }
    
    return err;
    }

/*
 * Get the data required to verify kernel priming traces
 *
 * @param aBuffer Buffer containing the trace data
 * @param aSize Size of the trace buffer
 * @param aGroupId Group ID of traces
 * @param aVerificationData1 Output parameter set to first data required
 * @param aVerificationData2 Output parameter set to second data required
 * @return Standard Symbian error code
 */
TInt TTraceDataParser::GetPrimingVerificationDataL(TUint8*  aBuffer, 
                                                   TInt     aSize, 
                                                   TGroupId aGroupId, 
                                                   TUint32& aVerificationData1, 
                                                   TUint32& aVerificationData2,
                                                   RFile*   aFile)
    {
    TInt err = KErrNone;
    switch(aGroupId)
        {
        case BTrace::EThreadIdentification:
            err = GetThreadPrimingVerificationDataL(aBuffer, aSize, aVerificationData1, aVerificationData2, aFile);
            break;
        case BTrace::ECodeSegs:
            err = GetCodeSegsVerificationDataL(aBuffer, aSize, aVerificationData1, aVerificationData2, aFile);
            break;
        default:
            break;
        }
    return err;    
    }

/*
 * Parse trace data for kernel priming data
 * 
 * Note that this function can only verify priming data if the group ID is EThreadIdentification or EFastMutex,
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
EXPORT_C TInt TTraceDataParser::ParsePrimingDataL(TDesC8&        aBuffer, 
                                                  TGroupId       aGroupId, 
                                                  TInt&          aNumPrimingTraces, 
                                                  TInt&          aNumTraces, 
                                                  TBool          aVerifyData, 
                                                  TUint32        aVerificationData1, 
                                                  TUint32        aVerificationData2)
    {
    aNumPrimingTraces = 0;
    aNumTraces = 0;

    TUint8*         data = (TUint8*) aBuffer.Ptr();
    TUint8*         endOfData = data + aBuffer.Size();    
    
    // If the output data is to be verified but no verification data provided, try getting it automatically
    if (aVerifyData && aVerificationData1 == 0 && aVerificationData2 == 0)
        {
        TInt err = GetPrimingVerificationDataL(data, endOfData-data, aGroupId, aVerificationData1, aVerificationData2);
        if (err != KErrNone)
            {
            return err;
            }
        }
    
    TInt            returnErr = KErrNone;
    TInt            dataVerificationErr = KErrNone;
    TInt            generalErr = KErrNone;
    TBool           firstTrace = ETrue;
    TBool           startOfBatch = ETrue;
    TBool           inPrimingData = EFalse;
    TBool           useMetaTraces = EFalse;
    TInt            numPotentialPrimingTraces = 0;
    TUint32         lastTimestamp = 0;
    TInt            elementsFound = 0;
    
    TTraceHeaderSettings  traceHeaderSettings;

    // Loop through all traces
    while (data < endOfData)
        {
        // Get trace info from header
        generalErr = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (generalErr != KErrNone)
            {
            if (returnErr == KErrNone)
                {
                // Set error to return, if this is first error so far
                returnErr = generalErr;
                }
            break;
            }

        // If this is a meta trace, work out if it signifies start of kernel priming data for GID of interest
        if (traceHeaderSettings.iCategory == BTrace::EMetaTrace)
            {
            useMetaTraces = ETrue;
            startOfBatch = IsStartOfKernelPrimingBatch(data, aGroupId, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory);
            }     
        
        // This is a trace of the correct GID/CID
        if ( (traceHeaderSettings.iComponentID == KKernelHooksOSTComponentUID) && (traceHeaderSettings.iCategory == aGroupId) )
            {
            // Increase the number of traces for this GID
            aNumTraces++;

            if (useMetaTraces)
                {
                // If data contains meta traces, work out if this is a priming trace
                if (startOfBatch)
                    {
                    // This is the start of a batch of priming traces
                    inPrimingData = ETrue;
                    startOfBatch = EFalse;
                    }
                if (inPrimingData)
                    {
                    // This is a priming trace, so increase number found for this GID
                    aNumPrimingTraces++; 
                    // If data is to be verified, attempt the verification
                    if (aVerifyData)
                        {
                        dataVerificationErr = VerifyPrimingData(data, traceHeaderSettings.iLengthOfPayload, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, elementsFound, traceHeaderSettings.iFromTestWriter, aVerificationData1, aVerificationData2);
                        }
                    }
                }
            else
                {
                // If data contains no meta traces, work out if this is a priming trace using heuristics
                generalErr = IsInPrimingBatch(traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, traceHeaderSettings.iTimestamp-lastTimestamp, inPrimingData, firstTrace, startOfBatch);
                if ( (generalErr != KErrNone) && (returnErr == KErrNone) )
                    {
                    // Set error to return, if this is first error so far
                    returnErr = generalErr;
                    }
                lastTimestamp = traceHeaderSettings.iTimestamp;
                if (inPrimingData)
                    {
                    // This is possibly a priming trace, so increase potential number found for this GID
                    numPotentialPrimingTraces++;
                    // If data is to be verified, attempt the verification
                    if (aVerifyData)
                        {
                        dataVerificationErr = VerifyPrimingData(data, traceHeaderSettings.iLengthOfPayload, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, elementsFound, traceHeaderSettings.iFromTestWriter, aVerificationData1, aVerificationData2);
                        }
                    }
                else
                    {
                    // If we have enough potential priming traces, assume they are priming traces
                    if (numPotentialPrimingTraces >= KMinimumPrimingTracesInBatch)
                        {
                        aNumPrimingTraces += numPotentialPrimingTraces;
                        }
                    numPotentialPrimingTraces = 0;
                    }                
                }
            }
        else
            {
            // This is not a trace of the correct GID/CID
            inPrimingData = EFalse;
            }
        
        // Go to the next trace
        data+=traceHeaderSettings.iLengthOfPayload;
        }

    if (!useMetaTraces)
        {
        // If we have enough potential priming traces left over, assume they are priming traces
        if (numPotentialPrimingTraces >= KMinimumPrimingTracesInBatch)
            {
            aNumPrimingTraces += numPotentialPrimingTraces;
            }
        }

    if ( (dataVerificationErr != KErrNone) && (returnErr == KErrNone) )
        {
        // Set error to return, if this is first error so far
        returnErr = dataVerificationErr;
        }
    return returnErr;
    }

/*
 * Parse trace data for kernel priming data
 * 
 * Note that this function can only verify priming data if the group ID is EThreadIdentification or EFastMutex,
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
EXPORT_C TInt TTraceDataParser::ParsePrimingDataL(const TDesC&   aFilePath,
                                                  RFs&           aFs, 
                                                  TGroupId       aGroupId, 
                                                  TInt&          aNumPrimingTraces, 
                                                  TInt&          aNumTraces, 
                                                  TBool          aVerifyData, 
                                                  TUint32        aVerificationData1, 
                                                  TUint32        aVerificationData2)
    {
    aNumPrimingTraces = 0;
    aNumTraces = 0;
    
    RFile file;

    // Open file with trace data
    TInt err = file.Open(aFs, aFilePath, EFileRead|EFileShareAny);
    if (err != KErrNone)
        {
        return err;
        }
    CleanupClosePushL(file);
    
    // If the output data is to be verified but no verification data provided, try getting it automatically
    if (aVerifyData && aVerificationData1 == 0 && aVerificationData2 == 0)
        {
        err = GetPrimingVerificationDataL(NULL, 0, aGroupId, aVerificationData1, aVerificationData2, &file);
        if (err != KErrNone)
            {
            CleanupStack::PopAndDestroy(&file); // close file
            return err;
            }
        }
    
    RBuf8   fileBuffer;
    TInt    filePos = 0;
    
    // Create file buffer and read first chunk from file
    err = CreateFileBuffer(fileBuffer, file);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }
    fileBuffer.CleanupClosePushL();
    
    TUint8* data = NULL;
    TUint8* startOfData = NULL;
    TUint8* endOfData = NULL;    

    err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }

    TInt            dataVerificationErr = KErrNone;
    TInt            generalErr = KErrNone;
    TBool           firstTrace = ETrue;
    TBool           startOfBatch = ETrue;
    TBool           inPrimingData = EFalse;
    TBool           useMetaTraces = EFalse;
    TInt            numPotentialPrimingTraces = 0;
    TUint32         lastTimestamp = 0;
    TInt            elementsFound = 0;
    
    TTraceHeaderSettings  traceHeaderSettings;

    // Loop through all traces
    while (data < endOfData)
        {
        // Get trace info from header
        generalErr = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (generalErr == KErrOverflow)
            {
            // We don't have all the trace data, so read next chunk from file
            generalErr = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (generalErr == KErrNone)
                {
                continue;
                }
            }
        if (generalErr != KErrNone)
            {
            if (err == KErrNone)
                {
                // Set error to return, if this is first error so far
                err = generalErr;
                }
            break;
            }

        // If this is a meta trace, work out if it signifies start of kernel priming data for GID of interest
        if (traceHeaderSettings.iCategory == BTrace::EMetaTrace)
            {
            useMetaTraces = ETrue;
            startOfBatch = IsStartOfKernelPrimingBatch(data, aGroupId, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory);
            }     
        
        // This is a trace of the correct GID/CID
        if ( (traceHeaderSettings.iComponentID == KKernelHooksOSTComponentUID) && (traceHeaderSettings.iCategory == aGroupId) )
            {
            // Increase the number of traces for this GID
            aNumTraces++;

            if (useMetaTraces)
                {
                // If data contains meta traces, work out if this is a priming trace
                if (startOfBatch)
                    {
                    // This is the start of a batch of priming traces
                    inPrimingData = ETrue;
                    startOfBatch = EFalse;
                    }
                if (inPrimingData)
                    {
                    // This is a priming trace, so increase number found for this GID
                    aNumPrimingTraces++; 
                    // If data is to be verified, attempt the verification
                    if (aVerifyData)
                        {
                        dataVerificationErr = VerifyPrimingData(data, traceHeaderSettings.iLengthOfPayload, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, elementsFound, traceHeaderSettings.iFromTestWriter, aVerificationData1, aVerificationData2);
                        }
                    }
                }
            else
                {
                // If data contains no meta traces, work out if this is a priming trace using heuristics
                generalErr = IsInPrimingBatch(traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, traceHeaderSettings.iTimestamp-lastTimestamp, inPrimingData, firstTrace, startOfBatch);
                if ( (generalErr != KErrNone) && (err == KErrNone) )
                    {
                    // Set error to return, if this is first error so far
                    err = generalErr;
                    }
                lastTimestamp = traceHeaderSettings.iTimestamp;
                if (inPrimingData)
                    {
                    // This is possibly a priming trace, so increase potential number found for this GID
                    numPotentialPrimingTraces++;
                    // If data is to be verified, attempt the verification
                    if (aVerifyData)
                        {
                        dataVerificationErr = VerifyPrimingData(data, traceHeaderSettings.iLengthOfPayload, traceHeaderSettings.iCategory, traceHeaderSettings.iSubCategory, elementsFound, traceHeaderSettings.iFromTestWriter, aVerificationData1, aVerificationData2);
                        }
                    }
                else
                    {
                    // If we have enough potential priming traces, assume they are priming traces
                    if (numPotentialPrimingTraces >= KMinimumPrimingTracesInBatch)
                        {
                        aNumPrimingTraces += numPotentialPrimingTraces;
                        }
                    numPotentialPrimingTraces = 0;
                    }                
                }
            }
        else
            {
            // This is not a trace of the correct GID/CID
            inPrimingData = EFalse;
            }
        
        // Go to the next trace
        data+=traceHeaderSettings.iLengthOfPayload;

        if (data == endOfData)
            {
            // We might not have all the trace data, so read next chunk from file
            generalErr = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (generalErr != KErrNone)
                {
                if (err == KErrNone)
                    {
                    // Set error to return, if this is first error so far
                    err = generalErr;
                    }
                break;
                }
            }
        }

    if (!useMetaTraces)
        {
        // If we have enough potential priming traces left over, assume they are priming traces
        if (numPotentialPrimingTraces >= KMinimumPrimingTracesInBatch)
            {
            aNumPrimingTraces += numPotentialPrimingTraces;
            }
        }

    if ( (dataVerificationErr != KErrNone) && (err == KErrNone) )
        {
        // Set error to return, if this is first error so far
        err = dataVerificationErr;
        }

    CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
    CleanupStack::PopAndDestroy(&file); // close file

    return err;
    }

TInt TTraceDataParser::CreateFileBuffer(RBuf8&  aFileBuffer, 
                                        RFile&  aFile)
    {
    TInt fileSize;
    TInt err = aFile.Size(fileSize);
    if (err == KErrNone)
        {
        if (fileSize < KFileBufferSize)
            {
            err = aFileBuffer.Create(fileSize);
            }
        else
            {
            err = aFileBuffer.Create(KFileBufferSize);
            }
        }
    return err;
    }

TInt TTraceDataParser::ReadNextChunkFromFile(TDes8&   aFileBuffer, 
                                             RFile&   aFile, 
                                             TInt&    aFilePosition,
                                             TUint8*& aData,
                                             TUint8*& aStartOfData,
                                             TUint8*& aEndOfData)
    {
    aFilePosition += aData-aStartOfData;
    TInt err = aFile.Read(aFilePosition, aFileBuffer);
    if (err == KErrNone)
        {
        aData = (TUint8*) aFileBuffer.Ptr();
        aStartOfData = aData;
        aEndOfData = aData + aFileBuffer.Size();
        }
    return err;
    }

/*
 * Parse trace data for Printf data, returning the number of occurances of a given string
 *
 * @param aBuffer Buffer containing the trace data
 * @param aFindString String to search for
 * @param aNumFound Output parameter set to number of occurances of given string
 * @return Standard Symbian error code
 */
EXPORT_C TInt TTraceDataParser::DataHasPrintfString(TDesC8&        aBuffer,
                                                    const TDesC8&  aFindString,
                                                    TInt&          aNumFound)
    {
    aNumFound = 0;
    
    TInt            err = KErrNone;
    TUint8*         data = (TUint8*) aBuffer.Ptr();
    TUint8*         endOfData = data + aBuffer.Size();    
    
    TTraceHeaderSettings  traceHeaderSettings;

    while(data < endOfData)
        {
        err = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (err != KErrNone)
            {
            break;
            }
        if (traceHeaderSettings.iPrintfTrace)
            {
            TPtr8 printfString = ReadTracePrintf(data, traceHeaderSettings.iLengthOfPayload);
            if (StringsMatch(aFindString, printfString))
                {
                aNumFound++;
                }
            }
        data += traceHeaderSettings.iLengthOfPayload; //go to next trace
        }

    return err;
    }

/*
 * Parse trace data file for Printf data, returning the number of occurances of a given string
 *
 * @param aFilePath Full path of file containing the trace data
 * @param aFs File system object
 * @param aFindString String to search for
 * @param aNumFound Output parameter set to number of occurances of given string
 * @return Standard Symbian error code
 */
EXPORT_C TInt TTraceDataParser::DataHasPrintfStringL(const TDesC&   aFilePath,
                                                     RFs&           aFs,
                                                     const TDesC8&  aFindString,
                                                     TInt&          aNumFound)
    {
    aNumFound = 0;
    
    RFile file;

    // Open file with trace data
    TInt err = file.Open(aFs, aFilePath, EFileRead|EFileShareAny);
    if (err != KErrNone)
        {
        return err;
        }
    CleanupClosePushL(file);
    
    RBuf8   fileBuffer;
    TInt    filePos = 0;
    
    // Create file buffer and read first chunk from file
    err = CreateFileBuffer(fileBuffer, file);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }
    fileBuffer.CleanupClosePushL();
    
    TUint8* data = NULL;
    TUint8* startOfData = NULL;
    TUint8* endOfData = NULL;    

    err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }

    TTraceHeaderSettings  traceHeaderSettings;

    while(data < endOfData)
        {
        err = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (err == KErrOverflow)
            {
            // We don't have all the trace data, so read next chunk from file
            err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (err == KErrNone)
                {
                continue;
                }
            }
        if (err != KErrNone)
            {
            break;
            }
        if (traceHeaderSettings.iPrintfTrace)
            {
            TPtr8 printfString = ReadTracePrintf(data, traceHeaderSettings.iLengthOfPayload);
            if (StringsMatch(aFindString, printfString))
                {
                aNumFound++;
                }
            }
        data += traceHeaderSettings.iLengthOfPayload; //go to next trace

        if (data == endOfData)
            {
            // We might not have all the trace data, so read next chunk from file
            err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (err != KErrNone)
                {
                break;
                }
            }
        }

    CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
    CleanupStack::PopAndDestroy(&file); // close file

    return err;
    }

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
EXPORT_C TInt TTraceDataParser::DataHasTraceSequenceL(const TDesC&   aFilePath,
                                                      RFs&           aFs,
                                                      TInt&          aLastNumberFound,
                                                      TInt&          aNumDroppedTraces, 
                                                      TDesC8*        aFindPrintfPattern,
                                                      TGroupId       aGroupId,
                                                      TComponentId   aComponentID)
    {
    aLastNumberFound = 0;
    aNumDroppedTraces = 0;
    
    RFile file;

    // Open file with trace data
    TInt err = file.Open(aFs, aFilePath, EFileRead|EFileShareAny);
    if (err != KErrNone)
        {
        return err;
        }
    CleanupClosePushL(file);
    
    RBuf8   fileBuffer;
    TInt    filePos = 0;
    
    // Create file buffer and read first chunk from file
    err = CreateFileBuffer(fileBuffer, file);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }
    fileBuffer.CleanupClosePushL();
    
    TUint8* data = NULL;
    TUint8* startOfData = NULL;
    TUint8* endOfData = NULL;    

    err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
    if (err != KErrNone)
        {
        CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
        CleanupStack::PopAndDestroy(&file); // close file
        return err;
        }
    
    TInt            currentNumber = 0;
    TInt            expectedNumber = 1;
    TBool           gotNumberFromTrace = EFalse;
    TBool           isDroppedTrace = EFalse;

    TTraceHeaderSettings  traceHeaderSettings;

    while(data < endOfData)
        {
        err = ParseHeader(data, endOfData-data, traceHeaderSettings);
        
        if (err == KErrOverflow)
            {
            // We don't have all the trace data, so read next chunk from file
            err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (err == KErrNone)
                {
                continue;
                }
            }
        if (err != KErrNone)
            {
            break;
            }
        
        gotNumberFromTrace = EFalse;
                    
        if (traceHeaderSettings.iPrintfTrace && aGroupId == BTrace::ERDebugPrintf)
            {
            TPtr8 printfString = ReadTracePrintf(data, traceHeaderSettings.iLengthOfPayload);
            // See if this is a printf that matches given string
            if (!aFindPrintfPattern)
                {
                err = KErrArgument;
                break;
                }
            else if (printfString.Match(*aFindPrintfPattern) != KErrNotFound)
                {
                // Get the line number from the string
                err = ParseStringForNumber(printfString, *aFindPrintfPattern, currentNumber);
                if (err != KErrNone)
                    {
                    // Could not extract number from printf, so stop and fail
                    break;
                    }
                gotNumberFromTrace = ETrue;
                } 
            // See if this is a dropped trace
            else if (printfString.Compare(KDroppedTrace()) == 0)
                {
                // If we get 2 dropped trace notifications in a row, return an error
                if (isDroppedTrace)    
                    {
                    err = KErrGeneral;
                    break;
                    }
                isDroppedTrace = ETrue;
                }
            else
                {
                isDroppedTrace = EFalse;
                }
            }
        else if (!traceHeaderSettings.iPrintfTrace && traceHeaderSettings.iCategory == aGroupId && traceHeaderSettings.iComponentID == aComponentID)
            {
            // check dropped trace flag
            if (traceHeaderSettings.iHeaderFlags & BTrace::EMissingRecord)
                {
                // If we get 2 dropped trace notifications in a row, return an error
                if (isDroppedTrace)
                    {
                    err = KErrGeneral;
                    break;
                    }
                isDroppedTrace = ETrue;
                }
            else
                {
                isDroppedTrace = EFalse;
                }

            if (traceHeaderSettings.iLengthOfPayload >= 4)
                {
                currentNumber = (TInt) Swap(ReadUint32FromBuf(data, traceHeaderSettings.iFromTestWriter));
                traceHeaderSettings.iLengthOfPayload -= 4;
                gotNumberFromTrace = ETrue;
                }                
            else
                {
                // Could not get number from trace, so stop and fail
                err = KErrGeneral;
                break;
                }
            }

        if (gotNumberFromTrace)
            {
            if (currentNumber < expectedNumber)
                {
                // Start of a new sequence, so stop
                break;
                }
            aLastNumberFound = currentNumber;
            if (aLastNumberFound != expectedNumber && !isDroppedTrace)
                {
                // A printf has been missed out with no notification, so stop and fail
                err = KErrGeneral;
                break;
                }
            else if (aLastNumberFound == expectedNumber && isDroppedTrace)
                {
                // A printf hasn't been missed out despite notification, so stop and fail
                err = KErrGeneral;
                break;
                }
            
            aNumDroppedTraces += (aLastNumberFound - expectedNumber);
            expectedNumber = aLastNumberFound + 1;
            
            if (traceHeaderSettings.iPrintfTrace)
                {
                isDroppedTrace = EFalse;
                }
            }
        
        data += traceHeaderSettings.iLengthOfPayload; //go to next trace

        if (data == endOfData)
            {
            // We might not have all the trace data, so read next chunk from file
            err = ReadNextChunkFromFile(fileBuffer, file, filePos, data, startOfData, endOfData);
            if (err != KErrNone)
                {
                break;
                }
            }
        }

    CleanupStack::PopAndDestroy(&fileBuffer); // close file buffer
    CleanupStack::PopAndDestroy(&file); // close file

    return err;
    }

/*
 * Get Printf string from single trace data
 *
 * @param aBuffer Buffer containing the trace data
 * @param aPrintfString Output parameter set to the print string in the trace data
 * @return Standard Symbian error code
 */
EXPORT_C TInt TTraceDataParser::GetPrintfFromTrace(TDesC8& aBuffer, TDes8& aPrintfString)
    {
    TUint8*         data = (TUint8*) aBuffer.Ptr();

    TTraceHeaderSettings  traceHeaderSettings;

    TInt err = ParseHeader(data, aBuffer.Size(), traceHeaderSettings);

    if (err == KErrNone && traceHeaderSettings.iPrintfTrace)
        {
        aPrintfString.Copy(ReadTracePrintf(data, traceHeaderSettings.iLengthOfPayload));
        }
    
    return err;
    }

/*
 * Parse trace data for multipart data, returning the number of traces found and total size of trace data.
 * The data is expected to contain a sequence of integers, starting at 0
 *
 * @param aBuffer Buffer containing the trace data
 * @param aGroupId Group ID of traces to parse
 * @param aComponentID Component ID of traces to parse
 * @param aMultipartDataSize Output parameter set to total size of data in multipart trace
 * @param aNumMultipartTraces Output parameter set to number of traces that make up multipart trace
 * @return Standard Symbian error code
 */
EXPORT_C TInt TTraceDataParser::ValidateMultipartTraces(TDesC8&        aBuffer,
                                                        TGroupId       aGroupID, 
                                                        TComponentId   aComponentID, 
                                                        TInt&          aMultipartDataSize, 
                                                        TInt&          aNumMultipartTraces)
    {
    aMultipartDataSize = 0;
    aNumMultipartTraces = 0;
    
    TInt            err = KErrNone;
    TUint8*         data = (TUint8*) aBuffer.Ptr();
    TUint8*         endOfData = data + aBuffer.Size();    
    TUint8          currentNumber = 0;
    TUint8          expectedNumber = 0;
    TUint32         expectedMultiPartTraceID = 0;
    TBool           foundStartOfMultipart = EFalse;
    TBool           foundEndOfMultipart = EFalse;
    TBool           isMultipartTrace = EFalse;
    
    TTraceHeaderSettings  traceHeaderSettings;

    while(data < endOfData)
        {
        err = ParseHeader(data, endOfData-data, traceHeaderSettings);
        if (err != KErrNone)
            {
            return err;
            }
               
        // Only look at traces with correct group ID / component ID
        if (traceHeaderSettings.iCategory == aGroupID && traceHeaderSettings.iComponentID == aComponentID)
            {
            isMultipartTrace = EFalse;
            if (traceHeaderSettings.iMultiPartType == BTrace::EMultipartFirst ||
                traceHeaderSettings.iMultiPartType == BTrace::EMultipartMiddle ||
                traceHeaderSettings.iMultiPartType == BTrace::EMultipartLast)
                {
                isMultipartTrace = ETrue;
                }
            
            if (!isMultipartTrace)
                {
                // If not a multipart trace, then this one trace should contain all data
                aNumMultipartTraces = 1;
                aMultipartDataSize = traceHeaderSettings.iLengthOfPayload;
                // If already found some of the data then we shouldn't have this trace
                if (foundStartOfMultipart || foundEndOfMultipart)
                    {
                    return KErrCorrupt;
                    }
                foundStartOfMultipart = ETrue;
                foundEndOfMultipart = ETrue;

                // Validate payload
                while (traceHeaderSettings.iLengthOfPayload > 0)
                    {
                    currentNumber = (TInt) data[0];
                    data++;
                    // Adjust length of payload remaining
                    traceHeaderSettings.iLengthOfPayload -= 1;   
                    if (currentNumber != expectedNumber)
                        {
                        if (traceHeaderSettings.iLengthOfPayload >= 4 || currentNumber != 0)
                            {
                            // Data is 4-byte aligned, so it's ok to have up to 3 zeros at the end of payload
                            return KErrCorrupt;
                            }
                        // This byte is zero padding, so not part of data
                        aMultipartDataSize--;
                        }
                    expectedNumber++;
                    }
                }
            // Only look at multipart traces with correct multipart trace ID
            else if (traceHeaderSettings.iMultiPartTraceID == expectedMultiPartTraceID || expectedMultiPartTraceID == 0)
                {
                aNumMultipartTraces++;
                
                // If already found all data then we shouldn't get another trace with this ID
                if (foundEndOfMultipart)
                    {
                    return KErrCorrupt;
                    }
                
                // Trace is start of multipart
                if (traceHeaderSettings.iMultiPartType == BTrace::EMultipartFirst)
                    {
                    // If already found start of data then we shouldn't get this part
                    if (foundStartOfMultipart)
                        {
                        return KErrCorrupt;
                        }
                    foundStartOfMultipart = ETrue;
                    aMultipartDataSize = 0;
                    
                    // Set the expected multipart trace ID
                    expectedMultiPartTraceID = traceHeaderSettings.iMultiPartTraceID;                    
                    }
                else if (traceHeaderSettings.iMultiPartType == BTrace::EMultipartMiddle ||
                         traceHeaderSettings.iMultiPartType == BTrace::EMultipartLast)
                    {
                    // If not yet found start of data then we shouldn't get this part
                    if (!foundStartOfMultipart)
                        {
                        return KErrCorrupt;
                        }
                    }
                
                // Add length of payload to size of total data found
                aMultipartDataSize += traceHeaderSettings.iLengthOfPayload;

                while (traceHeaderSettings.iLengthOfPayload > 0)
                    {
                    // Validate the next part of payload
                    currentNumber = (TInt) data[0];
                    data++;
                    // Adjust length of payload remaining
                    traceHeaderSettings.iLengthOfPayload -= 1;   
                    if (currentNumber != expectedNumber)
                        {
                        if (traceHeaderSettings.iLengthOfPayload >= 4 || currentNumber != 0)
                            {
                            // Data is 4-byte aligned, so it's ok to have up to 3 zeros at the end of payload
                            return KErrCorrupt;
                            }
                        // This byte is zero padding, so not part of data
                        aMultipartDataSize--;
                        }
                    expectedNumber++;
                    }

                if (traceHeaderSettings.iMultiPartType == BTrace::EMultipartLast)
                    {                    
                    // Found end of trace data
                    foundEndOfMultipart = ETrue;
                    }
                }
            }
        
        data += traceHeaderSettings.iLengthOfPayload; //go to next trace
        }

    if (!foundEndOfMultipart)
        {
        // Did not find end of trace data, so return KErrNotFound
        err = KErrNotFound;
        }
    return err;
    }
