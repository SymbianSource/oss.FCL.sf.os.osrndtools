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

#include "TraceCore.h"
#include "TraceCorePrintfTraceHandler.h"
#include "TraceCoreDebug.h"
#include "TraceCoreConstants.h"
#include "TraceCoreActivation.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCorePrintfTraceHandlerTraces.h"
#endif
#include "TraceCoreTComArgMacro.h"


#ifdef __SMP__
#include <nkernsmp/nkern.h>
static TInt gIrqState = 0;
TSpinLock gSpinLock(TSpinLock::TSpinLock::EOrderBTrace);
#define __LOCK() gIrqState=__SPIN_LOCK_IRQSAVE( gSpinLock );
#define __UNLOCK() __SPIN_UNLOCK_IRQRESTORE(gSpinLock, gIrqState);
#else 
#define __LOCK()
#define __UNLOCK()
#endif

// Constants
#define GRP_SHIFT 16

#ifdef ADD_OST_HEADER_TO_PRINTF
const TUint KDWordShift = 32;
const TUint KDWordMask = 0xFFFFFFFF;
const TUint KNanoSeconds = 1000000000;

/**
 * Musti timestamp mask. First 4 bits are reserved for flags
 */
const TUint KTimestampMask = 0x0FFFFFFF;

/**
 * Musti timestamp flags for XTIv2
 */
const TUint KTimestampFlags = 0xD0000000;

const TUint KTimestampLenght(8);
#endif

/**
 * String that is send to indicate a trace has been dropped
 */
_LIT8(KDroppedTrace,"* Dropped Trace");

/**
 * Static instance is needed when calling traces from handler function
 */
DTraceCorePrintfTraceHandler* DTraceCorePrintfTraceHandler::iInstance = NULL;

/**
 * State of the Printf activations
 */
TBool DTraceCorePrintfTraceHandler::iKernPrintfActive = ETrue;
TBool DTraceCorePrintfTraceHandler::iRDebugPrintfActive = ETrue;
TBool DTraceCorePrintfTraceHandler::iPlatSecPrintfActive = ETrue;

/**
 * Constructor
 */
DTraceCorePrintfTraceHandler::DTraceCorePrintfTraceHandler()
: iPrintfHandler( NULL )
    {
    }


/**
 * Destructor
 */
DTraceCorePrintfTraceHandler::~DTraceCorePrintfTraceHandler()
    {
    DTraceCorePrintfTraceHandler::iInstance = NULL;
    }
	

/**
 * Initializes Printf handler
 */
TInt DTraceCorePrintfTraceHandler::Init()
    {    
    TC_TRACE( ETraceLevelFlow, Kern::Printf( ">DTraceCorePrintfTraceHandler::Init()" ) );
    // Register to TraceCore
    TInt ret = Register();
    if ( ret == KErrNone )
        {
        DTraceCorePrintfTraceHandler::iInstance = this;      
        RegisterNotificationReceiver( KKernelHooksOSTComponentUID, BTrace::ERDebugPrintf );
    	RegisterNotificationReceiver( KKernelHooksOSTComponentUID, BTrace::EKernPrintf );
    	RegisterNotificationReceiver( KKernelHooksOSTComponentUID, BTrace::EPlatsecPrintf );
        }
    
    //activate printf group IDs by default
    DTraceCore* traceCore = DTraceCore::GetInstance();
    if (traceCore)
        {
        MTraceCoreActivation* activation = traceCore->GetActivation( KKernelHooksOSTComponentUID );
        if ( activation )
            {
             activation->ActivateTrace( KKernelHooksOSTComponentUID, BTrace::ERDebugPrintf );
             activation->ActivateTrace( KKernelHooksOSTComponentUID, BTrace::EKernPrintf  );
             activation->ActivateTrace( KKernelHooksOSTComponentUID, BTrace::EPlatsecPrintf  );
            }
        }
    
    TC_TRACE( ETraceLevelFlow, Kern::Printf( "<DTraceCorePrintfTraceHandler::Init() - %d", ret ) );
    return ret;
    }

    
