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

#include <e32def.h>
#include <e32btrace.h>

#include "BTraceOstCategoryBitmap.h"
#include "TraceCoreOstHeader.h"
#include "TraceCoreCommon.h"
#include "TraceCore.h"
#include "TraceCoreNotifier.h"
#include "TraceCoreConstants.h"
#include "TraceCoreDebug.h"
#include "TraceCoreSettings.h"
#include "BTraceOstCategoryHandler.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "BTraceOstCategoryBitmapTraces.h"
#endif


//Split: These needed from sos_mon_isi_m.h (which is removed):
#define SOS_MON_TRACECORE_OST_ACTIVATION_REQ 0xD4

// OST message type
#define SOS_MON_TRACECORE_OST_ACTIVATION 1
#define SOS_MON_TRACECORE_OST_ACTIVATION_QUERY 2

/**
 * OST activation message types
 */
#define SOS_MON_TRACECORE_OST_DEACTIVATE 0
#define SOS_MON_TRACECORE_OST_ACTIVATE 1
#define SOS_MON_TRACECORE_OST_DEACTIVATE_ALL 2
#define SOS_MON_TRACECORE_OST_ACTIVATE_ALL 3
#define SOS_MON_TRACECORE_OST_OVERWRITE 4


/**
 * Offset to OST message id
 * Field length 1 byte
 */
const TInt KOstMessageIdOffset = 0;

/**
 * Offset to activation info
 * Field length 1 byte
 */
const TInt KOstActivationInfoOffset = 3;

/**
 * Offset to Number of groups field
 * Field length 4 bytes
 */
const TInt KOstNumberOfGroupsOffset = 4;

/**
 * Offset to Component id
 * Field length 4 bytes
 */
const TInt KOstComponentIdOffset = 8;

/**
 * Offset to 1st Group Id
 * Field length 2 bytes
 */
const TInt KOstGroupIdOffset = 12;

/**
 * Minimum OST frame length
 *  - Message ID            (1 byte)
 *  - Fillers               (2 bytes)
 *  - Activation info       (1 byte)
 *  - Number of groups      (4 bytes)
 *  - Component ID          (4 bytes)
 */
const TInt KOstMinFrameLength = 12;

/**
 * Group id length, 16bits, 2 bytes
 */
const TUint KGroupIdLength = 2;

/**
 * Increase array size
 */
const TUint KOstComponentsArrayGranularity = 8;

/**
 * Maximum activation buffer length
 */
const TUint32 KMaxActivationBufferLength = 1024 * 16;

/**
 * Constants for bit manipulation
 */
const TInt K8BitShift = 8;
const TInt K16BitShift = 16;
const TInt K24BitShift = 24;
const TInt K8BitMask = 0xff;
const TInt K4Bytes = 4;
const TInt K2Bytes = 2;

/**
 * Number of bytes to shift the group ID
 */
#define GRP_SHIFT 16

/**
 * Number of bytes to shift the group ID
 */
#define GRP_MASK 0x00ff0000

/**
 * Dummy handler used when no traces activated
 */
TBool DummyBTraceHandler(TUint32,TUint32,const TUint32,const TUint32,const TUint32,const TUint32,const TUint32,const TUint32)
	{
	return EFalse;
	}

BTrace::THandler oldHandler;

/**
 * Reads 16 bits from buffer
 */
TUint32 Read16(const TUint8* aPtr)
    {
    return ( (TUint32 )*aPtr ) | ( ( (TUint32 )*(aPtr + 1 ) ) << K8BitShift ); // CodForChk_Dis_Magic
    }

/**
 * Reads 32 bits from buffer
 */
TUint32 Read32(const TUint8* aPtr)
    {
    return ( (TUint32 )*aPtr ) |
           ( ( (TUint32 )*(aPtr + 1 ) ) << K8BitShift ) | // CodForChk_Dis_Magic
           ( ( (TUint32 )*(aPtr + 2 ) ) << K16BitShift ) | // CodForChk_Dis_Magic
           ( ( (TUint32 )*(aPtr + 3 ) ) << K24BitShift ); // CodForChk_Dis_Magic
    }

/**
 * Endian swapper for 32-bit variable
 */
#define SWAP32( num ) ( \
    ( ( ( num ) & 0xff000000 ) >> K24BitShift ) | ( ( ( num ) & 0x00ff0000 ) >> K8BitShift  ) | \
    ( ( ( num ) & 0x0000ff00 ) <<  K8BitShift ) | ( ( ( num ) & 0x000000ff ) << K24BitShift ) )

/**
 * Endian swapper for 16-bit variable
 */
#define SWAP16( num ) ( ( ( ( num ) & 0xff00 ) >> K8BitShift ) | ( ( ( num ) & 0x00ff ) << K8BitShift  ) )

/**
 * Constructor
 */
DBTraceOstCategoryBitmap::DBTraceOstCategoryBitmap(DBTraceOstCategoryHandler* aBTraceOstCategoryHandler) :
      iOstComponents(NULL),
      iTmpArray(NULL),
      iChangedArray(NULL),
      iActivationBuffer(NULL),
      iBTraceOstCategoryHandler( aBTraceOstCategoryHandler )
    {
    }

/**
 * Destructor
 */
DBTraceOstCategoryBitmap::~DBTraceOstCategoryBitmap()
    {
    Kern::Free( iActivationBuffer );
    iActivationBuffer = NULL;
    
    CleanupComponentsArrays();
    }

/**
 * Init TraceBitmap
 */
TInt DBTraceOstCategoryBitmap::Init()
    {
    iOstComponents = new RArray<TOstComponent>(KOstComponentsArrayGranularity);
    if(!iOstComponents)
        return KErrNoMemory;
    iTmpArray = new RArray<TOstComponent>(KOstComponentsArrayGranularity);
    if(!iTmpArray)
        {
        CleanupComponentsArrays();
        return KErrNoMemory;
        }
    iChangedArray = new RArray<TOstComponent>(1);
    if(!iChangedArray)
        {
        CleanupComponentsArrays();
        return KErrNoMemory;
        }
    
    AllocateActivationBuffer();
        
    // Init bitmap
    TInt err = DTraceCoreActivationBase::Init( SOS_MON_TRACECORE_OST_ACTIVATION_REQ);
    if (err == KErrNone)
        {
        // Subscribe message
        err = DTraceCoreActivationBase::SubscribeMessage(OstConstants::OstBaseProtocol::KOstTraceActivationProtocol,
                EMessageHeaderFormatOst);

        // Save old BTrace handler and use dummy for better performance
        oldHandler = BTrace::SetHandler(DummyBTraceHandler);
        }
    
    return err;
    }

