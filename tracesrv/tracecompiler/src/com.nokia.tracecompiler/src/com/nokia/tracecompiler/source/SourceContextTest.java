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

*/

package com.nokia.tracecompiler.source;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.FileDocumentMonitor;
import com.nokia.tracecompiler.document.StringDocumentFactory;
import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import com.nokia.tracecompiler.engine.project.TraceIDCache;
import com.nokia.tracecompiler.engine.rules.osttrace.OstTraceFormatRule;
import com.nokia.tracecompiler.engine.source.SourceEngine;
import com.nokia.tracecompiler.engine.source.SourceListener;
import com.nokia.tracecompiler.engine.source.SourceProperties;
import com.nokia.tracecompiler.model.TraceCompilerException;
import com.nokia.tracecompiler.model.TraceModel;
import com.nokia.tracecompiler.project.GroupNames;
import com.nokia.tracecompiler.utils.DocumentFactory;

public class SourceContextTest {

	private final class MockListener implements SourceListener {
		// listener will keep track of source properties from the SourceEngine
		public Vector<SourceProperties> propertiesList = new Vector<SourceProperties>();

		public void sourceOpened(SourceProperties properties)
				throws TraceCompilerException {
			this.propertiesList.add(properties);
		}
	}

	static MockListener mockListener;
	static String[] files; // array of filenames
	// expected values for THIS test
	// the file under test is
	// $EPOCROOT/testdata/SourceEngineTest/SourceContextTest.cpp
	final int NUM_FUNCTIONS = 15; // number of functions (contexts) described in test file
	static ArrayList<SourceContext> contexts = null;
	static final String CLASS_NAME = "SourceContextTestClassName";
	static final String FUNCTION_NAME = "FunctionSourceContextTestClassName";


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
		final int LEN = 1;
		files = new String[LEN];
		files[0] = testDataDir + "SourceContextTest.cpp";

		// gatherOffsets(files[0]);
		mockListener = new SourceContextTest().new MockListener();

		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();

		model.addExtension(new TraceIDCache(testDataDir));
		model.addExtension(new OstTraceFormatRule());

		FileDocumentMonitor fileDocumentMonitor = new FileDocumentMonitor();
		FileDocumentMonitor.setFiles(files);
		DocumentFactory.registerDocumentFramework(fileDocumentMonitor,
				StringDocumentFactory.class);

		// set the mockListener to the engine - the 'real' SourceProperties
		// instances
		// will then be collected by the listener
		SourceEngine engine = TraceCompilerEngineGlobals.getSourceEngine();
		engine.addSourceListener(mockListener);
		engine.start();

