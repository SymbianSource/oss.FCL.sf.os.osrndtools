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
* Test class for TraceGroup Class
*
*/
package com.nokia.tracecompiler.model;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Iterator;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class TraceGroupTest {
	static TraceModel model;
	static TraceObjectRuleFactoryMock factory;
	static TraceObjectPropertyVerifierMock v;
	static MockTraceModelListener listener;
	
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
			factory = new TraceObjectRuleFactoryMock();
			v = new TraceObjectPropertyVerifierMock();
			model = new TraceModel(factory, v);
			listener = new MockTraceModelListener();
			model.addModelListener(listener);
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
		//create a TraceGroup
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
		//create two Traces owned by the existing group
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(2);
			t2.setName("trace2");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
		//add an extension to trace t1
		MockTraceObjectRuleOnDelete extension = new MockTraceObjectRuleOnDelete();
		t1.addExtension(extension);
		
		//check group has only the traces we added
		assertTrue(g.hasTraces());
		Iterator<Trace> traces = g.getTraces();
		assertTrue(traces.hasNext());
		g.reset();
		assertFalse(g.hasTraces());
		assertEquals(0, g.getTraceCount());
		traces = g.getTraces();
		assertFalse(traces.hasNext());
		
		//the reset also does a reset on all traces
		assertTrue(extension.getNotifocation());
		extension.resetNotification();
		
		//check that all traces have been reset. 
		//TODO why group cleans up traces it did create in the first place.
		assertEquals(0, t1.getID());
		assertEquals("", t1.getName());
		assertEquals(0, t2.getID());
		assertEquals("", t2.getName());
		
		//check group ID and name have been reset
		assertEquals(0, g.getID());
		assertEquals("", g.getName());
	}

	@Test
	public void testTraceGroup() {
		//see above
	}

	@Test
	public void testAddTrace() {
		//TODO is this needed as Trace constructor always requires a Group
		//this is similar to the Group -- Model relation
	}

	@Test
	public void testRemoveTrace() {
		model.reset();
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(2);
			t2.setName("trace2");
			g.removeTrace(t1);
			assertTrue(listener.isNotified());
			listener.resetNotification();
			
			// check there is only t2 left
			assertTrue(g.hasTraces());
			assertEquals(1, g.getTraceCount());
			Iterator<Trace> traces = g.getTraces();
			assertSame(t2, traces.next());
			
			//check t1 is reset.
			//TODO is it necessary to reset t1?
			assertEquals(0, t1.getID());
			assertEquals("", t1.getName());
			
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testHasTraces() {
		//tested above
	}

	@Test
	public void testGetTraceCount() {
		//tested above
	}

	@Test
	public void testGetTraces() {
		//tested above
	}

	@Test
	public void testIterator() {
		//TODO delete this is exactly like getTraces()
	}

	@Test
	public void testGetNextTraceID() {
		model.reset();
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(20);
			t2.setName("trace2");
			
			assertEquals(21, g.getNextTraceID());
			g.removeTrace(t2);
			assertEquals(2, g.getNextTraceID());
			
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testFindTraceByID() {
		model.reset();
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(20);
			t2.setName("trace2");
			
			assertSame(t2, g.findTraceByID(20));
			assertSame(t1, g.findTraceByID(1));
			assertNull(g.findTraceByID(5555));
			
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testTracePropertyUpdated() {
		model.reset();
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(20);
			t2.setName("trace2");
			
			Iterator<Trace> traces = g.getTraces();
			assertSame(t1, traces.next());
			assertSame(t2, traces.next());
			
			t1.setID(40);
			g.tracePropertyUpdated(new MockTraceObject(), TraceModelListener.ID);
			traces = g.getTraces();
			assertSame(t2, traces.next());
			assertSame(t1, traces.next());
			
			//TODO tracePropertyUpdated take a TraceObject parameters which is unused
			//TODO it seems that tracesByName is not used. This must impact performance as 
			// group keeps ordering traces by name.
			// TraceModelListener.NAME
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testTraceIDsUpdated() {
		//similar to testTracePropertyUpdated(). 
		model.reset();
		TraceGroup g = new TraceGroup(model);
		
		try {
			g.setID(22);
			g.setName("testGroup");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Trace t1 = new Trace(g);
		Trace t2 = new Trace(g);
		
		try {
			t1.setID(1);
			t1.setName("trace1");
		
			t2.setID(20);
			t2.setName("trace2");
			
			Iterator<Trace> traces = g.getTraces();
			assertSame(t1, traces.next());
			assertSame(t2, traces.next());
			
			t1.setID(40);
			t2.setID(30);
			g.traceIDsUpdated();
			traces = g.getTraces();
			assertSame(t2, traces.next());
			assertSame(t1, traces.next());
			
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}
	
	/**
	 * a Mock of the TraceObjectRuleFactory need for for creating a model. empty because we are not testing
	 * the model in this class.
	 *
	 */
	class TraceObjectRuleFactoryMock implements TraceObjectRuleFactory {
		
		public TraceModelPersistentExtension createExtension(
				TraceObject object, String name) {
			return null;
		}
		
		public void postProcessNewRules(TraceObject object) {
		}
		
		public void preProcessNewRules(TraceObject object) {
		}		
	}
	
	class TraceObjectPropertyVerifierMock implements TraceObjectPropertyVerifier {

		
		public void checkConstantProperties(TraceConstantTable table,
				TraceConstantTableEntry entry, int id, String value)
				throws TraceCompilerException {
			
			
		}

		
		public void checkConstantTableProperties(TraceModel owner,
				TraceConstantTable table, int id, String tableName)
				throws TraceCompilerException {
			
			
		}

		
		public void checkTraceGroupProperties(TraceModel owner,
				TraceGroup group, int id, String name)
				throws TraceCompilerException {
			
			
		}

		
		public void checkTraceModelProperties(TraceModel model, int id,
				String name, String path) throws TraceCompilerException {
			
			
		}

		
		public void checkTraceParameterProperties(Trace owner,
				TraceParameter parameter, int id, String name, String type)
				throws TraceCompilerException {
			
			
		}

		
		public void checkTraceProperties(TraceGroup group, Trace trace, int id,
				String name, String data) throws TraceCompilerException {
			
			
		}
		
	}
	
	class MockTraceObjectRuleOnDelete implements TraceObjectRuleOnDelete {

		private boolean notified = false;
		
		public void objectDeleted() {
			notified = true;
		}

		
		public TraceObject getOwner() {			
			return null;
		}

		public void setOwner(TraceObject owner) {			
		}
		
		public boolean getNotifocation() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
		}
	}
	

}
