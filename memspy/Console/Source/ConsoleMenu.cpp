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

#include "ConsoleMenu.h"

// System includes
#include <e32debug.h>

// Engine includes
#include <memspyengineclientinterface.h>
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyenginehelperheap.h>
#include <memspy/engine/memspyengineoutputsink.h>
#include <memspy/engine/memspyenginehelperstack.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspydevicewideoperations.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyenginehelperkernelcontainers.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>

// User includes
#include "ConsoleConstants.h"
#include "ConsoleDWOperation.h"


CMemSpyConsoleMenu::CMemSpyConsoleMenu( CMemSpyEngine& aEngine, CConsoleBase& aConsole )
:   CActive( EPriorityHigh ), iEngine( aEngine ), iConsole( aConsole )
    {
    CActiveScheduler::Add( this );
    iEngine.SetObserver( this );
    }


CMemSpyConsoleMenu::~CMemSpyConsoleMenu()
    {
    Cancel();
    iEngine.SetObserver( NULL );
    }


void CMemSpyConsoleMenu::ConstructL()
    {
    DrawMenuL();
    WaitForInput();
    }


CMemSpyConsoleMenu* CMemSpyConsoleMenu::NewLC( CMemSpyEngine& aEngine, CConsoleBase& aConsole )
    {
    CMemSpyConsoleMenu* self = new(ELeave) CMemSpyConsoleMenu( aEngine, aConsole );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


void CMemSpyConsoleMenu::DrawMenuL()
    {
    iConsole.ClearScreen();
   
    // First line - sink type (defaults to file)
    _LIT( KLine1, "1 or T. Toggle output mode between file or trace [%S]" );
    if  ( iEngine.SinkType() == ESinkTypeDebug )
        {
        _LIT( KLine1Trace, "Trace" );
        iConsole.Printf( KLine1, &KLine1Trace );
        }
    else
        {
        _LIT( KLine1File, "File" );
        iConsole.Printf( KLine1, &KLine1File );
        }
    iConsole.Write( KMemSpyConsoleNewLine );

    // Kernel heap dump
    _LIT( KLine2, "2 or K. Dump kernel heap data" );
    iConsole.Write( KLine2 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Kernel heap dump
    _LIT( KLine3, "3 or O. Dump kernel object listing" );
    iConsole.Write( KLine3 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Heap (CSV) listing
    _LIT( KLine4, "4 or H. Heap CSV-information (for all threads)" );
    iConsole.Write( KLine4 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Stack (CSV) listing
    _LIT( KLine5, "5 or S. Stack CSV-information (for all threads)" );
    iConsole.Write( KLine5 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Heap data
    _LIT( KLine6, "6 or D. Get heap data for a user-thread" );
    iConsole.Write( KLine6 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Heap cell listing
    _LIT( KLine7, "7 or L. Get heap cell list for a user-thread" );
    iConsole.Write( KLine7 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Exit key
    _LIT( KLine8, "8 or X. Exit" );
    iConsole.Write( KLine8 );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Spacer
    iConsole.Write( KMemSpyConsoleNewLine );
    iConsole.Write( KMemSpyConsoleNewLine );

    // Status message
    iStatusMessagePos = iConsole.CursorPos();
    RedrawStatusMessage();

    // Spacer
    iConsole.Write( KMemSpyConsoleNewLine );

    // Show input prompt.
    iCommandPromptPos = iConsole.CursorPos();
    RedrawInputPrompt();
    }


void CMemSpyConsoleMenu::WaitForInput()
    {
    ASSERT( !IsActive() );
    iConsole.Read( iStatus );
    SetActive();
    }


void CMemSpyConsoleMenu::RunL()
    {
    TKeyCode key = iConsole.KeyCode();
    //
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::RunL() - START - key = %d", key );
#endif
    //
    if  ( key == EKeyEnter || key == KMemSpyUiS60KeyCodeButtonOk || key == KMemSpyUiS60KeyCodeRockerEnter )
        {
        TRAP_IGNORE( ProcessCommandBufferL() );
        }
    else if ( key == EKeyEscape || key == KMemSpyUiS60KeyCodeButtonCancel )
        {
        ClearCommandBuffer();
        RedrawInputPrompt();
        }   
    else if ( key == EKeyBackspace )
        {
        const TInt cmdBufLength = iCommandBuffer.Length();
        if  ( cmdBufLength > 0 )
            {
            iCommandBuffer.SetLength( cmdBufLength - 1 );
            RedrawInputPrompt();
            }
        }
    else
        {
        TChar character( key );
        if  ( character.IsPrint() )
            {
            if  ( iCommandBuffer.Length() < iCommandBuffer.MaxLength() )
                {
                iCommandBuffer.Append( TChar( key ) );
                }

            RedrawInputPrompt();
            }
        }

    WaitForInput();

#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::RunL() - END" );
#endif
    }


void CMemSpyConsoleMenu::DoCancel()
    {
    iConsole.ReadCancel();
    }


void CMemSpyConsoleMenu::OnCmdSinkTypeToggleL()
    {
    if  ( iEngine.SinkType() == ESinkTypeDebug )
        {
        iEngine.InstallSinkL( ESinkTypeFile );
        }
    else
        {
        iEngine.InstallSinkL( ESinkTypeDebug );
        }
    }


void CMemSpyConsoleMenu::OnCmdHeapDataKernelL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdHeapDataKernelL() - START" );
#endif

    _LIT( KMsg, "Ouputting Kernel data..." );
    RedrawStatusMessage( KMsg );

    iEngine.HelperHeap().OutputHeapDataKernelL();

    RedrawStatusMessage( KNullDesC );
    }


void CMemSpyConsoleMenu::OnCmdKernelObjectListingL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdKernelObjectListingL() - START" );
#endif

    _LIT( KMsg, "Ouputting Kernel Object listing..." );
    RedrawStatusMessage( KMsg );
    //
    CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
    CMemSpyEngineGenericKernelObjectContainer* model = kernelContainerManager.ObjectsAllL();
    CleanupStack::PushL( model );
    //
    CMemSpyEngineOutputSink& sink = iEngine.Sink();
    model->OutputL( sink );
    //
    CleanupStack::PopAndDestroy( model );

    RedrawStatusMessage( KNullDesC );
    }


void CMemSpyConsoleMenu::OnCmdCSVListingStackL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdCSVListingStackL() - START" );
#endif

    iEngine.HelperStack().OutputStackInfoForDeviceL();
    }


void CMemSpyConsoleMenu::OnCmdCSVListingHeapL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdCSVListingHeapL() - START" );
#endif

    iEngine.HelperHeap().OutputHeapInfoForDeviceL();
    }


void CMemSpyConsoleMenu::OnCmdHeapDataUserL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdHeapDataUserL() - START" );
#endif

    GetProcessName();

    // Work out what to do.
    iCommandBuffer.Trim();

#ifdef _DEBUG
    RDebug::Print( _L("[MCon] CMemSpyConsoleMenu::OnCmdHeapDataUserL() - requested dump heap for proc: %S"), &iCommandBuffer );
#endif

    if  ( iCommandBuffer.Length() > 0 )
        {
        iConsole.Write( KMemSpyConsoleNewLine );
        iConsole.Write( KMemSpyConsoleNewLine );
        //
        HBufC* cmdBuf = HBufC::NewLC( KMemSpyMaxInputBufferLength + 10 );
        TPtr pCmdBuf( cmdBuf->Des() );
        pCmdBuf.Copy( iCommandBuffer );
        pCmdBuf.Append( KMemSpyConsoleWildcardCharacter );
        //
        CMemSpyEngineObjectContainer& container = iEngine.Container();
        const TInt count = container.Count();
        TFullName fullThreadName;
        //
        TInt index = 0;
#ifdef _DEBUG
        RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdHeapDataUserL() - procCount: %d", count );
#endif

        while( index < count )
            {
            CMemSpyProcess& process = container.At( index );
            const TPtrC processName( process.Name() );
#ifdef _DEBUG
            RDebug::Print( _L("[MCon] CMemSpyConsoleMenu::OnCmdHeapDataUserL() - procName: 0x%08x %S"), &process, &processName );
#endif

            //
            if  ( processName.MatchF( pCmdBuf ) >= 0 )
                {
                _LIT( KProcessingRequest, "** Dumping Heap Data for thread: %S" );
                const TInt threadCount = process.Count();
                for( TInt i=0; i<threadCount; i++ )
                    {
                    CMemSpyThread& thread = process.At( i );
                    //
                    fullThreadName = thread.FullName();
                    iConsole.Printf( KProcessingRequest, &fullThreadName );
                    iConsole.Write( KMemSpyConsoleNewLine );
                    //
                    TRAP_IGNORE( iEngine.HelperHeap().OutputHeapDataUserL( thread ) );
                    }

                break;
                }
            
            ++index;
            }

        CleanupStack::PopAndDestroy( cmdBuf );
        DrawMenuL();
        }
    else
        {
        // Bad user data entry
        DrawMenuL();
        _LIT( KInvalidEntry, "*** ERROR - Invalid Command ***" );
        RedrawStatusMessage( KInvalidEntry );
        RedrawInputPrompt();
        }
    }


void CMemSpyConsoleMenu::OnCmdHeapCellListUserL()
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdHeapCellListUserL() - START" );
#endif

    GetProcessName();

    // Work out what to do.
    iCommandBuffer.Trim();
    if  ( iCommandBuffer.Length() > 0 )
        {
        iConsole.Write( KMemSpyConsoleNewLine );
        iConsole.Write( KMemSpyConsoleNewLine );
        //
        HBufC* cmdBuf = HBufC::NewLC( KMemSpyMaxInputBufferLength + 10 );
        TPtr pCmdBuf( cmdBuf->Des() );
        pCmdBuf.Copy( iCommandBuffer );
        pCmdBuf.Append( KMemSpyConsoleWildcardCharacter );
        //
        CMemSpyEngineObjectContainer& container = iEngine.Container();
        const TInt count = container.Count();
        TFullName fullThreadName;
        //
        TInt index = 0;
        while( index < count )
            {
            CMemSpyProcess& process = container.At( index );
            const TPtrC processName( process.Name() );
            //
            if  ( processName.MatchF( pCmdBuf ) >= 0 )
                {
                _LIT( KProcessingRequest, "** Dumping Heap Cell List for thread: %S" );
                const TInt threadCount = process.Count();
                for( TInt i=0; i<threadCount; i++ )
                    {
                    CMemSpyThread& thread = process.At( i );
                    //
                    fullThreadName = thread.FullName();
                    iConsole.Printf( KProcessingRequest, &fullThreadName );
                    iConsole.Write( KMemSpyConsoleNewLine );
                    //
                    TRAP_IGNORE( iEngine.HelperHeap().OutputCellListingUserL( thread ) );
                    }

                break;
                }
            
            ++index;
            }

        CleanupStack::PopAndDestroy( cmdBuf );
        DrawMenuL();
        }
    else
        {
        // Bad user data entry
        DrawMenuL();
        _LIT( KInvalidEntry, "*** ERROR - Invalid Command ***" );
        RedrawStatusMessage( KInvalidEntry );
        RedrawInputPrompt();
        }
    }


void CMemSpyConsoleMenu::ClearCommandBuffer()
    {
    iCommandBuffer.Zero();
    }


void CMemSpyConsoleMenu::ProcessCommandBufferL()
    {
    iCommandBuffer.Trim();
    //
#ifdef _DEBUG
    RDebug::Print( _L("[MCon] CMemSpyConsoleMenu::ProcessCommandBufferL() - cmd: [%S]"), &iCommandBuffer );
#endif
    //
    TBool validCommand = EFalse;
    if  ( iCommandBuffer.Length() == 1 )
        {
        // Reset if not recognised...
        validCommand = ETrue;

        const TChar cmd = iCommandBuffer[ 0 ]; 
        switch( cmd )
            {
        case '1':
        case 't':
        case 'T':
            OnCmdSinkTypeToggleL();
            break;
        case '2':
        case 'k':
        case 'K':
            OnCmdHeapDataKernelL();
            break;
        case '3':
        case 'o':
        case 'O':
            OnCmdKernelObjectListingL();
            break;
        case '4':
        case 'h':
        case 'H':
            OnCmdCSVListingHeapL();
            break;
        case '5':
        case 's':
        case 'S':
            OnCmdCSVListingStackL();
            break;
        case '6':
        case 'd':
        case 'D':
            OnCmdHeapDataUserL();
            break;
        case '7':
        case 'l':
        case 'L':
            OnCmdHeapCellListUserL();
            break;
        case '8':
        case 'x':
        case 'X':
            CActiveScheduler::Stop();
            return; // NB: avoid redrawing menu when exiting

        default:
            validCommand = EFalse;
            break;
            }
        }
    //
    ClearCommandBuffer();
    //
    if  ( !validCommand )
        {
        _LIT( KInvalidEntry, "*** ERROR - Invalid Command ***" );
        RedrawStatusMessage( KInvalidEntry );
        RedrawInputPrompt();
        }
    else
        {
        DrawMenuL();
        }

#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::ProcessCommandBufferL() - END" );
#endif
    }


void CMemSpyConsoleMenu::RedrawInputPrompt()
    {
    iConsole.SetCursorPosAbs( iCommandPromptPos );
    iConsole.ClearToEndOfLine();
    iConsole.Printf( KMemSpyConsoleInputPrompt, &iCommandBuffer );
    }


void CMemSpyConsoleMenu::RedrawStatusMessage()
    {
    RedrawStatusMessage( KNullDesC );
    }


void CMemSpyConsoleMenu::RedrawStatusMessage( const TDesC& aMessage )
    {
    iConsole.SetCursorPosAbs( iStatusMessagePos );
    iConsole.ClearToEndOfLine();
    iConsole.Write( aMessage );
    iConsole.Write( KMemSpyConsoleNewLine );
    }


void CMemSpyConsoleMenu::GetProcessName()
    {
    iConsole.ClearScreen();

    _LIT( KPromptMessage1, "Enter the full or partial process name" );
    iConsole.Write( KPromptMessage1 );
    iConsole.Write( KMemSpyConsoleNewLine );
    _LIT( KPromptMessage2, "of the process that you are interested in." );
    iConsole.Write( KPromptMessage2 );
    iConsole.Write( KMemSpyConsoleNewLine );
    _LIT( KPromptMessage3, "Press \'enter\' when done." );
    iConsole.Write( KPromptMessage3 );
    iConsole.Write( KMemSpyConsoleNewLine );
    //
    ClearCommandBuffer();
    iConsole.Write( KMemSpyConsoleNewLine );
    iCommandPromptPos = iConsole.CursorPos();
    RedrawInputPrompt();
    //
    TBool done = EFalse;
    while( !done )
        {
        // Get a key
        const TKeyCode key = iConsole.Getch();

        // Process it
#ifdef _DEBUG
        RDebug::Printf( "[MCon] CMemSpyConsoleMenu::OnCmdHeapDataUserL() - START - key = %d", key );
#endif
        //
        if  ( key == EKeyEnter || key == KMemSpyUiS60KeyCodeButtonOk || key == KMemSpyUiS60KeyCodeRockerEnter )
            {
            done = ETrue;
            }
        else if ( key == EKeyEscape || key == KMemSpyUiS60KeyCodeButtonCancel )
            {
            ClearCommandBuffer();
            done = ETrue;
            }   
        else if ( key == EKeyBackspace )
            {
            const TInt cmdBufLength = iCommandBuffer.Length();
            if  ( cmdBufLength > 0 )
                {
                iCommandBuffer.SetLength( cmdBufLength - 1 );
                RedrawInputPrompt();
                }
            }
        else
            {
            TChar character( key );
            if  ( character.IsPrint() )
                {
                if  ( iCommandBuffer.Length() < iCommandBuffer.MaxLength() )
                    {
                    iCommandBuffer.Append( TChar( key ) );
                    }

                RedrawInputPrompt();
                }
            }
        }
    }


void CMemSpyConsoleMenu::HandleMemSpyEngineEventL( MMemSpyEngineObserver::TEvent aEvent, TAny* aContext )
    {
    if  ( aEvent == MMemSpyEngineObserver::EHandleClientServerOperationRequest )
        {
        const TInt function = reinterpret_cast< TInt >( aContext );
        InitiateMemSpyClientServerOperationL( function );
        }
    }


void CMemSpyConsoleMenu::InitiateMemSpyClientServerOperationL( TInt aOpCode )
    {
#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::InitiateMemSpyClientServerOperationL() - START - aOpCode: %d, iRunningDeviceWideOperation: %d", aOpCode, iRunningDeviceWideOperation );
#endif
    //
    if ( aOpCode == EMemSpyClientServerOpExit )
        {
        // Exit console app UI
        CActiveScheduler::Stop();
        }
    else
        {
        CMemSpyDeviceWideOperations::TOperation op = CMemSpyDeviceWideOperations::EPerEntityGeneralSummary;
        switch( aOpCode )
            {
        case EMemSpyClientServerOpSummaryInfo:
            op = CMemSpyDeviceWideOperations::EPerEntityGeneralSummary;
            break;
        case EMemSpyClientServerOpSummaryInfoDetailed:
            op = CMemSpyDeviceWideOperations::EPerEntityGeneralDetailed;
            break;
        //
        case EMemSpyClientServerOpHeapInfo:
            op = CMemSpyDeviceWideOperations::EPerEntityHeapInfo;
            break;
        case EMemSpyClientServerOpHeapCellListing:
            op = CMemSpyDeviceWideOperations::EPerEntityHeapCellListing;
            break;
        case EMemSpyClientServerOpHeapData:
            op = CMemSpyDeviceWideOperations::EPerEntityHeapData;
            break;
        //
        case EMemSpyClientServerOpStackInfo:
            op = CMemSpyDeviceWideOperations::EPerEntityStackInfo;
            break;
        case EMemSpyClientServerOpStackDataUser:
            op = CMemSpyDeviceWideOperations::EPerEntityStackDataUser;
            break;
        case EMemSpyClientServerOpStackDataKernel:
            op = CMemSpyDeviceWideOperations::EPerEntityStackDataKernel;
            break;
        
        // These are not supported by the console UI
        default:
        case EMemSpyClientServerOpBitmapsSave:
        case EMemSpyClientServerOpSendToBackground:
        case EMemSpyClientServerOpBringToForeground:
            User::Leave( KErrNotSupported );
            break;
            }

        if  ( iRunningDeviceWideOperation )
            {
            User::Leave( KErrInUse );
            }
        else
            {
            iRunningDeviceWideOperation = ETrue;
            TRAP_IGNORE( CMemSpyDeviceWideOperationWaiter::ExecuteLD( iEngine, op ) );
            iRunningDeviceWideOperation = EFalse;
            }
        }

#ifdef _DEBUG
    RDebug::Printf( "[MCon] CMemSpyConsoleMenu::InitiateMemSpyClientServerOperationL() - END - aOpCode: %d", aOpCode );
#endif
    }



