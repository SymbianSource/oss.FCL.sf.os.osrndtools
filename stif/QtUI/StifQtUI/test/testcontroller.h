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
* Description: QT C++ based Class.
*              Unit Test to StfQtUI's controller and model.
*
*/
#ifndef TESTCONTROLLER_H_
#define TESTCONTROLLER_H_
#include <QtTest/QtTest>
#include "istfqtuicontroller.h"
#include "istfqtuimodel.h"


class testcontroller: public QObject, public IStfEventListener, public IStifModelEventListener
    {
        Q_OBJECT
 private slots:
     /* operation:
      *     GetModuleList();
      * check:
      *     module's number.
      * */
     void T_GetModuleList_ModuleNumber();
     /* operation:
      *     GetModuleList();
      * check:
      *     module's name. 
      */
     void T_GetModuleList_ModuleName();
     /*operation:
      *     OpenEngineIniFile();
      *     GetModuleList();
      * check:
      *     module's number.
      * 
      * */
     void T_OpenEngineIniFile_ModuleNumber();
     
     /*operation:
      *     OpenEngineIniFile();
      * check:
      *     return value;
      * */
     void T_OpenEngineIniFile();
     
     /*operation:
      *     GetCaseListByModule(); with currect moduleName
      * check:
      *     case List number.
      * */
     void T_GetCaseListByModule();
     
     /*operation:
      *     GetCase();
      *check:
      *     return value of the case.
      * */
     void T_GetCase();
     
     void T_GetSetList();
     
     void T_GetCaseListBySet();
     
     void T_AddCaseToSet();
     
     void T_CreateSet();
     
     void T_DeleteSet();
     
     void T_ShowOutput();
     
     void T_RunCase();
     
     void T_RunSet();
     
     void T_PauseCase_ResumeCase();
     
     void T_AbortCase();
     
     void T_Model_ClearCasesStatus();
     
     void T_Model_AddCaseByStatus_GetCasesByStatus();
     
     void T_Model_AddRunningCase_RemoveRunningCase();
     
     
     
public:
    testcontroller();
    virtual ~testcontroller();
    
public: //Implement IStfEventListener
    void OnGetMessage(const QString& aMessage);
    void OnSetListChanged();
    void OnCaseOutputChanged(const IStfEventListener::CaseOutputCommand& cmd, const QString& index, const QString& msg);

    
public: //Implement IStifModelEventListener
    void OnCaseStatisticChanged() ;
    void OnRunningCaseChanged() ;
    
    
private:
    bool OnSetListChangedFired;    
    bool OnCaseStatisticChangedFired;
    bool OnRunningCaseChangedFired;
    
private:
    IStfQtUIController* controller;
    IStfQtUIModel* model;
    
    
    };

#endif /* TESTCONTROLLER_H_ */
