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
* Trace test class
*
*/
package com.nokia.tracecompiler.model;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Iterator;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class TraceTest {
	
	//TODO fix this design by either only allowing Trace parameters to be created for a Trace
	//or create them separately and adding them to a Trace.
	static private Trace aTrace;
	static private TraceGroup aTraceGroup;
	static private TraceModel aTraceModel;
	static private MockTraceModelListener aModelListener;

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
		//aTrace = null;
		//aTraceGroup = null;
		//aTraceModel = null;
		//aModelListener = null;
	}

	@Test
	public void testReset() {
		try {
			aTraceModel = new TraceModel(new TraceObjectRuleFactoryMock(), null);
		} catch (TraceCompilerException e) {
			fail("failed to intantiate TraceModel object.");
		}
		aModelListener = new MockTraceModelListener();
		aTraceModel.addModelListener(aModelListener);
		aTraceGroup = new TraceGroup(aTraceModel);
		aTrace = new Trace(aTraceGroup);
		
		TraceParameter aTraceParam = new TraceParameter(aTrace);
		//TODO API design defect: Why instantiating TraceParameter adds parameter to Trace object?
		//aTrace.addParameter(aTraceParam);
		assertTrue(aTrace.hasParameters());
		assertEquals(1, aTrace.getParameterCount());
		
		aTrace.reset();
		assertFalse(aTrace.hasParameters());
		assertEquals(0, aTrace.getParameterCount());
	}

//	@Test
//	public void testTrace() {
//	}

	@Test
	public void testSetTrace() {
		String str = null;
		aModelListener.resetNotification();
		try {
			aTrace.setTrace(str);
		} catch (TraceCompilerException e) {
			fail("failed to call setTrace");
		}
		str = aTrace.getTrace();
		assertNotNull(str);
		assertEquals("", str);
		assertFalse(aModelListener.isNotified());
		
		str = "test";
		try {
			aTrace.setTrace(str);
		} catch (TraceCompilerException e) {
			fail("failed to create TraceModel.");
		}
		str = aTrace.getTrace();
		assertNotNull(str);
		assertEquals("test", str);
		assertTrue(aModelListener.isNotified());
	}

