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
 *
 */

package com.nokia.tracecompiler.engine.source;

import static org.junit.Assert.*;

import java.io.File;
import java.util.Iterator;
import java.util.Vector;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.FileDocumentMonitor;
import com.nokia.tracecompiler.document.StringDocumentFactory;
import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import com.nokia.tracecompiler.engine.TraceLocation;
import com.nokia.tracecompiler.engine.project.TraceIDCache;
import com.nokia.tracecompiler.engine.rules.osttrace.OstTraceFormatRule;
import com.nokia.tracecompiler.engine.source.SourceEngineTest.MockSourcePropertyProvider;
import com.nokia.tracecompiler.model.TraceCompilerException;
import com.nokia.tracecompiler.model.TraceModel;
import com.nokia.tracecompiler.project.GroupNames;
import com.nokia.tracecompiler.source.SourceDocumentInterface;
import com.nokia.tracecompiler.source.SourceDocumentMonitor;
import com.nokia.tracecompiler.source.SourceParser;
import com.nokia.tracecompiler.source.SourceParserException;
import com.nokia.tracecompiler.utils.DocumentFactory;

public class SourcePropertiesTest {

	private final class MockListener implements SourceListener {
		// listener will keep track of source properties from the SourceEngine
		public Vector<SourceProperties> propertiesList = new Vector<SourceProperties>();

		
		public void sourceOpened(SourceProperties properties)
				throws TraceCompilerException {
			this.propertiesList.add(properties);
		}

	}

	static MockListener mockListener;
	static String[] files;  		// array of filenames
	static boolean[] fileRwFlags;  	// array of properties for files

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TraceCompilerEngineGlobals.start();
		GroupNames.initialiseGroupName(); //
		TraceCompilerEngineGlobals.start();
		String epocroot = System.getenv("EPOCROOT");
		// need to check that the path ends in a backslash
		if (!epocroot.endsWith("\\")) {
			epocroot += "\\";
		}

		String testDataDir = epocroot + "testdata\\SourceEngineTest\\";
		testDataDir = testDataDir.replace('\\', '/');
		
		// just setting up with 2 files here
		final int LEN = 2;		
		files = new String[LEN];
		files[0] = testDataDir + "foo.cpp";
		files[1] = testDataDir + "foo.h";
		fileRwFlags = new boolean[LEN];
		
		for(int i=0;i<LEN;i++){
			fileRwFlags[i] =  files[i].endsWith(".h"); 			
		}
				
		mockListener = new SourcePropertiesTest().new MockListener();

		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();

		model.addExtension(new TraceIDCache(testDataDir));
		model.addExtension(new OstTraceFormatRule());

		FileDocumentMonitor fileDocumentMonitor = new FileDocumentMonitor();
		FileDocumentMonitor.setFiles(files);
		DocumentFactory.registerDocumentFramework(fileDocumentMonitor,
				StringDocumentFactory.class);

		// set the mockListener to the engine - the 'real' SourceProperties instances
		// will then be collected by the listener
		SourceEngine engine = TraceCompilerEngineGlobals.getSourceEngine();
		engine.addSourceListener(mockListener);
		engine.start();
	}

	@Test
	public void testSourceProperties() throws Exception {
		SourceDocumentMonitor documentMonitor = DocumentFactory
				.getDocumentMonitor();
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();
		MockSourcePropertyProvider mockProvider 
			= new SourceEngineTest().new MockSourcePropertyProvider(files[0]);

		SourceProperties properties = new SourceProperties(model,
				documentMonitor.getFactory(), mockProvider);
		assertNotNull(properties);

		// TODO try with some null parameters - these ALL crash at the moment
		// properties = new SourceProperties(null,
		// documentMonitor.getFactory(), mockProvider);
		// assertNotNull(properties);

		// properties = new SourceProperties(model, null, mockProvider);
		// assertNotNull(properties);

		// properties = new SourceProperties(model,
		// documentMonitor.getFactory(), null);
		// assertNotNull(properties);

		// properties = new SourceProperties(null, null, null);
		// assertNotNull(properties);
	}

	@Test
	public void testGetSourceParser() throws SourceParserException {
		assertTrue(mockListener.propertiesList.size() > 0);
		for (SourceProperties p : mockListener.propertiesList) {
			SourceParser parser = p.getSourceParser();
			assertNotNull(parser);
			// quick sanity check on the parser
			SourceDocumentInterface sdi = parser.getSource();
			assertNotNull(sdi);
		}
	}

	@Test
	public void testIterator() throws Exception {
		assertTrue(mockListener.propertiesList.size() > 0);
		for (SourceProperties p : mockListener.propertiesList) {
			Iterator<TraceLocation> it = p.iterator();
			while (it.hasNext()) {

				// TODO do some checks on TraceLocation - move this to TraceLocation tests
				// just print out the trace location data here
				TraceLocation tl = it.next();
				String filename = tl.getFilePath() + tl.getFileName();
				System.out.println(filename);
				System.out.println("tl.getClassName()");
				System.out.println(tl.getClassName());
				System.out.println("tl.getLineNumber()");
				System.out.println(tl.getLineNumber());
				System.out.println("tl.getFunctionName()");
				System.out.println(tl.getFunctionName());
				System.out.println("tl.getOriginalName()");
				System.out.println(tl.getOriginalName());
				System.out.println("tl.getConvertedName()");
				System.out.println(tl.getConvertedName());
				Iterator<String> params = tl.getParameters();
				int n=0;
				while(params.hasNext()){
					System.out.println("param: " + n++ +' '+ params.next());					
				}				
				assertNotNull(filename);
				assertTrue(filename.equals(files[0]));				
				
			}
		}
	}

	@Test
	public void testGetFileName() {
		int n = 0;
		for (SourceProperties p : mockListener.propertiesList) {
			String filename = p.getFileName();
			assertNotNull(filename);
			assertTrue(filename.equals(new File(files[n++]).getName()));
		}
		assertEquals(n, files.length);
	}

	@Test
	public void testIsReadOnly() {
		int n=0;
		for (SourceProperties p : mockListener.propertiesList) {
			boolean readOnly = p.isReadOnly();
			assertEquals(readOnly, fileRwFlags[n++]);			
		}
		assertEquals(n, fileRwFlags.length);
	}
}
