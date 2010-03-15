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
#include <QtGui>
#include "frmmain.h"
#include "stfqtuicontroller.h"
#include "stfqtuimodel.h"
#include <QList>
#include "version.h"
#include <QCursor>

const QString SELECTITEMHEADER = " * ";
const QString UNSELECTITEMHEADER = "   ";

frmMain::frmMain()
    {
    uiSetting = new UiSetting();
    createMenus();
    load();
    LoadSubMenu();
    model = new StfQtUIModel();
    model->AddStifModelEventListener(this);
    controller = new StfQtUIController(model);
    controller->AddStfEventListener(this);
    loadContent();
    dlgOutput = new DlgOutput(controller);
    setSetting();
    }

frmMain::~frmMain()
    {
    model->AbortCase();
    controller->RemoveStfEventListener(this);
    model->RemoveStifModelEventListener(this);
    
    delete uiSetting;
    delete dlgOutput;
    delete controller;
    delete model;
    }

void frmMain::setSetting()
    {
    controller->SetShowOutput(uiSetting->ReadSetting("showoutput") == "true");    
    }

void frmMain::OnGetMessage(const QString& aMessage)
    {
    txtOutput->appendPlainText(aMessage);
    }

void frmMain::OnRunningCaseChanged()
    {
    QList<CSTFCase> caseList = controller->GetCasesByStatus(EStatusRunning);
    lstStartedCases->clear();
    foreach(CSTFCase aCase, caseList)
            {
            lstStartedCases->addItem(aCase.Name());
            }
    if (caseList.size() != 0)
        {
        btnPauseCase->setEnabled(true);
        btnAbortCase->setEnabled(true);
        actPause->setEnabled(true);
        actAbort->setEnabled(true);
        }
    else
        {
        btnPauseCase->setEnabled(false);
        btnAbortCase->setEnabled(false);
        actPause->setEnabled(false);
        actAbort->setEnabled(false);
        }
    }

void frmMain::OnCaseOutputChanged(const IStfEventListener::CaseOutputCommand& /*cmd*/, const QString& /*index*/, const QString& /*msg*/)
    {
    //nothing to do.
    }

void frmMain::OnSetListChanged()
    {
    loadSetList();
    }

void frmMain::OnCaseStatisticChanged()
    {
    loadStatistic();
    }

void frmMain::createMenus()
    {
    //operateMenu = menuBar()->addMenu(tr("&Operation"));
    actAbout = new QAction(tr("&About"), this);
    connect(actAbout, SIGNAL(triggered()), this,
            SLOT(on_actAbout_triggered()));

    actExit = new QAction(tr("&Exit"), this);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actOpenFile = new QAction(tr("&Open Ini File"), this);
    connect(actOpenFile, SIGNAL(triggered()), this,
            SLOT(on_actOpenFile_triggered()));

    actRunCaseSeq = new QAction(tr("Run Case Sequentially"), this);
    connect(actRunCaseSeq, SIGNAL(triggered()), this,
            SLOT(on_actRunCaseSeq_triggered()));

    actRunCasePar = new QAction(tr("Run Case Parallel"), this);
    connect(actRunCasePar, SIGNAL(triggered()), this,
            SLOT(on_actRunCasePar_triggered()));

    actAddtoSet = new QAction(tr("Add cases to Set"), this);
    connect(actAddtoSet, SIGNAL(triggered()), this,
            SLOT(on_actAddtoSet_triggered()));

    actSelectAll = new QAction(tr("Select All"), this);
    connect(actSelectAll, SIGNAL(triggered()), this,
            SLOT(on_actSelectAll_triggered()));

    actExpandAll = new QAction(tr("Expand All"), this);
    connect(actExpandAll, SIGNAL(triggered()), this,
            SLOT(on_actExpandAll_triggered()));

    actCollapseAll = new QAction(tr("Collapse All"), this);
    connect(actCollapseAll, SIGNAL(triggered()), this,
            SLOT(on_actCollapseAll_triggered()));

    actSetting = new QAction(tr("Setting"), this);
    connect(actSetting, SIGNAL(triggered()), this,
            SLOT(on_actSetting_triggered()));

    actRunSetSeq = new QAction(tr("Run Set Sequentially"), this);
    connect(actRunSetSeq, SIGNAL(triggered()), this,
            SLOT(on_actRunSetSeq_triggered()));

    actRunSetPar = new QAction(tr("Run Set Parallel"), this);
    connect(actRunSetPar, SIGNAL(triggered()), this,
            SLOT(on_actRunSetPar_triggered()));

    actNewSet = new QAction(tr("Create New Set"), this);
    connect(actNewSet, SIGNAL(triggered()), this,
            SLOT(on_actNewSet_triggered()));

    actDelSet = new QAction(tr("Delete Set"), this);
    connect(actDelSet, SIGNAL(triggered()), this,
            SLOT(on_actDelSet_triggered()));

    actPause = new QAction(tr("Pause"), this);
    actPause->setEnabled(false);
    connect(actPause, SIGNAL(triggered()), this,
            SLOT(on_actPause_triggered()));

    actAbort = new QAction(tr("Abort"), this);
    actAbort->setEnabled(false);
    connect(actAbort, SIGNAL(triggered()), this,
            SLOT(on_actAbort_triggered()));

    actClearStatistics = new QAction(tr("Clear Statistics"), this);
    connect(actClearStatistics, SIGNAL(triggered()), this,
            SLOT(on_actClearStatistics_triggered()));

    }

