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
 * 
 */
#include "dlgoutput.h"
#include <QtGui>

DlgOutput::DlgOutput(IStfQtUIController* ctl, QWidget *parent) :
    QDialog(parent), controller(ctl)
    {
    QSize btnSize(100,30);
    QGridLayout *mainLayout = new QGridLayout(this);
    this->setLayout(mainLayout);
    this->setContextMenuPolicy(Qt::NoContextMenu);

    tabMain = new QTabWidget();
    tabMain->setContextMenuPolicy(Qt::NoContextMenu);

    QWidget *toolWidget = new QWidget(this);
    toolWidget->setContextMenuPolicy(Qt::NoContextMenu);
    QGridLayout *toolLayout = new QGridLayout();
    toolWidget->setLayout(toolLayout);
    btnPause = new QPushButton(tr("Pause"), toolWidget);
    btnPause->setContextMenuPolicy(Qt::NoContextMenu);
    btnPause->setFixedSize(btnSize);
    QObject::connect(btnPause, SIGNAL(clicked()), this,
            SLOT(on_btnPause_clicked()));
    btnAbort = new QPushButton(tr("Abort"), toolWidget);
    btnAbort->setContextMenuPolicy(Qt::NoContextMenu);
    btnAbort->setFixedSize(btnSize);
    QObject::connect(btnAbort, SIGNAL(clicked()), this,
            SLOT(on_btnAbort_clicked()));
    toolLayout->addWidget(btnPause, 0, 0);
    toolLayout->addWidget(btnAbort, 0, 1);

    mainLayout->addWidget(toolWidget, 0, 0);
    mainLayout->addWidget(tabMain, 1, 0);
    controller->AddStfEventListener(this);
    }

DlgOutput::~DlgOutput()
    {
    controller->RemoveStfEventListener(this);
    }

void DlgOutput::CreateItem(QString index, QString item)
    {
    QPlainTextEdit* edit = new QPlainTextEdit(this);
    edit->setContextMenuPolicy(Qt::NoContextMenu);
    tabMain->addTab(edit, item);
    tabList.insert(index, edit);
    }

void DlgOutput::CloseItem(QString index)
    {
    int u = tabList.keys().indexOf(index);
    tabList.remove(index);
    tabMain->removeTab(u);
    if (tabMain->count() == 0)
        {
        this->close();
        }
    }

void DlgOutput::ShowMessage(QString index, QString msg)
    {
    if(tabList.contains(index))
        {
        tabList.value(index)->setPlainText(msg);    
        }
    }

void DlgOutput::on_btnPause_clicked()
    {
    if (btnPause->text() == "Pause")
        {
        controller->PauseCase();
        btnPause->setText(tr("Resume"));
        }
    else
        {
        controller->ResumeCase();
        btnPause->setText(tr("Pause"));
        }
    }

void DlgOutput::on_btnAbort_clicked()
    {
    controller->AbortCase();
    }

void DlgOutput::OnCaseOutputChanged(const IStfEventListener::CaseOutputCommand& cmd,
        const QString& index, const QString& msg)
    {
    this->showMaximized();
    switch (cmd)
        {
        case IStfEventListener::ECreate:
            CreateItem(index, msg);
            break;
        case IStfEventListener::EClose:
            CloseItem(index);
            break;
        default:
            ShowMessage(index, msg);
            break;
        }

    }