/**
 * Prepares for writer change
 * 
 * @param aWriter Pointer to writer
 */       
void DTraceCorePrintfTraceHandler::PrepareSetWriter( DTraceCoreWriter* aWriter )
    {
    OstTrace1( TRACE_FLOW, DTRACECOREPRINTFTRACEHANDLER_PREPARESETWRITER_ENTRY, "> DTraceCorePrintfTraceHandler::PrepareSetWriter 0x%x", ( TUint )( aWriter ) );

    TBool validMedia = aWriter != NULL;
    
    if ( validMedia )
        {
        DTraceCoreHandler::PrepareSetWriter( aWriter );
        // SetTraceHandler is not called if it has already been called before
        if ( iPrintfHandler == NULL )
            {
#ifndef TRACECORE_HANDLE_PRINTF_EXCLUDED  // define TRACECORE_HANDLE_PRINTF_EXCLUDED when debugging using printfs
            iPrintfHandler = Kern::SetTraceHandler( DTraceCorePrintfTraceHandler::PrintfHandler );
#endif
            OstTraceExt2( TRACE_NORMAL, DTRACECOREPRINTFTRACEHANDLER_SETWRITER_HANDLER_STARTED,"DTraceCorePrintfTraceHandler::PrepareSetWriter - Printf trace routing started. Addr:0x%x WriterType:%d",(TInt)aWriter, (TInt) aWriter->GetWriterType() );
            }
        }
    else
        {
        if ( aWriter == NULL )
            {
            OstTrace0( TRACE_NORMAL, DTRACECOREPRINTFTRACEHANDLER_PREPARESETWRITER_HANDLER_RESET,"DTraceCorePrintfTraceHandler::PrepareSetWriter - Printf trace routing stopped");
            DTraceCoreHandler::PrepareSetWriter( aWriter );
            
#ifndef TRACECORE_HANDLE_PRINTF_EXCLUDED // define TRACECORE_HANDLE_PRINTF_EXCLUDED when debugging using printfs
            Kern::SetTraceHandler( iPrintfHandler );
#endif
            iPrintfHandler = NULL;
            }
        else
            {
            OstTraceExt2( TRACE_NORMAL, DTRACECOREPRINTFTRACEHANDLER_PREPARESETWRITER_HANDLER_NOT_CHANGED,"DTraceCorePrintfTraceHandler::PrepareSetWriter - Printf trace routing was not changed Addr:0x%x WriterType:%d",(TUint)iWriter, (TInt) iWriter->GetWriterType() );
            }
        }
    }


/*
	Trace handler hook.
	Should be able to run in any content (including ISR).
	@param aText      Debug log. The content of the descriptor resides in kernel memory.
	@param aTraceType Identifies the origin of the debug log.
	@return Specifies whether the log is processed or not.
		If ETrue,  the log is processed. Kernel will drop the log (it won't be passed to trace port - UART)
        If EFalse, the log is not processed. Kernel will pass the log to the trace port, as well.
*/
TBool DTraceCorePrintfTraceHandler::PrintfHandler( const TDesC8& aText, TTraceSource aTraceSource )
    {
    __LOCK();
    
    // Print
    switch( aTraceSource )
        {
        case EKernelTrace:
            // Check if Kernel traces are on
            if ( DTraceCorePrintfTraceHandler::iKernPrintfActive )
                {
                HandleTrace( aText );
                }
            break;  
        case EPlatSecTrace:
            // Check if PlatSec traces are on
            if ( DTraceCorePrintfTraceHandler::iPlatSecPrintfActive )
                {
                HandleTrace( aText );
                }
            break;           
        case EUserTrace:
            // Check if User traces are on
            if ( DTraceCorePrintfTraceHandler::iRDebugPrintfActive )
                {
                HandleTrace( aText );
                }

            break;
        default:
            break;            
        }
    
    __UNLOCK();
       
    return ETrue;
    }


