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

#include "TraceCore.h"
#include "BTraceOstCategoryHandler.h"
#include "BTraceOstCategoryBitmap.h"
#include "TraceCoreWriter.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "BTraceOstCategoryHandlerTraces.h"
#endif

#ifndef __SMP__
/**
 * Length of Trace group ID + Trace ID in the MultiPart traces data part
 */
const TUint KTraceGroupIdLen = 4;

/**
 * Length of BTrace header
 */
const TUint KBTraceHeaderLen = 4;

/**
 * Length of single BTrace variable
 */
const TUint KBTraceVariableLen = 4;

/**
 * Four bytes
 */
const TUint KFourBytes = 4;

#endif // !__SMP__
/**
 * Group value in GroupId
 */
const TUint32 KGroupMask = 0xffff0000;


/**
 * Constructor
 */
DBTraceOstCategoryHandler::DBTraceOstCategoryHandler()
: iAllTracesDeactivated( ETrue )
, iCacheItem1( iComponentGroupCache )   // One "cache" item contains ComponentId, groupId, and activation info (3*32bit)
, iCacheItem2( iComponentGroupCache+3 ) //CodForChk_Dis_Magic
, iCacheItem3( iComponentGroupCache+6 ) //CodForChk_Dis_Magic
, iCacheItem4( iComponentGroupCache+9 ) //CodForChk_Dis_Magic
, iOstTraceBitmap( NULL )
    {
    }


/**
 * Destructor
 */
DBTraceOstCategoryHandler::~DBTraceOstCategoryHandler()
    {
	// Delete trace bitmap
	delete iOstTraceBitmap;
    }


/**
 * Initializes this handler
 *
 * @param aHandler The BTrace handler
 */
TInt DBTraceOstCategoryHandler::Init()
    {    
    TInt ret = KErrNone;

#ifdef __SMP__	    
    for( TInt i = 0; i<MAX_MULTIPART_TRACES; i++ )
        {
        iMultipartArray[i].iMultiPartId = 0xffffffff;
        iFreeList[i] = 1; // Set Free
        }
#endif // __SMP__
    
    for (TUint category = KMaxKernelCategory + 1; category <= KMaxCategory; category++)
    	{
    	if (category != KCategoryNokiaBranchCoverage) // Is this exception needed? 
    		{
    		ret = AddCategory( category );
        	if (ret != KErrNone)
        		{
        		break;
        		}
    		}
    	}
    
    if (ret == KErrNone)
    	{    
		iOstTraceBitmap = new DBTraceOstCategoryBitmap( this );	
		if ( iOstTraceBitmap != NULL )
			{
			// Initialize TraceBitmap
			ret = iOstTraceBitmap->Init();
			if ( ret == KErrNone )
				{
				ret = iOstTraceBitmap->RegisterActivationNotification( *this );
				if ( ret == KErrNone )
					{
	                ret = Register();
	                if ( ret == KErrNone )
	                    {
	                    TC_TRACE( ETraceLevelNormal, Kern::Printf( 
	                            "DBTraceAutogenCategoryHandler::Init - Register OK" ) );
	                    }
	                // Register failed
	                else
						{
						ret = KErrGeneral;
						}
					}
				// RegisterActivationNotification failed
				else
					{
					ret = KErrGeneral;
					}
				}
			}
		// Memory allocation failed
		else
			{
			TC_TRACE( ETraceLevelError, Kern::Printf( "DBTraceOstCategoryHandler::Init - Memory allocation failed" ) );
			ret = KErrNoMemory;
			}
    	}
    TC_TRACE( ETraceLevelError, Kern::Printf( "<DBTraceOstCategoryHandler::Init() - %d", ret ) );  
    return ret;
    }
    
/**
 * Set settings
 * 
 * @param aSettings Settings
 */
void DBTraceOstCategoryHandler::SetSettings( DTraceCoreSettings* aSettings )
	{
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYHANDLER_SETSETTINGS_ENTRY, "> DBTraceOstCategoryHandler::SetSettings 0x%x", ( TUint )( aSettings ) );
    DBTraceCategoryHandler::SetSettings( aSettings );
    if ( iOstTraceBitmap != NULL && aSettings != NULL )
        {
        // Reads the bitmap configuration from settings
        iOstTraceBitmap->ReadFromSettings( *aSettings );
        }
	}

