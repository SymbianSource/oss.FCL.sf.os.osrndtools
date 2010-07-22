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
* Description:  Definitions for the class RAnalyzeToolMainAllocator.
*
*/


#include "analyzetoolmainallocator.h"
#include "atlog.h"
#include "analyzetooleventhandler.h"
#include "analyzetoolmemoryallocator.h"
#include "analyzetoolpanics.pan"
#include "analyzetoolfastlog.h"
#include <e32svr.h>


// CONSTANTS

// The name of the memoryhook dll
_LIT8( KMemoryHook, "AToolMemoryHook.dll" );

// The name of the storage server dll
_LIT8( KStorageServer, "AToolStorageServerClnt.dll" );

// Length of the callstack address
const TUint32 KAddressLength = 4;

// Thread count
const TInt KThreadCount = 1;

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::RAnalyzeToolMainAllocator()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
RAnalyzeToolMainAllocator::RAnalyzeToolMainAllocator( TBool aNotFirst, 
    const TFileName aFileName, TUint32 aLogOption, TUint32 aIsDebug,
    TUint32 aAllocCallStackSize, TUint32 aFreeCallStackSize ) :
    RAnalyzeToolMemoryAllocator( aNotFirst ),
    iAnalyzeToolOpen( EFalse ),
    iDeviceDriverLoaded( EFalse ),
    iCodeblocks( KATMaxCallstackLength ),
    iThreadArray( KATMaxCallstackLength ),
    iLogOption( aLogOption ),
    iProcessId( RProcess().Id().operator TUint() ),
    iAllocMaxCallStack( aAllocCallStackSize ),
    iFreeMaxCallStack( aFreeCallStackSize )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::RAnalyzeToolMainAllocator()" );

    // Basic error variable used in method.
    TInt error( KErrNone );
    
    // Connect to the storage server if logging mode not fast trace.
    if ( iLogOption != EATLogToTraceFast )
        {
        error = iStorageServer.Connect();

        LOGSTR2( "ATMH Opening RATStorageServer error %i", error );
    
        if ( KErrNone == error )
            {
            iStorageServerOpen = ETrue;
            }
        else
            {
            iStorageServerOpen = EFalse;
            }
    
        if ( KErrNone == error )
            {
            // Make the storage server handle shared between threads
            error = iStorageServer.ShareAuto();
            }
    
        LOGSTR2( "ATMH Sharing RATStorageServer error %i", error );
        }
    
    // Create mutex for schedule access to shared resources
    error = iMutex.CreateLocal();

    __ASSERT_ALWAYS( KErrNone == error, AssertPanic( ECantCreateMutex ) );

    LOGSTR2( "ATMH Creating mutex error %i", error );

    // Load the kernel side device driver
    error = User::LoadLogicalDevice( KAnalyzeToolLddName );

    if ( error != KErrNone && error != KErrAlreadyExists )
        {
        __ASSERT_ALWAYS( EFalse, AssertPanic( ECantLoadDeviceDriver ) );
        }
    else
        {
        iDeviceDriverLoaded = ETrue;
        }

    LOGSTR2( "ATMH Loading device driver error %i", error );

    // Open handle to the kernel sidedevice driver
    error = iAnalyzeTool.Open();

    __ASSERT_ALWAYS( KErrNone == error, AssertPanic( ECantConnectDeviceDriver ) );

    if ( KErrNone == error )
        {
        iAnalyzeToolOpen = ETrue;
        }

    LOGSTR2( "ATMH Opening RAnalyzeTool handle %i error", error );

    // Set memory model by asking kernel side device driver
    if ( iAnalyzeToolOpen )
        {
        TATMemoryModelBuf model;
        if ( KErrNone == iAnalyzeTool.GetMemoryModel( model ) )
            {
            iMemoryModel = model().iMemoryModel;
            LOGSTR2( "ATMH AnalyzeTool MemoryModel: %i", iMemoryModel );
            }
        else
            LOGSTR2( "ATMH AnalyzeTool GetMemoryModel error: %i", error );
        }
    
    // Retrieve the initial process information
    LogProcessInformation( aFileName, aLogOption, aIsDebug );

    // Create handler for receiving kernel events
    iEventHandler = new CLibraryEventHandler( iAnalyzeTool,
                                              iCodeblocks,
                                              iStorageServer,
                                              iProcessId,
                                              iMutex, 
                                              *this,
                                              aLogOption);

    __ASSERT_ALWAYS( iEventHandler != NULL, AssertPanic( ENoMemory ) );
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::~RAnalyzeToolMainAllocator()
// Destructor.
// -----------------------------------------------------------------------------
//
RAnalyzeToolMainAllocator::~RAnalyzeToolMainAllocator()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::~RAnalyzeToolMainAllocator()" );

    TUint handleLeakCount( 0 );
    if ( iAnalyzeToolOpen && iThreadArray.Count() > 0 )
        {
        TProcessHandleInfoBuf params;
        params().iProcessId = iProcessId;
        TInt error( iAnalyzeTool.GetProcessHandleInfo( params ) );
        handleLeakCount = params().iThreadHandleCount;
        }

    // Close handle for process memory blocks
    iCodeblocks.Close();

    // Delete the eventhandler
    delete iEventHandler;

    // The count of device driver users
    TClientCountBuf count;

    // Check the flag
    if ( iAnalyzeToolOpen )
        {
        TInt error  = iAnalyzeTool.ClientCount( count );
        LOGSTR2( "ATMH closing analyze tool handle error: %i", error );
        iAnalyzeTool.Close();
        }

    // Check the flag
    if ( iDeviceDriverLoaded )
        {
        LOGSTR2( "ATMH device driver client count: %i", count().iClientCount );

        // Check if there is another user for device driver
        if ( count().iClientCount <= 1 )
            {
            // There was no other users -> unload the device driver
            TInt error = User::FreeLogicalDevice( KAnalyzeToolLddName );
            LOGSTR2( "ATMH Unloading ldd error: %i", error );
            }
        }
    
    // Close the thread array
    iThreadArray.Close();

    if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
        {
        if ( iLogOption == EATLogToTraceFast )
            {
            LOGSTR1( "ATMH ATFastLogProcessEnded()" );
            ATFastLogProcessEnded( iProcessId, handleLeakCount );
            }
        else
            {
            iStorageServerOpen = EFalse;
            // Inform that process has ended and close the handle
            LOGSTR1( "ATMH iStorageServer.LogProcessEnded()" );
            iStorageServer.LogProcessEnded( iProcessId, handleLeakCount );
            // Close the handle
            iStorageServer.Close();
            }
        }
    
    // Close the mutex
    iMutex.Close();
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::ShareHeap()
// Share heap with other thread
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::ShareHeap()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ShareHeap()" );

    // Call the overwrited Open function
    Open();
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Uninstall()
// Uninstalls the current allocator
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::Uninstall()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Uninstall()" );

    // Acquire the mutex
    iMutex.Wait();
    
    TMainThreadParamsBuf params;
    params().iProcessId = iProcessId;
    iAnalyzeTool.MainThreadAlloctor( params );
    
    // Release the mutex
    iMutex.Signal();
    
    // Check if this is shared allocator between threads
    if ( iThreadArray.Count() > KThreadCount && !params().iAlone )
        {
        // Close the shared allocator
        Close();
        LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Uninstall() - Close called" );
        return;
        }

