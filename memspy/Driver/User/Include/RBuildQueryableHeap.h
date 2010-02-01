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

#ifndef RBUILDQUERYABLEHEAP_H
#define RBUILDQUERYABLEHEAP_H

// System includes
#include <e32cmn.h>

// User includes
#include <memspy/driver/memspydriverenumerationsshared.h>


class RBuildQueryableHeap : public RHeap
    {
public: // API
    TBool IsDebugEUser() const;
    TInt CellHeaderSize( TMemSpyDriverCellType aType ) const;

public: // Exposure of RHeap protected data members
    inline TInt MinCellSize() const
        {
        return iMinCell;
        }

    };



#endif
