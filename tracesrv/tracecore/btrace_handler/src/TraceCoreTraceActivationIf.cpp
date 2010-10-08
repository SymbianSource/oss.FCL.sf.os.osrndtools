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
// Implementation of Trace activation interface
// 

#include "TraceCore.h"
#include "TraceCoreTraceActivationIf.h"
#include "BTraceOstCategoryBitmap.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreTraceActivationIfTraces.h"
#endif




/**
 * Constructor
 */
DTraceActivationIf::DTraceActivationIf()
    {
    }


/**
 * Destructor
 */
DTraceActivationIf::~DTraceActivationIf()
    {
    }
    
    
/**
 * Activate trace group
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 */
EXPORT_C void DTraceActivationIf::ActivateTrace( TUint32 aComponentId, TUint32 aGroupId )
    {    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        MTraceCoreActivation* activation = traceCore->GetActivation( aComponentId );
        if ( activation != NULL )
            {
            activation->ActivateTrace( aComponentId, aGroupId );
            }
        }
    }
    
    
/**
 * Deactivate trace group
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 */    
EXPORT_C void DTraceActivationIf::DeactivateTrace( TUint32 aComponentId, TUint32 aGroupId )
    {
    OstTraceExt2( TRACE_BORDER, DTRACEACTIVATIONIF_DEACTIVATETRACE_ENTRY,"> DTraceActivationIf::DeactivateTrace. CID:0x%x GID:0x%x", aComponentId, aGroupId );
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        MTraceCoreActivation* activation = traceCore->GetActivation( aComponentId );
        if ( activation != NULL )
            {
            activation->DeactivateTrace( aComponentId, aGroupId );
            }
        }

    }

/**
 * Check if a Trace is activated for the supplied component and group id
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 * @return returns ETrue if trace is activated otherwise EFalse
 */    
EXPORT_C TBool DTraceActivationIf::IsTraceActivated(TUint32 aComponentId, TUint32 aGroupId)
    {
    TBool ret = EFalse;
    
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if (traceCore != NULL)
        {
        MTraceCoreActivation* activation = traceCore->GetActivation(aComponentId);
        if (activation != NULL)
            {
            if(activation->IsComponentSupported(aComponentId))
                {
                RArray<TUint> groups = activation->GetActivatedGroups(aComponentId);
                
                const TUint8 KGroupId = (TUint8)aGroupId;
                
                for(TInt i=0;i<groups.Count();i++)
                    {
                    if( (TUint8)groups[i] == KGroupId)
                        {
                        ret = ETrue;
                        break;
                        }
                    }
                }
            } 
        else
            {
            OstTrace0(TRACE_IMPORTANT,DTRACEACTIVATIONIF_DEACTIVATETRACE_NO_ACTIVATION,"DTraceActivationIf::IsTraceActivated - No activation interface");
            }
        }
    else
        {
        OstTrace0(TRACE_IMPORTANT,DTRACEACTIVATIONIF_DEACTIVATETRACE_NULL_TRACECORE,"DTraceActivationIf::IsTraceActivated - NULL TraceCore");
        }
   
    return ret;    
    }


/**
 * Reactivate all currently activated traces
 *
 * @return KErrNone if refresh successful
 */
EXPORT_C TInt DTraceActivationIf::RefreshActivations()
    {    
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        return traceCore->RefreshActivations();
        }
    return KErrNotReady;
    }
    