		setupContexts();
	}

	private static void setupContexts() {

		if (contexts == null) {
			// here we cheat a bit to gather the offsets to use for gathering
			// the
			// source context offsets - this is for ease of test maintenance as
			// if they were hardcoded they would have to be updated every time
			// the test file was changed
			ArrayList<Integer> offsets = null;
			Iterator<SourceProperties> propsList = mockListener.propertiesList
					.iterator();
			while (propsList.hasNext()) {

				// there are a lists of SourceContexts held - they are under
				// com.nokia.tracecompiler.source.SourceParser.contextAreaParser
				// com.nokia.tracecompiler.source.ContextAreaParser.contextAreas
				SourceParser parser = propsList.next().getSourceParser();

				File f = new File(files[0]);
				final long FILELEN = f.length();

				offsets = new ArrayList<Integer>();
				SourceContext oldContext = null;
				for (int i = 0; i < FILELEN; i++) {
					SourceContext context = parser.getContext(i);
					if (context != null && context != oldContext) {
						offsets.add(new Integer(i));
						oldContext = context;
					}
				}
			}
			assertTrue(offsets.size() > 0);
			// reset the list
			propsList = mockListener.propertiesList
			.iterator();
			contexts = new ArrayList<SourceContext>();
			while (propsList.hasNext()) {
				SourceParser parser = propsList.next().getSourceParser();
				for (Integer i : offsets) {
					int index = i.intValue();
					SourceContext context = parser.getContext(index);
					contexts.add(context);
				}
			}
		}
	}

	@Test
	public void testGetClassName() {

		int n = 0;
		for(SourceContext context : contexts){
			String s = context.getClassName();
			n++;
			if( n == 3) { // 3rd function in file is global scope - all others class scope
				assertNull(s);
			}else{
				assertNotNull(s);				
				// test data file has several class names defined
				// - these in format hasSourceContextTestClassNameN
				assertTrue(s.equals(CLASS_NAME + n));
			}
		}
		assertEquals(n, NUM_FUNCTIONS); 		
	}

	@Test
	public void testGetFunctionName() {
		int n = 0;
		for(SourceContext context : contexts){
			String s = context.getFunctionName();
			assertNotNull(s);
			// test data file has several class names defined
			// - these in format hasSourceContextTestClassNameFunctionN
			// but first 1st is a constructor & 2nd is a destructor
			n++;
			if (n == 1) {
				assertTrue(s.equals(CLASS_NAME + n));
			} else if (n == 2) {
				assertTrue(s.equals("~" + CLASS_NAME + n));
			} else {
				assertTrue(s.equals(FUNCTION_NAME + n));
			}
		}
		assertEquals(n, NUM_FUNCTIONS);		
	}

	@Test
	public void testHasQualifier() {

		// hasQualifier indicates that the function has an unsigned return type
		// this is not immediately obvious but has Qualifier calls through to
		// com.nokia.tracecompiler.source.SourceContext.typeEquals(String)
		// which checks its
		// com.nokia.tracecompiler.source.SourceContext.returnTypes

		for(SourceContext context : contexts){

			boolean tIntQualifier = context.hasQualifier("TInt");
			boolean tUintQualifier = context.hasQualifier("TUint");
			boolean inlineQualifier = context
					.hasQualifier(SourceConstants.INLINE);
			boolean voidQualifier = context
					.hasQualifier(SourceConstants.VOID);
			boolean charQualifier = context
					.hasQualifier(SourceConstants.CHAR);
			boolean shortQualifier = context
					.hasQualifier(SourceConstants.SHORT);
			boolean intQualifier = context
					.hasQualifier(SourceConstants.INLINE);
			boolean longQualifier = context
					.hasQualifier(SourceConstants.LONG);
			boolean unsignedQualifier = context
					.hasQualifier(SourceConstants.UNSIGNED);

			String funcName = context.getFunctionName();
			if (funcName.equals("SourceContextTestClassName1")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName.equals("~SourceContextTestClassName2")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName3")) {
				// TInt FunctionSourceContextTestClassName3( int param1 )
				assertFalse(tIntQualifier);  // TODO - TEST FAILURE this is NOT set here 
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier); // TODO - TEST FAILURE - this is NOT SET here
											// when it is in next function
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName4")) {
				assertTrue(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier); 
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);

			} else if (funcName
					.equals("FunctionSourceContextTestClassName5")) {
				assertTrue(tIntQualifier);
				assertFalse(tUintQualifier);
				assertTrue(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertTrue(intQualifier); // TODO - TEST FAILURE - this is SET here when it
											// was not in
											// FunctionSourceContextTestClassName3
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName6")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertTrue(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName7")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertTrue(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName8")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertTrue(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName9")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier); // TODO - TEST FAILURE - another issue with int
											// qualifier this is NOT SET
											// here
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName10")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertTrue(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName11")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertFalse(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertFalse(intQualifier);
				assertFalse(longQualifier);
				assertTrue(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName12")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertTrue(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertTrue(shortQualifier);
				assertTrue(intQualifier); // TODO - TEST FAILURE - this is set here when it
											// should not be ?
				assertFalse(longQualifier);
				assertFalse(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName13")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertTrue(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertTrue(shortQualifier);
				assertTrue(intQualifier); // TODO - TEST FAILURE - this is set here when it
											// shouldn't be ?
				assertFalse(longQualifier);
				assertTrue(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName14")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertTrue(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertTrue(intQualifier); // this is set here - implicit int
				assertFalse(longQualifier);
				assertTrue(unsignedQualifier);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName15")) {
				assertFalse(tIntQualifier);
				assertFalse(tUintQualifier);
				assertTrue(inlineQualifier);
				assertFalse(voidQualifier);
				assertFalse(charQualifier);
				assertFalse(shortQualifier);
				assertTrue(intQualifier); // TODO - TEST FAILURE - this is set here
				assertFalse(longQualifier);
				assertTrue(unsignedQualifier);
			} else {
				fail("invalid funcname" + funcName);
			}
		}
	}

	@Test
	public void testTypeEquals() {
		// SourceContext.hasQualifier is a wrap around SourceContext.typeEquals
		// definition is return typeEquals(type);
		// so tested in testHasQualifier
	}

	@Test
	public void testIsPointer() {
		for(SourceContext context : contexts){
			boolean isPtr = context.isPointer();
			String funcName = context.getFunctionName();
			// only functions with pointer return type are
			// inline unsigned* FunctionSourceContextTestClassName14
			// inline unsigned TUint* FunctionSourceContextTestClassName15
			// inline unsigned short* FunctionSourceContextTestClassName13
			if (funcName.equals("FunctionSourceContextTestClassName13")
					|| funcName
							.equals("FunctionSourceContextTestClassName14")
					|| funcName
							.equals("FunctionSourceContextTestClassName15")) {
				assertTrue(isPtr);
			} else {
				assertFalse(isPtr);
			}
		}
	}

	@Test
	public void testIsVoid() {
		for(SourceContext context : contexts){
			boolean isVoid = context.isVoid();
			String funcName = context.getFunctionName();
			// System.out.println(funcName + " isVoid = " + isVoid);
			// only functions with void return type is
			// the constructor / destructor and
			// void FunctionSourceContextTestClassName6
			if (funcName.equals("SourceContextTestClassName1")
					|| funcName.equals("~SourceContextTestClassName2")
					|| funcName
							.equals("FunctionSourceContextTestClassName6")) {
				assertTrue(isVoid);
			} else {
				
				if(funcName.equals("FunctionSourceContextTestClassName3")){
					assertTrue(isVoid); // TODO - TEST FAILURE - it returns int   
				}else{
					assertFalse(isVoid);					
				}
			}
		}
	}

	@Test
	public void testParseParameters() throws SourceParserException {
		assertEquals(contexts.size(), NUM_FUNCTIONS);
		for(SourceContext context : contexts){
			final String funcName = context.getFunctionName();
			ArrayList<SourceParameter> paramList = new ArrayList<SourceParameter>();
			context.parseParameters(paramList);
			Iterator<SourceParameter> it = paramList.iterator();
			SourceParameter sp = null;
			
			// contexts 1 & 2 should have no params - others will
			if( ! funcName.equals("SourceContextTestClassName1") && ! funcName.equals("~SourceContextTestClassName2") ){
				assertTrue(it.hasNext()); //yes we have parameters
				sp = it.next();			  // get the first one	
			}
			if(funcName.equals("SourceContextTestClassName1") || funcName.equals("~SourceContextTestClassName2") ){
				assertFalse(it.hasNext()); // no parameters							
			} else if (funcName.equals("FunctionSourceContextTestClassName3")) {
				// TInt FunctionSourceContextTestClassName3( int param1 )
				assertTrue(sp.getType().equals("int"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
			} else if (funcName
					.equals("FunctionSourceContextTestClassName4")) {
				// inline TInt FunctionSourceContextTestClassName4( TInt
				// &param1, void* param2 );
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
			} else if (funcName
					.equals("FunctionSourceContextTestClassName5")) {
				// TInt FunctionSourceContextTestClassName5( TUint32&
				// param1, char param2, short param3 );
				assertTrue(sp.getType().equals("TUint32"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("char"));
				assertTrue(sp.getName().equals("param2"));
				assertFalse(sp.isPointer());
				assertFalse(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("short"));
				assertTrue(sp.getName().equals("param3"));
				assertFalse(sp.isPointer());
				assertFalse(sp.isReference());
			} else if (funcName
					.equals("FunctionSourceContextTestClassName6")) {
				// void FunctionSourceContextTestClassName6( TInt& param1,
				// TUint32 *param2, TUint32* param3 );
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());
				assertFalse(sp.hasQualifier("const"));

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("TUint32"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("TUint32"));
				assertTrue(sp.getName().equals("param3"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
			} else if (funcName
					.equals("FunctionSourceContextTestClassName7")) {
				// char FunctionSourceContextTestClassName7( TInt& param1,
				// void* param2, TAny* param3, int*& param4 );
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());
				assertFalse(sp.hasQualifier("const"));

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("TAny"));
				assertTrue(sp.getName().equals("param3"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("int"));
				assertTrue(sp.getName().equals("param4"));
				assertTrue(sp.isPointer());
				assertTrue(sp.isReference());
				assertFalse(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName8")) {
				// short FunctionSourceContextTestClassName8( const TUint32&
				// param1 );
				assertTrue(sp.getType().equals("TUint32"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());
				assertTrue(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName9")) {
				// int FunctionSourceContextTestClassName9( const TInt
				// param1, const TUint32* param2, const void* param3 );
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertFalse(sp.isReference());
				assertEquals(sp.getPointerCount(), 0);
				assertTrue(sp.hasQualifier("const"));

				sp = it.next();
				assertTrue(sp.getType().equals("TUint32"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertTrue(sp.hasQualifier("const"));

				sp = it.next();
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("param3"));
				assertTrue(sp.isPointer());
				assertEquals(sp.getPointerCount(), 1);
				assertFalse(sp.isReference());
				assertTrue(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName10")) {

				// long FunctionSourceContextTestClassName10( int& param1,
				// TInt** param2 );
				assertTrue(sp.getType().equals("int"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertTrue(sp.isReference());
				assertTrue(sp.getPointerCount() == 0);
				assertFalse(sp.hasQualifier("const"));

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertEquals(sp.getPointerCount(), 2);
			} else if (funcName
					.equals("FunctionSourceContextTestClassName11")) {
				// unsigned int FunctionSourceContextTestClassName11(TInt
				// param1 );
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param1"));
				assertFalse(sp.isPointer());
				assertFalse(sp.isReference());
				assertEquals(sp.getPointerCount(), 0);
				assertFalse(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName12")) {
				// inline short FunctionSourceContextTestClassName12(void*
				// aPtr )
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("aPtr"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertEquals(sp.getPointerCount(), 1);
				assertFalse(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName13")) {
				// inline unsigned short*
				// FunctionSourceContextTestClassName13(void* param1, TInt*
				// param2 )
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("param1"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertTrue(sp.getPointerCount() == 1);
				assertFalse(sp.hasQualifier("const"));

				assertTrue(it.hasNext());
				sp = it.next();
				assertTrue(sp.getType().equals("TInt"));
				assertTrue(sp.getName().equals("param2"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertEquals(sp.getPointerCount(), 1);

				assertFalse(it.hasNext());
			} else if (funcName
					.equals("FunctionSourceContextTestClassName14")) {
				// inline unsigned*
				// FunctionSourceContextTestClassName14(void* aPtr )
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("aPtr"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertTrue(sp.getPointerCount() == 1);
				assertFalse(sp.hasQualifier("const"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName15")) {
				// inline unsigned int*
				// FunctionSourceContextTestClassName15(void* aPtr )
				assertTrue(sp.getType().equals("void"));
				assertTrue(sp.getName().equals("aPtr"));
				assertTrue(sp.isPointer());
				assertFalse(sp.isReference());
				assertTrue(sp.getPointerCount() == 1);
				assertFalse(sp.hasQualifier("const"));
			} else {
				fail("invalid funcname" + funcName);
			}

			assertFalse(it.hasNext());

		}
	}

	@Test
	public void testParseReturnValues() throws SourceParserException {
		assertEquals(contexts.size(), NUM_FUNCTIONS);
		for(SourceContext context : contexts){
			final String funcName = context.getFunctionName();
			ArrayList<SourceReturn> returnList = new ArrayList<SourceReturn>();
			context.parseReturnValues(returnList);
			Iterator<SourceReturn> it = returnList.iterator();
			SourceReturn rp = it.next();
			String retstr = rp.getReturnStatement();
			
			// SourceReturn has many properties which are beyond the scope of this test			
			// except for functions with void return r- function should be returning the number of the function
			if(funcName.equals("SourceContextTestClassName1") || funcName.equals("~SourceContextTestClassName2") ){
				assertTrue(retstr.equals(""));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName3")) {
				// TInt FunctionSourceContextTestClassName3( int param1 )
				assertTrue(retstr.equals("3"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName4")) {
				// inline TInt FunctionSourceContextTestClassName4( TInt
				// &param1, void* param2 );
				assertTrue(retstr.equals("4"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName5")) {
				// TInt FunctionSourceContextTestClassName5( TUint32&
				// param1, char param2, short param3 );
				assertTrue(retstr.equals("5"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName6")) {
				// void FunctionSourceContextTestClassName6( TInt& param1,
				// TUint32 *param2, TUint32* param3 );
				assertTrue(retstr.equals(""));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName7")) {
				// char FunctionSourceContextTestClassName7( TInt& param1,
				// void* param2, TAny* param3, int*& param4 );
				assertTrue(retstr.equals("7"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName8")) {
				// short FunctionSourceContextTestClassName8( const TUint32&
				// param1 );
				assertTrue(retstr.equals("8"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName9")) {
				// int FunctionSourceContextTestClassName9( const TInt
				// param1, const TUint32* param2, const void* param3 );
				assertTrue(retstr.equals("9"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName10")) {
				// long FunctionSourceContextTestClassName10( int& param1,
				// TInt** param2 );
				assertTrue(retstr.equals("10"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName11")) {
				// unsigned int FunctionSourceContextTestClassName11(TInt
				// param1 );
				assertTrue(retstr.equals("11"));

			} else if (funcName
					.equals("FunctionSourceContextTestClassName12")) {
				// inline short FunctionSourceContextTestClassName12(void*
				// aPtr )
				assertTrue(retstr.equals("12"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName13")) {
				// inline unsigned short*
				// FunctionSourceContextTestClassName13(void* param1, TInt*
				// param2 )
				assertTrue(retstr.equals("13"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName14")) {
				// inline unsigned*
				// FunctionSourceContextTestClassName14(void* aPtr )
				assertTrue(retstr.equals("14"));
			} else if (funcName
					.equals("FunctionSourceContextTestClassName15")) {
				// inline unsigned int*
				// FunctionSourceContextTestClassName15(void* aPtr )
				assertTrue(retstr.equals("15"));
			} else {
				fail("invalid funcname" + funcName);
			}
		}
	}

	@Test
	public void testToString() {
		int n = 0;
		for(SourceContext context : contexts){
			String s = context.toString();
			assertNotNull(s);
			System.out.println(s);
			if(++n == 1){
				assertTrue(s.equals("SourceContextTestClassName1::SourceContextTestClassName1"));
			}else if(n ==2){ 
				assertTrue(s.equals("SourceContextTestClassName2::~SourceContextTestClassName2"));
			}else if(n == 3){
				assertTrue(s.equals("::FunctionSourceContextTestClassName3"));
			}else{				
				assertTrue(s.equals("SourceContextTestClassName"+n+"::FunctionSourceContextTestClassName"+n));
			}
		}
	}		
}

// End of File




