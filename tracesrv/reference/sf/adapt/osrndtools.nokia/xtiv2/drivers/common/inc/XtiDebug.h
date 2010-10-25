/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __XTIDEBUG_H__
#define __XTIDEBUG_H__

//#define XTI_DEBUG_ENABLE
/**
 * Internal debug traces are on only in debug builds
 */

#if defined(XTI_DEBUG_ENABLE) && defined(_DEBUG)
#define XTI_TRACE( trace ) { trace; } 
#else
#define XTI_TRACE( trace ) {}
#endif // _DEBUG

#endif // __XTIDEBUG_H__

// End of File
