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
* Description:  Declaration of the server side main class CATStorageServer
*
*/


#ifndef ATSTORAGESERVER_H
#define ATSTORAGESERVER_H


//  INCLUDES
#include    <e32base.h>
#include    <analyzetool/atcommon.h>
#include 	"atdllinfo.h"

// CONSTANTS
// Constants for the server's version
const TUint KVersionNumberMaj = 0;
const TUint KVersionNumberMin = 1;
const TUint KVersionNumberBld = 1;

// DATA TYPES

// FORWARD DECLARATIONS
class TATProcessInfo;
class CATDynProcessInfo;
class CATStorageServerSession;

// CLASS DECLARATION

/**
*  The main class of the Storage Server.
*  When a client connects to the server, a new session object
*  is created by an object of this class. If the server does not exist when
*  connecting, a new process and an object of this class are created first.
*/
class CATStorageServer : public CPolicyServer
    {
    
    public: // Enumerations
    
        // opcodes used in message passing between client and server
        enum TStorageServerReq
            {
            EProcessStarted,
            EDllLoaded,
            EDllUnloaded,
            EMemoryAllocated,
            EMemoryFreed,
            EProcessEnded,
            EMemoryCheck,
            EGetProcesses,
            EGetDlls,
            ESetLoggingMode,
            EGetLoggingMode,
            ESubtestStart,
            ESubtestStop,
            ESubtestStart2,
            ESubtestStop2,
            EGetCurrentAllocs,
            EGetMaxAllocs,
            ECancelLogging,
            EGetUdeb,
            EGetLoggingFile,
            EProcessUdeb,
            EIsMemoryAdded,
            EOutOfBounds
            };
    
    
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor that can leave.
        * @return A new instance of this class
        */
        static CATStorageServer* NewL();
        
        /**
        * Two-phased constructor that can leave and leaves a pointer
        * on the cleanup stack.
        * @return A new instance of this class
        */
        static CATStorageServer* NewLC();
        
        /**
        * Destructor
        */
        virtual ~CATStorageServer();

    public: // New functions
        
  
        /**
        * Creates a new server object and starts the server.
        */
        static void RunServerL();
        
        /**
        * Increments the count of active sessions for this server.
        */
        void IncSessionCount();

        /**
        * Decrements the count of active sessions for this server.
        */
        void DecSessionCount();
        
        /**
        * Adds a new process into the server's arrays.
        * @param aProcessName The name of the process to be added.
        * @param aProcessId The ID of the process to be added.
        * @param aSessionObject A pointer to the current process's associated session
        *   object.
        * @param aStartTime The starting time of the process to be added. It represents
        *   time as a number of microseconds since midnight, January 1st, 0 AD nominal 
        *   Gregorian. This is the representation used by, e.g., the TTime class.
        * @return KErrNone, if the operation is successful; KErrAlreadyExists, if
        *   an attempt is being made to insert a duplicate process; otherwise one of
        *   the other system wide error codes.
        */
        TInt AddProcessL( const TDesC8& aProcessName, 
                         TUint aProcessId,
                         CATStorageServerSession* aSessionObject,
                         const TInt64& aStartTime );
        
        /**
        * Removes a process from the server's arrays.
        * @param aProcessId The ID of the process to be removed.
        * @return KErrNone, if successful; KErrNotFound, if a process with
        *   the specified process ID could not be found in the array.
        */
        TInt RemoveProcessL( TUint aProcessId );
        
        /**
        * Adds a DLL for a given process to the dynamic process info array of this class.
        * @param aProcessId The ID of the process that has loaded the DLL to be added.
        * @param aDllInfo Information of the DLL.
        * @return KErrNone, if successful; KErrNotFound, if a process with the given
        *   process ID could not be found; KErrAlreadyExists, if a DLL with the given
        *   name was already stored; otherwise one of the other system wide error codes.
        */
        TInt AddDllL( TUint aProcessId, const TATDllInfo aDllInfo );
        
        /**
        * Removes a DLL associated with the given process from the dynamic process
        * info array of this class.
        * @param aProcessId The ID of the process that has loaded the DLL to be removed.
        * @param aDllName The name of the DLL to be removed.
        * @return KErrNone, if successful; KErrNotFound, if a process with
        *   the specified process ID could not be found, or if a DLL with the specified
        *   name could not be found.
        */
        TInt RemoveDllL( TUint aProcessId, const TDesC8& aDllName );
            
        /**
        * Gives a reference to the process info array of this class.
        * @return A reference to iProcesses array
        */
        const RArray<TATProcessInfo>& ProcessInfoArray();
        
        /**
        * Gives a reference to the dynamic process info array of this class.
        * @return A reference to iProcessesDyn array
        */
        const RPointerArray<CATDynProcessInfo>& DynProcessInfoArray();
                
    public: // Functions from base classes
        
        /**
        * From CPolicyServer.
        * Creates a new server-side session object.
        * @param aVersion The version of this server
        * @param aMessage A message from the client
        * @return A pointer to a new session object.
        */
        CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const;
        
    private:

        /**
        * C++ default constructor.
        * @param aPriority A priority for this active object
        */
        CATStorageServer( TInt aPriority );

        /**
        *  Symbian 2nd phase constructor.
        */
        void ConstructL();
   
    private: // Data
        
        /** The number of active sessions */
        TInt iSessionCount;
        
        /** An array for storing TATProcessInfo objects */
        RArray<TATProcessInfo> iProcesses;
        
        /** An array for storing CATDynProcessInfo pointers */
        RPointerArray<CATDynProcessInfo> iProcessesDyn;
    };

#endif      // ATSTORAGESERVER_H
            
// End of File