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
#ifndef __TRACECOMPILERTESTERBASIC_H__
#define __TRACECOMPILERTESTERBASIC_H__


// Include files
#include <TraceCoreNotificationReceiver.h>
#include "BigNumbers.inl"

//  FORWARD DECLARATIONS

/**
 * TraceCompilerTester application
 */
NONSHARABLE_CLASS( DTraceCompilerTesterBasic ) : public DTraceCoreNotificationReceiver
    {
public:

    /**
     * Constructor
     */
    DTraceCompilerTesterBasic();
    
    /**
     * Destructor
     */
    ~DTraceCompilerTesterBasic();
    
    /**
     * Second-phase constructor
     *
     * @param
     * @return System-wide error code
     */   
    TInt Construct();   

public: // from DTraceCoreNotificationReceiver    
    
    /**
     * Callback function for Trace Activation
     * 
     * @param aComponentId
     * @param aGroupId
     * @return None          
     */
    void TraceActivated( TUint32 aComponentId, TUint16 aGroupId  );
    
    /**
     * Callback function for Trace Deactivation
     * 
     * @param aComponentId
     * @param aGroupId
     * @return None     
     */
    void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  );

private:

    };


#endif

// End of File
