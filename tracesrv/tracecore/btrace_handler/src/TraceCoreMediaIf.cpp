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

#include "TraceCoreMediaIf.h"


/**
 * Constructor
 * 
 * @param aFlags the media flags
 */
DTraceCoreMediaIf::DTraceCoreMediaIf( TUint32 aFlags )
: iCallback( NULL )
, iFlags( aFlags )
    {
    }


/**
 * Sends a message
 *
 * @param aMsg The message to be sent
 */
TInt DTraceCoreMediaIf::Send( TTraceMessage& /*aMsg*/ )
    {
    return KErrNotSupported;
    }


/**
 * Sends a trace
 *
 * @param aMsg The trace to be sent
 */
TInt DTraceCoreMediaIf::SendTrace( const TDesC8& /*aMsg*/ )
    {
    return KErrNotSupported;
    }


// End of File
