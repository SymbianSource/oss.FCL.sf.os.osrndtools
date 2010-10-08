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
// Handler interface for BTrace categories
//

#ifndef __TRACECOREBTRACEHANDLER_H__
#define __TRACECOREBTRACEHANDLER_H__


// Include files
#include "TraceCoreHandler.h"


// Forward declarations
class DTraceCoreWriter;
class DTraceCoreSettings;
class DBTraceCategoryHandler;
class DBTraceOstCategoryHandler;
class DBTraceKernelCategoryHandler;
class DBTracePrintfCategoryHandler;


/**
 * TraceCoreHandler implementation for BTrace
 */
NONSHARABLE_CLASS( DTraceCoreBTraceHandler ) : public DTraceCoreHandler
    {
public:

    /**
     * Constructor
     */
    DTraceCoreBTraceHandler();
    
    /**
     * Destructor
     */
    ~DTraceCoreBTraceHandler();
    
    /**
     * Initializes this handler
     */
    TInt Init();
    
    /**
     * Delegates the writer to category handlers
     *
     * @param aWriter The new writer
     */
    void PrepareSetWriter( DTraceCoreWriter* aWriter );
    
    /**
     * Delegates the writer to category handlers
     *
     * @param aWriter The new writer
     */
    void SetWriter( DTraceCoreWriter* aWriter );
  
    /**
     * Delegates the settings (saver) to category handlers
     */
    void SetSettings( DTraceCoreSettings* aSettings );
  
    /**
     * Registers a category handler. This overwrites the existing category handler for given category
     *
     * @param aCategory The category to be processed with the category handler
     * @param aHandler The handler which processes the category
     */
    void RegisterCategoryHandler( TUint8 aCategory, DBTraceCategoryHandler& aHandler );

    /**
     * Unregisters a category handler
     *
     * @param aCategory The category to be removed
     */
    void UnregisterCategoryHandler( TUint8 aCategory );

private:

    /**
     * Callback function that is registered to BTrace
     *
     * @param aHeader BTrace header
     * @param aHeader2 Extra header data
     * @param aContext The thread context in which this function was called
     * @param a1 The first trace parameter
     * @param a2 The second trace parameter
     * @param a3 The third trace parameter
     * @param aExtra Extra trace data
     * @param aPc The program counter value
     */
    static TBool BTraceHandlerFunc( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
        const TUint32 a1, const TUint32 a2, const TUint32 a3, const TUint32 aExtra, const TUint32 aPc );
    
    /**
     * Gets the category handler for given BTrace header
     *
     * @param aHeader BTrace header
     */
    inline DBTraceCategoryHandler* GetCategoryHandler( TUint32 aHeader );
    
    /**
     * Starts the OST category handler
     */
    TInt StartOstHandler();
    /**
     * Starts the kernel category handler
     */
    TInt StartKernelHandler();

private:

    /**
     * Static instance, accessed from the BTrace handler callback
     */
    static DTraceCoreBTraceHandler* iInstance;

    /**
     * Registered category handlers
     */
    DBTraceCategoryHandler** iCategoryHandlers;
    
    /**
     * Category handler for OST
     */
    DBTraceOstCategoryHandler* iOstHandler;

    /**
     * Category handler for Symbian BTrace categories
     */
    DBTraceKernelCategoryHandler* iKernelHandler;

};

#endif // __TRACECOREBTRACEHANDLER_H__

// End of File
