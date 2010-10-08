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
// Tracks the activation of BTrace OST categories  
//

#ifndef __BTRACEOSTCATEGORYBITMAP_H__
#define __BTRACEOSTCATEGORYBITMAP_H__


// Include files
#include <opensystemtrace_types.h>
#include "TraceCoreActivationBase.h"

class DBTraceOstCategoryHandler;

/**
 * OST trace component.
 */
class TOstComponent
    {
public:
    TUint32 iComponentId;
    RArray< TUint > iGroups; //TODO: implement this as a bitmap rather than RArray
    };
    

/**
 * OST Trace bitmap (dynamic). This extends the subscriber base class to receive bitmap updates
 */
NONSHARABLE_CLASS( DBTraceOstCategoryBitmap ) : public DTraceCoreActivationBase
    {
public:

    /**
     * Constructor
	 * @param aBTraceOstCategoryHandler Pointer to CBTraceOstCategoryHandler
     */
    DBTraceOstCategoryBitmap(DBTraceOstCategoryHandler* aBTraceOstCategoryHandler);
    
    /**
     * Destructor
     */
    ~DBTraceOstCategoryBitmap();
    
    /**
     * Initializes this bitmap. This subscribes to the bitmap update message
     */
    TInt Init();
    
    /**
     * Checks if trace points with a specific component ID and group ID are
     * currently active
     *
     * @param aComponentId The component ID
     * @param aTraceWord The trace word containing the group ID to check
     * @return ETrue if trace is active, EFalse if not
     */
    TBool IsTraceActivated( const TUint32 aComponentId, TUint32 aTraceWord );
    
    /**
     * Activate trace group
     *
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */
    void ActivateTrace( const TUint32 aComponentId, const TUint32 aGroupId );
    
    /**
     * Deactivate trace group
     *
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */
    void DeactivateTrace( const TUint32 aComponentId, const TUint32 aGroupId );
 
    /**
     * Deactivates all groups from a component
     * 
     * @param aComponentId The component ID
     */
    void DeactivateAll( const TUint32 aComponentId );
    
    /**
     * Updates this activation object from the given settings
     * 
     * @param aSettings the settings to be read
     */
    void ReadFromSettings( DTraceCoreSettings& aSettings );
    
    /**
     * Updates the settings with the content of this activation object
     * 
     * @param aSettings the settings to be written
     */
    void WriteToSettings( DTraceCoreSettings& aSettings );
    
    /**
     * Gets activated groups for this component ID
     * @param aComponentId the component ID
     */
    RArray< TUint > GetActivatedGroups( const TUint32 aComponentId );
    
    /**
     * Reactivate all currently activated traces
     * 
     * This function deactivates all groups currently active and re-activates them.
     * This results in kernel priming data being generated for all active kernel groups.
     * 
     * @return KErrNone if refresh successful
     */
    TInt RefreshActivations();
    
private:

    TInt FindComponentId( TUint32 aComponentId, RArray<TOstComponent>& aOstComponents );
    TInt AddComponentId( TUint32 aComponentId, RArray<TOstComponent>& aOstComponents );
    TInt RemoveComponentId( TUint32 aComponentId, TBool aNotifyActivationListeners, 
            RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray );
    TInt AddGroupId( TUint32 aComponentId, TGroupId aGroupId, TInt aIndex, 
            RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray );
    TInt RemoveGroupId( TUint32 aComponentId, TGroupId aGroupId, TInt aIndex, 
            RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray );
    TInt OverwriteGroups (const TUint8* aFrameStart,
                       TInt aComponentIndex, TUint32 aComponentId , TUint32 numberOfGroups, 
                       RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray);
    TInt ReadOstProtocolFrame(const TDesC8& aMessage, TUint32 aFrameStartIndex);
    TInt ReadOstProtocolFrameGroups(const TDesC8& aMessage,
            TInt aFrameStartIndex, TUint8 aActivationInfo, TInt aComponentIndex,
            TUint32 aComponentId, RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray);
    TInt ReadOstFrame( const TDesC8& aMessage, TUint32 aFrameStartIndex );
    TInt ReadOstFrameGroups( const TDesC8& aMessage, TInt aFrameStartIndex, 
            TUint8 aActivationInfo, TInt aComponentIndex, TUint32 aComponentId, 
            RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray );
    void MessageReceived( TTraceMessage &aMsg );
    void OstProtocolMessage( TTraceMessage &aMsg );
    TBool IsComponentSupported( TUint32 /*aComponentID*/ );
 	TInt AllocateActivationBuffer();
    TUint32 Read32FromBuffer(TUint8*& aPtr);
    TUint16 Read16FromBuffer(TUint8*& aPtr);
    void WriteToBuffer(TUint8*& aPtr, TUint32 aData32);
    void WriteToBuffer(TUint8*& aPtr, TUint16 aData16);    
    void HandleTraceActivationChanges(RArray< TOstComponent >& aChangedArray);
    
    //used by locking mechanism
    template <class T> void CopyArray(RArray<T>* aSrc, RArray<T>* aDst);
    void SetNewComponentsArrayPtr(RArray<TOstComponent>*& aNewPtr);
    
    //utility methods
    void CleanupComponentsArray(RArray< TOstComponent >& aComponentArray);
    void CleanupComponentsArrays();
    void AddGroupToChangedArray(RArray< TOstComponent >& aChangedArray, TUint32 aComponentId, TGroupId aGroupId);
    
private:

    /**
     * OST Components
     */	
	RArray< TOstComponent >* iOstComponents;
	
	//temporary array used in activation/deactivation methods
	RArray< TOstComponent >* iTmpArray;
	
	//array used in to store changed components
	RArray< TOstComponent >* iChangedArray;
	
	/**
	 * Buffer for saving and loading activations
	 */
	TUint8* iActivationBuffer;
    DBTraceOstCategoryHandler* iBTraceOstCategoryHandler;
	
    };

#endif // __BTRACEOSTCATEGORYBITMAP_H__

// End of File
