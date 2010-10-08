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
// TraceCore TraceCompiler Arguments Macro
//

#ifndef __TRACECORETCOMARGMACRO_H__
#define __TRACECORETCOMARGMACRO_H__

/**
 * Macro used to suppress build warnings when OST_TRACE_COMPILER_IN_USE is not defined
 */
#if defined( OST_TRACE_COMPILER_IN_USE )

#define TCOM_ARG(x)  x

#else

#define TCOM_ARG(x)

#endif


#endif

// End of File
