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

#include "MemSpyCommandLine.h"

// System includes
#include <bacline.h>
#include <bautils.h>
#include <memspyengineclientinterface.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerconfig.h>

// User includes
#include "MemSpyCommands.h"


CMemSpyCommandLine::CMemSpyCommandLine()
    {
    }


CMemSpyCommandLine::~CMemSpyCommandLine()
    {
    if ( iMemSpy )
        {
        iMemSpy->Close();
        }
    delete iMemSpy;
    iFsSession.Close();
    }


void CMemSpyCommandLine::ConstructL()
    {
    User::LeaveIfError( iFsSession.Connect() );
    iMemSpy = new(ELeave) RMemSpyEngineClientInterface();
    ConnectToMemSpyL();
    }


CMemSpyCommandLine* CMemSpyCommandLine::NewLC()
    {
    CMemSpyCommandLine* self = new(ELeave) CMemSpyCommandLine();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


void CMemSpyCommandLine::PerformBatchL( const TDesC& aFileName )
    {
    TInt err = KErrNone;
    RFile file;
    err = file.Open( iFsSession, aFileName, EFileRead );
    TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformBatchL() - START - this: 0x%08x, openErr: %d, fileName: %S"), this, err, &aFileName ) );
    User::LeaveIfError( err );

    CleanupClosePushL( file );
    CDesCArray* lines = ReadLinesL( file );
    CleanupStack::PopAndDestroy( &file );
    CleanupStack::PushL( lines );
    
    const TInt count = lines->Count();
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - got %d lines", count ) );
    iIsBatch = ETrue;
    for( TInt i=0; i<count; i++ )
        {
        const TPtrC pLine( (*lines)[ i ] );
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - processing line[%03d] \"%S\""), i, &pLine ) );
    
        // Must be at least 3 chars big, i.e. '[' and <command> and then ']'
        if  ( pLine.Length() <= 2 || pLine[ 0 ] != '[' )
            {
            TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - ignoring line: \"%S\""), &pLine ) );
            }
        else if  ( pLine[0] == '[' )
            {
            // Try to find end of command...
            const TInt posOfClosingArgChar = pLine.Locate( ']' );
            if  ( posOfClosingArgChar >= 2 )
                {
                // Get command
                const TPtrC pCommand( pLine.Mid( 1, posOfClosingArgChar - 1 ) );
                TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - got command: %S"), &pCommand ) );

                // Next, try to get any args
                CDesCArrayFlat* args = new(ELeave) CDesCArrayFlat( 2 );
                CleanupStack::PushL( args );

                // There must be a mandatory space between closing ] and start of args...
                // E.g.:
                //
                //  [CMD] ARG
                //
                const TInt remainderLength = pLine.Length() - posOfClosingArgChar;
                if  ( remainderLength > 1 )
                    {
                    const TPtrC remainder( pLine.Mid( posOfClosingArgChar + 1 ) );
                    TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - got remainder: %S"), &pLine ) );

                    // Extract arguments separated by tabs or space characters
                    // and store in arguments array
                    HBufC* argText = HBufC::NewLC( pLine.Length() + 1 );
                    TPtr pArgText( argText->Des() );
                    for( TInt j=0; j<remainder.Length(); j++ )
                        {
                        const TChar c( remainder[ j ] );
                        //
                        if  ( c == '\t' || c == ' ' )
                            {
                            pArgText.Trim();
                            if  ( pArgText.Length() )
                                {
                                TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - arg[%02d] %S"), args->Count(), &pArgText ) );
                                args->AppendL( pArgText );
                                pArgText.Zero();
                                }
                            }
                        else
                            {
                            pArgText.Append( c );
                            }
                        }

                    // Save leftovers...
                    pArgText.Trim();
                    if  ( pArgText.Length() )
                        {
                        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - arg[%02d] %S"), args->Count(), &pArgText ) );
                        args->AppendL( pArgText );
                        }

                    CleanupStack::PopAndDestroy( argText );
                    }

                // Now we can perform the operation!
                PerformSingleOpL( pCommand, *args );

                CleanupStack::PopAndDestroy( args );
                }
            }

        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - processing line: \"%S\""), &pLine ) );
        }

    iIsBatch = EFalse;
    
    CleanupStack::PopAndDestroy( lines );
    TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformBatchL() - END - this: 0x%08x, fileName: %S"), this, &aFileName ) );
    }


