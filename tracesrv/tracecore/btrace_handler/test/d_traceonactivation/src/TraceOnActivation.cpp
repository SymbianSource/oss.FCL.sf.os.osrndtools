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
//

#include <kernel/arm/assp.h>
#include <kernel/kernel.h>
#include <TraceCoreCommon.h>

#include "TraceCore.h"
#include "TraceCoreConstants.h"
#include "TraceOnActivation.h"
#include "TraceOnActivationIf.h"
#include "TraceConnection.h"


//===========================================================================
// DTcFactory Class
//===========================================================================

//---------------------------------------------------------------------------
/**
    DTcFactory Constructor
*/
DTcFactory::DTcFactory()
: DLogicalDevice()
    {
    // The version of this logical device
    iVersion = TVersion( RTraceOnActivation::EMajorVersionNumber,
                         RTraceOnActivation::EMinorVersionNumber,
                         RTraceOnActivation::EBuildVersionNumber );

    // A bitmasks that indicates device properties
    iParseMask = 0;
    iUnitsMask = 0;
    }


//---------------------------------------------------------------------------
/**
    DTcFactory Destructor
*/
DTcFactory::~DTcFactory()
    {
    }


//---------------------------------------------------------------------------
/**
    Completes the installation of this logical device i.e. to do secondary
    initialization

    @return KErrNone, if successful
*/
TInt DTcFactory::Install()
    {
    return SetName( &KTraceOnActivationDriverName );
    }


//---------------------------------------------------------------------------
/**
    Gets the capabilities of this logical device. Current implementation does
    nothing.

    @param  aDes  On return, contains information describing
                  the capabilities of the device

*/
void DTcFactory::GetCaps(TDes8& /*aDes*/) const
    {
    }


//---------------------------------------------------------------------------
/**
    Creates a logical channel

    @param  aChannel Set to point to the created Logical Channel

    @return KErrNone, if successfull
*/
TInt DTcFactory::Create(DLogicalChannelBase*& aChannel)
    {
	TInt r(KErrNone);
    aChannel = new DTcChannel( this );

    if ( aChannel == NULL )
        {
        r = KErrNoMemory;
        }

    return r;
    }
 
//===========================================================================
// DTcChannel Class
//===========================================================================

//---------------------------------------------------------------------------
/**
    Constructor

    @param  aDevice  A pointer to the LDD factory object that
                    is constructing this logical channel

*/
DTcChannel::DTcChannel(DLogicalDevice* /*aDevice*/)
        : DLogicalChannel()
        , iClientThread( NULL )
        , iActivationQue(NULL)
    {   
    }


//---------------------------------------------------------------------------

/**
    Destructor
*/
DTcChannel::~DTcChannel()
    {
    delete iTraceConnection;
    iTraceConnection = NULL;

    // Close our reference on the client thread
    Kern::SafeClose( reinterpret_cast<DObject*&>( iClientThread ), NULL );

#ifdef __SMP_
    __e32_memory_barrier();
#endif
    }


//---------------------------------------------------------------------------
/**
    Completes the outstanding request identified by the specified request
    number.

    @param  aMsg  System message from user side
*/

void DTcChannel::HandleMsg(TMessageBase* aMsg)
    {
    // Synchronous kernel-side message. There is one per thread, and the
    // thread always blocks while the message is outstanding.
    TThreadMessage& msg = *( 
    				static_cast<TThreadMessage*>( aMsg ) ); 
    
    TInt id = msg.iValue;

    if ( id == static_cast<TInt>( ECloseMsg ) )
        {
        // Don't receive any more messages
        msg.Complete( KErrNone, EFalse );

        // Complete all outstanding messages on this queue
        iMsgQ.CompleteAll( KErrServerTerminated );
        }
    else
        {
        // DoControl
        TInt ret = DoControl( id, msg.Ptr0() );
        msg.Complete( ret, ETrue );
        }
    }

//---------------------------------------------------------------------------
/**
    Performs secondary initialisation of the logical channel

    @param  aUnit  parameter to the user side handle
    @param  anInfo  extra information for the device
    @param  aVer  version requested by the thread

    @return KErrNone, if succesfull
*/
TInt DTcChannel::DoCreate(TInt aUnit, const  TDesC8* /*anInfo*/,
                                             const  TVersion& aVer)
    {
    // Get pointer to client threads DThread object
    iClientThread = &Kern::CurrentThread();
        
    TInt ret = iClientThread->Open();
    if(ret != KErrNone)
        return ret;
       
    DTraceCore* tCore = DTraceCore::GetInstance();
    if(!tCore)
        return KErrNotReady;
    iActivationQue = tCore->ActivationQ();
      
    // This device driver framework does not apply meaning to the idea of a unit.
    if ( aUnit != KNullUnit || !iDevice->QueryVersionSupported( aVer ) )
        {
        ret = KErrNotSupported;
        }
    else
        {
        // security check
        if ( !Kern::CurrentThreadHasCapability( ECapabilityCommDD, 
                        __PLATSEC_DIAGNOSTIC_STRING("Checked by Tc driver") ) )
            {
            ret = KErrPermissionDenied;
            }
        if(ret==KErrNone)
            {
            iTraceConnection = new DTraceConnection(this);
            if(!iTraceConnection)
                {
                ret=KErrNoMemory;
                }
            }
        if(ret==KErrNone)
            {
            // Set the DFC queue to be used by this logical channel to
            // low priority DFC queue (owned by the TraceCore)
            SetDfcQ( iActivationQue  );
            // Mark queue ready to accept messages
            iMsgQ.Receive();
            }
        }
    
    return ret;
    }


