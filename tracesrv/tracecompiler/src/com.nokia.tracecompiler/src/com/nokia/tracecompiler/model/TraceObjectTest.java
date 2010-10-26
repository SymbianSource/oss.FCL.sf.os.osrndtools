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
* Test Class for TraceObject class
*
*/
package com.nokia.tracecompiler.model;

import static org.junit.Assert.*;

import java.util.Iterator;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;


public class TraceObjectTest {
	
	//TODO there is nothing to justify why TraceObject is abstract
	static TraceObject trace;
	static TraceModel model;
	static MockTraceModelListener listener;
	static TraceModelExtension traceModelExtension;
	static TraceModelExtension traceModelExtension2;
	static TraceModelExtension traceModelExtension3;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		// this is a workaround to instantiate a class from the abstract class we want to test.
		trace  = new MockTraceObject();
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
	public void testSetModel() {
		try {
			//create a trace model
			model = new TraceModel(new TraceObjectRuleFactoryMock(), null);
			//create a mock model listener
			listener = new MockTraceModelListener();
			//set the listener on the model
			model.addModelListener(listener);
		} catch (TraceCompilerException e) {
			fail("failed to create TraceModel.");
		}
		//set the model for the trace object
		trace.setModel(model);
		//check the model is set and is the same we passed in
		assertEquals(model, trace.getModel());
	}

	@Test
	public void testGetModel() {
		//check our model again
		assertEquals(model, trace.getModel());
		//set a null model i.e. it should be removed from the trace object
		trace.setModel(null);
		//check the trace object does not have a model.
		assertNull(trace.getModel());
	}

	@Test
	public void testSetID() {
		try {
			//TODO TraceObject setID is not right. what happen if we call getID without setID?
			//either the ID is passed to the costructor and make the default constructor private
			//or set a flag when setID is called, if not getID would raise an excpetion for uninitialzed variable.
			trace.setModel(model);
			//set a trace id 1
			trace.setID(1);
			//check it set to 1
			assertTrue(trace.getID() == 1);
			//check the model listener has been notified
			assertTrue(listener.isNotified());
			listener.resetNotification();
			//change the trace id and check it again and check the model listener
			trace.setID(2);
			assertTrue(trace.getID() == 2);
			assertTrue(listener.isNotified());
			listener.resetNotification();
			
		} catch (TraceCompilerException e) {
			fail("failed to set Trace ID.");
		}
	}

	@Test
	public void testInternalSetID() {
		//reset id without nofying the model listener
		trace.internalSetID(1);
		//check it's set to 1
		assertTrue(trace.getID() == 1);
		//check the listener is NOT notified of the change
		assertFalse(listener.isNotified());
		//do it again with another value
		trace.internalSetID(2);
		assertTrue(trace.getID() == 2);
		assertFalse(listener.isNotified());
	}

	@Test
	public void testSetName() {
		try {
			//set trace name and check the listener is notified
			trace.setName("TestTrace");
			assertEquals("TestTrace", trace.getName());
			//check the listener had the event
			assertTrue(listener.isNotified());
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			fail("failed to set Trace Name.");
		}
	}

	@Test
	public void testAddExtension() {
		//add an extension TraceModelExtensionMock and check it's set
		traceModelExtension = new TraceModelExtensionMock();
		trace.addExtension(traceModelExtension);
		Iterator<TraceModelExtensionMock> extensions = trace.getExtensions(TraceModelExtensionMock.class);
		assertTrue(extensions.hasNext());
		assertEquals(traceModelExtension, extensions.next());
		assertFalse(extensions.hasNext());
		
		//add another extension
		traceModelExtension2 = new TraceModelExtensionMock2();
		trace.addExtension(traceModelExtension2);
		Iterator<TraceModelExtensionMock2> extensions2 = trace.getExtensions(TraceModelExtensionMock2.class);
		assertTrue(extensions2.hasNext());
		assertEquals(traceModelExtension2, extensions2.next());
		assertFalse(extensions2.hasNext());
		
		//add a duplicate
		traceModelExtension3 = new TraceModelExtensionMock2();
		trace.addExtension(traceModelExtension3);
		extensions2 = trace.getExtensions(TraceModelExtensionMock2.class);
		assertTrue(extensions2.hasNext());
		assertEquals(traceModelExtension2, extensions2.next());
		assertEquals(traceModelExtension3, extensions2.next());
		assertFalse(extensions2.hasNext());
		
		//add a null extension
		try {
			trace.addExtension(null);
		} catch(NullPointerException e) {
			fail("Code should never throw NulPointerException " + e.toString());
		}
		
	}

