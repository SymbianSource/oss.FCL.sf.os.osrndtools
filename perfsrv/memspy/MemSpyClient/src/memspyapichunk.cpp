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

#include <memspy/api/memspyapichunk.h>
#include <memspy/engine/memspychunkdata.h>


EXPORT_C CMemSpyApiChunk::~CMemSpyApiChunk()
	{
	delete iChunkData;
	}

EXPORT_C const TDesC& CMemSpyApiChunk::Name() const
    {
    return iChunkData->iName;
    }

EXPORT_C const TDesC& CMemSpyApiChunk::OwnerName() const
	{
	return iChunkData->iOwnerName;
	}

EXPORT_C TUint8* CMemSpyApiChunk::Handle() const
    {
    return iChunkData->iHandle;
    }

EXPORT_C TUint8* CMemSpyApiChunk::BaseAddress() const
    {
    return iChunkData->iBaseAddress;
    }

EXPORT_C TUint CMemSpyApiChunk::Size() const
    {
    return iChunkData->iSize;
    }

EXPORT_C TUint CMemSpyApiChunk::MaxSize() const
    {
    return iChunkData->iMaxSize;
    }

EXPORT_C TUint CMemSpyApiChunk::OwnerId() const
    {
    return iChunkData->iOwnerId;
    }

EXPORT_C TMemSpyDriverChunkType CMemSpyApiChunk::ChunkType() const
    {
    return iChunkData->iType;
    }

EXPORT_C TInt CMemSpyApiChunk::Attributes() const
    {
    return iChunkData->iAttributes;
    }

CMemSpyApiChunk::CMemSpyApiChunk() : iChunkData(0)
	{
	}

void CMemSpyApiChunk::ConstructL(const TMemSpyChunkData& aData)
	{
	iChunkData = new (ELeave) TMemSpyChunkData(aData);
	}

CMemSpyApiChunk* CMemSpyApiChunk::NewL(const TMemSpyChunkData& aData)
	{
    CMemSpyApiChunk* self = CMemSpyApiChunk::NewLC(aData);
	CleanupStack::Pop(self);
	return (self);
	}

CMemSpyApiChunk* CMemSpyApiChunk::NewLC(const TMemSpyChunkData& aData)
	{
    CMemSpyApiChunk* self = new (ELeave) CMemSpyApiChunk;
	CleanupStack::PushL(self);
	self->ConstructL(aData);
	return (self);
	}