#if ( SYMBIAN_VERSION_SUPPORT >= SYMBIAN_3 )
    #ifndef __WINS__ 
    // Remove dummy Tls handle
    UserSvr::DllFreeTls( KDummyHandle );
    #endif
#endif
    
    // Since this is the last thread using this allocator it can be deleted
    delete this;
    }

#ifdef __WINS__
// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Alloc() WINS version
// Allocates a cell of specified size from the heap.
// -----------------------------------------------------------------------------
//
UEXPORT_C TAny* RAnalyzeToolMainAllocator::Alloc( TInt aSize )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc()" );

    // Acquire the mutex
    iMutex.Wait();

    // Alloc memory from the original allocator
    TAny* p = iAllocator->Alloc( aSize );

    LOGSTR3( "ATMH RAnalyzeToolMainAllocator::Alloc() - aSize: %i, address: %x", 
            aSize,  (TUint32) p );

    // Don't collect or log data if storage server not open or logging mode fast.
    if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
        {
        TInt error( KErrNone );
        
        // Check if eventhandler is started already
        if ( !iEventHandler->IsStarted() )
            {
            // Install the eventhandler if needed
            InstallEventHandler();
            }
        
        // Reset the callstack
        iCallStack.Reset();

        // If we don't want any call stack to be saved skip the next part
        if( iAllocMaxCallStack > 0 )
            {
            // Find the current thread callstack start address
            TUint32 stackstart( 0 );
            TBool found( FindCurrentThreadStack( stackstart ) );
            LOGSTR3( "ATMH > stackstart: %x , found = %i", stackstart, found );
            
            // Returns the value of the stack pointer at the 
            // current point in your program.
            TUint32 _sp;
            __asm
                {
                mov [_sp], esp
                }
            
            // Get process loaded code segments count
            TInt blocksCount( iCodeblocks.Count() );
            TUint arrayCounter = 0;
            
            // Iterate through callstack to find wanted callstack addresses
            // - Start: current stack address
            // - Stop: stack start address(Run-address of user stack)
            // - Add: address length(The word size in the current system is 32 bits, which is 4 bytes)
            for ( TUint32 i = _sp; i < stackstart; i = i + KAddressLength )//lint !e1055 !e526 !e628 !e348
                {
                TUint32 addr = (TUint32) *( (TUint32*) i );
                
                // Checks is the given address in loaded code memory area.
                if ( !IsAddressLoadedCode( addr ) )
                    continue;
                
                // Iterate through array of code blocks to check if address is in code segment area 
                for ( TInt j = 0; j < blocksCount; j++ )
                    {
                    // Checks if the given address is in this memory block area
                    if ( iCodeblocks[j].CheckAddress( addr ) )
                        {
                        // To avoid recursive call to ReAlloc specifying granularity
                        // Add address to the callstack
                        iCallStack[arrayCounter] = ( addr );
                        arrayCounter++;
                        break;
                        }
                    }
                
                // Checks if the wanted callstack items are gathered
                if ( arrayCounter == KATMaxCallstackLength ||
                     arrayCounter == iAllocMaxCallStack )
                    {
                    LOGSTR2( "ATMH > Wanted CallStack items ready( %i )", arrayCounter );
                    break;
                    }
                } 
            }

        // Log the memory allocation information
        if ( iLogOption == EATLogToTraceFast )
            {
            // Using fast mode.
            ATFastLogMemoryAllocated( iProcessId, (TUint32) p , iCallStack, aSize );
            }
        else
            {
            // Using storage server.
            error = iStorageServer.LogMemoryAllocated( (TUint32) p,
                                                       iCallStack,
                                                       aSize );
            if ( KErrNone != error )
                {
                switch ( error )
                    {
                    case KErrNoMemory:
                        {
                        LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc() - KErrNoMemory case"  );
                        // Check if eventhandler is active
                        if ( iEventHandler->IsActive() )
                            {
                            // Cancel iEventHandler because not needed anymore
                            iEventHandler->Cancel();
                            }
                        if ( iStorageServerOpen )
                            {
                            // Close storage server
                            iStorageServerOpen = EFalse;
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc() - close iStorageServer"  );
                            iStorageServer.Close();
                            }
                        break;
                        }
                    default:
                        {
                        LOGSTR2( "ATMH LogMemoryAllocated error %i", error );
                        break;
                        }
                    }
                }
            }
        }
    // Release the mutex
    iMutex.Signal();

    return p;
    }
