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
* Contains the declaration of  TMemSpyProcessData class
*/

#ifndef MEMSPYTHREADINFOITEMDATA_H
#define MEMSPYTHREADINFOITEMDATA_H

#include <memspy/driver/memspydriverobjectsshared.h>

const TInt KMaxBufSize = 32;

// TMemSpyProcess data class holds data to be sent to the UI
class TMemSpyThreadInfoItemData 
	{	
public:
	//constructor & destructor
	inline TMemSpyThreadInfoItemData()		
		{
		}
	
public:
    TBuf<KMaxBufSize> iCaption;
    TBuf<KMaxBufSize> iValue;
	};

#endif // MEMSPYTHREADINFOITEMDATA_H
