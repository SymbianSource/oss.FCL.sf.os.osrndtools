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
* Test class for DuplicateParameterModifier
*
*/

package com.nokia.tracecompiler.model;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;

public class DuplicateParameterNameModifierTest {

	
	
	char defaultSeparator = '_';
	final String theName = "traceName";
	final String theText= "the text";	
	final int theId = 0;
	final String paramName = "paramName";	
	static DuplicateParameterNameModifier modifier;
	static TraceParameter traceParameter;
	
	
	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		
		TraceCompilerEngineGlobals.start();

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
	public void testDuplicateParameterNameModifier() throws TraceCompilerException {
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();
		TraceGroup group = new TraceGroup(model);
		model.addGroup(group);			

		Trace trace = new Trace(group);		
		trace.setID(theId);
		trace.setName(theName);
		trace.setTrace(theText);
		traceParameter = new TraceParameter(trace, theId);

		traceParameter.setName(paramName);
		assertNotNull(traceParameter);
		trace.addParameter(traceParameter);
		modifier = new DuplicateParameterNameModifier(trace, paramName);		
	}

	@Test
	public void testFindObject() {
		TraceObject object = modifier.findObject(paramName);		
		assertNotNull(object);
		assertEquals(object, traceParameter);		
	}

	@Test
	public void testGetData() {
		String data = modifier.getData();			
		assertNotNull(data);
		assertTrue(paramName.equals(data));
	}

	@Test
	public void testHasChanged() {
		boolean changed = modifier.hasChanged();		
		assertFalse(changed);		
	}

	@Test
	public void testGetSeparator() {
		char sep = modifier.getSeparator();	
		assertEquals(sep, defaultSeparator);		
	}

	@Test
	public void testProcessName() {
		boolean changed = modifier.hasChanged();				
		assertFalse(changed);
		modifier.processName();
		changed = modifier.hasChanged();
		assertTrue(changed);
	}

	@Test
	public void testGetModifier() {
		String str = DuplicateValueModifier.getModifier("DUP_the first one");		
		assertNotNull(str);
		assertTrue(str.equals("DUP_"));

		str = DuplicateValueModifier.getModifier("DUP1_the second one");		
		assertNotNull(str);
		assertTrue(str.equals("DUP1_"));
		
		str = DuplicateValueModifier.getModifier("DUP999_another one");		
		assertNotNull(str);
		assertTrue(str.equals("DUP999_"));

		str = DuplicateValueModifier.getModifier("DUP1234_------------------------------------------------- hello --------------");		
		assertNotNull(str);
		assertTrue(str.equals("DUP1234_"));

		str = DuplicateValueModifier.getModifier("DUP this one is valid");		
		assertNotNull(str);
		assertTrue(str.equals("DUP "));  // TODO - is this correct expected behavior

		str = DuplicateValueModifier.getModifier("DuP this one is invalid");		
		assertNull(str);

		str = DuplicateValueModifier.getModifier("DuP_this one is invalid");		
		assertNull(str);

	}

}
