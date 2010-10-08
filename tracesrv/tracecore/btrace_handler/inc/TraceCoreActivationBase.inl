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
// Base class for activation classes
//

#ifndef __TRACECOREACTIVATIONBASE_INL__
#define __TRACECOREACTIVATIONBASE_INL__

#include <kern_priv.h>
#include "TraceCoreActivationBase.h"


inline TInt DTraceCoreActivationBase::ReadLock()
    {
#if defined(__SMP__)
   return __SPIN_LOCK_IRQSAVE(iActivationReadLock);
#else
    return NKern::DisableAllInterrupts();
#endif
    }


inline void DTraceCoreActivationBase::ReadUnlock(TInt aIrqLevel)
    {
#if defined(__SMP__)
    __SPIN_UNLOCK_IRQRESTORE(iActivationReadLock, aIrqLevel);
#else
    NKern::RestoreInterrupts(aIrqLevel);
#endif
    }


#endif

