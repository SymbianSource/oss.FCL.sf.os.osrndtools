// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// e32test\debug\d_tracecore.cpp
// 
//

#include <kernel/kern_priv.h>
#include <kernel/kernel.h>

#include <e32btrace.h>
#include <opensystemtrace_types.h>

#ifndef __SMP__
#include <nkern/nkern.h>
#else
#include <nkernsmp/nkern.h>
#endif //__SMP__
#include <TraceCoreTraceActivationIf.h>
#include <TraceCoreNotificationReceiver.h>
#include "TraceCore.h"
#include "d_tracecore.h"


#include "TraceCoreTestWriter.h"
#include "TestDataWriterNotifier.h"

const TInt KFrameBufferLength = 4096;

class DTraceCoreTestFactory : public DLogicalDevice 
	{
public:
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
	};

class DTraceCoreTestChannel : public DLogicalChannel, MTestWriterNotifier, MTraceCoreNotificationReceiver
	{
public:
	DTraceCoreTestChannel();
	virtual ~DTraceCoreTestChannel();
	//	Inherited from DObject
	virtual TInt RequestUserHandle(DThread* aThread, TOwnerType aType);
	// Inherited from DLogicalChannelBase
	virtual TInt DoCreate(TInt aUnit, const TDesC8* anInfo, const TVersion& aVer);
	virtual void HandleMsg(TMessageBase* aMsg);
	TInt DoControl(TInt aFunction, TAny* a1, TAny* a2);
	TInt DoRequest(TInt aId, TRequestStatus* aStatus, TAny* a1, TAny* a2);
	
    // Virtual from MTraceCoreNotificationReceiver. Called from TraceCore.
    void TraceActivated( TUint32 aComponentId, TUint16 aGroupId  );
    void TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  );
	

private:
	DThread* iClient;
	
private:
	void ActivateTrace(TcDriverParameters& aDriverParameters, TInt aNumTraces);
	void DeactivateTrace(TcDriverParameters& aDriverParameters, TInt aNumTraces);
    TInt RefreshActivations();
    TInt ValidateFilterSync(TcDriverParameters& aDriverParams);
    void DropNextTrace(TBool aDrop);
    void RegisterActivationNotification(TcDriverParameters& aDriverParameters, TBool aRegister);
    TInt CheckActivationNotificationOk(TBool aShouldBeNotified);
	
	TInt CreateWriter();
	// from MTestWriterNotifier
    virtual void WriteComplete(TNotifyData aData);
    virtual void WriteStart();
private:
  
    TRequestStatus* iTraceDataRequestStatus;      // request status for asynct trace requests
    TDes8*          iTraceDataDestination;        // pointer to write trace data to
    TDynamicDfcQue* iOstTestDriverDfcQ;           // Dedicated non-realtime DfcQ
    TBool           iDropTrace;                   // if test doesn't want trace to actually be sent
    TInt                            iFrameCount;  // Number of frames to capture before notify is issued 
    TBuf8<KFrameBufferLength>       iFrameBuffer; // the frame buffer
    TBool           iFilterInSyncWhenNotified;    // If true, filter in TraceCore was in sync with the notification
    TBool           iNotificationReceived;        // If true, trace activation notification was received
	};

const TInt KTestOstDriverThreadPriority = 24;
_LIT(KTestOstDriverThread,"d_tracecore_dfcq");


//
// DTraceCoreTestFactory
//

TInt DTraceCoreTestFactory::Install()
	{
	return SetName(&RTraceCoreTest::Name());
	}

void DTraceCoreTestFactory::GetCaps(TDes8& aDes) const
	{
	Kern::InfoCopy(aDes,0,0);
	}

TInt DTraceCoreTestFactory::Create(DLogicalChannelBase*& aChannel)
	{
	aChannel=new DTraceCoreTestChannel();
	if(!aChannel)
		return KErrNoMemory;
	return KErrNone;
	}


//
// DTraceCoreTestChannel
//

DTraceCoreTestChannel::DTraceCoreTestChannel()
: iTraceDataRequestStatus(NULL)
, iTraceDataDestination(NULL)
, iDropTrace(EFalse)
, iFilterInSyncWhenNotified(EFalse)
    {
	}

DTraceCoreTestChannel::~DTraceCoreTestChannel()
	{	
	// Detatch (stop notifications) from writer	     
    DTraceCoreTestWriter* testWriter = DTraceCoreTestWriter::GetInstance();
    if(testWriter)
        {
        testWriter->SetNotifier(NULL);        
        }           
    delete testWriter;
    
    //destroy dfcq
    if (iOstTestDriverDfcQ)
        {
        iOstTestDriverDfcQ->Destroy();
        }
	}

