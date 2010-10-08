// Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// e32test\debug\d_tracecore.h
// 
//

#ifndef D_TRACECORE_H
#define D_TRACECORE_H

#include <e32cmn.h>
#include <e32ver.h>
#ifndef __KERNEL_MODE__
#include <e32std.h>
#include <e32debug.h>
#endif

/**

TcDriverParameters

Parameters for activation/notification requests

*/
class TcDriverParameters
    {
    public:
    TUint32 iComponentId;
    TUint32 iGroupId;
    };
	
/**
Interface to the trace activation/deactivation mechanism
*/
class RTraceCoreTest : public RBusLogicalChannel
	{
public:
    
    enum TControl
        {
        EActivateTrace
        , EDeactivateTrace
        , ERefreshActivations
        , ERequestTraceData
        , EValidateFilterSync
        , EDropNextTrace
        , ERegisterActivationNotification
        , ECheckActivationNotificationOk
        };
    
#ifndef __KERNEL_MODE__
	inline TInt Open()
		{
            return DoCreate(Name(),TVersion(0,1,1),KNullUnit,NULL,NULL,EOwnerThread);
		}

	inline TInt ActivateTrace(const TcDriverParameters& aDriverParameters, TInt aNumTraces=1)
		{
		return DoControl( EActivateTrace, (TAny*) &aDriverParameters, (TAny*)(aNumTraces));
		}
	
    inline TInt DeactivateTrace(const TcDriverParameters& aDriverParameters, TInt aNumTraces=1)
        {
        return DoControl( EDeactivateTrace, (TAny*) &aDriverParameters, (TAny*)(aNumTraces) );
        }	

    inline TInt RefreshActivations()
        {
        return DoControl( ERefreshActivations );
        }
    
    inline TInt ValidateFilterSync(const TcDriverParameters& aParams)
        {
        return DoControl( EValidateFilterSync, (TAny*)&aParams);
        }   
    
    inline TInt DropNextTrace(TBool aDrop)
        {
        return DoControl( EDropNextTrace, (TAny*)(aDrop));
        }
    
    inline TInt RegisterActivationNotification(const TcDriverParameters& aParams, TBool aRegister)
        {
        return DoControl( ERegisterActivationNotification, (TAny*) &aParams, (TAny*)(aRegister));
        }     

    inline TInt CheckActivationNotificationOk(TBool aShouldBeNotified)
        {
        return DoControl( ECheckActivationNotificationOk, (TAny*)(aShouldBeNotified));
        }  

    inline void RequestTraceData(TRequestStatus& aStatus, TDes8& aDes, TInt aFrameCount = 1)
        {
        TDes8* p = &aDes;
        DoRequest( ERequestTraceData, aStatus, (TAny*) p, (TAny*)aFrameCount);
        }   

#endif //__KERNEL_MODE__

	inline static const TDesC& Name();
    
	};

inline const TDesC& RTraceCoreTest::Name()
	{
	_LIT(KTraceCoreName,"d_tracecore");
	return KTraceCoreName;
	}

#endif //D_TRACECORE_H