/**
 * Notification that the activation state has changed.
 * 
 * @param aActivation the activation interface that was updated
 * @param aFromSettings ETrue if activation was due to settings read, EFalse if from some other source
 * @param aComponentId Component ID of the activation
 */
void DBTraceOstCategoryHandler::ActivationChanged( MTraceCoreActivation& aActivation, TBool aFromSettings,
        TUint32 /*aComponentId*/)
    {
    OstTrace1( TRACE_FLOW, DBTRACEOSTCATEGORYHANDLER_ACTIVATIONCHANGED_ENTRY, "> DBTraceOstCategoryHandler::ActivationChanged 0x%x", ( TUint )&( aActivation ) );
    
    // Call the bitmap to write changes to settings saver
    if ( !aFromSettings )
        {
        if ( iSettings != NULL )
            {
            aActivation.WriteToSettings( *iSettings );
            }
        }
    }

/**
 * Handler for KCategoryNokiaOst
 *
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 The first trace parameter
 * @param a2 The second trace parameter
 * @param a3 The third trace parameter
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @return ETrue if trace was processed, EFalse if not
 */
TBool DBTraceOstCategoryHandler::HandleFrame( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                                              const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                                              const TUint32 aExtra, const TUint32 aPc )
    {
    TBool retval(EFalse);
    if ( iWriter != NULL && !iAllTracesDeactivated )
        {
        __ASSERT_DEBUG( iOstTraceBitmap != NULL, 
                Kern::Fault( "DBTraceOstCategoryHandler::HandleFrame - NULL", KErrGeneral ) );
        
        // Check if tracing is certified
        DTraceCore* tracecore = DTraceCore::GetInstance();
        if (!tracecore || !tracecore->IsTraceCertified())
            {
            return EFalse;
            }
        
        // Check if the trace is a multipart trace
        TBool isMultiPart = CheckMultiPart(aHeader, aHeader2);
        
        if (isMultiPart)
            {
		
#ifdef __SMP__
            // Handle the multipart trace
            retval = HandleMultiPart(aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc);
#else
            // Handle the multipart trace. Returns EFalse if trace is not activated.
            TBool isActivatedMultiPart = HandleMultiPart(aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc);
            if (isActivatedMultiPart)
                {
                retval = ETrue;    
                }
            else
                {
                retval = EFalse;
                }
#endif // __SMP__

            }
        // Not a multipart trace
        else
            {
        
            // Take group and leave trace id away
            TUint32 group = a2 & KGroupMask;

            if ((*iCacheItem1) == a1 && (*(iCacheItem1 + 1)) == group)
                {
                retval = *(iCacheItem1 + 2); //CodForChk_Dis_Magic
                }
            else if ((*iCacheItem2) == a1 && (*(iCacheItem2 + 1)) == group)
                {
                retval = *(iCacheItem2 + 2); //CodForChk_Dis_Magic
                }
            else if ((*iCacheItem3) == a1 && (*(iCacheItem3 + 1)) == group)
                {
                retval = *(iCacheItem3 + 2); //CodForChk_Dis_Magic
                }
            else if ((*iCacheItem4) == a1 && (*(iCacheItem4 + 1)) == group)
                {
                retval = *(iCacheItem4 + 2); //CodForChk_Dis_Magic
                }
            else
                {
                retval = iOstTraceBitmap->IsTraceActivated(a1, a2);

                iTemp = iCacheItem1;
                iCacheItem1 = iCacheItem4;
                iCacheItem4 = iCacheItem3;
                iCacheItem3 = iCacheItem2;
                iCacheItem2 = iTemp;
                *iCacheItem1 = a1;
                *(iCacheItem1 + 1) = group;
                *(iCacheItem1 + 2) = retval;

                }
                
            if ( ( ( aHeader >> ( BTrace::ESubCategoryIndex * KByteSize ) ) & KByteMask ) == EOstTraceActivationQuery )
                {
                //Trace from here is not allowed
                }
            else if ( retval )
                {                
                CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
                TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
                iWriter->WriteTraceCoreFrame( a1, a2, aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc, recordSize );
                }
            }
        }
    else
        {        
        if( iWriter == NULL )
            {
            //Trace from here is not allowed
            retval = EFalse;
            }
            
        }

    //Trace from here is not allowed
    return retval;
    }
   
