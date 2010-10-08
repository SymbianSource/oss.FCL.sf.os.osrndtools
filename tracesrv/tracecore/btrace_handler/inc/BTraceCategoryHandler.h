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
// Base-class handler interface for BTrace categories 
//

#ifndef __BTRACECATEGORYHANDLER_H__
#define __BTRACECATEGORYHANDLER_H__


// Include files
#include <kernel/kernel.h>


// Forward declarations
class DTraceCoreWriter;
class DTraceCoreSettings;

/**
 * Saved activation item for multipart traces
 */
class TMultiPartActivationInfo
    {
public:

    /**
     * Constructor sets member variables to zero
     */
    TMultiPartActivationInfo()
        {
        iComponentId = 0;
        iTraceWord = 0;
        iMultiPartId = 0;
        }
    
    TUint32 iComponentId;
    TUint32 iTraceWord;
    TUint32 iMultiPartId;
    };

/**
 * Category-specific handler interface which can be registered to DTraceCoreBTraceHandler.
 */
class DBTraceCategoryHandler : public DBase
    {
public:

    /**
     * Constructor
     */
    IMPORT_C DBTraceCategoryHandler();

    /**
     * Destructor
     */
    IMPORT_C ~DBTraceCategoryHandler();
    
    /**
     * Registers the categories to BTrace handler
     */
    IMPORT_C TInt Register();
    
    /**
     * Called before SetWriter with interrupts enabled
     *
     * @param aWriter The writer to be used
     */
    IMPORT_C virtual void PrepareSetWriter( DTraceCoreWriter* aWriter );
    
    /**
     * Sets the active writer.  This is called with interrupts disabled
     *
     * @param aWriter The writer to be used
     */
    IMPORT_C virtual void SetWriter( DTraceCoreWriter* aWriter );
    
    /**
     * Sets the active settings.
     *
     * @param aSettings The settings to be used
     */
    IMPORT_C virtual void SetSettings( DTraceCoreSettings* aSettings );

    /**
     * Category-specific handler function.
     *
     * @param aHeader BTrace header
     * @param aHeader2 Extra header data
     * @param aContext The thread context in which this function was called
     * @param a1 The first trace parameter
     * @param a2 The second trace parameter
     * @param a3 The third trace parameter
     * @param aExtra Extra trace data
     * @param aPc The program counter value
     * @return ETrue if trace was processed, EFalse if not
     */
    virtual TBool HandleFrame( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                               const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                               const TUint32 aExtra, const TUint32 aPc ) = 0;

    /**
     * Adds a category to the list of categories to be registered
     *
     * @param aCategory The category to be registered
     */
    IMPORT_C TInt AddCategory( TUint8 aCategory );

private:

    /**
     * Unregisters from BTrace handler. Called from destructor
     */
    void Unregister();

protected:

    /**
     * Currently active writer
     */
    DTraceCoreWriter* iWriter;
    
    /**
     * Currently active settings
     */
    DTraceCoreSettings* iSettings;

private:

    /**
     * The categories of this handler
     */
    RArray< TUint8 > iCategories;
    };

#endif

// End of File
