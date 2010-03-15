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
 *              Stf Controller implementation.
 *
 */
#include "stfqtuicontroller.h"
#include <stifinternal/UIStoreIf.h>
#include <stifinternal/UIStoreContainer.h>
//#include "stiflogger.h"
#include <QDateTime>

const QString TEMPSETNAME = "TEMPSET";
const QString DEFAULTINI = "c:\\testframework\\testframework.ini";

//__DECLARE_LOG

StfQtUIController::StfQtUIController(IStfQtUIModel* aModel) :
    model(aModel), isShowOutput(false)
    {
//    __OPENLOGL ("\\STFQtUI\\", "StifQtUi.log" );
    executor = new CStifExecutor();
    executor->OpenIniFile(DEFAULTINI);
    executor->AddStifCaseUpdateListener(this);
    }

StfQtUIController::~StfQtUIController()
    {
    executor->RemoveStifCaseUpdateListener(this);
    delete executor;
    executor = NULL;
//    __CLOSELOG;
    }
//for cases

bool StfQtUIController::OpenEngineIniFile(const QString& fileName)
    {
    delete executor;
    executor = new CStifExecutor();
    bool rst = executor->OpenIniFile(fileName);
    executor->AddStifCaseUpdateListener(this);
    return rst;
    }

QList<QString> StfQtUIController::GetModuleList()
    {
    QList<CSTFModule> modules = executor->GetModuleList();
    QList<QString> moduleList;
    foreach(CSTFModule m, modules)
            {
            moduleList.append(m.Name());
            }
    return moduleList;
    }

CSTFModule StfQtUIController::GetModuleByName(const QString& moduleName)
    {
    QList<CSTFModule> modules = executor->GetModuleList();
    CSTFModule module;
    foreach(CSTFModule m, modules)
        {
        if(m.Name() == moduleName)
            {
            module = m;
            break;
            }
        }
    return module;
        
    }

QList<QString> StfQtUIController::GetCaseListByModule(const QString& moduleName)
    {
    QList<QString> caseList;
    if (moduleName != "")
        {
        QList<CSTFCase> cases = executor->GetCaseList(moduleName);
        foreach(CSTFCase c, cases)
                {
                caseList.append(c.Name());
                }
        }

    return caseList;
    }

CSTFCase StfQtUIController::GetCase(const QString& moduleName, const int index)
    {
    CSTFCase rst;
    if(moduleName != "")
        {
        QList<CSTFCase> cases = executor->GetCaseList(moduleName);
        if(index < cases.length())
            {
            rst = cases.at(index);
            }
        }
    return rst;
    }

void StfQtUIController::RunCases(const QList<CSTFCase>& caseList,
        const TSTFCaseRunningType& type)
    {
    if (caseList.size() == 1)
        {
        CSTFCase aCase = caseList.at(0);
        QString msg = "Start execute case:" + aCase.Name();
        FireOnGetOutput(msg);
        executor->ExecuteSingleCase(aCase.ModuleName(), aCase.Index());
        }
    else
        {
        //create a temp set, append cases into the set and execute it.
        executor->CreateSet(TEMPSETNAME);
        foreach(CSTFCase aCase, caseList)
                {
                executor->AddtoSet(TEMPSETNAME, aCase);
                }
        RunSets(TEMPSETNAME, type);
        executor->RemoveSet(TEMPSETNAME);
        }
    }

void StfQtUIController::AddCaseToSet(const QList<CSTFCase>& caseList,
        const QString& /*setName*/)
    {
    QString setName = QDateTime::currentDateTime().toString("hh_mm_ss");
    setName.append(".set");
    executor->CreateSet(setName);
    foreach(CSTFCase aCase, caseList)
            {
            executor->AddtoSet(setName, aCase);
            }
    executor->SaveSet(setName);
    executor->RemoveSet(setName);
    FireOnSetListChanged();
    }

//for set

QList<QString> StfQtUIController::GetSetList()
    {
    return executor->GetSetList();
    }
QList<QString> StfQtUIController::GetCaseListBySet(const QString& setName)
    {
    QList<CSTFCase> cases = executor->GetCaseListFromSet(setName);
    QList<QString> caseList;
    foreach(CSTFCase c, cases)
            {
            caseList.append(c.Name());
            }
    return caseList;
    }