/**
 * Activate trace group
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 */
void DBTraceOstCategoryBitmap::ActivateTrace(const TUint32 aComponentId, const TUint32 aGroupId)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_ACTIVATETRACE_ENTRY,"> DBTraceOstCategoryBitmap::ActivateTrace CID:0x%x GID:0x%x", aComponentId, aGroupId );
    //TODO: check is thread context (assert)
    //TODO: can be executed only from iActivationQ (__ASSERT_DEBUG)
    
    if (GroupIdIsValid(aGroupId))
      {
    //TODO: check how many users use this array (use atomics to increment/decrement)
      CopyArray<TOstComponent>(iOstComponents, iTmpArray);
      CleanupComponentsArray(*iChangedArray);
      
      TInt index = AddComponentId(aComponentId, *iTmpArray);
      AddGroupId(aComponentId, aGroupId, index, *iTmpArray, *iChangedArray);
      SetNewComponentsArrayPtr( iTmpArray );
      
      //notify internal listeners and update internal activation cache
      HandleTraceActivationChanges(*iChangedArray);
      NotifyInternalActivation( EFalse, aComponentId );
      }
    else
      {
      OstTrace0( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_ACTIVATETRACE_INVALID_GROUP,"DBTraceOstCategoryBitmap::ActivateTrace - Invalid group ID");
      // Notifies activation interfaces of invalid group ID
      DTraceCore* traceCore = DTraceCore::GetInstance();
      if ( traceCore != NULL )
          {
          traceCore->GetNotifier().TraceError( aComponentId, aGroupId, KErrArgument );
          }
      }
    }

/**
 * Deactivate trace group
 *
 * @param aComponentId The component ID
 * @param aGroupId The group ID
 */
void DBTraceOstCategoryBitmap::DeactivateTrace(const TUint32 aComponentId, const TUint32 aGroupId)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_DEACTIVATETRACE_ENTRY,"> DBTraceOstCategoryBitmap::DeactivateTrace. CID:0x%x GID:0x%x", aComponentId, aGroupId );
    if (ComponentIdIsValid(aComponentId) )
        {
        if (GroupIdIsValid(aGroupId))
            {
            
            TInt componentIndex = FindComponentId(aComponentId, *iOstComponents);
            if (componentIndex >= 0)
                {
                CopyArray<TOstComponent>(iOstComponents, iTmpArray);
                CleanupComponentsArray(*iChangedArray);
                RemoveGroupId(aComponentId, aGroupId, componentIndex, *iTmpArray, *iChangedArray);
                if ((*iTmpArray)[ componentIndex ].iGroups.Count() == 0)
                    {
                    RemoveComponentId(aComponentId, EFalse, *iTmpArray, *iChangedArray);
                    }
                
                //Swap pointers.
                SetNewComponentsArrayPtr( iTmpArray );
             
                //notify internal listeners and update internal activation cache
                HandleTraceActivationChanges(*iChangedArray);
                NotifyInternalActivation( EFalse, aComponentId );
                }
            }
        else
            {
            OstTrace0( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_DEACTIVATETRACE_INVALID_GROUP,"DBTraceOstCategoryBitmap::DeactivateTrace - Invalid group ID");
            // Notifies activation interfaces of invalid group ID
            DTraceCore* traceCore = DTraceCore::GetInstance();
            if ( traceCore != NULL )
                {
                traceCore->GetNotifier().TraceError( aComponentId, aGroupId, KErrArgument );
                }
            }
        }
    }

/**
 * Deactivates all groups from a component
 * 
 * @param aComponentId The component ID
 */
void DBTraceOstCategoryBitmap::DeactivateAll(const TUint32 aComponentId)
    {
    OstTrace1( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_DEACTIVATEALL_ENTRY,"> DBTraceOstCategoryBitmap::DeactivateAll. CID:0x%x", aComponentId );

    CopyArray<TOstComponent>(iOstComponents, iTmpArray);
    CleanupComponentsArray(*iChangedArray);
    
    if( RemoveComponentId(aComponentId, ETrue, *iTmpArray, *iChangedArray) != KErrNotFound)
        SetNewComponentsArrayPtr( iTmpArray );
    
    //notify internal listeners and update internal activation cache
    HandleTraceActivationChanges(*iChangedArray);
    NotifyInternalActivation( EFalse, aComponentId );
    }

/**
 * Reads the bitmap from the settings. If the settings does 
 * not have a bitmap, the current bitmap is written to it
 * 
 * @param aSettings the settings
 */
void DBTraceOstCategoryBitmap::ReadFromSettings(DTraceCoreSettings& aSettings)
    {
    // Read buffer
    if ( aSettings.Read( iActivationBuffer, KMaxActivationBufferLength ) )
    	{
        CopyArray<TOstComponent>(iOstComponents, iTmpArray);
        CleanupComponentsArray(*iChangedArray);
        
	    TUint8* ptr = iActivationBuffer;
	    TInt componentcount = Read32FromBuffer(ptr);
	    TC_TRACE( ETraceLevelFlow, Kern::Printf( " DBTraceOstCategoryBitmap::ReadFromSettings() components=%d", componentcount ) );
	    TInt groupcount = 0;
	    TUint32 componentID = 0;
	    TUint16 groupID = 0;
	    TInt index = 0;
	    for ( TInt i = 0; i < componentcount; ++i )
	    	{
	    	// Read number of group IDs (4 bytes)
	    	groupcount = Read32FromBuffer(ptr);
	    	// Read component ID (4 bytes)
	    	componentID = Read32FromBuffer(ptr);
	    	index = AddComponentId(componentID, *iTmpArray);
	    	// Read group IDs (2 bytes each)
	    	for ( TInt k = 0; k < groupcount; ++k )
	    		{
	    		groupID = Read16FromBuffer(ptr);
	            if (GroupIdIsValid(groupID))
	                {
	                AddGroupId(componentID, groupID, index, *iTmpArray, *iChangedArray);
	                }
	            else
	                {
	                // Notifies activation interfaces of invalid group ID
	                DTraceCore* traceCore = DTraceCore::GetInstance();
	                if ( traceCore != NULL )
	                    {
	                    traceCore->GetNotifier().TraceError( componentID, groupID, KErrArgument );
	                    }
	                }            
	    		}
	    	}
	    
	    //swap pointers and clean up
	    SetNewComponentsArrayPtr( iTmpArray );
	    HandleTraceActivationChanges(*iChangedArray);
	    NotifyInternalActivation( ETrue, componentID );
    	}
    }

/**
 * Writes the bitmap to given settings
 * 
 * @param aSettings the settings where bitmap is written
 */
void DBTraceOstCategoryBitmap::WriteToSettings(DTraceCoreSettings& aSettings)
    { 
    TUint8* ptr = iActivationBuffer;
    TInt componentcount = iOstComponents->Count();
    WriteToBuffer(ptr, (TUint32)(componentcount));
    for ( TInt i = 0; i < componentcount; ++i )
        {
        // Add number of group IDs (4 bytes)
        TInt groupcount = (*iOstComponents)[i].iGroups.Count();
        WriteToBuffer(ptr, (TUint32)(groupcount));
        // Add component ID (4 bytes)
        WriteToBuffer(ptr, (*iOstComponents)[i].iComponentId);
        TC_TRACE( ETraceLevelFlow, Kern::Printf( " DBTraceOstCategoryBitmap::WriteToSettings() component=0x%x with %d groups",(*iOstComponents)[i].iComponentId, groupcount ) );
        // Add group IDs (2 bytes each)
        for ( TInt k = 0; k < groupcount; ++k )
            {
            WriteToBuffer(ptr, (TUint16)((*iOstComponents)[i].iGroups[k]) );
            }
        }
    // Save buffer
    aSettings.Write( iActivationBuffer, KMaxActivationBufferLength );
    }

