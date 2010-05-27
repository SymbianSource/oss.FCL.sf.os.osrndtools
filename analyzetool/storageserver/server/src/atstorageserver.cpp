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
* Description:  Definitions for the class CATStorageServer. Includes also the
*                entry point of the atoolstorageserver.exe executable.
*
*/




// INCLUDE FILES

#include    "atstorageserver.h"
#include    "atstorageserversession.h"
#include    "atstorageservercommon.h"
#include    "atlog.h"
#include    "atdynprocessinfo.h"


// CONSTANTS


// The amount of different ranges of requests specified for this policy server.
// In a policy server, different actions can be determined for different ranges
// of requests.
const TUint8 KAmountOfRanges = 2;


const TInt ranges[KAmountOfRanges] =
    {
    CATStorageServer::EProcessStarted,
    CATStorageServer::EOutOfBounds
    };


const TUint8 actionForRange[ KAmountOfRanges ] = 
    { 0, CPolicyServer::ENotSupported };


const CPolicyServer::TPolicyElement elements[] =
    {
    
    { _INIT_SECURITY_POLICY_C1( ECapability_None ),
          CPolicyServer::EFailClient }
    
    };


const CPolicyServer::TPolicy policy =
    {
    CPolicyServer::EAlwaysPass, // On connect
    KAmountOfRanges, // Range count
    ranges,
    actionForRange,
    elements,
    };



// ENTRY POINT

TInt E32Main()
    { 
    LOGSTR1( "STSE TInt E32Main() in ATStorageServer.cpp" );
    
    __UHEAP_MARK;
    
    // Create a cleanup stack 
    CTrapCleanup* cleanup = CTrapCleanup::New();
    
    TInt errorCode( KErrNoMemory );
    
    // If there was enough memory to create a cleanup stack, 
    // create and start the server.
    if( cleanup )
        {
        // If RunServerL() doesn't leave, errorCode will be set to KErrNone
        TRAP( errorCode, CATStorageServer::RunServerL() );
        delete cleanup;
        }

    // Signal the client that server creation failed, if a leave occured during
    // the call to 'RunServerL()' function
    if( errorCode != KErrNone )
        {
        RProcess::Rendezvous( errorCode );
        }
    
    __UHEAP_MARKEND;
       
    return errorCode;
    
    }


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CATStorageServer::CATStorageServer
// C++ default constructor can NOT contain any code that
// might leave.
// -----------------------------------------------------------------------------
//
CATStorageServer::CATStorageServer( TInt aPriority ) :
    CPolicyServer( aPriority, policy, ESharableSessions ),
    iSessionCount( 0 )
    {
    LOGSTR1( "STSE CATStorageServer::CATStorageServer()" );
    }

// -----------------------------------------------------------------------------
// CATStorageServer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CATStorageServer::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CATStorageServer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CATStorageServer* CATStorageServer::NewL()
    {
    CATStorageServer* self = NewLC();
    CleanupStack::Pop( self );
    
    return self;
    }

// -----------------------------------------------------------------------------
// CATStorageServer::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CATStorageServer* CATStorageServer::NewLC()
    {  
    CATStorageServer* self = new ( ELeave ) CATStorageServer( EPriorityStandard );
    CleanupStack::PushL( self );
    self->ConstructL();
    self->StartL( KStorageServerName );
    return self;
    }

// Destructor
CATStorageServer::~CATStorageServer()
    {
    LOGSTR1( "STSE CATStorageServer::~CATStorageServer()" );
    
    iProcesses.Close();
    iProcessesDyn.ResetAndDestroy();
    iProcessesDyn.Close();
    }

// -----------------------------------------------------------------------------
// CATStorageServer::NewSessionL
// Create a new server session.
// -----------------------------------------------------------------------------
// 
CSession2* CATStorageServer::NewSessionL(
    const TVersion &aVersion,
    const RMessage2& /*aMessage*/ ) const
    {
    LOGSTR1( "STSE CSession2* CATStorageServer::NewSessionL()" );
    
    // The server's version
    TVersion version( KVersionNumberMaj, KVersionNumberMin,
        KVersionNumberBld );

    // Check this is a high enough version of the server
    if ( !User::QueryVersionSupported(version, aVersion) )
        {
        User::Leave( KErrNotSupported );
        }
    
    // Construct and return a new session object
    return CATStorageServerSession::NewL( const_cast<CATStorageServer&>(*this) );
    }

