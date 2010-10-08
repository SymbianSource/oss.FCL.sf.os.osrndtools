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
#include <nkern/nk_priv.h> // Symbian internal -> Needed for kernel lock and NThread mutex state

#include "TraceCoreMediaWriter.h" 
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreMediaWriterTraces.h"
#endif



//- Internal definitions ----------------------------------------------------
#ifdef MEDIA_WRITER_STATISTICS

/**
 * Collects statistics about the media writer
 */
class TMediaWriterStatistics
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
TBool DTraceCoreMediaWriter::InitStatistics()
    {
    iStatistics = new TMediaWriterStatistics();
    return iStatistics != NULL;
    }

/**
 * Number of idle timer pings before sending statistics
 */
const TUint KPingsBeforeStatistics = 640;

#else

/**
 * Empty class for statistics if flag is not set
 */
class TMediaWriterStatistics {};

#define STATS( x )
    
#endif


/**
 * Max length of one trace
 */
const TUint32 KMaxBufLength = 256;

/**
 * Number of traces that can be buffered
 */
const TUint32 KBufCount = 2048;

/**
 * Send timer ping frequency in milliseconds
 */
const TUint KTimerPing = 10000;

/**
 * Starts to sleep threads when lots of buffers are in use
 */
const TUint32 KSleepBufLimit = 512;

/**
 * Ticks to sleep when using NKern::Sleep
 */
const TUint32 KSleepTicks = 1;

/**
 * Ticks to sleep when using Kern::NanoWait
 */
const TUint32 KNanoWaitTicks = 10000;

/**
 * Starts to block threads when running out of buffers
 */
const TUint32 KBlockBufLimit = 32;

/**
 * Maximum number of loops to run when blocking
 */
const TInt KMaxBlockLoops = 100;


/**
 * Constructor
 */
EXPORT_C DTraceCoreMediaWriter::DTraceCoreMediaWriter()
: DTraceCoreWriter( EWriterTypeUSBPhonet )
, iSendDfc( DTraceCoreMediaWriter::SendDfc, this, DTraceCore::GetActivationQ()), KDefaultDfcPriority )
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
EXPORT_C DTraceCoreMediaWriter::~DTraceCoreMediaWriter()
    {
    
    // Delete all trace buffers
    for ( int i = 0; i < KBufCount; i++ )
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
EXPORT_C TInt DTraceCoreMediaWriter::Register()
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
            if (memOK)
                {
                iTraceBuffers = new TTraceBuffer[ KBufCount ];
                }
            if ( iTraceBuffers == NULL )
                {
                memOK = EFalse;
                }
            if ( memOK  )
                {
                for ( int i = 0; i < KBufCount && memOK; i++ )
                    {
                    iTraceBuffers[ i ].iBuffer = new TUint8[ KMaxBufLength ];
                    if (iTraceBuffers[ i ].iBuffer == NULL)
                        {
                        memOK = EFalse;
                        }
                    if ( memOK )
                        {
                        // Initially all buffers are linked to iFirstFreeBuffer
                        if ( i < ( KBufCount - 1 ) )
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
                    OstTrace0( TRACE_NORMAL, DTRACECOREMEDIAWRITER_REGISTER_WRITER_REGISTERED,
                    		"DTraceCoreMediaWriter::Register - Media writer registered");
                    iMediaIf = writerIf;
                    }
                }
            else
                {
                OstTrace0( TRACE_IMPORTANT , DTRACECOREMEDIAWRITER_REGISTER_MEM_ALLOC_FAILED,
                		"DTraceCoreMediaWriter::Register - Failed to allocate memory");
                retval = KErrNoMemory;
                }
            }
        }
    OstTrace1( TRACE_BORDER, DTRACECOREMEDIAWRITER_REGISTER_EXIT, "< DTraceCoreMediaWriter::Register. Ret:%d", retval );
    return retval;
    }


/*
 * Timer callback enques the DFC
 * 
 * @param aMediaWriter the media writer
 */
void DTraceCoreMediaWriter::SendTimerCallback( TAny* aMediaWriter )
    {
    DTraceCoreMediaWriter* writer = static_cast< DTraceCoreMediaWriter* >( aMediaWriter );
    writer->SendTimerCallback();
    }


/*
 * Called from the static timer callback function
 */
