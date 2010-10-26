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
* Test class for TraceObjectfactory class
*
*/

package com.nokia.tracecompiler.model;

import java.util.ArrayList;
import java.util.Iterator;

import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class TraceObjectFactoryTest {

	static TraceObjectFactory factory;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TraceCompilerEngineGlobals.start();
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();
		factory = model.getFactory();
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
	public void testTraceObjectFactory() throws TraceCompilerException {
		
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();		
		TraceObjectTest.TraceObjectRuleFactoryMock mockRuleFactory = new TraceObjectTest(). new TraceObjectRuleFactoryMock();
		TraceObjectFactory factory = new TraceObjectFactory(model, mockRuleFactory);
		
		assertNotNull(factory);
	}

	@Test
	public void testGetRuleFactory() throws TraceCompilerException {
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();		
		TraceObjectTest.TraceObjectRuleFactoryMock mockRuleFactory = new TraceObjectTest(). new TraceObjectRuleFactoryMock();
		TraceObjectFactory factory = new TraceObjectFactory(model, mockRuleFactory);		
		assertSame(mockRuleFactory, factory.getRuleFactory());
	}

	@Test
	public void testCreateTraceGroup() throws Exception {
		int id = 1;
		String name = "hello";

		TraceModelExtension[] extensions = null;
		TraceGroup group = factory.createTraceGroup(id, name, extensions);

		assertSame(id, group.getID());
		assertTrue(name.equals(group.getName()));
	}


	@Test
	public void testCreateTrace() throws TraceCompilerException {

		int id = 1;
		String name = "hello";
		String traceText = "traceText";

		int numExtensions = 8;
		ArrayList<MockExtension> ext1 = MockExtension.createExtensionsHelper(numExtensions);
		MockExtension extsArr1[] = new MockExtension[numExtensions];
		ext1.toArray(extsArr1);
		
		TraceGroup group = factory.createTraceGroup(id, name, extsArr1 );
		Trace trace = factory.createTrace(group, id, name, traceText, extsArr1);
		assertSame(trace.getID(), id);
		assertSame(trace.getName(),  name);
		assertTrue(traceText.equals(trace.getTrace()));

		Iterator<MockExtension> it2 = trace.getExtensions(MockExtension.class);
		assertTrue(MockExtension.compareExtensionsHelper(ext1.iterator(), it2));

	}

	@Test
	public void testCreateTraceParameterTraceIntStringStringTraceModelExtensionArray()
			throws TraceCompilerException {

		int id = 1;

		String name = "theName";
		String type = "theType";
			
		int numExtensions = 8;
		ArrayList<MockExtension> ext1 = MockExtension.createExtensionsHelper(numExtensions);
		MockExtension extsArr1[] = new MockExtension[numExtensions];
		ext1.toArray(extsArr1);
	
		TraceGroup group = factory.createTraceGroup(id, name, extsArr1);
		Trace trace = factory.createTrace(group, id, name, type, extsArr1);
		TraceParameter param = factory.createTraceParameter(trace, id, name, type, extsArr1);

		assertSame(id, param.getID());
		assertTrue(name.equals(param.getName()));
		assertTrue(type.equals(param.getType()));
		assertSame(param.getTrace(), trace);

		Iterator<MockExtension> it2 = trace.getExtensions(MockExtension.class);
		assertTrue(MockExtension.compareExtensionsHelper(ext1.iterator(), it2));
	}

	@Test
	public void testCreateTraceParameterIntTraceIntStringStringTraceModelExtensionArray() throws Exception{
		int objectIndex = 0;
		int id = 1;

		String name = "theName";
		String type = "theType";
			
		int numExtensions = 12;
		ArrayList<MockExtension> ext1 = MockExtension.createExtensionsHelper(numExtensions);
		MockExtension extsArr1[] = new MockExtension[numExtensions];
		ext1.toArray(extsArr1);
	
		TraceGroup group = factory.createTraceGroup(id, name, extsArr1);
		Trace trace = factory.createTrace(group, id, name, type, extsArr1);
		TraceParameter param = factory.createTraceParameter(objectIndex, trace,
				id, name, type, extsArr1);

		assertEquals(id, param.getID());
		assertTrue(name.equals(param.getName()));
		assertTrue(type.equals(param.getType()));
		assertSame(param.getTrace(), trace);

		Iterator<MockExtension> it2 = trace.getExtensions(MockExtension.class);
		assertTrue(MockExtension.compareExtensionsHelper(ext1.iterator(), it2));

	}

	@Test
	public void testCreateConstantTable() throws Exception{
		
		int numExtensions = 9;
		ArrayList<MockExtension> ext1s = MockExtension.createExtensionsHelper(numExtensions);
		MockExtension extensions[] =  new MockExtension[numExtensions];
		ext1s.toArray(extensions);
		
		int id = 2;
		String typeName = "the type name";
		
		
		
		TraceConstantTable table = factory.createConstantTable(id, typeName, extensions);

		// check the extensions are in the table
		Iterator<MockExtension> exts =  table.getExtensions(MockExtension.class);

		int count = 0;
		while(exts.hasNext()){
			count++;
			MockExtension ext = exts.next();
			assertNotNull(ext);
			
			// now check that ext is in our original list of extensions
			boolean found = false;
			for (MockExtension me : ext1s) {
				if(ext.equals(me)){
					found = true;
					break;
				}				
			}			
			assertTrue(found);						
		}
		
		assertEquals(count, numExtensions);		
	}

	@Test
	public void testCreateConstantTableEntry() throws Exception{
		int numExtensions = 3;
		ArrayList<MockExtension> ext1 = MockExtension.createExtensionsHelper(numExtensions);
		MockExtension extensions[] =  new MockExtension[numExtensions];
		ext1.toArray(extensions);
		
		int id = 399;
		String typeName = "the type name1234abcd";
		TraceConstantTable table = factory.createConstantTable(id, typeName, extensions);

		String value = "tha_value";
		// create an entry on the table
		factory.createConstantTableEntry(table, id, value, extensions);
		
		Iterator<TraceConstantTableEntry> it1 = table.iterator();
		assertTrue(it1.hasNext());
		TraceConstantTableEntry entry = it1.next();
		
		
		assertEquals(entry.getID(), id);
		assertSame(entry.getTable(), table);
		assertTrue(value.equals(entry.getName()) );		
		assertFalse(it1.hasNext());
	}

	@Test
	public void testCreateExtension() {		
		MockTraceObject object = new MockTraceObject();
		
		// we should only create valid extensions for existing types 
		// one of these is TypeMapping		
		String name1 = "TypeMapping";					
		TraceModelPersistentExtension ext1 = factory.createExtension(object, name1);
		assertNotNull(ext1);
		
		String name2 = "NoTypeMapping";					
		assertNull(factory.createExtension(object, name2));		
		
	}
}
