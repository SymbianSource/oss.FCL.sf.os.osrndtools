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
* Description:  Declaration of the class TATDllInfo.
*
*/


#ifndef ATDLLINFO_H
#define ATDLLINFO_H

// INCLUDES
#include <u32std.h>

/**
*  Stores information of process loaded library
*/
class TATDllInfo
    {
    
    public: // Constructors
        
        /**
        * C++ default constructor.
        * @param aStartAddress Start address of the library 
        * @param aEndAddress The end address of the library 
        * @param aLoadTime The current time in a 64-bit form.
        * @param aDllName The name of the library
        */
        TATDllInfo( const TUint32 aStartAddress, const TUint32 aEndAddress, 
        		const TInt64& aLoadTime, const TDesC8& aDllName );

    public: // New functions

        /**
        * Returs library start address
        * @return TUint32 start address of the library
        */
        TUint32 StartAddress();
        
        /**
		* Returns library end address
		* @return TUint32 end address of the library
		*/
		TUint32 EndAddress();
                
        /**
        * Gets specific library name
        * return TDes8 Name of the library
        */
        TDes8& Name();
        
        /**
		* Returns library load time
		* @return TInt64 library load time
		*/
        TInt64 LibraryLoadTime();
        		
        /**
		* Checks if two objects of this class match based on the objects's
        * saved library name.
		* @param aFirst Library object
		* @param aSecond Library object
		* return TBool ETrue, if the two objects match. EFalse otherwise.
		*/
        static TBool Match( const TATDllInfo& aFirst, const TATDllInfo& aSecond );
                
    public: // Member variables

        /* Start address of the loaded dll */
    	const TUint32 iStartAddress;

        /* End address of the loaded dll */
        const TUint32 iEndAddress;

        /* Loaded dll name */
        TBuf8<KMaxLibraryName> iName;
        
        /** For storing the time when DLL has loaded. */
        const TInt64 iLoadTime;
    };

#endif // ATDLLINFO_H

// End of File

