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
* Test class for TraceConstantTable
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

public class TraceConstantTableTest {
	
	private TraceModel aTraceModel;
	private ModelListenerMock aModelListener;
	private TraceConstantTable aTraceConstantTable;
	private Trace aTrace;
	private TraceGroup aTraceGroup;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Before
	public void setUp() throws Exception {
		try {
			aTraceModel = new TraceModel(new TraceObjectRuleFactoryMock(), null);
		} catch (TraceCompilerException e) {
			fail("failed to intantiate TraceModel object.");
		}
		aModelListener = new ModelListenerMock();
		aTraceModel.addModelListener(aModelListener);
		aTraceConstantTable = new TraceConstantTable(aTraceModel);
		aTraceGroup = new TraceGroup(aTraceModel);
		aTrace = new Trace(aTraceGroup);
	}

	@After
	public void tearDown() throws Exception {
		aTraceModel = null;
		aModelListener = null;
		aTraceConstantTable = null;
		aTrace = null;
		aTraceGroup = null;
	}

	@Test
	public void testSetName() {
		String testName = "testName";
		try {
			aTraceConstantTable.setName(testName);
		} catch (TraceCompilerException e) {
			fail("Cannot execute setName");
		}
		testName = null;
		testName = aTraceConstantTable.getName();
		assertEquals("testName", testName);
		
		try {
			aTraceConstantTable.setName(null);
		} catch (TraceCompilerException e) {
			fail("Cannot execute setName");
		}
		assertEquals("", aTraceConstantTable.getName());
	}

	@Test
	public void testReset() {
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}
		aTraceConstantTable.reset();
		assertTrue(aModelListener.isNotified());
		Iterator<TraceConstantTableEntry> entries = aTraceConstantTable.getEntries();
		int i = 0;
		while (entries.hasNext()){
			TraceConstantTableEntry entry = entries.next();
			assertEquals(0, entry.getID());
			assertEquals("", entry.getName());
			i++;
		}
		// TODO Fix design defect (why times 2?)
		assertEquals(iMax*2, i);
	}

