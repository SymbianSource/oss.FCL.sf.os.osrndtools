/**
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file
 @internalTechnology
*/
#ifndef __TE_PROCESSCREATOR_H__
#define __TE_PROCESSCREATOR_H__

#include <e32base.h>

struct TProcessCreatorResults
    {
	TInt iCode;
	TExitType iExitType;
	TBuf<64> iDesc;
    };

class CProcessCreator : public CBase
{
public:
	static TProcessCreatorResults StartProgram(const TDesC& aProgramName, 
			const TDesC& aCommandLineArgs, TBool aWaitForCompletion=ETrue);
	
};

#endif // __TE_PROCESSCREATOR_H__
