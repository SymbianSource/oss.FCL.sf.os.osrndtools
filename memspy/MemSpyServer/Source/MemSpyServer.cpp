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
* Implementation of the new MemSpyServer
*/

//user includes
#include "MemSpyServer.h"
#include "MemSpyServerSession.h"

// System includes
#include <e32svr.h>

// User includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyenginelogger.h>
//#include <memspyengineclientinterface.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyenginehelperchunk.h>
#include <memspy/engine/memspyenginehelpercodesegment.h>
#include <memspy/engine/memspyenginehelperheap.h>
#include <memspy/engine/memspyenginehelperstack.h>
#include <memspy/engine/memspyenginehelperthread.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelperfilesystem.h>
#include <memspy/engine/memspyenginehelperram.h>
#include <memspy/engine/memspyenginehelpersysmemtracker.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerconfig.h>
#include <memspy/engine/memspyenginehelperkernelcontainers.h>

#include <memspysession.h>

// ---------------------------------------------------------
// CMemSpyServer( CMemSpyEngine& aEngine )
// ---------------------------------------------------------
//
CMemSpyServer::CMemSpyServer( CMemSpyEngine& aEngine )
:   CServer2( EPriorityNormal ), iEngine( aEngine )
    {
    }

// ---------------------------------------------------------
// ~CMemSpyServer()
// ---------------------------------------------------------
//
CMemSpyServer::~CMemSpyServer()
    {
	iLog.CloseLog();
	iLog.Close();
    }

// ---------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------
//
void CMemSpyServer::ConstructL()
    {
    StartL( KMemSpyServer2 );
	//TInt error = Start( KMemSpyServerName );
    RDebug::Printf( "[MemSpy] CMemSpyServer::ConstructL() - server started" );
    
    iLog.Connect();
    iLog.CreateLog(_L("memspy"), _L("server.txt"), EFileLoggingModeAppend);
    iLog.Write(_L("[MemSpy] CMemSpyServer::ConstructL() - server started"));
    iLog.Write(_L("[MemSpy] CMemSpyServer::ConstructL() - server name: "));
    iLog.Write(KMemSpyServer2);
    }

// ---------------------------------------------------------
// NewL( CMemSpyEngine& aEngine )
// Two phased constructor
// ---------------------------------------------------------
//
CMemSpyServer* CMemSpyServer::NewL( CMemSpyEngine& aEngine )
    {
	CMemSpyServer* self = CMemSpyServer::NewLC(aEngine);
	CleanupStack::Pop(self);
	return (self);
    }

// ---------------------------------------------------------
// NewLC( CMemSpyEngine& aEngine )
// Two phased constructor
// ---------------------------------------------------------
//
CMemSpyServer* CMemSpyServer::NewLC( CMemSpyEngine& aEngine )
    {
    CMemSpyServer* self = new(ELeave) CMemSpyServer( aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------
// NewSessionL( const TVersion& aVersion, const RMessage2& aMessage )
// Creates new client server session
// ---------------------------------------------------------
//
CSession2* CMemSpyServer::NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const
    {
	if  ( aVersion.iMajor != KMemSpyClientServerVersion )
        {
        RDebug::Printf( "[MemSpy] CMemSpyServerSession::NewSessionL() - BAD VERSION" );
        User::Leave( KErrNotSupported );
        }
    //
    CMemSpyServerSession* session = CMemSpyServerSession::NewL( iEngine, aMessage );
	return session;
    }

// END OF FILE
