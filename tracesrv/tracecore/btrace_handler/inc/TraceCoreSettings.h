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

#ifndef __TRACECORESETTINGS_H__
#define __TRACECORESETTINGS_H__


// Include files
#include <kernel/kernel.h>


/**
 * Interface for saving and restoring trace activation settings
 */
class DTraceCoreSettings : public DBase
    {
public:

    /**
     * Constructor
     *
     * @param aWriterType Type of writer 
     */
    IMPORT_C DTraceCoreSettings();

    /**
     * Destructor
     */    
    IMPORT_C virtual ~DTraceCoreSettings();

    /**
     * Registers this activation settings saver to TraceCore. Only one saver can register.
     */
    IMPORT_C TInt Register();
    
    /**
     * Write settings
     * 
     * @param aBuffer Buffer to save
     */
    virtual void Write( const TUint8* aBuffer, TUint32 aLength ) = 0;
    
    /**
     * Read settings
     * 
     * @param aBuffer Buffer where to read
     * @return ETrue if buffer copied
     */
    virtual TBool Read( TUint8* aBuffer, TUint32 aLength ) = 0;
    
private:
    
    /**
     * Unregisters this activation settings saver from TraceCore.
     */
    TInt Unregister();
    };

#endif //__TRACECOREACTIVATIONSETTINGSIF_H__

// End of File
