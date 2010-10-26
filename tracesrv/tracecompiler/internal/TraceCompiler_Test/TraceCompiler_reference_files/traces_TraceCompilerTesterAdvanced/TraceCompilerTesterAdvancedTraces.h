// Created by TraceCompiler 2.1.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __TRACECOMPILERTESTERADVANCEDTRACES_H__
#define __TRACECOMPILERTESTERADVANCEDTRACES_H__

#define KOstTraceComponentID 0x20011111

#define HELLO_WORLD 0xde0001
#define HELLO_MOON 0x8a0001
#define TEST_TRACE_2 0x8a0002
#define POINTER_PARAMETER_TRACE1 0xdf0001
#define POINTER_PARAMETER_TRACE2 0xdf0002
#define ASCII60 0xe00001
#define ASCII300 0xe00002
#define ASCII512 0xe00003
#define ASCII600 0xe00004
#define ASCII300_ASCII512 0xe00005
#define ASCII600_ASCII300 0xe00006
#define ASCII333_ASCII333 0xe00007
#define ASCII1 0xe10001
#define ASCII2 0xe10002
#define ASCII3 0xe10003
#define ASCII4 0xe10004
#define ASCII5 0xe10005
#define STATE_TRACE_0 0x870001
#define STATE_TRACE_1 0x870002
#define TRACE_DATA_TRACE 0xe20001
#define POST1 0xe30001


inline TBool OstTraceGen2( TUint32 aTraceID, const TAny* aParam1, const TAny* aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( const TAny** )ptr ) = aParam1;
        ptr += sizeof ( const TAny* );
        *( ( const TAny** )ptr ) = aParam2;
        ptr += sizeof ( const TAny* );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }


inline TBool OstTraceGen1( TUint32 aTraceID, const TAny* aParam1 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 4 ];
        TUint8* ptr = data;
        *( ( const TAny** )ptr ) = aParam1;
        ptr += sizeof ( const TAny* );
        ptr -= 4;
        retval = BTraceFilteredContext12( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, *( ( TUint32* )ptr ) );
        }
    return retval;
    }


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


inline TBool OstTraceGen3( TUint32 aTraceID, const TDesC8& aParam1, const TDesC8& aParam2, TUint aParam3 )
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
        if ((length + sizeof ( TUint )) <= KOstMaxDataLength)
            {
            *( ( TUint* )ptr ) = aParam3;
            ptr += sizeof ( TUint );
            length += sizeof ( TUint );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }

inline TBool OstTraceGen3( TUint32 aTraceID, const TDesC8& aParam1, const TDesC8& aParam2, TUint32 aParam3 )
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
        if ((length + sizeof ( TUint )) <= KOstMaxDataLength)
            {
            *( ( TUint* )ptr ) = aParam3;
            ptr += sizeof ( TUint );
            length += sizeof ( TUint );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }


#endif

// End of file

