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
* Description:  Definitions for the class CATParseTraceFile.
*
*/


#include "../inc/ATCommonDefines.h"
#include "../inc/CATParseTraceFile.h"
#include "../inc/catdatasaver.h"
#include "../inc/CATDatParser.h"

#include <time.h>

#define MAIN_ID "PCSS"
#define ALLOC_ID "ALLOC" // < V.1.6 allocation.
#define ALLOCH_ID "ALLOCH" // Header of multi message allocation.
#define ALLOCF_ID "ALLOCF" // Fragment of multi message allocation.
#define FREE_ID "FREE"
#define FREEH_ID "FREEH" // Header of multi message free.
#define FREEF_ID "FREEF" // Fragment of multi message free.
#define HANDLE_LEAK_ID "HANDLE_LEAK"

const string ERROR_OCCURED = "ERROR_OCCURED"; // Error messages.
const string INCORRECT_ATOOL_VERSION = "INCORRECT_ATOOL_VERSION";
/**
* Invalid characters in trace file line content.
* These will be filtered out before actuall parsing of line.
10 = LF
13 = CR
124 = |
*/
const char cINVALID_TRACE_FILE_CHARS[] = { 10, 13, 124 };

// -----------------------------------------------------------------------------
// CATParseTraceFile::CATParseTraceFile
// Constructor.
// -----------------------------------------------------------------------------
CATParseTraceFile::CATParseTraceFile()
{
	LOG_FUNC_ENTRY("CATParseTraceFile::CATParseTraceFile");
	m_DataSaver.SetPrintFlag( false );
}

