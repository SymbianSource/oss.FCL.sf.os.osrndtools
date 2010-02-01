/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#include "MemSpyDriverLogChanContainers.h"

// System includes
#include <memspy/driver/memspydriverconstants.h>
#include <memspy/driver/memspydriverobjectsshared.h>

// Shared includes
#include "MemSpyDriverOpCodes.h"
#include "MemSpyDriverObjectsInternal.h"

// User includes
#include "MemSpyDriverUtils.h"
#include "MemSpyDriverUserEventMonitor.h"
#include "MemSpyDriverSuspensionManager.h"
#include "MemSpyDriverOSAdaption.h"

// Constants
const TInt KMemSpyDriverLogChanContainersXferBufferSize = 1024 * 4;


DMemSpyDriverLogChanContainers::DMemSpyDriverLogChanContainers( DMemSpyDriverDevice& aDevice, DThread& aThread )
:   DMemSpyDriverLogChanContainerBase( aDevice, aThread )
    {
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::DMemSpyDriverLogChanContainers() - this: 0x%08x", this ));
    }


DMemSpyDriverLogChanContainers::~DMemSpyDriverLogChanContainers()
	{
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::~DMemSpyDriverLogChanContainers() - START - this: 0x%08x", this ));

	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::~DMemSpyDriverLogChanContainers() - END - this: 0x%08x", this ));
	}


TInt DMemSpyDriverLogChanContainers::Construct()
	{
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::Construct() - START - this: 0x%08x", this ));
    
    const TInt ret = BaseConstruct( KMemSpyDriverLogChanContainersXferBufferSize );

	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::Construct() - END - this: 0x%08x, err: %d", this, ret ));
    return ret;
	}


TInt DMemSpyDriverLogChanContainers::Request( TInt aFunction, TAny* a1, TAny* a2 )
	{
	TInt r = DMemSpyDriverLogChanContainerBase::Request( aFunction, a1, a2 );
    if  ( r == KErrNone )
        {
	    switch( aFunction )
		    {
	    case EMemSpyDriverOpCodeContainersGetHandles:
            r = GetContainerHandles( (TMemSpyDriverInternalContainerHandleParams*) a1 );
            break;
	    case EMemSpyDriverOpCodeContainersGetHandleInfo:
            r = GetGenericHandleInfo( (TInt) a1, (TMemSpyDriverHandleInfoGeneric*) a2 );
            break;
        case EMemSpyDriverOpCodeContainersGetApproxSize:
            r = GetKernelObjectSizeApproximation( (TMemSpyDriverContainerType) ((TInt) a1), (TInt*) a2 );
            break;
        case EMemSpyDriverOpCodeContainersGetReferencesToMyThread:
            r = GetReferencesToMyThread( (TUint) a1, (TDes8*) a2 );
            break;
        case EMemSpyDriverOpCodeContainersGetReferencesToMyProcess:
            r = GetReferencesToMyProcess( (TUint) a1, (TDes8*) a2 );
            break;
        case EMemSpyDriverOpCodeContainersGetPAndSInfo:
            r = GetPAndSInfo( (DObject*) a1, (TMemSpyDriverPAndSInfo*) a2 );
            break;
        case EMemSpyDriverOpCodeContainersGetCondVarSuspendedThreads:
            r = GetCondVarSuspendedThreads( (TMemSpyDriverInternalCondVarSuspendedThreadParams*)a1 );
            break;
        case EMemSpyDriverOpCodeContainersGetCondVarSuspendedThreadInfo:
            r = GetCondVarSuspendedThreadInfo( a1, (TMemSpyDriverCondVarSuspendedThreadInfo*)a2 );
            break;

        default:
            r = KErrNotSupported;
		    break;
		    }
        }
    //
    return r;
	}


TBool DMemSpyDriverLogChanContainers::IsHandler( TInt aFunction ) const
    {
    return ( aFunction > EMemSpyDriverOpCodeContainersBase && aFunction < EMemSpyDriverOpCodeContainersEnd );
    }