// -----------------------------------------------------------------------------
// CATStorageServer::RunServerL
// Create active scheduler and a server object
// -----------------------------------------------------------------------------
//    
void CATStorageServer::RunServerL()
    {
    LOGSTR1( "STSE void CATStorageServer::RunServerL()" );
    
    // Create and install an active scheduler
    CActiveScheduler* activeSched = new (ELeave) CActiveScheduler;
    CleanupStack::PushL( activeSched );
    CActiveScheduler::Install( activeSched );

    // Create server
    CATStorageServer::NewLC();
     
    // The initialization performed alright, signal the client
    RProcess::Rendezvous( KErrNone );

    // Start the active scheduler
    CActiveScheduler::Start();

    // Remove the server and active scheduler from the cleanup stack
    CleanupStack::PopAndDestroy( 2, activeSched );
    }
  
// -----------------------------------------------------------------------------
// CATStorageServer::IncSessionCount
// Increments the count of active sessions for this server.
// -----------------------------------------------------------------------------
// 
void CATStorageServer::IncSessionCount()
    {
    LOGSTR1( "STSE void CATStorageServer::IncSessionCount()" );
    
    iSessionCount++;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServer::DecSessionCount
// Decrements the count of active sessions for this server.
// -----------------------------------------------------------------------------
// 
void CATStorageServer::DecSessionCount()
    {
    LOGSTR1( "STSE void CATStorageServer::DecSessionCount()" );
       
    iSessionCount--;
    
    // Cancels all outstanding messages and stops the active scheduler,
    // if there are no other sessions open at the moment
    if ( iSessionCount <= 0 )
        {
        this->Cancel();
        CActiveScheduler::Stop();
        }
    }
  
// -----------------------------------------------------------------------------
// CATStorageServer::AddProcessL
// -----------------------------------------------------------------------------
// 
TInt CATStorageServer::AddProcessL( const TDesC8& aProcessName,
                                   TUint aProcessId,
                                   CATStorageServerSession* aSessionObject,
                                   const TInt64& aStartTime )
    {
    LOGSTR1( "STSE TInt CATStorageServer::AddProcessL()" );

    TInt error( KErrNone );
    TATProcessInfo processInfo;
    
    processInfo.iProcessId = aProcessId;
    processInfo.iProcessName.Copy( aProcessName );
    processInfo.iStartTime = aStartTime;
    
    // Insert the static process info into the iProcesses array
    error = iProcesses.InsertInUnsignedKeyOrder( processInfo );
    
    // Return if an error occured
    if ( error != KErrNone)
        {
        return error;
        }
    
    // Construct a CATDynProcessInfo object with the given process ID and logging mode
    CATDynProcessInfo* dynProcessInfo = 
                            new (ELeave) CATDynProcessInfo( aProcessId,
                                                            aSessionObject );
    
    // Insert the dynamic process info into the iProcessesDyn array
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    error = iProcessesDyn.InsertInOrder( dynProcessInfo, order );
    
    return error;
    }
    
// -----------------------------------------------------------------------------
// CATStorageServer::RemoveProcessL
// -----------------------------------------------------------------------------
// 
TInt CATStorageServer::RemoveProcessL( TUint aProcessId )
    {
    LOGSTR1( "STSE TInt CATStorageServer::RemoveProcessL()" );
    
    TATProcessInfo processInfo;
    processInfo.iProcessId = aProcessId;
    
    TInt index = iProcesses.FindInUnsignedKeyOrder( processInfo );

    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index; 
        }
     
    // Remove the TATProcessInfo object at "index" from the array
    iProcesses.Remove( index );
     
    // Now, start removing the associated dynamic process info object

    // Construct a CATDynProcessInfo object with the given process ID
    CATDynProcessInfo* dynProcessInfo = 
                            new (ELeave) CATDynProcessInfo( aProcessId );
    
    // Find the index of a CATDynProcessInfo object with the given ID in the array
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    index = iProcessesDyn.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
     
    // Return, if a process with the requested process ID was not found
    if ( index == KErrNotFound )
        {
        return index;
        }
     
    // Otherwise, delete the object and remove the pointer at "index"
    delete iProcessesDyn[index];
    iProcessesDyn.Remove( index ); 
     
    return KErrNone;   
    }

