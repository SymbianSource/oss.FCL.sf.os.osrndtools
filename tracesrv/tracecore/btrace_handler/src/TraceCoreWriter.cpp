// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Trace Core
// 

#include "TraceCoreWriter.h"
#include "TraceCore.h"
#include "TraceCoreDebug.h"

#include "TraceCoreConstants.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreWriterTraces.h"
#endif

#ifdef WRITE_OST_HEADER

const TUint KDWordShift = 32;
const TUint KDWordMask = 0xFFFFFFFF;
const TUint KNanoSeconds = 1000000000;

/**
 * Musti timestamp mask. First 4 bits are reserved for flags
 */
const TUint KTimestampMask = 0x0FFFFFFF;

/**
 * Musti timestamp flags for XTIv2
 */
const TUint KTimestampFlags = 0xD0000000;

const TUint KComponentIdLength(4);
const TUint KGroupIdLength(4);
const TUint KTimestampLenght(8);

#endif // WRITE_OST_HEADER

/**
 * Constructor
 */
EXPORT_C DTraceCoreWriter::DTraceCoreWriter( TWriterType aWriterType )
: iWriterType( aWriterType )
    {
    }


/**
 * Destructor
 */
EXPORT_C DTraceCoreWriter::~DTraceCoreWriter()
    {
    Unregister();
    }


/**
 * Registers this writer to TraceCore
 */
EXPORT_C TInt DTraceCoreWriter::Register()
    {
    TInt ret= KErrGeneral;  
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        // Register this writer
        ret = traceCore->RegisterWriter( *this );
        }
    OstTrace1( TRACE_BORDER, DTRACECOREWRITER_REGISTER_EXIT, "< DTraceCoreWriter::Register %d", ret );
    return ret;
    }


/**
 * Unregisters this writer from TraceCore
 */
void DTraceCoreWriter::Unregister()
    {
    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        // Unregister this writer
        traceCore->UnregisterWriter( *this );
        }
    }


/**
 * Gets the writer type
 */
TWriterType DTraceCoreWriter::GetWriterType()
    {
    return iWriterType;
    }
    
    
/**
 * Outputs a TraceCore frame. This calls WriteStart, writes the component and group ID's,
 * calls WriteBTraceFrame and calls WriteEnd.
 * 
 * Tracing is not allowed from this method.
 *
 * @param aComponentId the component ID
 * @param aTraceWord The trace word containing the group ID and the trace ID to write
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 The first trace parameter
 * @param a2 The second trace parameter
 * @param a3 The third trace parameter
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @param aRecordSize The record size
 */
EXPORT_C void DTraceCoreWriter::WriteTraceCoreFrame( const TUint32 aComponentId, const TUint32 aTraceWord, 
        TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, const TUint32 a1, 
        const TUint32 a2, const TUint32 a3, const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize )
    {    
    //TODO: tidy up code from #ifdef's
    
#ifdef USE_OPTIMIZED_WRITE
	if ( iWriterType == EWriterTypeXTI ) 
#else
    if ( false )
#endif
		{
		}
	else
		{
	    TUint32 entryId = WriteStart( EWriterEntryTrace );

#ifdef WRITE_OST_HEADER
	if ( iWriterType == EWriterTypeXTI )
	    {
        // Size is in the beginning of the 
        TUint32 size = aRecordSize + KTimestampLenght + KComponentIdLength + KGroupIdLength; //+(aHeader & 0xff)
    #ifdef AUTOGEN_ADD_BTRACE_TIMESTAMP
        size += KTimestampSize;
    #endif
    
        // Decrease component, group and trace ID (8 bytes) from the size as they are also included in the BTrace
        // variables and they won't be written in the BTrace packet anymore
        size -= ( KA1Size + KA2Size );
        
        //  version(0x05);    // OST Base Protocol version "1.0" (v00-80-00_r1-04.pdf)
        //  entityId(0x01);   // Entity id TODO: check if this needs to change with CPU
        //  protocolId(0x03); // Simple Application Trace Protocol (Not in MIPI specs 08 yet)
        
        WriteData( entryId, ( TUint8 )0x05 ); // Version "0.5" because BTrace header included with protocol id 0x03
        WriteData( entryId, ( TUint8 )0x01 ); // EntityId
        WriteData( entryId, ( TUint8 )0x03 ); // ProtocolId (not specified in MIPI specs yet)
        
        // Set length
        // If write size less than 256
        if(size < 256 )
            {
            //  length(size);
            WriteData( entryId, ( TUint8 )size );
            
            }
        else
            {
            //  extendedLengthBits0_7(0x00);
            //  extendedLengthBits8_15(0x00);
            //  extendedLengthBits16_23(0x00);
            //  extendedLengthBits24_31(0x00);
            WriteData( entryId, ( TUint8 )0x00 ); // length field 0 if extended length in use
            //WriteData( entryId, ( TUint32 )size );// No swap, Length in protocol is little endian //SWAP_DATA( size ) );
            // Swap needed after all to make little endian in XTI trace??
            WriteData( entryId, ( TUint32 )SWAP_DATA( size ) );
            }
            
        // Write timestamp
        TUint64 timestamp = NKern::FastCounter();
        timestamp = (timestamp * KNanoSeconds ) / NKern::FastCounterFrequency();
        
        TUint32 timestampLSB = timestamp & KDWordMask;
        TUint32 timestampMSB = ( (timestamp >> KDWordShift ) & KTimestampMask ) | KTimestampFlags;
        
        WriteData( entryId, SWAP_ID( timestampMSB ) );
        WriteData( entryId, SWAP_ID( timestampLSB ) );
	    }
        
    // Write Component  and groupid (including traceid )
    WriteData( entryId, SWAP_ID( aComponentId ) );
    WriteData( entryId, SWAP_ID( aTraceWord ) );          
        
#else // WRITE_OST_HEADER 
    WriteData( entryId, SWAP_ID( aComponentId ) );
    WriteData( entryId, SWAP_ID( aTraceWord ) );
#endif
    WriteBTraceFrame( entryId, aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc, aRecordSize );
    
    WriteEnd( entryId );
		}
		
    }


