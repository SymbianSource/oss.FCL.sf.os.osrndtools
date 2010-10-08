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

#ifndef __TRACECORE_H__
#define __TRACECORE_H__


// Include files
#include <kernel/kernel.h>
#include "TraceCoreWriter.h"


// Forward declarations
class DTraceCoreSendReceive;
class DTraceCoreNotifier;
class DTraceCoreConfiguration;
class DTraceCoreInternalMessageHandler;
class DTraceCoreRouter;
class DTraceCoreHandler;
class DTraceCoreBTraceHandler;
class DTraceCorePrintfTraceHandler;
class DTraceCoreSettings;
class MTraceCoreActivation;


/**
 * TraceCore main class
 */
NONSHARABLE_CLASS( DTraceCore ) : public DBase
    {
public:

    /**
     * Gets the TraceCore instance.
     * NULL is returned when no instance is created.
     */
    IMPORT_C static DTraceCore* GetInstance();
    
    /**
     * Returns ETrue if tracecore is loaded already EFalse otherwies
     */
    IMPORT_C static TBool IsLoaded();

    /*
     * Destroy the static instance of tracecore and sets the instance to null
     */
    IMPORT_C static void DestroyTraceCore();
    /**
     * Gets the BTrace handler
     */
    static DTraceCoreBTraceHandler* GetBTraceHandler();
    
    /**
     * Gets the router
     */
    inline DTraceCoreRouter& GetRouter();
    
    /**
     * Gets the send-receive interface
     */
    inline DTraceCoreSendReceive& GetSendReceive();
    
    /**
     * Gets the trace core notifier
     */
    inline DTraceCoreNotifier& GetNotifier();
    
    /**
     * Registers a handler to TraceCore.
     *
     * @param aHandler The handler to be registered
     */
    TInt RegisterHandler( DTraceCoreHandler& aHandler );

    /**
     * Unregisters a handler from TraceCore.
     *
     * @param aHandler The handler to be unregistered
     */
    void UnregisterHandler( DTraceCoreHandler& aHandler );

    /**
     * Registers a writer to TraceCore.
     *
     * @param aWriter The writer to be registered
     */
    TInt RegisterWriter( DTraceCoreWriter& aWriter );

    /**
     * Unregisters a writer from TraceCore.
     *
     * @param aWriter The writer to be unregistered
     */
    void UnregisterWriter( DTraceCoreWriter& aWriter );

    /**
     * Registers a settings saver to TraceCore.
     *
     * @param aSettings The settings (saver) to be registered
     */
    TInt RegisterSettings( DTraceCoreSettings& aSettings );

    /**
     * Unregisters a settings saver from TraceCore.
     *
     * @param aSettings The Settings to be unregistered
     */
    void UnregisterSettings( DTraceCoreSettings& aSettings );
    
    /**
     * Registers an activation interface
     * 
     * @param aActivation The activation interface
     */
    TInt RegisterActivation( MTraceCoreActivation& aActivation );

    /**
     * Gets an activation interface for a specific component ID
     * 
     * @param aComponentID The component ID
     * @return The activation interface or NULL if none exist
     */
    MTraceCoreActivation* GetActivation( TUint32 aComponentID );
     
    /**
     * Starts to use the specific writer
     * 
     * @return KErrNone if successful, KErrNotSupported if type is not valid
     */
    IMPORT_C TInt SwitchToWriter( TWriterType aWriterType );     
    
    /**
     * Get current writer type
     * 
     * @return Current writer type
     */
    IMPORT_C TInt GetCurrentWriterType();
    
    
    /**
     * Returns pointer to TraceCore owned activation / deactivation TDfcQue object.
     */
    inline TDynamicDfcQue* ActivationQ() {
        return iActivationQ;
        }
    
    /**
     * Static method that returns pointer to TraceCore activation / deactivation dfc queue.
     */
    static TDynamicDfcQue* GetActivationQ() {
        DTraceCore* tCore = DTraceCore::GetInstance();
        if(tCore)
            return tCore->ActivationQ();
        else
            return NULL;
        }
    
   
    
    /**
     * @internalTechnology 
     * Sets the "Trace Dropped" flag
     * @param aTraceDropped ETrue if Trace Dropped, EFalse Otherwise
     */
    IMPORT_C void SetPreviousTraceDropped(TBool aTraceDropped);
    
    /**
     * @internalTechnology 
     * returns the state of the  "Trace Dropped" flag
     * @param none
     */
    IMPORT_C TBool PreviousTraceDropped() const; 
    
    /**
     * @internalTechnology
     * @return DTraceCore instance or NULL if creation failed.
     */
    IMPORT_C static DTraceCore* CreateInstance();
    
    /**
     * Reactivate all currently activated traces
     * @param none
     * @return KErrNone if refresh successful
     */
    TInt RefreshActivations();
    
    /**
     * Returns whether or not tracing is certified.
     * The certification status is checked when TraceCore is initialised and then stored for later use.
     * This stored value is used throughout the session, even if the status of the certificate changes.
     * This is done to avoid performance issues when performing the check with every trace outputted. 
     * @param none
     * @return ETrue if tracing is certified, EFalse otherwise
     */
    inline TBool IsTraceCertified() { return iTraceCertified; };

    /**
     * Get the active writer
     * @param none
     * @return a pointer to the current
     */
    IMPORT_C DTraceCoreWriter* GetActiveWriter(); 

private:

    /**
     * Constructor
     */
	DTraceCore();
	
	/**
	 * Destructor
	 */
	~DTraceCore();
	
    /**
     * Initializes trace core
     */
    TInt Init();
    
    /**
     * Starts the BTrace handler
     */
    TInt StartBTrace();
    
    /**
     * Starts the Printf trace handler
     */
    TInt StartPrintfTraceHandler();

    /**
     * Starts to use the first writer from the writers list
     */
    void SwitchToFirstWriter();

    /**
     * Forwards the active writer to handlers
     */
    void SetWriterToHandlers();
    
private:

    /**
     * TraceCore instance
     */
    static DTraceCore* iInstance;
    
    /**
     * List of writers
     */
    RPointerArray< DTraceCoreWriter > iWriters;
    
    /**
     * List of handlers
     */
    RPointerArray< DTraceCoreHandler > iHandlers;

    /**
     * List of activation interfaces
     */
    RPointerArray< MTraceCoreActivation > iActivations;
    
    /**
     * Message router
     */
    DTraceCoreRouter* iRouter;
    
    /**
     * Message sender / receiver
     */
    DTraceCoreSendReceive* iSendReceive;
    
    /**
     * Activation notifier
     */
    DTraceCoreNotifier* iNotifier;    
    
    /**
     * Configuration
     */
    DTraceCoreConfiguration* iConfiguration;    
    
    /**
     * Internal message handler
     */
    DTraceCoreInternalMessageHandler* iInternalMessageHandler;    
    
    
    /**
     * BTrace handler
     */
    DTraceCoreBTraceHandler* iBTrace;
    
    /**
     * Printf trace handler
     */
    DTraceCorePrintfTraceHandler* iPrintf;
    
    /**
     * TraceCore settings
     */
    DTraceCoreSettings* iTraceCoreSettings;
     
    /**
     * Writer that is currently active
     */
    DTraceCoreWriter* iActiveWriter;
    
    /**
     * Activation / Deactivation DFC queue.
     */
    TDynamicDfcQue* iActivationQ;

    /**
     * Previous missing trace flag
     */
    TBool iPreviousTraceDropped;

    /**
     * Trace certified flag.
     * The certification status is checked when TraceCore is initialised and then stored in this variable.
     * This stored value is used throughout the session, even if the status of the certificate changes.
     * This is done to avoid performance issues when performing the check with every trace outputted. 
     */
    TBool iTraceCertified;

    };

#endif

#include "TraceCore.inl"

// End of File