void DTraceCoreMediaWriter::SendTimerCallback()
    {
    NKern::Lock();
    if ( iFreeBuffers < KBufCount )
        {
        STATS( iStatistics->iIdlePings = 0 );
        
        // Kernel locked, can call DoEnque
        iSendDfc.DoEnque(); 
        }
    else
        {
        STATS( iStatistics->iIdlePings++ );
#ifndef MEDIA_WRITER_STATISTICS
        // The callback timer is stopped unless writing statistics
        iSendTimer.Cancel();
        iSendTimerActive = EFalse;
#endif
        }
    NKern::Unlock();
#ifdef MEDIA_WRITER_STATISTICS
    if ( iStatistics->iIdlePings == KPingsBeforeStatistics && iStatistics->iStatisticsChanged )
        {
        
        // Print out trace statistics
        OstTraceExt4( TRACE_INTERNAL, DTRACECOREMEDIAWRITER_STATISTICS,
        		"DTraceCoreMediaWriter - Statistics. Traces:%u Missed:%u Sent:%u Failed:%u",
        		iStatistics->iTraces, iStatistics->iMissed, iStatistics->iSent, iStatistics->iFailed );
        
        // Print out sleep / wait statistics
        OstTraceExt4( TRACE_INTERNAL, DTRACECOREMEDIAWRITER_SLEEPS,
        		"DTraceCoreMediaWriter - Sleep / wait statistics. Sleeps:%u CannotSleeps:%u Blocks:%u CannotBlocks:%u",
        		iStatistics->iSleepCount, iStatistics->iCannotSleepCount, iStatistics->iBlockCount,
        		iStatistics->iCannotBlockCount );
        
        // Print out timer values
        OstTraceExt4( TRACE_INTERNAL, DTRACECOREMEDIAWRITER_TIMERS,
        		"DTraceCoreMediaWriter - Timer values %d %d %d %d",
        		(TInt)NKern::TickCount(), (TInt)NKern::FastCounter(), (TInt)NKern::TickPeriod(),
        		(TInt)NKern::FastCounterFrequency() );
        
        // Print out misc statistics
        OstTraceExt3( TRACE_INTERNAL, DTRACECOREMEDIAWRITER_MISC,
        		"DTraceCoreMediaWriter - Misc statistics WaitDidntHelp:%u MinFreeBuffers:%u InvalidContextTraces%u",
        		iStatistics->iWaitDidNotHelp, iStatistics->iMinFreeBuffers, iStatistics->iInvalidContextTrace );
        iStatistics->iStatisticsChanged = EFalse;
        }
#endif
    }


/**
 * Starts an entry
 * 
 * @param aType the type of the trace entry
 * @return the entry ID that is passed to other Write-functions
 */
EXPORT_C TUint32 DTraceCoreMediaWriter::WriteStart( TWriterEntryType aType )
    {
    // Detaches the first free buffer from the free buffers list
    TInt context( NKern::CurrentContext() );
    TTraceBuffer* freeBuf = NULL;
    STATS( iStatistics->iStatisticsChanged = ETrue );
    if ( context == NKern::EThread )
        {
        NKern::Lock();
        freeBuf = iFirstFreeBuffer;
        if ( freeBuf != NULL )
            {
            iFirstFreeBuffer = freeBuf->iNext;
            freeBuf->iNext = NULL;
            freeBuf->iMissedBefore = 0;
            iFreeBuffers--;
            STATS( iStatistics->iMinFreeBuffers = ( iFreeBuffers < iStatistics->iMinFreeBuffers )
                    ? iFreeBuffers : iStatistics->iMinFreeBuffers );
            // Timer is activated and stays active until all buffers are empty
            if ( !iSendTimerActive )
                {
                iSendTimer.Periodic( KTimerPing, SendTimerCallback, this );
                iSendTimerActive = ETrue;
                }
            }
        else
            {
            iFirstReadyBuffer->iMissedBefore++;
            STATS( iStatistics->iMissed++ );
            }
        NKern::Unlock();
        if ( freeBuf != NULL )
            {
            StartBuffer( aType, *freeBuf );
            }
        }
    else
        {
        STATS( iStatistics->iInvalidContextTrace++ );
        freeBuf = NULL;
        }
    return ( TUint32 )freeBuf;
    }


/**
 * Ends an entry
 *
 * @param aEntryId the entry ID returned by WriteStart
 */
