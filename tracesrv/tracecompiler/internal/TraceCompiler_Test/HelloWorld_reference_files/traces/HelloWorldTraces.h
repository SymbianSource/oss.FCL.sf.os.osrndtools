// Created by TraceCompiler 2.1.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __HELLOWORLDTRACES_H__
#define __HELLOWORLDTRACES_H__

#define KOstTraceComponentID 0xe9fbe6ee

#define TRACE0 0xde0001
#define TRACE1 0xde0002
#define TRACE2 0xde0003
#define TRACE3 0xde0004
#define TRACE4 0xde0005
#define TRACE5 0xde0006
#define TRACE7 0xde0007
#define TRACE8 0xde0008
#define TRACE9 0xde0009
#define TRACE10 0xde000a
#define TRACE11 0xde000b
#define TRACE12 0xde000c
#define TRACE13 0xde000d
#define TRACE14 0xde000e
#define TRACE15 0xde000f
#define TRACE16 0xde0010
#define TRACE17 0xde0011
#define TRACE18 0xde0012
#define TRACE19 0xde0013
#define TRACE20 0xde0014
#define TRACE21 0xde0015
#define TRACE22 0xde0016
#define TRACE23 0xde0017
#define TRACE24 0xde0018
#define TRACE25 0xde0019
#define TRACE26 0xde001a
#define TRACE27 0xde001b
#define TRACE28 0xde001c
#define TRACE29 0xde001d
#define TRACE30 0xde001e
#define TRACE31 0xde001f
#define TRACE32 0xde0020
#define TRACE33 0xde0021
#define TRACE34 0xde0022
#define TRACE35 0xde0023
#define TRACE36 0xde0024


inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC8& aParam1, const TDesC8& aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC16& aParam1, const TDesC16& aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }
#endif


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC8& aParam1, const TDesC16& aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }
#endif


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC16& aParam1, const TDesC8& aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }
#endif


inline TBool OstTraceGen1( TUint32 aTraceID, const TDesC8& aParam1 )
    {
    TBool retval;
    TInt size = aParam1.Size();
    // BTrace assumes that parameter size is atleast 4 bytes
    if (size % 4 == 0)
        {
        TUint8* ptr = ( TUint8* )aParam1.Ptr();
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    else
        {
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        if (size > KOstMaxDataLength)
            {
            size = KOstMaxDataLength;
            }
        TInt sizeAligned = ( size + 3 ) & ~3;
        memcpy( ptr, aParam1.Ptr(), size );
        ptr += size;
        // Fillers are written to get 32-bit alignment
        while ( size++ < sizeAligned )
            {
            *ptr++ = 0;
            }
        ptr -= sizeAligned;
        size = sizeAligned;
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    return retval;
    }


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen1( TUint32 aTraceID, const TDesC16& aParam1 )
    {
    TBool retval;
    TInt size = aParam1.Size();
    // BTrace assumes that parameter size is atleast 4 bytes
    if (size % 4 == 0)
        {
        TUint8* ptr = ( TUint8* )aParam1.Ptr();
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    else
        {
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        if (size > KOstMaxDataLength)
            {
            size = KOstMaxDataLength;
            }
        TInt sizeAligned = ( size + 3 ) & ~3;
        memcpy( ptr, aParam1.Ptr(), size );
        ptr += size;
        // Fillers are written to get 32-bit alignment
        while ( size++ < sizeAligned )
            {
            *ptr++ = 0;
            }
        ptr -= sizeAligned;
        size = sizeAligned;
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    return retval;
    }
#endif


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen3( TUint32 aTraceID, const TDesC16& aParam1, const TDesC8& aParam2, TInt aParam3 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        // Check that there are enough space to next parameter
        if ((length + sizeof ( TInt )) <= KOstMaxDataLength)
            {
            *( ( TInt* )ptr ) = aParam3;
            ptr += sizeof ( TInt );
            length += sizeof ( TInt );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }

inline TBool OstTraceGen3( TUint32 aTraceID, const TDesC16& aParam1, const TDesC8& aParam2, TInt32 aParam3 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        // Check that parameter lenght is not too long
        TInt length2 = aParam2.Size();
        if ((length + length2 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length2 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned2 = ( length2 + 3 ) & ~3;
        if (lengthAligned2 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned2;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        if (length2 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length2 / (aParam2.Size() / aParam2.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam2.Ptr(), length2 );
            ptr += length2;
            // Fillers are written to get 32-bit alignment
            while ( length2++ < lengthAligned2 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned2;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        // Check that there are enough space to next parameter
        if ((length + sizeof ( TInt )) <= KOstMaxDataLength)
            {
            *( ( TInt* )ptr ) = aParam3;
            ptr += sizeof ( TInt );
            length += sizeof ( TInt );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }
#endif


#endif

// End of file