/**
 * Outputs a BTrace frame into this writer.
 * Tracing is not allowed from this method.
 *
 * @param aEntryId the entry ID returned by WriteStart
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 The first trace parameter
 * @param a2 The second trace parameter
 * @param a3 The third trace parameter
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @param aRecordSize The record size
 */
void DTraceCoreWriter::WriteBTraceFrame( const TUint32 aEntryId, TUint32 aHeader, TUint32 aHeader2, 
        const TUint32 aContext, const TUint32 a1, const TUint32 a2, const TUint32 a3, 
        const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize )
    {
    // BTrace frame header. Don't read size from the header because Multipart trace can bigger than can fit to one byte.
    TUint32 size = aRecordSize;
    TUint8 flags = static_cast< TUint8 >( ( aHeader >> ( BTrace::EFlagsIndex * KByteSize ) ) & KByteMask );
    TUint8 category = static_cast< TUint8 >( ( aHeader >> ( BTrace::ECategoryIndex * KByteSize ) ) & KByteMask );
    TUint8 subcategory = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESubCategoryIndex * KByteSize ) ) & KByteMask );
    
    #ifdef __SMP__
	// Header 2 always present and contains CPU number
	// If Header2 not originally there, add 4 to size
	if (!( flags & BTrace::EHeader2Present))
		{
        flags |= BTrace::EHeader2Present;
        aHeader2 = 0;
        size += KHeader2Size;
	    }
	aHeader2 = (aHeader2 &~ BTrace::ECpuIdMask) | (NKern::CurrentCpu()<<20); 
	#endif
    
    // If timestamp is added to frame, the timestamp flag is also set to the header
    // and the header size is increased by the size of the timestamp
#ifdef AUTOGEN_ADD_BTRACE_TIMESTAMP
	if (!( flags & BTrace::ETimestampPresent))
	    {
	    size += KTimestampSize;
	    flags |= BTrace::ETimestampPresent;
	    }
#endif

    // In case of autogen and OST categories, the BTrace frame size needs to be adjusted
    // depending on whether the group / trace ID info is replicated into the
    // frame or not.
    if ( category == KCategoryNokiaAutogen )
        {
        // Group and trace ID's are not written -> Skip a1
        size -= KA1Size;
        }
        
    // Same applies to OST categories
    else if (category >= KMaxKernelCategory    && // category <= KMaxCategory && // Not needed since category is a TUint8
       	     category != KCategoryNokiaBranchCoverage)
        {
        // Component, group and trace ID's from a1 and a2 are skipped
        size -= ( KA1Size + KA2Size );
        }
    else
        {
        // Other categories do not have component / group / trace ID's
        // -> No flagging is needed here
        }

    // Insert possibly changed values to BTrace header
    TUint8 sizeToHeader = size;
    if (size >= KByteMask )
        {
        sizeToHeader = KByteMask;
        }
    
    aHeader = ( sizeToHeader << ( BTrace::ESizeIndex * KByteSize ) )
            | ( flags << ( BTrace::EFlagsIndex * KByteSize ) )
            | ( category << ( BTrace::ECategoryIndex * KByteSize ) )
            | ( subcategory << ( BTrace::ESubCategoryIndex * KByteSize ) );
    
    // Writes the header
    WriteData( aEntryId, SWAP_DATA( aHeader ) );
    size -= KHeaderSize; // Subtract header size
    
    if ( flags & BTrace::EHeader2Present )
        {
        WriteData( aEntryId, SWAP_DATA( aHeader2 ) );
    	size -= KHeader2Size;
        }