	@Test
	public void testRemoveExtension() {
		//remove and extension and check it's removed
		trace.removeExtension(traceModelExtension3);
		Iterator<TraceModelExtensionMock> extensions = trace.getExtensions(TraceModelExtensionMock.class);
		assertTrue(extensions.hasNext());
		assertEquals(traceModelExtension, extensions.next());
		assertFalse(extensions.hasNext());
		Iterator<TraceModelExtensionMock2> extensions2 = trace.getExtensions(TraceModelExtensionMock2.class);
		assertTrue(extensions2.hasNext());
		assertEquals(traceModelExtension2, extensions2.next());
		assertFalse(extensions2.hasNext());
		
	}

	@Test
	public void testRemoveExtensions() {
		//remove all extension of a specific type and check they have been removed
		trace.addExtension(traceModelExtension3);
		trace.removeExtensions(TraceModelExtensionMock2.class);
		Iterator<TraceModelExtensionMock2> extensions2 = trace.getExtensions(TraceModelExtensionMock2.class);
		assertFalse(extensions2.hasNext());
	}

	@Test
	public void testGetExtension() {
		// test get a extension
		assertNull(trace.getExtension(TraceModelExtensionMock2.class));
		assertEquals(traceModelExtension, trace.getExtension(TraceModelExtensionMock.class));
	}


	@Test
	public void testReset() {
		//reset the trace object and check the id and name reset to 0 and empty string
		trace.internalSetID(11);
		try {
			trace.setName("TestTraceRenamed");
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			fail("failed to rename trace.");
		}
		trace.reset();
		assertTrue(trace.getID() == 0);
		assertEquals("", trace.getName());
	}

	@Test
	public void testSetComplete() {
		//set trace object processing complete and check its status
		listener.resetNotification();
		try {
			assertFalse(trace.isComplete());
			trace.setComplete();
			assertTrue(listener.isNotified());
			assertTrue(trace.isComplete());
		} catch (TraceCompilerException e) {
			fail("failed to set trace to complete status");
		}
		
	}

	@Test
	public void testNotifyOnDelete() {
		//test notifucation on delete
		TraceObjectRuleOnDeleteMock extension = new TraceObjectRuleOnDeleteMock();
		assertFalse(extension.isNotified());
		trace.addExtension(extension);
		trace.notifyOnDelete(trace);
		assertTrue(extension.isNotified());
		extension.resetNotification();
	}
	
	/**
	 * Class to Mock a TraceModel extension
	 *
	 */
	class TraceModelExtensionMock implements TraceModelExtension {		
		public TraceObject getOwner() {
			return null;
		}
		
		public void setOwner(TraceObject owner) {			
		}		
	}
	
	/**
	 * another Trace Model extension
	 *
	 */
	class TraceModelExtensionMock2 implements TraceModelExtension {		
		public TraceObject getOwner() {			
			return null;
		}
		
		public void setOwner(TraceObject owner) {			
		}		
	}
	
	/**
	 * Class to Mock TraceObjectRuleOnDelete listener, all what we are interested in here is the notification bit
	 *
	 */
	class TraceObjectRuleOnDeleteMock implements  TraceObjectRuleOnDelete {

		boolean notified = false;
		
		public void objectDeleted() {
			notified = true;
		}

		public TraceObject getOwner() {
			return null;
		}
		
		public void setOwner(TraceObject owner) {
		}
		
		public boolean isNotified() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
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
}