void frmMain::load()
    {
    this->setContextMenuPolicy(Qt::NoContextMenu);
    QSize btnSize(100,35);
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setVerticalSpacing(2);
    mainLayout->setHorizontalSpacing(2);
    mainLayout->setSpacing(2);
    mainLayout->setMargin(2);

    MainWidget = new QWidget(this);
    MainWidget->setContextMenuPolicy(Qt::NoContextMenu);

    //tab control
    tabWidget = new QTabWidget(this);
    tabWidget->setContextMenuPolicy(Qt::NoContextMenu);
    tabCase = new QWidget(tabWidget);
    tabCase->setContextMenuPolicy(Qt::NoContextMenu);
    tabWidget->addTab(tabCase, tr("Cases"));
    tabSet = new QWidget(tabWidget);
    tabSet->setContextMenuPolicy(Qt::NoContextMenu);
    tabWidget->addTab(tabSet, tr(" Set "));
    tabStarted = new QWidget(tabWidget);
    tabStarted->setContextMenuPolicy(Qt::NoContextMenu);
    tabWidget->addTab(tabStarted, tr("Running"));
    tabStatistic = new QWidget(tabWidget);
    tabStatistic->setContextMenuPolicy(Qt::NoContextMenu);
    tabWidget->addTab(tabStatistic, tr("Statistics"));
    connect(tabWidget, SIGNAL(currentChanged(int)), this,
            SLOT(onTabWidgetSelectIndexChanged()));

    //output panel
    QGroupBox *groupBox = new QGroupBox(this);
    groupBox->setFixedHeight(150);
    groupBox->setContextMenuPolicy(Qt::NoContextMenu);
    groupBox->setTitle(tr("Information"));
    QFont serifFont("Times", 5, QFont::Normal);
    txtOutput = new QPlainTextEdit(groupBox);
    txtOutput->setFont(serifFont);
    txtOutput->setContextMenuPolicy(Qt::NoContextMenu);
    txtOutput->setReadOnly(true);
    txtOutput->setFocusPolicy(Qt::NoFocus);
    //txtOutput->setEditFocus(false);
    QGridLayout *groupBoxLayout = new QGridLayout(this);
    groupBoxLayout->setVerticalSpacing(2);
    groupBoxLayout->setHorizontalSpacing(2);
    groupBoxLayout->setSpacing(2);
    groupBoxLayout->setMargin(2);   
    groupBoxLayout->addWidget(txtOutput, 0, 0);
    groupBox->setLayout(groupBoxLayout);

    //Create MainLayout and MainWidget
    mainLayout->addWidget(tabWidget, 0, 0);
    mainLayout->addWidget(groupBox, 1, 0, Qt::AlignBottom);
    MainWidget->setLayout(mainLayout);
    

    //Tab page: Case
    QGridLayout *tabCaseLayout = new QGridLayout(this);
    tabCaseLayout->setVerticalSpacing(2);
    tabCaseLayout->setHorizontalSpacing(2);
    tabCaseLayout->setSpacing(2);
    tabCaseLayout->setMargin(2);   
    treeModuleList = new QTreeWidget(tabCase);
    treeModuleList->setContextMenuPolicy(Qt::NoContextMenu);
    treeModuleList->headerItem()->setText(0, tr("Module List"));
    treeModuleList->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeModuleList->setEditFocus(false);
    connect(treeModuleList, SIGNAL(itemClicked(QTreeWidgetItem* , int)), this, 
            SLOT(on_treeModuleList_itemClicked(QTreeWidgetItem* , int)));
    

    QWidget *caseToolWidget = new QWidget(tabCase);
    caseToolWidget->setContextMenuPolicy(Qt::NoContextMenu);
    QGridLayout *caseToolWidgetLayout = new QGridLayout;
    QPushButton *btnRunCase = new QPushButton(tr("Run"), caseToolWidget);
    btnRunCase->setContextMenuPolicy(Qt::NoContextMenu);
    btnRunCase->setFixedSize(btnSize);
    connect(btnRunCase, SIGNAL(clicked()), this,
            SLOT(on_actRunCaseSeq_triggered()));
    QPushButton *btnExpandAll = new QPushButton(tr("Expand"), caseToolWidget);
    btnExpandAll->setContextMenuPolicy(Qt::NoContextMenu);
    btnExpandAll->setFixedSize(btnSize);
    connect(btnExpandAll, SIGNAL(clicked()), this,
            SLOT(on_actExpandAll_triggered()));
    QPushButton *btnCollapseAll = new QPushButton(tr("Collapse"),
            caseToolWidget);
    btnCollapseAll->setContextMenuPolicy(Qt::NoContextMenu);
    btnCollapseAll->setFixedSize(btnSize);
    connect(btnCollapseAll, SIGNAL(clicked()), this,
            SLOT(on_actCollapseAll_triggered()));

    caseToolWidgetLayout->addWidget(btnRunCase, 0, 0);
    caseToolWidgetLayout->addWidget(btnExpandAll, 0, 1);
    caseToolWidgetLayout->addWidget(btnCollapseAll, 0, 2);
    caseToolWidget->setLayout(caseToolWidgetLayout);

    tabCaseLayout->addWidget(caseToolWidget, 1, 0);
    tabCaseLayout->addWidget(treeModuleList, 0, 0);
    tabCase->setLayout(tabCaseLayout);

    //Tab page: Set

    QGridLayout *tabSetLayout = new QGridLayout(this);
    tabSetLayout->setVerticalSpacing(2);
    tabSetLayout->setHorizontalSpacing(2);
    tabSetLayout->setSpacing(2);
    tabSetLayout->setMargin(2);   

    QGridLayout *tabSetMainLayout = new QGridLayout(this);
    tabSetMainLayout->setVerticalSpacing(2);
    tabSetMainLayout->setHorizontalSpacing(2);
    tabSetMainLayout->setSpacing(2);
    tabSetMainLayout->setMargin(2);   
    QWidget *tabSetMainWidget = new QWidget(tabSet);
    tabSetMainWidget->setContextMenuPolicy(Qt::NoContextMenu);
    QLabel *lblSet = new QLabel(tr("Test Set:"), tabSetMainWidget);
    lblSet->setContextMenuPolicy(Qt::NoContextMenu);
    QLabel *lblCase = new QLabel(tr("Cases:"), tabSetMainWidget);
    lblCase->setContextMenuPolicy(Qt::NoContextMenu);
    cboSetList = new QComboBox(tabSetMainWidget);
    cboSetList->setContextMenuPolicy(Qt::NoContextMenu);
    cboSetList->setEditable(false);
    connect(cboSetList, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(on_cboSetList_currentIndexChanged(QString)));
    lstSetCases = new QListWidget(tabSetMainWidget);
    lstSetCases->setContextMenuPolicy(Qt::NoContextMenu);
    tabSetMainLayout->addWidget(lblSet, 0, 0);
    tabSetMainLayout->addWidget(cboSetList, 0, 1);
    tabSetMainLayout->addWidget(lblCase, 1, 0,
            (Qt::AlignTop | Qt::AlignRight));
    tabSetMainLayout->addWidget(lstSetCases, 1, 1);
    tabSetMainWidget->setLayout(tabSetMainLayout);

    QWidget *setToolWidget = new QWidget(tabSet);
    setToolWidget->setContextMenuPolicy(Qt::NoContextMenu);
    QGridLayout *setToolWidgetLayout = new QGridLayout(this);
    setToolWidgetLayout->setVerticalSpacing(2);
    setToolWidgetLayout->setHorizontalSpacing(2);
    setToolWidgetLayout->setSpacing(2);
    setToolWidgetLayout->setMargin(2);     
    QPushButton *btnRunSetCase = new QPushButton(tr("Run"), setToolWidget);
    btnRunSetCase->setContextMenuPolicy(Qt::NoContextMenu);
    btnRunSetCase->setFixedSize(btnSize);
    connect(btnRunSetCase, SIGNAL(clicked()), this,
            SLOT(on_actRunSetSeq_triggered()));
    QPushButton *btnNewSet = new QPushButton(tr("New Set"), setToolWidget);
    btnNewSet->setContextMenuPolicy(Qt::NoContextMenu);
    btnNewSet->setFixedSize(btnSize);
    connect(btnNewSet, SIGNAL(clicked()), this,
            SLOT(on_actNewSet_triggered()));
    QPushButton *btnDelSet = new QPushButton(tr("Delete Set"), setToolWidget);
    btnDelSet->setContextMenuPolicy(Qt::NoContextMenu);
    btnDelSet->setFixedSize(btnSize);
    connect(btnDelSet, SIGNAL(clicked()), this,
            SLOT(on_actDelSet_triggered()));

    setToolWidgetLayout->addWidget(btnRunSetCase, 0, 0);
    setToolWidgetLayout->addWidget(btnNewSet, 0, 1);
    setToolWidgetLayout->addWidget(btnDelSet, 0, 2);
    setToolWidget->setLayout(setToolWidgetLayout);

    tabSetLayout->addWidget(tabSetMainWidget, 0, 0);
    tabSetLayout->addWidget(setToolWidget, 1, 0);
    tabSet->setLayout(tabSetLayout);

    //Tab Started
    QGridLayout *tabStartedLayout = new QGridLayout(this);
    tabStartedLayout->setVerticalSpacing(2);
    tabStartedLayout->setHorizontalSpacing(2);
    tabStartedLayout->setSpacing(2);
    tabStartedLayout->setMargin(2);     
    lstStartedCases = new QListWidget(tabStarted);
    lstStartedCases->setContextMenuPolicy(Qt::NoContextMenu);
    QWidget *startedToolWidget = new QWidget(tabStarted);
    startedToolWidget->setContextMenuPolicy(Qt::NoContextMenu);
    QGridLayout *startedToolWidgetLayout = new QGridLayout(this);
    startedToolWidgetLayout->setVerticalSpacing(2);
    startedToolWidgetLayout->setHorizontalSpacing(2);
    startedToolWidgetLayout->setSpacing(2);
    startedToolWidgetLayout->setMargin(2);
    btnPauseCase = new QPushButton(tr("Pause"), startedToolWidget);
    btnPauseCase->setContextMenuPolicy(Qt::NoContextMenu);
    btnPauseCase->setFixedSize(btnSize);
    connect(btnPauseCase, SIGNAL(clicked()), this,
            SLOT(on_actPause_triggered()));
    btnPauseCase->setEnabled(false);

    btnAbortCase = new QPushButton(tr("Abort"), startedToolWidget);
    btnAbortCase->setContextMenuPolicy(Qt::NoContextMenu);
    btnAbortCase->setFixedSize(btnSize);
    connect(btnAbortCase, SIGNAL(clicked()), this,
            SLOT(on_actAbort_triggered()));
    btnAbortCase->setEnabled(false);
    //    
    //    QPushButton *btnShowOutput = new QPushButton(tr("Output"), startedToolWidget);
    //    connect(btnShowOutput, SIGNAL(clicked()), this,
    //                SLOT(on_btnShowOutput_clicked()));


    startedToolWidgetLayout->addWidget(btnPauseCase, 0, 0);
    startedToolWidgetLayout->addWidget(btnAbortCase, 0, 1);
    //startedToolWidgetLayout->addWidget(btnShowOutput, 0, 2);
    startedToolWidget->setLayout(startedToolWidgetLayout);

    tabStartedLayout->addWidget(lstStartedCases, 0, 0);
    tabStartedLayout->addWidget(startedToolWidget, 1, 0);
    tabStarted->setLayout(tabStartedLayout);

    //Tab Statistic
    QGridLayout *tabStatisticLayout = new QGridLayout(this);
    tabStatisticLayout->setVerticalSpacing(2);
    tabStatisticLayout->setHorizontalSpacing(2);
    tabStatisticLayout->setSpacing(2);
    tabStatisticLayout->setMargin(2);

    treeStatistic = new QTreeWidget(tabStatistic);
    treeStatistic->setContextMenuPolicy(Qt::NoContextMenu);
    treeStatistic->headerItem()->setText(0, tr("Statistics"));
    tabStatisticLayout->addWidget(treeStatistic, 0, 0);
    tabStatistic->setLayout(tabStatisticLayout);

    executedItems = new QTreeWidgetItem(treeStatistic);
    executedItems->setText(0, tr("Executed Cases(0)"));
    passedItems = new QTreeWidgetItem(treeStatistic);
    passedItems->setText(0, tr("Passed Cases(0)"));
    failedItems = new QTreeWidgetItem(treeStatistic);
    failedItems->setText(0, tr("Failed Cases(0)"));
    crashedItems = new QTreeWidgetItem(treeStatistic);
    crashedItems->setText(0, tr("Crashed Cases(0)"));
    abortedItems = new QTreeWidgetItem(treeStatistic);
    abortedItems->setText(0, tr("Aborted Cases(0)"));
    
    
    setCentralWidget(MainWidget);

    }

