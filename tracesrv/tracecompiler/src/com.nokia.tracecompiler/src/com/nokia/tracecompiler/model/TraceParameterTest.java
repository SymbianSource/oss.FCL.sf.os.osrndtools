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
* Test class for TraceParameter class
*
*/

package com.nokia.tracecompiler.model;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class TraceParameterTest {

	static private TraceParameter p;
	static private MockTraceModelListener aModelListener;
	static private Trace trace;
	static private TraceModel model;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
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
	public void testReset() {
		try {
			aModelListener = new MockTraceModelListener();
			model = new TraceModel(new TraceObjectRuleFactoryMock(), null);
			model.addModelListener(aModelListener);
			trace = new Trace(new TraceGroup(model));
			p = new TraceParameter(trace);
			assertTrue(aModelListener.isNotified());
			p.setID(1);
			p.setName("param1");
			p.setType("type1");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}

		assertTrue(trace.hasParameters());
		assertEquals(1, trace.getParameterCount());
		p.reset();
		assertTrue(trace.hasParameters());
		assertEquals(1, trace.getParameterCount());

	}

	@Test
	public void testTraceParameterTrace() {
		//parameter already created
		try {
			p.setID(1);
			p.setName("param1");
			p.setType("type1");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
		assertEquals(1, p.getID());
		assertEquals("param1", p.getName());
		assertEquals("type1", p.getType());
		
		TraceParameter p2 = new TraceParameter(trace);
		assertSame(trace, p2.getTrace());

	}

	@Test
	public void testTraceParameterTraceInt() {
		aModelListener.resetNotification();
		TraceParameter p2 = new TraceParameter(trace, 1);
		assertSame(trace, p.getTrace());
		assertSame(p2, trace.getParameter(1));
		assertTrue(aModelListener.isNotified());
		
		
		//what if we add to an index that does not exist
		try {
			TraceParameter p3 = new TraceParameter(trace, 20);
		} catch (IndexOutOfBoundsException e) {
			//Defect
			fail("tried to add a paramater with no existant index.");
		}
	}

	@Test
	public void testGetTrace() {
		trace.reset();
		p = new TraceParameter(trace);
		assertEquals(trace, p.getTrace());
		try {
			trace.removeParameter(p);
		} catch (TraceCompilerException e) {
			fail("could not remove parameter from trace: " + e.toString());
		}
		assertEquals(0, p.getID());
		assertEquals("", p.getName());
		assertNull(p.getTrace()); //TODO defect, the parameter was removed from trace but not completely cleaned
		//TODO this would also fail should return empty string or null
		assertEquals("", p.getType());
	}

	@Test
	public void testSetType() {
		try {
			p.setType("type");
		} catch (TraceCompilerException e) {
			fail("failed to set type to parameter :" + e.toString());
		}
		assertEquals("type", p.getType());
		p.reset();
	}

	@Test
	public void testGetType() {
		//done above
	}

	class TraceObjectRuleFactoryMock implements TraceObjectRuleFactory {

		public TraceModelPersistentExtension createExtension(TraceObject object, String name) {
			return null;
		}

		public void postProcessNewRules(TraceObject object) {
		}

		public void preProcessNewRules(TraceObject object) {
		}
	}

}