void StfQtUIController::CreateSet(const QString& setName)
    {
    executor->CreateSet(setName);
    //executor->SaveSet(setName);
    FireOnSetListChanged();
    }

void StfQtUIController::DeleteSet(const QString& setName)
    {
    executor->RemoveSet(setName);
    //executor->SaveSet(setName);
    FireOnSetListChanged();
    }

void StfQtUIController::RunSets(const QString& setName, const TSTFCaseRunningType& type)
    {
    executor->ExecuteSet(setName, 0, type);
    }

//for Started
void StfQtUIController::PauseCase()
    {
    model->PauseCase();
    QString msg = "Execution Paused";
    FireOnGetOutput(msg);
    }

void StfQtUIController::ResumeCase()
    {
    model->ResumeCase();
    FireOnGetOutput("Execution Resumed");
    }

void StfQtUIController::AbortCase()
    {
    model->AbortCase();
    FireOnGetOutput("Case Aborted");
    }

bool StfQtUIController::ShowOutput()
    {
    return isShowOutput;
    }

void StfQtUIController::SetShowOutput(bool isShow)
    {
    isShowOutput = isShow;
    }

QList<CSTFCase> StfQtUIController::GetCasesByStatus(const TSTFCaseStatusType& type)
    {
    return model->GetCasesByStatus(type);
    }

void StfQtUIController::AddStfEventListener(IStfEventListener* listener)
    {
    if (!listenerList.contains(listener))
        {
        listenerList.append(listener);
        }
    }
void StfQtUIController::RemoveStfEventListener(IStfEventListener* listener)
    {
    if (listenerList.contains(listener))
        {
        listenerList.removeOne(listener);
        }
    }

void StfQtUIController::OnGetCaseUpdated(CStartedTestCase* aCase,
        CSTFCase& stfcase, int flags)
    {
    if (flags & CUIStoreIf::EPrintUpdate || aCase == NULL)
        {
        return;
        }
    QString msg = "case Name:";
    msg += stfcase.Name() + "\r\n Status:";
    flags = aCase->Status();
    if (flags & CUIStoreIf::EStatusRunning)
        {
        model->AddRunningCase(aCase, stfcase);
        msg += "start running";
        FireOnCaseOutputChanged(IStfEventListener::ECreate, (int) aCase,
                stfcase.Name());
        }
    else if (flags & CUIStoreIf::EStatusAborted)
        {
        FireOnCaseOutputChanged(IStfEventListener::EClose, (int) aCase, "");
        model->RemoveRunningCase(aCase);
        model->AddCaseByStatus(EStatusAborted, stfcase);
        msg += "aborted";

        }
    else if (flags & CUIStoreIf::EStatusExecuted)
        {
        FireOnCaseOutputChanged(IStfEventListener::EClose, (int) aCase, "");
        model->RemoveRunningCase(aCase);
        model->AddCaseByStatus(EStatusExecuted, stfcase);

        if (flags & CUIStoreIf::EStatusCrashed)
            {
            model->AddCaseByStatus(EStatusCrashed, stfcase);
            msg += "crashed";
            }
        else if (flags & CUIStoreIf::EStatusFailed)
            {
            model->AddCaseByStatus(EStatusFailed, stfcase);
            msg += "failed";
            }
        else if (flags & CUIStoreIf::EStatusPassed)
            {
            model->AddCaseByStatus(EStatusPassed, stfcase);
            msg += "passed";
            }

        }
    else
        {
        return;
        }

    FireOnGetOutput(msg);
    }

void StfQtUIController::OnGetCaseOutput(CStartedTestCase* aCase, QString& msg)
    {
    FireOnCaseOutputChanged(IStfEventListener::EOutput, (int) aCase, msg);
    }

void StfQtUIController::FireOnCaseOutputChanged(
        IStfEventListener::CaseOutputCommand cmd, int index, QString msg)
    {
    if (ShowOutput())
        {
        foreach(IStfEventListener* listener, listenerList)
                {
                listener->OnCaseOutputChanged(cmd,
                        QString::number(index, 10), msg);
                }
        }
    }

void StfQtUIController::FireOnGetOutput(QString message)
    {
    foreach(IStfEventListener* listener, listenerList)
            {
            listener->OnGetMessage(message);
            }
    }

void StfQtUIController::FireOnSetListChanged()
    {
    foreach(IStfEventListener* listener, listenerList)
            {
            listener->OnSetListChanged();
            }
    }
