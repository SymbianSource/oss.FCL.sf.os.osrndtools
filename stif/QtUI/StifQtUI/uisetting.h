/*
 * uisetting.h
 *
 *  Created on: 2010-2-8
 *      Author: y183zhan
 */

#ifndef UISETTING_H_
#define UISETTING_H_
#include <QHash>

class UiSetting
    {
public:
    UiSetting();
    ~UiSetting();
    
public:
    QString ReadSetting(const QString& item);
    void SetSetting(const QString& item,const QString& value);
    
private:
    bool load();
    bool save();
    void loadDefault();
    
private:
    QHash<QString, QString> settingList;
    };

#endif /* UISETTING_H_ */
