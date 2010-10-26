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
* Description:  
*
*/


#include <piprofiler/ProfilerVersion.h>

#include <kern_priv.h>
#include <plat_priv.h>

#include "IttSamplerImpl.h"

#ifdef ITT_EVENT_HANDLER
_LIT8(KIttVersion, "2.00");
#else
_LIT8(KIttVersion, "1.22");
#endif

/*
 *	ITT sampler definition
 *  	
 */
IttSamplerImpl::IttSamplerImpl():
        sampleDescriptor(&(this->sample[1]),0,KITTSampleBufferSize)
{
	iCurrentLibCount = 0;
	iLibsCount = 0;
	iCodeSegsCount = 0;
	iTimeToSample = ETrue;
	iInitState = KIttHandlingLibs;
	this->Reset();
}

/*
 * destructor
 */
IttSamplerImpl::~IttSamplerImpl()
{
    LOGSTRING("IttSamplerImpl::~IttSamplerImpl()");
}

/*
 * IttSamplerImpl::CreateFirstSample()
 * 
 * Function for creating the first sample to the log file
 */
TInt IttSamplerImpl::CreateFirstSample() 
{	
	this->iVersionData.Zero();
	this->iVersionData.Append(_L8("Bappea_ITT_V"));
	this->iVersionData.Append(KIttVersion);
	this->itt_sample = (TUint8*)iVersionData.Ptr();
	return iVersionData.Length();
}

/*
 * IttSamplerImpl::SampleNeeded(TUint32 sampleNum)
 * 
 * @param TUint32 Sample number
 * 
 */
TBool IttSamplerImpl::SampleNeeded(TUint32 sampleNum)
    {
    iCount = sampleNum;
    LOGSTRING2("IttSamplerImpl::SampleNeeded - time: %d", iCount);

    if ((iCount % iIttSamplingPeriod) == 0)
        {
        return true;
        }
    return false;
    }

/*
 * IttSamplerImpl::SampleImpl(TUint32 pc, TUint32 sampleNum)
 * 
 * @param TUint32 program counter
 * @param TUint32 sample number
 */
TInt IttSamplerImpl::SampleImpl(TUint32 pc,TUint32 sampleNum)
    {	
    //LOGSTRING3("IttSamplerImpl::SampleImpl pc %d samplenum %d",pc, sampleNum);
    // in order to avoid overloading the interrupt
	// only one dynamic file in each 50ms is added to the stream
	// with the application of the tool in mind, this is
	// a reasonable measure
    TInt ret(0);
    // encode a process binary
 
    //sampleDescriptor.Zero();
    if(iTimeToSample)
        {
		// encode a library binary
		//sampleDescriptor.Zero();
			if(iInitState == KIttHandlingLibs)
		    {
            if(iLibsCount < 20)
                {
                ret = HandleLibs(sampleNum);
                iLibsCount++;
                while(ret == KErrAlreadyExists)
                    {
                    ret = HandleLibs(sampleNum);
                    iLibsCount++;
                    }
                }
            else
                {
                iLibsCount = 0;
                return 0;
                }
            }
		//LOGSTRING2("IttSamplerImpl::SampleImpl HandledLibs %d", ret);
		// one library has been read to sample, needs to be written to file

		else if(iInitState == KIttHandlingCodeSegs)
		    {
		    if(iCodeSegsCount < 20)
		        {
		        ret = HandleSegs(sampleNum);
		        iCodeSegsCount++;
                while(ret == KErrAlreadyExists)
                    {
                    ret = HandleSegs(sampleNum);
                    iCodeSegsCount++;
                    }
		        }
		    
		    else
		        {
		        iCodeSegsCount = 0;
		        return 0;
		        }
		    }

		//LOGSTRING2("IttSamplerImpl::SampleImpl HandledSegs %d", ret);
		else
		    {
		    // nothing to do
		    Kern::Printf("IttSamplerImpl::SampleImpl - should not be here");
		    return 0;
		    }
        }
    else
        {
        LOGSTRING("IttSamplerImpl::SampleImpl Not time to sample");
	}

	return ret;
}

/*
 * IttSamplerImpl::Reset()
 */
void IttSamplerImpl::Reset()
    {
    iTimeToSample = ETrue;
#ifdef ITT_EVENT_HANDLER
    iInitialLibsTaken= EFalse;
    iInitialSegsTaken= EFalse;
#endif
	iCurrentLibCount = 0;
	iLibsCount = 0;
	iCodeSegsCount = 0;
	iLatestCodeseg = NULL;
	this->itt_sample = (TUint8*)&(this->sample[0]);
	sampleDescriptor.Zero();

//	#ifdef ITT_TEST	
	SDblQue* codeSegList = Kern::CodeSegList();
	// the global list
	for (SDblQueLink* codeseg= codeSegList->First(); codeseg!=(SDblQueLink*) codeSegList; codeseg=codeseg->iNext)
	    {				
		DCodeSeg* seg = _LOFF(codeseg,DCodeSeg, iLink);
		//if(seg != 0)
            {
			if( (seg->iMark & 0x80) > 0)
			    {
				seg->iMark = (seg->iMark & ~0x80);
			    }
            }
	    }	
	// the garbage list
	
	NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
	DObjectCon* libs = Kern::Containers()[ELibrary];
	libs->Wait();
	TInt libCount = libs->Count();
	for(TInt i=0;i<libCount;i++)
	    {
		DLibrary* lib = (DLibrary*)(*libs)[i];
		DCodeSeg* seg = lib->iCodeSeg;
		if( (seg->iMark & 0x80) > 0)
		    {
			seg->iMark = (seg->iMark & ~0x80);
		    }
	    }
    libs->Signal();
	NKern::ThreadLeaveCS();
	//#endif   //ITT_TEST
}
/*
 * 
 */