#else

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Alloc() ARMV5 version
// Allocates a cell of specified size from the heap.
// -----------------------------------------------------------------------------
//
TAny* RAnalyzeToolMainAllocator::Alloc( TInt aSize )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc()"  );

    // acquire the mutex
    iMutex.Wait();

    // Alloc memory from the original allocator
    TAny* p = iAllocator->Alloc( aSize );

    LOGSTR3( "ATMH RAnalyzeToolMainAllocator::Alloc() - aSize: %i, address: %x", 
            aSize,  (TUint32) p );

    TInt error( KErrNone );
    
    if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
        {
        // Check if eventhandler is active already
        // IsActive might return false value if a tested software has created many
        // threads which install own CActiveScheduler.
        if ( !iEventHandler->IsStarted() )
            {
            // Install the eventhandler if needed
            InstallEventHandler();
            }
        
        // Reset the callstack
        iCallStack.Reset();
        
        // If we don't want any call stack to be saved skip the next part
        if( iAllocMaxCallStack > 0 )
            {
            // Find the current thread callstack start address
            TUint32 stackstart( 0 );
            TBool found( FindCurrentThreadStack( stackstart ) );
            LOGSTR3( "ATMH > stackstart: %x , found = %i", stackstart, found );
            
            // Get process loaded code segments count
            TInt blocksCount( iCodeblocks.Count() );            
            TUint arrayCounter = 0;
            
            // Iterate through callstack to find wanted callstack addresses
            // - Start: current stack address(__current_sp(): Returns the value of the 
            //      stack pointer at the current point in your program.)
            // - Stop: stack start address(Run-address of user stack)
            // - Add: address length(The word size in the current system is 32 bits, which is 4 bytes)
            for ( TUint32 i = __current_sp(); i < stackstart; i = i + KAddressLength )//lint !e1055 !e526 !e628 !e348
                {
                TUint32 addr = (TUint32) *( (TUint32*) i );
                
                // Checks is the given address in loaded code memory area.
                if ( !IsAddressLoadedCode( addr ) )
                    continue;
                
                // Iterate through array of code blocks to check if address is in code segment area 
                for ( TInt j = 0; j < blocksCount; j++ )
                    {
                    // Checks if the given address is in this memory block area
                    if ( iCodeblocks[j].CheckAddress( addr ) )
                        {
                        // To avoid recursive call to ReAlloc specifying granularity
                        // Add address to the callstack
                        iCallStack[arrayCounter] = ( addr );
                        arrayCounter++;
                        break;
                        }
                    }
                
                // Checks if the wanted callstack items are gathered
                if ( arrayCounter == KATMaxCallstackLength ||
                     arrayCounter == iAllocMaxCallStack )
                    {
                    LOGSTR2( "ATMH > Wanted CallStack items ready( %i )", arrayCounter );
                    break;
                    }
                }
            }
        // Log the memory allocation information
        if ( iLogOption == EATLogToTraceFast )
            {
            // Using fast mode.
            ATFastLogMemoryAllocated( iProcessId, (TUint32) p, iCallStack, aSize );
            }
        else
            {
            // Using storage server.
            error = iStorageServer.LogMemoryAllocated( (TUint32) p,
                                                       iCallStack,
                                                       aSize );
            if ( KErrNone != error )
                {
                switch ( error )
                    {
                    case KErrNoMemory:
                        {
                        LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc() - KErrNoMemory case"  );
                        // Check if eventhandler is active
                        if ( iEventHandler->IsActive() )
                            {
                            // Cancel ieventhandler because not needed anymore
                            iEventHandler->Cancel();
                            }
                        if ( iStorageServerOpen )
                            {
                            // Close storage server
                            iStorageServerOpen = EFalse;
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Alloc() - close iStorageServer"  );
                            iStorageServer.Close();
                            }
                        break;
                        }
                    default:
                        {
                        LOGSTR2( "ATMH LogMemoryAllocated error %i", error );
                        break;
                        }
                    }
                }
            }
        }
    
    // Release the mutex
    iMutex.Signal();

    // Return the allocatated memory
    return p;
    }
