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
* Contains the declaration of  CMemSpyChunk class 
*/

#ifndef MEMSPYAPICHUNK_H
#define MEMSPYAPICHUNK_H

// System includes
#include <e32base.h>

#include <memspy/driver/memspydriverenumerationsshared.h>

class TMemSpyChunkData;

NONSHARABLE_CLASS( CMemSpyApiChunk ) 
	{
public:
	IMPORT_C ~CMemSpyApiChunk();
	
	static CMemSpyApiChunk* NewL(const TMemSpyChunkData& aData);

	static CMemSpyApiChunk* NewLC(const TMemSpyChunkData& aData);		
	
public:
	IMPORT_C const TDesC& Name() const;
	
	IMPORT_C const TDesC& OwnerName() const;
	
	IMPORT_C TUint8* Handle() const;
	
	IMPORT_C TUint8* BaseAddress() const;
	
	IMPORT_C TUint Size() const;
	
	IMPORT_C TUint MaxSize() const;
	
	IMPORT_C TUint OwnerId() const;
	
	IMPORT_C TMemSpyDriverChunkType ChunkType() const;
	
	IMPORT_C TInt Attributes() const;	
	
private:
	CMemSpyApiChunk();
	
	void ConstructL(const TMemSpyChunkData& aData);
		
private:
	TMemSpyChunkData *iChunkData;
};

	
#endif // MEMSPYAPICHUNK_H