void frmMain::LoadSubMenu()
    {
    menuBar()->clear();
    menuBar()->setContextMenuPolicy(Qt::NoContextMenu);
    if (tabWidget->currentIndex() == 0)
        {
        //Cases Tab
        menuBar()->addAction(actOpenFile);
        menuBar()->addAction(actRunCaseSeq);
        menuBar()->addAction(actRunCasePar);
        menuBar()->addSeparator();
        menuBar()->addAction(actAddtoSet);
        menuBar()->addSeparator();
        menuBar()->addAction(actSelectAll);
        menuBar()->addAction(actExpandAll);
        menuBar()->addAction(actCollapseAll);
        }
    else if (tabWidget->currentIndex() == 1)
        {
        //Set Tab
        menuBar()->addAction(actRunSetSeq);
        menuBar()->addAction(actRunSetPar);
        menuBar()->addSeparator();
        menuBar()->addAction(actNewSet);
        menuBar()->addAction(actDelSet);
        }
    else if (tabWidget->currentIndex() == 2)
        {
        //Started Tab
        menuBar()->addAction(actPause);
        menuBar()->addAction(actAbort);
        
        }
    else
        {
        //Staticstic tab
        menuBar()->addAction(actClearStatistics);
        }
    menuBar()->addSeparator();
    menuBar()->addAction(actSetting);
    menuBar()->addAction(actAbout);
    menuBar()->addAction(actExit);

    }

