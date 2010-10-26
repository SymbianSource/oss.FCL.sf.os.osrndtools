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

#ifndef RTESTSAMPLER_H
#define RTESTSAMPLER_H

#ifndef __KERNEL_MODE__
#include <utf.h>
#endif

#include <piprofiler/PluginSampler.h>

class RtestSampler :  public RBusLogicalChannel 
{   
public:
#ifndef __KERNEL_MODE__
    inline TInt Open();
#endif
};
_LIT(KName,"PIProfilerGeneralsSampler");

#ifndef __KERNEL_MODE__
    inline TInt RtestSampler::Open()
    {
        return DoCreate(KName,TVersion(1,0,1),KNullUnit,NULL,NULL);
    }
#endif


#endif //RTESTSAMPLER_H