EXPORT_C void DTraceCoreMediaWriter::WriteEnd( TUint32 aEntryId )
    {
    if ( aEntryId != 0 )
        {
        TTraceBuffer* buf = ( TTraceBuffer* )aEntryId;
        EndBuffer( *buf );
        // If there's no existing ready buffers, the new buffer is set as first.
        // Otherwise, it is assigned to the end of the ready buffers queue
        NKern::Lock();
        if ( iFirstReadyBuffer == NULL )
            {
            iFirstReadyBuffer = buf;
            }
        else
            {
            iLastReadyBuffer->iNext = buf;
            }
        iLastReadyBuffer = buf;
        STATS( iStatistics->iTraces++ );
        NKern::Unlock();
        // Queues the DFC that sends the trace out. The DFC checks the timestamp of the 
        // last trace sent, so the trace might not be immediately sent out.
        iSendDfc.Enque();

        // If the number of buffers is below the sleep limit, this thread is slept for a while.
        // If the number of buffers is below the block limit after sleep, 
        // this thread is slept for longer time
        // Pre-conditions for Sleep:
        //  - Sleeping thread must not be the one sending traces out
        //  - Kernel must not be locked
        //  - Fast mutex must not be held by the thread
        if ( iFreeBuffers < KSleepBufLimit )
            {
            NThread* thread( NKern::CurrentThread() );
            if ( thread != iSenderThread && thread->iHeldFastMutex == NULL && !TScheduler::Ptr()->iKernCSLocked )
                {
                STATS( iStatistics->iSleepCount++ );
                // User-side threads can use NKern::Sleep
#ifdef __WINS__
                TInt type = 0;
#else
                TInt type = NKern::CurrentThread()->UserContextType();
#endif
                Block( type );
                if ( iFreeBuffers < KBlockBufLimit )
                    {
                    STATS( iStatistics->iBlockCount++ );
                    TInt loops = 0;
                    while ( ( iFreeBuffers < KBlockBufLimit ) && ( ++loops ) < KMaxBlockLoops )
                        {
                        iSendDfc.Enque();
                        Block( type );
                        }
                    if ( loops == KMaxBlockLoops )
                        {
                        STATS( iStatistics->iWaitDidNotHelp++ );
                        }
                    }
                }
            else
                {
                STATS( iStatistics->iCannotSleepCount++ );
                if ( iFreeBuffers < KBlockBufLimit )
                    {
                    STATS( iStatistics->iCannotBlockCount++ );
                    }
                }
            }
        }
    }


/**
 * Blocks the current thread for a while
 * 
 * @param aType the context type from NThread::UserContextType
 */
void DTraceCoreMediaWriter::Block( TInt aType )
    {
#ifdef __WINS__
    ( void )aType;
    NKern::Sleep( KSleepTicks );
#else
    if ( aType == NThread::EContextExec )
        {
        NKern::Sleep( KSleepTicks );
        }
    else
        {
        Kern::NanoWait( KNanoWaitTicks );
        }
#endif
    }


/**
 * Writes 8-bit data to given entry
 * 
 * @param aEntryId the entry ID returned by WriteStart
 * @param aData the trace data
 */
EXPORT_C void DTraceCoreMediaWriter::WriteData( TUint32 aEntryId, TUint8 aData )
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
EXPORT_C void DTraceCoreMediaWriter::WriteData( TUint32 aEntryId, TUint16 aData )
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
EXPORT_C void DTraceCoreMediaWriter::WriteData( TUint32 aEntryId, TUint32 aData )
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
void DTraceCoreMediaWriter::SendDfc( TAny* aMediaWriter )
    {
    DTraceCoreMediaWriter* writer = static_cast< DTraceCoreMediaWriter* >( aMediaWriter );
    writer->SendDfc();
    }


/**
 * Called from static DFC function to send a buffer
 */
void DTraceCoreMediaWriter::SendDfc()
    {
    if ( iSenderThread == NULL )
        {
        iSenderThread = NKern::CurrentThread();
        }
    TUint32 time = NKern::FastCounter();
    // Timestamp is checked so that this does not send too frequently
    // Otherwise the USB Phonet Link will crash
    // FastCounter may overflow, so the less than check is also needed
    if ( time > ( iLastTraceSent + iFastCounterBetweenTraces ) || ( time < iLastTraceSent ) )
        {
        // Gets the first buffer that is ready for sending.
        // Assigns the next buffer as first ready buffer
        NKern::Lock();
        TTraceBuffer* buf = iFirstReadyBuffer;
        if ( buf != NULL )
            {
            iFirstReadyBuffer = buf->iNext;
            }
        NKern::Unlock();
        if ( buf != NULL )
            {
            TPtrC8 ptr( buf->iBuffer, buf->iLength );
            TInt res = iMediaIf->SendTrace( ptr );
            // After sending the buffer is moved to the free buffers list
            NKern::Lock();
            if ( res != KErrNone )
                {
                STATS( iStatistics->iFailed++ );
                if ( iFirstReadyBuffer != NULL )
                    {
                    iFirstReadyBuffer->iMissedBefore += buf->iMissedBefore;
                    }
                }
            else
                {
                STATS( iStatistics->iSent++ );
                }
            buf->iNext = iFirstFreeBuffer;
            iFirstFreeBuffer = buf;
            iFreeBuffers++;
            NKern::Unlock();
            iLastTraceSent = time;
            }
        }
    }

// End of File