/**
 * Checks if the given trace is a Multipart trace
 *
 * @param aHeader Header data
 * @param aHeader2 Extra header data
 * @return ETrue if trace is a Multipart trace, EFalse if not
 */
inline TBool DBTraceOstCategoryHandler::CheckMultiPart( TUint32 aHeader, TUint32 aHeader2 )
    {
    TBool retval = EFalse;
    TUint8 flags = static_cast< TUint8 >( ( aHeader >> ( BTrace::EFlagsIndex * KByteSize ) ) & KByteMask );
    if (flags & BTrace::EHeader2Present)
        {
        // First, middle or last part of Multipart trace
        if (aHeader2 & BTrace::EMultipartFlagMask)
            {
            retval = ETrue;
            }
        }
    
    //Trace from here is not allowed
    return retval;
    }

/**
 * Checks if there is a dropped trace and updates header if so
 *
 * @param aHeader Header data
 */
inline TBool DBTraceOstCategoryHandler::CheckDroppedTrace( TUint32& aHeader )
    {
    TBool ret(EFalse);
    
    DTraceCore* tracecore = DTraceCore::GetInstance();
    if (tracecore && (tracecore->PreviousTraceDropped())) //if previous trace was dropped 
        {
        //set flags back to EFalse first
        tracecore->SetPreviousTraceDropped(EFalse);
        
        //set missing flag in BTrace
        aHeader |= BTrace::EMissingRecord<<(BTrace::EFlagsIndex * KByteSize);
        
        ret = ETrue;
        }
    
    return ret;
    }

#ifdef __SMP__


TInt DBTraceOstCategoryHandler::Find( TUint32 aMultiPartId )
    {
    TInt ret(KErrNotFound);
    for( TInt i = 0; i<MAX_MULTIPART_TRACES; i++ )
        {
        if( iMultipartArray[i].iMultiPartId == aMultiPartId )
            {
            ret = i;
            break;
            }
        }
    return ret;
    }
    
void DBTraceOstCategoryHandler::Remove( TUint32 aIndex )
    {
    iMultipartArray[aIndex].iMultiPartId = 0xffffffff;    
    iFreeList[aIndex] = 1; // Set free
    }
    
    /**
     * 
     * @return KErrNone if added, otherwise KErrNotFound.
     */
TInt DBTraceOstCategoryHandler::InsertIfNotExist( TMultiPartActivationInfo &aMultiPartTrace )
    {
    TInt ret = Find( aMultiPartTrace.iMultiPartId );
    if( ret == KErrNotFound )
        {
        // Find free slot
        for( TInt freeIndex = 0; freeIndex<MAX_MULTIPART_TRACES; freeIndex++ )
            {
            if( iFreeList[freeIndex] )
                {
                iFreeList[freeIndex] = 0; // Set reserved
                iMultipartArray[freeIndex].iComponentId = aMultiPartTrace.iComponentId;
                iMultipartArray[freeIndex].iTraceWord = aMultiPartTrace.iTraceWord;
                iMultipartArray[freeIndex].iMultiPartId = aMultiPartTrace.iMultiPartId;
                ret = KErrNone;
                break;
                }
            }

        }
    return ret;
    }
 

/**
 * Handles this Multipart trace
 *
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 First parameter
 * @param a2 Second parameter
 * @param aData The data
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @return ETrue if trace is activated
 */
