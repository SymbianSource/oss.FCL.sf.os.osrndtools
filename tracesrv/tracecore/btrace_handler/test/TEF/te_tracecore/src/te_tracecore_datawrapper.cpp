// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// 
//
#include <bautils.h> 
#include <test/tracedataparser.h>

#include "te_tracecore_datawrapper.h"
#include "te_processcreator.h"

_LIT(KConstantsSectionName,             "Constants");

_LIT(KCmdCopyFile,                      "CopyFile");
_LIT(KCmdDeleteFile,                    "DeleteFile");
_LIT(KCmdDeleteFileFromAllDrives,       "DeleteFileFromAllDrives");
_LIT(KCmdDeleteEmptyFolder,             "DeleteEmptyFolder");
_LIT(KCmdFileExists,                    "FileExists");
_LIT(KCmdRunProcess,                    "RunProcess");
_LIT(KCmdCheckFileSize,                 "CheckFileSize");
_LIT(KCmdCreateSubstDrive,              "CreateSubstDrive");
_LIT(KCmdUnSubstDrive,                  "UnSubstDrive");
_LIT(KCmdCheckMaxFileSize,              "CheckMaxFileSize");
_LIT(KCmdCheckFileIsAppended,           "CheckFileIsAppended");
_LIT(KCmdVerifyTraceData,               "VerifyTraceData");
_LIT(KCmdVerifyPrimingData,             "VerifyPrimingData");
_LIT(KCmdVerifyDroppedTraces,           "VerifyDroppedTraces");
_LIT(KCmdVerifySlot,                    "VerifySlot");
_LIT(KCmdFileChangeNotification,        "FileChangeNotification");
_LIT(KCmdSetUpTraceOnActivation,        "SetUpTraceOnActivation");
_LIT(KCmdRegisterNotifReceiver,         "RegisterNotifReceiver");
_LIT(KCmdUnregisterNotifReceiver,       "UnregisterNotifReceiver");
_LIT(KCmdCleanUpTraceOnActivation,      "CleanUpTraceOnActivation");
_LIT(KCmdInstrumentTraceSequence,       "InstrumentTraceSequence");
_LIT(KCmdSetUpTcLdd,                    "SetUpTcLdd");
_LIT(KCmdCleanUpTcLdd,                  "CleanUpTcLdd");
_LIT(KCmdSetUpTcOstLdd,                 "SetUpTcOstLdd");
_LIT(KCmdCleanUpTcOstLdd,               "CleanUpTcOstLdd");
_LIT(KCmdActivateDeactivateTrace,       "ActivateDeactivateTrace");
_LIT(KCmdReadTracesFromBuffer,          "ReadTracesFromBuffer");

_LIT(KSrcFileKey,                       "src_file");
_LIT(KDstFileKey,                       "dst_file");
_LIT(KFileKey,                          "file");
_LIT(KExeNameKey,                       "exe_name");
_LIT(KArgsKey,                          "args");
_LIT(KNoOfCallsKey,                     "calls");
_LIT(KNoOfAllowedTimeoutsKey,           "allowed_timeouts");
_LIT(KFolderPathKey,                    "path");
_LIT(KDriveKey,                         "drive");
_LIT(KSizeKey,                          "size");
_LIT(KMinSizeKey,                       "min_size");
_LIT(KMaxSizeKey,                       "max_size");
_LIT(KSystemDriveKey,                   "system_drive");
_LIT(KUnfetteredKey,                    "unfettered");
_LIT(KDurationKey,                      "duration");
_LIT(KMinIncreaseKey,                   "min_increase");
_LIT(KGroupIdKey,                       "group_id");
_LIT(KComponentIdKey,                   "component_id");
_LIT(KSlotNumberKey,                    "slot");
_LIT(KTracesSlotNumberKey,              "traces_slot");
_LIT(KPrimingTracesSlotNumberKey,       "priming_traces_slot");
_LIT(KSlotANumberKey,                   "slot_a");
_LIT(KSlotBNumberKey,                   "slot_b");
_LIT(KValueAKey,                        "value_a");
_LIT(KValueBKey,                        "value_b");
_LIT(KDifferenceKey,                    "difference");
_LIT(KMinDifferenceKey,                 "min_difference");
_LIT(KMaxDifferenceKey,                 "max_difference");
_LIT(KVerifyData,                       "verify_data");
_LIT(KFindStringKey,                    "find_string");
_LIT(KExpectedKey,                      "expected");
_LIT(KLengthKey,                        "length");
_LIT(KStringKey,                        "string");
_LIT(KActivateKey,                      "activate");
_LIT(KBufferSizeKey,                    "buffer_size");
_LIT(KAllowErrorKey,                    "allow_error");
_LIT(KSingleUseKey,                     "single_use");
_LIT(KOpenLddKey,                       "open_ldd");
_LIT(KCloseLddKey,                      "close_ldd");

_LIT(KSpace,                            " ");
_LIT(KInt,                              "%d");

_LIT(KTcLdd,                            "TcLdd");
_LIT(KTraceCoreOstLdd,                  "TraceCoreOstLdd");
_LIT(KTraceOnActivationLdd,             "d_traceonactivation.ldd");

const TInt KSecondsToMicroSeconds       = 1000000;
const TInt KDefaultBufferSize           = 1024;
const TInt KLongStringLength            = 512;


CTraceCoreDataWrapper::CTraceCoreDataWrapper()
:   iActiveFileChangeNotification(NULL)
,   iActiveFileChangeNotificationTimeout(NULL)
,   iTcLddLoadedAtStart(EFalse)
,   iTcOstLddLoadedAtStart(EFalse)
,   iTraceOnActivationLoadedAtStart(EFalse)
    {  
    }

CTraceCoreDataWrapper::~CTraceCoreDataWrapper()
    {  
    delete iActiveFileChangeNotification;
    delete iActiveFileChangeNotificationTimeout;
    iFs.Close();
    iTimer.Close();
    iTraceBuffer.Close();
    iTcLdd.Close();
    iTraceCoreOstLdd.Close();
    iTraceOnActivationLdd.Close();
    }

