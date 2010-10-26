/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
* Modifier properties is returned by one of the TraceObjectUtils.modify methods
*
*/

package com.nokia.tracecompiler.engine.source;


import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.*;
import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import com.nokia.tracecompiler.engine.project.TraceIDCache;
import com.nokia.tracecompiler.engine.rules.osttrace.OstTraceFormatRule;
import com.nokia.tracecompiler.engine.source.SourceEngineTest.MockSourceListener.SourceStatus;
import com.nokia.tracecompiler.model.TraceCompilerException;
import com.nokia.tracecompiler.model.TraceModel;
import com.nokia.tracecompiler.project.GroupNames;
import com.nokia.tracecompiler.source.SourcePropertyProvider;
import com.nokia.tracecompiler.utils.DocumentAdapter;
import com.nokia.tracecompiler.utils.DocumentFactory;

public class SourceEngineTest {

	
	class MockSourcePropertyProvider extends DocumentAdapter  
		implements SourcePropertyProvider
	{

		String filename;
		// ctor takes full filename and path as 
		public MockSourcePropertyProvider(String filename){
			this.filename = filename;
		}
		public String getFileName(){
			return new File(filename).getName();			
			
		}
		public String getFilePath(){
			return new File(filename).getPath();			
		}
	}


	
	class MockSourceListener implements SourceListener{
		
		// mock listener will receive callbacks on source opened
		// we can then check that the correct files have been opened 
		// and the others HAVE NOT  !
		
		class SourceStatus
		{
			public String filename;
			public boolean inList = false;
			public boolean fileExists = false;
			//public SourceProperties properties; // TODO we may use this later to check parser			
		}
		
		private Vector<SourceStatus> sourceList = new Vector<SourceStatus>();
		
		public void sourceOpened(SourceProperties properties) throws TraceCompilerException {
			String filename = properties.getFileName(); // gets the filename without path
			
			SourceStatus ss = new SourceStatus();

			if( filename != null){
				ss.filename = testDataDir + filename;								
				ss.fileExists = new File(ss.filename).exists();		
				for( int i=0;i<files.length;i++){
					if(ss.filename.compareTo(files[i]) == 0){
						// found filename in list
						ss.inList = true;
						break;
					}
				}
			}
			sourceList.add(ss);
		}
		
		public Vector<SourceStatus> getSourceList(){
			return sourceList;		
		}
		
		public boolean verifySourceList(){
			
			boolean ret = true;
			// source list should contain the files that do exist
			for(SourceStatus ss : sourceList){
				if(ss.filename == null){
					System.out.println("ERROR: Found a null filename");
					ret = false;					
				}else{					
					if(! ss.fileExists ){
						System.out.println("ERROR:"+ss.filename + " file does not exist");
						ret = false;
					}
					
					if(!ss.inList){
						System.out.println("ERROR: recieved notification for file "+ss.filename +" that was NOT in list");
						ret = false;						
					}
					// we could break here if ret == false - but I want error  printouts for all list items				
					
				}				
				
			}			
			return ret;			
		}		
	}

	
	static SourceEngine engine = null;	
	static MockSourceListener mockListener;
	static MockSourcePropertyProvider mockProvider;
	static String files[] = null;	
	static final String names[] = {  "BTraceCategoryHandlerTraces.h", "OstTraceDefinitions.h", "foo.cpp", "bar.cpp", "foo.h"
						 , "not_exist.cpp", "not_exist.h", "fixed_id.definitions", "wrong.ext"};
	
	
	static String testDataDir = null;
	static String nonFile = "this file does not exist";
	static String testFile = "data.txt";
	
	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		
		GroupNames.initialiseGroupName();  // 
		TraceCompilerEngineGlobals.start();
		String epocroot = System.getenv("EPOCROOT");
		// need to check that the path ends in a backslash
		if(!epocroot.endsWith("\\")){ 
			epocroot += "\\"; 
		}
		
		final int LEN = names.length;
		files = new String[LEN];

		testDataDir = epocroot + "testdata\\SourceEngineTest\\";		
		for(int i=0;i<LEN;i++){
			files[i] = testDataDir + names[i];
		}
		
		testFile = testDataDir + testFile;
		
		mockListener =  new SourceEngineTest().new MockSourceListener();
		mockProvider = new SourceEngineTest().new MockSourcePropertyProvider(testFile);
		
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();		
		model.addExtension(new TraceIDCache(testDataDir));
		model.addExtension(new OstTraceFormatRule());
		
		FileDocumentMonitor fileDocumentMonitor = new FileDocumentMonitor();
		DocumentFactory.registerDocumentFramework(fileDocumentMonitor, StringDocumentFactory.class);
		
		engine = TraceCompilerEngineGlobals.getSourceEngine();
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testSourceEngine() {
		
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();
		SourceEngine engine = new SourceEngine(model);		
		assertNotNull(engine != null);		
	}

