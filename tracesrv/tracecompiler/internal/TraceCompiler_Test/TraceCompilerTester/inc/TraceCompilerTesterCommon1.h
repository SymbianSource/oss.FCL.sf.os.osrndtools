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
#ifndef __TRACECOMPILERTESTERCOMMON1_H__
#define __TRACECOMPILERTESTERCOMMON1_H__

// Include files
#include <klib.h> 

//  FORWARD DECLARATIONS

/**
 * TraceCompilerTester application
 */
NONSHARABLE_CLASS( DTraceCompilerTesterCommon1 ) : public DBase
    {
public:

    /**
     * Constructor
     */
    DTraceCompilerTesterCommon1();

    /**
     * Destructor
     */
    ~DTraceCompilerTesterCommon1();

    /**
     * Set values
     * 
     * @param aValue1
     * @param aValue2
     * @param aValue3
     * @param aValue4
     * @return sum of the values          
     */
    TInt32 SumValues( TInt aValue1, TInt aValue2, TUint8 aValue3, TUint8 aValue4 );

    };
#endif

// End of File