CTraceCoreDataWrapper* CTraceCoreDataWrapper::NewLC()
    {
    CTraceCoreDataWrapper* self = new (ELeave)CTraceCoreDataWrapper();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CTraceCoreDataWrapper* CTraceCoreDataWrapper::NewL()
    {
    CTraceCoreDataWrapper* self=CTraceCoreDataWrapper::NewLC();
    CleanupStack::Pop(self); 
    return self;
    }

void CTraceCoreDataWrapper::ConstructL()
    {
    User::LeaveIfError(iFs.Connect());
    User::LeaveIfError(iTimer.CreateLocal());
    iActiveFileChangeNotification = CActiveCallback::NewL(*this);
    iActiveFileChangeNotificationTimeout = CActiveCallback::NewL(*this);
    }

TAny* CTraceCoreDataWrapper::GetObject()
    {
    return this;
    }

TBool CTraceCoreDataWrapper::GetStringFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TPtrC& aResult)
    {
    TBool found = CDataWrapper::GetStringFromConfig(aSectName, aKeyName, aResult);    
    if (found)
        {
        TPtrC constantName(aResult);
        CDataWrapper::GetStringFromConfig(KConstantsSectionName(), constantName, aResult);
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetBoolFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TBool& aResult)
    {
    TBool found = CDataWrapper::GetIntFromConfig(aSectName, aKeyName, aResult);
    if (!found)
        {
        TPtrC constantName;
        found = CDataWrapper::GetStringFromConfig(aSectName, aKeyName, constantName);
        if (found)
            {
            found = CDataWrapper::GetBoolFromConfig(KConstantsSectionName(), constantName, aResult);
            if (!found)
                {
                found = CDataWrapper::GetBoolFromConfig(aSectName, aKeyName, aResult);
                }
            }
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetIntFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult)
    {
    TBool found = CDataWrapper::GetIntFromConfig(aSectName, aKeyName, aResult);
    if (!found)
        {
        TPtrC constantName;
        found = CDataWrapper::GetStringFromConfig(aSectName, aKeyName, constantName);
        if (found)
            {
            found = CDataWrapper::GetIntFromConfig(KConstantsSectionName(), constantName, aResult);
            }
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetHexFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult)
    {
    TBool found = CDataWrapper::GetIntFromConfig(aSectName, aKeyName, aResult);
    if (!found)
        {
        TPtrC constantName;
        found = CDataWrapper::GetStringFromConfig(aSectName, aKeyName, constantName);
        if (found)
            {
            found = CDataWrapper::GetHexFromConfig(KConstantsSectionName(), constantName, aResult);
            if (!found)
                {
                found = CDataWrapper::GetHexFromConfig(aSectName, aKeyName, aResult);
                }
            }
        }
    else
        {
        found = CDataWrapper::GetHexFromConfig(aSectName, aKeyName, aResult);
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetPathFromConfig(const TTEFSectionName& aSection, const TDesC& aPathKey, TFileName& aPath)
    {
    TBool found = EFalse;
    TPtrC file;
    if (GetStringFromConfig(aSection, aPathKey, file))
        {
        found = ETrue;
        TBool useSysDrive = EFalse;
        GetBoolFromConfig(aSection, KSystemDriveKey(), useSysDrive);
        TPtrC driveString;
        if (GetStringFromConfig(aSection, KDriveKey(), driveString))
            {
            aPath.Zero();
            aPath.Append(driveString);
            aPath.Append(_L(":"));
            aPath.Append(file);
            }
        else if (useSysDrive)
            {
            aPath.Zero();
            aPath.Append(RFs::GetSystemDriveChar());
            aPath.Append(_L(":"));
            aPath.Append(file);
            }
        else
            {
            aPath.Copy(file);
            }
        }
    else
        {
        ERR_PRINTF2(_L("No %S in INI file"), &aPathKey);
        SetBlockResult(EFail);
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetArgsFromConfig(const TTEFSectionName& aSection, TDes& aArgs)
    {
    TPtrC cmdLineArgs(KNullDesC);
    TBool found = GetStringFromConfig(aSection, KArgsKey(), cmdLineArgs);
    aArgs.Copy(cmdLineArgs);
    if (!found)
        {
        // Args may be seperated
        TInt argNumber = 0;
        TBuf<KTEFMaxNameLength> argKeyName;
        found = ETrue;
        
        while (found)
            {
            argNumber++;
            argKeyName.Copy(KArgsKey());
            argKeyName.AppendFormat(KInt(), argNumber);
            found = GetStringFromConfig(aSection, argKeyName, cmdLineArgs);
            if (found)
                {
                if (argNumber > 1)
                    {
                    aArgs.Append(KSpace());
                    }
                aArgs.Append(cmdLineArgs);
                }
            }
        
        if (argNumber > 1)
            {
            found = ETrue;
            }
        }
    if (found)
        {
        INFO_PRINTF2(_L("Found args \"%S\" in INI file"), &aArgs);
        }
    return found;
    }

TBool CTraceCoreDataWrapper::GetLongStringFromConfig(const TTEFSectionName& aSection, const TDesC& aKeyName, TDes& aResult)
    {
    TPtrC stringLine;
    TBool found = GetStringFromConfig(aSection, aKeyName, stringLine);
    if (found)
        {
        aResult.Copy(stringLine);
        }
    else
        {
        TInt lineNumber = 0;
        TBuf<KTEFMaxNameLength> keyName;
        found = ETrue;
        aResult.Zero();
        
        while (found)
            {
            lineNumber++;
            keyName.Copy(aKeyName);
            keyName.AppendFormat(KInt(), lineNumber);
            found = GetStringFromConfig(aSection, keyName, stringLine);
            if (found)
                {
                aResult.Append(stringLine);
                }
            }
        
        if (lineNumber > 1)
            {
            found = ETrue;
            }
        }
    return found;
    }

TBool CTraceCoreDataWrapper::DoCommandL(const TTEFFunction& aCommand,
                                             const TTEFSectionName& aSection,
                                             const TInt aAsyncErrorIndex)
    {
    TBool ret = ETrue;
    if (aCommand == KCmdCopyFile())
        {
        DoCmdCopyFileL(aSection);
        }
    else if (aCommand == KCmdDeleteFile())
        {
        DoCmdDeleteFileL(aSection);
        }
    else if (aCommand == KCmdDeleteFileFromAllDrives())
        {
        DoCmdDeleteFileFromAllDrivesL(aSection);
        }
    else if (aCommand == KCmdDeleteEmptyFolder())
        {
        DoCmdDeleteEmptyFolder(aSection);
        }
    else if (aCommand == KCmdFileExists())
        {
        DoCmdFileExists(aSection);
        }
    else if (aCommand == KCmdCheckFileSize())
        {
        DoCmdCheckFileSize(aSection);
        }
    else if (aCommand == KCmdRunProcess())
        {
        DoCmdRunProcess(aSection);
        }
    else if (aCommand == KCmdCreateSubstDrive())
        {
        DoCmdCreateSubstDrive(aSection);
        }
    else if (aCommand == KCmdUnSubstDrive())
        {
        DoCmdUnSubstDrive(aSection);
        }
    else if (aCommand == KCmdCheckMaxFileSize())
        {
        DoCmdCheckMaxFileSizeL(aSection);
        }
    else if (aCommand == KCmdCheckFileIsAppended())
        {
        DoCmdCheckFileIsAppendedL(aSection);
        }
    else if (aCommand == KCmdVerifyTraceData())
        {
        DoCmdVerifyTraceDataL(aSection);
        }
    else if (aCommand == KCmdVerifyPrimingData())
        {
        DoCmdVerifyPrimingDataL(aSection);
        }
    else if (aCommand == KCmdVerifyDroppedTraces())
        {
        DoCmdVerifyDroppedTracesL(aSection);
        }
    else if (aCommand == KCmdVerifySlot())
        {
        DoCmdVerifySlot(aSection);
        }
    else if (aCommand == KCmdFileChangeNotification())
        {
        DoCmdFileChangeNotification(aSection, aAsyncErrorIndex);
        }
    else if (aCommand == KCmdSetUpTraceOnActivation())
        {
        DoCmdSetUpTraceOnActivation(aSection);
        }
    else if (aCommand == KCmdRegisterNotifReceiver())
        {
        DoCmdRegisterNotifReceiver(aSection);
        }
    else if (aCommand == KCmdUnregisterNotifReceiver())
        {
        DoCmdUnregisterNotifReceiver(aSection);
        }
    else if (aCommand == KCmdCleanUpTraceOnActivation())
        {
        DoCmdCleanUpTraceOnActivation(aSection);
        }
    else if (aCommand == KCmdInstrumentTraceSequence())
        {
        DoCmdInstrumentTraceSequence(aSection);
        }
    else if (aCommand == KCmdSetUpTcLdd())
        {
        DoCmdSetUpTcLdd(aSection);
        }
    else if (aCommand == KCmdCleanUpTcLdd())
        {
        DoCmdCleanUpTcLdd(aSection);
        }
    else if (aCommand == KCmdSetUpTcOstLdd())
        {
        DoCmdSetUpTcOstLdd(aSection);
        }
    else if (aCommand == KCmdCleanUpTcOstLdd())
        {
        DoCmdCleanUpTcOstLdd(aSection);
        }
    else if (aCommand == KCmdActivateDeactivateTrace())
        {
        DoCmdActivateDeactivateTrace(aSection);
        }
    else if (aCommand == KCmdReadTracesFromBuffer())
        {
        DoCmdReadTracesFromBuffer();
        }
    else
        {
        ret = EFalse;
        }
    return ret;
    }

void CTraceCoreDataWrapper::CopyFileL(const TDesC& aSrcFile,
                                            const TDesC& aDstFile)
    {
    INFO_PRINTF3(_L("CopyFile aSrcFile (%S) aDestFile (%S)"), &aSrcFile, &aDstFile);

    TInt err = KErrNone;
    
    // attempt to create the folder if it does not exist
    if (!BaflUtils::FolderExists(iFs, aDstFile))
        {
        err = iFs.MkDirAll(aDstFile);
        if (err != KErrNone)
            {
            ERR_PRINTF3(_L("CopyFile MKDIRALL FAILED aDestFile (%S) err (%d)"), &aDstFile, err);
            SetError(err);
            }
        }
    
    CFileMan* fileMan = CFileMan::NewL(iFs);

    // Make file read-write
    err = fileMan->Attribs(aDstFile, 0, KEntryAttReadOnly, TTime(0), CFileMan::ERecurse);
    if (err != KErrNone && err != KErrNotFound)
        {
        ERR_PRINTF3(_L("CopyFile ATTRIBS FAILED aDestFile (%S) err (%d)"), &aDstFile, err);
        SetError(err);
        }
    
    err = fileMan->Copy(aSrcFile, aDstFile);
    if (err != KErrNone)
        {
        ERR_PRINTF4(_L("CopyFile COPY FAILED aSrcFile (%S) aDestFile (%S) err (%d)"), &aSrcFile, &aDstFile, err);
        SetError(err);
        }
    else
        {
        INFO_PRINTF3(_L("CopyFile copied file OK aSrcFile (%S) aDestFile (%S)"), &aSrcFile, &aDstFile);
        }
    
    delete fileMan;
    }

void CTraceCoreDataWrapper::DeleteFileFromAllDrivesL(const TDesC& aFile)
    {
    INFO_PRINTF2(_L("DeleteFileFromAllDrives aFile (%S)"), &aFile);

    TDriveList driveList;
    TInt err =  iFs.DriveList(driveList);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("DeleteFileFromAllDrives DRIVELIST FAILED err (%d)"), err);
        SetError(err);
        }
    else
        {
        CFileMan* fileMan = CFileMan::NewL(iFs);
        TChar drives[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
        for (TInt i=0; i<driveList.Size(); i++)
            {
            if (driveList[i])
                {
                TFileName file;
                file.Zero();
                file.Append(drives[i]);
                file.Append(_L(":"));
                file.Append(aFile);
                
                DeleteFileL(file, EFalse);
                }
            }
        delete fileMan;
        }
    }

void CTraceCoreDataWrapper::DeleteFileL(const TDesC& aFile, TInt aStrictMode)
    {
    if (aStrictMode)
        {
        INFO_PRINTF2(_L("DeleteFile aFile (%S)"), &aFile);
        }
    
    CFileMan* fileMan = CFileMan::NewL(iFs);

    // Make file read-write
    TInt err = fileMan->Attribs(aFile, 0, KEntryAttReadOnly, TTime(0), CFileMan::ERecurse);
    if (err == KErrNone)
        {
        err = fileMan->Delete(aFile);
        if (err == KErrNone)
            {
            INFO_PRINTF2(_L("DeleteFile deleted file OK file (%S)"), &aFile);
            }
        else
            {
            ERR_PRINTF3(_L("DeleteFile DELETE FAILED file (%S) err (%d)"), &aFile, err);
            SetError(err);
            }
        }                
    else if (err == KErrAccessDenied && !aStrictMode)
        {
        WARN_PRINTF2(_L("DeleteFile file (%S) READONLY"), &aFile);
        }
    else if (err == KErrNotReady && !aStrictMode)
        {
        WARN_PRINTF2(_L("DeleteFile file (%S) NOT READY"), &aFile);
        }
    else if (err != KErrNotFound && err != KErrPathNotFound)
        {
        ERR_PRINTF3(_L("DeleteFile ATTRIBS error (%d) file (%S)"), err, &aFile);
        SetError(err);
        }    
    else if (aStrictMode)
        {
        INFO_PRINTF2(_L("DeleteFile aFile (%S) DOES NOT EXIST"), &aFile);
        }
    
    delete fileMan;
    }

/**
 * Deletes an empty folder. If folder is not empty the current test is failed with KErrInUse
 */
void CTraceCoreDataWrapper::DeleteEmptyFolder(const TDesC& aFolder)
    {
    INFO_PRINTF2(_L("DeleteEmptyFolder aFolder (%S)"), &aFolder);
    
    if (BaflUtils::PathExists(iFs, aFolder))
        {
        TInt err = iFs.SetAtt(aFolder, 0, KEntryAttReadOnly);
        if (err != KErrNone)
            {
            ERR_PRINTF3(_L("DeleteEmptyFolder SETATT FAILED aFolder (%S) err (%d)"), &aFolder, err);
            SetError(err);
            }
        else
            {
            err = iFs.RmDir(aFolder);
            if (err != KErrNone)
                {
                ERR_PRINTF3(_L("DeleteEmptyFolder DELETE FAILED aFolder (%S) err (%d)"), &aFolder, err);
                SetError(err);
                }
            else
                {
                INFO_PRINTF2(_L("DeleteEmptyFolder deleted folder OK aFolder (%S)"), &aFolder);
                }        
            }
        }
    else
        {
        INFO_PRINTF2(_L("DeleteEmptyFolder aFolder DOES NOT EXIST (%S)"), &aFolder);
        }
    }

TBool CTraceCoreDataWrapper::FileExists(const TDesC& aFile)
    {
    INFO_PRINTF2(_L("FileExists aFile (%S)"), &aFile);
    TBool fileExists = EFalse;
    
    if (BaflUtils::FileExists(iFs, aFile))
        {
        INFO_PRINTF2(_L("FileExists file exists OK aFile (%S)"), &aFile);
        fileExists = ETrue;
        }
    else
        {
        ERR_PRINTF2(_L("FileExists aFile DOES NOT EXIST (%S)"), &aFile);
        SetError(KErrNotFound);
        }
    
    return fileExists;
    }

/**
 * Checks that a given file has size within given limits
 * 
 * @return Size of file on success otherwise symbian error code 
 *
 */
TInt CTraceCoreDataWrapper::CheckFileSize(const TDesC& aFile, TInt aMinSize, TInt aMaxSize)
    {
    INFO_PRINTF2(_L("CheckFileSize aFile (%S)"), &aFile);
    TInt result = KErrNotFound;
    
    if (FileExists(aFile))
        {
        TEntry entry;
        result = iFs.Entry(aFile, entry);
        if (result != KErrNone)
            {
            ERR_PRINTF3(_L("CheckFileSize ENTRY FAILED aFile (%S) err (%d)"), &aFile, result);
            }
        else
            {
            TInt fileSize = entry.iSize;
            INFO_PRINTF3(_L("CheckFileSize aFile (%S) fileSize (%d)"), &aFile, fileSize);    
            if ( (aMinSize == -1) && (aMaxSize == -1) )
                {
                ERR_PRINTF2(_L("CheckFileSize failed - need to specify max and / or min"), result);
                result = KErrArgument;
                }
            else if ( (aMinSize == -1 || fileSize >= aMinSize) &&
                      (aMaxSize == -1 || fileSize <= aMaxSize) )
                {
                INFO_PRINTF3(_L("CheckFileSize file size OK aFile (%S) fileSize (%d)"), &aFile, fileSize);
                result = fileSize;
                }
            else
                {
                ERR_PRINTF3(_L("CheckFileSize FAILED aFile (%S) fileSize (%d)"), &aFile, fileSize);
                result = KErrGeneral;
                }
            }
        }
    
    if (result < 0)
        {
        SetError(result);
        }
    
    return result;
    }

void CTraceCoreDataWrapper::RunProcess(const TDesC& aProgramName, const TDesC& aCmdLineArgs, TInt aNoOfCalls, TInt aAllowedErrorCode)
    {
    for (TInt i=0; i<aNoOfCalls; i++)
        {
        TProcessCreatorResults res = CProcessCreator::StartProgram(aProgramName, aCmdLineArgs);
        if (res.iCode == KErrNone || res.iCode == aAllowedErrorCode)
            {
            INFO_PRINTF7(_L("\"%S %S\" executes with code: %d, exitType: %d, desc: \"%S\" on call number %d"), &aProgramName, &aCmdLineArgs, res.iCode, res.iExitType, &(res.iDesc), i+1);
            }
        else
            {
            ERR_PRINTF7(_L("\"%S %S\" executes with code: %d, exitType: %d, desc: \"%S\" on call number %d"), &aProgramName, &aCmdLineArgs, res.iCode, res.iExitType, &(res.iDesc), i+1);
            SetError(res.iCode);
            }
        }
    }

void CTraceCoreDataWrapper::CreateSubstDrive(const TDesC& aFolderPath, TDriveNumber aDrive)
    {
    TInt err = KErrNone;
    
    // attempt to create the folder if it does not exist
    if (!BaflUtils::FolderExists(iFs, aFolderPath))
        {
        err = iFs.MkDirAll(aFolderPath);
        if (err != KErrNone)
            {
            ERR_PRINTF3(_L("CreateSubstDrive MKDIRALL FAILED aFile (%S) err (%d)"), &aFolderPath, err);
            SetError(err);
            }
        }
    
    if (err == KErrNone)
        {
        err = iFs.SetSubst(aFolderPath, aDrive);
        if (err != KErrNone)
            {
            ERR_PRINTF4(_L("CreateSubstDrive SETSUBST FAILED aFolderPath (%S) aDrive (%d) err (%d)"), &aFolderPath, aDrive, err);
            SetError(err);
            }
        else
            {
            INFO_PRINTF3(_L("CreateSubstDrive substed drive OK aFolderPath (%S) aDrive (%d)"), &aFolderPath, aDrive);
            }    
        }
    }

void CTraceCoreDataWrapper::UnSubstDrive(TDriveNumber aDrive)
    {
    TInt err = iFs.SetSubst(KNullDesC, aDrive);
    if (err != KErrNone)
        {
        ERR_PRINTF3(_L("UnSubstDrive SETSUBST FAILED aDrive (%d) err (%d)"), aDrive, err);
        SetError(err);
        }
    else
        {
        INFO_PRINTF2(_L("UnSubstDrive unsubsted drive OK aDrive (%d)"), aDrive);
        }    
    }


// Macro for matching drive letter
#define DRIVE_CONDITION(aDrv) if (aDriveString == _L(#aDrv)) { aDriveNumber = EDrive##aDrv; found = ETrue; }

TBool CTraceCoreDataWrapper::DriveNumberFromString(const TDesC& aDriveString, TDriveNumber& aDriveNumber)
    {
    TBool found = EFalse;
    
    DRIVE_CONDITION(A)
    DRIVE_CONDITION(B)
    DRIVE_CONDITION(C)
    DRIVE_CONDITION(D)
    DRIVE_CONDITION(E)
    DRIVE_CONDITION(F)
    DRIVE_CONDITION(G)
    DRIVE_CONDITION(H)
    DRIVE_CONDITION(I)
    DRIVE_CONDITION(J)
    DRIVE_CONDITION(K)
    DRIVE_CONDITION(L)
    DRIVE_CONDITION(M)
    DRIVE_CONDITION(N)
    DRIVE_CONDITION(O)
    DRIVE_CONDITION(P)
    DRIVE_CONDITION(Q)
    DRIVE_CONDITION(R)
    DRIVE_CONDITION(S)
    DRIVE_CONDITION(T)
    DRIVE_CONDITION(U)
    DRIVE_CONDITION(V)
    DRIVE_CONDITION(W)
    DRIVE_CONDITION(X)
    DRIVE_CONDITION(Y)
    DRIVE_CONDITION(Z)
    
    return found;
    }

void CTraceCoreDataWrapper::DoCmdCopyFileL(const TTEFSectionName& aSection)
    {
    TPtrC srcFile;
    if (GetStringFromConfig(aSection, KSrcFileKey(), srcFile))
        {
        TFileName dstFile;
        if (GetPathFromConfig(aSection, KDstFileKey(), dstFile))
            {
            CopyFileL(srcFile, dstFile);
            }
        }
    else
        {
        ERR_PRINTF2(_L("No %S in INI file"), &KSrcFileKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdDeleteFileFromAllDrivesL(const TTEFSectionName& aSection)
    {
    TFileName file;
    if (GetPathFromConfig(aSection, KFileKey(), file))
        {
        DeleteFileFromAllDrivesL(file);
        }
    }

void CTraceCoreDataWrapper::DoCmdDeleteFileL(const TTEFSectionName& aSection)
    {
    TFileName file;
    if (GetPathFromConfig(aSection, KFileKey(), file))
        {
        DeleteFileL(file);
        }
    }

void CTraceCoreDataWrapper::DoCmdDeleteEmptyFolder(const TTEFSectionName& aSection)
    {
    TFileName folder;
    if (GetPathFromConfig(aSection, KFolderPathKey(), folder))
        {
        DeleteEmptyFolder(folder);
        }
    }

void CTraceCoreDataWrapper::DoCmdFileExists(const TTEFSectionName& aSection)
    {
    TFileName file;
    if (GetPathFromConfig(aSection, KFileKey(), file))
        {
        FileExists(file);
        }
    }

void CTraceCoreDataWrapper::DoCmdCheckFileSize(const TTEFSectionName& aSection)
    {
    TFileName file;
    if (GetPathFromConfig(aSection, KFileKey(), file))
        {
        TInt minSize = -1;
        TInt maxSize = -1;
        TInt size = -1;
        if (GetIntFromConfig(aSection, KSizeKey(), size))
            {
            minSize = size;
            maxSize = size;
            }
        else
            {
            GetIntFromConfig(aSection, KMinSizeKey(), minSize);
            GetIntFromConfig(aSection, KMaxSizeKey(), maxSize);
            }
        TInt fileSize = CheckFileSize(file, minSize, maxSize);
        
        // Store number in slot for future use, if slot number is given
        TInt slotNumber;
        if (GetIntFromConfig(aSection, KSlotNumberKey(), slotNumber))
            {
            StoreInSlot(fileSize, slotNumber);
            }
        }
    }

void CTraceCoreDataWrapper::DoCmdRunProcess(const TTEFSectionName& aSection)
    {
    TPtrC programName;
    if (GetStringFromConfig(aSection, KExeNameKey(), programName))
        {
        TBuf<KTEFMaxNameLength> cmdLineArgs;
        GetArgsFromConfig(aSection, cmdLineArgs);
        TInt noOfCalls = 1;
        GetIntFromConfig(aSection, KNoOfCallsKey(), noOfCalls);
        TInt allowedErrorCode = KErrNone;
        GetIntFromConfig(aSection, KAllowErrorKey(), allowedErrorCode);
        RunProcess(programName, cmdLineArgs, noOfCalls, allowedErrorCode);
        }
    else
        {
        ERR_PRINTF2(_L("No %S in INI file"), &KExeNameKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdCreateSubstDrive(const TTEFSectionName& aSection)
    {
    TPtrC folderPath;
    if (GetStringFromConfig(aSection, KFolderPathKey(), folderPath))
        {
        TPtrC driveString;
        if (GetStringFromConfig(aSection, KDriveKey(), driveString))
            {
            TDriveNumber driveNumber;
            if (DriveNumberFromString(driveString, driveNumber))
                {
                CreateSubstDrive(folderPath, driveNumber);
                }
            else
                {
                ERR_PRINTF2(_L("Could not parse drive %S in INI file"), &driveString);
                SetBlockResult(EFail);
                }
            }
        else
            {
            ERR_PRINTF2(_L("No %S in INI file"), &KDriveKey());
            SetBlockResult(EFail);
            }
        }
    else
        {
        ERR_PRINTF2(_L("No %S in INI file"), &KFolderPathKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdUnSubstDrive(const TTEFSectionName& aSection)
    {
    TPtrC driveString;
    if (GetStringFromConfig(aSection, KDriveKey(), driveString))
        {
        TDriveNumber driveNumber;
        if (DriveNumberFromString(driveString, driveNumber))
            {
            UnSubstDrive(driveNumber);
            }
        else
            {
            ERR_PRINTF2(_L("Could not parse drive %S in INI file"), &driveString);
            SetBlockResult(EFail);
            }
        }
    else
        {
        ERR_PRINTF2(_L("No %S in INI file"), &KDriveKey());
        SetBlockResult(EFail);
        }
    }

/**
 * Test command to verify the max file size of output file
 * 
 * Tracing needs to be started for this command to complete
 *
 */
void CTraceCoreDataWrapper::DoCmdCheckMaxFileSizeL(const TTEFSectionName& aSection)
    {
    TFileName   file;
    TInt        maxSize;
    TPtrC       programName;
    // Get all required parameters from INI file
    if ( GetPathFromConfig(aSection, KFileKey(), file) &&
         GetIntFromConfig(aSection, KMaxSizeKey(), maxSize) && 
         GetStringFromConfig(aSection, KExeNameKey(), programName) )
        {        
        // Get optional params from INI file
        TBool unfetteredSize = EFalse;
        GetBoolFromConfig(aSection, KUnfetteredKey(), unfetteredSize);
        TBuf<KTEFMaxNameLength> cmdLineArgs;
        GetArgsFromConfig(aSection, cmdLineArgs);
        TInt noOfCalls = 1;
        GetIntFromConfig(aSection, KNoOfCallsKey(), noOfCalls);
        TInt minSize = 0;
        if (!unfetteredSize)
            {
            GetIntFromConfig(aSection, KMinSizeKey(), minSize);
            }
        
        // Number of seconds to perform test for (for unfettered case)
        TBool useTimer = EFalse;
        RTimer timer;
        TRequestStatus timerStatus(KRequestPending);
        TInt duration;
        if (unfetteredSize && GetIntFromConfig(aSection, KDurationKey(), duration))
            {
            TInt err = timer.CreateLocal();
            CleanupClosePushL(timer);
            if (err == KErrNone)
                {
                useTimer = ETrue;
                INFO_PRINTF2(_L("Running timer for %d seconds"), duration);
                timer.After(timerStatus, duration * KSecondsToMicroSeconds);
                }
            else
                {
                ERR_PRINTF2(_L("Error in creating timer %d"), err);
                SetBlockResult(EFail);
                }
            }

        // Keep calling an EXE so that trace data is appended to output file
        // Tracing must be started and the EXE must generate trace data for the config file used
        TBool finished = EFalse;
        TInt lastFileSize = 0;
        TInt currentFileSize = 0;
        while (!finished)
            {
            // Call the EXE
            RunProcess(programName, cmdLineArgs, noOfCalls);
            if (unfetteredSize)
                {
                // Get the size of output file
                currentFileSize = CheckFileSize(file, 0, -1);
                }
            else
                {
                // Check the size of output file is smaller than max file size
                currentFileSize = CheckFileSize(file, 0, maxSize);
                }
            if (currentFileSize < 0)
                {
                // An error has occurred in checking the file size. Stop and fail the test 
                ERR_PRINTF2(_L("Error in file size %d"), currentFileSize);
                SetBlockResult(EFail);
                finished = ETrue;
                }
            else if (currentFileSize < lastFileSize)
                {
                // Output file size has decreased rather than increased. Stop and fail the test
                ERR_PRINTF2(_L("File size (%d) has decreased rather than increased"), currentFileSize);
                SetBlockResult(EFail);
                finished = ETrue;
                }
            else if (unfetteredSize)
                {
                if (currentFileSize > maxSize)
                    {
                    // File size got to required size. Test has passed.
                    INFO_PRINTF1(_L("File size checking completed successfully"));
                    finished = ETrue;
                    }
                else if (currentFileSize == lastFileSize)
                    {
                    // File size has not increased. Print a warning and continue
                    WARN_PRINTF2(_L("File size (%d) has not increased"), currentFileSize);
                    }
                }
            else if (currentFileSize == lastFileSize)
                {
                if (currentFileSize < minSize)
                    {
                    // File size has not reached required minimum. Stop and fail the test
                    ERR_PRINTF3(_L("File size (%d) has not met the minimum required (%d)"), currentFileSize, minSize);
                    SetBlockResult(EFail);
                    }
                else
                    {
                    // File size got to minimum required and has stopped increasing, staying below required max size. Test has passed.
                    INFO_PRINTF1(_L("File size checking completed successfully"));
                    }
                finished = ETrue;
                }
            lastFileSize = currentFileSize;
            if (useTimer && timerStatus == KErrNone)
                {
                // File size has been increasing for required number of seconds. Test has passed.
                INFO_PRINTF2(_L("File size checking completed successfully after %d seconds"), duration);
                finished = ETrue;
                }
            }
        if (useTimer)
            {
            timer.Cancel();
            CleanupStack::PopAndDestroy(&timer);
            }
        }
    else
        {
        ERR_PRINTF3(_L("Need %S and %S in INI file"), &KMaxSizeKey(), &KExeNameKey());
        SetBlockResult(EFail);
        }
    }

/**
 * Test command to verify the output file is appended to and not overwritten
 * 
 * Tracing needs to be started for this command to complete and
 * an output file containing some data must exist
 *
 */
void CTraceCoreDataWrapper::DoCmdCheckFileIsAppendedL(const TTEFSectionName& aSection)
    {
    TFileName   file;
    TPtrC       programName;
    TInt        maxSize;
    TInt        duration;
    // Get all required parameters from INI file
    if ( GetPathFromConfig(aSection, KFileKey(), file) &&
         GetIntFromConfig(aSection, KMaxSizeKey(), maxSize) &&
         GetIntFromConfig(aSection, KDurationKey(), duration) &&
         GetStringFromConfig(aSection, KExeNameKey(), programName) )
        {
        // Get optional params from INI file
        TBuf<KTEFMaxNameLength> cmdLineArgs;
        GetArgsFromConfig(aSection, cmdLineArgs);
        TInt noOfCalls = 1;
        GetIntFromConfig(aSection, KNoOfCallsKey(), noOfCalls);
        TInt minSize = 1;
        GetIntFromConfig(aSection, KMinSizeKey(), minSize);
        TInt noOfAllowedTimeouts = 0;
        GetIntFromConfig(aSection, KNoOfAllowedTimeoutsKey(), noOfAllowedTimeouts);
        TInt minIncrease = 0;
        GetIntFromConfig(aSection, KMinIncreaseKey(), minIncrease);
        
        // Create timer for time-outs
        RTimer timer;
        TInt err = timer.CreateLocal();
        CleanupClosePushL(timer);
        if (err != KErrNone)
            {
            ERR_PRINTF2(_L("Create timer error %d"), err);
            SetBlockResult(EFail);
            }    
        
        // Keep calling an EXE so that trace data is created in output file
        // Tracing must be started and the EXE must generate trace data for the config file used
        // The output file must exist and already contain some data
        // Get current size of output file, and check it is non-zero or min provided
        TInt originalFileSize = CheckFileSize(file, minSize, -1);
        if (originalFileSize < 0)
            {
            // An error has occurred in checking the file size. Stop and fail the test 
            ERR_PRINTF2(_L("Error in file size %d"), originalFileSize);
            SetBlockResult(EFail);
            }
        else
            {
            TInt currentFileSize = 0;
            TInt noOfTimeouts = 0;
            while (currentFileSize < maxSize)
                {
                TRequestStatus notifyStatus(KRequestPending);
                TRequestStatus timerStatus(KRequestPending);
                // Start timeout timer
                timer.After(timerStatus, duration * KSecondsToMicroSeconds);
                // Request notification of file change
                iFs.NotifyChange(ENotifyAll, notifyStatus, file);
                
                // Call the EXE
                RunProcess(programName, cmdLineArgs, noOfCalls);

                // Wait for file to update or timeout
                User::WaitForRequest(notifyStatus, timerStatus);

                if (timerStatus == KErrNone)
                    {
                    // We have timed out
                    noOfTimeouts++;
                    WARN_PRINTF3(_L("Have not received notification of file change in %d seconds (%d tries remaining)"), duration, noOfAllowedTimeouts-noOfTimeouts+1);
                    }
                else
                    {
                    // Cancel timer
                    timer.Cancel();
                    }
                
                if (notifyStatus == KErrNone)
                    {
                    // We have received file change notification
                    INFO_PRINTF2(_L("Have received notification of file change (%S)"), &file);
                    }
                else
                    {
                    // Cancel file change notification
                    iFs.NotifyChangeCancel();
                    }
                
                // We get a timeout when output file is not updated and so there are dropped traces.
                // This test is only checking append functionality so we allow this for a user-defined number of times.
                if (noOfTimeouts > noOfAllowedTimeouts)
                    {
                    // We have timed out too many times, so fail the test
                    ERR_PRINTF2(_L("File has not been updated %d times"), noOfTimeouts);
                    SetBlockResult(EFail);
                    break;
                    }
                
                // Check the size of output file is expected size
                currentFileSize = CheckFileSize(file, originalFileSize + minIncrease, -1);
                if (currentFileSize < 0)
                    {
                    // An error has occurred in checking the file size. Stop and fail the test 
                    ERR_PRINTF2(_L("Error in file size %d"), currentFileSize);
                    SetBlockResult(EFail);
                    break;
                    }
                originalFileSize = currentFileSize;
                }             
            }
        CleanupStack::PopAndDestroy(&timer); // close timer
        }
    else
        {
        ERR_PRINTF4(_L("Need %S, %S and %S in INI file"), &KMaxSizeKey(), &KExeNameKey(), &KDurationKey());
        SetBlockResult(EFail);
        }
    }

/**
 * Test command to verify the trace buffer contains the expected number of given printf traces
 */
void CTraceCoreDataWrapper::DoCmdVerifyTraceDataL(const TTEFSectionName& aSection)
    {
    TBuf<KLongStringLength> findString;
    // Get all required parameters from INI file
    if (GetLongStringFromConfig(aSection, KFindStringKey(), findString))
        {
        TInt expectedNumFound = 1;
        GetIntFromConfig(aSection, KExpectedKey(), expectedNumFound);
        TInt numFound = 0;
        BufferHasPrintfStringL(iTraceBuffer, findString, numFound);
        if (numFound != expectedNumFound)
            {
            ERR_PRINTF3(_L("Expected printf %d times, found it %d times"), expectedNumFound, numFound);
            if (numFound == 0)
                {
                SetError(KErrNotFound);
                }
            else
                {
                SetError(KErrGeneral);
                }
            }
        }
    else
        {
        ERR_PRINTF2(_L("Need %S in INI file"), &KFindStringKey());
        SetBlockResult(EFail);
        }
    }

/**
 * Test command to verify the output file contains the expected number of traces / priming traces
 * and optionally validates that the priming data contains the correct values
 * 
 * This function gets the number of traces / priming traces and stores them in the required slots
 * so the values can be verified later
 */
void CTraceCoreDataWrapper::DoCmdVerifyPrimingDataL(const TTEFSectionName& aSection)
    {
    TFileName   filePath;
    TInt        groupId;
    // Get all required parameters from INI file
    if ( GetPathFromConfig(aSection, KFileKey(), filePath) &&
         GetIntFromConfig(aSection, KGroupIdKey(), groupId) )
        {
        // Get optional params from INI file
        TBool verifyData = EFalse;
        GetBoolFromConfig(aSection, KVerifyData(), verifyData);
        
        TInt numTraces = 0;
        TInt numPrimingTraces = 0;
        VerifyPrimingDataL(filePath, groupId, verifyData, numTraces, numPrimingTraces);

        // Store numbers in slots for future use, if slot numbers are given
        TInt slotNumber;
        if (GetIntFromConfig(aSection, KTracesSlotNumberKey(), slotNumber))
            {
            StoreInSlot(numTraces, slotNumber);
            }
        if (GetIntFromConfig(aSection, KPrimingTracesSlotNumberKey(), slotNumber))
            {
            StoreInSlot(numPrimingTraces, slotNumber);
            }
        }
    else
        {
        ERR_PRINTF2(_L("Need %S in INI file"), &KGroupIdKey());
        SetBlockResult(EFail);
        }
    }

/**
 * This function gets the number of traces / priming traces from the data file given for the required GID
 * It optionally validates that the priming data contains the correct values
 */
void CTraceCoreDataWrapper::VerifyPrimingDataL(const TDesC& aFile, TInt aGroupId, TBool aVerifyData, TInt& aNumTraces, TInt& aNumPrimingTraces)
    {
    INFO_PRINTF3(_L("VerifyPrimingData aFile (%S) aGroupId (%d)"), &aFile, aGroupId);

    aNumTraces = 0;
    aNumPrimingTraces = 0;

    INFO_PRINTF3(_L("Calling TTraceDataParser::ParsePrimingDataL(%S, %d)"), &aFile, aGroupId);
    TInt err = TTraceDataParser::ParsePrimingDataL(aFile, iFs, aGroupId, aNumPrimingTraces, aNumTraces, aVerifyData);
    INFO_PRINTF3(_L("Number of traces for GID %d: %d"), aGroupId, aNumTraces);
    INFO_PRINTF3(_L("Number of priming traces for GID %d: %d"), aGroupId, aNumPrimingTraces);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("ParsePrimingDataL() error %d"), err);
        SetError(err);
        }
    }

/**
 * Test command to verify the output file contains the expected sequence of traces
 * and checks that any dropped traces are correctly handled
 */
void CTraceCoreDataWrapper::DoCmdVerifyDroppedTracesL(const TTEFSectionName& aSection)
    {
    TFileName filePath;
    // Get all required parameters from INI file
    if (GetPathFromConfig(aSection, KFileKey(), filePath))
        {
        TInt lastNumberFound = 0;
        TInt numDroppedTraces = 0;
    
        TPtrC findString;
        // Look to see if we have a string to fing
        if (GetStringFromConfig(aSection, KFindStringKey(), findString))
            {
            VerifyDroppedPrintfTracesL(filePath, findString, lastNumberFound, numDroppedTraces);
            }
        else
            {
            TInt groupId = 0;
            GetIntFromConfig(aSection, KGroupIdKey(), groupId);
            TInt componentId = 0;
            GetHexFromConfig(aSection, KComponentIdKey(), componentId);
            VerifyDroppedBinaryTracesL(filePath, groupId, componentId, lastNumberFound, numDroppedTraces);
            }
        
        TInt expectedLastNumberFound;
        if (GetIntFromConfig(aSection, KExpectedKey(), expectedLastNumberFound))
            {
            if (expectedLastNumberFound == lastNumberFound)
                {
                INFO_PRINTF2(_L("Last number found (%d) as expected"), lastNumberFound);
                }
            else
                {
                ERR_PRINTF3(_L("Last number found (%d) not as expected (%d)"), lastNumberFound, expectedLastNumberFound);
                SetBlockResult(EFail);
                }
            }
        }
    }

/**
 * This function verifies the given file contains the expected sequence of printf traces
 * and checks that any dropped traces are correctly handled
 */
void CTraceCoreDataWrapper::VerifyDroppedPrintfTracesL(const TDesC& aFile, const TDesC& aFindString, TInt& aLastNumberFound, TInt& aNumDroppedTraces)
    {
    INFO_PRINTF3(_L("VerifyDroppedPrintfTracesL aFile (%S) aFindString (%S)"), &aFile, &aFindString);

    aLastNumberFound = 0;
    aNumDroppedTraces = 0;

    TBuf8<KTEFMaxNameLength> findString;
    findString.Copy(aFindString);
    INFO_PRINTF3(_L("Calling TTraceDataParser::DataHasTraceSequenceL(%S, \"%S\")"), &aFile, &aFindString);
    TInt err = TTraceDataParser::DataHasTraceSequenceL(aFile, iFs, aLastNumberFound, aNumDroppedTraces, &findString);
    INFO_PRINTF3(_L("Last number found is %d, (%d dropped traces)"), aLastNumberFound, aNumDroppedTraces);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("DataHasTraceSequenceL() error %d"), err);
        SetError(err);
        }    
    }

/**
 * This function verifies the given file contains the expected sequence of binary traces
 * and checks that any dropped traces are correctly handled
 */
void CTraceCoreDataWrapper::VerifyDroppedBinaryTracesL(const TDesC& aFile, TInt aGroupId, TInt aComponentId, TInt& aLastNumberFound, TInt& aNumDroppedTraces)
    {
    INFO_PRINTF4(_L("VerifyDroppedBinaryTracesL aFile (%S) aGroupId (%d) aComponentId (0x%x)"), &aFile, aGroupId, aComponentId);

    aLastNumberFound = 0;
    aNumDroppedTraces = 0;

    INFO_PRINTF4(_L("Calling TTraceDataParser::DataHasTraceSequenceL(%S, %d, 0x%x)"), &aFile, aGroupId, aComponentId);
    TInt err = TTraceDataParser::DataHasTraceSequenceL(aFile, iFs, aLastNumberFound, aNumDroppedTraces, NULL, aGroupId, aComponentId);
    INFO_PRINTF3(_L("Last number found is %d, (%d dropped traces)"), aLastNumberFound, aNumDroppedTraces);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("DataHasTraceSequenceL() error %d"), err);
        SetError(err);
        }    
    }

/**
 * This function checks if printf traces in the data file given contain a given string
 */
void CTraceCoreDataWrapper::FileHasPrintfStringL(const TDesC& aFile, const TDesC& aFindString, TInt& aNumFound)
    {
    INFO_PRINTF3(_L("FileHasPrintfStringL aFile (%S) aFindString (%S)"), &aFile, &aFindString);
    aNumFound = 0;
    TBuf8<KTEFMaxNameLength> findString;
    findString.Copy(aFindString);
    INFO_PRINTF3(_L("Calling TTraceDataParser::DataHasPrintfStringL(%S, \"%S\")"), &aFile, &aFindString);
    TInt err = TTraceDataParser::DataHasPrintfStringL(aFile, iFs, findString, aNumFound);
    INFO_PRINTF2(_L("Data in file contains string %d times"), aNumFound);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("DataHasPrintfString() error %d"), err);
        SetError(err);
        }
    }

/**
 * This function checks if printf traces in the data buffer given contain a given string
 */
void CTraceCoreDataWrapper::BufferHasPrintfStringL(TDesC8& aBuffer, const TDesC& aFindString, TInt& aNumFound)
    {
    INFO_PRINTF2(_L("BufferHasPrintfStringL aFindString (%S)"), &aFindString);
    aNumFound = 0;
    TBuf8<KTEFMaxNameLength> findString;
    findString.Copy(aFindString);
    INFO_PRINTF2(_L("Calling TTraceDataParser::DataHasPrintfString(\"%S\")"), &aFindString);
    TInt err = TTraceDataParser::DataHasPrintfString(aBuffer, findString, aNumFound);
    INFO_PRINTF2(_L("Data in buffer contains string %d times"), aNumFound);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("DataHasPrintfString() error %d"), err);
        SetError(err);
        }
    }

TInt CTraceCoreDataWrapper::GetSlotValue(const TTEFSectionName& aSection, const TDesC& aValueKey, const TDesC& aSlotNumberKey)
    {
    TInt value = 0;
    TInt slot;
    if (GetIntFromConfig(aSection, aValueKey, value))
        {
        // Value given directly in INI file, so don't need slot number
        if (GetIntFromConfig(aSection, aSlotNumberKey, slot))
            {
            ERR_PRINTF3(_L("Cannot have %S and %S in INI file"), &aValueKey, &aSlotNumberKey);
            SetBlockResult(EFail);
            }
        }
    else if (GetIntFromConfig(aSection, aSlotNumberKey, slot))
        {
        // Slot number given in INI file, so get value from it
        value = ReadFromSlot(slot);
        }
    else
        {
        ERR_PRINTF3(_L("Need %S or %S in INI file"), &aValueKey, &aSlotNumberKey);
        SetBlockResult(EFail);
        }
    return value;
    }

void CTraceCoreDataWrapper::DoCmdVerifySlot(const TTEFSectionName& aSection)
    {
    // Get values to compare
    TInt valueA = GetSlotValue(aSection, KValueAKey(), KSlotANumberKey());
    TInt valueB = GetSlotValue(aSection, KValueBKey(), KSlotBNumberKey());

    // Compare the values
    INFO_PRINTF3(_L("Comparing value A (%d) with value B (%d)"), valueA, valueB);

    TInt difference = 0;
    TInt minDifference = -1;
    TInt maxDifference = -1;
    if (GetIntFromConfig(aSection, KDifferenceKey(), difference))
        {
        if (difference != 0)
            {
            minDifference = difference;
            maxDifference = difference;
            }
        }
    else
        {
        GetIntFromConfig(aSection, KMinDifferenceKey(), minDifference);
        GetIntFromConfig(aSection, KMaxDifferenceKey(), maxDifference);
        }
    if ( (minDifference == -1) && (maxDifference == -1) && (valueA != valueB) )
        {
        ERR_PRINTF1(_L("Values are not the same"));
        SetBlockResult(EFail);
        }
    else if ( (minDifference != -1 && valueB - valueA < minDifference) ||
              (maxDifference != -1 && valueB - valueA > maxDifference) )
        {
        ERR_PRINTF1(_L("Difference in values not in given range"));
        SetBlockResult(EFail);
        }
    else
        {
        INFO_PRINTF1(_L("Slot value successfully verified"));
        }
    }

void CTraceCoreDataWrapper::StoreInSlot(TInt aEntry, TInt aSlot)
    {
    if (aSlot >= 1 && aSlot <= iSlots.Count())
        {
        iSlots[aSlot-1] = aEntry;
        INFO_PRINTF3(_L("Storing number %d in slot %d"), aEntry, aSlot);
        }
    else
        {
        ERR_PRINTF3(_L("Cannot store number in slot %d - not in range [1,%d]"), aSlot, iSlots.Count());
        SetBlockResult(EFail);
        }
    }

TInt CTraceCoreDataWrapper::ReadFromSlot(TInt aSlot)
    {
    TInt entry = 0;
    if (aSlot >= 1 && aSlot <= iSlots.Count())
        {
        entry = iSlots[aSlot-1];
        INFO_PRINTF3(_L("Reading number %d from slot %d"), entry, aSlot);
        }
    else
        {
        ERR_PRINTF3(_L("Cannot read number from slot %d - not in range [1,%d]"), aSlot, iSlots.Count());
        SetBlockResult(EFail);
        }
    return entry;
    }

void CTraceCoreDataWrapper::DoCmdFileChangeNotification(const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
    {
    TFileName   file;
    TInt        duration;
    // Get all required parameters from INI file
    if ( GetPathFromConfig(aSection, KFileKey(), file) &&
         GetIntFromConfig(aSection, KDurationKey(), duration) )
        {
        INFO_PRINTF2(_L("Requesting file change notification for file %S"), &file);
        // Request notification of file change
        iFs.NotifyChange(ENotifyAll, iActiveFileChangeNotification->iStatus, file);
        iActiveFileChangeNotification->Activate(aAsyncErrorIndex);
        IncOutstanding();
        // Start timeout timer
        iTimer.After(iActiveFileChangeNotificationTimeout->iStatus, duration * KSecondsToMicroSeconds);
        iActiveFileChangeNotificationTimeout->Activate(aAsyncErrorIndex);
        IncOutstanding();
        iCurrentSectionName.Set(aSection);
        }
    else
        {
        ERR_PRINTF2(_L("Need %S in INI file"), &KDurationKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::RunL(CActive* aActive, TInt aIndex)
    {
    if (aActive == iActiveFileChangeNotification)
        {
        TInt err = iActiveFileChangeNotification->iStatus.Int();
        if (err == KErrNone)
            {
            INFO_PRINTF1(_L("Received file change notification"));
            }
        else
            {
            WARN_PRINTF2(_L("File change notification error %d"), err);
            }
        
        TBool fileReady = ETrue;
        if (err != KErrNone)
            {
            fileReady = EFalse;
            }
        TPtrC findString;
        // Look to see if we have a string to find
        if (GetStringFromConfig(iCurrentSectionName, KFindStringKey(), findString))
            {
            fileReady = EFalse;
            TFileName file;
            if (GetPathFromConfig(iCurrentSectionName, KFileKey(), file))
                {
                // The file is ready if it contains the string given in the INI file
                TInt expectedNumFound = 1;
                GetIntFromConfig(iCurrentSectionName, KExpectedKey(), expectedNumFound);
                TInt numFound = 0;
                FileHasPrintfStringL(file, findString, numFound);
                if (numFound == expectedNumFound)
                    {
                    fileReady = ETrue;
                    }
                }
            if (!fileReady && err == KErrNone)
                {
                // File isn't ready, so request notification again
                // Keep doing this until file is ready or it times out
                INFO_PRINTF2(_L("Requesting file change notification for file %S"), &file);
                iFs.NotifyChange(ENotifyAll, iActiveFileChangeNotification->iStatus, file);
                iActiveFileChangeNotification->Activate(aIndex);
                IncOutstanding();
                }
            }
        if (fileReady)
            {
            // Cancel file change notification timeout
            iTimer.Cancel();
            }
        else if (err != KErrNone)
            {
            ERR_PRINTF2(_L("File change notification error %d"), err);
            SetAsyncError(aIndex, err);
            }
        DecOutstanding();
        }
    else if (aActive == iActiveFileChangeNotificationTimeout)
        {
        TInt err = iActiveFileChangeNotificationTimeout->iStatus.Int();
        if (err == KErrNone)
            {
            INFO_PRINTF1(_L("File change notification has timed out"));
            // Cancel file change notification
            iFs.NotifyChangeCancel();
            }
        else if (err != KErrCancel) 
            {
            ERR_PRINTF2(_L("File change notification timeout error %d"), err);
            SetAsyncError(aIndex, err);
            }
        DecOutstanding();
        }
    else
        {
        ERR_PRINTF1(_L("Stray signal"));
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCancel(CActive* aActive, TInt /*aIndex*/)
    {
    if (aActive == iActiveFileChangeNotification)
        {
        INFO_PRINTF1(_L("File change notification cancelled"));
        }
    else if (aActive == iActiveFileChangeNotificationTimeout)
        {
        INFO_PRINTF1(_L("File change notification timeout cancelled"));
        }
    else
        {
        ERR_PRINTF1(_L("Stray signal"));
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdRegisterNotifReceiver(const TTEFSectionName& aSection)
    {
    TInt    groupId;
    TInt    componentId;
    // Get all required parameters from INI file
    if ( GetIntFromConfig(aSection, KGroupIdKey(), groupId) &&
         GetHexFromConfig(aSection, KComponentIdKey(), componentId) )
        {
        TTraceOnActivationParams traceOnActivationParams;
        traceOnActivationParams.iGroupId = groupId;
        traceOnActivationParams.iComponentId = componentId;
        
        TInt err = iTraceOnActivationLdd.RegisterNotificationReceiver(traceOnActivationParams);
        if (err != KErrNone)
            {
            ERR_PRINTF4(_L("RegisterNotificationReceiver(%d,0x%x) error %d"), groupId, componentId, err);
            SetError(err);
            }
        else
            {
            INFO_PRINTF3(_L("RegisterNotificationReceiver(%d,0x%x) returns no error"), groupId, componentId);
            }
        }
    else
        {
        ERR_PRINTF3(_L("Need %S and %S in INI file"), &KGroupIdKey(), &KComponentIdKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdUnregisterNotifReceiver(const TTEFSectionName& aSection)
    {
    TInt    groupId;
    TInt    componentId;
    // Get all required parameters from INI file
    if ( GetIntFromConfig(aSection, KGroupIdKey(), groupId) &&
         GetHexFromConfig(aSection, KComponentIdKey(), componentId) )
        {
        TTraceOnActivationParams traceOnActivationParams;
        traceOnActivationParams.iGroupId = groupId;
        traceOnActivationParams.iComponentId = componentId;
    
        TInt err = iTraceOnActivationLdd.UnregisterNotificationReceiver(traceOnActivationParams);
        if (err != KErrNone)
            {
            ERR_PRINTF4(_L("UnregisterNotificationReceiver(%d,0x%x) error %d"), groupId, componentId, err);
            SetError(err);
            }
        else
            {
            INFO_PRINTF3(_L("UnregisterNotificationReceiver(%d,0x%x) returns no error"), groupId, componentId);
            }
        }
    else
        {
        ERR_PRINTF3(_L("Need %S and %S in INI file"), &KGroupIdKey(), &KComponentIdKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::InstrumentBinaryTracePoint(TInt aGroupId, TInt aComponentId, TInt aData, TInt aTraceId)
    {
    TComponentId KOstTraceComponentID = aComponentId;
    // Calculate trace word from Group ID
    TUint32 traceWord = (aGroupId << GROUPIDSHIFT) | aTraceId;
    OstTrace1(aGroupId, traceWord, "You will only see this text in Trace Viewer: %d", aData);    
    }

void CTraceCoreDataWrapper::InstrumentBinaryTraceSequence(TInt aSequenceLength, TInt aGroupId, TInt aComponentId)
    {
    for (TInt i=1; i<=aSequenceLength; i++)
        {
        InstrumentBinaryTracePoint(aGroupId, aComponentId, i, i);
        }    
    }

void CTraceCoreDataWrapper::InstrumentPrintfTraceSequence(TInt aSequenceLength, const TDesC& aString)
    {
    INFO_PRINTF3(_L("InstrumentPrintfTraceSequence(%d, \"%S\""), aSequenceLength, &aString);
    TBuf8<KTEFMaxNameLength> givenString;
    givenString.Copy(aString);
    for (TInt i=1; i<=aSequenceLength; i++)
        {
        TBuf8<KTEFMaxNameLength> formattedString;
        formattedString.Format(givenString, i);
        RDebug::Printf("%S", &formattedString);
        }    
    }

void CTraceCoreDataWrapper::DoCmdInstrumentTraceSequence(const TTEFSectionName& aSection)
    {
    TInt sequenceLength;
    // Get all required parameters from INI file
    if (GetIntFromConfig(aSection, KLengthKey(), sequenceLength))
        {
        TPtrC printfString;
        if (GetStringFromConfig(aSection, KStringKey(), printfString))
            {
            InstrumentPrintfTraceSequence(sequenceLength, printfString);
            }
        else
            {
            TInt groupId = 0;
            GetIntFromConfig(aSection, KGroupIdKey(), groupId);
            TInt componentId = 0;
            GetHexFromConfig(aSection, KComponentIdKey(), componentId);
            InstrumentBinaryTraceSequence(sequenceLength, groupId, componentId);
            }
        }
    else
        {
        ERR_PRINTF2(_L("Need %S in INI file"), &KLengthKey());
        SetBlockResult(EFail);
        }
    }

TInt CTraceCoreDataWrapper::LoadLdd(const TDesC& aLddName, TBool aSingleUse)
    {  
    TInt err = User::LoadLogicalDevice(aLddName);
    if (err == KErrNone)
        {
        INFO_PRINTF2(_L("LDD %S successfully loaded with no error"), &aLddName);
        }
    else if (err == KErrAlreadyExists && !aSingleUse)
        {
        INFO_PRINTF2(_L("LDD %S was already loaded"), &aLddName);
        }
    else
        {
        ERR_PRINTF3(_L("LoadLogicalDevice(%S) error %d"), &aLddName, err);
        SetError(err);
        }
    return err;
    }

TBool CTraceCoreDataWrapper::LoadLdd(const TDesC& aLddName, const TTEFSectionName& aSection)
    {
    TBool singleUse = EFalse;
    GetBoolFromConfig(aSection, KSingleUseKey(), singleUse);
    
    TBool isLoadedAlready = EFalse;    
    TInt err = LoadLdd(aLddName, singleUse);
    if (!singleUse)
        {
        isLoadedAlready = (err == KErrAlreadyExists);
        }    
    
    return isLoadedAlready;
    }

TInt CTraceCoreDataWrapper::FreeLdd(const TDesC& aLddName)
    {  
    TInt err = User::FreeLogicalDevice(aLddName);
    if (err == KErrNone)
        {
        INFO_PRINTF2(_L("LDD %S successfully freed with no error"), &aLddName);
        }
    else
        {
        ERR_PRINTF3(_L("FreeLogicalDevice(%S) error %d"), &aLddName, err);
        //Won't set error from this until
        //issue with always returning KErrInUse
        //User::FreeLogicalDevice() is resolved
        //SetError(err);
        }
    return err;
    }

template <class T>
TInt CTraceCoreDataWrapper::OpenLdd(const TDesC& aLddName, T& aLdd)
    {  
    TInt err = aLdd.Open();
    if (err != KErrNone)
        {
        ERR_PRINTF3(_L("%S Open() error %d"), &aLddName, err);
        SetError(err);
        }
    else
        {
        INFO_PRINTF2(_L("%S successfully set up"), &aLddName);
        }
    return err;
    }

template <class T>
TBool CTraceCoreDataWrapper::OpenLdd(const TDesC& aLddName, T& aLdd, const TTEFSectionName& aSection)
    {  
    TBool openLdd = ETrue;
    GetBoolFromConfig(aSection, KOpenLddKey(), openLdd);
    
    if (openLdd)
        {
        OpenLdd(aLddName, aLdd);
        }

    return openLdd;
    }

void CTraceCoreDataWrapper::DoCmdSetUpTcLdd(const TTEFSectionName& aSection)
    {
    iTcLddLoadedAtStart = LoadLdd(KTcLdd(), aSection);
    
    OpenLdd(KTcLdd(), iTcLdd, aSection);
    }

void CTraceCoreDataWrapper::DoCmdCleanUpTcLdd(const TTEFSectionName& aSection)
    {
    TBool closeLdd = ETrue;
    GetBoolFromConfig(aSection, KCloseLddKey(), closeLdd);
    
    if (closeLdd)
        {
        iTcLdd.Close();
        }

    if (!iTcLddLoadedAtStart)
        {
        FreeLdd(KTcLdd());
        }
    }

void CTraceCoreDataWrapper::DoCmdSetUpTcOstLdd(const TTEFSectionName& aSection)
    {
    iTcOstLddLoadedAtStart = LoadLdd(KTraceCoreOstLdd(), aSection);
    
    if (OpenLdd(KTraceCoreOstLdd(), iTraceCoreOstLdd, aSection))
        {
        //Switch to OstMemoryWriter
        TUint32 writer(1);
        iTraceCoreOstLdd.ChangeWriter(writer);
        
        TUint32 handle;
        TInt err = iTraceCoreOstLdd.GetChunkHandle(handle);
        if (err != KErrNone)
            {
            ERR_PRINTF2(_L("GetChunkHandle() error %d"), err);
            SetError(err);
            }
    
        TInt bufferSize = KDefaultBufferSize;
        GetIntFromConfig(aSection, KBufferSizeKey(), bufferSize);
        INFO_PRINTF2(_L("Calling SetBufferSize(%d)"), bufferSize);
        err = iTraceCoreOstLdd.SetBufferSize((TUint32&) bufferSize);
        if (err != KErrNone)
            {
            ERR_PRINTF2(_L("SetBufferSize() error %d"), err);
            SetError(err);
            }
        }
    }

void CTraceCoreDataWrapper::DoCmdCleanUpTcOstLdd(const TTEFSectionName& aSection)
    {
    TBool closeLdd = ETrue;
    GetBoolFromConfig(aSection, KCloseLddKey(), closeLdd);
    
    if (closeLdd)
        {
        iTraceBuffer.Close();
        //Switch back to XtiWriter
        TUint32 writer(0);
        iTraceCoreOstLdd.ChangeWriter(writer);    
        iTraceCoreOstLdd.Close();
        }

    if (!iTcOstLddLoadedAtStart)
        {
        FreeLdd(KTraceCoreOstLdd());
        }
    }

void CTraceCoreDataWrapper::DoCmdSetUpTraceOnActivation(const TTEFSectionName& aSection)
    {
    iTraceOnActivationLoadedAtStart = LoadLdd(KTraceOnActivationLdd(), aSection);
    
    OpenLdd(KTraceOnActivationLdd(), iTraceOnActivationLdd, aSection);
    }

void CTraceCoreDataWrapper::DoCmdCleanUpTraceOnActivation(const TTEFSectionName& aSection)
    {    
    TBool closeLdd = ETrue;
    GetBoolFromConfig(aSection, KCloseLddKey(), closeLdd);
    
    if (closeLdd)
        {
        iTraceOnActivationLdd.Close();
        }

    if (!iTraceOnActivationLoadedAtStart)
        {
        FreeLdd(KTraceOnActivationLdd());
        }
    }

void CTraceCoreDataWrapper::DoCmdActivateDeactivateTrace(const TTEFSectionName& aSection)
    {
    TInt groupId;
    TInt componentId;
    TBool activate;
    // Get all required parameters from INI file
    if ( GetIntFromConfig(aSection, KGroupIdKey(), groupId) &&
         GetHexFromConfig(aSection, KComponentIdKey(), componentId) &&
         GetBoolFromConfig(aSection, KActivateKey(), activate) )
        {
        RTcDriverParameters tcDriverParameters;
        tcDriverParameters.iGroupId = groupId;
        tcDriverParameters.iComponentId = componentId;
        tcDriverParameters.iActivation = EFalse;
        TInt err = KErrNone;
        if (activate)
            {
            INFO_PRINTF3(_L("Calling ActivateTrace(%d,0x%x)"), groupId, componentId);
            err = iTcLdd.ActivateTrace(tcDriverParameters);
            if (err != KErrNone)
                {
                ERR_PRINTF4(_L("ActivateTrace(%d,0x%x) error %d"), groupId, componentId, err);
                SetError(err);
                }
            }
        else
            {
            INFO_PRINTF3(_L("Calling DeactivateTrace(%d,0x%x)"), groupId, componentId);
            err = iTcLdd.DeactivateTrace(tcDriverParameters);
            if (err != KErrNone)
                {
                ERR_PRINTF4(_L("DeactivateTrace(%d,0x%x) error %d"), groupId, componentId, err);
                SetError(err);
                }
            }
        }
    else
        {
        ERR_PRINTF4(_L("Need %S, %S and %S in INI file"), &KGroupIdKey(), &KComponentIdKey(), &KActivateKey());
        SetBlockResult(EFail);
        }
    }

void CTraceCoreDataWrapper::DoCmdReadTracesFromBuffer()
    {
    iTraceBuffer.Close();
    TInt bufferSize = iTraceCoreOstLdd.GetBufferSize();
    
    TInt err = iTraceBuffer.ReAlloc(bufferSize);
    if (err != KErrNone)
        {
        ERR_PRINTF3(_L("ReAlloc(%d) error %d"), bufferSize, err);
        SetError(err);
        }

    INFO_PRINTF2(_L("Calling ReadNTraces() with buffer size %d"), bufferSize);
    err = iTraceCoreOstLdd.ReadNTraces(iTraceBuffer);
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("ReadNTraces() error %d"), err);
        SetError(err);
        }
    else
        {
        INFO_PRINTF2(_L("%d bytes of trace data read"), iTraceBuffer.Size());
        // Print the trace data to the log, if there's not too much of it
        if (iTraceBuffer.Size() <= KDefaultBufferSize && iTraceBuffer.Size() > 0)
            {
            TBuf<KDefaultBufferSize> tempBuffer;
            tempBuffer.Copy(iTraceBuffer);
            INFO_PRINTF2(_L("Trace data:\r\n%S"), &tempBuffer);
            }
        }

    err = iTraceCoreOstLdd.ClearBuffer();
    if (err != KErrNone)
        {
        ERR_PRINTF2(_L("ClearBuffer() error %d"), err);
        SetError(err);
        }
    }
