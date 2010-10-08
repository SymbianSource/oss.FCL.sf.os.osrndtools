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
// Handler for BTrace OST categories  
//

#ifndef __BTRACEOSTCATEGORYHANDLER_H__
#define __BTRACEOSTCATEGORYHANDLER_H__


// Include files
#include <kernel/kernel.h>
#include "BTraceCategoryHandler.h"
#include "TraceCoreActivation.h"

#define MAX_TRACE_DATA_SIZE 512
#define COMPONENT_GROUP_CACHE_SIZE 12 // 4 * (1 ComponentId + 1 groupId + activation info )
#define MAX_MULTIPART_TRACES 4

// Forward declarations
class DBTraceOstCategoryBitmap;

#ifndef __SMP__

/**
 * Multipart trace message.
 */
class TMultiPartTrace
    {
public:
    TUint32 iMultiPartId;
    TUint32 iHeader;
    TUint32 iHeader2;
    TUint32 iContext;
    TUint32 iComponentId;
    TUint32 iTraceWord;
    TUint8 iData[MAX_TRACE_DATA_SIZE];
    TUint32 iPc;
    TUint32 iDataInserted;
    };
	
#endif // !__SMP__

/**
 * Category handler for OST
 */
NONSHARABLE_CLASS( DBTraceOstCategoryHandler ) : public DBTraceCategoryHandler
, public MTraceCoreActivationNotification
    {
public:

    /**
     * Constructor
     */
    DBTraceOstCategoryHandler();
     
     /**
      * Destructor
      */
    ~DBTraceOstCategoryHandler();
        
    /**
     * Initializes this handler
     */
    TInt Init();
    
    /**
     * Set settings
     * 
     * @param aSettings Settings
     */
    void SetSettings( DTraceCoreSettings* aSettings );
    
    /**
     * Traces deactivated
     */
     TBool iAllTracesDeactivated;
     
    /**
     * Component/Group cache
     */
    TUint32  iComponentGroupCache[COMPONENT_GROUP_CACHE_SIZE];
    TUint32* iCacheItem1;
    TUint32* iCacheItem2;
    TUint32* iCacheItem3;
    TUint32* iCacheItem4;
    
    TUint32* iTemp;
     
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
     * MTraceCoreBTraceCategoryHandler implementation for KCategoryNokiaOst
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
    TBool HandleFrame( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                       const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                       const TUint32 aExtra, const TUint32 aPc );
    
    /**
     * Checks if given trace is a Multipart trace
     *
     * @param aHeader Header data
     * @param aHeader2 Extra header data
     */
    inline TBool CheckMultiPart( TUint32 aHeader, TUint32 aHeader2 );
    
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
     * Checks if there is a dropped trace and updates header if so
     *
     * @param aHeader Header data
     */
    inline TBool CheckDroppedTrace( TUint32& aHeader );
    
#ifndef __SMP__
	
    /**
     * Calculates data start offset
     *
     * @param aHeader BTrace header
     */
    TUint32 CalculateDataStartOffset( TUint32 aHeader );
	
    /**
     * Multipart traces
     */ 
    TMultiPartTrace iMultiPartTrace;
	
#else // !__SMP__

    /**
     * Find multi part id from multipart array
     * @return index where the MultiPartActivationInfo with aMultiPartId is found.
     */
    TInt Find( TUint32 aMultiPartId );
    
	/**
     * "Remove" multipart from array
     */
    void Remove( TUint32 aIndex );
        
    /**
     * Insert multi part activation info to array if it doesn't exist yet.
     * @return KErrNone if insert ok.
     */
    TInt InsertIfNotExist( TMultiPartActivationInfo &aMultiPartTrace );

    /**
     * Saved activation item for multipart traces
     */ 
    TMultiPartActivationInfo iMultipartArray[MAX_MULTIPART_TRACES];
    
	/**
     * List of free/used multipart indexes
     */ 
	TInt iFreeList[MAX_MULTIPART_TRACES];

#endif // __SMP__

    /**
     * Trace bitmap for OST category
     */
    DBTraceOstCategoryBitmap* iOstTraceBitmap;
    

    };
    
#endif

// End of File