	@Test
	public void testAddSourceListener() {		
		// TODO - engine allows us to add null listeners -fix as this causes exception when accessed
		//engine.addSourceListener(null);		
		engine.addSourceListener(mockListener);		
	}

	@Test
	public void testStart() throws Exception{	

		// note the testAddSourceListener must be run or mockListener must be added as a sourceListener first 
		mockListener.getSourceList().clear();
		
		FileDocumentMonitor.setFiles(files);
		// TODO - calling engine.start() after FileDocumentMonitor.setFiles(null); - causes crash - fix
	
		engine.start();
		// files should have been processed by mockListener - check the list
		assertTrue(mockListener.verifySourceList());
	}



	@Test
	public void testGetSources() {		
		
		/* 
		TODO we are not getting any sources here DocumentMonitorBase.sources only appears to be cleared
		data comes from DocumentMonitorBase.sources - which has no member to add any
		so sources probably could be deleted - or fixed to return the correct data
		*/				
		Iterator<SourceProperties> propsList = engine.getSources();		
		assertFalse(propsList.hasNext());		
	}

	@Test
	public void testIterator() {
		// see testGetSources
	}

	@Test
	public void testAddNonSourceFile() throws Exception{
		
		/* TODO - nonSource files and source files actually go to the same place in SourceEngine
		* as this code adds the files to the same list as SourceEngine.addNonSourceFile
		 * while the sources (DocumentMonitorBase.sources) see above test 
		 * is never used
		FileDocumentMonitor.setFiles(files);
		FileDocumentMonitor fdm= new FileDocumentMonitor();
		DocumentFactory.registerDocumentFramework(fdm, StringDocumentFactory.class);
		*/
		ArrayList<String> arr = engine.getNonSourceFiles();		
		final int originalSize = arr.size();
		
		// add an existing file - not already added
		engine.addNonSourceFile(testFile);		
		assertEquals( originalSize +1 , arr.size()); 
		assertTrue(testFile.equals(arr.get(arr.size() - 1)));
				
		// add a duplicate file - engine does not check for duplicates
		engine.addNonSourceFile(files[0]);		
		assertEquals( originalSize + 2 , arr.size()); 
		assertTrue(files[0].equals(arr.get(arr.size() - 1)));
				
		// TODO - fix engine allows us to add null 
		engine.addNonSourceFile(null);
		assertEquals( originalSize + 3 , arr.size());
		assertNull(arr.get(arr.size() - 1));
		
		// TODO - also allows addition of non existient files
		engine.addNonSourceFile(nonFile);
		assertEquals( originalSize + 4 , arr.size());
		assertTrue(nonFile.equals(arr.get(arr.size() - 1)));				
	}

	@Test
	public void testRemoveNonSourceFile() {
		
		
		ArrayList<String> arr = engine.getNonSourceFiles();		
		final int originalSize = arr.size();
		
		// add an existing file - not already added
		assertTrue(arr.contains(testFile));
		engine.removeNonSourceFile(testFile);
		assertFalse(arr.contains(testFile));		
		assertEquals( originalSize -1 , arr.size()); 

		// add an existing file - not already added
		assertTrue(arr.contains(nonFile));
		engine.removeNonSourceFile(nonFile);
		assertFalse(arr.contains(nonFile));		
		assertEquals( originalSize - 2 , arr.size()); 
				
		// remove a duplicate file - there should be 2 instances of files[0] in list 
		assertTrue(arr.contains(files[0]));
		engine.removeNonSourceFile(files[0]);
		assertTrue(arr.contains(files[0]));  // TODO - remove only removes 1 occurrence - 1 copy still exists 		
		assertEquals( originalSize - 3 , arr.size());  // only 1 item removed
		
		// TODO operations on null are allowed - fix 
		assertTrue(arr.contains(null));
		engine.removeNonSourceFile(null);
		assertFalse(arr.contains(null));  		
		assertEquals( originalSize - 4 , arr.size()); 
						
	}

	@Test
	public void testGetNonSourceFiles() {
		// see testAddNonSourceFile and testRemoveNonSourceFile
		assertNotNull(engine.getNonSourceFiles());				
	}

	@Test
	public void testSourceOpened() throws TraceCompilerException{		
		
		mockListener.getSourceList().clear();
		
		// method SourceEngine is a call back - we can however check that this notifys the listener
		// IN this case should have recieved a single callback - adding a null file
		// that does not exist
		engine.sourceOpened( mockProvider);
		Vector<SourceStatus> sl = mockListener.getSourceList();	
		assertEquals(sl.size(), 1);
			
	}

	@Test
	public void testRemoveSourceListener() throws TraceCompilerException {
		
		engine.removeSourceListener(mockListener);
		mockListener.getSourceList().clear();
		
		engine.sourceOpened( mockProvider);
		Vector<SourceStatus> sl = mockListener.getSourceList();	
		assertEquals(sl.size(), 0);
	}
	
	@Test
	public void testShutdown() {
		// because junit executes tests in order they appear in source file - this must be last
		engine.shutdown();
	}

}