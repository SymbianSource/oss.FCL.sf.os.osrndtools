// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
//

#ifndef __TRACE_CERTIFICATION_H__
#define __TRACE_CERTIFICATION_H__


class TraceCertification
    {			  
    public:
        /**
         * Get the trace certification status.
         * This method is called once by TraceCore on initialisation (i.e. during boot of device) to
         * check whether tracing is certified on the device. It is not called again.
         * If this method returns EFalse, then TraceCore will suppress traces (except Printf traces).
         * If this method return ETrue then all traces will come through.
         * If reimplementing this method it is recommended that the call is not too time consuming,
         * so that boot time of device is not overly impacted.
         * @publishedAll
         * @return ETrue if tracing is certified or EFalse otherwise
         */
        IMPORT_C static TBool IsTraceCertified();

    };

#endif // __TRACE_CERTIFICATION_H__