/**
 * Checks if trace points with a specific component ID and group ID are
 * currently active
 *
 * @param aComponentId The component ID
 * @param aTraceWord The trace word containing the group ID to check
 * @return ETrue if trace is active, EFalse if not
 */
TBool DBTraceOstCategoryBitmap::IsTraceActivated(const TUint32 aComponentId, TUint32 aTraceWord)
    {
    // Find component
    TOstComponent ostComponent;
    ostComponent.iComponentId = aComponentId;
    TInt irq = ReadLock();
    TInt index = iOstComponents->FindInUnsignedKeyOrder(ostComponent);
    if (index != KErrNotFound)
        {
        index = (*iOstComponents)[ index ].iGroups.FindInOrder((aTraceWord & GRP_MASK) >> GRP_SHIFT);
        }
    TBool ret = (index != KErrNotFound );
    ReadUnlock(irq);
    return ret;
    }

/**
 * Find component id index in array
 * 
 * @param aComponentId Component id to be searched
 * @param aOstComponents Array to search through
 * @return index of component id in array if found, otherwise KErrNotFound
 */
TInt DBTraceOstCategoryBitmap::FindComponentId(TUint32 aComponentId, RArray<TOstComponent>& aOstComponents)
    {
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_FINDCOMPONENTID_ENTRY,"> DBTraceOstCategoryBitmap::FindComponentId 0x%x", aComponentId );
    TOstComponent tempComponent;
    tempComponent.iComponentId = aComponentId;
    // Check index of component id in array
    TInt index = aOstComponents.FindInUnsignedKeyOrder(tempComponent);
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_FINDCOMPONENTID_EXIT,"< DBTraceOstCategoryBitmap::FindComponentId. Index:%d", index );
    return index;
    }

/**
 * Add component id to array
 * 
 * @param aComponentId Component id to be added
 * @return index of (added of existing) component id in array or an error code
 */
TInt DBTraceOstCategoryBitmap::AddComponentId(TUint32 aComponentId, RArray<TOstComponent>& aOstComponents)
    {
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_ADDCOMPONENTID_ENTRY,"> DBTraceOstCategoryBitmap::AddComponentId 0x%x", aComponentId );
    TOstComponent tempComponent;
    tempComponent.iComponentId = aComponentId;
    TInt index(KErrNotFound);
    // Add component id
    TInt err = aOstComponents.InsertInUnsignedKeyOrder(tempComponent);
    OstTraceExt2( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_ADDCOMPONENTID_INSERT_FAILED,"DBTraceOstCategoryBitmap::AddComponentId - Insert returned. CID:0x%x Err:%d",(TUint)aComponentId, (TInt)err );
    if ( (err == KErrNone ) || (err == KErrAlreadyExists ))
        {
        // Check index of component id in array
        index = aOstComponents.FindInUnsignedKeyOrder(tempComponent);
        }
        
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_ADDCOMPONENTID_EXIT,"< DBTraceOstCategoryBitmap::AddComponentId. Index:%d", index );
    return index;
    }

/**
 * Remove component id in array.
 * 
 * @param aComponentId Component id to be removed
 * @param aNotifyActivationListeners if true, inform activation notification listeners
 * @param aOstComponents array from which component has to be removed 
 * @param aChangedArray Array that will be updated to contain the changed TOstComponent objects
 * @return index of removed component id if found, otherwise KErrNotFound
 */
TInt DBTraceOstCategoryBitmap::RemoveComponentId(TUint32 aComponentId, TBool aNotifyActivationListeners, 
        RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray )
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_REMOVECOMPONENTID_ENTRY, "> DBTraceOstCategoryBitmap::RemoveComponentId 0x%x %d", aComponentId, aNotifyActivationListeners );
    
    // Check index of component id in array
    TInt index = FindComponentId(aComponentId, aOstComponents);
    if (index >= 0)
        {
        // Component id found - Remove it
        aOstComponents.Remove(index);
        
        // Inform about removal if aNotifyActivationListeners is true
        if ( aNotifyActivationListeners )
        	{
            AddGroupToChangedArray(aChangedArray, aComponentId, KAllGroups);
        	}
        }
	
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_REMOVECOMPONENTID_EXIT,"< DBTraceOstCategoryBitmap::RemoveComponentId. Index:%d", index );
    return index;
    }

/**
 * Adds group id to array
 * 
 * @param aComponentId Component where the group belongs
 * @param aGroupId Group id to be added
 * @param aIndex Index of component array where to add the groupId
 * @param aOstComponents Array with TOstComponent objects.
 * @param aChangedArray Array that will be updated to contain the changed TOstComponent objects
 */
TInt DBTraceOstCategoryBitmap::AddGroupId(TUint32 aComponentId, TGroupId aGroupId, TInt aIndex, 
        RArray<TOstComponent>& aOstComponents, RArray<TOstComponent>& aChangedArray)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_ADDGROUPID_ENTRY,"> DBTraceOstCategoryBitmap::AddGroupId. GID:0x%x. Index:%d", (TUint32) aGroupId, (TUint32) aIndex );
    TInt ret = KErrNone;

    if (aIndex >= 0 && aIndex < aOstComponents.Count() )
        {
        // Add group Id
        ret = aOstComponents[ aIndex ].iGroups.InsertInOrder(aGroupId);
        if (ret == KErrNone)
            {
            // Inform notifier that trace has been activated
            DTraceCore* traceCore = DTraceCore::GetInstance();
            if (traceCore != NULL)
                {
                // Don't activate Kernel groups from BTrace here, it will be done in KernelHandler
                if ((TUint) aGroupId > KMaxKernelCategory)
                    {
                    BTrace::SetFilter((TUint) aGroupId, 1);
                    }
                
                // Add group to changed array
                AddGroupToChangedArray(aChangedArray, aComponentId, aGroupId);
                }
            else
                {
                ret = KErrGeneral;
                }
            }
        else
            if (ret == KErrAlreadyExists)
                {
                ret = KErrNone;
                }
        }
    else
        {
        ret = KErrArgument;
        }
    
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_ADDGROUPID_EXIT,"< DBTraceOstCategoryBitmap::AddGroupId. Ret:%d", ret );
    return ret;
    }

/**
 * Removes group id from array
 * 
 * @param aComponentId Component where the group belongs
 * @param aGroupId Group id to be removed
 * @param aIndex Index of component array from where to remove the groupId
 * @param aOstComponents Array with TOstComponent objects.
 * @param aChangedArray Array that will be updated to contain the changed TOstComponent objects
 */
