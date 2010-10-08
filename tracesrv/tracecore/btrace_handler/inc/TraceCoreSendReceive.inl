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
// Trace Core
//

#ifndef __TRACECORESENDRECEIVE_INL__
#define __TRACECORESENDRECEIVE_INL__


/**
 * Gets the media writer interface
 */
inline DTraceCoreMediaIf* DTraceCoreSendReceive::GetMediaWriterIf()
    {
    return iMediaWriterInterface;
    }


/**
 * Gets the media plug-in interface
 */
inline DTraceCorePluginIf* DTraceCoreSendReceive::GetPluginIf()
    {
    return iPluginInterface;
    }


#endif

// End of File
