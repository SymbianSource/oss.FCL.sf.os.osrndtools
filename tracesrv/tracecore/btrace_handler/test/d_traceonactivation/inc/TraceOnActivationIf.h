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

#ifndef     __TRACEONACTIVATIONIF_H__
#define     __TRACEONACTIVATIONIF_H__


//- Include Files  ----------------------------------------------------------

#include <e32ver.h>
#include <e32cmn.h>
#include <TraceCoreSubscriber.h>

//- Constants ---------------------------------------------------------------

/** Name of logical driver */
_LIT( KTraceOnActivationDriverName, "TraceOnActivation" );

	
/**

TTraceOnActivationParams

Parameters for activation/notification requests

*/
class TTraceOnActivationParams
    {
    public:
    TUint32 iComponentId;
    TUint32 iGroupId;
    };

/**

RTraceOnActivation

User side interface

*/
class RTraceOnActivation : public RBusLogicalChannel
    {
    public:
        enum TVer
            {
            EMajorVersionNumber = 1,
            EMinorVersionNumber = 1,
            EBuildVersionNumber = KE32BuildVersionNumber
            };

        /** A set of bits identifying the request numbers which are valid */
        enum TControl
            {
            ERegisterNotificationReceiver,
            EUnregisterNotificationReceiver
            };
                 
    public:
#ifndef __KERNEL_MODE__


        /**
            Create the logical channel for the current thread and open a handle to it

            @param  aUnit  Parameter that contains unit info

            @return KErrNone, if successful
        */
        inline TInt Open(TInt aUnit = KNullUnit) 
            { return DoCreate( KTraceOnActivationDriverName, VersionRequired(), aUnit, NULL, NULL ); };
	
        /**
            Returns version information

            @return Version information
        */
        inline TVersion VersionRequired() const
            { return TVersion( EMajorVersionNumber, EMinorVersionNumber, EBuildVersionNumber ); };


         /**
         Register notification receiver
         
         @param aParameters Contains component ID and group ID
         
         @return KErrNone, if successful
        */
        inline TInt RegisterNotificationReceiver( TTraceOnActivationParams& aParameters )
            { return DoControl( ERegisterNotificationReceiver, static_cast<TAny*>(&aParameters) ); };
        
        
        /**
         Unregister notification receiver
         
         @param aParameters Contains component ID and group ID
        
         @return KErrNone, if successful
         */
        inline TInt UnregisterNotificationReceiver( TTraceOnActivationParams& aParameters )
            { return DoControl( EUnregisterNotificationReceiver, static_cast<TAny*>(&aParameters) ); };
        
#endif
    };

#endif      //  __TRACEONACTIVATIONIF_H__

// End of File