#endif // __WINS__

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Free()
// Frees the allocated memory
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::Free( TAny* aPtr )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Free()" );

    // Acquire the mutex
    iMutex.Wait();
    
    if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
        {
        // Reset the callstack
        iFreeCallStack.Reset();
        
        // Check if trace logging mode
        // Also if we don't want any call stack to be stored skip the next part
        if ( (iLogOption == EATUseDefault || iLogOption == EATLogToTrace || iLogOption == EATLogToTraceFast )
                && iFreeMaxCallStack > 0 )
            {
            // Find the current thread callstack start address
            TUint32 stackstart( 0 );
            TBool found( FindCurrentThreadStack( stackstart ) );
            LOGSTR3( "ATMH > stackstart: %x , found = %i", stackstart, found );
            TUint32 _sp;
            
            // Returns the value of the stack pointer at the 
            // current point in your program.
            #ifdef __WINS__
                __asm
                    {
                    mov [_sp], esp
                    }
            #else
                _sp = __current_sp();
            #endif
            
            // Get process loaded code segments count
            TInt blocksCount( iCodeblocks.Count() );
            TUint arrayCounter = 0;
            
            // Iterate through callstack to find wanted callstack addresses
            // - Start: current stack address
            // - Stop: stack start address(Run-address of user stack)
            // - Add: address length(The word size in the current system is 32 bits, which is 4 bytes)            
            for ( TUint32 i = _sp; i < stackstart; i = i + KAddressLength )//lint !e1055 !e526 !e628 !e348
                {
                TUint32 addr = (TUint32) *( (TUint32*) i );
                // Checks is the given address in loaded code memory area.
                if ( ! IsAddressLoadedCode( addr ) )
                    continue;
                
                // Iterate through array of code blocks to check if address is in code segment area 
                for ( TInt j = 0; j < blocksCount; j++ )
                    {
                    // Checks if the given address is in this memory block area
                    if ( iCodeblocks[j].CheckAddress( addr ) )
                        {
                        // To avoid recursive call to ReAlloc specifying granularity
                        // Add address to the callstack
                        iFreeCallStack[arrayCounter] = addr;
                        arrayCounter++;
                        break;
                        }
                    }
                // Checks if the wanted callstack items are gathered
                if ( arrayCounter == KATMaxFreeCallstackLength ||
                     arrayCounter == iFreeMaxCallStack )
                    {
                    break;
                    }
                }
            LOGSTR2( "ATMH > iFreeCallStack count ( %i )", arrayCounter );
            }
        // Log the memory free information.
        if ( iLogOption == EATLogToTraceFast )
            {
            // Using fast mode.
            ATFastLogMemoryFreed( iProcessId, (TUint32) aPtr, iFreeCallStack );
            }
        else
            {
            // Using storage server.
            TInt err( iStorageServer.LogMemoryFreed( (TUint32) aPtr, iFreeCallStack ) );
            if ( err != KErrNone )
                {
                LOGSTR2( "ATMH > LogMemoryFreed err( %i )", err );
                }
            }
        }
    
    // Free the memory using original allocator
    iAllocator->Free( aPtr );

    LOGSTR2( "ATMH RAnalyzeToolMainAllocator::Free() - aPtr: %x", (TUint32)aPtr );

    // Release the mutex
    iMutex.Signal();
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Open()
// Opens this heap for shared access. Opening the heap increases
// the heap's access count by one.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::Open()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Open() " );

    // Acquire the mutex
    iMutex.Wait();

    // Share the memory using original allocator
    TInt error = iAllocator->Open();

    // If everything is OK add thread to the array which use this allocator
    if ( KErrNone == error )
        {
        TThreadParamsBuf params;
        params().iThreadId = RThread().Id().operator TUint();
        error = iAnalyzeTool.ThreadStack( params );

        __ASSERT_ALWAYS( KErrNone == error, AssertPanic( ECantAppendToTheArray ) );

        if ( KErrNone == error )
            {
            LOGSTR2( "ATMH Thread stack address: %x", params().iStackAddress );
            LOGSTR2( "ATMH Thread stack size:    %x", params().iStackSize );
            iThreadArray.Append( TThreadStack( RThread().Id(),
                    params().iStackAddress + params().iStackSize ) );
            }
        }

    // Release the mutex
    iMutex.Signal();

    // Return the error code
    return error;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Close()
// Closes this shared heap. Closing the heap decreases the heap's
// access count by one.
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::Close()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Close()" );

    // Acquire the mutex
    iMutex.Wait();

    // Close the memory using original allocator
    iAllocator->Close();
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Close() - allocator closed" );
    TInt count = iThreadArray.Count();

    // Iterate through array of threads to remove current thread
    for ( TInt i = 0; i < count; i++ )
        {
        // Check if this is current thread
        if ( iThreadArray[ i ].Match() )
            {
            // Remove the thread
            iThreadArray.Remove( i );
            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Close() - thread removed" );
            break;
            }
        }
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Close() - about to mutex signal" );
    // Release the mutex
    iMutex.Signal();
    }

