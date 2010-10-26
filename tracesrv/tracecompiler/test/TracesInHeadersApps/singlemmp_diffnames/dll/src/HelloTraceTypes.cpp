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



#include <e32std.h>

#include "HelloTraceTypes.h"

#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "HelloTraceTypesTraces.h"
#endif

void HelloTraceTypes::TestBoolean()
    {
    
    _LIT(KTextConsoleTitle, "Test bool .... : ");
    bool joo = true;
    OstTraceExt1( TRACE_INTERNALS, _TEST_BOOL_HHU_EXT1, "My bool = %hhu", (TUint8)joo);
    OstTraceExt2( TRACE_INTERNALS, _TEST_BOOL_HHU_EXT2, "Text: %S. My bool = %hhu", KTextConsoleTitle, (TUint8)joo);
    OstTraceExt2( TRACE_INTERNALS, _TEST_BOOL_U_EXT2, "Text: %S. My bool = %u", KTextConsoleTitle, (TUint32)joo);
    }

void HelloTraceTypes::SignedIntegers()
    {
    TInt8  eightBit     = - (TInt8)  1 << 6;
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_SIGNEDINTEGERS_8, "8 bit signed: %hhd", eightBit );
    
    _LIT(KText1, "Test %hhd .... : ");
    OstTraceExt2( TRACE_INTERNALS, _TEST_HHD_EXT2, "Text: %S. My data = %hhd", KText1, (TInt8)eightBit);
       
    TInt16 sixteenBit   = - (TInt16) 1 << 14; 
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_SIGNEDINTEGERS_16, "16 bit signed: %hd", sixteenBit );
    
    _LIT(KText2, "Test %hd .... : ");
     OstTraceExt2( TRACE_INTERNALS, _TEST_HD_EXT2, "Text: %S. My data = %hd", KText2, (TInt16)sixteenBit);
  

    TInt32 thirtyTwoBit = - (TInt32) 1 << 30; 
    OstTrace1( TRACE_INTERNALS, HELLOTRACETYPES_SIGNEDINTEGERS_32, "32 bit signed: %d", thirtyTwoBit );
    
    _LIT(KText3, "Test %d .... : ");
     OstTraceExt2( TRACE_INTERNALS, _TEST_D_EXT2, "Text: %S. My data = %d", KText3, thirtyTwoBit);
  

    TInt64 sixtyFourBit = - (TInt64) 1 << 62; 
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_SIGNEDINTEGERS_64, "64 bit signed: %Ld", sixtyFourBit );
    }

void HelloTraceTypes::UnsignedIntegers()
    {
    TUint8  eightBit     = (TUint8)  1 << 6;
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_UNSIGNEDINTEGERS_8, "8 bit unsigned: %hhu", eightBit );
    
    _LIT(KText1, "Test %hhu .... : ");
    OstTraceExt2( TRACE_INTERNALS, _TEST_HHU_EXT2, "Text: %S. My data = %hhu", KText1, (TUint8)eightBit);

    
    TUint16 sixteenBit   = (TUint16) 1 << 14; 
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_UNSIGNEDINTEGERS_16, "16 bit unsigned: %hu", sixteenBit );

    _LIT(KText2, "Test %hu .... : ");
     OstTraceExt2( TRACE_INTERNALS, _TEST_HU_EXT2, "Text: %S. My data = %hu", KText2, (TUint16)sixteenBit);

    
    TUint32 thirtyTwoBit = (TUint32) 1 << 30; 
    OstTrace1( TRACE_INTERNALS, HELLOTRACETYPES_UNSIGNEDINTEGERS_32, "32 bit unsigned: %u", thirtyTwoBit );

    _LIT(KText3, "Test %u .... : ");
        OstTraceExt2( TRACE_INTERNALS, _TEST_U_EXT2, "Text: %S. My data = %u", KText3, thirtyTwoBit);
        
    TUint64 sixtyFourBit = (TUint64) 1 << 62; 
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_UNSIGNEDINTEGERS_64, "64 bit unsigned: %Lu", sixtyFourBit );
    }

void HelloTraceTypes::Descriptors()
    {
    _LIT8(KAscii, "Some ASCII text");
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_ASCII, "8 bit descriptor: %s", KAscii() );

    _LIT16(KUnicode, "Some unicode text");    
    OstTraceExt1( TRACE_INTERNALS, HELLOTRACETYPES_UNICODE, "16 bit descriptor: %S", KUnicode() );
    }
