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

#include <kernel/kernel.h>

#include "TraceCoreMediaSWriter.h"
#include "TraceCoreMediaIf.h"
#include "TraceCoreConstants.h"
#include "TraceCore.h"
#include "TraceCoreSendReceive.h"
#include "TraceCoreDebug.h"


//- Internal definitions ----------------------------------------------------
#ifdef MEDIA_WRITER_STATISTICS

/**
 * Collects statistics about the media writer
 */
class TMediaSWriterStatistics
    {
public:
    TUint32 iTraces;                // Number of traces received
    TUint32 iSent;                  // Number of traces sent
    TUint32 iFailed;                // Number of traces failed to send
    TUint32 iMissed;                // Number of traces missed due to not enough buffers
    TUint32 iSleepCount;            // Number of times the threads have slept waiting for buffers
    TUint32 iCannotSleepCount;      // Number of times a thread could not be slept when it should
    TUint32 iBlockCount;            // Number of times a thread has been blocked waiting for buffers
    TUint32 iCannotBlockCount;      // Number of times a thread could not be blocked when it should
    TUint32 iWaitDidNotHelp;        // Number of waits that did not help
    TUint32 iMinFreeBuffers;        // Minimum number of free buffers since boot
    TUint32 iInvalidContextTrace;   // Number of IRQ / IDFC traces ignored
    TUint32 iIdlePings;             // Statistics are written after some idle time
    TBool iStatisticsChanged;       // Flags indicating change in statistics
    };

/**
 * Macro to wrap statistics-related code
 */
#define STATS( x ) x

/**
 * Initializes the statistics structure
 */
TBool DTraceCoreMediaSWriter::InitStatistics()
    {
    iStatistics = new TMediaSWriterStatistics();
    return iStatistics != NULL;
    }

#else

/**
 * Empty class for statistics if flag is not set
 */
class TMediaSWriterStatistics {};

#define STATS( x )

#endif


/**
 * Max length of one trace
 */
const TUint32 KMaxBufLength = 256;

/**
 * Number of traces that can be buffered
 */
const TUint32 KBufCount = 1;

/**
 * Constructor
 */
EXPORT_C DTraceCoreMediaSWriter::DTraceCoreMediaSWriter()
: DTraceCoreWriter( EWriterTypeUSBPhonet )
, iSendDfc( DTraceCoreMediaSWriter::SendDfc, this, DTraceCore::GetActivationQ(), KDefaultDfcPriority )
, iSendTimerActive( EFalse )
, iFirstFreeBuffer( NULL )
, iFirstReadyBuffer( NULL )
, iLastReadyBuffer( NULL )
, iTraceBuffers( NULL )
, iFreeBuffers( KBufCount )
, iSenderThread( NULL )
, iLastTraceSent( 0 )
, iStatistics( NULL )
    {
    // No implementation in constructor
    }


/**
 * Destructor
 */
EXPORT_C DTraceCoreMediaSWriter::~DTraceCoreMediaSWriter()
    {
    // Delete all trace buffers
    for ( TUint i = 0; i < KBufCount; i++ )
        {
        delete[] iTraceBuffers[ i ].iBuffer;
        }

    delete[] iTraceBuffers;
    delete iStatistics;
    }


/**
 * Registers this writer to TraceCore
 *
 * @return KErrNone if successful
 */
EXPORT_C TInt DTraceCoreMediaSWriter::Register()
    {
    // Media writer uses the media interface from SendReceive
    TInt retval( KErrGeneral );

    // Get TraceCore
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {

        // Get MediaWriter interface
        DTraceCoreMediaIf* writerIf = traceCore->GetSendReceive().GetMediaWriterIf();
        if ( writerIf != NULL )
            {
            // Allocates memory for the trace buffers
            TBool memOK = ETrue;
            STATS( memOK = InitStatistics() );
            if(memOK)
                {
                iTraceBuffers = new TTraceBuffer[ KBufCount ];
                }
            if (iTraceBuffers == NULL)
                {
                memOK=EFalse;
                }
            if ( memOK )
                {
                for ( TUint i = 0; i < KBufCount && memOK; i++ )
                    {
                    iTraceBuffers[ i ].iBuffer = new TUint8[ KMaxBufLength ];
                    if (iTraceBuffers[ i ].iBuffer == NULL )
                        {
                        memOK = EFalse;
                        }
                    if ( memOK )
                        {
                        // Initially all buffers are linked to iFirstFreeBuffer
                        // Casting needed to work around armv5 "pointless comparison of unsigned integer with zero" warning
                        if ( (TInt) i < (TInt) ( KBufCount - 1 ) )
                            {
                            iTraceBuffers[ i ].iNext = &( iTraceBuffers[ i + 1 ] );
                            }
                        }
                    else
                        {
                        memOK = EFalse;
                        }
                    }
                }
            else
                {
                memOK = EFalse;
                }

            // Memory was allocated succesfully
            if ( memOK )
                {
                iFastCounterBetweenTraces = NKern::FastCounterFrequency() / GetTraceFrequency();
                iFirstFreeBuffer = iTraceBuffers;

                // Register to the TraceCore
                retval = DTraceCoreWriter::Register();
                if ( retval == KErrNone )
                    {
                    iMediaIf = writerIf;
                    }
                }
            else
                {
                retval = KErrNoMemory;
                }
            }
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf("<DTraceCoreMediaSWriter::Register - %d", retval) );
    return retval;
    }


