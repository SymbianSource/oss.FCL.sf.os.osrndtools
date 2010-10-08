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
// TraceCore configuration message subscriber
//

#include "TraceCore.h"
#include "TraceCoreConfiguration.h"


DTraceCoreConfiguration::DTraceCoreConfiguration()
    {
    }

DTraceCoreConfiguration::~DTraceCoreConfiguration()
    {
    }

/**
 * Init configuration
 */         
TInt DTraceCoreConfiguration::Init()
    {
    TInt err(KErrNone);
    return err;
    }

/**
 * GetSet
 * 
 * @param aFunc Function choosed (@see TTraceCoreConfiguration)
 * @return Depending of aFunc paramter (@see TTraceCoreConfiguration)
 */
EXPORT_C TUint32 DTraceCoreConfiguration::GetSet(TTraceCoreConfiguration aFunc, TUint32 aParam)
    {
    TUint32 ret(0);
    switch(aFunc)
        {
        case EGetCurrentWriterType:
            {
            ret = (TUint32) DTraceCore::GetInstance()->GetCurrentWriterType();
            break;
            }
        case ESetWriter:
            {
            DTraceCore::GetInstance()->SwitchToWriter((TWriterType) aParam);
            break;
            }
        case ELastFunction: // Go through
        default:
            __ASSERT_DEBUG( EFalse, 
            Kern::Fault( "DTraceCoreConfiguration::GetSet: Default case!", KErrArgument ) );
            ret = (TUint32) KErrNotSupported;
        }
    
    return ret;
    }