#ifdef __WINS__

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::ReAlloc()
// Increases or decreases the size of an existing cell.
// -----------------------------------------------------------------------------
//
TAny* RAnalyzeToolMainAllocator::ReAlloc( TAny* aPtr, TInt aSize, TInt aMode )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc()" );

    // Acquire the mutex
    iMutex.Wait();

    // Realloc the memory using original allocator
    TAny* ptr = iAllocator->ReAlloc( aPtr, aSize, aMode );
    
    // NULL addresses are not in a process under test
    if ( ptr && !( aMode & ENeverMove ) )
        {
        LOGSTR3( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - aPtr: %x, ptr: %x", 
                (TUint32)aPtr, (TUint32)ptr );
        LOGSTR3( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - aSize: %i, aMode: %i", 
                aSize, aMode );
      
        // Don't collect or log data if storage server not open or logging mode is not fast.
        if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
            {
            TInt error( KErrNone );
            TUint arrayCounter = 0;
            
            // Reset the callstack
            iReCallStack.Reset();
            
            // If we don't want any call stack to be saved skip the next part
            if( iAllocMaxCallStack > 0 )
                {
                // Find the current thread callstack start address
                TUint32 stackstart( 0 );
                TBool found( FindCurrentThreadStack( stackstart ) );
                LOGSTR3( "ATMH > stackstart: %x , find = %i", stackstart, found );
    
                // Returns the value of the stack pointer at the 
                // current point in your program.
                TUint32 _sp( 0 );
                __asm
                    {
                    mov [_sp], esp
                    }
                
                // Get process loaded code segments count
                TInt blocksCount( iCodeblocks.Count() );
                
                // Iterate through callstack to find wanted callstack addresses
                // - Start: current stack address
                // - Stop: stack start address(Run-address of user stack)
                // - Add: address length(The word size in the current system is 32 bits, which is 4 bytes)                
                for ( TUint32 i = _sp; i < stackstart; i = i + KAddressLength )//lint !e1055 !e526 !e628 !e348
                    {
                    TUint32 addr = (TUint32) *( (TUint32*) i );
                    // Checks is the given address in loaded code memory area.
                    if ( ! IsAddressLoadedCode( addr ) )
                        continue;
                    
                    // Iterate through array of code blocks to check if address is in code segment area 
                    for ( TInt j = 0; j < blocksCount; j++ )
                        {
                        // Checks if the given address is in this memory block area
                        if ( iCodeblocks[j].CheckAddress( addr ) )
                            {
                            // To avoid recursive call to ReAlloc specifying granularity
                            // Add address to the callstack
                            iReCallStack[arrayCounter] = addr;
                            arrayCounter++;
                            break;
                            }
                        }
                    // Checks if the wanted callstack items are gathered
                    if ( arrayCounter == KATMaxCallstackLength || 
                         arrayCounter == iAllocMaxCallStack )
                        {
                        LOGSTR2( "ATMH > Wanted CallStack items ready( %i )", arrayCounter );
                        break;
                        }
                    }
                }
            
            // No need to report free if the aPtr was NULL
            if ( aPtr != NULL )
                {
                // Reset the free callstack
                iFreeCallStack.Reset();
                
                // Check that logging mode is trace/trace fast so we use free call stack 
                // and call stack size bigger than zero
                if ( ( iLogOption == EATUseDefault || iLogOption == EATLogToTrace || iLogOption == EATLogToTraceFast ) && iFreeMaxCallStack > 0 )
                    {
                    for ( TInt i = 0; i < arrayCounter; i++ )
                        {
                        if ( i == KATMaxFreeCallstackLength || i == iFreeMaxCallStack )
                            {
                            break;
                            }
                        iFreeCallStack[i] = iReCallStack[i];
                        }
                    }
                // Try to remove old address from the storage server's
                // leak array. If found. it's removed from the array because system frees
                // old address directly in the RHeap in ReAlloc case.
                if ( iLogOption == EATLogToTraceFast )
                    {
                    ATFastLogMemoryFreed( iProcessId, (TUint32) aPtr, iFreeCallStack );
                    }
                else
                    {
                    iStorageServer.LogMemoryFreed( (TUint32) aPtr, iFreeCallStack );
                    }
                }
            // Log the memory allocation information
            if ( iLogOption == EATLogToTraceFast )
                {
                // Using fast logging mode.
                ATFastLogMemoryAllocated( iProcessId, (TUint32) ptr, iReCallStack, aSize );
                }
            else
                {
                // Using storage server.
                error = iStorageServer.LogMemoryAllocated( (TUint32) ptr,
                                                           iReCallStack,
                                                           aSize );
                if ( KErrNone != error )
                    {
                    switch ( error )
                        {
                        case KErrNoMemory:
                            {
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - KErrNoMemory case"  );
                            // Check if eventhandler is active
                            if ( iEventHandler->IsActive() )
                                {
                                // Cancel iEventHandler because not needed anymore
                                iEventHandler->Cancel();
                                }
                            if ( iStorageServerOpen )
                                {
                                // Close storage server
                                iStorageServerOpen = EFalse;
                                LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - close iStorageServer"  );
                                iStorageServer.Close();
                                }
                            break;
                            }
                        default:
                            {
                            LOGSTR2( "ATMH LogMemoryAllocated error %i", error );
                            break;
                            }
                        }
                    }
                }
            }
        }
    
    // Release the mutex
    iMutex.Signal();

    // Return pointer to the reallocated cell
    return ptr;
    }