void frmMain::onTabWidgetSelectIndexChanged()
    {
    LoadSubMenu();
    }

void frmMain::loadContent()
    {
    //Load ModuleList
    loadModuleList();
    //Load SetList
    loadSetList();
    //Load Statistic List
    loadStatistic();
    }

void frmMain::loadModuleList()
    {
    treeModuleList->clear();
    
    QList<QString> moduleList = controller->GetModuleList();
    foreach(QString moduleName, moduleList)
            {
            QTreeWidgetItem* item = new QTreeWidgetItem(treeModuleList);
            item->setText(0, UNSELECTITEMHEADER + moduleName);
            
            QList<QString> caseList = controller->GetCaseListByModule(
                    moduleName);

            foreach(QString caseName, caseList)
                    {
                    QTreeWidgetItem* caseItem = new QTreeWidgetItem(item);        
                    caseItem->setText(0, UNSELECTITEMHEADER + caseName);
                    }
            }
    if (moduleList.size() > 0)
        {
        treeModuleList->setCurrentItem(treeModuleList->topLevelItem(0));
        }
    }

void frmMain::reloadStatisticItem(QString name, QTreeWidgetItem* item,
        TSTFCaseStatusType type)
    {
    QList<CSTFCase> caseList = controller->GetCasesByStatus(type);
    while (item->childCount() != 0)
        {
        item->removeChild(item->child(0));
        }
    item->setText(0, name + "(" + QString::number(caseList.size(), 10) + ")");
    foreach(CSTFCase aCase, caseList)
            {
            QTreeWidgetItem* child = new QTreeWidgetItem(item);
            child->setText(0, aCase.Name());
            }
    }

