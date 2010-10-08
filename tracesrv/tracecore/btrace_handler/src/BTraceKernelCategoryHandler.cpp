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

// Include files
#include "BTraceKernelCategoryHandler.h"
#include "BTraceOstCategoryBitmap.h"
#include "TraceCore.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"

#include "OstTraceDefinitions.h"

#ifdef OST_TRACE_COMPILER_IN_USE
#include "BTraceKernelCategoryHandlerTraces.h"
#endif

#include "TraceCoreTComArgMacro.h"

//Split: These needed from TraceCoreAutogen.h (which is removed):
#define SYMBIAN_CF_SERVERDEN 0xC2
#define SYMBIAN_CF_MESHMACHINE 0xC3
#define SYMBIAN_CF_FACTORIES 0xC4

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

// Trace group shift when checking if group is active
#define GRP_SHIFT 16

// BTrace category shift for unknown categories
#define CATEGORY_SHIFT 8

/**
 * Constructor
 */
DBTraceKernelCategoryHandler::DBTraceKernelCategoryHandler()
: iPrimeDfc( DBTraceKernelCategoryHandler::PrimeDfc, this, DTraceCore::GetActivationQ(), KDefaultDfcPriority )
, iMultiPartActivationInfos(2, _FOFF( TMultiPartActivationInfo, iMultiPartId ))
    {
    }


/**
 * Destructor
 */
DBTraceKernelCategoryHandler::~DBTraceKernelCategoryHandler()
    {
    iMultiPartActivationInfos.Close();
    }


/**
 * Initializes this handler
 *
 * @param aHandler The BTrace handler
 */
TInt DBTraceKernelCategoryHandler::Init()
    {
    TInt ret( KErrGeneral );

    DTraceCore* traceCore = DTraceCore::GetInstance();
    if ( traceCore != NULL )
        {
        // Gets the Autogen category bitmap from the list of registered activation interfaces
        iTraceBitmap = static_cast< DBTraceOstCategoryBitmap* >( 
                traceCore->GetActivation( KKernelHooksOSTComponentUID ) );
        if ( iTraceBitmap != NULL )
            {
            // Registers to bitmap for change notifications. The primary BTrace filters are
            // updated when the bitmap changes
            iTraceBitmap->RegisterActivationNotification( *this );
            ret = KErrNone;
            }
        }
    
    // Registers kernel categories to BTrace
    if ( ret == KErrNone )
        {
        for ( TInt i = KMinKernelCategory; ( ret == KErrNone ) && ( i <= KMaxKernelCategory ); i++ )
            {
            ret = AddCategory( i );
            }

        if ( ret == KErrNone )
            {
            ret = Register();
            }
        }
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DBTraceKernelCategoryHandler::Init() - %d", ret ) );
    return ret;
    }


/**
 * Called before SetWriter with interrupts enabled
 * 
 * @param aWriter The new writer
 */
void DBTraceKernelCategoryHandler::PrepareSetWriter( DTraceCoreWriter* aWriter )
    {
    OstTrace1( TRACE_FLOW, DBTRACEKERNELCATEGORYHANDLER_PREPARESETWRITER_ENTRY,"> DBTraceKernelCategoryHandler::PrepareSetWriter 0x%x", ( TUint )( aWriter ) );
    if ( iWriter == NULL && aWriter != NULL )
        {
        DBTraceCategoryHandler::PrepareSetWriter( aWriter );
        // When writer is set, the kernel categories are primed
        // Priming is done via DFC.
        PrimeKernelCategories();
        }
    else
        {
        DBTraceCategoryHandler::PrepareSetWriter( aWriter );
        if ( aWriter == NULL )
            {
            // If writer is set to NULL, the kernel categories are disabled. 
            // This needs to be done immediately, not via timer
            PrimeDfc();
            }
        }
    if ( aWriter != NULL && aWriter->GetWriterType() == EWriterTypeUSBPhonet )
        {
        // CPU events must be disabled when using media writer
        PrimeCategory( BTrace::ECpuUsage );
        }
    }