TInt IttSamplerImpl::HandleLibs(TUint32 sampleNum)
    {
    NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
    DObjectCon* libs = Kern::Containers()[ELibrary];
    libs->Wait();
    TInt libCount(libs->Count());
    sampleDescriptor.Zero();
    
    // go 20 binaries through at a time
    //for(TInt i=0;i<libCount;i++)
    if (iCurrentLibCount < libCount)
        {
        // get libs from 
        DLibrary* lib = (DLibrary*)(*libs)[iCurrentLibCount];
        libs->Signal();
        NKern::ThreadLeaveCS();
        iCurrentLibCount++;
        
        DCodeSeg* seg = lib->iCodeSeg;
        if(seg != 0)
            {
            if( (seg->iMark & 0x80) == 0)
                {
                this->sample[0] = seg->iFileName->Length();
                sampleDescriptor.Append(*(seg->iFileName));
                sampleDescriptor.Append((TUint8*)&(seg->iRunAddress),4);
                sampleDescriptor.Append((TUint8*)&(seg->iSize),4);

#ifdef ITT_EVENT_HANDLER
                sampleDescriptor.Append((TUint8*)&(sampleNum),4);
                //Kern::Printf("DLL: NM %S : RA:0x%x SZ:0x%x, SN:0x%x",seg->iFileName,seg->iRunAddress,seg->iSize, sampleNum);
                //this->iFirstSampleTaken = ETrue;
#else
                //Kern::Printf("DLL: NM %S : RA:0x%x SZ:0x%x",seg->iFileName,seg->iRunAddress,seg->iSize);
#endif
                seg->iMark = (seg->iMark | 0x80);
                this->sample[0] = sampleDescriptor.Size();
                return sampleDescriptor.Size()+1;
                }
            else 
                {
                //Kern::Printf("Already met DLL: NM %S : RA:0x%x SZ:0x%x, SN:0x%x",seg->iFileName,seg->iRunAddress,seg->iSize, sampleNum);
                return KErrAlreadyExists;
                }
            }
        }
    else
        {
        // check if list gone through
        iInitState = KIttHandlingCodeSegs;
        iInitialLibsTaken = ETrue;
        }
    libs->Signal();
    NKern::ThreadLeaveCS();
    return 0;
    }
/*
 * 
 */
TInt IttSamplerImpl::HandleSegs(TUint32 sampleNum)
    {
    SDblQue* codeSegList = Kern::CodeSegList();
    //Kern::Printf("PI");
    TUint count(0);
    sampleDescriptor.Zero();
    if(iLatestCodeseg == NULL)
        {
        iLatestCodeseg = codeSegList->First();
        }
    else
        {
        iLatestCodeseg = iLatestCodeseg->iNext;
        }
    
    // search the global code segment list
    //for (; iLatestCodeseg!=(SDblQueLink*) codeSegList; iLatestCodeseg=iLatestCodeseg->iNext)
    if (iLatestCodeseg != (SDblQueLink*) codeSegList)
        {             
        DCodeSeg* seg = _LOFF(iLatestCodeseg, DCodeSeg, iLink);
        if(seg != 0)
            {
            if( (seg->iMark & 0x80) == 0)
                {
                this->sample[0] = seg->iFileName->Length();
                sampleDescriptor.Append(*(seg->iFileName));
                sampleDescriptor.Append((TUint8*)&(seg->iRunAddress),4);
                sampleDescriptor.Append((TUint8*)&(seg->iSize),4);
#ifdef ITT_EVENT_HANDLER
                sampleDescriptor.Append((TUint8*)&(sampleNum),4);
                //Kern::Printf("EXE2: NM %S : RA:0x%x SZ:0x%x, time: %d",seg->iFileName,seg->iRunAddress,seg->iSize, sampleNum);
#else
                //Kern::Printf("EXE2: NM %S : RA:0x%x SZ:0x%x, time: %d",seg->iFileName,seg->iRunAddress,seg->iSize, sampleNum);
#endif                  
                seg->iMark = (seg->iMark | 0x80);
                count++;
                this->sample[0] = sampleDescriptor.Size();
                return sampleDescriptor.Size()+1;
                }
            else 
                {
                //Kern::Printf("Already met EXE2: NM %S : RA:0x%x SZ:0x%x, time: %d",seg->iFileName,seg->iRunAddress,seg->iSize, sampleNum);
                return KErrAlreadyExists;
                }
            }
        }
    // check if list gone through
    else //if (count == 0)
        {
        iInitialSegsTaken = ETrue;
        iTimeToSample = false;
        LOGSTRING("ITT sampler - all initial samples generated!");
        }
    return 0;
    }

// end of file