TBool DBTraceOstCategoryHandler::HandleMultiPart( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext,
                   const TUint32 a1, const TUint32 a2, const TUint32 aData, const TUint32 aExtra,
                   const TUint32 aPc)
    {
    TBool retval = ETrue;
    TInt multiPartOffset = aHeader2 & BTrace::EMultipartFlagMask;
    TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
    
    // First part of multipart trace
    if (multiPartOffset == BTrace::EMultipartFirst)
        {
        // Create new MultiPart activation info and save it to the array
        TMultiPartActivationInfo activationInfo;
        activationInfo.iComponentId = a2;
        
        // This should be safe operation as if there is not enough data, the trace should not
        // be first part of multipart trace
        TUint32* ptr = reinterpret_cast< TUint32* >(aData);
        activationInfo.iTraceWord = *ptr++;
        activationInfo.iMultiPartId = aExtra;
        retval = iOstTraceBitmap->IsTraceActivated( activationInfo.iComponentId, activationInfo.iTraceWord );
        
        if (retval)
            {
            // Insert the item to the array        
            TInt ret = InsertIfNotExist( activationInfo );
			
            if (KErrNone == ret)
                {
                CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
                
                // Write the trace. Skip first 4 bytes as it's the traceWord which is given in different parameter
                iWriter->WriteTraceCoreFrame( activationInfo.iComponentId, activationInfo.iTraceWord, 
                        aHeader, aHeader2, aContext, activationInfo.iComponentId, 
                        activationInfo.iTraceWord, aData + 4, aExtra, aPc, recordSize - 4 );
                }
            else
                {
                retval = EFalse;
                }
            }
        }
    // Middle or last part of multipart trace
    else if (multiPartOffset == BTrace::EMultipartMiddle || multiPartOffset == BTrace::EMultipartLast)
        {
        // Check index of component id in array
        TMultiPartActivationInfo tempInfo;
        tempInfo.iMultiPartId = aExtra;
        
		TInt index = Find( aExtra );
        
        if (index != KErrNotFound)
            {
            CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
        
            TMultiPartActivationInfo activationInfo;
            activationInfo.iComponentId = iMultipartArray[index].iComponentId;
            activationInfo.iTraceWord = iMultipartArray[index].iTraceWord;
            activationInfo.iMultiPartId = iMultipartArray[index].iMultiPartId;
            
            // Write the trace
            iWriter->WriteTraceCoreFrame( activationInfo.iComponentId, activationInfo.iTraceWord, aHeader, aHeader2, 
                    aContext, activationInfo.iComponentId, activationInfo.iTraceWord, aData, aExtra, aPc, recordSize );
            
            // Last part, remove the item from the array
            if (multiPartOffset == BTrace::EMultipartLast)
                {
                Remove(index);
                }
            }
        }
    
    //Trace from here is not allowed
        
    return retval;
    }

#else // __SMP__

/**
 * Handles this Multipart trace
 *
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param a1 First parameter
 * @param a2 Second parameter
 * @param aData The data
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @return ETrue if trace was processed, EFalse if not
 */