#ifdef AUTOGEN_ADD_BTRACE_TIMESTAMP
    // Timestamp is written after header2
    TUint32 timestamp = NKern::FastCounter();
    WriteData( aEntryId, SWAP_DATA( timestamp ) );
    size -= KTimestampSize;
#endif

    if ( flags & BTrace::EContextIdPresent )
        {
        WriteData( aEntryId, SWAP_DATA( aContext ) );
        size -= KContextIdSize;
        }
    if ( flags & BTrace::EPcPresent )
        {
        WriteData( aEntryId, SWAP_DATA( aPc ) );
        size -= KPcSize;
        }
    if ( flags & BTrace::EExtraPresent )
        {
        WriteData( aEntryId, SWAP_DATA( aExtra ) );
        size -= KExtraSize;
        }
    // If A1 is present, it is written
    if ( size >= KA1Size )
        {
        if ( category == KCategoryNokiaAutogen )
            {
            // Don't write anything
            }
        else if (category >= KMaxKernelCategory    && // category <= KMaxCategory && // Not needed since category is a TUint8
           	     category != KCategoryNokiaBranchCoverage)
            {
            // Don't write anything
            }
        else
            {
            WriteData( aEntryId, SWAP_DATA( a1 ) );
            size -= KA1Size;
            }
    
        // If A2 is present, it is written
        if ( size >= KA2Size )
            {
            if (category >= KMaxKernelCategory    && // category <= KMaxCategory && // Not needed since category is a TUint8
                category != KCategoryNokiaBranchCoverage)
                {
                // In OST, group / trace ID is in a2 -> Not written
                }
            else
                {
                WriteData( aEntryId, SWAP_DATA( a2 ) );
                size -= KA2Size;
                }
            
            // If there is 4 bytes of data left, A3 is written as is
            // If more, the data is read from buffer pointed by A3
            if ( size <= KA3Size && size > 0 )
                {
                WriteData( aEntryId, SWAP_DATA( a3 ) );
                }
            else if ( size > 0 )
                {
                WriteRemainingBytes( aEntryId, size, a3 );
                }
            }
        }
    }

    
/**
 * Writes the remaining bytes if data is not 32-bit aligned
 */
void DTraceCoreWriter::WriteRemainingBytes( TUint32 aEntryId, TUint32 aSize, TUint32 a3 )
    {
    TUint8 extra = aSize % 4; // CodForChk_Dis_Magic
    TUint32* ptr = ( TUint32* )a3;
    TUint32* end = ( TUint32* )( ( ( TUint8* )a3 ) + ( aSize - extra ) );
    TUint32 val;
    while( ptr < end )
        {
        val = *ptr++;
        WriteData( aEntryId, SWAP_DATA( val ) );
        }
    // Write remaining bytes and aligns to 32-bit boundary
    if ( extra > 0 )
        {
        TUint8* ptr8 = ( TUint8* )ptr;
        TUint8* end8 = ptr8 + extra;
        TUint8* alignEnd = ( TUint8* )( end + 1 );
        while ( ptr8 < alignEnd )
            {
            if ( ptr8 < end8 )
                {
                WriteData( aEntryId, *ptr8++ );
                }
            else
                {
                WriteData( aEntryId, ( TUint8 )0 );
                ptr8++;
                }
            }
        }
    }

   
EXPORT_C void DTraceCoreWriter::WriteEnd( const TWriteEndParams& aWriteEndParams )
    {
    WriteEnd( aWriteEndParams.iEntryId );
    }


/**
 * Signal to TraceCore if there has been a dropped trace
 * 
 * @param aDropped ETrue if the writer dropped the trace - otherwise EFalse
 * 
 */

EXPORT_C void DTraceCoreWriter::TraceDropped(TBool aDropped)
    {
    DTraceCore* tracecore = DTraceCore::GetInstance();
    tracecore->SetPreviousTraceDropped(aDropped);
    }


/**
  * Interrogates tracecore if the last trace was dropped
  * 
  * @return  returns ETrue if the trace was dropped otherwise EFalse
  * 
  */
EXPORT_C TBool DTraceCoreWriter::WasLastTraceDropped() const 
    {
    TBool ret = EFalse;
    DTraceCore* tracecore = DTraceCore::GetInstance();
    ret = tracecore->PreviousTraceDropped();
    return ret;
    }



EXPORT_C TBool DTraceCoreWriter::AbleToWrite(TUint /*aTraceSize*/)
    {
    return ETrue;
    }
   
// End of File
