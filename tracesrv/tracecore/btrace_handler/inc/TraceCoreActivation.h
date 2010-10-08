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
// Interface to activation bitmap classes
//

#ifndef __TRACECOREACTIVATION_H__
#define __TRACECOREACTIVATION_H__


class MTraceCoreActivation;
class DTraceCoreSettings;


/**
 * Interface to get change notification from activation classes
 */
NONSHARABLE_CLASS( MTraceCoreActivationNotification )
    {
public:

    /**
     * Notification that the activation state has changed
     * 
     * @param aActivation the activation interface that was updated
     * @param aFromSettings ETrue if activation was due to settings read, EFalse if from some other source
     * @param aComponentId Component ID of the activation
     */
    virtual void ActivationChanged( MTraceCoreActivation& aActivation, TBool aFromSettings,
            TUint32 aComponentId ) = 0;
    };


/**
 * Interface to activation classes
 */
NONSHARABLE_CLASS( MTraceCoreActivation )
    {
public:

    /**
     * Checks if this bitmap supports the given component ID
     */
    virtual TBool IsComponentSupported( TUint32 aComponentID ) = 0;
    
    /**
     * Gets activated groups for this component ID
     * @param aComponentId the component ID
     */
    virtual RArray<TUint> GetActivatedGroups( const TUint32 aComponentId ) = 0;
    
    /**
     * Updates this activation object from the given settings
     * 
     * @param aSettings the settings to be read
     */
    virtual void ReadFromSettings( DTraceCoreSettings& aSettings ) = 0;
    
    /**
     * Updates the settings with the content of this activation object
     * 
     * @param aSettings the settings to be written
     */
    virtual void WriteToSettings( DTraceCoreSettings& aSettings ) = 0;
    
    /**
     * Registers an activation notification interface
     */
    virtual TInt RegisterActivationNotification( MTraceCoreActivationNotification& aNotification ) = 0;
    
     /**
     * Activate trace group.
     * Method can be called only from Symbian OS thread context. 
     * Activation of groupID/ComponentID from IRQs, IDFCs is not allowed.
     *
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */
    virtual void ActivateTrace( const TUint32 aComponentId, const TUint32 aGroupId ) = 0;
    
    /**
     * Deactivate trace group
     *
     * Method can be called only from Symbian OS thread context.
     * Activation of groupID/ComponentID from IRQs, IDFCs is not allowed.
     * 
     * @param aComponentId The component ID
     * @param aGroupId The group ID
     */
    virtual void DeactivateTrace( const TUint32 aComponentId, const TUint32 aGroupId ) = 0;

    /**
     * Deactivates all groups from a component
     * 
     * @param aComponentId The component ID
     */
    virtual void DeactivateAll( const TUint32 aComponentId ) = 0;

    /**
     * Reactivate all currently activated traces
     * 
     * @return KErrNone if refresh successful
     */
    virtual TInt RefreshActivations() { return KErrNotFound; };
    };

    
#endif // __TRACECOREACTIVATION_H__
