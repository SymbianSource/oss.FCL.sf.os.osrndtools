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

//- Include Files  ----------------------------------------------------------

#include <TraceCoreConstants.h>
#include "TraceCoreRegisterWriter.h" 
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "TraceCoreRegisterWriterTraces.h"
#endif


/**
 * Constructor
 */
DTraceCoreRegisterWriter::DTraceCoreRegisterWriter()
: DTraceCoreWriter( EWriterTypeXTI )
	{
	}


/**
 * Destructor
 */
DTraceCoreRegisterWriter::~DTraceCoreRegisterWriter()
	{
	}


/**
 * Registers this writer
 */
TInt DTraceCoreRegisterWriter::Register()
	{
	OstTrace0( TRACE_BORDER, DTRACECOREREGISTERWRITER_REGISTER_ENTRY, "> DTraceCoreRegisterWriter::Register");
	InitProperties( iProperties );
	
	// Register to TraceCore
    TInt retval = DTraceCoreWriter::Register();
	OstTrace1( TRACE_BORDER, DTRACECOREREGISTERWRITER_REGISTER_EXIT, "< DTraceCoreRegisterWriter::Register. Ret:%d", retval );
	return retval;
	}

// End of File