#else

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::ReAlloc()
// Increases or decreases the size of an existing cell.
// -----------------------------------------------------------------------------
//
TAny* RAnalyzeToolMainAllocator::ReAlloc( TAny* aPtr, TInt aSize, TInt aMode )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc()" );

    // Acquire the mutex
    iMutex.Wait();

    // Realloc the memory using original allocator
    TAny* ptr = iAllocator->ReAlloc( aPtr, aSize, aMode );
    
    TInt error( KErrNone );
    TUint arrayCounter = 0;
    
    // NULL addresses are not in a process under test
    if ( ptr && !( aMode & ENeverMove ) )
        {
        LOGSTR3( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - aPtr: %x, ptr: %x", 
                (TUint32)aPtr, (TUint32)ptr );
        LOGSTR3( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - aSize: %i, aMode: %i", 
                aSize, aMode );
 
        // Don't collect or log data if storage server not open or logging mode is not fast.
        if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
            {
            // Reset the callstack
            iReCallStack.Reset();
            
            // If we don't want any call stack to be saved skip the next part
            if( iAllocMaxCallStack > 0 )
                {
                // Find the current thread callstack start address
                TUint32 stackstart( 0 );
                TBool found( FindCurrentThreadStack( stackstart ) );
                LOGSTR3( "ATMH > stackstart: %x , find = %i", stackstart, found );
                
                // Get process loaded code segments count
                TInt blocksCount( iCodeblocks.Count() );
                
                // Iterate through callstack to find wanted callstack addresses
                // - Start: current stack address(__current_sp(): Returns the value of the 
                //      stack pointer at the current point in your program.)
                // - Stop: stack start address(Run-address of user stack)
                // - Add: address length(The word size in the current system is 32 bits, which is 4 bytes)
                for ( TUint32 i = __current_sp(); i < stackstart; i = i + KAddressLength )//lint !e1055 !e526 !e628 !e348
                    {
                    TUint32 addr = (TUint32) *( (TUint32*) i );
                    
                    // Checks is the given address in loaded code memory area.
                    if ( !IsAddressLoadedCode( addr ) )
                        continue;
                                
                    // Iterate through array of code blocks to check if address is in code segment area 
                    for ( TInt j = 0; j < blocksCount; j++ )
                        {
                        // Checks if the given address is in this memory block area
                        if ( iCodeblocks[j].CheckAddress( addr ) )
                            {
                            // To avoid recursive call to ReAlloc specifying granularity
                            // Add address to the callstack
                            iReCallStack[arrayCounter] = ( addr );
                            arrayCounter++;
                            break;
                            }
                        }
                    // Checks if the wanted callstack items are gathered
                    if ( arrayCounter == KATMaxCallstackLength || 
                         arrayCounter == iAllocMaxCallStack )
                        {
                        LOGSTR2( "ATMH > Wanted CallStack items ready( %i )", arrayCounter );
                        break;
                        }
                    }
                }
            
            // No need to report free if the aPtr was NULL
            if ( aPtr != NULL )
                {
                // Reset the free callstack
                iFreeCallStack.Reset();
                
                // Check that logging mode is trace/trace fast so we use free call stack 
                // and call stack size bigger than zero
                if ( (iLogOption == EATUseDefault || iLogOption == EATLogToTrace || iLogOption == EATLogToTraceFast )
                        && iFreeMaxCallStack > 0 )
                    {
                    for ( TInt i = 0; i < arrayCounter; i++ )
                        {
                        if ( i == KATMaxFreeCallstackLength || i == iFreeMaxCallStack )
                            {
                            break;
                            }
                        iFreeCallStack[i] = ( iReCallStack[i] );
                        }
                    }
                
                // Try to remove old address from the storage server's
                // leak array. If found. it's removed from the array because system frees
                // old address directly in the RHeap in ReAlloc case.
                if ( iLogOption == EATLogToTraceFast )
                    {
                    ATFastLogMemoryFreed( iProcessId, (TUint32) aPtr, iFreeCallStack );
                    }
                else
                    {
                    iStorageServer.LogMemoryFreed( (TUint32) aPtr, iFreeCallStack );
                    }
                }
            
            // Log the memory allocation information
            if ( iLogOption == EATLogToTraceFast )
                {
                // Using fast logging mode.
                ATFastLogMemoryAllocated( iProcessId, (TUint32) ptr, iReCallStack, aSize );
                }
            else
                {
                // Using storage server.
                error = iStorageServer.LogMemoryAllocated( (TUint32) ptr,
                                                        iReCallStack,
                                                        aSize );
                if ( KErrNone != error )
                    {
                    switch ( error )
                        {
                        case KErrNoMemory:
                            {
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - KErrNoMemory case"  );
                            // Check if eventhandler is active
                            if ( iEventHandler->IsActive() )
                                {
                                // Cancel iEventHandler because not needed anymore
                                iEventHandler->Cancel();
                                }
                            if ( iStorageServerOpen )
                                {
                                // Close storage server
                                iStorageServerOpen = EFalse;
                                LOGSTR1( "ATMH RAnalyzeToolMainAllocator::ReAlloc() - close iStorageServer"  );
                                iStorageServer.Close();
                                }
                            break;
                            }
                        default:
                            {
                            LOGSTR2( "ATMH LogMemoryAllocated error %i", error );
                            break;
                            }
                        }
                    }
                }
            }
        }

    // Release the mutex
    iMutex.Signal();

    // Return pointer to the reallocated cell
    return ptr;
    }

#endif // __WINS__

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Compress()
// The function frees excess committed space from the top of the heap.
// The size of the heap is never reduced below the minimum size
// specified during creation of the heap.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::Compress()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Compress()" );

    // Acquire the mutex
    iMutex.Wait();

    // Compress the memory using original allocator
    TInt compress = iAllocator->Compress();

    // Release the mutex
    iMutex.Signal();

    // Return the space reclaimed
    return compress;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Reset()
// Frees all allocated cells on this heap.
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::Reset()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Reset()" );

    // Acquire the mutex
    iMutex.Wait();

    // Reset the memory using original allocator
    iAllocator->Reset();

    // Release the mutex
    iMutex.Signal();
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::AllocSize()
// Gets the number of cells allocated on this heap, and
// the total space allocated to them.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::AllocSize( TInt& aTotalAllocSize ) const
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::AllocSize()" );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Acquire the memory information using original allocator
    TInt size = iAllocator->AllocSize( aTotalAllocSize );
    
    // Release the mutex
    iMutex.Signal();
    
    // Return the number of cells allocated on this heap.
    return size;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Available()
