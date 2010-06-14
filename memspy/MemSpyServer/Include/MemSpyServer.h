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

#ifndef MEMSPYSERVER_H
#define MEMSPYSERVER_H

// System includes
#include <e32base.h>

#include <flogger.h>

// User includes
#include <memspyengineclientinterface.h>

// Classes referenced
class CMemSpyEngine;

// Literal constants 
//_LIT( KMemSpyServer2, "MemSpyServer.exe" );

// MemSpyServer class declaration
NONSHARABLE_CLASS( CMemSpyServer ) : public CServer2
    {
    public:
        static CMemSpyServer* NewL( CMemSpyEngine& aEngine );
        static CMemSpyServer* NewLC( CMemSpyEngine& aEngine );
        ~CMemSpyServer();

    private:
        CMemSpyServer( CMemSpyEngine& aEngine );
    	void ConstructL();

    protected: // From CServer2
    	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

    private:
        CMemSpyEngine& iEngine;
        
        RFileLogger iLog;
    };

#endif