TInt DMemSpyDriverLogChanContainers::GetContainerHandles( TMemSpyDriverInternalContainerHandleParams* aParams )
	{
    TMemSpyDriverInternalContainerHandleParams params;
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverInternalContainerHandleParams) );
    if  ( r != KErrNone )
        {
    	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles() - END - params read error: %d", r));
        return r;
        }

	const TInt maxCount = params.iMaxCount;
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles() - START - tid or pid: %d, maxCount: %d, container: %d, handleSource: %d", params.iTidOrPid, maxCount, params.iContainer, params.iHandleSource ));

    DMemSpyDriverOSAdaptionDThread& threadAdaption = OSAdaption().DThread();
    DMemSpyDriverOSAdaptionDProcess& processAdaption = OSAdaption().DProcess();

    ResetTempHandles();

    if  ( params.iTidOrPid == KMemSpyDriverEnumerateContainerHandles )
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - enumerating container handles... " ));
	    NKern::ThreadEnterCS();

        // We are enumerating an entire container - not a thread-specific request
        const TObjectType type = ObjectTypeFromMemSpyContainerType( params.iContainer );

        DObjectCon* container = Kern::Containers()[type];
        container->Wait();
        NKern::LockSystem();

        const TInt count = container->Count();
        for(TInt i=0; i<count; i++)
            {
            DObject* object = (*container)[ i ];
            if  ( object != NULL )
                {
                AddTempHandle( object );
                }
            }

        NKern::UnlockSystem();
        container->Signal();

    	NKern::ThreadLeaveCS();
        }
    else
        {
        // Are we dealing with threads or processes?
        const TObjectType sourceContainerType = (TObjectType) params.iHandleSource;
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - sourceContainerType: %d", sourceContainerType ));

        // We need to open the process or thread in order to get its handle
        r = OpenTempObject( params.iTidOrPid, sourceContainerType );
        if  ( r == KErrNone )
		    {
            TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - handle source object: %O", TempObject() ));

            // This is the source for the handles we want to check. Its either the process or thread's handle index...
            MemSpyObjectIx* handles = NULL;

            // Check that the process' thread's are suspended and get handle index
            if  ( params.iHandleSource == EMemSpyDriverThreadOrProcessTypeThread )
                {
                DThread* thread = (DThread*) TempObject();
                if  ( !SuspensionManager().IsSuspended( *thread ) )
                    {
                    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - parent process not suspended => KErrAccessDenied"));
                    r = KErrAccessDenied;
                    }
                else
                    {
                    handles = threadAdaption.GetHandles( *thread );
                    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - checking handles for thread: %O", thread ));
                    }
                }
            else if ( params.iHandleSource == EMemSpyDriverThreadOrProcessTypeProcess )
                {
                DProcess* process = (DProcess*) TempObject();
                if  ( !SuspensionManager().IsSuspended( *process ) )
                    {
                    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - process not suspended => KErrAccessDenied"));
                    r =KErrAccessDenied;
                    }
                else
                    {
                    handles = processAdaption.GetHandles( *process );
                    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - checking handles for process: %O", process ));
                    }
                }

            if  ( handles )
                {
	            NKern::ThreadEnterCS();

                // Iterate through each handle in the thread/process and add it to the temp handles container if
                // the handle is of the correct type.
                MemSpyObjectIx_Wait( handles );

	            TInt handleCount = handles->Count();
                TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - %d handles in index...", handleCount ));

                for( TInt handleIndex=0; handleIndex<handleCount; handleIndex++ )
    	            {
    	            // Get a handle from the container...
                    NKern::LockSystem();
    	            DObject* objectToSearchFor = (*handles)[ handleIndex ];
                    NKern::UnlockSystem();
        
                    if  ( objectToSearchFor != NULL )
                        {
                        // Check to see if this object is of the specified type. We can use quick mode
                        // because we know the object is valid since it's registered as a handle of the
                        // thread/process.
                        DObject* matchResult = CheckIfObjectIsInContainer( params.iContainer, objectToSearchFor, ETrue );
                        if  ( matchResult )
                            {
                            // Found a match in the specified container. Write the object's handle (aka the object address)
                            // back to the client address space
                            AddTempHandle( matchResult );
                            }
                        }
    	            }

                MemSpyObjectIx_Signal( handles );
                NKern::ThreadLeaveCS();
                }

            // Done with this now.
            CloseTempObject();
            }
        else
		    {
    	    Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - thread / process not found");
		    }
        }

    // Write back handles to client
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles - trying to write %d handles to client...", TempHandleCount() ));
    r = WriteToClient( params.iHandles, params.iCountPtr, maxCount );

	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetContainerHandles() - END - r: %d", r));
	return r;
    }