// Gets the total free space currently available on the heap and the
// space available in the largest free block. The space available
// represents the total space which can be allocated. Note that
// compressing the heap may reduce the total free space available
// and the space available in the largest free block.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::Available( TInt& aBiggestBlock ) const
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Available()" );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Acquire the memory information using original allocator
    TInt available = iAllocator->Available( aBiggestBlock );
    
    // Release the mutex
    iMutex.Signal();
    
    // Return the total free space currently available on the heap
    return available;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::AllocLen()
// Gets the length of the available space in the specified
// allocated cell.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::AllocLen( const TAny* aCell ) const
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::AllocLen()" );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Acquire the memory information using original allocator
    TInt len = iAllocator->AllocLen( aCell );
    
    // Release the mutex
    iMutex.Signal();
    
    // Return the length of the available space in the allocated cell.
    return len;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::DebugFunction()
// Invocates specified debug funtionality.
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::DebugFunction( TInt aFunc, TAny* a1, TAny* a2 )
    {
    LOGSTR2( "ATMH RAnalyzeToolMainAllocator::DebugFunction() %i", aFunc );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Invocate debug funtion using original allocator
    TInt debug = iAllocator->DebugFunction( aFunc, a1, a2 );
    
    switch( aFunc )
        {  
        case EMarkEnd:
            {
            // Disables the __UHEAP_MARKEND macro
            LOGSTR1( "ATMH __UHEAP_MARKEND macro called" );
            if ( debug > 0 )
                {
                LOGSTR2( "ATMH __UHEAP_MARKEND detects leaks: %d", debug );
                // Because there is leaks the alloc panic will occur but
                // lets return a zero to pretend that everything is OK
                debug = 0;
                }
            }
        break;
        
        default:
            {
            }
        break;
        }
    
    // Release the mutex
    iMutex.Signal();
    
    // Return information of the debug function success
    return debug;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::RemoveKilledThread()
// Remove killed thread from threads array.
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::RemoveKilledThread( const TUint aThreadId  )
    {
    LOGSTR2( "ATMH RAnalyzeToolMainAllocator::RemoveKilledThread(%i)", 
            aThreadId );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Iterate through array of threads to remove current thread
    TInt count( iThreadArray.Count() );
    LOGSTR2( "ATMH > iThreadArray.Count() %i", count );
    
    for ( TInt i = 0; i < count; i++ )
        {
        // Check if this is current thread
        if ( iThreadArray[ i ].Match( aThreadId ) )
            {
            // Remove the thread
            iThreadArray.Remove( i );
            LOGSTR1( "ATMH > thread removed" );
            break;
            }
        }

    // Release the mutex
    iMutex.Signal();
    }
// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::Extension_()
// Extension function
// -----------------------------------------------------------------------------
//
TInt RAnalyzeToolMainAllocator::Extension_( TUint aExtensionId, TAny*& a0,
    TAny* a1)
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::Extension_()" );
    
    // Acquire the mutex
    iMutex.Wait();
    
    // Invocate extension funtion using original allocator
    TInt ext = RAllocator::Extension_( aExtensionId, a0, a1 );
    
    // Release the mutex
    iMutex.Signal();
    
    // Return information of the extension function success
    return ext;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::LogProcessInformation()
// Retrieve and log the process initial information
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::LogProcessInformation( const TFileName aFileName,
    TUint32 aLogOption, TUint32 aIsDebug )
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation()" );
    
    // Create local variable and retrieve the process information
    TProcessIdentityParamsBuf params;
    params().iProcessId = iProcessId;
    params().iThreadId = RThread().Id().operator TUint();
    TInt error = iAnalyzeTool.GetProcessInfo( params );
    
    LOGSTR2( "ATMH GetProcessInfo %i error", error );
    
    if ( KErrNone == error )
        {
        LOGSTR2( "ATMH Process %i", iProcessId );

        // Store stack start address
        LOGSTR2( "ATMH Thread stack address: %x", params().iStackAddress );
        LOGSTR2( "ATMH Thread stack size:    %x", params().iStackSize );

        // Append thread to array of the users of this allocator
        error = iThreadArray.Append(
        TThreadStack( RThread().Id(), params().iStackAddress + params().iStackSize) );

        __ASSERT_ALWAYS( KErrNone == error, AssertPanic( ECantAppendToTheArray ) );

        // Log process information
        if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
            {
            if ( iLogOption == EATLogToTraceFast )
                {
                // Using fast logging mode.
                LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - ATFastLogProcessStarted() #1" );
                ATFastLogProcessStarted( params().iProcessName, iProcessId, aIsDebug );                
                }
            else
                {
                // Using storage server.
                LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - iStorageServerOpen #1" );
                error = iStorageServer.LogProcessStarted(
                        aFileName,
                        params().iProcessName,
                        iProcessId, 
                        aLogOption, 
                        aIsDebug );
                }
            }

        LOGSTR2( "ATMH LogProcessStarted error %i", error );

        // Iterate through process codesegments
        for( TInt i = 0; i < params().iCodesegCount; i++ )
            {
            // Create local variable and retrieve codesegment info
            TCodesegInfoBuf codeinfo;
            codeinfo().iProcessId = iProcessId;
            codeinfo().iIndex = i;
            error = iAnalyzeTool.GetCodesegInfo( codeinfo );

            LOGSTR2( "ATMH GetCodesegInfo error %i", error );
            if ( KErrNone == error )
                {
                // Don't log AnalyzeTool libraries
                if ( 0 != codeinfo().iFullName.CompareC( KMemoryHook ) &&
                     0 != codeinfo().iFullName.CompareC( KStorageServer ) )
                    {
                    // Log the loaded codesegment(s)
                    if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
                        {
                        if ( iLogOption == EATLogToTraceFast )
                            {
                            // Using fast logging mode.
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - ATFastLogDllLoaded() #2" );
                            ATFastLogDllLoaded( iProcessId,
                                    codeinfo().iFullName,
                                    codeinfo().iRunAddress,
                                    codeinfo().iRunAddress + codeinfo().iSize );
                            }
                        else
                            {
                            // Using storage server.
                            LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - iStorageServerOpen #2" );
                            error = iStorageServer.LogDllLoaded(
                                    codeinfo().iFullName,
                                    codeinfo().iRunAddress,
                                    codeinfo().iRunAddress + codeinfo().iSize );
                            }
                        }

                    LOGSTR2( "ATMH LogDllLoaded error %i", error );

                    // Check that everything is OK
                    if ( KErrNone == error )
                        {
                        // Append the codesegment to the array
                        error = iCodeblocks.Append(
                                        TCodeblock( codeinfo().iRunAddress,
                                        codeinfo().iSize,
                                        codeinfo().iFullName ) );
                        LOGSTR2( "ATMH Append error %i", error );
                        }
                    }
                }
            }

        // Iterate through process dynamic codesegments
        for ( TInt i = 0; i < params().iDynamicCount; i++ )
            {
            // Create local variable and retrieve codesegment info
            TLibraryInfoBuf info;
            info().iProcessId = iProcessId;
            info().iIndex = i;
            error = iAnalyzeTool.GetLibraryInfo( info );
            LOGSTR2( "ATMH GetLibraryInfo error %i", error );
            if ( KErrNone == error )
                {
                // Log the loaded dynamic codesegment(s)
                if ( iStorageServerOpen || iLogOption == EATLogToTraceFast )
                    {
                    if ( iLogOption == EATLogToTraceFast )
                        {
                        // Using fast logging mode.
                        LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - - ATFastLogDllLoaded()#3" );
                        ATFastLogDllLoaded( iProcessId,
                                info().iLibraryName,
                                info().iRunAddress,
                                info().iRunAddress + info().iSize );
                        }
                    else
                        {
                        // Using storage server.
                        LOGSTR1( "ATMH RAnalyzeToolMainAllocator::LogProcessInformation() - iStorageServerOpen #3" );
                        error = iStorageServer.LogDllLoaded(
                                info().iLibraryName,
                                info().iRunAddress,
                                info().iRunAddress + info().iSize );
                        }
                    }


                LOGSTR2( "ATMH LogDllLoaded error %i", error );

                if ( KErrNone == error )
                    {
                    // Append the codesegment to the array
                    error = iCodeblocks.Append(
                            TCodeblock( info().iRunAddress, 
                                        info().iSize, info().iLibraryName ) );
                    LOGSTR2( "ATMH Append error %i", error );
                    }
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::FindCurrentThreadStack()
// Find the current thread which is using the heap
// -----------------------------------------------------------------------------
//
TBool RAnalyzeToolMainAllocator::FindCurrentThreadStack( TUint32& aStackStart )
    {
    LOGSTR2( "ATMH RAnalyzeToolMainAllocator::FindCurrentThreadStack(), count( %i )",
            iThreadArray.Count() );
    
    // Flag for indicating that right thread has been found
    TBool found( EFalse );
    // If threre is only one thread it must be the right thread
    if ( iThreadArray.Count() == KThreadCount )
        {
        if ( !iThreadArray[ 0 ].ThreadStackStart( aStackStart ) )
            {
            // This MUST BE the right thread
            //__ASSERT_ALWAYS( EFalse, AssertPanic( ECantFindRightThread ) );
            }
        else if ( iThreadArray[ 0 ].ThreadStackStart( aStackStart ) )
            {
            found = ETrue;
            }
        }
    else
        {
        // Iterate through array to find right thread
        TInt count = iThreadArray.Count();

        for ( TInt i = 0; i < count; i++ )
            {
            // Check if this is the right thread
            if ( iThreadArray[ i ].ThreadStackStart( aStackStart ) )
                {
                // Right thread found. Mark the flag
                found = ETrue;
                break;
                }
            }
        // If right thread was not found the panic must be raised
        if ( !found )
            {
            //__ASSERT_ALWAYS( EFalse, AssertPanic( ECantFindRightThread ) );
            }
        }
    return found;
    }

// -----------------------------------------------------------------------------
// RAnalyzeToolMainAllocator::InstallEventHandler()
// Installs the eventhandler, if possible
// -----------------------------------------------------------------------------
//
void RAnalyzeToolMainAllocator::InstallEventHandler()
    {
    LOGSTR1( "ATMH RAnalyzeToolMainAllocator::InstallEventHandler()" );
    
    // Active eventhalder is not active, trying to start it
    if ( NULL != CActiveScheduler::Current() )
        {
        iEventHandler->Start();
        }
    }

// End of File