void frmMain::loadStatistic()
    {
    //executedItems;
    reloadStatisticItem("Executed Cases", executedItems, EStatusExecuted);

    //passedItems;
    reloadStatisticItem("Passed Cases", passedItems, EStatusPassed);

    //failedItems;
    reloadStatisticItem("Failed Cases", failedItems, EStatusFailed);

    //crashedItems;
    reloadStatisticItem("Crashed Cases", crashedItems, EStatusCrashed);

    //abortedItems;
    reloadStatisticItem("Aborted Cases", abortedItems, EStatusAborted);

    }

void frmMain::loadSetList()
    {
    cboSetList->clear();

    QList<QString> setList = controller->GetSetList();
    foreach(QString setName, setList)
            {
            cboSetList->addItem(setName);
            }
//    if (setList.size() > 0)
//        {
//        //cboSetList->setCurrentIndex(0);
//        on_cboSetList_currentIndexChanged(setList.at(0));
//        }
    }

QList<CSTFCase> frmMain::getSelectedCases()
    {
    int index = 0;
    QTreeWidgetItem* item = treeModuleList->topLevelItem(index);
    QList<CSTFCase> caseList;
    while (item != 0)
        {
        for (int i = 0; i < item->childCount(); i++)
            {
            QTreeWidgetItem* child = item->child(i);
            if (child->text(0).startsWith(SELECTITEMHEADER))
                {
                CSTFCase aCase(child->text(0).remove(0,3), i);
                aCase.SetIndex(i);
                //aCase.SetModuleName(moduleBox->text());
                aCase.SetModuleName(item->text(0).remove(0,3));
                caseList.append(aCase);
                }
            }
        index++;
        item = treeModuleList->topLevelItem(index);
        }
    return caseList;
    }

