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
// Base class for trace core handlers.
//

#ifndef __TRACECOREHANDLER_H__
#define __TRACECOREHANDLER_H__


// Include files
#include <kernel/kernel.h>


// Forward declarations
class DTraceCoreSettings;
class DTraceCoreWriter;


/**
 * Base class for trace core handlers. A handler receives trace data and delegates it to writer
 */
NONSHARABLE_CLASS( DTraceCoreHandler ) : public DBase
    {
public:

    /**
     * Constructor
     */
    DTraceCoreHandler();
    
    /**
     * Destructor
     */
    ~DTraceCoreHandler();

    /**
     * Registers this handler to trace core.
     */
    TInt Register();
    
    /**
     * Called before SetWriter with interrupts enabled
     *
     * @param aWriter The new writer
     */
    virtual void PrepareSetWriter( DTraceCoreWriter* aWriter );

    /**
     * Sets the active writer. This is called with interrupts disabled
     *
     * @param aWriter The new writer
     */
    virtual void SetWriter( DTraceCoreWriter* aWriter );

	/**
     * Sets the settings (saver). Called by trace core
     *
     * @param aSettings The new settings
     */
    virtual void SetSettings( DTraceCoreSettings* aSettings );
    
private:

    /**
     * Unregisters this handler from trace core. Called from destructor
     */
    void Unregister();
    
protected:
    
    /**
     * Writer interface
     */
 	DTraceCoreWriter* iWriter;
 	
 	/**
     * Settings interface
     */
 	DTraceCoreSettings* iSettings;
 	
};

#endif

// End of File
