// Created by TraceCompiler 2.1.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __TRACECOMPILERTESTERCOMMON1TRACES_H__
#define __TRACECOMPILERTESTERCOMMON1TRACES_H__

#define KOstTraceComponentID 0x20011112

#define DTRACECOMPILERTESTERCOMMON1_SUMVALUES_ENTRY 0x8a0001, aValue1, aValue2, aValue3, aValue4
#define DTRACECOMPILERTESTERCOMMON1_SUMVALUES_EXIT 0x8a0002


inline TBool OstTraceGenExt( TUint32 aTraceID, TInt aParam1, TInt aParam2, TUint8 aParam3, TUint8 aParam4, TUint aParam5 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 16 ];
        TUint8* ptr = data;
        *( ( TInt* )ptr ) = aParam1;
        ptr += sizeof ( TInt );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        *( ( TUint8* )ptr ) = aParam3;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = aParam4;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = 0;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = 0;
        ptr += sizeof ( TUint8 );
        *( ( TUint* )ptr ) = aParam5;
        ptr += sizeof ( TUint );
        ptr -= 16;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 16 );
        }
    return retval;
    }

inline TBool OstTraceGenExt( TUint32 aTraceID, TInt32 aParam1, TInt32 aParam2, TUint8 aParam3, TUint8 aParam4, TUint32 aParam5 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 16 ];
        TUint8* ptr = data;
        *( ( TInt* )ptr ) = aParam1;
        ptr += sizeof ( TInt );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        *( ( TUint8* )ptr ) = aParam3;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = aParam4;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = 0;
        ptr += sizeof ( TUint8 );
        *( ( TUint8* )ptr ) = 0;
        ptr += sizeof ( TUint8 );
        *( ( TUint* )ptr ) = aParam5;
        ptr += sizeof ( TUint );
        ptr -= 16;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 16 );
        }
    return retval;
    }


inline TBool OstTraceGen2( TUint32 aTraceID, TUint aParam1, TInt aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TUint* )ptr ) = aParam1;
        ptr += sizeof ( TUint );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }

inline TBool OstTraceGen2( TUint32 aTraceID, TUint32 aParam1, TInt32 aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TUint8 data[ 8 ];
        TUint8* ptr = data;
        *( ( TUint* )ptr ) = aParam1;
        ptr += sizeof ( TUint );
        *( ( TInt* )ptr ) = aParam2;
        ptr += sizeof ( TInt );
        ptr -= 8;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, 8 );
        }
    return retval;
    }


#endif

// End of file