TInt DMemSpyDriverLogChanContainers::GetGenericHandleInfo( TInt aTid, TMemSpyDriverHandleInfoGeneric* aParams )
    {
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - START" ));

    TMemSpyDriverHandleInfoGeneric params;
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverHandleInfoGeneric) );
    if  ( r != KErrNone )
        {
    	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - END - params read error: %d", r));
        return r;
        }

	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo  - tid: %d, handle: 0x%08x", aTid, params.iHandle ));
    
    if  ( aTid == KMemSpyDriverEnumerateContainerHandles )
        {
	    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo - getting info for non-thread specific handle..."));
        }
    else
        {
	    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo - thread-specific request..."));

        r = OpenTempObject( aTid, EThread );
	    if (r != KErrNone)
		    {
    	    Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - END - thread not found");
		    return r;
		    }
    
        // Check that the process' thread's are suspended
        DThread* thread = (DThread*) TempObject();
	    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo - opened thread: %O", thread));
        if  ( !SuspensionManager().IsSuspended( *thread ) )
            {
            TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - END - parent process not suspended => KErrAccessDenied"));
            CloseTempObject();
            return KErrAccessDenied;
            }
        }

	NKern::ThreadEnterCS();

    // First, locate the specific DObject in question. Cast the handle, but don't use the object...
    DObject* handleAsObject = (DObject*) params.iHandle;
    handleAsObject = CheckIfObjectIsInContainer( params.iType, handleAsObject );
    if  ( handleAsObject != NULL )
        {
        // We found the right object. First get generic info.
        handleAsObject->FullName( params.iName );
        handleAsObject->Name( params.iNameDetail );
        
        NKern::LockSystem();
        r = handleAsObject->Open();
        NKern::UnlockSystem();
        //
        if  ( r == KErrNone )
            {
            // Using threadAddaption to fetch generic info.
            // Implementations of following get functions are actually in DMemSpyDriverOSAdaptionDObject
            // so it does not matter what adaption to use for generic info.
            DMemSpyDriverOSAdaptionDThread& threadAddaption = OSAdaption().DThread();
            params.iAccessCount = threadAddaption.GetAccessCount( *handleAsObject );
            params.iUniqueID = threadAddaption.GetUniqueID( *handleAsObject );
            params.iProtection = threadAddaption.GetProtection( *handleAsObject );
            params.iAddressOfKernelOwner = threadAddaption.GetAddressOfKernelOwner( *handleAsObject );
            //
            handleAsObject->Close( NULL );
            }
        
        // Get type-specific info.
        if  ( params.iType == EMemSpyDriverContainerTypeThread )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeThread" ));

            DThread* object = (DThread*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDThread& threadAdaption = OSAdaption().DThread();
                //
                params.iId = threadAdaption.GetId( *object );
                params.iPriority = threadAdaption.GetPriority( *object );
                params.iAddressOfOwningProcess = threadAdaption.GetAddressOfOwningProcess( *object );
                threadAdaption.GetNameOfOwningProcess( *object, params.iNameOfOwner );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeProcess )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeProcess" ));

            DProcess* object = (DProcess*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDProcess& processAdaption = OSAdaption().DProcess();
                //
                params.iId = processAdaption.GetId( *object );
                //
                params.iPriority = processAdaption.GetPriority( *object );
                params.iAddressOfOwningProcess = processAdaption.GetAddressOfOwningProcess( *object );
                params.iCreatorId = processAdaption.GetCreatorId( *object );
                params.iSecurityZone = processAdaption.GetSecurityZone( *object );
                params.iAttributes = processAdaption.GetAttributes( *object );
                params.iAddressOfDataBssStackChunk = processAdaption.GetAddressOfDataBssStackChunk( *object );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeChunk )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeChunk" ));

            DChunk* object = (DChunk*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDChunk& ca = OSAdaption().DChunk();
                //
                params.iSize = ca.GetSize( *object );
                params.iId = ca.GetOwningProcessId( *object );
                params.iAddressOfOwningProcess = ca.GetAddressOfOwningProcess( *object );
                params.iMaxSize = ca.GetMaxSize( *object );
                params.iBottom = ca.GetBottom( *object );
                params.iTop = ca.GetTop( *object );
                params.iAttributes = ca.GetAttributes( *object );
                params.iStartPos = ca.GetStartPos( *object );
                params.iControllingOwner = ca.GetControllingOwnerId( *object );
                params.iRestrictions = ca.GetRestrictions( *object );
                params.iMapAttr = ca.GetMapAttr( *object );
                params.iChunkType = ca.GetType( *object );
                ca.GetNameOfOwningProcess( *object, params.iNameOfOwner );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeLibrary )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeLibrary" ));

	        Kern::AccessCode();
            //
            DLibrary* object = (DLibrary*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDCodeSeg& csa = OSAdaption().DCodeSeg();
                DCodeSeg* codeSeg = csa.GetCodeSeg( *object );
                params.iAddressOfCodeSeg = (TUint8*)codeSeg;
                params.iMapCount = csa.GetMapCount( *object );
                params.iState = csa.GetState( *object );
                //
                if  ( codeSeg )
                    {
                    params.iSize = csa.GetSize( *codeSeg );
                    }
                else
                    {
                    r = KErrNotFound;
                    }
                //
                object->Close( NULL );
                }
            //
	        Kern::EndAccessCode();
            }
        else if ( params.iType == EMemSpyDriverContainerTypeSemaphore )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeSemaphore" ));

            DSemaphore* object = (DSemaphore*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDSemaphore& sa = OSAdaption().DSemaphore();
                params.iCount = sa.GetCount( *object );
                params.iResetting = sa.GetResetting( *object );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeMutex )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeMutex" ));

            DMutex* object = (DMutex*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDMutex& ma = OSAdaption().DMutex();
                //
                params.iCount = ma.GetHoldCount( *object );
                params.iWaitCount = ma.GetWaitCount( *object );
                params.iResetting = ma.GetResetting( *object );
                params.iOrder = ma.GetOrder( *object );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeTimer )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeTimer" ));

            // Get timer properties
            DTimer* object = (DTimer*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDTimer& ta = OSAdaption().DTimer();
                //
                params.iTimerType = MapToMemSpyTimerType( ta.GetType( *object ) );
                params.iTimerState = MapToMemSpyTimerState( ta.GetState( *object ) );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeServer )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeServer" ));

            DServer* object = (DServer*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDServer& sa = OSAdaption().DServer();
                //
                params.iCount = sa.GetSessionCount( *object );
                params.iId = sa.GetOwningThreadId( *object );
                params.iSessionType = sa.GetSessionType( *object );
                params.iAddressOfOwningThread = sa.GetAddressOfOwningThread( *object );
                sa.GetNameOfOwningThread( *object, params.iNameOfOwner );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeSession )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeSession" ));

            DSession* object = (DSession*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - open error: %d", r ));
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDServer& serverAdaption = OSAdaption().DServer();
                DMemSpyDriverOSAdaptionDSession& sessionAdaption = OSAdaption().DSession();

                params.iName.Zero();

                TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - getting session type..." ));
                params.iSessionType = sessionAdaption.GetSessionType( *object );
                params.iAddressOfServer = sessionAdaption.GetAddressOfServer( *object );
                params.iTotalAccessCount = sessionAdaption.GetTotalAccessCount( *object );
                params.iSvrSessionType = sessionAdaption.GetSessionType( *object );
                params.iMsgCount = sessionAdaption.GetMsgCount( *object );
                params.iMsgLimit = sessionAdaption.GetMsgLimit( *object );
                
                // Its more useful in this instance, if the name object
                // points to the server which the session is connected to
                // (rather than displaying a process-local name).
                DServer* server = sessionAdaption.GetServer( *object );
	            TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - getting full name, server: 0x%08x", server ));
                //
                if  ( server )
                    {
	                NKern::LockSystem();
                    r = server->Open();
     	            NKern::UnlockSystem();

                    if  ( r == KErrNone )
                        {
                        server->FullName( params.iName );
 
                        // Continue as normal for other items
	                    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - server: 0x%08x, server->iOwningThread: 0x%08x", server, server->iOwningThread ));
                        DThread* owningThread = serverAdaption.GetOwningThread( *server );
                        if  ( owningThread )
                            {
	                        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - getting server thread id..." ));
                            params.iId = serverAdaption.GetOwningThreadId( *server );
                            }

                        server->Close( NULL );
                        }
                    }

                TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - closing session object..." ));
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeLogicalDevice )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeLogicalDevice" ));

            DLogicalDevice* object = (DLogicalDevice*) handleAsObject;
	        NKern::LockSystem();
            r = object->Open();
    	    NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDLogicalDevice& lda = OSAdaption().DLogicalDevice();
                params.iOpenChannels = lda.GetOpenChannels( *object );
                params.iVersion = lda.GetVersion( *object );
                params.iParseMask = lda.GetParseMask( *object );
                params.iUnitsMask = lda.GetUnitsMask( *object );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypePhysicalDevice )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypePhysicalDevice" ));
	        
	        DPhysicalDevice* object = (DPhysicalDevice*) handleAsObject;
            NKern::LockSystem();
            r = object->Open();
            NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDPhysicalDevice& pda = OSAdaption().DPhysicalDevice();
                params.iVersion = pda.GetVersion( *object );
                params.iUnitsMask = pda.GetUnitsMask( *object );
                params.iAddressOfCodeSeg = pda.GetAddressOfCodeSeg( *object );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeLogicalChannel )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeLogicalChannel" ));
            }
        else if ( params.iType == EMemSpyDriverContainerTypeChangeNotifier )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeChangeNotifier" ));

	        DChangeNotifier* object = (DChangeNotifier*) handleAsObject;
            NKern::LockSystem();
            r = object->Open();
            NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDChangeNotifier& cna = OSAdaption().DChangeNotifier();
                params.iChanges = cna.GetChanges( *object );
                params.iAddressOfOwningThread = cna.GetAddressOfOwningThread( *object );
                cna.GetNameOfOwningThread( *object, params.iNameOfOwner );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeUndertaker )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeUndertaker" ));
	        
            DUndertaker* object = (DUndertaker*) handleAsObject;
            NKern::LockSystem();
            r = object->Open();
            NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDUndertaker& uta = OSAdaption().DUndertaker();
                params.iAddressOfOwningThread = uta.GetAddressOfOwningThread( *object );
                uta.GetNameOfOwningThread( *object, params.iNameOfOwner );
                //
                object->Close( NULL );
                }
            }
        else if ( params.iType == EMemSpyDriverContainerTypeMsgQueue )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeMsgQueue" ));
            }
        else if ( params.iType == EMemSpyDriverContainerTypePropertyRef )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypePropertyRef" ));
            }
        else if ( params.iType == EMemSpyDriverContainerTypeCondVar )
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - EMemSpyDriverContainerTypeCondVar" ));
	        
            DCondVar* object = (DCondVar*) handleAsObject;
            NKern::LockSystem();
            r = object->Open();
            NKern::UnlockSystem();
            //
            if  ( r == KErrNone )
                {
                DMemSpyDriverOSAdaptionDCondVar& cva = OSAdaption().DCondVar();
                params.iResetting = cva.GetResetting( *object );
                params.iAddressOfOwningThread = cva.GetAddressOfMutex( *object );
                cva.GetNameOfMutex( *object, params.iNameOfOwner );
                params.iWaitCount = cva.GetWaitCount( *object );
                //
                object->Close( NULL );
                }
            }
        else
            {
	        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - KErrNotSupported" ));
            r = KErrNotSupported;
            }
        }
    else
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo - couldn not find object => KErrNotFound"));
        r = KErrNotFound;
        }

	NKern::ThreadLeaveCS();

    if  ( r == KErrNone )
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo - writing back to client thread..."));
        r = Kern::ThreadRawWrite( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverHandleInfoGeneric) );
        }
    if  ( TempObject() != NULL )
        {
    	CloseTempObject();
        }

	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetGenericHandleInfo() - END - r: %d", r));
	return r;
    }