void frmMain::on_cboSetList_currentIndexChanged(QString item)
    {
    lstSetCases->clear();
    QList<QString> list = controller->GetCaseListBySet(item);
    foreach(QString caseName, list)
            {
            lstSetCases->addItem(caseName);
            }
    }

void frmMain::on_actRunCaseSeq_triggered()
    {
    //run case seq
    controller->RunCases(getSelectedCases(), Sequentially);
    }

void frmMain::on_actRunCasePar_triggered()
    {
    controller->RunCases(getSelectedCases(), Parallel);
    }

void frmMain::on_actAddtoSet_triggered()
    {

    QList<CSTFCase> list = getSelectedCases();
    if (list.size() == 0)
        {
        QErrorMessage *errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(tr(
                "Please select cases you want to added to set."));
        return;
        }

    QList<QString> setList = controller->GetSetList();

    /*    
     bool ok;
     QString setName = QInputDialog::getItem(this, tr(
     "Add select cases to Set"), tr("Sets:"), setList, 0, false, &ok);
     if (ok && !setName.isEmpty())
     {
     controller->AddCaseToSet(list, setName);
     }
     */

    //temp code, because UIStore()->AddCaseToSet() is not support to define a set name.
    controller->AddCaseToSet(list, "");
    tabWidget->setCurrentIndex(1);

    }

void frmMain::on_actSelectAll_triggered()
    {
    QString header = UNSELECTITEMHEADER;
    if(actSelectAll->text() == "Select All")
        {
        actSelectAll->setText("UnSelect All");
        header = SELECTITEMHEADER;
        }
    else
        {
        actSelectAll->setText("Select All");    
        }
    
    int index = 0;
    QTreeWidgetItem* item = treeModuleList->topLevelItem(index);
    while (item != 0)
        {
        item->setText(0, item->text(0).replace(0,3, header));
        for (int i = 0; i < item->childCount(); i++)
            {
            QTreeWidgetItem* child = item->child(i);
            child->setText(0,child->text(0).replace(0,3,header));
            }
        index++;
        item = treeModuleList->topLevelItem(index);
        }
    }

void frmMain::on_actExpandAll_triggered()
    {
    treeModuleList->expandAll();
    }

void frmMain::on_actCollapseAll_triggered()
    {
    treeModuleList->collapseAll();
    }

void frmMain::on_actSetting_triggered()
    {
    DlgSetting dlgSet(uiSetting);
    int result = dlgSet.exec();
    if(result == QDialog::Accepted)
        {
        setSetting();
        }
    }