TInt DBTraceOstCategoryBitmap::RemoveGroupId(TUint32 aComponentId, TGroupId aGroupId, TInt aIndex, 
        RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_REMOVEGROUPID_ENTRY,"> DBTraceOstCategoryBitmap::RemoveGroupId. GID:0x%x. Idx:%d", (TUint32) aGroupId, (TUint32) aIndex );
    TInt ret = KErrNone;
    if (aIndex >= 0 && aIndex < aOstComponents.Count() )
        {
        // Component id found, remove group ID
        TInt groupIndex = aOstComponents[ aIndex ].iGroups.FindInOrder(aGroupId);
        if (groupIndex >= 0)
            {
            aOstComponents[ aIndex ].iGroups.Remove(groupIndex);
            // Inform notifier that trace has been deactivated
            DTraceCore* traceCore = DTraceCore::GetInstance();
            if (traceCore != NULL)
                {
                BTrace::SetFilter((TUint) aGroupId, 0);                        
                OstTrace1( TRACE_INTERNAL, DBTRACEOSTCATEGORYBITMAP_REMOVEGROUPID_NOTIFIER_DEACTIVATED,"DBTraceOstCategoryBitmap::RemoveGroupId - Group deactivated, calling notifier. GID:0x%x",(TUint32) aGroupId );
                
                AddGroupToChangedArray(aChangedArray, aComponentId, aGroupId);                
                ret = KErrNone;
                }
            else
                {
                ret = KErrGeneral;
                }
            }
        else
            {
            ret = KErrNone;
            }
        }
    else
        {
        ret = KErrArgument;
        }

    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_REMOVEGROUPID_EXIT,"< DBTraceOstCategoryBitmap::RemoveGroupId. Ret:%d", ret );
    return ret;
    }

/**
 * Read OST Protocol frame from OST protocol message.
 * 
 * @pre On entry to this methods writer mutex (iActivationWriteLock) must be held.
 * 
 * @param aMessage Received message
 * @param aFrameStartIndex Start offset of the frame in message
 * @return Offset after read frame
 */
TInt DBTraceOstCategoryBitmap::ReadOstProtocolFrame(const TDesC8& aMessage, TUint32 aFrameStartIndex)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTPROTOCOLFRAME_ENTRY, "< DBTraceOstCategoryBitmap::ReadOstProtocolFrame. aMessage:%s, aFrameStartIndex:%u", aMessage,aFrameStartIndex );
    
    //__ASSERT_MUTEX(iActivationWriteLock);
    TInt retPosition = 0;
    TInt length = aMessage.Length();

    TInt ostBaseHeaderSize = OstHeader::OstBaseProtocol::KOstBaseHeaderSize;
    TInt applicationIdOffset = OstHeader::OstTraceActivation::SetApplicationStatusRequest::KApplicationIdOffset-ostBaseHeaderSize;
    TInt activationStatusOffset = 
                      OstHeader::OstTraceActivation::SetApplicationStatusRequest::KActivationStatusOffset-ostBaseHeaderSize;
    
    TInt groupIdOffset = OstHeader::OstTraceActivation::SetApplicationStatusRequest::KGroupIdOffset-ostBaseHeaderSize;
    TInt activated = OstConstants::OstTraceActivation::ActivationStatus::KApplicationTracesActivated;
    TInt deactivated = OstConstants::OstTraceActivation::ActivationStatus::KApplicationTracesDeactivated;

    // Checks that buffer is large enough for the frame
    if (length >= (activationStatusOffset +1 /*1==activatin status  size*/- ostBaseHeaderSize ))
        {
        const TUint8* frameStart = aMessage.Ptr() + aFrameStartIndex;
        // Check if activation or deactivation
        TUint8 activationStatus = *(frameStart + activationStatusOffset );

        if ( (activationStatus == activated ) || (activationStatus == deactivated ))
            {
            // One frame may contain multiple group ID's. Each occupy 2 bytes
            TUint32 numberOfGroups = (length-groupIdOffset)/2; // CodForChk_Dis_Magic

            if ((length-groupIdOffset)%2!=0) // CodForChk_Dis_Magic
                {
                numberOfGroups--;
                }
            // One frame contains one component id
            TUint32 applicationId = Read32(frameStart + applicationIdOffset);

            // Swap byte order
            applicationId = SWAP32( applicationId );
            
            CopyArray<TOstComponent>(iOstComponents, iTmpArray);
            CleanupComponentsArray(*iChangedArray);
            
            TInt index(0); 
            if (activationStatus == activated)
                {
                index = AddComponentId(applicationId, *iTmpArray);
                }
            else
                {
                index = FindComponentId(applicationId, *iTmpArray);
                }

            if (index >= 0)
                {
                // Updates the group ID's that have changed
                retPosition = ReadOstProtocolFrameGroups(aMessage, aFrameStartIndex, activationStatus, index, applicationId, *iTmpArray, *iChangedArray);
                aFrameStartIndex = retPosition;
                }
            else
                {
                OstTrace1( TRACE_INTERNAL, DBTRACEOSTCATEGORYBITMAP_READOSTFRAME_NOT_FOUND,"DBTraceOstCategoryBitmap::ReadOstFrame - Component ID not found: 0x%x", applicationId );
                }
            
            //swap pointers and cleanup
            SetNewComponentsArrayPtr(iTmpArray);
           
            //notify internal listeners and update internal activation cache
            HandleTraceActivationChanges(*iChangedArray);
            NotifyInternalActivation( EFalse, applicationId );
            
            }
        }

    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTFRAME_EXIT, "< DBTraceOstCategoryBitmap::ReadOstFrame. Ret:%d", retPosition );
    return retPosition;
    }

/**
 * Read the groups from an OST frame
 * 
 * @param aMessage Received message
 * @param aFrameStartIndex Start offset of the frame in message
 * @param aActivationInfo Activation / deactivation flag
 * @param aComponentIndex The index of the component where the groups belong to
 * @param aComponentId The component ID
 * @param aOstComponents Array with TOstComponent objects.
 * @param aChangedArray Array that will be updated to contain the changed TOstComponent objects
 * @return Offset after read frame
 */