/**
	Send the string to the active writer
 	@param aDes String to be printed out
 	
 	@note CR, LF and other special characters are not checked
 	@note, Tracing is not allowed from this method (esspecially on SMP system)
*/
void DTraceCorePrintfTraceHandler::DebugPrint( const TDesC8& aDes )
    {
    // When a writer registers, Kern::SetTraceHandler is called to start printf routing
    // However, the iWriter member will be NULL for a brief period after Kern::SetTraceHandler
    //  -> Printf from an interrupt could crash this if NULL-check is not made
    if ( iWriter != NULL )
        {
        const TText8* pS = aDes.Ptr();
#ifdef ADD_OST_HEADER_TO_PRINTF		
        TUint length = aDes.Length();
        const TText8* pE = pS + length;
#else
        const TText8* pE = pS + aDes.Length();
#endif
        const TUint32* chS_ptr = reinterpret_cast< const TUint32* >( pS );
        const TUint32* chE_ptr = reinterpret_cast< const TUint32* >( pE );
        
        if ( chS_ptr != chE_ptr )
            {
            // First send the printf start character
            TUint32 entryId = iWriter->WriteStart( EWriterEntryAscii );
            
#ifdef ADD_OST_HEADER_TO_PRINTF    
            // DEBUG make sure we use this printfhandler:
            //iWriter->WriteData( entryId, (TUint8)0x39 );
            //  version(0x05);    // OST Base Protocol version "1.0" (v00-80-00_r1-04.pdf)
            //  entityId(0x01);   // Entity id TODO: check if this needs to change with CPU
            //  protocolId(0x02); // Ascii Trace Protocol (Not in MIPI specs 08 yet)
            
            iWriter->WriteData( entryId, ( TUint8 )0x05 ); // Version "0.5" because BTrace header included with protocol id 0x03
            iWriter->WriteData( entryId, ( TUint8 )0x01 ); // EntityId
            iWriter->WriteData( entryId, ( TUint8 )0x02 ); // Ascii trace ProtocolId (not specified in MIPI specs yet)
            
            length += KTimestampLenght;
            // Set length
            // If write size less than 256
            if(length < 256 )
            {
                //  length(size);
                iWriter->WriteData( entryId, ( TUint8 )length );
            
            }
            else
            {
                //  extendedLengthBits0_7(0x00);
                //  extendedLengthBits8_15(0x00);
                //  extendedLengthBits16_23(0x00);
                //  extendedLengthBits24_31(0x00);
                iWriter->WriteData( entryId, ( TUint8 )0x00 ); // length field 0 if extended length in use
                iWriter->WriteData( entryId, ( TUint32 )length ); // No swap, Length in protocol is little endian //SWAP_DATA( length ) );
            }
            
            // Write timestamp
        
            TUint64 timestamp = NKern::FastCounter();
            timestamp = (timestamp * KNanoSeconds ) / NKern::FastCounterFrequency();
            
            TUint32 timestampLSB = timestamp & KDWordMask;
            TUint32 timestampMSB = ( (timestamp >> KDWordShift ) & KTimestampMask ) | KTimestampFlags;
            
            iWriter->WriteData( entryId, SWAP_ID( timestampMSB ) );
        	iWriter->WriteData( entryId, SWAP_ID( timestampLSB ) );
            
            // DEBUG - END

#endif
        	
            while ( chS_ptr + 1 <= chE_ptr )
                {
                // Print using 32bit write
                TUint32 val = *( chS_ptr++ );
                iWriter->WriteData( entryId, SWAP_DATA( val ) );
                }    
            
            // Print the rest
            pS = ( TText8* )chS_ptr;
            if ( pS != pE )
                {
                while ( pS < pE )
                    {
                    iWriter->WriteData( entryId, *( pS++ ) );
                    }
                }
            
            // End trace
            iWriter->WriteData( entryId, static_cast< TUint8 >( 0x00 ) );
            
            TWriteEndParams params(entryId, ETrue);
            iWriter->WriteEnd( params );
            }
        }
    }

