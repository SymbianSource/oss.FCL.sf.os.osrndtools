// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Symbian Foundation License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.symbianfoundation.org/legal/sfl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Trace Core
//

#ifndef TRACECOREMEDIASWRITER_H_
#define TRACECOREMEDIASWRITER_H_


// Include files
#include "TraceCoreWriter.h"


// Forward declarations
class DTraceCoreMediaIf;
class TMediaSWriterStatistics;

/**
 * Properties of a single trace
 */
class TTraceBuffer
    {
public:
    TTraceBuffer* iNext;
    TUint8* iBuffer;
    TUint16 iLength;
    TUint16 iMissedBefore;
    TUint8 iType;
    };


/**
 * Writer implementation, which uses the media interface to write data
 */
class DTraceCoreMediaSWriter : public DTraceCoreWriter
    {
public:
    
    /**
     * Constructor
     */
    IMPORT_C DTraceCoreMediaSWriter();
    
    /**
     * Destructor
     */
    IMPORT_C ~DTraceCoreMediaSWriter();
    
    /**
     * Registers this writer to TraceCore
     */
    IMPORT_C TInt Register();
    
    /**
     * Gets the number of traces allowed per second
     */
    virtual TInt GetTraceFrequency() = 0;
    
    /**
     * Initializes a trace buffer
     * 
     * @param aType the entry type
     * @param aBuffer the buffer
     */
    virtual void StartBuffer( TWriterEntryType aType, TTraceBuffer& aBuffer ) = 0;
    
    /**
     * Finishes a trace buffer
     * 
     * @param aBuffer the buffer
     * @return ETrue if trace can be 'dropped'
     */
    virtual TBool EndBuffer( TTraceBuffer& aBuffer ) = 0;
    
private:

    /**
     * Starts an entry
     *
     * @return the entry ID that is passed to other Write-functions
     */
    IMPORT_C TUint32 WriteStart( TWriterEntryType aType );

    /**
     * Ends an entry
     *
     * @param aEntryId the entry ID returned by WriteStart
     */
    IMPORT_C void WriteEnd( TUint32 aEntryId );

    /**
     * Writes 8-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
    IMPORT_C void WriteData( TUint32 aEntryId, TUint8 aData );

    /**
     * Writes 16-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
    IMPORT_C void WriteData( TUint32 aEntryId, TUint16 aData );

    /**
     * Writes 32-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
    IMPORT_C void WriteData( TUint32 aEntryId, TUint32 aData );
    
    /**
     * DFC for sending data
     *  
     * @param aMediaWriter the media writer
     */
    static void SendDfc( TAny* aMediaWriter );
    
    /**
     * Called from the static function to send data
     */
    void SendDfc();
    
    /**
     * Initializes the statistics
     */
    TBool InitStatistics();
    
    
private:
        
    /**
     * Media interface for sending data
     */
    DTraceCoreMediaIf* iMediaIf;
        
    /**
     * DFC for sending data
     */
    TDfc iSendDfc;
        
    /**
     * Timer which is used to send traces
     */
    TTickLink iSendTimer;
    
    /**
     * Flags which tells if the timer is active or not
     */
    TBool iSendTimerActive;
    
    /**
     * Pointer to the list of free trace buffers
     */
    TTraceBuffer* iFirstFreeBuffer;
    
    /**
     * Pointer to the next trace to be sent
     */
    TTraceBuffer* iFirstReadyBuffer;
    
    /**
     * Pointer to the end of list where new traces are added
     */
    TTraceBuffer* iLastReadyBuffer;
    
    /**
     * All trace buffers
     */
    TTraceBuffer* iTraceBuffers;

    /**
     * Number of free buffers
     */
    TUint32 iFreeBuffers;
    
    /**
     * Thread which sends data
     */
    NThread* iSenderThread;

    /**
     * Timestamp of the last trace sent
     */
    TUint32 iLastTraceSent;
    
    /**
     * Media writer statistics
     */
    TMediaSWriterStatistics* iStatistics;
    
    /**
     * Number of FastCounter ticks between traces
     */
    TInt iFastCounterBetweenTraces;
    
    };


#endif /*TRACECOREMEDIASWRITER_H_*/

// End of file
