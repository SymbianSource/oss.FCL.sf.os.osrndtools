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

#include <memspy/api/memspyapicodesegment.h>
#include <memspy/engine/memspycodesegmentdata.h>


EXPORT_C CMemSpyApiCodeSegment::~CMemSpyApiCodeSegment()
	{
	delete iCodeSegmentData;
	}

EXPORT_C const TDesC& CMemSpyApiCodeSegment::Name() const
    {
    return iCodeSegmentData->iName;
    }

EXPORT_C TInt CMemSpyApiCodeSegment::CodeSize() const
	{
	return iCodeSegmentData->iCodeSize;
	}

EXPORT_C TInt CMemSpyApiCodeSegment::TotalDataSize() const
	{
	return iCodeSegmentData->iTotalDataSize;
	}

EXPORT_C TInt CMemSpyApiCodeSegment::TextSize() const
	{
	return iCodeSegmentData->iTextSize;
	}

EXPORT_C TInt CMemSpyApiCodeSegment::DataSize() const
	{
	return iCodeSegmentData->iDataSize;
	}

EXPORT_C TInt CMemSpyApiCodeSegment::BssSize() const
	{
	return iCodeSegmentData->iBssSize;
	}

EXPORT_C TUidType CMemSpyApiCodeSegment::Uids() const
	{
	return iCodeSegmentData->iUids;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::ModuleVersion() const
	{
	return iCodeSegmentData->iModuleVersion;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::SID() const
	{
	return iCodeSegmentData->iSecureId;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::VID() const
	{
	return iCodeSegmentData->iVendorId;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::EntryPtVeneer() const		// address of first instruction to be called
	{
	return iCodeSegmentData->iEntryPtVeneer;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::FileEntryPoint() const	// address of entry point within this code segment
	{
	return iCodeSegmentData->iFileEntryPoint;
	}

EXPORT_C TInt CMemSpyApiCodeSegment::DepCount() const
	{
	return iCodeSegmentData->iDepCount;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::CodeLoadAddress() const	// 0 for RAM loaded code, else pointer to TRomImageHeader
	{
	return iCodeSegmentData->iCodeLoadAddress;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::DataLoadAddress() const
	{
	return iCodeSegmentData->iDataLoadAddress;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::Capabilities1() const
	{
	return iCodeSegmentData->iCapabilities1;
	}

EXPORT_C TUint32 CMemSpyApiCodeSegment::Capabilities2() const
	{
	return iCodeSegmentData->iCapabilities2;
	}

CMemSpyApiCodeSegment::CMemSpyApiCodeSegment() : iCodeSegmentData(0)
	{
	}

void CMemSpyApiCodeSegment::ConstructL(const TMemSpyCodeSegmentData& aData)
	{
	iCodeSegmentData = new (ELeave) TMemSpyCodeSegmentData(aData);
	}

CMemSpyApiCodeSegment* CMemSpyApiCodeSegment::NewL(const TMemSpyCodeSegmentData& aData)
	{
	CMemSpyApiCodeSegment* self = CMemSpyApiCodeSegment::NewLC(aData);
	CleanupStack::Pop(self);
	return (self);
	}

CMemSpyApiCodeSegment* CMemSpyApiCodeSegment::NewLC(const TMemSpyCodeSegmentData& aData)
	{
	CMemSpyApiCodeSegment* self = new (ELeave) CMemSpyApiCodeSegment;
	CleanupStack::PushL(self);
	self->ConstructL(aData);
	return (self);
	}
