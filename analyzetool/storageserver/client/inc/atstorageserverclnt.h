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
* Description:  Declaration of the client side handle class RATStorageServer
*
*/



#ifndef ATSTORAGESERVERCLNT_H
#define ATSTORAGESERVERCLNT_H


// INCLUDES
#include    <e32base.h>
#include    <analyzetool/atcommon.h>


// CONSTANTS
// Constants for the lowest version of the server with which the
// client is compatible
const TUint KLowestVersionNumberMaj = 0;
const TUint KLowestVersionNumberMin = 1;
const TUint KLowestVersionNumberBld = 1;


// FORWARD DECLARATIONS
class TATProcessInfo;


// CLASS DECLARATION

/**
*  The main class of the Storage Server client. Objects of this class work as handles
*  to the Storage Server's server side. Use Connect() to connect a client to the server,
*  and Close() to end an opened session.
*/
class RATStorageServer: public RSessionBase
    {
    public:  // Constructor and destructor
       
        /**
        * C++ default constructor.
        */
        IMPORT_C RATStorageServer();
   
    public: // New functions
        
        /**
        * Creates a new process for the server, if that doesn't already exist, and
        * connects a client to the server by creating a new session with it, and
        * initializes the client for use. To end the server session, use Close().
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt Connect();
    
        /**
        * Closes a handle to a server session. This has also the effect of destroying
        * the associated server side session object. Also, the server itself and the
        * process where it is running are destroyed, if there are no other open sessions
        * left.
        */
        IMPORT_C void Close();
    
        /**
        * This method is called to initialize the server for logging and it is to be
        * called as the first method after connecting to the server. The method informs
        * the server that a new process has been started. The method cannot be called
        * again without a call to the LogProcessEnded() first. Otherwise a STSEClient: 3
        * panic is raised. A KERN-EXEC: 0 panic is raised if the client is not connected
        * to the server.
        * @param aFileName The name of the logging file used when the system is logging
        *   to a file (the logging mode is EATLogToFile). The length of aFileName must not be
        *   greater than KMaxFileName, otherwise the method raises a STSEClient: 2 panic.
        * @param aProcessName The name of the new process started. The length of this
        *   descriptor must not be greater than KMaxProcessName, otherwise the method
        *   raises a STSEClient: 2 panic.
        * @param aProcessId The ID of the process started.
        * @param aLogOption An option telling the logging mode.
        * @param aIsDebug Determines whether a binary is UDEB or UREL
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogProcessStarted( const TDesC& aFileName,
                                         const TDesC8& aProcessName,
                                         TUint aProcessId,
                                         TUint32 aLogOption, TUint32 aIsDebug );
        
        /**
        * Asks the server to log that a new DLL has been loaded. Note, the method
        * LogProcessStarted() has to be previously called. Otherwise a STSEClient: 3
        * panic is raised. A KERN-EXEC: 0 panic is raised if the client is not connected
        * to the server.
        * @param aDllName The name of the new DLL loaded. The length of this descriptor
        *   must not be greater than KMaxLibraryName, otherwise the method raises a
        *   STSEClient: 2 panic.
        * @param aStartAddress The start address of the DLL loaded.
        * @param aEndAddress The end address of the DLL loaded.
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogDllLoaded( const TDesC8& aDllName, TUint32 aStartAddress,
                                              TUint32 aEndAddress );
        
        /**
        * Asks the server to log that a new DLL has been unloaded. Note, the method
        * LogProcessStarted() has to be previously called. Otherwise a STSEClient: 3
        * panic is raised. A KERN-EXEC: 0 panic is raised if the client is not connected
        * to the server. If the method is called with illegal parameter values, a
        * STSEClient: 2 panic is raised.
        * @param aDllName The name of the DLL to be unloaded. The length of this
        *   descriptor must not be greater than KMaxLibraryName, otherwise the method
        *   raises a STSEClient: 2 panic.
        * @param aStartAddress The start address of the DLL to be unloaded.
        * @param aEndAddress The end address of the DLL to be unloaded.
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogDllUnloaded( const TDesC8& aDllName, TUint32 aStartAddress,
                                               TUint32 aEndAddress );
        
        /**
        * Informs the server that a memory allocation has occured at the specified
        * address. Sends also the current call stack to the server. Note, the method
        * LogProcessStarted() has to be previously called. Otherwise a STSEClient: 3 
        * panic is raised. A KERN-EXEC: 0 panic is raised if the client is not connected
        * to the server.
        * @param aMemAddress The memory location where memory has been allocated.
        * @param aCallstack An array including the current call stack.
        * @param aSize The size of the newly allocated memory chunk.
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogMemoryAllocated( TUint32 aMemAddress,
                                          TFixedArray<TUint32, KATMaxCallstackLength>& aCallstack,
                                          TInt aSize );
                
        /**
        * Informs the server that memory was released at the specified address. Note,
        * the method LogProcessStarted() has to be previously called. Otherwise a 
        * STSEClient: 3 panic is raised. A KERN-EXEC: 0 panic is raised if the client is
        * not connected to the server.
        * @param aMemAddress The memory location where memory has been deallocated.
        * @param aFreeCallstack An array including the current call stack.
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogMemoryFreed( TUint32 aMemAddress, 
                                      TFixedArray<TUint32, KATMaxFreeCallstackLength>& aFreeCallstack );
        
        /**
        * Tells the server that the process under test has been ended. Prints information
        * related to process ending, such as possibly occured memory leaks (when file
        * logging mode used), and possibly occured handle leaks. Note, the method 
        * LogProcessStarted() has to be previously called. Otherwise a STSEClient:
        * 3 panic is raised. A call to this method also closes the file opened for
        * the current process. So, no further logging is possible without a new call
        * to the method LogProcessStarted(). A KERN-EXEC: 0 panic is raised if the client
        * is not connected to the server.
        * @param aProcessId The ID number of the process ended.
        * @param aHandleLeaks Amount of handle leaks.
        * @return KErrNone, if successful; otherwise one of the other
        *   system-wide error codes.
        */
        IMPORT_C TInt LogProcessEnded( TUint aProcessId, 
                                       TUint aHandleLeaks );
           
        /**
        * Gives the lowest version number of the server needed for this client.
        * @return The lowest version of the server needed for this client.
        */
        IMPORT_C TVersion Version() const;  

        /**
        * Checks if the current process has allocated, but not freed, the given memory
        * address. Note, the method LogProcessStarted() has to be previously called.
        * Otherwise a STSEClient: 3 panic is raised. A KERN-EXEC: 0 panic is raised if
        * the client is not connected to the server.
        * @param aMemAddress The memory location to check.
        * @return When the system is logging into a file: the index of a found 
        *   memory address or KErrNotFound, if a matching memory address cannot
        *   be found. When the system is not logging into a file: always KErrNone.
        */
        IMPORT_C TInt CheckMemoryAddress( TUint32 aMemAddress ) const;
        
        /**
        * Gets all the processes with currently open logging sessions.
        * @param aProcesses A reference to a process array to be filled by the server.
        * @return KErrNone, if successful; otherwise one of the other
        *   system wide error codes.
        */
        IMPORT_C TInt GetProcessesL( RArray<TATProcessInfo>& aProcesses );
        
        /**
        * Gets all the DLLs loaded by the given process.
        * @param aProcessId The ID of the process whose DLLs will be fetched.
        * @param aDlls An array that after this function call includes names of
        *   the given process's dlls
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetLoadedDllsL( TUint aProcessId,
                                      RArray< TBuf8<KMaxLibraryName> >& aDlls );
                
        /**
        * Gets the logging mode of the given process.
        * @param aProcessId The ID of the process whose logging mode is being asked.
        * @param aLoggingMode An enumeration telling the logging mode after a call
        *   to this function.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetLoggingModeL( TUint aProcessId, TATLogOption& aLoggingMode );
        
        /**
        * Starts a sub test for the given process.
        * @param aProcessId The ID of the process for which to start a sub test.
        * @param aSubtestId The name identifying this particular sub test. The length
        *   of this descriptor must not be greater than KATMaxSubtestIdLength, or
        *   otherwise the method raises a STSEClient: 2 panic.
        * @param aHandleCount The current handle count of a particular process.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; KErrNotSupported, if the requested process does not have a logging
        *   session ongoing or its logging mode is not EATLogToTrace; Otherwise one of the
        *   other system wide error codes.
        */
        IMPORT_C TInt StartSubTest( TUint aProcessId, const TDesC8& aSubtestId, TInt aHandleCount = 0 );

        /**
        * Stops a sub test for the given process.
        * @param aProcessId The ID of the process for which to stop a sub test.
        * @param aSubtestId The name identifying this particular sub test. The length
        *   of this descriptor must not be greater than KATMaxSubtestIdLength, or
        *   otherwise the method raises a STSEClient: 2 panic.
        * @param aHandleCount The current handle count of a particular process.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; KErrNotSupported, if the requested process does not have a logging
        *   session ongoing or its logging mode is not EATLogToTrace; Otherwise one of the
        *   other system wide error codes.
        */
        IMPORT_C TInt StopSubTest( TUint aProcessId, const TDesC8& aSubtestId, TInt aHandleCount = 0 );
        
        /**
        * Gets the number and total size of the memory chunks currently allocated by the
        * requested process. 
        * @param aProcessId The ID of the process whose information is requested.
        * @param aNumber On return contains the number of memory chunks currenlty
        *   allocated by the requested process.
        * @param aSize On return contains the amount of memory currently allocated
        *   by the requested process.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetCurrentAllocsL( TUint aProcessId, TUint32& aNumber, TUint32& aSize );
        
        /**
        * Gets the maximum number and total size of the memory chunks allocated by the
        * requested process. 
        * @param aProcessId The ID of the process whose information is requested.
        * @param aNumber On return contains the maximum number of memory chunks
        *   allocated by the requested process during the test run.
        * @param aSize On return contains the maximum amount of memory allocated
        *   by the requested process during the test run.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetMaxAllocsL( TUint aProcessId, TUint32& aNumber, TUint32& aSize );
        
        /**
        * Starts a sub test for the calling process.
        * @param aSubtestId The name identifying this particular sub test. The length
        *   of this descriptor must not be greater than KATMaxSubtestIdLength, or
        *   otherwise the method raises a STSEClient: 2 panic.
        * @return KErrNone, if successful; KErrNotSupported, if the calling process
        *   does not have a logging session ongoing or its logging mode is not
        *   EATLogToTrace; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt StartSubTest( const TDesC8& aSubtestId );

        /**
        * Stops a sub test for the calling process
        * @param aSubtestId The name identifying this particular sub test. The length
        *   of this descriptor must not be greater than KATMaxSubtestIdLength, or
        *   otherwise the method raises a STSEClient: 2 panic.
        * @return KErrNone, if successful; KErrNotSupported, if the calling process
        *   does not have a logging session ongoing or its logging mode is not
        *   EATLogToTrace; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt StopSubTest( const TDesC8& aSubtestId );
        
        /**
        * Cancels logging for the requested process. After logging of a given process
        * has been cancelled, the session associated with that process will not be
        * usable anymore. If a process wants to start logging again, it needs to close
        * the handle and open it again in order to create a new session. Until then, most
        * of the client's methods will return KErrCancel.
        * @param aProcessId The process ID of the process whose logging is requested to
        *   be cancelled.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt CancelLogging( TUint aProcessId );

        /**
        * Gets the number and total size of the memory chunks currently allocated by the
        * requested process. 
        * @param aProcessId The ID of the process whose information is requested.
        * @param aIsUdeb On return contains an value greater than KErrNone if
        *   mode is UDEB. Else the mode is UREL
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetUdebL( TUint aProcessId, TUint32& aIsUdeb );

        /**
        * Gets the number and total size of the memory chunks currently allocated by the
        * requested process. 
        * @param aProcessId The ID of the process whose information is requested.
        * @param aFileName On return contains logging filename for the process.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was not
        *   found; Otherwise one of the other system wide error codes.
        */
        IMPORT_C TInt GetLoggingFileL( TUint aProcessId, TDes8& aFileName );
     
        /**
		* Check a memory allocation (memory address) from an internal array.
		* @param aMemAddress The memory address to be check
		* @return KErrNone, if memory address found in array; 
		* 	KErrNotFound, if the requested memory address was not found.
		*/
        IMPORT_C TInt IsMemoryAdded( TUint32 aMemAddress );
               
    protected: // New functions
    
        /**
        * Creates a new process for the server, if it does not already exist.
        * @return KErrNone, if successful; otherwise one of the other
        * system-wide error codes. 
        */
        TInt CreateProcess();
        
    private: // Internal functions
    
        /**
        * Creates two buffers: one for storing call stack memory addresses and one for
        * handle leaks. Internally used by this class. May leave, if there is 
        * insufficient memory.
        */
        void ConstructBuffersL();
        
    private: // Data
    
        /** A pointer to a buffer of call stack's memory addresses */
        CBufFlat* iStackBuf;
    };

#endif      // ATSTORAGESERVERCLNT_H   

            
// End of File
