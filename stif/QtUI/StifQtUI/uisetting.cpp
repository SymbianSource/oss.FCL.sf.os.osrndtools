/*
 * uisetting.cpp
 *
 *  Created on: 2010-2-8
 *      Author: y183zhan
 */

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "uisetting.h"
const QString SETTINGFILE = "c:\\TestFramework\\StfQtUISetting.ini";

UiSetting::UiSetting()
    {
    if(!load())
        {
        loadDefault();
        }
    }

UiSetting::~UiSetting()
    {
    }

QString UiSetting::ReadSetting(const QString& item)
    {
    QString value = "";
    if(settingList.contains(item))
        {
        value = settingList.value(item);
        }
    return value;
    }


void UiSetting::SetSetting(const QString& item, const QString& value)
    {
    if(settingList.contains(item))
        {
        settingList.remove(item);
        }
    settingList.insert(item, value);
    save();
    }


void UiSetting::loadDefault()
    {
    settingList.clear();
    settingList.insert("showoutput", "true");
    //add mor default setting here.
    }


bool UiSetting::load()
    {
    QFile file(SETTINGFILE);
    if(!file.open(QIODevice::ReadOnly))
        {
        return false;
        }
    QTextStream in(&file);
    QString line, item, value;
    int index;
    while(!in.atEnd())
        {
        line = in.readLine().trimmed().toLower();
        if(!line.startsWith("//"))
            {
            index = line.indexOf("=");
            if(index > 0 && index < line.length() - 1)
                {
                item = line.left(index).trimmed();
                value = line.right(line.length() - index  -1);
                settingList.insert(item, value);
                }
            }
        //end while.
        }
    return true;
    }

bool UiSetting::save()
    {
    QFile file(SETTINGFILE);
    if(!file.open(QIODevice::WriteOnly))
        {
        return false;
        }
    QTextStream in(&file);
    in << "//STFQtUI Setting.\r\n";
    in << "//Created at: " + QDateTime::currentDateTime().toString("yyyy.mm.dd hh:mm::ss");
    in << "\r\n";
    for(int i=0;i< settingList.size();i++)
        {
        in << settingList.keys()[i];
        in << "=";
        in << settingList.value(settingList.keys()[i]);
        in << "\r\n";
        }
    return true;    
    }

