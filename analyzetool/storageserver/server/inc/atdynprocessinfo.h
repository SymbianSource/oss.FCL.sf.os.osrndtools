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
* Description:  Declaration of the class CATDynProcessInfo
*
*/


#ifndef ATDYNPROCESSINFO_H
#define ATDYNPROCESSINFO_H


//  INCLUDES
#include    <e32base.h>
#include    <analyzetool/atcommon.h>
#include	"atdllinfo.h"

// FORWARD DECLARATIONS
class CATStorageServerSession;

// CLASS DECLARATION

/**
*  A class that can store different process related information. The information
*  is associated to a particular process wiht a member telling a process ID.
*  All the other information but the process ID and the pointer to this process's
*  associated session object can be subject to change dynamically, run-time.
*/
class CATDynProcessInfo : public CBase
    {
    public: // Constructor

        /**
        * Constructor. 
        */ 
        CATDynProcessInfo();
    
        /**
        * Constructor. 
        * @param aProcessId A process ID.
        * @param aSessionObject A pointer to this process's associated session object.
        * @param aDlls The loaded DLLs of the associated process.
        */
        CATDynProcessInfo( TUint aProcessId,
                           CATStorageServerSession* aSessionObject,
                           const RArray<TATDllInfo>& aDlls );
        
        /**
        * Constructor.
        * @param aProcessId A process ID.
        * @param aSessionObject A pointer to this process's associated session object.
        */
        CATDynProcessInfo( TUint aProcessId,
                           CATStorageServerSession* aSessionObject );
                           
        /**
        * Constructor.
        * @param aProcessId A process ID.
        */
        CATDynProcessInfo( TUint aProcessId );

        /**
        * Destructor
        */
        virtual ~CATDynProcessInfo();
        
    public:
        
        /**
        * Compares two objects of this class based on the process ID.
        * @param aFirst The first object of this class to be compared.
        * @param aSecond The second object of this class to be compared.
        * @return  Zero, if the two objects are equal. A negative value,
        *   if the first object is less than the second. A positive value,
        *   if the first object is greater than the second.
        */
        static TInt Compare( const CATDynProcessInfo& aFirst,
                             const CATDynProcessInfo& aSecond );
        
    public:

        /** The ID of the process. */
        const TUint iProcessId;
        
        /** A pointer to the session object associated with this process. */
        CATStorageServerSession* const iSessionObject;
        
        /** The DLLs loaded by the associated process */
        RArray<TATDllInfo> iDlls;
    };
  
#endif      // ATDYNPROCESSINFO_H