void frmMain::on_actRunSetSeq_triggered()
    {
    QString setName = cboSetList->currentText();
    controller->RunSets(setName, Sequentially);
    }

void frmMain::on_actRunSetPar_triggered()
    {
    QString setName = cboSetList->currentText();
    controller->RunSets(setName, Parallel);
    }

void frmMain::on_actNewSet_triggered()
    {
    //not supported.
    QErrorMessage *errorMessageDialog = new QErrorMessage(this);
    errorMessageDialog->showMessage(
            tr(
                    "The feature is not supported in this version.\r\n    \
            If you want to Add test set.\r\n           \
            Please switch to \"Case\" tab, Select case(s) and perform \"Add case to set\"."));
    return;
    /*
     * These function is not supported in this version.
     * Unless this function has been impelemented:
     * UIStore()->AddSet(setName);
     * 
     bool ok;
     QString text = QInputDialog::getText(this, tr("Create a new Set"), tr(
     "Input a set name:"), QLineEdit::Normal, QDir::home().dirName(),
     &ok);
     if (ok && !text.isEmpty())
     {
     controller->CreateSet(text);
     }
     */
    }

void frmMain::on_actDelSet_triggered()
    {
    //not supported.
    QErrorMessage *errorMessageDialog = new QErrorMessage(this);
    errorMessageDialog->showMessage(
            tr(
                    "The feature is not supported in this version.\r\n    \
            If you want to remove test set.\r\n           \
            Please delete them under {epoc root}\\winscw\\c\\TestFramework  \
            And restart Application"));
    return;
    /*
     * These function is not supported in this version.
     * Unless this function has been impelemented:
     * UIStore()->RemoveSet(setName);
     * 
     QString setName = cboSetList->currentText();
     QMessageBox msgBox(QMessageBox::Warning, tr("Delete a Set"), tr(
     "Do you really want to delete the set?"), 0, this);
     msgBox.addButton(tr("&Delete"), QMessageBox::AcceptRole);
     msgBox.addButton(tr("&Cancel"), QMessageBox::RejectRole);
     if (msgBox.exec() == QMessageBox::AcceptRole)
     {
     controller->DeleteSet(setName);
     }
     
     
     */
    }

void frmMain::on_actPause_triggered()
    {
    if (btnPauseCase->text() == "Pause")
        {
        controller->PauseCase();
        btnPauseCase->setText(tr("Resume"));
        actPause->setText(tr("Resume"));
        }
    else
        {
        controller->ResumeCase();
        btnPauseCase->setText(tr("Pause"));
        actPause->setText(tr("Pause"));
        }
    }

void frmMain::on_actAbort_triggered()
    {
    controller->AbortCase();
    }

void frmMain::on_treeModuleList_itemClicked(QTreeWidgetItem* item, int /*column*/)
    {
    QString header = UNSELECTITEMHEADER;
    if(item->text(0).startsWith(UNSELECTITEMHEADER))
        {
            header = SELECTITEMHEADER;
        }
    item->setText(0 , item->text(0).replace(0, 3, header));
    for(int i=0;i<item->childCount();i++)
        {
            item->child(i)->setText(0, item->child(i)->text(0).replace(0, 3, header));
        }
    }

void frmMain::on_actAbout_triggered()
    {
    QString str = QtUIVersion;
    str.append("\r\n").append("STF version:");
    
    str.append(QString::number(STIF_MAJOR_VERSION, 10)).append(".");
    str.append(QString::number(STIF_MINOR_VERSION, 10)).append(".");
    str.append(QString::number(STIF_BUILD_VERSION, 10));
    str.append("  --").append(STIF_REL_DATE).append("\r\n");
    str.append("---");
    str.append("\r\nCopyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. ");

    QMessageBox::information(this, tr("About"), str);
    }

void frmMain::on_actOpenFile_triggered()
    {
    QString fileName = QFileDialog::getOpenFileName(this, tr(
            "Select ini file"), tr("c:\\"), tr(
            "Ini Files (*.ini);;All Files (*)"));
    if (!fileName.isEmpty())
        {
        controller->OpenEngineIniFile(fileName);
        this->loadModuleList();
        }
    }

void frmMain::on_actClearStatistics_triggered()
    {
    model->ClearCasesStatus();
    }