TInt DTraceCoreTestChannel::DoCreate(TInt /*aUnit*/, const TDesC8* /*aInfo*/, const TVersion& /*aVer*/)
	{
    TDynamicDfcQue* q;
    TInt ret = Kern::DynamicDfcQCreate(q, KTestOstDriverThreadPriority , KTestOstDriverThread);
    if (ret==KErrNone)
        {
        //disable real-time state of the dfcq
        q->SetRealtimeState(ERealtimeStateOff);
        iOstTestDriverDfcQ=q;
        SetDfcQ(iOstTestDriverDfcQ);
        iMsgQ.Receive();
        }
    else
        {
        Kern::Printf("Kern::DynamicDfcQCreate returned with error: %d",ret);
        return ret;
        }
	iClient = &Kern::CurrentThread();			
	return CreateWriter();
	}

void DTraceCoreTestChannel::WriteStart()
    {
    }

void DTraceCoreTestChannel::WriteComplete(TNotifyData aNotifyData) 
    {

    if(iTraceDataRequestStatus && iFrameCount > 0 )
        {    

        // append the data into the frame buffer if it fits
        if( iFrameBuffer.Length() + aNotifyData.iLen < KFrameBufferLength)
            {        
            iFrameBuffer.Append(TPtrC8((TUint8*)aNotifyData.iAddr, aNotifyData.iLen ));
            }
        else
            {
            // force a send of what we have
            iFrameCount = 1;
            }
                       
        if( --iFrameCount == 0)
            {    
            if (iDropTrace)
                {
                // test client is requesting we force tracecore to drop the next trace
                // so we call the same function that a writer would call ( SetPreviousTraceDropped )
                // in order to notify tracecore that a trace has been dropped            
                DTraceCore* tracecore = DTraceCore::GetInstance();
                tracecore->SetPreviousTraceDropped(ETrue);
                }
            else//send the trace
                {
                Kern::KUDesPut(*iTraceDataDestination,iFrameBuffer);
                }
            
            // complete the clients request
            Kern::RequestComplete(iTraceDataRequestStatus, 0);
            iTraceDataRequestStatus = NULL;
            iFrameBuffer.Zero();            
            }
        }    
    }

TInt DTraceCoreTestChannel::CreateWriter()
    {
    TInt r = KErrNoMemory;
    DTraceCoreTestWriter* testWriter = DTraceCoreTestWriter::GetInstance(); 
    if(testWriter)
        {
        r = KErrNone;
        testWriter->SetNotifier(this);
        }       
    return r;
    }

TInt DTraceCoreTestChannel::RequestUserHandle(DThread* aThread, TOwnerType aType)
	{
	if (aType!=EOwnerThread || aThread!=iClient)
		return KErrAccessDenied;
	return KErrNone;
	}

void DTraceCoreTestChannel::HandleMsg(TMessageBase* aMsg)
	{    
    TThreadMessage& msg = *(static_cast<TThreadMessage*>(aMsg));    
    TInt id = msg.iValue;
    
    if ( id == static_cast<TInt>( ECloseMsg ) )
        {

        // Don't receive any more messages
        msg.Complete( KErrNone, EFalse );

        // Complete all outstanding messages on this queue
        iMsgQ.CompleteAll( KErrServerTerminated );
        }
    else if ( id == KMaxTInt )
        {
        // 'DoCancel' message
        TRequestStatus* pS = reinterpret_cast<TRequestStatus*>( msg.Ptr0() );
        Kern::RequestComplete(iClient,pS,KErrCancel);
        msg.Complete( KErrNone, ETrue );
        }
    else if ( id < 0 )
        {
        // DoRequest
        TRequestStatus* pS = reinterpret_cast<TRequestStatus*>( msg.Ptr0() );
        
        TInt ret = DoRequest( ~id, pS, msg.Ptr1(), msg.Ptr2());
        if ( ret != KErrNone )
            {
            Kern::RequestComplete( iClient, pS, ret );
            }//noelse
        
        msg.Complete( KErrNone, ETrue );
        }
    else
        {
        // DoControl
        TInt ret = DoControl( id, msg.Ptr0(), msg.Ptr1() );
        msg.Complete( ret, ETrue );
        }

	}