// -----------------------------------------------------------------------------
// CATStorageServer::AddDllL
// -----------------------------------------------------------------------------
// 
TInt CATStorageServer::AddDllL( TUint aProcessId, 
    const TATDllInfo aDllInfo )
    {
    LOGSTR1( "STSE TInt CATStorageServer::AddDllL()" );
 
    // Construct a CATDynProcessInfo object with the given process ID
    CATDynProcessInfo* dynProcessInfo = 
		new (ELeave) CATDynProcessInfo( aProcessId );
  
    // Find out if a process with this ID can be found in the dynamic process array
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = iProcessesDyn.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
     
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
        
    // If we are here, the wanted process was found at index => append a DLL for it
    dynProcessInfo = iProcessesDyn[index];
    
    // Get the DLL array from this dynamic process info object
    RArray<TATDllInfo>& dllArray = dynProcessInfo->iDlls;    
    
    // Let's see if the DLL to be added already exists
    TIdentityRelation<TATDllInfo> matcher( TATDllInfo::Match );
    index = dllArray.Find( aDllInfo, matcher );
    
    // If so, return KErrAlreadyExists
    if ( index != KErrNotFound )
        {
        return KErrAlreadyExists;
        }

    // Otherwise append this DLL to the array and return 
    return dynProcessInfo->iDlls.Append( aDllInfo );
    }

// -----------------------------------------------------------------------------
// CATStorageServer::RemoveDllL
// -----------------------------------------------------------------------------
// 
TInt CATStorageServer::RemoveDllL( TUint aProcessId,
                                  const TDesC8& aDllName )
    {
    LOGSTR1( "STSE TInt CATStorageServer::RemoveDllL()" );

    // Construct a CATDynProcessInfo object with the given process ID
    CATDynProcessInfo* dynProcessInfo = 
		new (ELeave) CATDynProcessInfo( aProcessId );
  
    // Find out if a process with this ID can be found in the dynamic process array
    TLinearOrder<CATDynProcessInfo> order( CATDynProcessInfo::Compare );
    TInt index = iProcessesDyn.FindInOrder( dynProcessInfo, order );
    delete dynProcessInfo;
    dynProcessInfo = NULL;
     
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
        
    // If we are here, the wanted process was found at index
    dynProcessInfo = iProcessesDyn[index];

    // Get the DLL array from this dynamic process info object
    RArray<TATDllInfo>& dllArray = dynProcessInfo->iDlls;
    
    // Try to find wanted DLL
    TIdentityRelation<TATDllInfo> matcher( TATDllInfo::Match );    
    index = dllArray.Find( TATDllInfo( 0, 0, 0, aDllName  ), matcher );
    
    // Return, if a process with the requested process ID was not found 
    if ( index == KErrNotFound )
        {
        return index;
        }
        
    // Otherwise remove the found DLL at "index"
    dllArray.Remove( index );    
        
    return KErrNone;  
    }

// -----------------------------------------------------------------------------
// CATStorageServer::ProcessInfoArray
// -----------------------------------------------------------------------------
// 
const RArray<TATProcessInfo>& CATStorageServer::ProcessInfoArray()
    {
    LOGSTR1( "STSE RArray<TATProcessInfo>& CATStorageServer::ProcessInfoArray()" );
     
    return iProcesses;   
    }
    
// -----------------------------------------------------------------------------
// CATStorageServer::DynProcessInfoArray
// -----------------------------------------------------------------------------
// 
const RPointerArray<CATDynProcessInfo>& CATStorageServer::DynProcessInfoArray()
    {
    LOGSTR1( "STSE RPointerArray<CATDynProcessInfo>& CATStorageServer::DynProcessInfoArray()" );
     
    return iProcessesDyn;   
    }    
    
//  End of File