TBool DBTraceOstCategoryHandler::HandleMultiPart( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext,
                   const TUint32 a1, const TUint32 a2, const TUint32 aData, const TUint32 aExtra,
                   const TUint32 aPc)
    {
    TBool retval = EFalse;
    
    if (iMultiPartTrace.iMultiPartId == 0 || iMultiPartTrace.iMultiPartId == aExtra)
        {    
        // First part of multipart trace
        if ((aHeader2 & BTrace::EMultipartFlagMask) == BTrace::EMultipartFirst)
            {
            TUint32 componentId = a2;
            TUint32* ptr = ( TUint32* )aData;
            TUint32 traceWord = *ptr++;
            retval = iOstTraceBitmap->IsTraceActivated( componentId, traceWord );
            if (retval)
                {
                CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
            
                // Set values
                iMultiPartTrace.iMultiPartId = aExtra;
                iMultiPartTrace.iComponentId = componentId;
                iMultiPartTrace.iTraceWord = traceWord;
                iMultiPartTrace.iContext = aContext;
                iMultiPartTrace.iPc = aPc;
                iMultiPartTrace.iHeader = aHeader;
                
                // Remove the multipart information from the header2
                aHeader2 &= ~BTrace::EMultipartFlagMask;
                iMultiPartTrace.iHeader2 = aHeader2;
    
                // Copy the data
                TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
                TUint32 dataSize = recordSize - CalculateDataStartOffset(aHeader);
                memcpy(iMultiPartTrace.iData, (TUint8*)aData + KTraceGroupIdLen, dataSize - KTraceGroupIdLen);
                iMultiPartTrace.iDataInserted = dataSize - KTraceGroupIdLen;
                }
            }      
        // Middle part of multipart trace
        else if ((aHeader2 & BTrace::EMultipartFlagMask) == BTrace::EMultipartMiddle)
            {
            retval = ETrue;
            // Add to the data buffer
            TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
            TUint32 dataSize = recordSize - CalculateDataStartOffset(aHeader);
            
            // Check that trace is not too big
            if (iMultiPartTrace.iDataInserted + dataSize < MAX_TRACE_DATA_SIZE)
                {
                CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
                memcpy(iMultiPartTrace.iData + iMultiPartTrace.iDataInserted, (TUint8*)aData, dataSize);
                iMultiPartTrace.iDataInserted += dataSize;
                }
            }
        // Last part of multipart trace
        else if ((aHeader2 & BTrace::EMultipartFlagMask) == BTrace::EMultipartLast)
            {
            retval = ETrue;
            TUint32 totalDataLen = a1;
            TUint32 offset = a2;
            
            // Add to the data buffer
            TUint32 partLen = totalDataLen - offset;
            TUint32 dataStartOffset = CalculateDataStartOffset(iMultiPartTrace.iHeader);
                           
            // Check that trace is not too big
            if (iMultiPartTrace.iDataInserted + partLen <= MAX_TRACE_DATA_SIZE)
                {
                CheckDroppedTrace( aHeader ); // Update header if there is a dropped trace
                
                // If the last part contains only last four bytes, it's a value instead of pointer
                if (partLen == KFourBytes)
                    {
                    iMultiPartTrace.iData[iMultiPartTrace.iDataInserted] = aData & KByteMask;
                    iMultiPartTrace.iData[iMultiPartTrace.iDataInserted + 1] = (aData >> 8) & KByteMask; //CodForChk_Dis_Magic
                    iMultiPartTrace.iData[iMultiPartTrace.iDataInserted + 2] = (aData >> 16) & KByteMask; //CodForChk_Dis_Magic
                    iMultiPartTrace.iData[iMultiPartTrace.iDataInserted + 3] = (aData >> 24) & KByteMask; //CodForChk_Dis_Magic
                    }
                else 
                    {
                    memcpy(iMultiPartTrace.iData + iMultiPartTrace.iDataInserted, (TUint8*)aData, partLen);   
                    }
                    
                // Total data length. Four bytes is taken out because it was parsed out from the data
                // array when parsing first part of the trace
                totalDataLen = totalDataLen + dataStartOffset - KFourBytes;    
                    
                }
            else
                {
                	// Total data length. Four bytes is taken out because it was parsed out from the data
                  // array when parsing first part of the trace
                	totalDataLen = iMultiPartTrace.iDataInserted + dataStartOffset - KFourBytes;
                }
                	
            // Write the trace
            iWriter->WriteTraceCoreFrame( iMultiPartTrace.iComponentId, iMultiPartTrace.iTraceWord,
            iMultiPartTrace.iHeader, iMultiPartTrace.iHeader2, iMultiPartTrace.iContext,
            iMultiPartTrace.iComponentId, iMultiPartTrace.iTraceWord, (TUint32)iMultiPartTrace.iData,
            iMultiPartTrace.iMultiPartId, iMultiPartTrace.iPc, totalDataLen);
                
            iMultiPartTrace.iMultiPartId = 0;
            }
        }

    return retval;
    }

/**
 * Calculates data start offset
 *
 * @param aHeader BTrace header
 */
TUint32 DBTraceOstCategoryHandler::CalculateDataStartOffset( TUint32 aHeader )
    {
    TUint32 offset = 0;
    TUint8 flags = static_cast< TUint8 >( ( aHeader >> ( BTrace::EFlagsIndex * KByteSize ) ) & KByteMask );
    
    // First add header length
    offset += KBTraceHeaderLen;
    
    // Header2 is present
    if ( flags & BTrace::EHeader2Present )
        {
        offset += KBTraceVariableLen;
        }
    // Timestamp is present
    if ( flags & BTrace::ETimestampPresent )
        {
        offset += KBTraceVariableLen;
        }
    // Timestamp2 is present
    if ( flags & BTrace::ETimestamp2Present )
        {
        offset += KBTraceVariableLen;
        }
    // Context ID is present
    if ( flags & BTrace::EContextIdPresent )
        {
        offset += KBTraceVariableLen;
        }
    // Program counter is present
    if ( flags & BTrace::EPcPresent )
        {
        offset += KBTraceVariableLen;
        }
    // Extra value is present
    if ( flags & BTrace::EExtraPresent )
        {
        offset += KBTraceVariableLen;
        }
    
    // Next 8 bytes came with first and second parameter of the multipart trace
    offset += KFourBytes;
    offset += KFourBytes;
    
    return offset;
    }
	
#endif // !__SMP__

// End of File
