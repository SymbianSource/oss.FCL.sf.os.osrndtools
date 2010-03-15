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
#include "testcontroller.h"
#include "stfqtuimodel.h"
#include "stfqtuicontroller.h"
#include "cstfcase.h"
#include "cstfmodule.h"
#include <QProcess>

const QString KConfigFile1 = "c:\\STFQTUI_Test\\testframework1.ini";
const QString KConfigFile2 = "c:\\STFQTUI_Test\\testframework2.ini";
const QString KDefaultModuleName = "demomodule";
const QString KDefaultSetName = "stfqtuitesting.set";

testcontroller::testcontroller() 
    : OnSetListChangedFired(false),
            OnCaseStatisticChangedFired(false),
            OnRunningCaseChangedFired(false)
    {
    // TODO Auto-generated constructor stub
    model = new StfQtUIModel();
    model->AddStifModelEventListener(this);
    controller = new StfQtUIController(model);
    controller->AddStfEventListener(this);
    controller->OpenEngineIniFile(KConfigFile1);
    }

testcontroller::~testcontroller()
    {
    // TODO Auto-generated destructor stub
    delete controller;
    delete model;
    }


void testcontroller::T_GetModuleList_ModuleNumber()
    {
    controller->OpenEngineIniFile(KConfigFile1);
    int moduleSize = controller->GetModuleList().size();
    QCOMPARE(moduleSize, 1);
    }

void testcontroller::T_GetModuleList_ModuleName()
    {
    controller->OpenEngineIniFile(KConfigFile1);
    QList<QString> moduleList = controller->GetModuleList();
    bool find = false;
    foreach(QString m, moduleList)
        {
        if(m.toLower() == KDefaultModuleName)
            {
            find = true;
            break;
            }
        }
    QCOMPARE(find, true);
    }

void testcontroller::T_OpenEngineIniFile_ModuleNumber()
    {
    controller->OpenEngineIniFile(KConfigFile2);    
    int moduleSize = controller->GetModuleList().size();
    QCOMPARE(moduleSize, 2);
    }



void testcontroller::T_OpenEngineIniFile()
    {
    bool rst;
    rst = controller->OpenEngineIniFile(KConfigFile2);
    QCOMPARE(rst, true);
    rst = controller->OpenEngineIniFile(KConfigFile1);    
    QCOMPARE(rst, true);
    rst = controller->OpenEngineIniFile(KConfigFile2);
    QCOMPARE(rst, true);
    rst = controller->OpenEngineIniFile("z:\\abc.ini");
    //QCOMPARE(rst, false);
    rst = controller->OpenEngineIniFile(KConfigFile1);    
    QCOMPARE(rst, true);                
    }

void testcontroller::T_GetCaseListByModule()
    {
    controller->OpenEngineIniFile(KConfigFile1); 
    QList<QString> list = controller->GetCaseListByModule(KDefaultModuleName);
    QCOMPARE(list.size(), 6);
    list = controller->GetCaseListByModule("null");
    QCOMPARE(list.size(), 0);
    
    }
void testcontroller::T_GetCase()
    {
    CSTFCase theCase = controller->GetCase(KDefaultModuleName, 1);
    QCOMPARE(theCase.Index(), 1);
    theCase = controller->GetCase(KDefaultModuleName, 21);    
    QCOMPARE(theCase.Index(), -1);
    QCOMPARE(theCase.Name(), QString(""));
    theCase = controller->GetCase("null", 0);
    QCOMPARE(theCase.Name(), QString(""));
    }

void testcontroller::T_GetSetList()
    {
    OnSetListChangedFired = false;
    QList<QString> list = controller->GetSetList();
    bool find = false;
    foreach(QString set, list)
        {
        if(set == KDefaultSetName)
            {
            find = true;
            break;
            }
        }
    QCOMPARE(find, true);
    QCOMPARE(OnSetListChangedFired, false);//ensure SetListChanged does not be fired.
    }

void testcontroller::T_GetCaseListBySet()
    {
    OnSetListChangedFired = false;
    QList<QString> list = controller->GetCaseListBySet(KDefaultSetName);
    QCOMPARE(list.size(),5);
    list = controller->GetCaseListBySet("null");
    QCOMPARE(list.size(),0);
    QCOMPARE(OnSetListChangedFired, false);//ensure SetListChanged does not be fired.
    }

void testcontroller::T_AddCaseToSet()
    {
    OnSetListChangedFired = false;
    QList<QString> list = controller->GetSetList();
    int before_count = list.count();
    QList<CSTFCase> cases;
    QList<QString> caselist = controller->GetCaseListByModule(KDefaultModuleName);
    for(int i=0;i<caselist.size();i++)
        {
        CSTFCase theCase = controller->GetCase(KDefaultModuleName, i);
        cases.append(theCase);
        }
    controller->AddCaseToSet(cases, "test1");
    list = controller->GetSetList();    
    int after_count = list.count();
    QCOMPARE(before_count + 1, after_count);
    QCOMPARE(OnSetListChangedFired, true);
    }

void testcontroller::T_CreateSet()
    {
    //tested object is still not implemented.
    }

void testcontroller::T_DeleteSet()
    {
    //tested object is still not implemented.
    }

void testcontroller::T_ShowOutput()
    {
    QCOMPARE(controller->ShowOutput(), false);
    controller->SetShowOutput(true);
    QCOMPARE(controller->ShowOutput(), true);
    controller->SetShowOutput(false);
    QCOMPARE(controller->ShowOutput(), false);     
    }