TInt DMemSpyDriverLogChanContainers::GetKernelObjectSizeApproximation( TMemSpyDriverContainerType aType, TInt* aSize )
    {
    TInt r = KErrNone;
    //
	TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetKernelObjectSizeApproximation() - START - aType: %d", aType));
    const TObjectType objectType = ObjectTypeFromMemSpyContainerType( aType );
    //
    TInt size = 0;
    //
    switch( objectType )
        {
    case EThread:
        size = sizeof(DThread);
        break;
    case EProcess:
        size = sizeof(DProcess);
        break;
    case EChunk:
        size = sizeof(DChunk);
        break;
    case ELibrary:
        size = sizeof(DLibrary);
        break;
    case ESemaphore:
        size = sizeof(DSemaphore);
        break;
    case EMutex:
        size = sizeof(DMutex);
        break;
    case ETimer:
        size = sizeof(DTimer);
        break;
    case EServer:
        size = sizeof(DServer);
        break;
    case ESession:
        size = sizeof(DSession);
        break;
    case ELogicalDevice:
        size = sizeof(DLogicalDevice);
        break;
    case EPhysicalDevice:
        size = sizeof(DPhysicalDevice);
        break;
    case ELogicalChannel:
        size = sizeof(DLogicalChannel);
        break;
    case EChangeNotifier:
        size = sizeof(DChangeNotifier);
        break;
    case EUndertaker:
        size = sizeof(DUndertaker);
        break;
    case EMsgQueue:
        size = sizeof(DObject); // best we can do!
        break;
    case EPropertyRef:
        size = sizeof(DObject); // best we can do!
        break;
    case ECondVar:
        size = sizeof(DCondVar);
        break;
    default:
        break;
        }
    //
    NKern::ThreadEnterCS();
    r = Kern::ThreadRawWrite( &ClientThread(), aSize, &size, sizeof(TInt) );
	NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetKernelObjectSizeApproximation() - END - r: %d, size; %d", r, size));
	return r;
    }














