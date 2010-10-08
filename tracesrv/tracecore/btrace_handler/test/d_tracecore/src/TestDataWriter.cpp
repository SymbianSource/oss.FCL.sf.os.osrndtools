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
//
//

#include <memmodel/epoc/platform.h>
#include <kernel.h>

#include "TestDataWriter.h"


#if defined _DEBUG

void Panic(TInt aReason)
    {
    TDW_PRINTF("Panic with reason %d", aReason);
    Kern::PanicCurrentThread(_L("Trace core TestDataWriter PANIC"), aReason);
    }

#endif

DTestDataWriter::DTestDataWriter()
: DTraceCoreWriter(EWriterTypeTest)
, iNotifier(NULL)
	{
	}

DTestDataWriter::~DTestDataWriter()
	{
	}

TInt DTestDataWriter::Close()
    {  
    return KErrNone; 
    }

TInt DTestDataWriter::Init()
    {    
    return Register();
    }
  
TUint32 DTestDataWriter::WriteStart( TWriterEntryType aType )
	{        
    if(iNotifier)
	    {
        iNotifier->WriteStart();
	    }

  iBuffer.Zero();
  if ( aType == EWriterEntryTrace )
    {
	iBuffer.SetLength( 1 );
	iBuffer[0] = SYMBIAN_TRACE;
    }
  else if ( aType == EWriterEntryAscii )
    {
	iBuffer.SetLength( 1 );
	iBuffer[0] = PRINTF_TRACE;
    }
  
    return 0;
	}

void DTestDataWriter::WriteEnd( TUint32 /*aEntryId*/ )
    { 
    if(iNotifier)
        {
        iNotifier->WriteComplete(TNotifyData((TUint32)iBuffer.Ptr(), iBuffer.Length()));
        }
    }

void DTestDataWriter::WriteData( TUint32 /*aEntryId*/, TUint8 aData )
	{
	TInt len = iBuffer.Length();
	TInt sz =  sizeof( TUint8 );
	
	if(len + sz  < iBuffer.MaxLength())
	    {
        iBuffer.Append((TUint8*) &aData, sz);
        }
	}

void DTestDataWriter::WriteData( TUint32 /*aEntryId*/, TUint16 aData )
	{
    TInt len = iBuffer.Length();
    TInt sz =  sizeof( TUint16 );
    if(len + sz < iBuffer.MaxLength())
        {
        iBuffer.Append((TUint8*) &aData, sz);
        }
	}

void DTestDataWriter::WriteData( TUint32 /*aEntryId*/, TUint32 aData )
	{

	TInt len = iBuffer.Length();
    TInt sz =  sizeof( TUint32 );

    if(len + sz < iBuffer.MaxLength())
        {        
        iBuffer.Append((TUint8*) &aData, sz);
	    }
	}