/**
 * Starts an entry
 *
 * @param aType the type of the trace entry
 * @return the entry ID that is passed to other Write-functions
 */
EXPORT_C TUint32 DTraceCoreMediaSWriter::WriteStart( TWriterEntryType aType )
    {

    // Detaches the first free buffer from the free buffers list
    TInt context( NKern::CurrentContext() );
    TTraceBuffer* freeBuf = NULL;

    if ( context == NKern::EThread || context == NKern::EIDFC )
        {
#ifndef __SMP__
        // In non SMP env the __SPIN_LOCK macro is empty. We need Kern::Lock.
		NKern::Lock(); // Unlocked in WriteEnd
#else
		//already locked by BTrace spin lock
#endif

		freeBuf = iFirstFreeBuffer;

        if ( freeBuf != NULL )
            {
            StartBuffer( aType, *freeBuf );
            }
        }
    else
        {
        freeBuf = NULL;
        }

    return ( TUint32 )freeBuf;
    }


/**
 * Ends an entry
 *
 * @param aEntryId the entry ID returned by WriteStart
 * @param aWriteEndingZero If ETrue, write ending zero.
 */
EXPORT_C void DTraceCoreMediaSWriter::WriteEnd( TUint32 aEntryId  )
    {

    if ( aEntryId != 0 )
        {
        TTraceBuffer* buf = ( TTraceBuffer* )aEntryId;
        TBool drop = EndBuffer( *buf );

        if( !drop ) //trace isn't dropped
            {
            iSendDfc.RawAdd();
            }

        // Locked in WriteStart
#ifndef __SMP__
        // In non SMP the __SPIN_LOCK macro is empty. We need Kern::Unlock.
		NKern::Unlock(); // Locked in WriteStart
#else
		//BTrace lock is used from outside of this method.
#endif
        }
    }


/**
 * Writes 8-bit data to given entry
 *
 * @param aEntryId the entry ID returned by WriteStart
 * @param aData the trace data
 */
EXPORT_C void DTraceCoreMediaSWriter::WriteData( TUint32 aEntryId, TUint8 aData )
    {
    if ( aEntryId != 0 )
        {
        TTraceBuffer* buf = ( TTraceBuffer* )aEntryId;
        if ( buf->iLength < KMaxBufLength )
            {
            *( buf->iBuffer + buf->iLength ) = aData;
            buf->iLength++;
            }
        }
    }


/**
 * Writes 16-bit data to given entry
 *
 * @param aEntryId the entry ID returned by WriteStart
 * @param aData the trace data
 */
EXPORT_C void DTraceCoreMediaSWriter::WriteData( TUint32 aEntryId, TUint16 aData )
    {
    if ( aEntryId != 0 )
        {
        TTraceBuffer* buf = ( TTraceBuffer* )aEntryId;
        if ( buf->iLength + sizeof ( TUint16 ) <= KMaxBufLength )
            {
            *( ( TUint16* )( buf->iBuffer + buf->iLength ) ) = aData;
            buf->iLength += sizeof ( TUint16 );
            }
        }
    }


/**
 * Writes 32-bit data to given entry
 *
 * @param aEntryId the entry ID returned by WriteStart
 * @param aData the trace data
 */
EXPORT_C void DTraceCoreMediaSWriter::WriteData( TUint32 aEntryId, TUint32 aData )
    {
    if ( aEntryId != 0 )
        {
        TTraceBuffer* buf = ( TTraceBuffer* )aEntryId;
        if ( buf->iLength + sizeof ( TUint32 ) <= KMaxBufLength )
            {
            *( ( TUint32* )( buf->iBuffer + buf->iLength ) ) = SWAP_DATA( aData );
            buf->iLength += sizeof ( TUint32 );
            }
        }
    }


/**
 * DFC for sending data
 *
 * @param aMediaWriter the media writer
 */
void DTraceCoreMediaSWriter::SendDfc( TAny* aMediaWriter )
    {
    DTraceCoreMediaSWriter* writer = static_cast< DTraceCoreMediaSWriter* >( aMediaWriter );
    writer->SendDfc();
    }


/**
 * Called from static DFC function to send a buffer
 */
void DTraceCoreMediaSWriter::SendDfc()
    {
    // Create and send note random values
    TBuf8<64> note; // CodForChk_Dis_Magic
    note.Append(0xAE); // Dont care, CodForChk_Dis_Magic
    note.Append(0xAE); // Dont care, CodForChk_Dis_Magic
    note.Append(0x00); // Lentght MSB, CodForChk_Dis_Magic
    note.Append(0x02); // Length LSB, CodForChk_Dis_Magic
    note.Append(0xED); // Dont care, CodForChk_Dis_Magic
    note.Append(0xED); // Dont care, CodForChk_Dis_Magic

    iMediaIf->SendTrace( note );
    }

// End of File
