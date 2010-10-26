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
* Contains the declaration of  TMemSpyChunkData class
*/

#ifndef MEMSPYCODESEGMENTDATA_H
#define MEMSPYCODESEGMENTDATA_H

#include <memspy/driver/memspydriverenumerationsshared.h>

// TMemSpyCodeSegmentData class holds data to be sent to the UI
class TMemSpyCodeSegmentData 
	{	
public:
	//constructor & destructor
	inline TMemSpyCodeSegmentData()
		: iCodeSize(0), iTotalDataSize(0), iTextSize(0), iDataSize(0), iBssSize(0), iSecureId(0), iVendorId(0), iEntryPtVeneer(0),
		  iFileEntryPoint(0), iDepCount(0), iCodeLoadAddress(0), iDataLoadAddress(0), iCapabilities1(0), iCapabilities2(0)
		{
		}
	
public:
	TFullName iName;
	TInt iCodeSize;
	TInt iTotalDataSize;
	TInt iTextSize;
	TInt iDataSize;
	TInt iBssSize;
	TUidType iUids;
	TUint32 iModuleVersion;
	TUint32	iSecureId;
	TUint32	iVendorId;
	TUint32 iEntryPtVeneer;		// address of first instruction to be called
	TUint32 iFileEntryPoint;	// address of entry point within this code segment
	TInt iDepCount;
	TUint32 iCodeLoadAddress;	// 0 for RAM loaded code, else pointer to TRomImageHeader
	TUint32 iDataLoadAddress;
	TUint32 iCapabilities1;
	TUint32 iCapabilities2;
	};


#endif // MEMSPYCODESEGMENTDATA_H
