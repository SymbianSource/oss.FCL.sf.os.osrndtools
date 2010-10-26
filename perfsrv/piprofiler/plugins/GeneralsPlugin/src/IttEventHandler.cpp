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
* Description:  Event based ITT sampler skeleton copypasted from MemoryEventHandler.cpp
*
*/

#include <e32def.h>
#include <e32cmn.h>
#include <arm.h>
#include <kernel.h>
#include <kern_priv.h>
#include <nk_trace.h>

#include "IttEventHandler.h"

/*
 * Constructor
 * 
 * @param DProfilerSampleBuffer*    pointer to sample buffer
 */
DIttEventHandler::DIttEventHandler(DProfilerSampleBuffer* aSampleBuffer, TProfilerGppSamplerData* aGppSamplerDataIn)
    :   DKernelEventHandler(EventHandler, this),
        iSampleBuffer(aSampleBuffer),
        iSampleDescriptor(&(this->iSample[1]),0,KITTBufferSize),
        gppSamplerData(aGppSamplerDataIn)
    {
    iSampleAvailable = false;
    }

/*
 * DIttEventHandler::Create()
 */
TInt DIttEventHandler::Create()
    {
    TInt err(Kern::MutexCreate(iLock, _L("IttEventHandlerLock"), KMutexOrdDebug));
    if (err != KErrNone)
        return err;
    
    return Add();
    }

/*
 * Destructor
 */
DIttEventHandler::~DIttEventHandler()
    {
    LOGSTRING("DIttEventHandler::~DIttEventHandler()");

    if (iLock)
        iLock->Close(NULL);
    }


TInt DIttEventHandler::Start()
    {
    LOGSTRING("DIttEventHandler::Start()");

    iTracking = ETrue;
    return KErrNone;
    }


TInt DIttEventHandler::Stop()
    {
    LOGSTRING("DIttEventHandler::Stop()");

    iTracking = EFalse;
    LOGSTRING2("ITT - gpp sample counter %d", this->gppSamplerData->sampleNumber);
    return KErrNone;
    }

TBool DIttEventHandler::SampleNeeded()
    {
    //LOGSTRING("DIttEventHandler::SampleNeeded()");
    
    // check if event handler was not running
    if(!iTracking)
        {
        return false;
        }
    // check if a new sample is available
    if(iSampleAvailable)
        {
        return true;
        }
    else
        {
        return false;
        }
    }

void DIttEventHandler::SampleHandled()
    {
    iSampleAvailable = false;
    }

TUint DIttEventHandler::EventHandler(TKernelEvent aType, TAny* a1, TAny* a2, TAny* aThis)
    {
    //LOGSTRING("DIttEventHandler::EventHandler()");
    return ((DIttEventHandler*)aThis)->HandleEvent(aType, a1, a2);
    }



TUint DIttEventHandler::HandleEvent(TKernelEvent aType, TAny* a1, TAny* a2)
    {
    
    if (iTracking)
        {
        switch (aType)
            {
            case EEventAddCodeSeg:
                LOGSTRING("DIttEventHandler::HandleEvent() EEventAddCodeSeg received");
                HandleAddCodeSeg((DCodeSeg*)a1);
                break;
                
            case EEventRemoveCodeSeg:
                LOGSTRING("DIttEventHandler::HandleEvent() EEventRemoveCodeSeg received");
                HandleRemoveCodeSeg((DCodeSeg*)a1);
                break;
   
            default:
                break;
            }
        }
    return DKernelEventHandler::ERunNext;
    }

/*
 * 
 */
TBool DIttEventHandler::HandleAddCodeSeg(DCodeSeg* aSeg)
    {    
    LOGSTRING("DIttEventHandler::HandleAddCodeSeg()");
    iSampleDescriptor.Zero();
    //Kern::Printf("DLL ADD: NM %S : RA:0x%x SZ:0x%x SN:0x%x",aSeg->iFileName,aSeg->iRunAddress,aSeg->iSize, this->gppSamplerData->sampleNumber);

    NKern::ThreadEnterCS();
    Kern::MutexWait(*iLock);
    iSample[0] = aSeg->iFileName->Length();
    iSampleDescriptor.Append(*aSeg->iFileName);
    iSampleDescriptor.Append((TUint8*)&(aSeg->iRunAddress),4);
    iSampleDescriptor.Append((TUint8*)&(aSeg->iSize),4);
    iSampleDescriptor.Append((TUint8*)&(this->gppSamplerData->sampleNumber),4);
    iSample[0] = iSampleDescriptor.Size();
   
    iSampleBuffer->AddSample(iSample,iSampleDescriptor.Size()+1);
    Kern::MutexSignal(*iLock);
    NKern::ThreadLeaveCS();
    
    iSampleAvailable = true;
    return ETrue;
    }

TBool DIttEventHandler::HandleRemoveCodeSeg(DCodeSeg* aSeg)
    {
    LOGSTRING("DIttEventHandler::HandleRemoveCodeSeg()");
    iSampleDescriptor.Zero();
    //Kern::Printf("DLL REM: NM %S : RA:0x%x SZ:0x%x SN:0x%x",aSeg->iFileName,aSeg->iRunAddress,aSeg->iSize, this->gppSamplerData->sampleNumber);

    NKern::ThreadEnterCS();
    Kern::MutexWait(*iLock);
    
    iSample[0] = aSeg->iFileName->Length();
    iSampleDescriptor.Append(*aSeg->iFileName);
    iSampleDescriptor.Append((TUint8*)&(aSeg->iRunAddress),4);
    iSampleDescriptor.Append((TUint8*)&(aSeg->iSize),4);
    iSampleDescriptor.Append((TUint8*)&(this->gppSamplerData->sampleNumber),4);
    iSample[0] = iSampleDescriptor.Size();

    iSampleBuffer->AddSample(iSample,iSampleDescriptor.Size()+1);
    Kern::MutexSignal(*iLock);
    NKern::ThreadLeaveCS();

    iSampleAvailable = true;
    return ETrue;
    }
// end of file
