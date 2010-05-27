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


#include <e32debug.h> // RDebug
#include <analyzetool/analyzetooltraceconstants.h>
#include "analyzetoolfastlog.h"
#include "atlog.h"

// Local time function.
TInt64 CurrentTime()
    {
    LOGSTR1( "ATFL CurrentTime()" );
    TTime time;
    time.UniversalTime();
    return time.Int64() - KMicroSecondsAt1970;
    }

TInt ATFastLogProcessStarted( const TDesC8& aProcessName,
                                 TUint aProcessId,
                                 TUint32 aIsDebug )
    {
    LOGSTR1( "ATFL ATFastLogProcessStarted()" );
    // Convert process name to 16-bit descriptor.
    TBuf<KMaxProcessName> processName;
    processName.Copy( aProcessName );
    // Buffer to trace.
    TBuf<KProcessStartBufLength> buffer;
    // Format process name and id.
    buffer.Format( KProcessStart16, &processName, aProcessId );
    // Timestamp.
    buffer.AppendNum( CurrentTime(), EHex ) ;
    // Append udeb/urel information to the process start.
    buffer.Append( KSpaceTrace );  
    buffer.AppendNum( aIsDebug, EHex );
    // Append version number.
    buffer.Append( KSpaceTrace );  
    buffer.AppendNum( KATTraceVersion, EHex );
    // Log to trace.
    RDebug::Print( KTraceMessage, aProcessId ,&buffer );
    return KErrNone;
    }

TInt ATFastLogProcessEnded( TUint aProcessId, 
                            TUint aHandleLeakCount )
    {
    LOGSTR1( "ATFL ATFastLogProcessEnded()" );
    // Handle leaks.
    if ( aHandleLeakCount > 0 )
        {
        // Buffer to trace.
        TBuf<KHandleLeakBufLength> buffer2;
        buffer2.Format( KHandleLeak16, &KUnknownModule16, aHandleLeakCount );
        // Trace it.
        RDebug::Print( KTraceMessage, aProcessId, &buffer2 );
        }
    // Process end trace.
    TBuf<KProcessEndBufLength> buffer;
    buffer.Format( KProcessEnd16, aProcessId );
    buffer.AppendNum( CurrentTime(), EHex);
    buffer.Append( KNewLineTrace );
    RDebug::Print( KTraceMessage, aProcessId, &buffer );
    return KErrNone;
    }

TInt ATFastLogDllLoaded( TUint aProcessId, 
                                        const TDesC8& aDllName,
                                        TUint32 aStartAddress,
                                        TUint32 aEndAddress )
    {
    LOGSTR1( "ATFL ATFastLogDllLoaded()" );
    // Timestamp.
    TInt64 time = CurrentTime();
    // Convert dll name to 16-bit descriptor.
    TBuf<KMaxLibraryName> dll;
    dll.Copy( aDllName );
    // Buffer to trace.
    TBuf<KDllLoadBufLength> buffer;
    buffer.Format( KDllLoad16, &dll, time, aStartAddress, aEndAddress );
    RDebug::Print( KTraceMessage, aProcessId, &buffer );
    return KErrNone;
    }

TInt ATFastLogDllUnloaded( TUint aProcessId, const TDesC8& aDllName, TUint32 aStartAddress,
                                       TUint32 aEndAddress )
    {
    LOGSTR1( "ATFL ATFastLogDllUnloaded()" );
    // Timestamp.
    TInt64 time = CurrentTime();
    // Convert dll name to 16-bit descriptor.
    TBuf<KMaxLibraryName> dll;
    dll.Copy( aDllName );
    // Buffer to trace.
    TBuf<KDllLoadBufLength> buffer;
    buffer.Format( KDllUnload16, &dll, time, aStartAddress, aEndAddress );
    RDebug::Print( KTraceMessage, aProcessId, &buffer );   
    return KErrNone;
    }