TInt DTraceCoreTestChannel::DoControl(TInt aFunction, TAny* a1, TAny* a2)
    {
    switch(aFunction)
          {
    // test functions
          case RTraceCoreTest::EActivateTrace:
              {
//            Kern::Printf("DTraceCoreTestChannel::DoControl() RTraceCoreTest::EActivateTrace");
              TcDriverParameters* tcDriverParameters = static_cast<TcDriverParameters*>(a1); 
              __ASSERT_ALWAYS(tcDriverParameters!=NULL, Kern::Fault("DTraceCoreTestChannel::DoControl: NULL parameter!", __LINE__) );
              ActivateTrace(*tcDriverParameters, (TInt)(a2));
              return KErrNone;
              }
         case RTraceCoreTest::EDeactivateTrace:
              {
//            Kern::Printf("DTraceCoreTestChannel::DoControl()  RTraceCoreTest::EDeactivateTrace");
              TcDriverParameters* tcDriverParameters = static_cast<TcDriverParameters*>(a1); 
              __ASSERT_ALWAYS(tcDriverParameters!=NULL, Kern::Fault("DTraceCoreTestChannel::DoControl: NULL parameter!", __LINE__) );              
              DeactivateTrace(*tcDriverParameters, (TInt)(a2));
              return KErrNone;
              }
         case RTraceCoreTest::ERefreshActivations:
             {
//            Kern::Printf("DTraceCoreTestChannel::DoControl() RTraceCoreTest::ERefreshActivations");
             return RefreshActivations();
             }
              
         case RTraceCoreTest::EValidateFilterSync:
             {
//           Kern::Printf("DTraceCoreTestChannel::ReqDoControluest()  RTraceCoreTest::EValidateFilterSync");
             TcDriverParameters* tcDriverParameters = static_cast<TcDriverParameters*>(a1); 
             __ASSERT_ALWAYS(tcDriverParameters!=NULL, Kern::Fault("DTraceCoreTestChannel::DoControl: NULL parameter!", __LINE__) );             
             return ValidateFilterSync(*tcDriverParameters);
             }
             
         case RTraceCoreTest::EDropNextTrace:
             {
//           Kern::Printf("DTraceCoreTestChannel::DoControl()  RTraceCoreTest::EDropNextTrace");        
             DropNextTrace(TBool(a1));
             return KErrNone;
             }
         case RTraceCoreTest::ERegisterActivationNotification:
             {
//           Kern::Printf("DTraceCoreTestChannel::DoControl()  RTraceCoreTest::ERegisterActivationNotification");
             TcDriverParameters* tcDriverParameters = static_cast<TcDriverParameters*>(a1); 
             __ASSERT_ALWAYS(tcDriverParameters!=NULL, Kern::Fault("DTraceCoreTestChannel::DoControl: NULL parameter!", __LINE__) );
             RegisterActivationNotification(*tcDriverParameters, (TBool)(a2));
             return KErrNone;
             }       
         case RTraceCoreTest::ECheckActivationNotificationOk:
             {
//             Kern::Printf("DTraceCoreTestChannel::DoControl()  RTraceCoreTest::ECheckActivationNotificationOk");        
             return CheckActivationNotificationOk((TBool)(a1));
             }                  
            
         default:
             break;
              }
    return KErrNotSupported;
    }

TInt DTraceCoreTestChannel::DoRequest(TInt aId, TRequestStatus* aStatus, TAny* a1, TAny* a2)
    {
    switch(aId)
        {
        case RTraceCoreTest::ERequestTraceData:  // async request
            {                     
            iTraceDataRequestStatus   = aStatus;           
            if(a1)
                {
                TDes8* p = NULL;
                XTRAPD(r, XT_DEFAULT, kumemget(&p, &a1, sizeof(TAny*)); )                
                iTraceDataDestination = (r == KErrNone) ? p : NULL;                
                }
            
            iFrameCount = (a2) ? (TInt)a2 : 1;
            iFrameBuffer.Zero();
            
            return KErrNone;
                }
          
         default:
            break;
        }
    return KErrNotSupported;
    }

void DTraceCoreTestChannel::ActivateTrace(TcDriverParameters& aDriverParameters, TInt aNumTraces)
    {
    TcDriverParameters tcDriverParams;
    TInt ret = Kern::ThreadRawRead(iClient, (const TAny *)&aDriverParameters,(TAny*)&tcDriverParams, sizeof(TcDriverParameters) );
    __ASSERT_ALWAYS(KErrDied!=ret, Kern::Fault("DTraceCoreTestChannel::ActivateTrace: ThreadRawRead: iClient died!", __LINE__) );    
    
    NKern::ThreadEnterCS();
    for (TInt i=0; i<aNumTraces; i++)
        {
        DTraceActivationIf::ActivateTrace((tcDriverParams.iComponentId)+i,(tcDriverParams.iGroupId)+i);
        }
    NKern::ThreadLeaveCS();    
    }

void DTraceCoreTestChannel::DeactivateTrace(TcDriverParameters& aDriverParameters, TInt aNumTraces)
    {    
    TcDriverParameters tcDriverParams;    
    TInt ret = Kern::ThreadRawRead(iClient, (const TAny *)&aDriverParameters,(TAny*)&tcDriverParams, sizeof(TcDriverParameters) );
    __ASSERT_ALWAYS(KErrDied!=ret,Kern::Fault("DTraceCoreTestChannel::DeactivateTrace: ThreadRawRead: iClient died!", __LINE__) );

    NKern::ThreadEnterCS();    
    for (TInt i=0; i<aNumTraces; i++)
        {
        DTraceActivationIf::DeactivateTrace((tcDriverParams.iComponentId)+i,(tcDriverParams.iGroupId)+i);
        }
    NKern::ThreadLeaveCS();    
    }

