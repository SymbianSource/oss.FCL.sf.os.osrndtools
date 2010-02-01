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

#ifndef MEMSPYCOMMANDS_H
#define MEMSPYCOMMANDS_H


// Literal constants
_LIT( KMemSpyCmdHeapDump, "CmdHeap_Dump" );
_LIT( KMemSpyCmdHeapDumpKernel, "CmdHeap_Dump_Kernel" );
_LIT( KMemSpyCmdHeapCompact, "CmdHeap_Compact" );
_LIT( KMemSpyCmdSWMTForceUpdate, "CmdSWMT_ForceUpdate" );
_LIT( KMemSpyCmdSWMTReset, "CmdSWMT_Reset" );
_LIT( KMemSpyCmdOpenFile, "CmdOpenFile" );
_LIT( KMemSpyCmdContainer, "CmdContainer" );
_LIT( KMemSpyCmdBitmapsSave, "CmdBitmaps_Save" );
_LIT( KMemSpyCmdRamDisableAknIconCache, "CmdRAM_DisableAknIconCache" );
_LIT( KMemSpyCmdOutputToFile, "CmdOutput_ToFile" );
_LIT( KMemSpyCmdOutputToTrace, "CmdOutput_ToTrace" );


_LIT( KMemSpyCmdUiSendToBackground, "CmdUI_Background" );
_LIT( KMemSpyCmdUiBringToForeground, "CmdUI_Foreground" );

_LIT( KMemSpyCmdUiExit, "CmdUI_Exit" );

_LIT( KMemSpyCmdSWMTTypeHeap,                    "HEAP" );
_LIT( KMemSpyCmdSWMTTypeChunk,                   "CHNK" );
_LIT( KMemSpyCmdSWMTTypeCode,                    "CODE" );
_LIT( KMemSpyCmdSWMTTypeStack,                   "STAK" );
_LIT( KMemSpyCmdSWMTTypeGlobalData,              "GLOD" );
_LIT( KMemSpyCmdSWMTTypeRamDrive,                "RAMD" );
_LIT( KMemSpyCmdSWMTTypeOpenFile,                "FILE" );
_LIT( KMemSpyCmdSWMTTypeDiskSpace,               "DISK" );
_LIT( KMemSpyCmdSWMTTypeHandleGeneric,           "HGEN" );
_LIT( KMemSpyCmdSWMTTypeFbserv,                  "FABS" );
_LIT( KMemSpyCmdSWMTTypeFileServerCache,         "F32C" );
_LIT( KMemSpyCmdSWMTTypeSystemMemory,            "SYSM" );
_LIT( KMemSpyCmdSWMTTypeWindowGroup,             "WNDG" );
_LIT( KMemSpyCmdSWMTTypeHeapFilter,              "HEAPFilter:" );

#endif
