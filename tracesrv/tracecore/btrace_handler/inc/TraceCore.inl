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

#ifndef __TRACECORE_INL__
#define __TRACECORE_INL__


/**
 * Gets the trace core router
 */
inline DTraceCoreRouter& DTraceCore::GetRouter()
    {
    __ASSERT_DEBUG( iRouter,
        Kern::Fault( "DTraceCore::GetRouter(): iRouter NULL", KErrGeneral ) );
    return *iRouter;
    }

/**
 * Gets the trace core send-receive interface
 */
inline DTraceCoreSendReceive& DTraceCore::GetSendReceive()
    {
    __ASSERT_DEBUG( iSendReceive,
        Kern::Fault( "DTraceCore::GetSendReceive(): iSendReceive NULL", KErrGeneral ) );
    return *iSendReceive;
    }
    
/**
 * Gets the trace core notifier
 */
inline DTraceCoreNotifier& DTraceCore::GetNotifier()
    {
    __ASSERT_DEBUG( iNotifier,
        Kern::Fault( "DTraceCore::GetNotifier(): iNotifier NULL", KErrGeneral ) );
    return *iNotifier;
    }

#endif

// End of File