TInt DMemSpyDriverLogChanContainers::GetReferencesToMyThread( TUint aTid, TDes8* aBufferSink )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetReferencesToMyThread() - START - aTid: %d", aTid ));

    // We open the source thread or process, just to ensure it doesn't die underneath us...
    TInt r = OpenTempObject( aTid, EThread );
    if  ( r == KErrNone )
		{
		TInt count = 0;
		
        // Open stream
        RMemSpyMemStreamWriter stream = OpenXferStream();

        // Get references to this thread by searching the handles of other threads
        TInt32* pCountMarkerThread = stream.WriteInt32( 0 );
        count = SearchThreadsFor( TempObject(), stream );
        *pCountMarkerThread = count;

        // Get references to this thread by searching the handles of other processes
        TInt32* pCountMarkerProcess = stream.WriteInt32( 0 );
        count = SearchProcessFor( TempObject(), stream );
        *pCountMarkerProcess = count;

        // Tidy up
        r = stream.WriteAndClose( aBufferSink );

        CloseTempObject();
        }
                
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetReferencesToMyThread() - END - r: %d", r));
	return r;
    }


TInt DMemSpyDriverLogChanContainers::GetReferencesToMyProcess( TUint aPid, TDes8* aBufferSink )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetReferencesToMyProcess() - START - aPid: %d", aPid ));

    // We open the source thread or process, just to ensure it doesn't die underneath us...
    TInt r = OpenTempObject( aPid, EProcess );
    if  ( r == KErrNone )
		{
		TInt count = 0;
		
        // Open stream
        RMemSpyMemStreamWriter stream = OpenXferStream();

        // Get references to this thread by searching the handles of other threads
        TInt32* pCountMarkerThread = stream.WriteInt32( 0 );
        count = SearchThreadsFor( TempObject(), stream );
        *pCountMarkerThread = count;

        // Get references to this thread by searching the handles of other processes
        TInt32* pCountMarkerProcess = stream.WriteInt32( 0 );
        count = SearchProcessFor( TempObject(), stream );
        *pCountMarkerProcess = count;

        // Tidy up
        r = stream.WriteAndClose( aBufferSink );

        CloseTempObject();
        }
                
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetReferencesToMyProcess() - END - r: %d", r));
	return r;
    }


