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

/**
@internalComponent
*/

#ifndef __TRACECOREREGISTERWRITER_H__
#define __TRACECOREREGISTERWRITER_H__


// Include files
#include "TraceCoreWriter.h"


class TTraceCoreRegisterWriterProperties
	{
public:
	TUint32 iDataRegister0;
	TUint32 iDataRegister1;
	TUint32 iControlRegister;
	TUint8  iDataStart;
	TUint8  iControlEnd0;
	TUint8  iControlEnd1;
	};


/**
 * Base class for optimized writers that use a register address to write data
 */
class DTraceCoreRegisterWriter : public DTraceCoreWriter
    {
public:
    
    /**
     * Constructor
     */
    /*IMPORT_C*/ DTraceCoreRegisterWriter();
    
    /**
     * Destructor
     */
    /*IMPORT_C*/ ~DTraceCoreRegisterWriter();

    /**
     * Registers this writer to TraceCore. The first writer to register becomes the active writer.
     */
    /*IMPORT_C*/ TInt Register();

    /**
     * Initializes the properties of this writer
     */
    virtual void InitProperties( TTraceCoreRegisterWriterProperties& aProperties ) = 0;
    
    /**
     * Outputs a TraceCore frame. This calls WriteStart, writes the component and group ID's,
     * calls WriteBTraceFrame and calls WriteEnd
     *
     * @param aComponentId the component ID
     * @param aTraceId the trace ID
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
    virtual void WriteTraceCoreFrame( const TUint32 aComponentId, const TUint32 aTraceId, 
            TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, const TUint32 a1, 
            const TUint32 a2, const TUint32 a3, const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize ) = 0;
       
    
    /**
     * Writes 8-bit data to given entry (Time stamped channel -> ends the trace for used channel)
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
	virtual void WriteDataTs( TUint32 aEntryId, TUint8 aData ) = 0;

    /**
     * Writes 16-bit data to given entry (Time stamped channel -> ends the trace for used channel)
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
    virtual void WriteDataTs( TUint32 aEntryId, TUint16 aData ) = 0;

    /**
     * Writes 32-bit data to given entry (Time stamped channel -> ends the trace for used channel)
     * 
     * @param aEntryId the entry ID returned by WriteStart
     * @param aData the trace data
     */
	virtual void WriteDataTs( TUint32 aEntryId, TUint32 aData ) = 0;
    
    /**
     * Outputs a TraceCore frame
     *
     * @param aComponentId the component ID
     * @param aTraceId the trace ID
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
//    void OptimizedWriteTraceCoreFrame( const TUint32 aComponentId, const TUint32 aTraceId, 
//    		TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, const TUint32 a1, 
//    		const TUint32 a2, const TUint32 a3, const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize );
#ifndef __WINS__    		
    /**
     * InterruptsDisabled
     * 
     */
    /*IMPORT_C*/ TUint32 InterruptsDisabled();
#endif
    
protected:
	
    /**
     * Outputs a BTrace frame
     *
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
    virtual void WriteBTraceFrame( TUint32 aWriteParameter, TUint32 aHeader, TUint32 aHeader2, 
    		const TUint32 aContext, const TUint32 a1, const TUint32 a2, const TUint32 a3, 
    		const TUint32 aExtra, const TUint32 aPc, TUint32 aRecordSize ) = 0;

    /**
     * Writes the remaining bytes if data is not 32-bit aligned
     */
    virtual void WriteRemainingBytes( TUint32 aWriteParameter, const TUint32 aSize, const TUint32 a3 ) = 0;

    /**
	 * Write helper functions
	 */
//    inline void LocalWriteData8( TUint32 aWriteParameter, TUint8 aValue, TInt32 aSizeLeftBeforeWrite );
//    inline void LocalWriteData16( TUint32 aWriteParameter, TUint16 aValue, TInt32 aSizeLeftBeforeWrite  );
//    inline void LocalWriteData32( TUint32 aWriteParameter, TUint32 aValue, TInt32 aSizeLeftBeforeWrite  );
    
	/**
	 * Writer properties
	 */
	TTraceCoreRegisterWriterProperties iProperties;
};

#endif

// End of file
