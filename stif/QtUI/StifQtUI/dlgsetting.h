/*
 * dlgsetting.h
 *
 *  Created on: 2010-2-8
 *      Author: y183zhan
 */

#ifndef DLGSETTING_H_
#define DLGSETTING_H_
#include <QDialog>
#include "uisetting.h"


QT_BEGIN_NAMESPACE
class QTabWidget;
class QGridLayout;
class QCheckBox;
class QPushButton;
QT_END_NAMESPACE

class DlgSetting : public QDialog {    
    Q_OBJECT
    
public:
    DlgSetting(UiSetting* settingObj, QWidget *parent = 0);
    
private:
    void SetupUI();
    
private:
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QCheckBox* chkShowoutput;
    
private slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    
private:
    UiSetting* setting;
    
    };

#endif /* DLGSETTING_H_ */
