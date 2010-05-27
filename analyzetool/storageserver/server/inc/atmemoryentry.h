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
* Description:  Declaration of the class CATMemoryEntry
*
*/


#ifndef ATMEMORYENTRY_H
#define ATMEMORYENTRY_H


//  INCLUDES
#include <e32base.h>


// CLASS DECLARATION

/**
*  A class for storing memory entries when the server is informed that memory has been
*  allocated. The class stores the memory address of a new allocation and a pointer to
*  a buffer including the current call stack. It also stores a time stamp, and the size
*  of the allocation. 
*     The class has member methods for evaluating two different objects of this class
*  with each other. In the construction the class takes a pointer to the call stack
*  buffer, but it does not create the buffer itself. However, it is responsible for
*  deleting the buffer in the destruction.
*/
class CATMemoryEntry : public CBase
    {
    public: // Constructor

        /**
        * Constructor.
        * @param aMemAddress An address to allocated memory.
        * @param aCallstackBuf A pointer to a buffer containing call stack's memory
        *   addresses.
        * @param aAllocTime The current time in a 64-bit form.
        * @param aAllocSize The size of an allocated memory chunk.
        */ 
        CATMemoryEntry( TUint32 aMemAddress, const CBufFlat* aCallstackBuf,
                        const TInt64& aAllocTime, TInt aAllocSize );

        /**
        * Destructor
        */
        virtual ~CATMemoryEntry();

    public:
        /**
        * Compares two objects of this class based on the allocation time.
        * @param aFirst The first object of this class to be compared.
        * @param aSecond The second object of this class to be compared.
        * @return  Zero, if the two objects are equal. A negative value,
        *   if the first object is less than the second. A positive value,
        *   if the first object is greater than the second.
        */
        static TInt Compare( const CATMemoryEntry& aFirst,
                             const CATMemoryEntry& aSecond );

        /**
        * Checks if two objects of this class match based on the objects's
        * saved memory allocation addresses.
        * @param aFirst The first object of this class to be evaluated.
        * @param aSecond The second object of this class to be evaluated.
        * @return ETrue, if the two objects match. EFalse otherwise.
        */
        static TBool Match( const CATMemoryEntry& aFirst,
                            const CATMemoryEntry& aSecond );

    public:

        /** For storing an address of a memory allocation. */
        const TUint32 iMemAddress;

        /** A pointer to an array for storing the current call stack. */
        const CBufFlat* const iCallstackBuf;
        
        /** For storing the time when an allocation has occured. */
        const TInt64 iAllocTime;
        
        /** For storing the size of an allocation. */
        const TInt iAllocSize;
        
    };
  
  
#endif      // ATMEMORYENTRY_H
