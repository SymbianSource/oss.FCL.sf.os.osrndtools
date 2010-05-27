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
* Description:  Declaration of the server side session class CATStorageServerSession
*
*/


#ifndef ATSTORAGESERVERSESSION_H
#define ATSTORAGESERVERSESSION_H


//  INCLUDES
#include    <e32base.h>
#include    <f32file.h>
#include    <analyzetool/atcommon.h>

// FORWARD DECLARATIONS
class CATStorageServer;
class CATMemoryEntry;

// CLASS DECLARATIONS

/**
*  A class for storing memory addresses and sizes of allocations.
*  Needed for giving allocation information for the configuration UI.
*/
class TAllocInfo
    {
    public:
    
    // Constructor
    TAllocInfo( TUint32 aMemAddress, TInt aAllocSize );
    
    /** Address of a memory allocation. */
    const TUint32 iMemAddress;
    
    /** Size of the associated allocation. */
    const TInt iAllocSize;
    };


/**
*  The session class for Storage Server.
*  When the Storage Server client connects to the server, a new object of this
*  class is constructed. So, for every client there is an associated object
*  of this class. CATStorageServer forwards all client messages to their associated
*  session objects.
*/
class CATStorageServerSession : public CSession2
    {
    public: // Enumerations
        
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor that can leave.
        * @return A new instance of this class
        */
        static CATStorageServerSession* NewL( CATStorageServer& aStorageServer );
        
        /**
        * Two-phased constructor that can leave and leaves a pointer
        * on the cleanup stack.
        * @return A new instance of this class
        */
        static CATStorageServerSession* NewLC( CATStorageServer& aStorageServer );
        
        /**
        * Destructor
        */
        virtual ~CATStorageServerSession();
        
    public: // Functions from base classes 
        
        /**
        * Receives messages sent by a client.
        * @param aMessage A message from a client
        */
        virtual void ServiceL( const RMessage2& aMessage );
        
    protected: // new functions
        
        /**
        * Initializes this session for logging. This includes opening a logging file
        * with a requested name. The method is to be called as the first method after
        * connecting to the server. The method logs information on process starting into
        * the just opened logging file.
        * If a logging session is already ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is already
        * ongoing. Otherwise one of the other system wide error codes.
        */
        TInt LogProcessStartedL( const RMessage2& aMessage );
        
        /**
        * Initializes this session for logging. The method is to be called as the first
        * method after connecting to the server. The method logs information on process
        * starting into debug channel.
        * If a logging session is already ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is already
        * ongoing. Otherwise one of the other system wide error codes.
        */
        TInt LogProcessStartTraceL( const RMessage2& aMessage );
         
        /**
        * Logs information on a DLL load into the opened file.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; Otherwise one of the other system wide error codes.
        */
        TInt LogDllLoadedL( const RMessage2& aMessage );
        
        /**
        * Logs information on a DLL load into debug channel.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; Otherwise one of the other system wide error codes.
        */
        TInt LogDllLoadTraceL( const RMessage2& aMessage );
        
        /**
        * Logs information on a DLL unload into the opened file.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; Otherwise one of the other system wide error codes.
        */
        TInt LogDllUnloadedL( const RMessage2& aMessage );
        
        /**
        * Logs information on a DLL unload into debug channel.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; Otherwise one of the other system wide error codes.
        */
        TInt LogDllUnloadTraceL( const RMessage2& aMessage );
        
        /**
        * Stores a memory allocation (memory address, current call stack, allocation
        * time and allocation size) in an internal array.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrAlreadyExists, if an attempt is being made to allocate
        *   again an already allocated memory area; Otherwise one of the other system
        *   wide error codes.
        */
        TInt LogMemoryAllocatedL( const RMessage2& aMessage );
        
        /**
        * Logs information on a memory allocation into debug channel.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrAlreadyExists, if an attempt is being made to allocate
        *   again an already allocated memory area; Otherwise one of the other system
        *   wide error codes.
        */
        TInt LogMemoryAllocTraceL( const RMessage2& aMessage );
                
        /**
        * Check a memory allocation (memory address) from an internal array.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @param aRemoveAlloc Boolean to enable removing the found address from allocations.
        * @return KErrNone, if memory address found in array; 
        *   KErrNotFound, if the requested memory address was not found.
        */
        TInt IsMemoryAdded( const RMessage2& aMessage, const TBool aRemoveAlloc = EFalse );
        
        /**
        * Removes a memory allocation (memory address, current call stack, allocation
        * time and allocation size) from an internal array.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrNotFound, if the requested memory address was not
        *   found; Otherwise one of the other system wide error codes.
        */
        TInt LogMemoryFreedL( const RMessage2& aMessage );
        
        /**
        * Logs information on freeing of a memory allocatin into debug channel.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrNotFound, if the requested memory address was not
        *   found; Otherwise one of the other system wide error codes.
        */
        TInt LogMemoryFreedTraceL( const RMessage2& aMessage );
        
        /**
        * Logs all non-released memory allocations into the opened file as memory leaks.
        * Then the method logs a number of possibly occured handle leaks into the file.
        * Next, the method logs information on process ending into the logging file.
        * Finally, this method closes the logging file opened for the connected process,
        * and ends the current logging session. However, this method does not end the 
        * actual client-server session.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrNotSupported if this method is called with a wrong
        *   process id from the client. Otherwise one of the other system wide error
        *   codes.
        */
        TInt LogProcessEndedL( const RMessage2& aMessage );
        
        /**
        * Logs a number of possibly occured handle leaks into debug channel. Then, the method
        * logs information on process ending into debug channel. The method also
        * ends the current logging session. However, this method does not end the actual
        * client-server session.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrCancel, if a logging session is not
        *   currently ongoing; KErrNotSupported if this method is called with a wrong
        *   process id from the client. Otherwise one of the other system wide error
        *   codes.
        */
        TInt LogProcessEndTraceL( const RMessage2& aMessage );
        
        /**
        * Checks if a given memory address can be found.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return When the system is logging into a file: the index of a found 
        *   memory address, or KErrNotFound, if a matching memory address cannot
        *   be found. KErrNone, when the system is not logging into a file.
        *   KErrCancel, if a logging session is not currently ongoing.
        */
        TInt CheckMemoryAddressL( const RMessage2& aMessage );
        
        /**
        * Checks if a given memory address can be found.
        * If a logging session is not ongoing, the method makes the client raise an
        * STSEClient: 3 panic, and returns KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return When the system is logging into a file: the index of a found 
        *   memory address, or KErrNotFound, if a matching memory address cannot
        *   be found. KErrNone, when the system is not logging into a file.
        *   KErrCancel, if a logging session is not currently ongoing.
        */
        TInt CheckMemoryAddressTrace( const RMessage2& aMessage );
        
        /**
        * Gets all the processes currently logging with AnalyzeTool
        * @param aMessage A message that includes parameters sent by the client.
        *   On return a buffer including all the currently logging processes is written
        *   into this message.
        * @return KErrNone, if successful; otherwise one of the other system wide error
        *   codes.
        */
        TInt GetProcessesL( const RMessage2& aMessage );
        
        /**
        * Gets all the DLLs loaded by the process determined by the parameters.
        * @param aMessage A message that includes parameters sent by the client.
        *   A buffer including all the DLLs loaded by the requested process is
        *   written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetDllsL( const RMessage2& aMessage );
        
        /**
        * Gets the logging mode of the process determined by the parameters.
        * @param aMessage A message that includes parameters sent by the client.
        *   On return, the logging mode is written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetLoggingModeL( const RMessage2& aMessage );
        
        /**
        * Starts a sub test for the given process.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrNotFound, if the requested process is not
        *   found; KErrNotSupported, if the requested process does not have a logging
        *   session ongoing or its logging mode is not EATLogToTrace; Otherwise one of the
        *   other system wide error codes.
        */
        TInt StartSubtestL( const RMessage2& aMessage );
        
        /**
        * Stops a sub test for the given process.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrNotFound, if the requested process is not
        *   found; KErrNotSupported, if the requested process does not have a logging
        *   session ongoing or its logging mode is not EATLogToTrace; Otherwise one of the
        *   other system wide error codes.
        */
        TInt StopSubtestL( const RMessage2& aMessage );
        
        /**
        * Starts a sub test for the calling process. Does not need a PID from the client.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrNotSupported, if the calling process
        *   does not have a logging session ongoing or its logging mode is not
        *   EATLogToTrace; Otherwise one of the other system wide error codes.
        */
        TInt StartSubtest2L( const RMessage2& aMessage );
        
        /**
        * Stops a sub test for the calling process. Does not need a PID from the client.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrNotSupported, if the calling process
        *   does not have a logging session ongoing or its logging mode is not
        *   EATLogToTrace; Otherwise one of the other system wide error codes.
        */
        TInt StopSubtest2( const RMessage2& aMessage );
        
        /**
        * Gets the number and total size of the current memory chunks allocated by the
        * requested process.
        * @param aMessage A message that includes parameters sent by the client. The
        *   requested information is written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetCurrentAllocsL( const RMessage2& aMessage );
        
        /**
        * Gets the maximum number of memory chunks and the maximum amount of memory 
        * allocated by the requested process during the test run.
        * @param aMessage A message that includes parameters sent by the client. The
        *   requested information is written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetMaxAllocsL( const RMessage2& aMessage );
        
        /**
        * Cancels logging for the requested process. After logging of a given process
        * has been cancelled, the session associated with that process will not be
        * usable anymore. If a process wants to start logging again, it needs to close
        * the handle and open it again in order to create a new session. Until then, any
        * of the server's methods that can be called by the client will return
        * KErrCancel.
        * @param aMessage A message that includes parameters sent by the client.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt CancelLoggingL( const RMessage2& aMessage );
        
        /**
        * Gets the logging file of the process determined by the parameters.
        * @param aMessage A message that includes parameters sent by the client.
        *   On return, the logging file is written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetLoggingFileL( const RMessage2& aMessage );
        
        /**
        * Gets the mode of the process determined by the parameters.
        * @param aMessage A message that includes parameters sent by the client.
        *   On return, the logging file is written into this message.
        * @return KErrNone, if successful; KErrNotFound, if the requested process was
        *   not found; otherwise one of the other system wide error codes.
        */
        TInt GetUdebL( const RMessage2& aMessage );
        
        /**
        * Sets the mode of the process determined by the parameters.
        * @param aMessage A message that includes parameters sent by the client.
        */
        void SetUdeb( const RMessage2& aMessage );
        
    private:  // New functions for internal use  
    
        /**
        * Releases the resources reserved by this object, and before closing the logging
        * file and file server session, the method logs information on the occured error
        * into the logging file.
        * @param aError The error code to be logged.
        */
        void HandleError( TInt aError );
        
        /**
        * Releases the resources reserved by this object, and logs information on the
        * occured error into debug channel.
        * @param aError The error code to be logged.
        */
        void HandleErrorTrace( TInt aError );
        
        /**
        * Opens a file server session and a file with the name specified.
        * @param aFileName The name of the file into which to log.
        * @param aProcessName Current process name.
        * @return KErrNone, if successful; otherwise one of the other system wide
        *   error codes.
        */
        TInt OpenFsAndFile( const TDesC& aFileName, const TDesC8& aProcessName );
        
        /**
        * Method is used to parse file name extension.
        * @param aFileName The name of the file into which to log.
        * @param aExtension Parsed file extension. 
        */
        void ParseExtension( TDes& aFileName, TDes& aExtension );
        
        /**
        * Called internally when need generate new file name.
        * @param aFileName The name of the file into which to log.
        * @param aProcessName Current process name.
        */
        void GenerateNewFileName( TDes& aFileName, const TDesC8& aProcessName );
        
        /**
        * Method is used to check that file exists and is valid.
        * @param aFileName The name of the file into which to log.
        */
        void CheckIfFileAlreadyExist( const TDes& aFileName );
        
        /**
        * Method is used to check file version.
        * @param aFileName The name of the file into which to log.
        */
        void CheckFileVersion( const TDes& aFileName );
        
        /**
        * Closes the handles to the file server session and to the file (if currently open).
        */
        void CloseFsAndFile();
        
        /**
        * Writes the memory leaks stored in the iLeakArray member array into the
        * logging file opened for the current logging session. If the method is
        * called in a wrong program state (logging session is not ongoing, or the
        * logging mode is not EATLogToFile), STSEClient: 4, and STSEServer: 4
        * panics are raised.
        * @param aMessage The current message from the client.
        * @return KErrNone, if successful; otherwise one of the other system wide
        *   error codes.
        */
        TInt PrintLeaksL( const RMessage2& aMessage );
        
        /**
        * Sets the server's mode of operation. The operation mode is determined with
        * an enumeration value, which is received from the client. If the method is
        * called in a wrong program state (logging session is ongoing), STSEClient: 4,
        * and STSEServer: 4 panics are raised.
        * @param aMessage The current message from the client.
        */
        void SetLogOption( const RMessage2& aMessage );
        
        /**
        * Logs the given descriptor through debug channel.
        * @param aLogString The descriptor to be logged.
        * @return KErrNone, if successful; KErrNotSupported, if a logging session is not
        *   ongoing, or the logging mode is not EATLogToTrace;
        */
        TInt LogThroughTrace( const TDesC& aLogString ) const;
        
        /**
        * Gives a reference to the allocation info array of this class.
        * @return A reference to iAllocInfoArray array
        */
        RArray<TAllocInfo>& AllocInfoArray();
               
        /**
        * Creates a panic in the associated client's code.
        * @param aPanic The panic code
        * @param aMessage The message associated with this panic.
        */
        void PanicClient( TInt aPanic, const RMessage2& aMessage );
        
        /**
        * Logs the abnormal end to the debug channel (If logging mode is EATLogToTrace) 
        * and to the file(If logging mode is EATLogToFile).
        */
        void LogAbnormalEnd();
        
        /**
        * Get the current universal time.
        * @return Time
        */
        TInt64 GetTime();
        
    private: // Constructors 
        
        /**
        * C++ default constructor.
        */
        CATStorageServerSession( CATStorageServer& aStorageServer );
        
        /**
        *  Symbian 2nd phase constructor.
        */
        void ConstructL();   
        
    private: // Data
        
        /** A reference to the CATStorageServer object that has created this session */    
        CATStorageServer& iStorageServer;
               
        /** A handle to a file server */
        RFs iFileServer;
        
        /** A handle to a file */
        RFile iFile;
        
        /** A member variable for storing results of server function calls. */ 
        TInt iError;
        
        /** An object for getting time stamps */ 
        TTime iTime;
        
        /** 
        * An array for storing CATMemoryEntry pointers. Used when logging
        * to an S60 file.
        */
        RPointerArray<CATMemoryEntry> iLeakArray;
        
        /** 
        * An array for storing TAllocInfo objects. Used for keeping records on memory
        * allocations and their sizes.
        */
        RArray<TAllocInfo> iAllocInfoArray;
        
        /** The ID of the process associated with this session. */
        TUint iProcessId;
        
        /**
        * A boolean telling whether this session has been initialized for logging
        * ( whether the client's LogProcessStarted() method has been called ) or not.
        */
        TBool iLoggingOngoing;
        
        /**
        * A variable telling the number of microseconds from January 1st, 0 AD
        * nominal Gregorian to January 1st, 1970 AD nominal Gregorian. 
        */
        TInt64 iMicroSecondsAt1970;
        
        /**
        * An enumeration that tells the current operation mode of the Storage Server.
        */
        TATLogOption iLogOption;
        
        /** The total size of all the current allocations of this process */
        TUint32 iCurAllocSize;
        
        /** The maximum number of allocations of this process */
        TUint32 iMaxAllocs;
        
        /** The maximum total size of allocations of this process */
        TUint32 iMaxAllocSize;
        
        /** The logging file of this process */
        TBuf8<KMaxFileName> iLogFile;

        /** The mode of the session. Default is 1 = UDEB */
        TUint32 iIsUdeb;
    };

#endif      // ATSTORAGESERVERSESSION_H

// End of File
