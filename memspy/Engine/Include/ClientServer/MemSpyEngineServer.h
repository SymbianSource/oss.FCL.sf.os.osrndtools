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
*
*/

#ifndef MEMSPYENGINESERVER_H
#define MEMSPYENGINESERVER_H

// System includes
#include <e32base.h>

// User includes
#include <memspyengineclientinterface.h>

// Classes referenced
class CMemSpyEngine;



NONSHARABLE_CLASS( CMemSpyEngineServer ) : public CServer2
    {
public:
    static CMemSpyEngineServer* NewL( CMemSpyEngine& aEngine );
    ~CMemSpyEngineServer();

private:
    CMemSpyEngineServer( CMemSpyEngine& aEngine );
	void ConstructL();

protected: // From CServer2
	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

private:
    CMemSpyEngine& iEngine;
    };



NONSHARABLE_CLASS( CMemSpyEngineSession ) : public CSession2
	{
public:
	static CMemSpyEngineSession* NewL( CMemSpyEngine& aEngine, const RMessage2& aMessage );
	~CMemSpyEngineSession();

private:
	CMemSpyEngineSession( CMemSpyEngine& aEngine );
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
