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

#ifndef MEMSPYCOMMANDLINE_H
#define MEMSPYCOMMANDLINE_H

// System includes
#include <e32base.h>
#include <f32file.h>
#include <badesca.h>

#ifdef _DEBUG
#   define TRACE( x ) x
#else
#   define TRACE( x )
#endif

// Constants
const TInt KMemSpyCommandLineMaxLength = 128;

// Classes referenced
class RFs;
class CCommandLineArguments;
class RMemSpyEngineClientInterface;

class CMemSpyCommandLine : public CBase
    {
public:
    static CMemSpyCommandLine* NewLC();
    ~CMemSpyCommandLine();

private:
    CMemSpyCommandLine();
    void ConstructL();

public: // API
    void PerformBatchL( const TDesC& aFileName );
    void PerformOpL( const CCommandLineArguments& aCommandLine );
    void PerformSingleOpL( const TDesC& aCommand, const CDesCArray& aParameters );

private: // Internal methods
    void ConnectToMemSpyL();
    void LaunchMemSpyL();
    CDesCArray* ReadLinesL( RFile& aFile );
    void ParseSWMTParametersL( const CDesCArray& aParameters, TInt& aCategories, TDes& aFilter );
    TInt FindBatchFile( TDes &aFileName );
    TInt FindFile( TDes &aFileName, const TDesC &aDirPath );

private: // Data members
    RFs iFsSession;
    RMemSpyEngineClientInterface* iMemSpy;
    TBool iIsBatch; // For avoiding recursion
    };


#endif