TInt DBTraceOstCategoryBitmap::ReadOstProtocolFrameGroups(const TDesC8& aMessage, TInt aFrameStartIndex, TUint8 aActivationInfo,
        TInt aComponentIndex, TUint32 aComponentId, RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray)
    {
    OstTraceExt3( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTPROTOCOLFRAMEGROUPS_ENTRY,"< DBTraceOstCategoryBitmap::ReadOstProtocolFrameGroups. aMessage:%s, aFrameStartIndex:%u, aActivationInfo:%u", aMessage,aFrameStartIndex, aActivationInfo );
   
    TInt length(aMessage.Length() );
    TInt ostBaseHeaderSize = OstHeader::OstBaseProtocol::KOstBaseHeaderSize;
    TInt groupIdOffset = OstHeader::OstTraceActivation::SetApplicationStatusRequest::KGroupIdOffset-ostBaseHeaderSize;
    TInt activated = OstConstants::OstTraceActivation::ActivationStatus::KApplicationTracesActivated;
    TInt deactivated = OstConstants::OstTraceActivation::ActivationStatus::KApplicationTracesDeactivated;

    const TUint8* frameStart = aMessage.Ptr() + aFrameStartIndex;
    
    TInt offset = groupIdOffset;
    TInt retPosition( 0);
    
    // One frame may contain multiple group ID's. Each occupy 2 bytes
    TUint32 numberOfGroups = (length-groupIdOffset) / 2; // CodForChk_Dis_Magic
    
    if ((length-groupIdOffset) % 2 != 0) // CodForChk_Dis_Magic
        {
        numberOfGroups--;
        }

    TBool swapGroups = ETrue;
    // This is supporting both byte orders
    if (numberOfGroups > KMaxGroupId)
        {
        swapGroups = ETrue;
        numberOfGroups = SWAP32( numberOfGroups );
        }
    
    // Read Group ids and remove from array
    for (TUint i = 0; i < numberOfGroups; i++)
        {
        TUint16 groupId = Read16(frameStart + offset);
        if (swapGroups)
            {
            groupId = SWAP16( groupId );
            }
        offset += KGroupIdLength;
        if (GroupIdIsValid(groupId)) 
        	{
			if (aActivationInfo == activated)
				{
				AddGroupId(aComponentId, groupId, aComponentIndex, aOstComponents, aChangedArray);
				}
			else
				{
				RemoveGroupId(aComponentId, groupId, aComponentIndex, aOstComponents, aChangedArray);
				}
        	}
        else
            {
            // Notifies activation interfaces of invalid group ID
            DTraceCore* traceCore = DTraceCore::GetInstance();
            if ( traceCore != NULL )
                {
                traceCore->GetNotifier().TraceError( aComponentId, groupId, KErrArgument );
                }
            }
        }
    // If all groups were removed from the component, it can also be removed
    if ( (aActivationInfo == deactivated ) && (aOstComponents[ aComponentIndex ].iGroups.Count() == 0 ))
        {
        RemoveComponentId(aComponentId, EFalse, aOstComponents, aChangedArray);
        }
    // The next frame offset is returned
    retPosition = aFrameStartIndex + offset;
        
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTPROTOCOLFRAMEGROUPS_EXIT,"< DBTraceOstCategoryBitmap::ReadOstFrameGroups. Ret:%d", retPosition );
    return retPosition;
    }

/**
 * Read OST frame from ISI message.
 * 
 * @pre On entry to this methods writer mutex (iActivationWriteLock) must be held.
 * 
 * @param aMessage Received message
 * @param aFrameStartIndex Start offset of the frame in message
 * @return Offset after read frame
 */
TInt DBTraceOstCategoryBitmap::ReadOstFrame(const TDesC8& aMessage, TUint32 aFrameStartIndex)
    {
    TInt retPosition = 0;
    TUint length = aMessage.Length();
    // Checks that buffer is large enough for the frame
    if (length >= (aFrameStartIndex + KOstMinFrameLength ))
        {
        const TUint8* frameStart = aMessage.Ptr() + aFrameStartIndex;
        // Check if activation or deactivation
        TUint8 activationInfo = *(frameStart + KOstActivationInfoOffset );
        
        if ( (activationInfo == SOS_MON_TRACECORE_OST_DEACTIVATE ) || (activationInfo == SOS_MON_TRACECORE_OST_ACTIVATE )
                || (activationInfo == SOS_MON_TRACECORE_OST_OVERWRITE ))
            {
            // One frame may contain multiple group ID's. Each occupy 2 bytes
            (void) Read32(frameStart + KOstNumberOfGroupsOffset); // This is the numberOfGroups
            // One frame contains one component id
            TUint32 componentId = Read32(frameStart + KOstComponentIdOffset);
            // Component Id in message is in Big Endian format so it need to be swap to Little Endian format
            componentId = SWAP32( componentId );

            CopyArray<TOstComponent>(iOstComponents, iTmpArray);
            CleanupComponentsArray(*iChangedArray);
            
            TInt index( 0);
            if ( (activationInfo == SOS_MON_TRACECORE_OST_ACTIVATE ) || (activationInfo == SOS_MON_TRACECORE_OST_OVERWRITE ))
                {
                index = AddComponentId(componentId, *iTmpArray);
                }
            else
                {
                index = FindComponentId(componentId, *iTmpArray);
                }
            if (index >= 0)
                {
                // Updates the group ID's that have changed
                retPosition = ReadOstFrameGroups(aMessage, aFrameStartIndex, activationInfo, index, componentId, *iTmpArray, *iChangedArray);
                }
            
            //swap pointers and cleanup
            SetNewComponentsArrayPtr(iTmpArray);
            
            //notify internal listeners and update internal activation cache
            HandleTraceActivationChanges(*iChangedArray);
            NotifyInternalActivation( EFalse, componentId );
            }
        else
            if (activationInfo == SOS_MON_TRACECORE_OST_DEACTIVATE_ALL)
                {
                // Deactivate all groups from component
                TUint32 componentId = Read32(frameStart + KOstComponentIdOffset);
                // Component Id in message is in Big Endian format so it need to be swap to Little Endian format
                componentId = SWAP32( componentId );
                
                CopyArray<TOstComponent>(iOstComponents, iTmpArray);
                CleanupComponentsArray(*iChangedArray);
                RemoveComponentId(componentId, ETrue, *iTmpArray, *iChangedArray);
                SetNewComponentsArrayPtr(iTmpArray);
                
                //notify internal listeners and update internal activation cache
                HandleTraceActivationChanges(*iChangedArray);
                NotifyInternalActivation( EFalse, componentId );
                }
        }

    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTFRAME_EXIT_,"< DBTraceOstCategoryBitmap::ReadOstFrame position:%d", retPosition );
    return retPosition;
    }

/**
 * Read the groups from an OST frame
 * 
 * @param aMessage Received message
 * @param aFrameStartIndex Start offset of the frame in message
 * @param aActivationInfo Activation / deactivation flag
 * @param aComponentIndex The index of the component where the groups belong to
 * @param aComponentId The component ID
 * @param aOstComponents Array with TOstComponent objects.
 * @param aChangedArray Array with changed TOstComponent objects. 
 * @return Offset after read frame
 */