/**
 * Callback function for Trace Activation
 * 
 * @param aComponentId
 * @param aGroupId         
 */   
void DTraceCorePrintfTraceHandler::TraceActivated( TUint32 TCOM_ARG(aComponentId), TUint16 aGroupId  )
    {
    OstTraceExt2( TRACE_FLOW, DTRACECOREPRINTFTRACEHANDLER_TRACEACTIVATED,"> DTraceCorePrintfTraceHandler::TraceActivated;aComponentId=0x%x;aGroupId=0x%x", (TUint) aComponentId, (TUint) aGroupId );
    
    switch( aGroupId )
    	{
    	case BTrace::EKernPrintf:
    		DTraceCorePrintfTraceHandler::iKernPrintfActive = ETrue;
    		break;
    	case BTrace::ERDebugPrintf:
    		DTraceCorePrintfTraceHandler::iRDebugPrintfActive = ETrue;
    		break;
    	case BTrace::EPlatsecPrintf:
    		DTraceCorePrintfTraceHandler::iPlatSecPrintfActive = ETrue;
    		break;
    	default:
    		break;	
    	}
    }
		
/**
 * Callback function for Trace Deactivation
 * 
 * @param aComponentId
 * @param aGroupId     
 */  
void DTraceCorePrintfTraceHandler::TraceDeactivated( TUint32 TCOM_ARG(aComponentId), TUint16 aGroupId  )
    {
    OstTraceExt2( TRACE_FLOW, DTRACECOREPRINTFTRACEHANDLER_TRACEDEACTIVATED,"> DTraceCorePrintfTraceHandler::TraceDeactivated;aComponentId=0x%x;aGroupId=0x%x", (TUint) aComponentId, (TUint) aGroupId );
    
    switch( aGroupId )
    	{
    	case BTrace::EKernPrintf:
    		DTraceCorePrintfTraceHandler::iKernPrintfActive = EFalse;
    		break;
    	case BTrace::ERDebugPrintf:
    		DTraceCorePrintfTraceHandler::iRDebugPrintfActive = EFalse;
    		break;
    	case BTrace::EPlatsecPrintf:
    		DTraceCorePrintfTraceHandler::iPlatSecPrintfActive = EFalse;
    		break;
    	default:
    		break;	
    	}
    }

/**
 * This method check if "dropped trace" notification must be sent and if so it
 * verify whether writer is able to send "dropped trace" and "new incomming trace".
 * If no the incomming trace is dropped and "dropped traces" flag is kept as set.
 *
 * @param aText Text-trace to send.
 */
inline void DTraceCorePrintfTraceHandler::HandleTrace( const TDesC8& aText )
    {    
    if(!DTraceCorePrintfTraceHandler::iInstance || !DTraceCorePrintfTraceHandler::iInstance->iWriter)
        return; //no writer or no handler instance - don't even try to send data
    
    DTraceCore* tracecore = DTraceCore::GetInstance();
    DTraceCorePrintfTraceHandler* handler = DTraceCorePrintfTraceHandler::iInstance;
    DTraceCoreWriter* writer = handler->iWriter;
    
    if (tracecore->PreviousTraceDropped() )
        {
        if( writer->AbleToWrite(KDroppedTrace().Length()+aText.Length()))
            { 
            //send a "dropped trace" text trace and then the actual trace
            handler->DebugPrint(KDroppedTrace);
            handler->DebugPrint(aText);
            tracecore->SetPreviousTraceDropped(EFalse); //rested "dropped trace" flag
            }
		else
			{
			//don't send anything to the buffer
			}
        }
    else
        {
        // send the actual trace
        handler->DebugPrint(aText);    
        }
    }

// End of File
