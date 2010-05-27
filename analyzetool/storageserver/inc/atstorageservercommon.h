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
* Description:  Includes common constants and enums for the AnalyzeTool 
*                StorageServer client and server.
*
*/



#ifndef ATSTORAGESERVERCOMMON_H
#define ATSTORAGESERVERCOMMON_H


//  INCLUDES

#include <e32base.h>


// CONSTANTS

// Server's file name
_LIT(KStorageServerFile, "AToolStorageServer.exe");

// server name
_LIT(KStorageServerName,"AToolStorageServer");


// The path of the storage file
#ifdef __WINS__
_LIT( KATDataFilePath, ":\\logs\\analyzetool\\" );
#else
_LIT( KATDataFilePath, ":\\analyzetool\\" );
_LIT( KATDataFilePath2, ":\\data\\analyzetool\\" );
#endif

// The word size in the current system is 32 bits, which is 4 bytes.
const TInt KWordSize = 4;

// Leak array granularity value
const TInt KLeakArrayGranularity = 100;

// DATA TYPES

/** Storage Server panic codes */
enum TStorageServerPanics
    {
    EAToolBadRequest = 1, // An undefined operation request from the client
    EAToolBadArgument,    // Illegal arguments / function parameters   
    EAToolNotAllowed,     // An operation is not allowed in the current program state
    EAToolInternalError,   // An internal error has occured
    EAToolIllegalLogOption // An undefined log option has been requested 
    };


/** Panic categories */
_LIT(KCategoryServer,"STSEServer");
_LIT(KCategoryClient,"STSEClient");

inline void StorageServerPanic( const TDesC &aCategory, TStorageServerPanics aReason )
    {
    User::Panic(aCategory, aReason);
    }
  
#endif      // ATSTORAGESERVERCOMMON_H   

// End of File