TInt DBTraceOstCategoryBitmap::ReadOstFrameGroups(const TDesC8& aMessage, TInt aFrameStartIndex, TUint8 aActivationInfo,
        TInt aComponentIndex, TUint32 aComponentId, RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTFRAMEGROUPS_ENTRY,"> DBTraceOstCategoryBitmap::ReadOstFrameGroups startIdx:%d componentIdx:%d", aFrameStartIndex, aComponentIndex );
    
    const TUint8* frameStart = aMessage.Ptr() + aFrameStartIndex;
    TUint frameLen(aMessage.Length() - aFrameStartIndex);
    TUint offset = KOstGroupIdOffset;
    TInt retPosition( 0);
    // One frame may contain multiple group ID's. Each occupy 2 bytes
    TUint32 numberOfGroups = Read32(frameStart + KOstNumberOfGroupsOffset);

    TUint i = 0;

    // Number of groups in message is in Big Endian format so it need to be swap to Little Endian format
    numberOfGroups = SWAP32( numberOfGroups );

    if (frameLen >= (offset + numberOfGroups * KGroupIdLength ))
        {
        if (aActivationInfo == SOS_MON_TRACECORE_OST_OVERWRITE)
            {
            offset = OverwriteGroups(frameStart, aComponentIndex, aComponentId, numberOfGroups, aOstComponents, aChangedArray);
            }
        else
            if (aActivationInfo == SOS_MON_TRACECORE_OST_DEACTIVATE || aActivationInfo == SOS_MON_TRACECORE_OST_ACTIVATE)
                {
                // Read Group ids and remove from array
                for (i = 0; i < numberOfGroups; i++)
                    {
                    TUint16 groupId = Read16(frameStart + offset);
                    // Group Id in message is in Big Endian format so it need to be swap to Little Endian format
                    groupId = SWAP16( groupId );

                    offset += KGroupIdLength;
                    if (GroupIdIsValid(groupId)) 
                    	{
                        if (aActivationInfo == SOS_MON_TRACECORE_OST_ACTIVATE)
                            {
                            AddGroupId(aComponentId, groupId, aComponentIndex, aOstComponents, aChangedArray);
                            }
                        else
                            {
                            RemoveGroupId(aComponentId, groupId, aComponentIndex, aOstComponents, aChangedArray);
                            }
                    	}
                    else
                        {
			            // Notifies activation interfaces of invalid group ID
			            DTraceCore* traceCore = DTraceCore::GetInstance();
			            if ( traceCore != NULL )
			                {
			                traceCore->GetNotifier().TraceError( aComponentId, groupId, KErrArgument );
			                }
                        }
                    }
                }
            else
                {
                OstTrace1( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_READOSTFRAMEGROUPS__,"DBTraceOstCategoryBitmap::ReadOstFrameGroups - Unknow activation type %d", aActivationInfo);
                }

        // If all groups were removed from the component, it can also be removed
        if ( ( (aActivationInfo == SOS_MON_TRACECORE_OST_DEACTIVATE ) || (aActivationInfo == SOS_MON_TRACECORE_OST_OVERWRITE ) )
                && (aOstComponents[ aComponentIndex ].iGroups.Count() == 0 ))
            {
            RemoveComponentId(aComponentId, EFalse, aOstComponents, aChangedArray);
            }
        // The next frame offset is returned
        retPosition = aFrameStartIndex + offset;
        }
    else
        {
        OstTraceExt2( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_READOSTFRAME_TOO_SHORT_FRAME,"DBTraceOstCategoryBitmap::ReadOstFrame - Too short frame nrOfGroups:%u len:%d",numberOfGroups, (TInt32)frameLen );
        }
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_READOSTFRAMEGROUPS_EXIT_,"< DBTraceOstCategoryBitmap::ReadOstFrameGroups position:%d", retPosition );
    return retPosition;
    }

/**
 * Callback when a trace activation message is received
 *
 * @param aMsg Reference to message
 */
void DBTraceOstCategoryBitmap::MessageReceived(TTraceMessage &aMsg)
    {
    TInt len(aMsg.iData->Length() );
    
    // Message header is in proprietary format
    if (aMsg.iMsgFormat == EMessageHeaderFormatProprietary)
        {
        
        // Check that message length is big enough 
        if (len >= KOstMinFrameLength)
            {
            // Get OST message ID
            TUint8 ostMessageId = *(aMsg.iData->Ptr() + KOstMessageIdOffset );
            switch (ostMessageId)
                {
                case SOS_MON_TRACECORE_OST_ACTIVATION:
                    {
                    TInt nextFrameStart( 0);
                    TInt lastFrameStart( -1);
                    
                    // Go through OST frames
                    while (lastFrameStart < nextFrameStart)
                        {
                        lastFrameStart = nextFrameStart;
                        nextFrameStart = ReadOstFrame( *aMsg.iData, nextFrameStart);
                        }
                    break;
                    }
                case SOS_MON_TRACECORE_OST_ACTIVATION_QUERY:
                    {
                    OstTrace0( TRACE_NORMAL, DBTRACEOSTCATEGORYBITMAP_MESSAGERECEIVED_ACTIVATION_QUERY_NOT_IMPLEMENTED,"DBTraceOstCategoryBitmap::MessageReceived - Activation query not implemented");
                    break;
                    }
                default:
                    {
                    OstTrace1( TRACE_IMPORTANT , DBTRACEOSTCATEGORYBITMAP_MESSAGERECEIVED_INVALID_MESSAGE_ID,"DBTraceOstCategoryBitmap::MessageReceived - Invalid message ID - %d", ostMessageId);
                    break;
                    }
                }
            }
        }
    
    // Message header is in OST format
    else
        if (aMsg.iMsgFormat == EMessageHeaderFormatOst)
            {
            OstProtocolMessage(aMsg);
            }
    }

/**
 * Ost Protocol Message handler
 *
 * @param aMessage The message
 */
void DBTraceOstCategoryBitmap::OstProtocolMessage(TTraceMessage &aMsg)
    {
    if ((*aMsg.iData)[OstHeader::OstTraceActivation::KOstHeaderMessageIdOffset-OstHeader::OstBaseProtocol::KOstBaseHeaderSize]
            == OstConstants::OstTraceActivation::KSetApplicationStatusRequest)
        {
        ReadOstProtocolFrame( *aMsg.iData, 0);
        
        // Send KSetApplicationResponse
        TBuf8<256> responseMessageHeader; // CodForChk_Dis_Magic
        responseMessageHeader.Append(OstConstants::KOstHeaderVersion);
        responseMessageHeader.Append(OstConstants::OstBaseProtocol::KOstTraceActivationProtocol); // 
        responseMessageHeader.Append( 0); // Data size MSB
        responseMessageHeader.Append( 4); // Data size LSB // CodForChk_Dis_Magic

        TBuf8<256> responseMessageData; // CodForChk_Dis_Magic
        TInt headerSize =
             OstHeader::OstTraceActivation::KOstHeaderTransactionIdOffset-OstHeader::OstBaseProtocol::KOstBaseHeaderSize;
        responseMessageData.Append((*aMsg.iData)[headerSize]);
        responseMessageData.Append(OstConstants::OstTraceActivation::KSetApplicationStatusResponse);
        responseMessageData.Append( 0x00); // Response request succeded TODO add constant to header
        responseMessageData.Append( 0x00); // Filler

        // Create TraceMessage
        TTraceMessage msg;
        msg.iMsgFormat = EMessageHeaderFormatOst;
        msg.iHeader = &responseMessageHeader;
        msg.iData = &responseMessageData;
        msg.iMessageId = OstConstants::OstTraceActivation::KSetApplicationStatusResponse;
        DTraceCoreActivationBase::SendResponse(msg);
        }
    }

/**
 * Overwite trace groups
 *
 * @param aComponentIndex The index of the component where the groups belong to
 * @param aComponentId The component ID
 * @param numberOfGroups Number of groups in overwrite message
 * @param aOstComponents Array with TOstComponent objects.
 * @param aChangedArray Array with changed TOstComponent objects.
 * @return Offset after read frame
 */
