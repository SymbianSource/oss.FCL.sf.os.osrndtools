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

#ifndef _TRACECOREOSTCONNECTIONIF_H_
#define _TRACECOREOSTCONNECTIONIF_H_

#include <kernel/kernel.h>


/**

Used in TraceCoreOstLdd for sending message (as callback) to registered component

*/
class MTraceCoreOstConnectionIf
    {
    public:
    
    /**
     * TxMessage
     *  
     * Message from TraceCore to OST connection
     * @param aDes Message to be sent
     */
    virtual void TxMessage( const TDesC8& aDes ) = 0;
    
    };

#endif /*_TRACECOREOSTCONNECTIONIF_H_*/

