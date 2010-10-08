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

#ifndef TE_TRACECORE_DATAWRAPPER_H_
#define TE_TRACECORE_DATAWRAPPER_H_

#include <test/datawrapper.h>
#include <test/TraceOnActivationIf.h>
#include <TraceCoreOstDriverIf.h> //TraceCoreOstLdd
#include <TcDriverIf.h> //TcLdd

const TInt KNoOfSlots = 5;

class CTraceCoreDataWrapper : public CDataWrapper
    {
public:
    static CTraceCoreDataWrapper* NewLC();
    static CTraceCoreDataWrapper* NewL();
    ~CTraceCoreDataWrapper();
    
    TAny* GetObject();
    TBool DoCommandL(const TTEFFunction& aCommand,
                     const TTEFSectionName& aSection, 
                     const TInt aAsyncErrorIndex);
    
    TBool GetStringFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TPtrC& aResult);
    TBool GetBoolFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TBool& aResult);
    TBool GetIntFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult);
    TBool GetHexFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult);
    
    TBool GetPathFromConfig(const TTEFSectionName& aSection, const TDesC& aPathKey, TFileName& aPath);
    TBool GetArgsFromConfig(const TTEFSectionName& aSection, TDes& aArgs);
    TBool GetLongStringFromConfig(const TTEFSectionName& aSection, const TDesC& aKeyName, TDes& aResult);

protected:
    CTraceCoreDataWrapper();
    void ConstructL();

    // Command functions
    void DoCmdCopyFileL(const TTEFSectionName& aSection);
    void DoCmdDeleteFileFromAllDrivesL(const TTEFSectionName& aSection);
    void DoCmdDeleteFileL(const TTEFSectionName& aSection);
    void DoCmdDeleteEmptyFolder(const TTEFSectionName& aSection);
    void DoCmdFileExists(const TTEFSectionName& aSection);
    void DoCmdCheckFileSize(const TTEFSectionName& aSection);
    void DoCmdRunProcess(const TTEFSectionName& aSection);
    void DoCmdCreateSubstDrive(const TTEFSectionName& aSection);
    void DoCmdUnSubstDrive(const TTEFSectionName& aSection);
    void DoCmdCheckMaxFileSizeL(const TTEFSectionName& aSection);
    void DoCmdCheckFileIsAppendedL(const TTEFSectionName& aSection);
    void DoCmdVerifyTraceDataL(const TTEFSectionName& aSection);
    void DoCmdVerifyPrimingDataL(const TTEFSectionName& aSection);
    void DoCmdVerifyDroppedTracesL(const TTEFSectionName& aSection);
    void DoCmdVerifySlot(const TTEFSectionName& aSection);
    void DoCmdFileChangeNotification(const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
    void DoCmdRegisterNotifReceiver(const TTEFSectionName& aSection);
    void DoCmdUnregisterNotifReceiver(const TTEFSectionName& aSection);
    void DoCmdInstrumentTraceSequence(const TTEFSectionName& aSection);
    void DoCmdSetUpTcLdd(const TTEFSectionName& aSection);
    void DoCmdCleanUpTcLdd(const TTEFSectionName& aSection);
    void DoCmdSetUpTcOstLdd(const TTEFSectionName& aSection);
    void DoCmdCleanUpTcOstLdd(const TTEFSectionName& aSection);
    void DoCmdSetUpTraceOnActivation(const TTEFSectionName& aSection);
    void DoCmdCleanUpTraceOnActivation(const TTEFSectionName& aSection);
    void DoCmdActivateDeactivateTrace(const TTEFSectionName& aSection);
    void DoCmdReadTracesFromBuffer();
    
protected:
    // Helper methods
    void CopyFileL(const TDesC& aSrcFile, const TDesC& aDstFile);
    void DeleteFileFromAllDrivesL(const TDesC& aFile);
    void DeleteFileL(const TDesC& aFile, TInt aStrictMode = ETrue);
    void DeleteEmptyFolder(const TDesC& aFolder);
    TBool FileExists(const TDesC& aFile);
    TInt CheckFileSize(const TDesC& aFile, TInt aMinSize, TInt aMaxSize);
    void RunProcess(const TDesC& aProgramName, const TDesC& aCmdLineArgs, TInt aNoOfCalls = 1, TInt aAllowedErrorCode = KErrNone);
    void CreateSubstDrive(const TDesC& aFolderPath, TDriveNumber aDrive);
    void UnSubstDrive(TDriveNumber aDrive);
    TBool DriveNumberFromString(const TDesC& aDriveString, TDriveNumber& aDriveNumber);
    void VerifyPrimingDataL(const TDesC& aFile, TInt aGroupId, TBool aVerifyData, TInt& aNumTraces, TInt& aNumPrimingTraces);
    void VerifyDroppedPrintfTracesL(const TDesC& aFile, const TDesC& aFindString, TInt& aLastNumberFound, TInt& aNumDroppedTraces);
    void VerifyDroppedBinaryTracesL(const TDesC& aFile, TInt aGroupId, TInt aComponentId, TInt& aLastNumberFound, TInt& aNumDroppedTraces);
    void StoreInSlot(TInt aEntry, TInt aSlot);
    TInt ReadFromSlot(TInt aSlot);
    TInt GetSlotValue(const TTEFSectionName& aSection, const TDesC& aValueKey, const TDesC& aSlotNumberKey);
    void FileHasPrintfStringL(const TDesC& aFile, const TDesC& aFindString, TInt& aNumFound);
    void BufferHasPrintfStringL(TDesC8& aBuffer, const TDesC& aFindString, TInt& aNumFound);
    void InstrumentBinaryTracePoint(TInt aGroupId, TInt aComponentId, TInt aData, TInt aTraceId = 1);
    void InstrumentBinaryTraceSequence(TInt aSequenceLength, TInt aGroupId, TInt aComponentId);
    void InstrumentPrintfTraceSequence(TInt aSequenceLength, const TDesC& aString);
    TInt LoadLdd(const TDesC& aLddName, TBool aSingleUse);
    TInt FreeLdd(const TDesC& aLddName);
    TBool LoadLdd(const TDesC& aLddName, const TTEFSectionName& aSection);
    template <class T> TInt OpenLdd(const TDesC& aLddName, T& aLdd);
    template <class T> TBool OpenLdd(const TDesC& aLddName, T& aLdd, const TTEFSectionName& aSection);

    //  MActiveCallback implementation
    void RunL(CActive* aActive, TInt aIndex);
    void DoCancel(CActive* aActive, TInt aIndex);

protected:
    // Member variables
    RFs                             iFs;
    RTimer                          iTimer;
    TFixedArray<TInt, KNoOfSlots>   iSlots;
    CActiveCallback*                iActiveFileChangeNotification;
    CActiveCallback*                iActiveFileChangeNotificationTimeout;
    TPtrC                           iCurrentSectionName;
    RTraceOnActivation              iTraceOnActivationLdd;
    RTcDriver                       iTcLdd;
    RTraceCoreOstDriver             iTraceCoreOstLdd;
    RBuf8                           iTraceBuffer;
    TBool                           iTcLddLoadedAtStart;
    TBool                           iTcOstLddLoadedAtStart;
    TBool                           iTraceOnActivationLoadedAtStart;
    };


#endif /* TE_TRACECORE_DATAWRAPPER_H_ */