TInt DBTraceOstCategoryBitmap::OverwriteGroups(const TUint8* aFrameStart, TInt aComponentIndex, TUint32 aComponentId,
        TUint32 numberOfGroups, RArray<TOstComponent>& aOstComponents, RArray< TOstComponent >& aChangedArray)
    {
    OstTraceExt3( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_OVERWRITEGROUPS_ENTRY,"> DBTraceOstCategoryBitmap::OverwriteGroups. ComponentIdx:%d CID:0x%x NoOfGroups:%u",(TInt)aComponentIndex, (TUint)aComponentId , (TUint)numberOfGroups );
    RArray<TUint> groupsInMessage;
    TInt ret = 0;
    TUint i = 0;
    TUint offset = KOstGroupIdOffset;

    // Read Group ids and remove from array
    for (i = 0; i < numberOfGroups; i++)
        {
        TUint16 groupId = Read16(aFrameStart + offset);
        // Group Id in message is in Big Endian format so it need to be swap to Little Endian format
        groupId = SWAP16( groupId );

        offset += KGroupIdLength;

        ret = groupsInMessage.InsertInOrder(groupId);
        }

    
    TUint numberOfGroupsInComponent = aOstComponents[ aComponentIndex ].iGroups.Count();
    TUint numberOfRemovedGroups = 0;

    // First remove those groups that are not any more activated
    for (i = 0; i < numberOfGroupsInComponent; i++)
        {

        ret = groupsInMessage.FindInOrder(aOstComponents[ aComponentIndex ].iGroups[ i - numberOfRemovedGroups ]);

        if (ret == KErrNotFound)
            {
            RemoveGroupId(aComponentId, aOstComponents[ aComponentIndex ].iGroups[ i - numberOfRemovedGroups ], aComponentIndex,
                    aOstComponents, aChangedArray);
            numberOfRemovedGroups++;
            }
        }

    // Add groups that are not already added   
    numberOfGroups = groupsInMessage.Count();
    for (i = 0; i < numberOfGroups; i++)
        {
        ret = aOstComponents[ aComponentIndex ].iGroups.FindInOrder(groupsInMessage[ i ]);

        if (ret == KErrNotFound)
            {
            if (GroupIdIsValid(groupsInMessage[ i ]))
                {
            
                AddGroupId(aComponentId, groupsInMessage[ i ], aComponentIndex, aOstComponents, aChangedArray);
                }
            else
                {
                OstTrace1( TRACE_ERROR, DBTRACEOSTCATEGORYBITMAP_OVERWRITEGROUPS_ERROR, "DBTraceOstCategoryBitmap::OverwriteGroups - Invalid group ID: 0x%x", groupsInMessage[ i ] );
                // Notifies activation interfaces of invalid group ID
                DTraceCore* traceCore = DTraceCore::GetInstance();
                if ( traceCore != NULL )
                    {
                    traceCore->GetNotifier().TraceError( aComponentId, groupsInMessage[ i ], KErrArgument );
                    }
                }            
            }
        }

	groupsInMessage.Close();
    OstTraceExt2( TRACE_FLOW, DBTRACEOSTCATEGORYBITMAP_OVERWRITEGROUPS_EXIT,"< DBTraceOstCategoryBitmap::OverwriteGroups. Offset:%u, ret:%d", offset, ret );
    return offset;

    }

/**
 * Checks if a component ID is supported by this activation object
 * 
 * @param aComponentID the component ID to be checked
 * @return ETrue if supported, EFalse if not
 */
TBool DBTraceOstCategoryBitmap::IsComponentSupported(TUint32 /*aComponentID*/)
    {
    // All components are supported
    return ETrue;
    }

/**
 * Allocates memory for activation buffer
 *
 * @return KErrNone if allocation succesfull otherwise KErrNoMemory
 */
TInt DBTraceOstCategoryBitmap::AllocateActivationBuffer()
	{  
    TInt ret = KErrNone;
    iActivationBuffer = ( TUint8* )Kern::Alloc( KMaxActivationBufferLength );
    if ( iActivationBuffer == NULL )
        {
        ret = KErrNoMemory;
        }
    return ret;
	}

/**
  * Read and write helper functions for activation buffer
  */
TUint32 DBTraceOstCategoryBitmap::Read32FromBuffer(TUint8*& aPtr)
	{
	TUint32 ret = 0;
	if ( aPtr >= iActivationBuffer 
		&& aPtr < iActivationBuffer+KMaxActivationBufferLength-K4Bytes)
		{
		ret = Read32(aPtr);
		aPtr += K4Bytes;
		}
	return ret;
	}

TUint16 DBTraceOstCategoryBitmap::Read16FromBuffer(TUint8*& aPtr)
	{
	TUint16 ret = 0;
	if ( aPtr >= iActivationBuffer 
		&& aPtr < iActivationBuffer+KMaxActivationBufferLength-K2Bytes)
		{
		ret = Read16(aPtr);
		aPtr += K2Bytes;
		}
	return ret;
	}

void DBTraceOstCategoryBitmap::WriteToBuffer(TUint8*& aPtr, TUint32 aData32)
	{
	if ( aPtr >= iActivationBuffer 
		&& aPtr < iActivationBuffer+KMaxActivationBufferLength-K4Bytes)
		{
		*aPtr++ = aData32 & K8BitMask;
		*aPtr++ = (aData32 >> K8BitShift) & K8BitMask;
		*aPtr++ = (aData32 >> K16BitShift) & K8BitMask;
		*aPtr++ = (aData32 >> K24BitShift) & K8BitMask;
		}
	}

void DBTraceOstCategoryBitmap::WriteToBuffer(TUint8*& aPtr, TUint16 aData16)
	{
	if ( aPtr >= iActivationBuffer 
		&& aPtr < iActivationBuffer+KMaxActivationBufferLength-K2Bytes)
		{
		*aPtr++ = aData16 & K8BitMask;
		*aPtr++ = (aData16 >> K8BitShift) & K8BitMask;
		}
	}

/**
 * Handles trace activation changes
 * 
 * @pre On entry to this method writer mutex (iActivationWriteLock) must be help.
 * @param aChangedArray Array with changed TOstComponent objects. 
 */
