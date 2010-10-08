// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <kernel/kernel.h>
#include "trace_certification.h"

/*
----------------------------------------------------------------------------

    IsTraceCertified

    Stubbed out functionality - tracing always certified
    
    @Return ETrue
    
-----------------------------------------------------------------------------
*/

EXPORT_C TBool TraceCertification::IsTraceCertified()
    {
    return ETrue;
    }