TInt DMemSpyDriverLogChanContainers::GetPAndSInfo( DObject* aHandle, TMemSpyDriverPAndSInfo* aInfo )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetPAndSInfo() - START - aHandle: 0x%08x", aHandle ));

    TInt r = KErrNotFound;
	NKern::ThreadEnterCS();

    // First, locate the specific DObject in question. Cast the handle, but don't use the object...
    // NB: This claims the system lock
    DObject* object = CheckIfObjectIsInContainer( EMemSpyDriverContainerTypePropertyRef, aHandle );
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetPAndSInfo() - handle search returned: 0x%08x", object ));

    if  ( object != NULL )
        {
        NKern::LockSystem();

        DMemSpyDriverOSAdaptionDPropertyRef& pra = OSAdaption().DPropertyRef();
        const TBool isReady = pra.GetIsReady( *object );
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetPAndSInfo() - isReady: %d", isReady ));

        if  ( isReady )
            {
            TMemSpyDriverPAndSInfo params;
            //
            params.iType = pra.GetType( *object );
            params.iCategory = pra.GetCategory( *object );
            params.iKey = pra.GetKey( *object );
            params.iRefCount = pra.GetRefCount( *object );
            params.iTid = pra.GetThreadId( *object );
            params.iCreatorSID = pra.GetCreatorSID( *object );
            //
            TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetPAndSInfo - writing back to client thread..."));
            r = Kern::ThreadRawWrite( &ClientThread(), aInfo, &params, sizeof( TMemSpyDriverPAndSInfo ) );
            }
        else
            {
            r = KErrNotReady;
            }

        NKern::UnlockSystem();
        }

    NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::GetPAndSInfo() - END - r: %d", r));
    return r;
    }