void DBTraceOstCategoryBitmap::HandleTraceActivationChanges(RArray< TOstComponent >& aChangedArray)
    {
    TC_TRACE( ETraceLevelFlow, Kern::Printf("Activated components count: %d", iOstComponents->Count()));
    *(iBTraceOstCategoryHandler->iCacheItem1) = 0;
    *(iBTraceOstCategoryHandler->iCacheItem2) = 0;
    *(iBTraceOstCategoryHandler->iCacheItem3) = 0;
    *(iBTraceOstCategoryHandler->iCacheItem4) = 0;
    
    TInt count = iOstComponents->Count();
    // No traces activated, use Dummy handler    
    if (count == 0)
        {
        iBTraceOstCategoryHandler->iAllTracesDeactivated = ETrue;
        BTrace::SetHandler(DummyBTraceHandler);
        TC_TRACE( ETraceLevelFlow, Kern::Printf("Dummy trace handler in use 1"));
        }
        
    // Check if only printf traces are activated
    else if (count == 1 && (*iOstComponents)[0].iComponentId == KKernelHooksOSTComponentUID)
        {
        	
        // Check if there are other groups than printfs activated
        RArray< TUint > groups = (*iOstComponents)[0].iGroups;
        TInt groupcount = groups.Count();
        TBool onlyPrintfs = ETrue;
        for ( TInt i = 0; i < groupcount; ++i )
            {
            if (groups[i] > BTrace::EPlatsecPrintf)
            	{
            		onlyPrintfs = EFalse;
            		break;
            	}
            }
            
        // If there are only printfs, keep using Dummy handler
        if (onlyPrintfs)
            {
            iBTraceOstCategoryHandler->iAllTracesDeactivated = ETrue;
            BTrace::SetHandler(DummyBTraceHandler);
            TC_TRACE( ETraceLevelFlow, Kern::Printf("Dummy trace handler in use 2"));
            }
            
        // Something else than printfs activated, use normal handler
        else
            {
            iBTraceOstCategoryHandler->iAllTracesDeactivated = EFalse;
            BTrace::SetHandler(oldHandler);
            TC_TRACE( ETraceLevelFlow, Kern::Printf("Normal trace handler in use 1"));
            }
        }
        
    // More than 1 component activated, use normal handler
    else
        {
        iBTraceOstCategoryHandler->iAllTracesDeactivated = EFalse;
        BTrace::SetHandler(oldHandler);
        TC_TRACE( ETraceLevelFlow, Kern::Printf("Normal trace handler in use 2"));
        }
    
    
    // Notify listeners about the changed groups
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        count = aChangedArray.Count();
        for (TInt i = 0; i < count; i++)
            {
            TUint32 componentId = aChangedArray[i].iComponentId;
            RArray< TUint > groups = aChangedArray[i].iGroups;
            TInt groupCount = groups.Count();
            for (TInt j=0;j < groupCount;j++)
                {
                TUint32 group = groups[j];
    
                // Shift to be able to use IsTraceActivated function
                if (IsTraceActivated(componentId, group << GRP_SHIFT))
                    {
                    traceCore->GetNotifier().TraceActivated(componentId, group);
                    }
                else
                    {
                    traceCore->GetNotifier().TraceDeactivated(componentId, group);
                    }
                }
            }
        }
    }

/**
 * Gets activated groups for this component ID
 * @param aComponentId the component ID
 */
RArray< TUint > DBTraceOstCategoryBitmap::GetActivatedGroups( const TUint32 aComponentId )
    {
    RArray< TUint > groups;
    // Check index of component id in array
    TInt index = FindComponentId(aComponentId, *iOstComponents);
    if (index >= 0)
        {
        // Component id found, get groups
        groups = (*iOstComponents)[ index ].iGroups;
        }
    return groups;
    }

/**
 * Adds a group to array of changed activation groups
 * @param aChangedArray Array with changed TOstComponent objects
 * @param aComponentId Component ID
 * @param aGroupId Group ID
 */
void DBTraceOstCategoryBitmap::AddGroupToChangedArray(RArray< TOstComponent >& aChangedArray, TUint32 aComponentId, TGroupId aGroupId)
    {
    TOstComponent newComponent;
    newComponent.iComponentId = aComponentId;
    aChangedArray.InsertInUnsignedKeyOrder(newComponent);
    TInt changedIndex = FindComponentId(aComponentId, aChangedArray);
    if(changedIndex != KErrNotFound)
        aChangedArray[ changedIndex ].iGroups.InsertInOrder(aGroupId);
    }

/**
* Utility method to copy data between two RArray objects.
*/
template <class T>
void DBTraceOstCategoryBitmap::CopyArray(RArray<T>* aSrc, RArray<T>* aDst)
    {
    aDst->Reset();
    TInt c = aSrc->Count();
    for(TInt i=0; i<c; ++i)
        aDst->Append( (*aSrc)[i] );
    }


//TODO: use this doxygen comment instead of old one
//TODO: don't pass any args as swap occurs only between iOstComponents and tmpArray
/**
 * Sets new pointer for array with activated components.
 * This operation is protected by spin lock on SMP systems.
 * 
 * @param aNewPtr Pointer to new array containing TOstComponent objects.
 */
void DBTraceOstCategoryBitmap::SetNewComponentsArrayPtr(RArray<TOstComponent>*& aNewPtr)
    {
    TInt irq = ReadLock();
    RArray<TOstComponent>* tmpPtr = iOstComponents;
    iOstComponents = aNewPtr;
    aNewPtr = tmpPtr;
    ReadUnlock(irq);
    }

/**
 * Internal method to destroy array used to store activated components.
 */
void DBTraceOstCategoryBitmap::CleanupComponentsArray(RArray< TOstComponent >& aComponentArray)
    {
    TInt componentCount = aComponentArray.Count();
    for (TInt i=0; i<componentCount; i++)
        {
        aComponentArray[i].iGroups.Reset();
        }
    aComponentArray.Reset();
    }

/**
 * Internal method to destroy arrays used to store activated components.
 */
void DBTraceOstCategoryBitmap::CleanupComponentsArrays()
    {
    if(iOstComponents)
        {
        CleanupComponentsArray(*iOstComponents);
        iOstComponents->Close();
        Kern::Free(iOstComponents);
        iOstComponents = NULL;
        }
        
    if(iTmpArray)
        {
        CleanupComponentsArray(*iTmpArray);
        iTmpArray->Close();
        Kern::Free(iTmpArray);
        iTmpArray = NULL;
        }
    
    if (iChangedArray)
        {
        CleanupComponentsArray(*iChangedArray);
        iChangedArray->Close();
        Kern::Free(iChangedArray);
        iChangedArray = NULL;
        }
    }
    
/**
 * Reactivate all currently activated traces
 *
 * @return KErrNone if refresh successful
 */
TInt DBTraceOstCategoryBitmap::RefreshActivations()
    {
    RArray< TOstComponent >* tmpArray = new RArray<TOstComponent>(KOstComponentsArrayGranularity);
    if(!tmpArray)
        return KErrNoMemory;

    CopyArray<TOstComponent>(iOstComponents, tmpArray);
    // Go through all components in array and refresh activations for each
    TInt componentCount = tmpArray->Count();
    for (TInt i=0; i<componentCount; i++)
        {
        TUint32 componentId = (*tmpArray)[ i ].iComponentId;
        RArray< TUint > groups = (*tmpArray)[ i ].iGroups;
        // Go through all groups for this component and refresh activations for each
        TInt groupCount = groups.Count();
        for (TInt j=0; j<groupCount; j++)
            {
            TUint groupId = groups[ j ];
            // Don't refresh EMetaTrace as this messes up the meta traces outputted
            if (componentId != KKernelHooksOSTComponentUID || groupId != BTrace::EMetaTrace)
                {
                DeactivateTrace(componentId, groupId);
                ActivateTrace(componentId, groupId);
                }
            }
        }
    tmpArray->Close();
    Kern::Free(tmpArray);
    return KErrNone;
    }

// End of File
