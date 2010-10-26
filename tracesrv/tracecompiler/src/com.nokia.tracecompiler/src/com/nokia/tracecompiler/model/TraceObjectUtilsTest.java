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
* Test class for static TracObjectUtils class
*
*/
package com.nokia.tracecompiler.model;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class TraceObjectUtilsTest {

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
	public void testModifyDuplicateParameterName() {
		aModelListener = new MockTraceModelListener();
		try {
			model = new TraceModel(new TraceObjectRuleFactoryMock(), null);
		} catch (TraceCompilerException e) {
			fail("failed to create trace model : " + e.toString());
		}
		model.addModelListener(aModelListener);
		trace = new Trace(new TraceGroup(model));
		p = new TraceParameter(trace);
		assertTrue(aModelListener.isNotified());
		try {
			p.setID(1);
			p.setName("myparam");
			p.setType("mytype");
		} catch(TraceCompilerException e) {
			fail("failed to set parameter : " + e.toString());
		}
		TraceObjectModifier modifyDuplicateParameterName = TraceObjectUtils.modifyDuplicateParameterName(trace, "myparam");
		//Strange stuff, nothing is documented, but this seems to prepend DUP + parameter_index to the name and return a new object
		assertEquals("DUP" + 1 + "_" + p.getName(), modifyDuplicateParameterName.getData());
		
		//what if parameter name already has DUP1_...
		try {
			p.setName("DUP1_myparam");
		} catch (TraceCompilerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		modifyDuplicateParameterName = TraceObjectUtils.modifyDuplicateParameterName(trace, "DUP1_myparam");
		// this would return DUP2_myparam
		assertEquals("DUP" + 2 + "_" + "myparam", modifyDuplicateParameterName.getData());
		
		//what if the name is DUPanything_myparam
		try {
			p.setName("DUPxxx_myparam");
		} catch (TraceCompilerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		modifyDuplicateParameterName = TraceObjectUtils.modifyDuplicateParameterName(trace, "DUPxxx_myparam");
		// this would return DUP1_DUPxxx_myparam
		assertEquals("DUP1_" + p.getName(), modifyDuplicateParameterName.getData());
		
		//what if it's not a duplicate
		
		modifyDuplicateParameterName = TraceObjectUtils.modifyDuplicateParameterName(trace, "MMM_myparam");
		//it just return the name we gave
		assertEquals("MMM_myparam", modifyDuplicateParameterName.getData());
		//TODO so what's the fuss about going through all those acrobatic thingies in the code
		//the method is just looking if a trace has a parameter with name and returns a generated name 
		//to avoid duplications according to some convention, if not it just returns what we gave.
	}

	@Test
	public void testRemoveDuplicateModifier() {
		
		assertEquals("myparam", TraceObjectUtils.removeDuplicateModifier("myparam"));
		assertEquals("myparam", TraceObjectUtils.removeDuplicateModifier("DUP1_myparam"));
		
		//this is strange as the rule has now changed, this should not be identified as duplicate name.
		//see previous test
		assertEquals("DUPxxx_myparam", TraceObjectUtils.removeDuplicateModifier("xx_myparam"));
		
	}

	@Test
	public void testFindProperty() {
		//TODO this method looks like a non-functional code
		//the method findProperty() is called from the dictionary but seems to always 
		//return empty string.
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