/**
 * Handler for KCategoryNokiaAutogen
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
TBool DBTraceKernelCategoryHandler::HandleFrame( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext, 
                                                  const TUint32 a1, const TUint32 a2, const TUint32 a3, 
                                                  const TUint32 aExtra, const TUint32 aPc )
    {
    TBool retval;
    if ( iWriter != NULL )
        {
        //deal with any possible missing traces
        DTraceCore* tracecore = DTraceCore::GetInstance();
        if(!tracecore)
            return EFalse;
        
        // Check if tracing is certified
        if (!tracecore->IsTraceCertified())
            return EFalse;

        if ((tracecore->PreviousTraceDropped())) //if previous trace was dropped 
            {
            //set flags back to EFalse first
            tracecore->SetPreviousTraceDropped(EFalse);

            //set missing flag in BTrace
            aHeader |= BTrace::EMissingRecord<<(BTrace::EFlagsIndex * KByteSize);
            }
        
        TUint8 category = ( aHeader >> ( BTrace::ECategoryIndex * KByteSize ) ) & KByteMask;
        TUint8 subCategory = ( aHeader >> ( BTrace::ESubCategoryIndex * KByteSize ) ) & KByteMask;
        TUint32 traceWord = MapCategoryToID( category, subCategory );
        if ( traceWord > 0 )
            {
            
            // Check if the trace is a multipart trace
            TBool isMultiPart = CheckMultiPart(aHeader, aHeader2);
            if (isMultiPart)
                {
                // Handle the multipart trace
                retval = HandleMultiPart(aHeader, aHeader2, aContext, traceWord, a2, a3, aExtra, aPc);
                }
            
            // Not a multipart trace
            else 
                {
                // If previous trace was discarded, add info about it to the header
                if (tracecore->PreviousTraceDropped())
                    {
                    aHeader |= BTrace::EMissingRecord<<(BTrace::EFlagsIndex * KByteSize);
                    tracecore->SetPreviousTraceDropped(EFalse);
                    }
                
                TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
                iWriter->WriteTraceCoreFrame( KKernelHooksOSTComponentUID, 
                    traceWord, aHeader, aHeader2, aContext, a1, a2, a3, aExtra, aPc, recordSize );

                retval = ETrue;
                }
            }
        else
            {
            retval = EFalse;
            }
        }
    else
        {
        retval = EFalse;
        }
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf("<DBTraceKernelCategoryHandler::HandleFrame - return %d", retval ) );
    return retval;
    }


/**
 * Maps a BTrace category / sub-category to group / trace ID
 *
 * @param aCategory The BTrace category
 * @param aSubCategory The BTrace sub-categoory
 * @return The group / trace ID combination
 */
TUint32 DBTraceKernelCategoryHandler::MapCategoryToID( TUint8 aCategory, TUint8 aSubCategory )
    {
    TUint32 ret;
    
    switch( aCategory )
        {
        case BTrace::EThreadIdentification:
        case BTrace::ECpuUsage:
        case BTrace::EClientServer:
        case BTrace::ERequests:
        case BTrace::EChunks:
        case BTrace::ECodeSegs:
        case BTrace::EPaging:
        case BTrace::EThreadPriority:
        case BTrace::EPagingMedia:
            ret = ( aCategory << GRP_SHIFT ) | aSubCategory;
            break;
            
        // Symbian 9.4 categories
        case BTrace::EKernelMemory:
        case BTrace::EHeap:
        case BTrace::EMetaTrace:
        case BTrace::ERamAllocator:
        case BTrace::EFastMutex:     
        case BTrace::EProfiling:
            ret = ( aCategory << GRP_SHIFT ) | aSubCategory;
            break;
            
        // Symbian 9.5 categories            
        case BTrace::EResourceManager:
        case BTrace::EResourceManagerUs:
        case BTrace::ERawEvent:
        case BTrace::EUsb:
        case BTrace::ESymbianKernelSync:
        case BTrace::EFlexibleMemModel:
            ret = ( aCategory << GRP_SHIFT ) | aSubCategory;
            break;
            
        // Symbian 9.6 categories            
        case BTrace::EIic:
            ret = ( aCategory << GRP_SHIFT ) | aSubCategory;
            break;                                                   

        // These are for Symbian for debugging purposes
		case 194:
			ret = ( SYMBIAN_CF_SERVERDEN << GRP_SHIFT ) | aSubCategory;
			break;
		case 195:
			ret = ( SYMBIAN_CF_MESHMACHINE << GRP_SHIFT ) | aSubCategory;
			break;
		case 196:
			ret = ( SYMBIAN_CF_FACTORIES << GRP_SHIFT ) | aSubCategory;
			break;
				
        default:
            // Unknown category but let's still use the same ID as we received
            ret = ( aCategory << GRP_SHIFT ) | aSubCategory;
            break;
        }

    return ret;
    }


