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
* Description:  Definitions for class CATParseTrace, CProcessData and
*               CSubTestData.
*
*/


#ifndef __CATPARSETRACEFILE_H__
#define __CATPARSETRACEFILE_H__

#include "../inc/ATCommonDefines.h"
#include "../inc/CATDataSaver.h"
#include "../inc/CATBase.h"
#include "../inc/catallocs.h"

/**
* Parses raw trace data to AnalyzeTool specific data file.
*/
class CATParseTraceFile : public CATBase
{
public:
	/**
	* Constructor
	*/
	CATParseTraceFile();

	/**
	* Main function to start parse.
	*
	* @param pFileName A trace file name
	* @param pOutputFileName Pointer to output file name
	*/
	bool StartParse( const char* pFileName, const char* pOutputFileName );

	/**
	* Get data saver
	* @return CATDataSaver*
	*/
	CATDataSaver* GetDataSaver(void);	

#ifndef MODULE_TEST
private:
#endif
	CATDataSaver m_DataSaver; /** Data saver */
};

/**
* Represents data of a single subtest.
*/
class CSubTestData : public CATAllocs
{
public:
	// Members

	bool bRunning; /** Is sub test running */
	vector<string> vData; /** data */
	string sSubTestName; /** test name */
	string sStartTime; /** start time */
	string sEndTime; /** end time */
	string sSubTestStartHandleCount; /** handle count at start */
	string sSubTestEndHandleCount; /** handle count at end */
};

/**
* Represents data of a single run.
*/
class CProcessData : public CATAllocs
{
public:
	// Members

	bool bProcessOnGoing; /** is process on going */
	int iProcessID; /** process id (pid)*/
	vector<string> vData; /** data */
	vector<string> vHandleLeaks; /** handle leak(s) */
	vector<CSubTestData> vSubTests; /** process sub test(s) */
};

#endif
