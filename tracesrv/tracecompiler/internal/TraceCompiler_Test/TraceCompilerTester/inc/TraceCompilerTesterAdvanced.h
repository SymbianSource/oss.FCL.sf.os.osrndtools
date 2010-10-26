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
#ifndef __TRACECOMPILERTESTERADVANCED_H__
#define __TRACECOMPILERTESTERADVANCED_H__


// Include files
#include <TraceCoreNotificationReceiver.h>
#include "BigNumbers.inl"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCompilerTesterAdvancedTraces.h"
#endif

//  FORWARD DECLARATIONS

/**
 * TraceCompilerTester application
 */
NONSHARABLE_CLASS( DTraceCompilerTesterAdvanced ) : public DTraceCoreNotificationReceiver
    {
public:
     /**
     * Constructor
     */
    DTraceCompilerTesterAdvanced();
    
    /**
     * Destructor
     */
    ~DTraceCompilerTesterAdvanced();

    /**
     * Hello moon
     */
    inline TInt HelloMoon() {
        OstTrace0(TRACE_FLOW, HELLO_MOON, "Hello moon!");
        return 1;
    };    
    
    /**
     * Hello world
     */
    inline void HelloWorld();   
    
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

#include "TraceCompilerTesterAdvanced.inl"

// End of File