// -----------------------------------------------------------------------------
// CATParseTraceFile::StartParse
// Main function to start trace parsing.
// -----------------------------------------------------------------------------
bool CATParseTraceFile::StartParse( const char* pFileName, const char* pOutputFileName )
{
	LOG_FUNC_ENTRY("CATParseTraceFile::StartParse");

	// Return value, will be changed to true if process start found.
	bool bRet = false;

	// Check pointers
	if ( pFileName == NULL  )
		return bRet;

	if( pOutputFileName == NULL )
		return bRet;

	if ( ! FileExists( pFileName ) )
	{
		cout << AT_MSG << "Error, input file \""
			<< pFileName
			<< "\" does not exist." << endl;
		return bRet;
	}

	// Open data file
	ifstream in( pFileName );

	// Check file opened ok
	if ( !in.good() )
		return false;

	// Get stream size
	size_t streamPos = in.tellg();
	in.seekg( 0, ios::end);
	size_t streamEnd = in.tellg();
	in.seekg( 0, ios::beg);

	//Origianl characters (not filtered).
	char cOriginalLineFromFile[MAX_LINE_LENGTH];

	vector<CProcessData> vProcessList;
	int iProcessIDinList = -1;

	bool bFileVersionSaved = false;
	// Read lines
	while( streamPos < streamEnd )
	{
		// Get one line. Don't use stream flags to determinate end of file
		// it can be found too early because trace can contain "anything".
		in.getline( cOriginalLineFromFile, MAX_LINE_LENGTH );
		
		// Refresh position
		streamPos = in.tellg();

		// Check has bad bit flag raised. (i.e. device problems reading data)
		if( in.bad() )
		{
			cout << AT_MSG << "Integrity error reading the trace file, reading aborted." << endl;
			return false;
		}
		//Filtered characters.
		char cLineFromFile[MAX_LINE_LENGTH];
		char* pFiltered = cLineFromFile;

		//Loop thru all characters in original line.
		for( size_t i = 0 ; cOriginalLineFromFile[i] != 0 ; i++ )
		{
			//If character in line is not in invalid character array append it
			//to filtered line.
			if ( strchr( cINVALID_TRACE_FILE_CHARS, cOriginalLineFromFile[i] ) == 0 )
					*pFiltered++ =  cOriginalLineFromFile[i];
		}
		*pFiltered++ = 0; //Add null termination to filtered line.

		if( !bFileVersionSaved && *cLineFromFile != 0 )
		{
			bFileVersionSaved = true;
			m_DataSaver.AddString( AT_DATA_FILE_VERSION );
			m_DataSaver.AddLineToLast();
		}

		// Is there main ID?
		if( strstr( cLineFromFile, MAIN_ID ) != NULL )
		{
			string sRestOfLine( cLineFromFile );
			string sTemp;

			// Delete all characters before main ID
			sRestOfLine.erase( 0, sRestOfLine.find( MAIN_ID ) );

			// Get main ID
			sTemp = GetStringUntilNextSpace( sRestOfLine );

			// Is there more data in line?
			if( sRestOfLine.empty() )
			{
				continue;
			}

			// Get next argument
			sTemp = GetStringUntilNextSpace( sRestOfLine );
			// This might be process id or error message
			if ( sTemp.compare( ERROR_OCCURED ) == 0 )
			{
				// Api mismatch between s60 side and atool.exe
				if ( sRestOfLine.find( INCORRECT_ATOOL_VERSION ) != string::npos )
				{
					cout << "Test run failed because version conflict between device binaries\nand the atool.exe version used to build the application." << endl;
					size_t pS = sRestOfLine.find_first_of('[');
					size_t pE =  sRestOfLine.find_first_of(']');
					size_t pSL = sRestOfLine.find_last_of('[');
					size_t pEL = sRestOfLine.find_last_of(']');
					if ( pS != string::npos && pE != string::npos && pSL != string::npos && pEL != string::npos )
					{
						string deviceVer = sRestOfLine.substr( pS+1, pE-pS-1 );
						string atoolVer = sRestOfLine.substr( pSL+1, pEL-pSL-1 );
						cout << "\tdevice: " << deviceVer << endl 
							<<  "\tatool.exe: " << atoolVer << endl;
					}
				}
				else
					cout << sRestOfLine << endl;
				continue;
			}
			unsigned long iProcessID = _httoi( sTemp.c_str() );

			iProcessIDinList = -1;
			// Find process from list
			for( unsigned int i = 0 ; i < vProcessList.size() ; i++ )
			{
				if( vProcessList[i].iProcessID == iProcessID )
				{
					iProcessIDinList = i;
					break;
				}
			}
			// Is Process ID found from list?
			if( iProcessIDinList == -1 )
			{
				CProcessData ProcessData;
				ProcessData.bProcessOnGoing = false;
				ProcessData.iProcessID = iProcessID;
				vProcessList.push_back( ProcessData );
				iProcessIDinList = (int)vProcessList.size() - 1;
			}

			// Remove spaces from end of line
			while( sRestOfLine[sRestOfLine.size()-1] == ' ' )
			{
				sRestOfLine.resize( sRestOfLine.size()-1 );
			}

			string sWholeTempLine( sRestOfLine );

			// Get command
			sTemp = GetStringUntilNextSpace( sRestOfLine );

			// Use c style string for easy comparisong of command.
			const char* pCommand = sTemp.c_str();		

			// Process start.
			if( ! _stricmp( pCommand, LABEL_PROCESS_START ) )
			{
				bRet = true; // Set return value true we found start.
				vProcessList[iProcessIDinList].vData.push_back( sWholeTempLine );
				vProcessList[iProcessIDinList].bProcessOnGoing = true;
				continue;
			}

			// Check is process ongoing if not skip other tags.
			if( vProcessList[iProcessIDinList].bProcessOnGoing == false )
				continue;

			// "Old style" allocation (< v.1.6)
			if( ! _stricmp( pCommand, ALLOC_ID ) )
			{
				// Add alloc
				vProcessList[iProcessIDinList].Alloc( sRestOfLine );

				// Subtests running?
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->bRunning )
					{
						// Save alloc also to sub test
						viSubTestIter->Alloc( sRestOfLine );
					}
					viSubTestIter++;
				}
			}
			else if ( ! _stricmp( pCommand, ALLOCH_ID ) )
			{
				// Add alloc
				vProcessList[iProcessIDinList].AllocH( sRestOfLine );

				// Subtests running?
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->bRunning )
					{
						// Save alloc also to sub test
						viSubTestIter->AllocH( sRestOfLine );
					}
					viSubTestIter++;
				}
			}
			// Allocation fragment (call stack).
			else if ( ! _stricmp( pCommand, ALLOCF_ID ) )
			{
				// Add alloc fragment
				vProcessList[iProcessIDinList].AllocF( sRestOfLine );
				
				// Subtests running?
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->bRunning )
					{
						// Save alloc fragment also to sub test
						viSubTestIter->AllocF( sRestOfLine );
					}
					viSubTestIter++;
				}
			}
			// Command free
			else if( ! _stricmp( pCommand, FREE_ID ) )
			{
				// Send free
				vProcessList[iProcessIDinList].Free( sRestOfLine );

				// Subtests running?
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->bRunning )
					{
						// Send free to subtest
						viSubTestIter->Free( sRestOfLine );
					}
					viSubTestIter++;
				}
			}
			// Header free.
			else if( ! _stricmp( pCommand, FREEH_ID ) )
			{
				// Send free
				vProcessList[iProcessIDinList].FreeH( sRestOfLine );

				// Subtests running?
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->bRunning )
					{
						// Send free to subtest
						viSubTestIter->FreeH( sRestOfLine );
					}
					viSubTestIter++;
				}
			
			}
			else if( ! _stricmp( pCommand, FREEF_ID ) )
			{
				// Not used currently.
			}
			// Command process end
			else if( ! _stricmp( pCommand, LABEL_PROCESS_END ) )
			{
				// Set process has ended.
				vProcessList[iProcessIDinList].bProcessOnGoing = false;

				// Save leaks
				vector<string> vLeaks;
				vector<string>::iterator viLeaks;
				vProcessList[iProcessIDinList].GetLeakList( vLeaks );
				for ( viLeaks = vLeaks.begin(); viLeaks != vLeaks.end(); viLeaks++ )
				{
					sTemp.clear();
					sTemp.append( LABEL_MEM_LEAK );
					sTemp.append( " " );
					sTemp.append( *viLeaks );
					vProcessList[iProcessIDinList].vData.push_back( sTemp );
				}
				vProcessList[iProcessIDinList].ClearAllocs();

				vector<string>::iterator viHandleIter = vProcessList[iProcessIDinList].vHandleLeaks.begin();
				// Print handle leaks
				while( viHandleIter != vProcessList[iProcessIDinList].vHandleLeaks.end() )
				{
					sTemp.clear();
					sTemp.append( viHandleIter->c_str() );
					vProcessList[iProcessIDinList].vData.push_back( sTemp );
					viHandleIter++;
				}
				// Clear handle leaks from list
				vProcessList[iProcessIDinList].vHandleLeaks.clear();

				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				// Print sub test leaks
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					// Print sub test start
					string sLine( LABEL_TEST_START ); sLine.append( " " );
					sLine.append( viSubTestIter->sStartTime ); sLine.append( " " );
					sLine.append( viSubTestIter->sSubTestName ); sLine.append( " " );
					sLine.append( viSubTestIter->sSubTestStartHandleCount );
					vProcessList[iProcessIDinList].vData.push_back( sLine );
					sLine.clear();

					// DLL Loads.
					for( vector<string>::iterator it = viSubTestIter->vData.begin();
						it != viSubTestIter->vData.end(); it++ )
					{
						vProcessList[iProcessIDinList].vData.push_back( (*it) );
					}

					// Subtest leaks.
					vector<string> vSubLeaks;
					vector<string>::iterator viSubLeaks;
					viSubTestIter->GetLeakList( vSubLeaks );
					for ( viSubLeaks = vSubLeaks.begin(); viSubLeaks != vSubLeaks.end(); viSubLeaks++ )
					{
						sLine.append( LABEL_MEM_LEAK );
						sLine.append( " " );
						sLine.append( *viSubLeaks );
						vProcessList[iProcessIDinList].vData.push_back( sLine );
						sLine.clear();
					}
					viSubTestIter->ClearAllocs();

					if( !viSubTestIter->sEndTime.empty() )
					{
						// Print sub test end
						sLine.append( LABEL_TEST_END ); sLine.append( " " );
						sLine.append( viSubTestIter->sEndTime ); sLine.append( " " );
						sLine.append( viSubTestIter->sSubTestName ); sLine.append( " " );
						sLine.append( viSubTestIter->sSubTestEndHandleCount );
						vProcessList[iProcessIDinList].vData.push_back( sLine );
					}
					viSubTestIter++;
				}

				// Clear sub tests from list
				vProcessList[iProcessIDinList].vSubTests.clear();
				vProcessList[iProcessIDinList].vData.push_back( sWholeTempLine );
			}
			else if( ! _stricmp( pCommand, LABEL_HANDLE_LEAK ) )
			{
				// Make whole line
				sTemp.append( " " );
				sTemp.append( sRestOfLine );
				vProcessList[iProcessIDinList].vHandleLeaks.push_back( sTemp );
			}
			else if( ! _stricmp( pCommand, LABEL_DLL_LOAD ) )
			{
				// Add module load to process data.
				vProcessList[iProcessIDinList].vData.push_back( sWholeTempLine );
				// Add module load to subtest data if test running.
				for( vector<CSubTestData>::iterator it = vProcessList[iProcessIDinList].vSubTests.begin();
					it != vProcessList[iProcessIDinList].vSubTests.end(); it++ )
				{
					if( it->bRunning )
						it->vData.push_back( sWholeTempLine );
				}

			}
			else if( ! _stricmp( pCommand, LABEL_DLL_UNLOAD ) )
			{
				// Add module load to process data.
				vProcessList[iProcessIDinList].vData.push_back( sWholeTempLine );
				// Add module unload to subtest data if test running.
				for( vector<CSubTestData>::iterator it = vProcessList[iProcessIDinList].vSubTests.begin();
					it != vProcessList[iProcessIDinList].vSubTests.end(); it++ )
				{
					if( it->bRunning )
						it->vData.push_back( sWholeTempLine );
				}
			}
			else if( sTemp.find( LABEL_LOGGING_CANCELLED ) != string::npos ||
				     sTemp.find( LABEL_PROCESS_END ) != string::npos || sTemp.find( LABEL_ERROR_OCCURED ) != string::npos ||
					 sTemp.find( LABEL_HANDLE_LEAK ) != string::npos )
			{
				vProcessList[iProcessIDinList].vData.push_back( sWholeTempLine );
			}
			else if( ! _stricmp( pCommand, LABEL_TEST_START ) )
			{
				bRet = true; // Set return value true we found start.
				// Get sub test time
				string sSubTestTime = GetStringUntilNextSpace( sRestOfLine );
				// Get sub test name
				string sSubTestName = GetStringUntilNextSpace( sRestOfLine );
				// Get sub test start handle count
				string sSubTestStartHandleCount = GetStringUntilNextSpace( sRestOfLine );

				CSubTestData SubTestData;
				SubTestData.bRunning = true;
				SubTestData.sStartTime = sSubTestTime;
				SubTestData.sSubTestName = sSubTestName;
				SubTestData.sSubTestStartHandleCount = sSubTestStartHandleCount.c_str();

				vProcessList[iProcessIDinList].vSubTests.push_back( SubTestData );
			}
			else if( ! _stricmp( pCommand, LABEL_TEST_END ) )
			{
				// Get sub test time
				string sSubTestEnd = GetStringUntilNextSpace( sRestOfLine );
				// Get sub test name
				string sSubTestName = GetStringUntilNextSpace( sRestOfLine );
				// Get sub test end handle count
				string sSubTestEndHandleCount = GetStringUntilNextSpace( sRestOfLine );

				// Find subtest
				vector<CSubTestData>::iterator viSubTestIter = vProcessList[iProcessIDinList].vSubTests.begin();
				while( viSubTestIter != vProcessList[iProcessIDinList].vSubTests.end() )
				{
					if( viSubTestIter->sSubTestName == sSubTestName && viSubTestIter->sEndTime.empty() )
					{
						viSubTestIter->sEndTime = sSubTestEnd;
						viSubTestIter->bRunning = false;
						viSubTestIter->sSubTestEndHandleCount = sSubTestEndHandleCount.c_str();
					}
					viSubTestIter++;
				}
			}
		}
	}

	// Print all saved data from processes
	for( unsigned int i = 0 ; i < vProcessList.size() ; i++ )
	{
		// Print saved lines
		for( unsigned int iDataCounter = 0 ; iDataCounter < vProcessList[i].vData.size() ; iDataCounter++ )
		{
			m_DataSaver.AddString( vProcessList[i].vData[iDataCounter].c_str() );
			m_DataSaver.AddLineToLast();
		}

		string sTemp;

		// Save leaks
		vector<string> vLeaks;
		vector<string>::iterator viLeaks;
		vProcessList[i].GetLeakList( vLeaks );
		for ( viLeaks = vLeaks.begin(); viLeaks != vLeaks.end(); viLeaks++ )
		{
			sTemp.clear();
			sTemp.append( LABEL_MEM_LEAK );
			sTemp.append( " " );
			sTemp.append( *viLeaks );
			m_DataSaver.AddString( sTemp.c_str() );
			m_DataSaver.AddLineToLast();
		}

		vector<string>::iterator viHandleIter = vProcessList[i].vHandleLeaks.begin();
		// Print handle leaks, if there is data left, there was no process end.
		while( viHandleIter != vProcessList[i].vHandleLeaks.end() )
		{
			sTemp.clear();
			sTemp.append( viHandleIter->c_str() );
			m_DataSaver.AddString( sTemp.c_str() );
			m_DataSaver.AddLineToLast();
			viHandleIter++;
		}
		vector<CSubTestData>::iterator viSubTestIter = vProcessList[i].vSubTests.begin();
		// Print sub test data, if there is data left, there was no process end.
		while( viSubTestIter != vProcessList[i].vSubTests.end() )
		{
			// Print sub test start
			string sLine( LABEL_TEST_START ); sLine.append( " " );
			sLine.append( viSubTestIter->sStartTime ); sLine.append( " " );
			sLine.append( viSubTestIter->sSubTestName ); sLine.append( " " );
			sLine.append( viSubTestIter->sSubTestStartHandleCount );
			m_DataSaver.AddString( sLine.c_str() );
			m_DataSaver.AddLineToLast();
			sLine.clear();

			// DLL Loads.
			for( vector<string>::iterator it = viSubTestIter->vData.begin();
				it != viSubTestIter->vData.end(); it++ )
			{
				m_DataSaver.AddString( (*it).c_str() );
				m_DataSaver.AddLineToLast();
			}

			// Subtest leaks.
			vector<string> vSubLeaks;
			vector<string>::iterator viSubLeaks;
			viSubTestIter->GetLeakList( vSubLeaks );
			for ( viSubLeaks = vSubLeaks.begin(); viSubLeaks != vSubLeaks.end(); viSubLeaks++ )
			{
				sLine.append( LABEL_MEM_LEAK );
				sLine.append( " " );
				sLine.append( *viSubLeaks );
				m_DataSaver.AddString( sLine.c_str() );
				m_DataSaver.AddLineToLast();
				sLine.clear();
			}

			// Print sub test end
			sLine.append( LABEL_TEST_END ); sLine.append( " " );
			sLine.append( viSubTestIter->sEndTime ); sLine.append( " " );
			sLine.append( viSubTestIter->sSubTestName ); sLine.append( " " );
			sLine.append( viSubTestIter->sSubTestEndHandleCount );
			m_DataSaver.AddString( sLine.c_str() );
			m_DataSaver.AddLineToLast();

			viSubTestIter++;
		}
	}
	// Save lines to file.
	m_DataSaver.SaveLinesToFile( pOutputFileName, TEXT_DATA );
	// Close file.
	in.close();
	return bRet;
}

// -----------------------------------------------------------------------------
// CATParseTraceFile::GetDataSaver
// Gets data saver object.
// -----------------------------------------------------------------------------
CATDataSaver* CATParseTraceFile::GetDataSaver(void)	
{
	LOG_LOW_FUNC_ENTRY("CATParseTraceFile::GetDataSaver");
	return &m_DataSaver;
}
//EOF