void testcontroller::T_Model_ClearCasesStatus()
    {
    OnCaseStatisticChangedFired = false;
    model->ClearCasesStatus();
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusFailed).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusAborted).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusCrashed).size(), 0);
    }

void testcontroller::T_Model_AddRunningCase_RemoveRunningCase()
    {
    CStartedTestCase* startedCase = 0;
    CSTFCase aCase;
    OnRunningCaseChangedFired = false;
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 0);
    model->AddRunningCase(startedCase, aCase);
    QCOMPARE(OnRunningCaseChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 1);
    
    OnRunningCaseChangedFired = false;    
    model->RemoveRunningCase(startedCase);
    QCOMPARE(OnRunningCaseChangedFired, true);    
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 0);    
    }

void testcontroller::T_Model_AddCaseByStatus_GetCasesByStatus()
    {
    CSTFCase aCase;
    model->ClearCasesStatus();
    OnCaseStatisticChangedFired = false;
    
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 0);
    model->AddCaseByStatus(EStatusExecuted,aCase);
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 1);
    
    OnCaseStatisticChangedFired = false;
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 0);
    model->AddCaseByStatus(EStatusPassed,aCase);    
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 1);

    OnCaseStatisticChangedFired = false;
    QCOMPARE(model->GetCasesByStatus(EStatusFailed).size(), 0);
    model->AddCaseByStatus(EStatusFailed,aCase);    
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusFailed).size(), 1);

    OnCaseStatisticChangedFired = false;
    QCOMPARE(model->GetCasesByStatus(EStatusAborted).size(), 0);
    model->AddCaseByStatus(EStatusAborted,aCase);    
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusAborted).size(), 1);

    OnCaseStatisticChangedFired = false;
    QCOMPARE(model->GetCasesByStatus(EStatusCrashed).size(), 0);
    model->AddCaseByStatus(EStatusCrashed,aCase);    
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusCrashed).size(), 1);
    
    model->ClearCasesStatus();
    
    }

void testcontroller::T_RunCase()
    {
    model->ClearCasesStatus();
    OnCaseStatisticChangedFired = false;
    OnRunningCaseChangedFired = false;
    controller->OpenEngineIniFile(KConfigFile1); 
    QList<CSTFCase> caseList;
    caseList.append(controller->GetCase(KDefaultModuleName,2));//math test
    controller->RunCases(caseList, Sequentially);
    QTest::qWait(2000);
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 1);
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 1);
    
    model->ClearCasesStatus();
    }
     
     
void testcontroller::T_RunSet()
    {
    model->ClearCasesStatus();
    //controller->OpenEngineIniFile(KConfigFile1); 
    QList<QString> list = controller->GetCaseListBySet(KDefaultSetName);    
    QCOMPARE(list.size(),5);
    controller->RunSets(KDefaultSetName,Parallel);
    QTest::qWait(20000);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 5);
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 4);
    QCOMPARE(model->GetCasesByStatus(EStatusFailed).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusAborted).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusCrashed).size(), 1);
    model->ClearCasesStatus();    
    }
     
     
void testcontroller::T_PauseCase_ResumeCase()
    {
    model->ClearCasesStatus();
    OnCaseStatisticChangedFired = false;
    OnRunningCaseChangedFired = false;
    controller->OpenEngineIniFile(KConfigFile1); 
    QList<CSTFCase> caseList;
    caseList.append(controller->GetCase(KDefaultModuleName,0));  //loop test
    controller->RunCases(caseList, Sequentially);
    QTest::qWait(500);
    QCOMPARE(OnRunningCaseChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 1);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 0);
    controller->PauseCase();
    QTest::qWait(15000);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 1);
    controller->ResumeCase();
    QTest::qWait(15000);
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 1);
    QCOMPARE(model->GetCasesByStatus(EStatusPassed).size(), 1);
    
    }
     
void testcontroller::T_AbortCase()
    {
    model->ClearCasesStatus();
    OnCaseStatisticChangedFired = false;
    OnRunningCaseChangedFired = false;
    controller->OpenEngineIniFile(KConfigFile1); 
    QList<CSTFCase> caseList;
    caseList.append(controller->GetCase(KDefaultModuleName,0));  //loop test
    controller->RunCases(caseList, Sequentially);
    QTest::qWait(500);
    QCOMPARE(OnRunningCaseChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 1);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 0);
    controller->AbortCase();
    QTest::qWait(1000);
    QCOMPARE(OnCaseStatisticChangedFired, true);
    QCOMPARE(model->GetCasesByStatus(EStatusRunning).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusExecuted).size(), 0);
    QCOMPARE(model->GetCasesByStatus(EStatusAborted).size(), 1);
    }


//===========================================================
void testcontroller::OnSetListChanged()
    {
    OnSetListChangedFired = true;
    }

void testcontroller::OnCaseStatisticChanged()
    {
    OnCaseStatisticChangedFired = true;
    }

void testcontroller::OnRunningCaseChanged()
    {
    OnRunningCaseChangedFired = true;
    }

void testcontroller::OnGetMessage(const QString& /*aMessage*/)
    {
    //nothing.
    }

void testcontroller::OnCaseOutputChanged(const CaseOutputCommand& /*cmd*/,
        const QString& , const QString&)
    {}

