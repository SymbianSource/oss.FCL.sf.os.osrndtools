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

#ifndef __PIPROFILERTEST_H__
#define __PIPROFILERTEST_H__

//  Include Files

#include <e32base.h>

class CSamplerPluginInterface;

//  Function Prototypes

GLDEF_C TInt E32Main();
LOCAL_C TInt LoadProfilerLDD();
LOCAL_C TInt LoadProfilerDLL();

#endif  // __PIPROFILERTEST_H__