void CMemSpyCommandLine::PerformOpL( const CCommandLineArguments& aCommandLine )
    {
    const TInt count = aCommandLine.Count();
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformOpL() - START - arg count: %d, this: 0x%08x", count, this ) );

    if  ( count >= 1 )
        {
        // Get main command
        TBuf<KMemSpyCommandLineMaxLength> command;
        command.Copy( aCommandLine.Arg( 1 ) );
        command.UpperCase();

        // Extract arguments into array
        CDesCArrayFlat* args = new(ELeave) CDesCArrayFlat(4);
        CleanupStack::PushL( args );
        for( TInt i=2; i<count; i++ )
            {
            args->AppendL( aCommandLine.Arg( i ) );
            }

        // Perform op
        PerformSingleOpL( command, *args );

        // Tidy up
        CleanupStack::PopAndDestroy( args );
        }
    else
        {
        User::Leave( KErrUnderflow );
        }
    }


void CMemSpyCommandLine::PerformSingleOpL( const TDesC& aCommand, const CDesCArray& aParameters )
    {
    // Record duration
    TTime timeStart; 
    timeStart.HomeTime();
    const TInt paramCount = aParameters.Count();

#ifdef _DEBUG
    TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - START - command: %S, paramCount: %d, this: 0x%08x"), &aCommand, paramCount, this ) );
    for( TInt i=0; i<paramCount; i++ )
        {
        const TPtrC pParam( aParameters[ i ] );
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - param[%02d] = [%S]"), i, &pParam ) );
        }
#else
    RDebug::Print( _L("[MemSpyCmdLine] COMMAND: [%S] {%02d}..."), &aCommand, paramCount );
#endif

    TFileName batchFile;
    batchFile.Append( aCommand );
    
    TInt err = KErrNotSupported;
    if  ( aCommand.CompareF( KMemSpyCmdSWMTForceUpdate ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - SWMT_ForceUpdate", this ) );
        if ( paramCount > 0 )
            {
            TInt categories( 0 );
            TName threadNameFilter;
            TRAP( err, ParseSWMTParametersL( aParameters, categories, threadNameFilter ) );            
            if ( !err )
                {
                err = iMemSpy->SystemWideMemoryTrackerCategoriesSet( categories );
                if ( !err && threadNameFilter.Length() > 0 )
                    {
                    err = iMemSpy->SystemWideMemoryTrackerThreadFilterSet( threadNameFilter );
                    }
                }
            }
        if ( !err )
            {
            err = iMemSpy->PerformOperation( EMemSpyClientServerOpSystemWideMemoryTrackingForceUpdate );
            }
        }
    else if ( aCommand.CompareF( KMemSpyCmdSWMTReset ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - SWMT_Reset", this ) );
        err = iMemSpy->PerformOperation( EMemSpyClientServerOpSystemWideMemoryTrackingReset );
        }
    else if ( aCommand.CompareF( KMemSpyCmdHeapDumpKernel ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Heap_DumpKernel", this ) );
        err = iMemSpy->PerformOperation( EMemSpyClientServerOpHeapData, KMemSpyClientServerThreadIdKernel );
        }
    else if ( aCommand.CompareF( KMemSpyCmdHeapCompact ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Heap_Compact", this ) );
        err = iMemSpy->PerformOperation( EMemSpyClientServerOpHeapInfoCompact );
        }
    else if ( aCommand.CompareF( KMemSpyCmdContainer ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Container", this ) );
        err = iMemSpy->PerformOperation( EMemSpyClientServerOpEnumerateKernelContainerAll );
        }
    else if ( aCommand.CompareF( KMemSpyCmdBitmapsSave ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Bitmaps_Save", this ) );
        err = iMemSpy->SaveAllBitmaps();
        }
    else if ( aCommand.CompareF( KMemSpyCmdRamDisableAknIconCache ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Ram_DisableAknIconCache", this ) );
        err = iMemSpy->DisableAknIconCache();
        }
    else if ( aCommand.CompareF( KMemSpyCmdOutputToFile ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Output_ToFile", this ) );
        err = iMemSpy->SwitchOutputModeFile();
        }
    else if ( aCommand.CompareF( KMemSpyCmdOutputToTrace ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Output_ToTrace", this ) );
        err = iMemSpy->SwitchOutputModeTrace();
        }
    else if ( aCommand.CompareF( KMemSpyCmdUiSendToBackground ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - UI_Background", this ) );
        err = iMemSpy->SendToBackground();
        }
    else if ( aCommand.CompareF( KMemSpyCmdUiBringToForeground ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - UI_Foreground", this ) );
        err = iMemSpy->BringToForeground();
        }
    else if ( aCommand.CompareF( KMemSpyCmdUiExit ) == 0 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - UI_Exit", this ) );
        err = iMemSpy->Exit();
        }
    else if ( aCommand.CompareF( KMemSpyCmdHeapDump ) == 0 )
        {
        if  ( paramCount == 0 )
            {
            // Dump heap data for all threads
            TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Heap_Dump (all threads)", this ) );
            err = iMemSpy->PerformOperation( EMemSpyClientServerOpHeapData );
            }
        else if ( paramCount >= 1 )
            {
            // Dump heap data for named thread
            const TPtrC pThreadName( aParameters[ 0 ] );
            TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Heap_Dump (%S)"), this, &pThreadName ) );
            err = iMemSpy->PerformOperation( EMemSpyClientServerOpHeapData, pThreadName );
            }
        }
    else if ( aCommand.CompareF( KMemSpyCmdOpenFile ) == 0 )
        {
        if  ( paramCount == 0 )
            {
            // Dump heap data for all threads
            TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - OpenFile (all threads)", this ) );
            err = iMemSpy->PerformOperation( EMemSpyClientServerOpOpenFiles );
            }
        else if ( paramCount >= 1 )
            {
            // Dump heap data for named thread
            const TPtrC pThreadName( aParameters[ 0 ] );
            TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - OpenFile (%S)"), this, &pThreadName ) );
            err = iMemSpy->PerformOperation( EMemSpyClientServerOpOpenFiles, pThreadName );
            }
        }
    else if ( !iIsBatch && FindBatchFile( batchFile ) == KErrNone )
        {
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Batch file: %S"), this, &batchFile ) );
        PerformBatchL( batchFile );
        }
    else
        {
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - this: 0x%08x - Unsupported Command: %S"), this, &aCommand ) );
        }

    TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::PerformSingleOpL() - END - err: %d, this: 0x%08x, cmd: %S" ), err, this, &aCommand ) );

    // Calculate duration
    TTime timeEnd;
    timeEnd.HomeTime();
    TTimeIntervalSeconds interval( 0 );
    timeEnd.SecondsFrom( timeStart, interval );

    // Print some info
    if  ( err != KErrNone )
        {
        RDebug::Print( _L("[MemSpyCmdLine] COMMAND: [%S] {%02d} {%08d sec} => ERROR: %d"), &aCommand, paramCount, interval.Int(), err );
        }
    else
        {
        RDebug::Print( _L("[MemSpyCmdLine] COMMAND: [%S] {%02d} {%08d sec} => OK"), &aCommand, paramCount, interval.Int() );
        }

    // Spacer
    RDebug::Printf( " " );
    }


void CMemSpyCommandLine::ConnectToMemSpyL()
    {
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - START - this: 0x%08x", this ) );

    TInt err = iMemSpy->Connect();
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - connect #1 err: %d, this: 0x%08x", err, this ) );

    if  ( err == KErrNotFound )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - launching MemSpy... - this: 0x%08x", this ) );
        LaunchMemSpyL();

        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - launched MemSpy - this: 0x%08x", this ) );
        err = iMemSpy->Connect();

        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - connect #2 err: %d, this: 0x%08x", err, this ) );
        }

    User::LeaveIfError( err );

    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ConnectToMemSpyL() - END - ok - this: 0x%08x", this ) );
    }


void CMemSpyCommandLine::LaunchMemSpyL()
    {
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - START - this: 0x%08x", this ) );

    TInt err = KErrGeneral;
    RProcess proc;

    // First try with s60 UI
    err = proc.Create( KMemSpyProcessName1, KNullDesC );
    if  ( err == KErrNone )
        {
        TFullName fullName;
        proc.FullName( fullName );
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - Create S60 UI process successfully... - this: 0x%08x, name: %S"), this, &fullName ) );

        TRequestStatus status;
        proc.Rendezvous( status );
        proc.Resume();

        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - MemSpy resumed, waiting for Rendezvous... - this: 0x%08x", this ) );
        User::WaitForRequest( status );
        err = status.Int();
        proc.Close();

        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - Rendezvous complete: %d, this: 0x%08x", err, this ) );
        }
    if  ( err != KErrNone )
        {
        // Try console UI
        err = proc.Create( KMemSpyProcessName2, KNullDesC );
        if  ( err == KErrNone )
            {
            TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - Create Console UI process successfully... - this: 0x%08x", this ) );

            TRequestStatus status;
            proc.Rendezvous( status );
            proc.Resume();

            TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - MemSpy resumed, waiting for Rendezvous... - this: 0x%08x", this ) );

            User::WaitForRequest( status );
            err = status.Int();
            proc.Close();

            TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - Rendezvous complete: %d, this: 0x%08x", err, this ) );
            }
        }

    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - final error: %d, this: 0x%08x", err, this ) );
    User::LeaveIfError( err );
    User::After( 10 * 1000000 );

    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::LaunchMemSpyL() - END - ok - this: 0x%08x", this ) );
    }


CDesCArray* CMemSpyCommandLine::ReadLinesL( RFile& aFile )
    {
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - START - this: 0x%08x", this ) );
    CDesCArrayFlat* lines = new(ELeave) CDesCArrayFlat( 10 );
    CleanupStack::PushL( lines );

    TInt size = 0;
    User::LeaveIfError( aFile.Size( size ) );
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x, file size: %d", this, size ) );

    // Read file
    HBufC8* narrowBuf = HBufC8::NewLC( size );
    TPtr8 pBufNarrow( narrowBuf->Des() );
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x, reading file..." ) );
    User::LeaveIfError( aFile.Read( pBufNarrow ) );
    if  ( pBufNarrow.Length() <= 2 )
        {
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x - BAD FILE LENGTH", this ) );
        User::Leave( KErrCorrupt );
        }

    // Look for BOM and convert to unicode
    HBufC* unicodeText = HBufC::NewL( size );
    TPtr pUnicodeText( unicodeText->Des() );
    if  ( pBufNarrow[0] == 0xFF && pBufNarrow[1] == 0xFE )
        {
        // It's unicode... and we don't want the BOM, hence -2
        const TInt textLength = size - 2;
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x - UNICODE TEXT - textLength: %d", this, textLength / 2 ) );
        Mem::Copy( (TUint8 *)pUnicodeText.Ptr(), pBufNarrow.Ptr() + 2, textLength );
        pUnicodeText.SetLength( textLength / 2 );
        }
    else
        {
        // It's ASCII, convert it to unicode...
        TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x - ASCII TEXT - textLength: %d", this, size ) );
        for (TInt i = 0; i<size; i++ )
            {
            pUnicodeText.Append( pBufNarrow[ i ] );
            }
        }

    // Discard old narrow text
    CleanupStack::PopAndDestroy( narrowBuf );
    CleanupStack::PushL( unicodeText );

    // Try to extract lines of text...
    HBufC* line = HBufC::NewLC( 1024 );
    TPtr pLine( line->Des() );
    const TInt length = unicodeText->Length();
    for( TInt i=0; i<length; i++ )
        {
        const TChar c( pUnicodeText[ i ] );
        //
        if  ( c == '\r' || c == '\n' )
            {
            pLine.Trim();
            if  ( pLine.Length() )
                {
                TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x - LINE[%03d] %S"), this, lines->Count(), line ) );
                lines->AppendL( pLine );
                }

            pLine.Zero();
            }
        else
            {
            pLine.Append( c );
            }
        }

    // Save last line, just in cae it didn't end with a CR/LF
    pLine.Trim();
    if ( pLine.Length() )
        {
        TRACE( RDebug::Print( _L("[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - this: 0x%08x - LINE[%03d] %S"), this, lines->Count(), line ) );
        lines->AppendL( pLine );
        }

    CleanupStack::PopAndDestroy( 2, unicodeText ); // line & unicodeText
    CleanupStack::Pop( lines );
    TRACE( RDebug::Printf( "[MemSpyCmdLine] CMemSpyCommandLine::ReadLinesL() - END - this: 0x%08x", this ) );

    return lines;
    }


void CMemSpyCommandLine::ParseSWMTParametersL( const CDesCArray& aParameters, TInt& aCategories, TDes& aFilter )
    {
    TInt result(0);
    // Check if the first parameter is a number.
    // In that case other parameters are ignored.
    TLex lex( aParameters[ 0 ] );
    if ( lex.Val( result ) != KErrNone )
        {
        // Parameters were given in text form:
        const TInt count( aParameters.Count() );
        for ( TInt i = 0; i < count ; i++ )
            {
            if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeHeap ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryUserHeap |
                          TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryKernelHeap;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeChunk ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryLocalChunks |
                          TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryGlobalChunks;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeCode ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryRAMLoadedCode;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeStack ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryUserStacks;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeGlobalData ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryGlobalData;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeRamDrive ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryRAMDrive;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeOpenFile ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryOpenFiles;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeDiskSpace ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryDiskusage;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeFbserv ) == 0 ) // enables both FABS and BITM 
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryBitmapHandles;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeHandleGeneric ) == 0 ) // enables both HGEN and HPAS
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryKernelHandles;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeFileServerCache ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryFileServerCache;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeSystemMemory ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategorySystemMemory;
            else if ( aParameters[i].CompareF( KMemSpyCmdSWMTTypeWindowGroup ) == 0 )
                result |= TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryWindowGroups;
            else if ( aParameters[i].Find( KMemSpyCmdSWMTTypeHeapFilter ) == 0 )
                {
                aFilter.Copy( aParameters[i].Right( aParameters[i].Length() -11 ) );
                }
            else
                User::Leave( KErrNotSupported );
            }
        }
    else if ( aParameters.Count() > 1 && aParameters[1].Find( KMemSpyCmdSWMTTypeHeapFilter ) == 0 )
        {
        aFilter.Copy( aParameters[1].Right( aParameters[1].Length() -11 ) );
        }
    aCategories = result;
    }


TInt CMemSpyCommandLine::FindBatchFile( TDes &aFileName )
    {
    if ( BaflUtils::FileExists( iFsSession, aFileName )) return KErrNone;
    if ( !FindFile( aFileName, _L("\\") ) ) return KErrNone;
    if ( !FindFile( aFileName, _L("\\data\\") ) ) return KErrNone;
    if ( !FindFile( aFileName, _L("\\documents\\") ) ) return KErrNone;
    if ( !FindFile( aFileName, _L("\\temp\\") ) ) return KErrNone;
    if ( !FindFile( aFileName, _L("\\system\\temp\\") ) ) return KErrNone;
    // if ( !FindFile( aFileName, _L("\\private\\1000484b\\") ) ) return KErrNone; // Mail folder KErrPermissionDenied  
    return KErrNotFound;
    }


TInt CMemSpyCommandLine::FindFile( TDes &aFileName, const TDesC &aDirPath )
    {
    TFindFile fileFinder( iFsSession );
    TInt err = fileFinder.FindByDir( aFileName, aDirPath );
    if ( !err )
        {
        aFileName.Copy( fileFinder.File() );
        }
    return err;
    }

