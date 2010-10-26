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

#ifndef MEMSPYCHUNKDATA_H
#define MEMSPYCHUNKDATA_H

#include <memspy/driver/memspydriverenumerationsshared.h>

// TMemSpyChunkData class holds data to be sent to the UI
class TMemSpyChunkData 
	{	
public:
	//constructor & destructor
	inline TMemSpyChunkData()
        : iHandle(0), iBaseAddress(0), iSize(0), iMaxSize(0), iOwnerId(0), iAttributes(0)
		{
		}
	
public:
	TFullName iName;
	TFullName iOwnerName;
	TUint8* iHandle;
	TUint8* iBaseAddress;
	TUint iSize;
	TUint iMaxSize;
	TUint iOwnerId;
	TMemSpyDriverChunkType iType;
	TInt iAttributes;
	};


#endif // MEMSPYCHUNKDATA_H
