/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* MemSpyServer headder file to declare MemSpyServer class
*
*/

#ifndef MEMSPYSERVERSESSION_H
#define MEMSPYSERVERSESSION_H

// System includes
#include <e32base.h>
#include <BADESCA.H>

// User includes
#include <memspyengineclientinterface.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>

    // Classes referenced
class CMemSpyEngine;


NONSHARABLE_CLASS( CMemSpyServerSession ) : public CSession2
	{
public:
	static CMemSpyServerSession* NewL( CMemSpyEngine& aEngine, const RMessage2& aMessage );
	~CMemSpyServerSession();

private:
	CMemSpyServerSession( CMemSpyEngine& aEngine );
	void ConstructL( const RMessage2& aMessage );

private: // From CSession2
	void ServiceL( const RMessage2& aMessage );

private: // Internal methods
    void DoServiceL( const RMessage2& aMessage );
    static TInt ValidateFunction( TInt aFunction, TBool aIncludesThreadId, TBool aIncludesThreadName );
    void HandleThreadSpecificOpL( TInt aFunction, const TThreadId& aThreadId );
    void HandleThreadSpecificOpL( TInt aFunction, const TDesC& aThreadName );
    void HandleThreadAgnosticOpL( TInt aFunction, const RMessage2& aMessage );
    
private:
    CMemSpyEngine& iEngine;
    HBufC* iClientThreadName;
    TUint32 iClientThreadId;
    };

#endif
