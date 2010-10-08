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
// Handler for BTrace kernel categories 
//

#ifndef __BTRACEKERNELCATEGORYHANDLER_H__
#define __BTRACEKERNELCATEGORYHANDLER_H__


// Include files
#include <kernel/kernel.h>
#include "BTraceCategoryHandler.h"
#include "TraceCoreActivation.h"


// Forward declarations
class DBTraceOstCategoryBitmap;


/**
 * Category handler for Symbian BTrace categories.
 */
NONSHARABLE_CLASS( DBTraceKernelCategoryHandler ) : public DBTraceCategoryHandler, public MTraceCoreActivationNotification
    {
public:

    /**
     * Constructor
     */
    DBTraceKernelCategoryHandler();
     
     /**
      * Destructor
      */
    ~DBTraceKernelCategoryHandler();
        
    /**
     * Initializes this handler
     * 
     * @return one of the standard error codes
     */
    TInt Init();

    /**
     * Called before writer is changed
     *
     * @param aWriter The new writer
     */
    void PrepareSetWriter( DTraceCoreWriter* aWriter );
    
	/**
 	 * Primes the BTrace kernel categories    
 	 */
	void PrimeKernelCategories();

private:

    /**
     * Notification that the activation state has changed
     * 
     * @param aActivation the activation interface that was updated
     * @param aFromSettings ETrue if activation was due to settings read, EFalse if from some other source
     * @param aComponentId Component ID of the activation
     */
    void ActivationChanged( MTraceCoreActivation& aActivation, TBool aFromSettings,
            TUint32 aComponentId);
    
    /**
     * MTraceCoreBTraceCategoryHandler implementation for Symbian-specific BTrace categories
     *
     * @param aHeader BTrace header
     * @param aHeader2 Extra header data
     * @param aContext The thread context in which this function was called
     * @param a1 The first trace parameter
     * @param a2 The second trace parameter
     * @param a3 The third trace parameter
     * @param aExtra Extra trace data
     * @param aPc The program counter value
     * @return ETrue if trace was processed
     */
    TBool HandleFrame( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                       const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                       const TUint32 aExtra, const TUint32 aPc );
    
    /**
     * Maps the BTrace category / sub-category to group / trace ID
     * 
     * @param aCategory The BTrace category
     * @param aSubCategory The BTrace sub-categoory
     * @return the group / trace ID combination
     */
    TUint32 MapCategoryToID( TUint8 aCategory, TUint8 aSubCategory );
    
    /**
     * Dfc function to prime the kernel categories
     * 
     * @param aHandler pointer to this handler
     */
    static void PrimeDfc( TAny* aHandler );
    
    /**
     * Called from the static DFC function to prime kernel categories
     */
    void PrimeDfc();

    /**
     * Primes a category
     * 
     * @param aCategory the category to be primed
     */
    void PrimeCategory( TUint8 aCategory );
    
private:

    /**
     * Handles this Multipart trace
     *
     * @param aHeader BTrace header
     * @param aHeader2 Extra header data
     * @param aContext The thread context in which this function was called
     * @param aTotalMessageLength Total message length of this multipart message 
     * @param aComponentId Component ID
     * @param aData The data
     * @param aExtra Extra trace data
     * @param aPc The program counter value
     */
    TBool HandleMultiPart( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext,
                       const TUint32 aTotalMessageLength, const TUint32 aComponentId,
                       const TUint32 aData, const TUint32 aExtra, const TUint32 aPc );
    
    /**
     * Checks if given trace is a Multipart trace
     *
     * @param aHeader Header data
     * @param aHeader2 Extra header data
     */
    inline TBool CheckMultiPart( TUint32 aHeader, TUint32 aHeader2 );
    
    /**
     * Calculates data start offset
     *
     * @param aHeader BTrace header
     */
    TUint32 CalculateDataStartOffset( TUint32 aHeader );    
    
    /**
     * This uses the same bitmap as the OST category handler
     */
    DBTraceOstCategoryBitmap* iTraceBitmap;
    
    /**
     * Dfc used when priming kernel categories
     */
    TDfc iPrimeDfc;
    
    /**
     * Saved activation item for multipart traces
     */ 
    RArray< TMultiPartActivationInfo > iMultiPartActivationInfos;
    };
    
#endif

// End of File