//	@Test
//	public void testGetTrace() {
//	}

	@Test
	public void testGetGroup() {
		TraceGroup group = aTrace.getGroup();
		assertEquals(group, aTraceGroup);
	}

	@Test
	public void testAddParameter() {
		TraceParameter param = new TraceParameter(aTrace);
		aTrace.addParameter(param);
		assertTrue(aTrace.hasParameters());
		assertEquals(2, aTrace.getParameterCount());
		TraceParameter param1 = aTrace.getParameter(1);
		assertEquals(param, param1);
	}

	@Test
	public void testInsertParameter() {
		TraceParameter param = new TraceParameter(aTrace);
		aTrace.insertParameter(1, param);
		assertTrue(aTrace.hasParameters());
		TraceParameter param1 = aTrace.getParameter(1);
		assertEquals(param, param1);
	}

	@Test
	public void testRemoveParameter() {
		aTrace.reset();
		aModelListener.resetNotification();
		TraceParameter aTraceParam = new TraceParameter(aTrace);
		//aTrace.addParameter(aTraceParam);
		try{
			aTrace.removeParameter(aTraceParam);
			assertFalse(aTrace.hasParameters());
			assertEquals(0, aTrace.getParameterCount());
			assertTrue(aModelListener.isNotified());
		} catch (TraceCompilerException tce) {
			fail("Cannot removeParameter");
		}
	}

	@Test
	public void testRemoveParameterAt() {
		aTrace.reset();
		aModelListener.resetNotification();
		TraceParameter aTraceParam = new TraceParameter(aTrace);
		aTrace.insertParameter(0, aTraceParam);
		try{
			aTrace.removeParameterAt(0);
			//TODO fix this design
			aTrace.removeParameterAt(0);
			assertTrue(aModelListener.isNotified());
		} catch (TraceCompilerException tce) {
			fail("Cannot removeParameterAt");
		}
		assertEquals(0, aTrace.getParameterCount()); 
		assertFalse(aTrace.hasParameters());
	}

	@Test
	public void testHasParameters() {
		aTrace.reset();
		aModelListener.resetNotification();
		TraceParameter aTraceParam = new TraceParameter(aTrace);
		aTrace.addParameter(aTraceParam);
		assertTrue(aTrace.hasParameters());
		try{
			aTrace.removeParameter(aTraceParam);
			assertTrue(aTrace.hasParameters());
			aTrace.removeParameter(aTraceParam);
			assertFalse(aTrace.hasParameters());
		} catch (TraceCompilerException tce) {
			fail("Cannot testHasParameters");
		}
	}

	@Test
	public void testGetParameterCount() {
		aTrace.reset();
		assertEquals(0, aTrace.getParameterCount());
		int iMax = 5;
		for (int i=0; i<iMax; i++) {
			aTrace.addParameter(new TraceParameter(aTrace));
		}
		assertEquals(iMax * 2, aTrace.getParameterCount());
	}

	@Test
	public void testGetNextParameterID() {
		TraceParameter param = new TraceParameter(aTrace);
		try{
			param.setID(12);
		} catch (TraceCompilerException tce) {
			fail("Failed to setID");
		}
		aTrace.addParameter(param);
		assertEquals(13, aTrace.getNextParameterID());
	}

	@Test
	public void testGetParameter() {
		aTrace.reset();
		int iMax = 3;
		try{
			for (int i=0; i<iMax; i++) {
				TraceParameter param = new TraceParameter(aTrace);
				param.setID(i);
				//aTrace.addParameter(param);
			}
		} catch (TraceCompilerException e){
			fail("Failed to setID to TraceParameter");
		}
		for (int i=0; i<iMax; i++) {
			TraceParameter param = aTrace.getParameter(i);
			assertEquals(i, param.getID());
		}
	}

	@Test
	public void testGetParameters() {
		aTrace.reset();
		int iMax = 3;
		try{
			for (int i=0; i<iMax; i++) {
				TraceParameter param = new TraceParameter(aTrace);
				param.setID(i);
				//aTrace.addParameter(param);
			}
		} catch (TraceCompilerException e){
			fail("Failed to setID to TraceParameter");
		}
		
		Iterator<TraceParameter> params = aTrace.getParameters();
		int i = 0;
		while (params.hasNext()){
			int iValue = ((TraceParameter)params.next()).getID();
			assertEquals(i, iValue);
			i++;
		}
	}

//	@Test
//	public void testIterator() {
//		fail("Not yet implemented");
//	}

	@Test
	public void testFindParameterByID() {
		aTrace.reset();
		int iMax = 5;
		TraceParameter[] parameters = new TraceParameter[iMax];
		try {
			for (int i=0; i<iMax; i++) {
				parameters[i] = new TraceParameter(aTrace);
				parameters[i].setID(i);
				parameters[i].setName("test"+i);
				//aTrace.addParameter(parameters[i]);
			}
		} catch (TraceCompilerException e){
			fail("Failed to setID to TraceParameter");
		}
		// Test
		for (int i=0; i<iMax; i++) {
			TraceParameter param = aTrace.findParameterByID(i);
			assertEquals(parameters[i], param);
			assertEquals(i, param.getID());
			assertEquals("test"+i, param.getName());
		}
	}

	@Test
	public void testFindParameterByName() {
		aTrace.reset();
		int iMax = 5;
		TraceParameter[] parameters = new TraceParameter[iMax];
		try {
			for (int i=0; i<iMax; i++) {
				parameters[i] = new TraceParameter(aTrace);
				parameters[i].setID(i);
				parameters[i].setName("test"+i);
				//aTrace.addParameter(parameters[i]);
			}
		} catch (TraceCompilerException e){
			fail("Failed to setID to TraceParameter");
		}
		// Test
		for (int i=0; i<iMax; i++) {
			TraceParameter param = aTrace.findParameterByName("test"+i);
			assertEquals(parameters[i], param);
			assertEquals(i, param.getID());
			assertEquals("test"+i, param.getName());
		}
	}
	
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