//---------------------------------------------------------------------------
/**
    Handles a synchronous request

    @param  aFunction  A number identifying specific functionality
    @param  a1  If specified, a parameter from the user side
    @param  a2  If specified, a parameter from the user side

    @return KErrNone, if successful
*/
TInt DTcChannel::DoControl(TInt aFunction, TAny* a1 = NULL)
    {
    TInt ret( KErrNotSupported );

    switch ( aFunction )
        {
        case RTraceOnActivation::ERegisterNotificationReceiver:
            {
            ret = RegisterNotificationReceiver( static_cast<TTraceOnActivationParams*>(a1) );
            break;
            }
        case RTraceOnActivation::EUnregisterNotificationReceiver:
            {
            ret = UnregisterNotificationReceiver( static_cast<TTraceOnActivationParams*>(a1) );
            break;
            }
        default:
            break;
        }

    return ret;
    }


//---------------------------------------------------------------------------
/**
    Read parameters from user-side thread and call TraceConnection::RegisterNotificationReceiver
    
    @param  aParameters Parameters from user-side
*/
TInt DTcChannel::RegisterNotificationReceiver( TTraceOnActivationParams* aParameters )
    {
    TTraceOnActivationParams params;
    TInt ret(KErrNone);
     //get userside information
	ret = Kern::ThreadRawRead(iClientThread, (const TAny *)aParameters,
			(TAny*)&params, sizeof(TTraceOnActivationParams) );
			
	if(ret != KErrNone)
	    {
	    return ret;
	    }
			
    if(iTraceConnection)
        {
        // Use correct part of iGroupId
        params.iGroupId = FixGroupId( params.iGroupId );
        // Check if iGroupId is valid
        if (!GroupIdIsValid(params.iGroupId))
            {
            ret = KErrArgument;
            }
        else
            {
            iTraceConnection->RegisterNotificationReceiver( params.iComponentId, params.iGroupId );
            }
        }
    else
        {
        ret = KErrGeneral;
        }
    
    return ret;
    }
    
//---------------------------------------------------------------------------
/**
    Read parameters from user-side thread and call TraceConnection::UnregisterNotificationReceiver
    
    @param  aParameters Parameters from user-side
*/
TInt DTcChannel::UnregisterNotificationReceiver( TTraceOnActivationParams* aParameters )
    {
    TTraceOnActivationParams params;
    TInt ret(KErrNone);
     //get userside information
	ret = Kern::ThreadRawRead(iClientThread, (const TAny *)aParameters,
			(TAny*)&params, sizeof(TTraceOnActivationParams) );
			
	if(ret != KErrNone)
	    return ret;

    if(iTraceConnection)
        {
        // Use correct part of iGroupId
        params.iGroupId = FixGroupId( params.iGroupId );
        // Check if iGroupId is valid
        if (!GroupIdIsValid(params.iGroupId))
            {
            ret = KErrArgument;
            }
        else
            {
            iTraceConnection->UnregisterNotificationReceiver( params.iComponentId, params.iGroupId );
            }
        }
    else
        {
        ret = KErrGeneral;
        }
    
    return ret;
    }


/**
    TraceActivated Notification
    
    Called from TraceConnection-TraceCore
    
    @param  aComponentId ComponentId of activated trace
    @param  aGroupId GroupId of activated trace
*/
TInt DTcChannel::TraceActivated( TUint32 aComponentId, TUint16 aGroupId )
    {    
    // Set component ID used in trace point
    TComponentId KOstTraceComponentID = aComponentId;

    // Calculate trace word from Group ID
    TUint32 traceWord = (aGroupId << GROUPIDSHIFT) | 1;

    // Instrument a trace point, as a simulated priming trace
    OstTrace0(aGroupId, traceWord, "Priming trace");

    return KErrNone;
    }


/**
    TraceDeactivated Notification
    
    Called from TraceConnection-TraceCore
    
    @param  aComponentId ComponentId of activated trace
    @param  aGroupId GroupId of activated trace
    @return 
*/
TInt DTcChannel::TraceDeactivated( TUint32 /*aComponentId*/, TUint16 /*aGroupId*/ )
    {
    return KErrNone;
    }


//---------------------------------------------------------------------------
/**
    DECLARE_STANDARD_LDD

    @return DTcFactory object
*/
DECLARE_STANDARD_LDD()
    {
    return new DTcFactory;
    }

