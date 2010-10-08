// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalTechnology
*/
#include "te_processcreator.h"


TProcessCreatorResults CProcessCreator::StartProgram(const TDesC& aProgramName, 
													 const TDesC& aCommandLineArgs, 
													 TBool aWaitForCompletion)
    {
    RProcess process;
	TProcessCreatorResults res;
	res.iExitType = EExitPending;
	res.iCode = process.Create(aProgramName, aCommandLineArgs);
	if(res.iCode == KErrNone)
		{
		if(aWaitForCompletion)
		    {
            TRequestStatus status = KRequestPending;
			process.Logon(status);
			process.Resume();
			User::WaitForRequest(status);
            if(status.Int() == KErrNone)
                {
                res.iCode = process.ExitReason();
                res.iExitType = process.ExitType();
                res.iDesc.Copy(process.ExitCategory());
                }
            else
                {
                res.iCode = status.Int();
                }
            }
		else
		    {
            process.Resume();
		    }
		}
	process.Close();
	
	return res;
    }

