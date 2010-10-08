// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// Trace Core
//

#ifndef DTRACECORETRACEACTIVATIONIF_H_
#define DTRACECORETRACEACTIVATIONIF_H_

#include <kernel/kernel.h>

/**
 * Interface to activate and deactivate trace groups
 */
class DTraceActivationIf : public DBase
    {
public:

    /**
     * Activate trace group
     *
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */
	IMPORT_C static void ActivateTrace( TUint32 aComponentId, TUint32 aGroupId );
	
    /**
     * Deactivate trace group
     *
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */    
	IMPORT_C static void DeactivateTrace( TUint32 aComponentId, TUint32 aGroupId );
	
	/**
	 * Check if a Trace is activated for the supplied component and group id
	 *
	 * @param aComponentId The component ID
	 * @param aGroupId The group ID
	 * @return returns ETrue if trace is activated otherwise EFalse
	 */    
	IMPORT_C static TBool IsTraceActivated(TUint32 aComponentId, TUint32 aGroupId);

    /**
     * Reactivate all currently activated traces
     *
     * @return KErrNone if refresh successful
     */
    IMPORT_C static TInt RefreshActivations();

private:

    /**
     * Constructor
     */
    DTraceActivationIf();
    
    /**
     * Destructor
     */
    ~DTraceActivationIf();    
    };

#endif /*DTRACECORETRACEACTIVATIONIF_H_*/
