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

#ifndef     __TRACEONACTIVATION_H__
#define     __TRACEONACTIVATION_H__

#include <kernel/kernel.h>
#include <kernel/kern_priv.h>
#include <TraceCoreNotificationReceiver.h>

//- Forward Declarations ----------------------------------------------------
class TTraceOnActivationParams;
class DTcChannel;
class DTraceConnection;

/**

Logical device factory object

*/
class DTcFactory : public DLogicalDevice
    {
    public:
        DTcFactory();
        virtual ~DTcFactory();
        virtual TInt Install();
        virtual void GetCaps(TDes8& aDes) const;
        virtual TInt Create(DLogicalChannelBase*& aChannel);	
    };


/**

The Kernel side logical channel

*/
class DTcChannel : public DLogicalChannel
    {
    public:
        DTcChannel(DLogicalDevice* aDevice);
        virtual ~DTcChannel();
        virtual TInt DoCreate(TInt aUnit, const TDesC8* aInfo, const TVersion& aVer);

        virtual void HandleMsg(TMessageBase* aMsg);
        TInt DoControl(TInt aFunction, TAny* a1);
        TInt RegisterNotificationReceiver( TTraceOnActivationParams* aParameters );
        TInt UnregisterNotificationReceiver( TTraceOnActivationParams* aParameters );
        virtual TInt TraceActivated( TUint32 aComponentId, TUint16 aGroupId );
        virtual TInt TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId );

    private:
        DThread* iClientThread;   // Store the pointer to the user thread
        DTraceConnection* iTraceConnection;
        TDfcQue* iActivationQue;
    };

#endif      //  __TRACEONACTIVATION_H__
