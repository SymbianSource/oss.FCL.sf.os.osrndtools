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

#ifndef __TRACECOREWRITER_H__
#define __TRACECOREWRITER_H__


// Include files
#include <kernel/kernel.h>


/**
 * Types for WriteStart
 */
enum TWriterEntryType
    {
	
	/**
	 * Ascii printf trace
	 */
    EWriterEntryAscii       = 1,
    
    /**
     * Binary trace
     */
    EWriterEntryTrace       = 2
    };

/**
 * Writer types for constructor
 */
enum TWriterType
    {
	/**
	 * No writer, can be used to remove writer
	 */
    EWriterTypeNone    		= 0,
    
	/**
	 * XTI writer
	 */
    EWriterTypeXTI          = 1,
    
    /**
     * USB phonet writer
     */
    EWriterTypeUSBPhonet    = 2,
  
    /**
     * Wins writer
     */
    EWriterTypeWins    = 3,
    
    /**
     * Custom writer 1
     */
    EWriterTypeCustom1 = 4,
    
    /**
     * Custom writer 2
     */
    EWriterTypeCustom2 = 5,

	/**
     * test writer
     */
    EWriterTypeTest    = 6,
    
    /**
     * Ost Buffer writer
     */
    EWriterTypeOstBuffer = 7
    
    
    };

/**
 * Write parameters for WriteEnd functions.
 * This structure can contain some output channel specific information.
 */
class TWriteEndParams
{
    public:
    TWriteEndParams(TUint32  aEntryId, TBool aWriteZero){ iEntryId = aEntryId; iWriteZero = aWriteZero; };
	TUint32 iEntryId;
	TBool iWriteZero;    
};



/**
 * Base class for trace core writers
 */
class DTraceCoreWriter : public DBase
    {
public:

    /**
     * Constructor
     *
     * @param aWriterType Type of writer 
     */
    IMPORT_C DTraceCoreWriter( TWriterType aWriterType );

    /**
     * Destructor
     */    
    IMPORT_C virtual ~DTraceCoreWriter();

    /**
     * Registers this writer to TraceCore. The first writer to register becomes the active writer.
     */
    IMPORT_C virtual TInt Register();

    /**
     * Outputs a TraceCore frame. This calls WriteStart, writes the component and trace ID's,
     * calls WriteBTraceFrame and calls WriteEnd
     *
     * @param aComponentId the component ID
     * @param aTraceWord The trace word containing the group ID and the Trace ID for the frame
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
    IMPORT_C virtual void WriteTraceCoreFrame( const TUint32 aComponentId, const TUint32 aTraceId, 
    		TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, const TUint32 a1, 
    		const TUint32 a2, const TUint32 a3, const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize );

    /**
     * Ends an entry. 
	 * This function should be overwritten in writers that derive from DTraceCoreWriter 
	 * and require non standard implementation.
     *
     * @param aWriteEndParams structure containing 'end of message' information. @see TWriteEndParams
     */
    IMPORT_C virtual void WriteEnd( const TWriteEndParams& aWriteEndParams );
    
    
    //TODO: assert that BtraceLock is held in Write Start!!!!
    //TODO: Write WritePreStart() or something
    
    /**
     * Starts an entry
     *
     * @return the entry ID that is passed to other Write-functions
     */
    virtual TUint32 WriteStart( TWriterEntryType aType ) = 0;

    /**
     * Ends an entry
     *
     * @param aEntryId the entry ID returned by WriteStart
     */
    virtual void WriteEnd( TUint32 aEntryId ) = 0;



    /**
     * Writes 8-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
	virtual void WriteData( TUint32 aEntryId, TUint8 aData ) = 0;

    /**
     * Writes 16-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
    virtual void WriteData( TUint32 aEntryId, TUint16 aData ) = 0;

    /**
     * Writes 32-bit data to given entry
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
	virtual void WriteData( TUint32 aEntryId, TUint32 aData ) = 0;
	
	/**
	 * Gets the type of this writer
	 */
	TWriterType GetWriterType();
	

	/**
	 * Signal to TraceCore if there has been a dropped trace
	 * 
     * @param aDropped ETrue if the writer dropped the trace - otherwise EFalse
     * 
	 */
    IMPORT_C void TraceDropped(TBool aDropped);




    /**
      * Interrogates tracecore if the last trace was dropped
      * 
      * @return  returns ETrue if the trace was dropped otherwise EFalse
      * 
    */

    IMPORT_C TBool WasLastTraceDropped() const ;

    
    /**
     * Checks whether writer can handle incoming trace data with specific size.
     * This method is useful in case where Trace Core must report dropped traces.
     * Currently it is used only in printf-handler and only in case where 
     * the "dropped trace" flag is set.
     * Typical implementation of this method should check writer internal state and answer
     * whether trace with specific size and type can be sent.
     * Default implementation of this method assumes writer is able to handle incoming 
     * trace (ETrue is returned).
     * 
     * @param aTraceSize Size of incomming trace.
     * @return ETrue if writer state is correct and it is able to send incomming data; EFalse otherwise.
     */
    IMPORT_C virtual TBool AbleToWrite(TUint aTraceSize); 

private:
    
    /**
     * Outputs a BTrace frame
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
    void WriteBTraceFrame( const TUint32 aEntryId, TUint32 aHeader, TUint32 aHeader2, 
    		const TUint32 aContext, const TUint32 a1, const TUint32 a2, const TUint32 a3, 
    		const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize );

    /**
     * Unregisters this writer from TraceCore. Called from destructor
     */
    void Unregister();

    /**
     * Writes the remaining bytes if data is not 32-bit aligned
     */
    void WriteRemainingBytes( TUint32 aEntryId, TUint32 aSize, TUint32 a3 );

    /**
     * Writer type
     */
    TWriterType iWriterType;
    };

#endif

// End of File
