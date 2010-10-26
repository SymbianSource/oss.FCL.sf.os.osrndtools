/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef __TRACECOMPILERTESTERCOMMON2_H__
#define __TRACECOMPILERTESTERCOMMON2_H__

// Include files
#include <klib.h> 


//  FORWARD DECLARATIONS

/**
 * TraceCompilerTester application
 */
NONSHARABLE_CLASS( DTraceCompilerTesterCommon2 ) : public DBase
    {
public:

    /**
     * Constructor
     */
    DTraceCompilerTesterCommon2();

    /**
     * Destructor
     */
    ~DTraceCompilerTesterCommon2();

    /**
     * Print Kekkonen
     */
    void Kekkonen();
    };
#endif

// End of File