/**
 * Called when an activation bitmap state changes
 * 
 * @param aActivation the activation object
 * @param aFromSettings ETrue if changes was due to settings read
 * @param aComponentId Component ID of the activation
 */
void DBTraceKernelCategoryHandler::ActivationChanged( MTraceCoreActivation& TCOM_ARG(aActivation), TBool TCOM_ARG(aFromSettings),
        TUint32 aComponentId)
    {
    OstTraceExt2( TRACE_FLOW, DBTRACEKERNELCATEGORYHANDLER_ACTIVATIONCHANGED_ENTRY,"> DBTraceKernelCategoryHandler::ActivationChanged 0x%x. FromSettings:%d",( TUint )( &aActivation ), aFromSettings );
    
    // Kernel categories are primed when activation bitmap changes
    if (aComponentId == KOldNokiaAutogenOSTComponentUID || aComponentId == KKernelHooksOSTComponentUID)
        {
        PrimeKernelCategories();
        }
    }


/**
 * Primes the kernel categories    
 */
void DBTraceKernelCategoryHandler::PrimeKernelCategories()
    {
    // Priming is done asynchronously to avoid blocking this thread
    iPrimeDfc.Enque();
    }


/**
 * Dfc function to prime kernel categories
 */
void DBTraceKernelCategoryHandler::PrimeDfc( TAny* aHandler )
    {
    OstTrace1( TRACE_FLOW, DBTRACEKERNELCATEGORYHANDLER_PRIMEDFC_ENTRY,"> DBTraceKernelCategoryHandler::PrimeDfc 0x%x", ( TUint )( aHandler ) );
    
    // Get handler and prime kernel categories
    DBTraceKernelCategoryHandler* handler = static_cast< DBTraceKernelCategoryHandler* >( aHandler );
    handler->PrimeDfc();
    }


/**
 * Called from the static DFC callback function
 */
void DBTraceKernelCategoryHandler::PrimeDfc()
    {
    OstTrace0( TRACE_FLOW, DBTRACEKERNELCATEGORYHANDLER_PRIMEDFC,
    		"> DBTraceKernelCategoryHandler::PrimeDfc" );
    		
    // Start from Thread Identification as we don't want to active Printfs from the BTrace
    for ( TInt i = KMinKernelCategory; i <= KMaxKernelCategory; i++ )
        {
        PrimeCategory( i );
        }
    }


/**
 * Primes a category
 * 
 * @param aCategory the category to be primed
 */
void DBTraceKernelCategoryHandler::PrimeCategory( TUint8 aCategory )
    {
    TUint32 traceWord = MapCategoryToID( aCategory, 0 );
    // CPU events are not possible when using USB phonet writer
    // They result in context switch trace loop
    if ( iWriter != NULL &&
         (iTraceBitmap->IsTraceActivated( KKernelHooksOSTComponentUID, traceWord ) )
            && ( aCategory != BTrace::ECpuUsage || iWriter->GetWriterType() != EWriterTypeUSBPhonet ) )
        {
        TInt ret = BTrace::SetFilter( aCategory, 1 );
        if ( ret == KErrNone )
            {
            OstTrace1( TRACE_NORMAL, DBTRACEKERNELCATEGORYHANDLER_PRIMEDFC__,"DBTraceKernelCategoryHandler::PrimeDfc - Priming 0x%x", aCategory );
            BTrace::Prime( aCategory );
            }
        else if ( ret == KErrNotSupported )
            {
            OstTrace1( TRACE_INTERNAL, DBTRACEKERNELCATEGORYHANDLER_PRIMEDFC_NOT_SUPPORTED,"DBTraceKernelCategoryHandler::PrimeDfc - Category not supported 0x%x", aCategory );
            }
        }
    else
        {
        (void) BTrace::SetFilter( aCategory, 0 );
        }
    }

/**
 * Handles this Multipart trace
 *
 * @param aHeader BTrace header
 * @param aHeader2 Extra header data
 * @param aContext The thread context in which this function was called
 * @param aTraceWord Trace Word
 * @param a1 First parameter
 * @param aData The data
 * @param aExtra Extra trace data
 * @param aPc The program counter value
 * @return ETrue if trace was processed
 */