TInt DTraceCoreTestChannel::RefreshActivations()
    {
    NKern::ThreadEnterCS();
    TInt err = DTraceActivationIf::RefreshActivations();
    NKern::ThreadLeaveCS();
    return err;
    }

/**
 * Validate that the BTrace::Filter matches the tracecore filters 
 * for all OST categories
 * 
 * returns KErrNone if filters match - KErrGeneral otherwise
 */
TInt  DTraceCoreTestChannel::ValidateFilterSync(TcDriverParameters& aDriverParameters)
    {
    TcDriverParameters tcDriverParams;
    TInt ret = Kern::ThreadRawRead(iClient, (const TAny *)&aDriverParameters,(TAny*)&tcDriverParams, sizeof(TcDriverParameters) );
    __ASSERT_ALWAYS(ret == KErrNone, Kern::Fault("DTraceCoreTestChannel::ValidateFilterSync: ThreadRawRead: iClient died!", __LINE__) );    
    
    TBool tcFilter = DTraceActivationIf::IsTraceActivated(tcDriverParams.iComponentId, tcDriverParams.iGroupId);
    TBool btFilter = BTrace::CheckFilter(tcDriverParams.iGroupId);                    
    if( tcFilter != btFilter)
        {
        ret = KErrGeneral;
        }            

    return ret;
    }

/**
 * Adds or removes a activation notification listener
 * 
 * returns KErrNone if filters match - KErrGeneral otherwise
 */
void DTraceCoreTestChannel::RegisterActivationNotification(TcDriverParameters& aDriverParameters, TBool aRegister)
    {
    TcDriverParameters tcDriverParams;
    TInt ret = Kern::ThreadRawRead(iClient, (const TAny *)&aDriverParameters,(TAny*)&tcDriverParams, sizeof(TcDriverParameters) );
    __ASSERT_ALWAYS(ret == KErrNone, Kern::Fault("DTraceCoreTestChannel::ValidateFilterSync: ThreadRawRead: iClient died!", __LINE__) );
        
    if (aRegister)
        {
        MTraceCoreNotificationReceiver::RegisterNotificationReceiver(tcDriverParams.iComponentId, tcDriverParams.iGroupId);
        }
    else
        {
        MTraceCoreNotificationReceiver::UnregisterNotificationReceiver(tcDriverParams.iComponentId, tcDriverParams.iGroupId);
        }
    }

/**
 * Add a activation notification listener, then activate a trace group and check if the activation  
 * has really happened when the notification arrives
 * 
 * returns KErrNone if filters match - KErrGeneral otherwise
 */
TInt DTraceCoreTestChannel::CheckActivationNotificationOk(TBool aShouldBeNotified)
    {
    TInt ret = KErrGeneral;
    
    // Everything OK if we should've got notification and we got and filters were in sync
    if (aShouldBeNotified && iNotificationReceived && iFilterInSyncWhenNotified)
        {
        ret = KErrNone;
        }
    
    // Everything OK if we should NOT got notification and we didn't
    else if (!aShouldBeNotified && !iNotificationReceived)
        {
        ret = KErrNone;
        }
    
    // Reset the variables for next test
    iFilterInSyncWhenNotified = EFalse;
    iNotificationReceived = EFalse;
    
    return ret;
    }

/**
 * Callback function from TraceCore when the trace is activated
 */
void DTraceCoreTestChannel::TraceActivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    TBool tcFilter = DTraceActivationIf::IsTraceActivated(aComponentId, aGroupId);
    iFilterInSyncWhenNotified = tcFilter;
    iNotificationReceived = ETrue;
    }

/**
 * Callback function from TraceCore when the trace is deactivated
 */
void DTraceCoreTestChannel::TraceDeactivated( TUint32 aComponentId, TUint16 aGroupId  )
    {
    TBool tcFilter = DTraceActivationIf::IsTraceActivated(aComponentId, aGroupId);
    iFilterInSyncWhenNotified = !tcFilter;
    iNotificationReceived = ETrue;
    }

/*
 * Tells the test writer that we don't actually want to 
 * send a trace... this is to test that the handlers
 * are formatting the data to include "missing" info.
 */
void DTraceCoreTestChannel::DropNextTrace(TBool aDrop)
    {
    iDropTrace=aDrop;
    }

DECLARE_STANDARD_LDD()
	{
	Kern::Printf("d_tracecore.ldd creating DTraceCoreTestFactory");
	return new DTraceCoreTestFactory;
	}