TInt DMemSpyDriverLogChanContainers::GetCondVarSuspendedThreads( TMemSpyDriverInternalCondVarSuspendedThreadParams* aParams )
    {
    
    TMemSpyDriverInternalCondVarSuspendedThreadParams params;
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverInternalCondVarSuspendedThreadParams) );
    if  ( r != KErrNone )
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrs() - END - params read error: %d", r));
        return r;
        }
    
    DMemSpyDriverOSAdaptionDCondVar& condVarAdaption = OSAdaption().DCondVar();
    
    const TInt maxCount = params.iMaxCount;
    NKern::ThreadEnterCS();
    
    DObject* condVarHandle = (DObject*) params.iCondVarHandle;
    condVarHandle = CheckIfObjectIsInContainer( EMemSpyDriverContainerTypeCondVar, condVarHandle );
    if  ( condVarHandle == NULL )
        {
        Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrs() - END - condVar not found");
        NKern::ThreadLeaveCS();
        return KErrNotFound;
        }
    
    ResetTempHandles();
        
    DCondVar* condVar = (DCondVar*) params.iCondVarHandle;;
    
    NKern::LockSystem();

    // Iterate through suspended threads, writing back thread pointer (handle)
    // to client
    SDblQue& suspQueue = condVarAdaption.GetSuspendedQ( *condVar );
    SDblQueLink* anchor = &suspQueue.iA;
    SDblQueLink* link = suspQueue.First();
    while( link != anchor )
        {
        DThread* thread = condVarAdaption.GetThread( link );

        // Found a match in the specified container. Write the object's handle (aka the object address)
        // back to the client address space
        if  ( thread )
            {
            AddTempHandle( thread );
            }

        // Get next item
        link = link->iNext;
        }

    NKern::UnlockSystem();

    // This variable holds the number of handles that we have already
    // written to the client-side.
    TInt currentWriteIndex = 0;
    const TInt handleCount = TempHandleCount();
    TRACE( Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrs - writing %d handles to client...", handleCount ) );
    for( ; currentWriteIndex<handleCount && r == KErrNone && currentWriteIndex < maxCount; )
        {
        TAny* handle = TempHandleAt( currentWriteIndex );
        r = Kern::ThreadRawWrite( &ClientThread(), params.iThrHandles + currentWriteIndex, &handle, sizeof(TAny*) );
        if  (r == KErrNone)
            {
            ++currentWriteIndex;
            }
        }

    if  ( r == KErrBadDescriptor )
        {
        MemSpyDriverUtils::PanicThread( ClientThread(), EPanicBadDescriptor );
        }
    else
        {
        const TInt finalWrite = Kern::ThreadRawWrite( &ClientThread(), params.iThrCountPtr, &currentWriteIndex, sizeof(TInt) );
        if  ( r == KErrNone )
            {
            r = finalWrite;
            }
        }

    NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrs() - END - r: %d", r));
    return r;
    }


TInt DMemSpyDriverLogChanContainers::GetCondVarSuspendedThreadInfo( TAny* aThreadHandle, TMemSpyDriverCondVarSuspendedThreadInfo* aParams )
    {
    TMemSpyDriverCondVarSuspendedThreadInfo params;
    
    TInt r = Kern::ThreadRawRead( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverCondVarSuspendedThreadInfo) );
    if  ( r != KErrNone )
        {
        TRACE( Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrInfo() - END - params read error: %d", r));
        return r;
        }
    
    DMemSpyDriverOSAdaptionDThread& threadAdaption = OSAdaption().DThread();

    NKern::ThreadEnterCS();

    DObject* threadHandle = (DObject*) aThreadHandle;
    threadHandle = CheckIfObjectIsInContainer( EMemSpyDriverContainerTypeThread, threadHandle );
    if  ( threadHandle == NULL )
        {
        Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrInfo() - END - thread not found");
        NKern::ThreadLeaveCS();
        return KErrNotFound;
        }

    DThread* thread = (DThread*) threadHandle;
    thread->FullName( params.iName );
    params.iAddress = (TUint8*)thread;

    if  ( r == KErrNone )
        {
        r = Kern::ThreadRawWrite( &ClientThread(), aParams, &params, sizeof(TMemSpyDriverCondVarSuspendedThreadInfo) );
        }
    
    NKern::ThreadLeaveCS();
    
    TRACE( Kern::Printf("DMemSpyDriverLogChanMisc::GetCondVarSuspThrInfo() - END - r: %d", r));
    return r;
    }




