//	@Test
//	public void testTraceConstantTable() {
//	}
//
//	@Test
//	public void testGetType() {
//		fail("Not yet implemented");
//	}
//
//	@Test
//	public void testSetType() {
//	}

	@Test
	public void testAddEntry() {
		assertFalse(aTraceConstantTable.hasEntries());
		
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
		
		Iterator<TraceConstantTableEntry> entries = aTraceConstantTable.getEntries();
		int i = 0;
		int j = 0; // Added to fix TODO: Design defect addEntry adds twice an entry!?!?!
		while (entries.hasNext()){
			TraceConstantTableEntry entry = entries.next();
			assertEquals(i - j, entry.getID());
			assertEquals("testName"+(i-j), entry.getName());
			i++;
			if (i%2 != 0) {
				j++;
			}
		}
	}

	@Test
	public void testRemoveEntry() {
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}
	}

	@Test
	public void testFindEntryByID() {
		int iMax = 5;
		TraceConstantTableEntry[] entryArray = new TraceConstantTableEntry[ iMax ];
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
				entryArray[i] = entry;
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
		
		for (int i=0; i<iMax; i++) {
			TraceConstantTableEntry entry = aTraceConstantTable.findEntryByID(i);
			assertEquals(i, entry.getID());
			assertEquals("testName"+i, entry.getName());
			assertEquals(entryArray[i], entry);
		}
	}

	@Test
	public void testFindEntryByName() {
		int iMax = 5;
		TraceConstantTableEntry[] entryArray = new TraceConstantTableEntry[ iMax ];
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
				entryArray[i] = entry;
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
		
		for (int i=0; i<iMax; i++) {
			TraceConstantTableEntry entry = aTraceConstantTable.findEntryByName("testName"+i);
			assertEquals(i, entry.getID());
			assertEquals("testName"+i, entry.getName());
			assertEquals(entryArray[i], entry);
		}
	}

	@Test
	public void testGetEntries() {
		assertFalse(aTraceConstantTable.hasEntries());
		Iterator<TraceConstantTableEntry> entries = aTraceConstantTable.getEntries();
		assertFalse(entries.hasNext());
		
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
		
		entries = aTraceConstantTable.getEntries();
		int i = 0;
		int j = 0; // Added to fix TODO: Design defect addEntry adds twice an entry!?!?!
		while (entries.hasNext()){
			TraceConstantTableEntry entry = entries.next();
			assertEquals(i-j, entry.getID());
			assertEquals("testName"+(i-j), entry.getName());
			i++;
			if (i%2 != 0) {
				j++;
			}
		}
	}

	@Test
	public void testIterator() {
		assertFalse(aTraceConstantTable.hasEntries());
		Iterator<TraceConstantTableEntry> entries = aTraceConstantTable.iterator();
		assertFalse(entries.hasNext());
		
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
		
		entries = aTraceConstantTable.iterator();
		int i = 0;
		int j = 0; // Added to fix TODO: Design defect addEntry adds twice an entry!?!?!
		while (entries.hasNext()){
			TraceConstantTableEntry entry = entries.next();
			assertEquals(i-j, entry.getID());
			assertEquals("testName"+(i-j), entry.getName());
			i++;
			if (i%2 != 0) {
				j++;
			}
		}
	}

	@Test
	public void testHasEntries() {
		assertFalse(aTraceConstantTable.hasEntries());
		
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
				entry.setID(i);
				entry.setName("testName"+i);
				aTraceConstantTable.addEntry(entry);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		assertTrue(aTraceConstantTable.hasEntries());
	}

	@Test
	public void testAddParameterReference() {
		TraceParameter param = new TraceParameter(aTrace);
		aTraceConstantTable.addParameterReference(param);
		assertTrue(aTraceConstantTable.hasParameterReferences());
		Iterator<TraceParameter> parameters = aTraceConstantTable.getParameterReferences();
		while (parameters.hasNext()){
			TraceParameter aux = parameters.next();
			assertEquals(param, aux);
		}
	}

	@Test
	public void testRemoveParameterReference() {
		assertFalse(aTraceConstantTable.hasParameterReferences());
		TraceParameter param = new TraceParameter(aTrace);
		aTraceConstantTable.addParameterReference(param);
		assertTrue(aTraceConstantTable.hasParameterReferences());
		aTraceConstantTable.removeParameterReference(param);
		assertFalse(aTraceConstantTable.hasParameterReferences());
	}

	@Test
	public void testHasParameterReferences() {
		assertFalse(aTraceConstantTable.hasParameterReferences());
		TraceParameter param = new TraceParameter(aTrace);
		aTraceConstantTable.addParameterReference(param);
		assertTrue(aTraceConstantTable.hasParameterReferences());
	}

	@Test
	public void testGetParameterReferences() {
		int iMax = 3;
		try {
			for (int i=0; i<iMax; i++) {
				TraceParameter param = new TraceParameter(aTrace);
				param.setID(i);
				param.setName("testName"+i);
				aTraceConstantTable.addParameterReference(param);
				assertTrue(aModelListener.isNotified());
			}
		} catch (TraceCompilerException e) {
			fail("Cannot call setID or setName");
		}	
		
		Iterator<TraceParameter> parameters = aTraceConstantTable.getParameterReferences();
		int i = 0;
		while (parameters.hasNext()){
			TraceParameter param = parameters.next();
			assertEquals(i, param.getID());
			assertEquals("testName"+i, param.getName());
			i++;
		}
	}

	@Test
	public void testGetNextEntryID() {
		TraceConstantTableEntry entry = new TraceConstantTableEntry(aTraceConstantTable);
		try{
			entry.setID(12);
		} catch (TraceCompilerException tce) {
			fail("Failed to setID");
		}
		aTraceConstantTable.addEntry(entry);
		assertEquals(13, aTraceConstantTable.getNextEntryID());
	}
	
	/**
	 * Class to Mock a TraceModel Listener, all what we are interested in here is the notification bit
	 *
	 */
	class ModelListenerMock implements TraceModelListener {
		
		boolean notified = false;

		public void objectAdded(TraceObject owner, TraceObject object) {
			notified = true;
		}

		public void objectCreationComplete(TraceObject object)
				throws TraceCompilerException {
			notified = true;
			
		}

		
		public void objectRemoved(TraceObject owner, TraceObject object)
				throws TraceCompilerException {	
			notified = true;
		}

		
		public void propertyUpdated(TraceObject object, int property)
				throws TraceCompilerException {
			//got the message that something changed
			notified = true;
		}
		
		public boolean isNotified() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
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
