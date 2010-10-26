/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

//  Include Files  
#include "HelloWorld.h"
#include <e32base.h>
#include <e32debug.h>
#include <e32std.h>
#include <e32cons.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "HelloWorldTraces.h"
#endif
// Console
//  Constants
_LIT(KTextConsoleTitle, "Console");
_LIT(KTextFailed, " failed, leave code = %d");
_LIT(KTextPressAnyKey, " [press any key]\n");

_LIT(buf60, "123456789_123456789_123456789_123456789_123456789_123456789X");
_LIT(buf333, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_12X");
_LIT(buf600, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789X");

_LIT8(ascii_buf600, "123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789X");


//  Global Variables
LOCAL_D CConsoleBase* console; // write all messages to this


//  Local Functions

LOCAL_C void MainL()
    {
        
    //
    // add your program code here, example code below
    //

    OstTrace0(UNICODE_STRING_TEST, TRACE0, "Test different kind of unicode strings.");
    OstTraceExt2( UNICODE_STRING_TEST , TRACE1,
            "ascii(5): %s ascii(4): %s", _L8( "STR08" ), _L8("STR8") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE2,
            "unicode(5): %S unicode(5): %S", _L( "STR16" ), _L("STR16") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE3,
            "ascii(4): %s unicode(5): %S", _L8( "STR8" ), _L("STR16") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE4,
            "unicode(5): %S ascii (5): %s", _L( "STR16" ), _L8("STR08") );

    OstTraceExt1( UNICODE_STRING_TEST, TRACE5,
            "ascii(5): %s", _L8( "STR08" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE7,
            "unicode(5): %S", _L( "STR16" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE8,
            "ascii(1): %s", _L8( "1" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE9,
            "unicode(1): %S", _L( "1" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE10,
            "ascii(4): %s", _L8( "STR8" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE11,
            "unicode(4): %S", _L( "ST16" ));

    OstTraceExt2( UNICODE_STRING_TEST, TRACE12,
            "ascii(6): %s unicode (8): %S", _L8( "STR008" ), _L("STR00016") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE13,
            "unicode(4): %S ascii(8): %s", _L( "ST16" ), _L8("STR00008") );

    OstTraceExt3( UNICODE_STRING_TEST, TRACE14, "unicode(4): %S ascii(8): %s int: %d", _L( "ST16" ), _L8("STR00008"), 7 );
    OstTraceExt3( UNICODE_STRING_TEST, TRACE15, "unicode(4): %S ascii(0): %s int: %d", _L( "ST16" ), _L8(""), 6 );

    OstTraceExt1( UNICODE_STRING_TEST, TRACE16, "unicode(60): %S", buf60);
    OstTraceExt1( UNICODE_STRING_TEST, TRACE17, "unicode(333): %S", buf333);
    OstTraceExt1( UNICODE_STRING_TEST, TRACE18, "unicode(600): %S", buf600);

    OstTraceExt2( UNICODE_STRING_TEST, TRACE19,
            "unicode(5): %S ascii(0): %s", _L( "STR16" ), _L8("") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE20,
            "unicode(0): %S ascii(5): %s", _L( "" ), _L8("STR08") );

    OstTraceExt1( UNICODE_STRING_TEST, TRACE21,
            "ascii(0): %s", _L8( "" ));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE22,
            "unicode(0): %S", _L( "" ));

    OstTraceExt2( UNICODE_STRING_TEST, TRACE23,
            "ascii(4): %s unicode(0): %S", _L8( "STR8" ), _L("") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE24,
            "ascii(0): %s unicode(5): %S", _L8( "" ), _L("STR16") );

    OstTraceExt2( UNICODE_STRING_TEST , TRACE25,
            "ascii(5): %s ascii(0): %s", _L8( "STR08" ), _L8("") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE26,
            "unicode(5): %S unicode(0): %S", _L( "STR16" ), _L("") );
            
    OstTraceExt2( UNICODE_STRING_TEST , TRACE27,
            "ascii(0): %s ascii(4): %s", _L8( "" ), _L8("STR8") );

    OstTraceExt2( UNICODE_STRING_TEST, TRACE28,
            "unicode(0): %S unicode(5): %S", _L( "" ), _L("STR16") );

    OstTraceExt3( UNICODE_STRING_TEST, TRACE29, "unicode(333): %S ascii(0): %s int: %d", buf333, _L8(""), 8 );
 
    OstTraceExt2( UNICODE_STRING_TEST, TRACE30, "unicode(333): %S unicode(333): %S", buf333, buf333);
    
    OstTraceExt1( UNICODE_STRING_TEST, TRACE31,
            "unicode(65): %S", _L( "Application_FileBrowser_004401103753378_29575020080101_090257.bin" ));
    
    
    _LIT(KFmtTextFilePrs, "Application_%s_%s_%s.bin");
    TBuf<120> fileName;
    
    TTime time;
    time.HomeTime();
    TDateTime dateTime = time.DateTime(); 
    TBuf<32> dt;
    _LIT(KDateTimeFormat,"%06d%04d%02d%02d_%02d%02d%02d");
    
    dt.Format(KDateTimeFormat, dateTime.MicroSecond(), dateTime.Year(), dateTime.Month()+1, 
                 dateTime.Day()+1, dateTime.Hour(), dateTime.Minute(), dateTime.Second());


	TBuf<60> prsName = _L("FileBrowser");
    TBuf<60> imeiName = _L("004401103753378");

   
    fileName.Format(KFmtTextFilePrs, prsName.PtrZ(), imeiName.PtrZ(), dt.PtrZ());
    
    OstTraceExt1( UNICODE_STRING_TEST, TRACE32, "CDataOutPut::DumpL() %S", fileName);
    
    OstTraceExt1( UNICODE_STRING_TEST, TRACE33, "unicode abc =  %S", _L("abc"));
    OstTraceExt1( UNICODE_STRING_TEST, TRACE34, "ascii abc =  %s", _L8("abc"));

    OstTraceExt1( UNICODE_STRING_TEST, TRACE35, "unicode(333): %S", buf333);
 
    OstTraceExt1( UNICODE_STRING_TEST, TRACE36, "ascii(600): %s", ascii_buf600);

   
    console->Write(_L("Hello, world!\n"));

    }

LOCAL_C void DoStartL()
    {
    // Create active scheduler (to run active objects)
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    MainL();

    // Delete active scheduler
    CleanupStack::PopAndDestroy(scheduler);

    }

//  Global Functions

GLDEF_C TInt E32Main()
    {
    // Create cleanup stack
    __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    // Create output console
    TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(
                            KConsFullScreen, KConsFullScreen)));
    if (createError)
        return createError;
    // Run application code inside TRAP harness, wait keypress when terminated
    TRAPD(mainError, DoStartL());
    if (mainError)
        console->Printf(KTextFailed, mainError);
    console->Printf(KTextPressAnyKey);
    console->Getch();

    delete console;
    delete cleanup;
    __UHEAP_MARKEND;
    return KErrNone;
    }

