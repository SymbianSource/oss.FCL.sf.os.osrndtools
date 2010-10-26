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
* Contains the declaration of  CMemSpyCodeSegment class 
*/

#ifndef MEMSPYAPICODESEGMENT_H
#define MEMSPYAPICODESEGMENT_H

// System includes
#include <e32base.h>

#include <memspy/driver/memspydriverenumerationsshared.h>

class TMemSpyCodeSegmentData;

NONSHARABLE_CLASS( CMemSpyApiCodeSegment ) 
	{
public:
	IMPORT_C ~CMemSpyApiCodeSegment();
	
	static CMemSpyApiCodeSegment* NewL(const TMemSpyCodeSegmentData& aData);

	static CMemSpyApiCodeSegment* NewLC(const TMemSpyCodeSegmentData& aData);		
	
public:
	IMPORT_C const TDesC& Name() const;
	
	IMPORT_C TInt CodeSize() const;
	
	IMPORT_C TInt TotalDataSize() const;
	
	IMPORT_C TInt TextSize() const;
	
	IMPORT_C TInt DataSize() const;
	
	IMPORT_C TInt BssSize() const;
	
	IMPORT_C TUidType Uids() const;
	
	IMPORT_C TUint32 ModuleVersion() const;
	
	IMPORT_C TUint32 SID() const;
	
	IMPORT_C TUint32 VID() const;
	
	IMPORT_C TUint32 EntryPtVeneer() const;		// address of first instruction to be called
	
	IMPORT_C TUint32 FileEntryPoint() const;	// address of entry point within this code segment
	
	IMPORT_C TInt DepCount() const;
	
	IMPORT_C TUint32 CodeLoadAddress() const;	// 0 for RAM loaded code, else pointer to TRomImageHeader
	
	IMPORT_C TUint32 DataLoadAddress() const;
	
	IMPORT_C TUint32 Capabilities1() const;
	
	IMPORT_C TUint32 Capabilities2() const;
				
private:
	CMemSpyApiCodeSegment();
	
	void ConstructL(const TMemSpyCodeSegmentData& aData);
		
private:
	TMemSpyCodeSegmentData* iCodeSegmentData;
};

	
#endif // MEMSPYAPICODESEGMENT_H