TInt ATFastLogMemoryAllocated( TUint aProcessId, TUint32 aMemAddress,
                                  TFixedArray<TUint32, KATMaxCallstackLength>& aCallstack,
                                  TInt aSize )
    {
    LOGSTR1( "ATFL ATFastLogMemoryAllocated()" );
    // ALLOCH <Memory address> <Time stamp> <Allocation size> <Call stack address count> 
    // <Call stack address> <Call stack address> ...
    
    // Timestamp.
    TInt64 time = CurrentTime();
    
    // Trace buffer and pointer to it.
    TBufC<KMemAllocBufLength> buffer;
    TPtr ptr( buffer.Des() );
    // Append the tag implying a memory allocation line in the data file
    ptr.Append( KMemoryAllocHeader );
    
    // Append the start address of this allocation in the 32-bit (max 8 characters)
    // hexadecimal text format.
    ptr.AppendNum( aMemAddress, EHex );
    
    // Append the current time in the 64-bit (max 16 characters) hexadecimal text
    // format
    ptr.Append( KSpaceTrace );
    ptr.AppendNum( time, EHex );
    
    // Append the size of the allocation in the 32-bit (max 8 characters) hexadecimal
    // text format.
    ptr.Append( KSpaceTrace );
    ptr.AppendNum( aSize, EHex );
    
    // Search call stack for address count.
    TInt addrCount(0);
    for ( TInt j = 0; j < aCallstack.Count() ; j++ )
        {
        if ( aCallstack.At(j) == 0 )
            break;
        addrCount++;
        }
    // Current position in call stack.
    TInt addrPos( 0 );
    
    // Append address count.
    ptr.Append( KSpaceTrace );
    ptr.AppendNum( addrCount, EHex );
            
    // Calculate last item length
    TInt lastItemLength( KTraceMessage().Length() + KHexa32Length + 
            KSpaceLength + KNewlineLength );
    
    TUint packetNumber( 1 );
    
    // Go through all call stack's memory addresses associated with
    // this memory allocation 
    for ( TInt j = 0; j < addrCount; j++ )
        {
        // ALLOCF <Memory address> <Time stamp> <Packet number> 
        // <Call stack address> <Call stack address> ...
        if ( ptr.Length() <= 0 )
            {               
            // Create alloc fragment message header
            ptr.Append( KMemoryAllocFragment );
            ptr.AppendNum( aMemAddress, EHex );
            ptr.Append( KSpaceTrace );
            ptr.AppendNum( time, EHex );
            ptr.Append( KSpaceTrace );        
            ptr.AppendNum( packetNumber, EHex );
            // Increase packet number
            packetNumber++;
            }
      
        // Append call stack address.
        ptr.AppendFormat( KHexaNumberTrace, aCallstack.At( addrPos ) );
        
        // Move the call stack position.
        addrPos++;
        
        // Check if buffer max length exceed
        if ( lastItemLength + ptr.Length() >= KMemAllocBufLength )
            {
            ptr.Append( KNewLineTrace );
            // Log through debug channel 
            RDebug::Print( KTraceMessage, aProcessId, &buffer );
            // Empty trace buffer
            ptr.Delete( 0, ptr.MaxLength() );
            }
        }
    // Send last message if exists.
    if ( ptr.Length() > 0 )
        {
        ptr.Append( KNewLineTrace );
        RDebug::Print( KTraceMessage, aProcessId, &buffer);
        }
    return KErrNone;
    }


TInt ATFastLogMemoryFreed( TUint aProcessId, TUint32 aMemAddress, 
                              TFixedArray<TUint32, KATMaxFreeCallstackLength>& aFreeCallstack )
    {
    LOGSTR1( "ATFL ATFastLogMemoryFreed()" );
    // FREEH <Memory address> <Time tamp> <Call stack address count> <Call stack address>
    // <Call stack address> ...
    
    // Timestamp.
    TInt64 time = CurrentTime();
    
    // Trace buffer and pointer to it.
    TBufC<KMemFreedBufLength> buffer;
    TPtr ptr( buffer.Des() );
 
    // Append the tag implying a memory allocation line in the data file
    ptr.Append( KMemoryFreedHeader );
    
    // Append the start address of this allocation in the 32-bit (max 8 characters)
    // hexadecimal text format.
    ptr.AppendNum( aMemAddress, EHex );
    
    // Append timestamp;
    ptr.Append( KSpaceTrace );
    ptr.AppendNum( time, EHex);
    
    // Search call stack for address count.
    TInt addrCount(0);
    for ( TInt j = 0; j < aFreeCallstack.Count() ; j++ )
        {
        if ( aFreeCallstack.At(j) == 0 )
            break;
        addrCount++;
        }
    // Current position in call stack.
    TInt addrPos( 0 );
    
    // Append address count.
    ptr.Append( KSpaceTrace );
    ptr.AppendNum( addrCount, EHex );
            
    // Calculate last item length
    TInt lastItemLength( KTraceMessage().Length() + KHexa32Length + 
            KSpaceLength + KNewlineLength );
    
    TUint packetNumber( 1 );
    
    // Go through all call stack's memory addresses associated with
    // this memory allocation 
    for ( TInt j = 0; j < addrCount; j++ )
        {
        // ALLOCF <Memory address> <Time stamp> <Packet number> 
        // <Call stack address> <Call stack address> ...
        if ( ptr.Length() <= 0 )
            {               
            // Create alloc fragment message header
            ptr.Append( KMemoryFreedFragment );
            ptr.AppendNum( aMemAddress, EHex );
            ptr.Append( KSpaceTrace );
            ptr.AppendNum( packetNumber, EHex );
            // Increase packet number
            packetNumber++;
            }
      
        // Append call stack address.
        ptr.AppendFormat( KHexaNumberTrace, aFreeCallstack.At( addrPos ) );
        
        // Move the call stack position.
        addrPos++;
        
        // Check if buffer max length exceed
        if ( lastItemLength + ptr.Length() >= KMemFreedBufLength )
            {
            ptr.Append( KNewLineTrace );
            // Log through debug channel 
            RDebug::Print( KTraceMessage, aProcessId, &buffer );
            // Empty trace buffer
            ptr.Delete( 0, ptr.MaxLength() );
            }
        }
    // Send last message if exists.
    if ( ptr.Length() > 0 )
        {
        ptr.Append( KNewLineTrace );
        RDebug::Print( KTraceMessage, aProcessId, &buffer);
        }
    return KErrNone;   
    }