TBool DBTraceKernelCategoryHandler::HandleMultiPart( TUint32 aHeader, TUint32 aHeader2, const TUint32 aContext,
                   const TUint32 aTraceWord, const TUint32 a1, const TUint32 aData, const TUint32 aExtra,
                   const TUint32 aPc)
    {
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( ">DBTraceOstCategoryHandler::HandleMultiPart" ) );
    TBool retval = ETrue;
    TInt multiPartOffset = aHeader2 & BTrace::EMultipartFlagMask;
    TUint8 recordSize = static_cast< TUint8 >( ( aHeader >> ( BTrace::ESizeIndex * KByteSize ) ) & KByteMask );
    
    // First part of multipart trace
    if (multiPartOffset == BTrace::EMultipartFirst)
        {
        // Create new MultiPart activation info and save it to the array
        TMultiPartActivationInfo activationInfo;
        activationInfo.iComponentId = KKernelHooksOSTComponentUID;
        activationInfo.iTraceWord = aTraceWord;
        activationInfo.iMultiPartId = aExtra;
   
        // Insert the item to the array        
        TInt ret = iMultiPartActivationInfos.InsertInUnsignedKeyOrder(activationInfo);
        
        if (KErrNone == ret)
            {            
            TUint32* ptr = reinterpret_cast< TUint32* >(aData);
            TUint32 a2 = *ptr++;
            
            // Write the trace. Move pointer by 4 bytes because first 4 bytes is moved from aData
            // to a2. Decrease record size by 4 bytes because the original a2 is not written
            iWriter->WriteTraceCoreFrame( activationInfo.iComponentId, activationInfo.iTraceWord, 
                    aHeader, aHeader2, aContext, a1, a2, 
                    aData + 4, aExtra, aPc, recordSize - 4);
            }
        else
            {
            retval = EFalse;
            DTraceCore* tcore = DTraceCore::GetInstance();
            if(tcore)
                tcore->SetPreviousTraceDropped(ETrue);
            }
        }
     
    // Middle or last part of multipart trace
    else if (multiPartOffset == BTrace::EMultipartMiddle || multiPartOffset == BTrace::EMultipartLast)
        {
        // Check index of component id in array
        TMultiPartActivationInfo tempInfo;
        tempInfo.iMultiPartId = aExtra;
        TInt index = iMultiPartActivationInfos.FindInUnsignedKeyOrder(tempInfo);
        
        if (index != KErrNotFound)
            {
            TMultiPartActivationInfo activationInfo = iMultiPartActivationInfos[index];

            TUint32 a1 = 0;
            TUint32 a2 = 0;
            TUint32 movePointerOffset = 0;
            
            // Calculate if we can move data from the aData to a1 and a2
            TUint32 dataStartOffset = CalculateDataStartOffset(aHeader);
            if ( recordSize - dataStartOffset >= 4 )
                {
                TUint32* ptr = reinterpret_cast< TUint32* >(aData);
                a1 = *ptr++;
                movePointerOffset += 4;
                
                if ( recordSize - dataStartOffset >= 8 )
                    {
                    a2 = *ptr++;
                    movePointerOffset += 4;
                    }
                }
                        
            // Write the trace. Decrease the record size by 8 because of the original a1 and a2
            // are not written
            iWriter->WriteTraceCoreFrame( activationInfo.iComponentId, activationInfo.iTraceWord, 
                    aHeader, aHeader2, aContext, a1, a2, 
                    aData + movePointerOffset, aExtra, aPc, recordSize - 8);
            
            // Last part, remove the item from the array
            if (multiPartOffset == BTrace::EMultipartLast)
                {
                iMultiPartActivationInfos.Remove(index);
                }
            }
        }
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( "<DBTraceOstCategoryHandler::HandleMultiPart > return %d", retval ) );
    return retval;
    }

/**
 * Checks if the given trace is a Multipart trace
 *
 * @param aHeader Header data
 * @param aHeader2 Extra header data
 * @return ETrue if trace is a Multipart trace, EFalse if not
 */
inline TBool DBTraceKernelCategoryHandler::CheckMultiPart( TUint32 aHeader, TUint32 aHeader2 )
    {
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( ">DBTraceKernelCategoryHandler::CheckMultiPart()" ) );
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
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( "<DBTraceKernelCategoryHandler::CheckMultiPart > return %d", retval ) );
    return retval;
    }

/**
 * Calculates data start offset
 *
 * @param aHeader BTrace header
 */
TUint32 DBTraceKernelCategoryHandler::CalculateDataStartOffset( TUint32 aHeader )
    {
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( ">DBTraceOstCategoryHandler::CalculateDataStartOffset()" ) );
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
    
    TC_TRACE( ETraceLevelTraceFlow, Kern::Printf( "<DBTraceOstCategoryHandler::CalculateDataStartOffset > return %d", offset ) );
    return offset;
    }

// End of File
