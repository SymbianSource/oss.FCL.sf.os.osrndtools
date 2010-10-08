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

#ifndef __TEST_DATA_WRITER_H_INCLUDED___
#define __TEST_DATA_WRITER_H_INCLUDED___

#include <kernel.h>
#include <TraceCoreWriter.h>

#include "TestDataWriterNotifier.h"


class DTestDataWriter : public DTraceCoreWriter {
public:
    DTestDataWriter();
    ~DTestDataWriter();

    TInt Init();
    TInt Close();

    TUint32 WriteStart( TWriterEntryType aType );  
    void WriteEnd( TUint32 aEntry );
    void WriteData( TUint32 aEntry, TUint8 aData );  
    void WriteData( TUint32 aEntry, TUint16 aData );
    void WriteData( TUint32 aEntry, TUint32 aData );
    TWriterType GetWriterType();
    
 private:
     TInt iInterrupts;
     TBuf8<KTcTdwBufSize> iBuffer;     
     MTestWriterNotifier* iNotifier;
   
     friend class DTraceCoreTestWriter;
};


#endif  // __TEST_DATA_WRITER_H_INCLUDED___