TInt DMemSpyDriverLogChanContainers::SearchThreadsFor( DObject* aHandleToLookFor, RMemSpyMemStreamWriter& aStream )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchThreadsFor() - START" ) );

    TInt matches = 0;
    DMemSpyDriverOSAdaptionDThread& threadAdaption = OSAdaption().DThread();
   
    DObjectCon* container = Kern::Containers()[ EThread ];
    NKern::ThreadEnterCS();
    container->Wait();

    const TInt containerEntryCount = container->Count();
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchThreadsFor - containerEntryCount: %d", containerEntryCount ));

    for( TInt i=0; i<containerEntryCount && !aStream.IsFull(); i++ )
        {
        DThread* thread = (DThread*) (*container)[ i ];
        MemSpyObjectIx* handles = threadAdaption.GetHandles( *thread );
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchThreadsFor - handles: 0x%08x", handles ));

        if  ( handles != NULL )
            {
            MemSpyObjectIx_Wait( handles );
            //
            TInt searchResult = handles->At( aHandleToLookFor );
	        if  ( searchResult != KErrNotFound )
		        {
                TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchThreadsFor - found handle match in [%O]", thread ));
                aStream.WriteUint32( (TUint32) thread );
                ++matches;
		        }
            //
            MemSpyObjectIx_Signal( handles );
            }
        }

    // Finished with thread/process container.
    container->Signal();
    NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchThreadsFor() - END" ) );
    return matches;
    }


TInt DMemSpyDriverLogChanContainers::SearchProcessFor( DObject* aHandleToLookFor, RMemSpyMemStreamWriter& aStream )
    {
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchProcessFor() - START" ) );

    TInt matches = 0;
    DMemSpyDriverOSAdaptionDProcess& processAdaption = OSAdaption().DProcess();
  
    DObjectCon* container = Kern::Containers()[ EProcess ];
    NKern::ThreadEnterCS();
    container->Wait();

    const TInt containerEntryCount = container->Count();
    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchProcessFor - containerEntryCount: %d", containerEntryCount ));

    for( TInt i=0; i<containerEntryCount && !aStream.IsFull(); i++ )
        {
        DProcess* process = (DProcess*) (*container)[ i ];
        MemSpyObjectIx* handles = processAdaption.GetHandles( *process );
        TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchProcessFor - handles: 0x%08x", handles ));

        if  ( handles != NULL )
            {
            MemSpyObjectIx_Wait( handles );
            //
            TInt searchResult = handles->At( aHandleToLookFor );
	        if  ( searchResult != KErrNotFound )
		        {
                TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchProcessFor - found handle match in [%O]", process ));
                aStream.WriteUint32( (TUint32) process );
                ++matches;
		        }
            //
            MemSpyObjectIx_Signal( handles );
            }
        }

    // Finished with thread/process container.
    container->Signal();
    NKern::ThreadLeaveCS();

    TRACE( Kern::Printf("DMemSpyDriverLogChanContainers::SearchProcessFor() - END" ) );
    return matches;
    }    


TMemSpyDriverTimerState DMemSpyDriverLogChanContainers::MapToMemSpyTimerState( TTimer::TTimerState aState )
    {
    TMemSpyDriverTimerState ret = EMemSpyDriverTimerStateUnknown;
    //
    switch( aState )
        {
    case TTimer::EIdle:
        ret = EMemSpyDriverTimerStateIdle;
        break;
    case TTimer::EWaiting:
        ret = EMemSpyDriverTimerStateWaiting;
        break;
    case TTimer::EWaitHighRes:
        ret = EMemSpyDriverTimerStateWaitHighRes;
        break;
    default:
        break;
        }
    //
    return ret;
    }

 
TMemSpyDriverTimerType DMemSpyDriverLogChanContainers::MapToMemSpyTimerType( TTimer::TTimerType aType )
    {
    TMemSpyDriverTimerType ret = EMemSpyDriverTimerTypeUnknown;
    //
    switch( aType )
        {
    case TTimer::ERelative:
    case TTimer::ELocked:
        ret = EMemSpyDriverTimerTypeRelative;
        break;
    case TTimer::EAbsolute:
        ret = EMemSpyDriverTimerTypeAbsolute;
        break;
    case TTimer::EHighRes:
        ret = EMemSpyDriverTimerTypeHighRes;
        break;
    case TTimer::EInactivity:
        ret = EMemSpyDriverTimerTypeInactivity;
        break;
    default:
        break;
        }
    //
    return ret;
    }








