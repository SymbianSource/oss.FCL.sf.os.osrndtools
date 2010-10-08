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
// TraceCore configuration message subscriber
//

#ifndef __TRACECORECONFIGURATION_H__
#define __TRACECORECONFIGURATION_H__

#include <klib.h>

/**
 * TTraceCoreConfiguration function to choosed in DTraceCoreConfiguration::GetSet.
 */
enum TTraceCoreConfiguration
    {
    EGetCurrentWriterType = 0,     // Return type TWriterType
    ESetWriter = 1,     // Return KErrNone/KErrNotFound
    ELastFunction       // Return KErrNotSupported
    };


class DTraceCoreConfiguration : public DBase
    {
public:
    DTraceCoreConfiguration();
    virtual ~DTraceCoreConfiguration();
    
    /**
     * Initializes configuration
     */
    TInt Init();
    
    
    /**
     * @param aFunc Function choosed (@see TTraceCoreConfiguration)
     * @return Depending of aFunc paramter (@see TTraceCoreConfiguration9
     */
    IMPORT_C static TUint32 GetSet(TTraceCoreConfiguration aFunc, TUint32 aParam);
    };

#endif /* __TRACECORECONFIGURATION_H__*/

// End of File
