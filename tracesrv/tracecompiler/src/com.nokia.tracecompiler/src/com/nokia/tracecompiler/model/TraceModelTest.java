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
* Test class for TraceModel
*
*/
package com.nokia.tracecompiler.model;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Iterator;
import java.util.Properties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import com.nokia.tracecompiler.engine.project.SortedProperties;
import com.nokia.tracecompiler.source.SourceConstants;

public class TraceModelTest {

	static TraceModel model;
	static MockTraceModelListener listener ;
	static TraceModelExtensionListenerMock extListener;
	static TraceModelResetListenerMock resetListener;
	static TraceProcessingListenerMock procListener;
	static TraceObjectRuleFactoryMock factory;
	static TraceObjectPropertyVerifierMock v;
	
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
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		//TODO move this down after filling the model
		model.reset();
	}

	@Test
	public void testTraceModel() {
		//see above
	}

	@Test
	public void testAddModelListener() {
		listener = new MockTraceModelListener();
		model.addModelListener(listener);
		//The model should have getModelListeners() method
		//one way to test is to call one of the notification methods and check the listener 
		//got the notification
		
		//notifyPropertyUpdated, notifyObjectAdded, notifyObjectRemoved, notifyObjectCreationComplete
		try {
			model.notifyPropertyUpdated(null, 0);
			assertTrue(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertTrue(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertTrue(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertTrue(listener.isNotified());
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}

	}

	@Test
	public void testRemoveModelListener() {
		model.removeModelListener(listener);
		// The model should have getModelListeners() method
		// one way to test is to call one of the notification methods and check
		// the removed listener
		// did the notification
		try {
			model.notifyPropertyUpdated(null, 0);
			assertFalse(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertFalse(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertFalse(listener.isNotified());
			listener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertFalse(listener.isNotified());
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testAddExtensionListener() {
		extListener = new TraceModelExtensionListenerMock();
		model.addExtensionListener(extListener);
		//The model should have getModelExtensionListeners() method
		//one way to test is to call one of the notification methods and check the listener 
		//got the notification
		model.notifyExtensionAdded(null, null);
		assertTrue(extListener.isNotified());
		extListener.resetNotification();
		model.notifyExtensionRemoved(null, null);
		assertTrue(extListener.isNotified());
		extListener.resetNotification();

	}

	@Test
	public void testRemoveExtensionListener() {
		//The model should have getModelExtensionListeners() method
		//one way to test is to call one of the notification methods and check the listener 
		//got the notification
		model.removeExtensionListener(extListener);
		try {
			model.notifyPropertyUpdated(null, 0);
			assertFalse(extListener.isNotified());
			listener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertFalse(extListener.isNotified());
			listener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertFalse(extListener.isNotified());
			listener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertFalse(extListener.isNotified());
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testAddResetListener() {
		resetListener = new TraceModelResetListenerMock();
		model.addResetListener(resetListener);
		try {
			model.reset();
			assertTrue(resetListener.isNotified());
			resetListener.resetNotification();
			model.setValid(true);
			assertTrue(resetListener.isNotified());
			resetListener.resetNotification();
			model.setValid(false);
			assertTrue(resetListener.isNotified());
			resetListener.resetNotification();
			model.setValid(false);
			assertFalse(resetListener.isNotified());
			resetListener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}

	}

	@Test
	public void testRemoveResetListener() {
		model.removeResetListener(resetListener);
		try {
			model.notifyPropertyUpdated(null, 0);
			assertFalse(resetListener.isNotified());
			resetListener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertFalse(resetListener.isNotified());
			resetListener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertFalse(resetListener.isNotified());
			resetListener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertFalse(resetListener.isNotified());
			resetListener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}

	}

	@Test
	public void testAddProcessingListener() {
		procListener = new TraceProcessingListenerMock();
		model.addProcessingListener(procListener);
		try {
			model.notifyPropertyUpdated(null, 0);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.startProcessing();
			assertTrue(procListener.isNotified());
			procListener.resetNotification();
			model.processingComplete();
			assertTrue(procListener.isNotified());
			procListener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
	}

	@Test
	public void testRemoveProcessingListener() {
		model.removeProcessingListener(procListener);
		try {
			model.notifyPropertyUpdated(null, 0);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectAdded(null, null);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.notifyObjectCreationComplete(null);
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.startProcessing();
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
			model.processingComplete();
			assertFalse(procListener.isNotified());
			procListener.resetNotification();
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
	}

	@Test
	public void testGetNextGroupID() {
		try {
			//TODO getNextGroupID() needs to be clarified
			// this function returns the next available group ID
			// the algorithm is as follows
			// if there fixed ids the
			//	find the highest group id from [GROUP] or [[OBSOLETE]][GROUP]
			//	if the value is not numeric then just pick 0
			//	find the max group ID already in the model
			//	get the next id after the highest of both.
			//TODO obviously fixed_ids should be parsed only once and max calculated once
			// what happen when there are no fixed ids (deafult is 0 but not documented)
			//also there is an exception raised if the computed max group id > TraceCompilerEngineGlobals.MAX_GROUP_ID
			//TODO must be documented and tested.
				
			//nothing set
			assertEquals(1, model.getNextGroupID());
			
			//set some groups
			//TODO as the model stands, it's possible to add groups with no ids
			//Refactor to only add valid groups to the model
			TraceGroup g1 = new TraceGroup(model);
			g1.setID(30);
			TraceGroup g2 = new TraceGroup(model);
			g2.setID(4444);
			assertEquals(4445, model.getNextGroupID());
			g2.setID(20);
			assertEquals(31, model.getNextGroupID());
			g1.reset();
			assertEquals(21, model.getNextGroupID());
			g2.setID(4444);
			
			//create some fixed is
			SortedProperties fixedIds = new SortedProperties();
			fixedIds.setProperty("[GROUP]g1", "333");
			model.setFixedIds(fixedIds);
			assertEquals(4445, model.getNextGroupID());
			
			
			fixedIds.setProperty("[GROUP]g2", "5000");
			assertEquals(5001, model.getNextGroupID());
			
			
			fixedIds.setProperty("[[OBSOLETE]][GROUP]g3", "234");
			assertEquals(5001, model.getNextGroupID());
			
			
			
			Integer i = Integer.valueOf(TraceCompilerEngineGlobals.MAX_GROUP_ID + 1);
			fixedIds.setProperty("[GROUP]g6", i.toString());
			try {
				model.getNextGroupID();
				fail("Groups should be higher than " + TraceCompilerEngineGlobals.MAX_GROUP_ID);
			} catch (TraceCompilerException exc) {
				//normal behaviour
			}
			
			//if we put any rubish in the properties, the model would return the next gid from the 
			//model not the fixedids
			fixedIds.setProperty("[[OBSOLETE]][GROUP]g4", "rubish");
			assertEquals(4445, model.getNextGroupID());
					
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
	}

	@Test
	public void testGetNextConstantTableID() {
		assertTrue(model.getNextConstantTableID()== 1);
		TraceConstantTable t1 = new TraceConstantTable(model);
		try {
			t1.setID(3);
		} catch (TraceCompilerException e) {
			// TODO this behaviour is a bit strange and not documented
			// this exception is raised when notifying the model listeners
			fail(e.toString());
		}
		assertTrue(model.getNextConstantTableID()== 4);
	}

	@Test
	public void testRemoveGroup() {
		Iterator<TraceGroup> groups = model.getGroups();
		TraceGroup g = null;
		if (groups.hasNext()) {
			g = groups.next();
		}
		groups = null;
		if (g != null) {
			try {
				model.removeGroup(g);
			} catch (TraceCompilerException e) {
				// TODO exception raised by a listener
				fail(e.toString());
			}
			//check the group has been removed
			groups = model.getGroups();
			while (groups.hasNext()) {
				TraceGroup aGroup = groups.next();
				assertNotSame(aGroup, g);
			}
		}
		
		//TODO expose a defect. It's perfectly possible to create duplicate groups
		// and this is due to the fact that groups are ArrayList and the model does not check
		//if the group has already been created.
		model.addGroup(g);
		model.addGroup(g); //duplicate
		groups = model.getGroups();
		while (groups.hasNext()) {
			TraceGroup aGroup = groups.next();
			assertNotSame(aGroup, g);
		}	
	}

	@Test
	public void testHasGroups() {
		assertTrue(model.hasGroups()); //because we have created groups before
		//TODO we are now forced to reset the whole model to remove all groups
		//add a method removeAllGroups() to the TraceModel class. 
		model.reset();
		assertFalse(model.hasGroups());
	}

	@Test
	public void testGetGroupCount() {
		assertEquals(0, model.getGroupCount());
		//add some groups
		TraceGroup g1 = new TraceGroup(model);
		TraceGroup g2 = new TraceGroup(model);
		assertEquals(2, model.getGroupCount());
	}

	@Test
	public void testGetGroups() {
		//TODO this method is not necessary as groups are defined as 
		//ArrayList in TraceModel.
	}

	@Test
	public void testIterator() {
		//TODO model.iterator() should be renamed to model.getGroupIterator() or simply remove it
		// the client can always do model.getGroups().iterator()
		//also model.iterator() as it stands is exactly model.getGroups()
		// so remove it and change getGroups() to return the real groups and protect groups by making it private.
		Iterator<TraceGroup> iterator = model.iterator();
	}

	@Test
	public void testFindGroupByID() {
		model.reset();
		TraceGroup g1 = new TraceGroup(model);
		TraceGroup g2 = new TraceGroup(model);
		TraceGroup g3 = new TraceGroup(model);
		TraceGroup g4 = new TraceGroup(model);
		try {
			g1.setID(1);
			g2.setID(4444);
		} catch (TraceCompilerException e) {
			// TODO Auto-generated catch block
			fail(e.toString());
		}
		
		assertSame(g1, model.findGroupByID(g1.getID()));
		assertSame(g2,model.findGroupByID(g2.getID()));
		assertNull(model.findGroupByID(3333));
		//TODO same defect for duplicates, also it's permissible to add groups with no id
		//or same name
		model.findGroupByID(0); //this would return the first group with no ID i.e. 0
	}

	@Test
	public void testFindGroupByName() {
		model.reset();
		TraceGroup g1 = new TraceGroup(model);
		TraceGroup g2 = new TraceGroup(model);
		TraceGroup g3 = new TraceGroup(model);
		try {
			g1.setID(1);
			g1.setName("group1");
			g2.setID(4444);
		} catch (TraceCompilerException e) {
			// TODO same as above
			fail(e.toString());
		}
		
		assertSame(g1, model.findGroupByName("group1"));
		assertNull(model.findGroupByName("rubish"));
		//TODO same to as for groups and Ids
		model.findGroupByName("");
	}

	@Test
	public void testFindTraceByName() {
		model.reset();
		Trace trace1 = new Trace(new TraceGroup(model));
		Trace trace2 = new Trace(new TraceGroup(model));
		try {
			trace1.setName("Trace1");
			trace2.setName("Trace2");
		} catch (TraceCompilerException e) {
			// TODO same as above
			fail(e.toString());
		}
		assertSame(trace1, model.findTraceByName("Trace1"));
		assertSame(trace2, model.findTraceByName("Trace2"));
		assertNull(model.findTraceByName("rubish"));
		//TODO same issue as above but it's even worse because traces have to belong to groups 
		//so the model should have a list of groups and each group should have a list of traces.
		//fix fix fix
	}

	@Test
	public void testGetGroupAt() {
		model.reset();
		TraceGroup g1 = new TraceGroup(model);
		TraceGroup g2 = new TraceGroup(model);
		TraceGroup g3 = new TraceGroup(model);
		//TODO this is not necessary as groups is just an ArrayList
		assertSame(g1, model.getGroupAt(0));
		assertSame(g2, model.getGroupAt(1));
		assertSame(g3, model.getGroupAt(2));
		try {
			model.removeGroup(g1);
		} catch (TraceCompilerException e) {
			// TODO Auto-generated catch block
			fail(e.toString());
		}
		assertSame(g2, model.getGroupAt(0));
		assertSame(g3, model.getGroupAt(1));
		
		//TODO defect: if we pass an index higher we get IndexOutOfBoudException
		try {
			model.getGroupAt(2);
		} catch (IndexOutOfBoundsException e) {
			fail("Defect: Trying to get a group which is not in the model.");
		}
	}

	@Test
	public void testRemoveConstantTable() {
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
		try {
			t1.setID(1);
			t1.setName("table1");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		
		try {
			model.removeConstantTable(t1);
			//TODO why the model resets the table removed??? bu anyway check the table is reset
			
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		Iterator<TraceConstantTable> constantTables = model.getConstantTables();
		while (constantTables.hasNext()) {
			assertNotSame(t1, constantTables.next());
		}
		try {
			model.removeConstantTable(t1);
		} catch (TraceCompilerException e) {
			// TODO ignore but this is so confusing, the error is coming from the listeners 
			// but the object may have been removed, therefore fail it
			//fail(e.toString());
		}
		
		
	}

	@Test
	public void testGetConstantTables() {
		//TODO not necessary as these tables are ArrayLists
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
		Iterator<TraceConstantTable> constantTables = model.getConstantTables();
		assertEquals(t1, constantTables.next());
		assertEquals(t2, constantTables.next());
		assertEquals(t3, constantTables.next());
		
	}

	@Test
	public void testFindConstantTableByID() {
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
		try {
			t1.setID(1);
			t1.setName("table1");
			t2.setID(2);
			t2.setName("table2");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		assertSame(t1, model.findConstantTableByID(1));
		assertSame(t2, model.findConstantTableByID(2));
		assertSame(t3, model.findConstantTableByID(0));
		//TODO same kind of defect as tables can be duplicates
		
	}

	@Test
	public void testFindConstantTableByName() {
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
		try {
			t1.setID(1);
			t1.setName("table1");
			t2.setID(2);
			t2.setName("table2");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		assertSame(t1, model.findConstantTableByName("table1"));
		assertSame(t2, model.findConstantTableByName("table2"));
		assertSame(t3, model.findConstantTableByName(""));
		//TODO same kind of defect as tables can be duplicates
	}

	@Test
	public void testHasConstantTables() {
		
		assertTrue(model.hasConstantTables());
		model.reset();
		//TODO same as for groups, should have removeAllTables()
		assertFalse(model.hasConstantTables());
	}

	@Test
	public void testGetConstantTableAt() {
		//TODO no need for this as constant tables is an ArrayList
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
		try {
			t1.setID(1);
			t1.setName("table1");
			t2.setID(2);
			t2.setName("table2");
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		assertSame(t1, model.getConstantTableAt(0));
		assertSame(t2, model.getConstantTableAt(1));
		assertSame(t3, model.getConstantTableAt(2));
		try {
			model.removeConstantTable(t1);
		} catch (TraceCompilerException e) {
			// TODO refactore
			fail(e.toString());
		}
		assertSame(t2, model.getConstantTableAt(0));
		assertSame(t3, model.getConstantTableAt(1));
		
		try {
			model.getConstantTableAt(3);
		} catch (IndexOutOfBoundsException e) {
			fail("Defect: tried to get a constant table not in the model.");
		}
		
		
	}

	@Test
	public void testAddGroup() {
		model.reset();
		TraceGroup g1 = new TraceGroup(model);
		try {
			model.removeGroup(g1);
		} catch (TraceCompilerException e) {
			// TODO refactor
			fail(e.toString());
		}
		model.addGroup(g1);
		//TODO this is strange because the apis allow to change the model of the group
		//check the group is added
		Iterator<TraceGroup> groups = model.getGroups();
		assertSame(g1, groups.next());
		assertFalse(groups.hasNext());
	}

	@Test
	public void testAddConstantTable() {
		//TODO method not need if we keep the tables as an Array list
		model.reset();
		TraceConstantTable t1 = new TraceConstantTable(model);
		TraceConstantTable t2 = new TraceConstantTable(model);
		TraceConstantTable t3 = new TraceConstantTable(model);
				
		//check the model has only the tables we added
		Iterator<TraceConstantTable> constantTables = model.getConstantTables();
		assertSame(t1, constantTables.next());
		assertSame(t2, constantTables.next());
		assertSame(t3, constantTables.next());
		assertFalse(constantTables.hasNext());
	}

	@Test
	public void testNotifyPropertyUpdated() {
		model.reset();
		MockTraceModelListener listener = new MockTraceModelListener();
		model.addModelListener(listener);
		try {
			model.notifyPropertyUpdated(null, 0);
			assertTrue(listener.isNotified());
			listener.resetNotification();
		} catch (TraceCompilerException e) {
			//TODO refactore
			fail(e.toString());
		}
	}

	@Test
	public void testNotifyObjectAdded() {
		model.reset();
		MockTraceModelListener listener = new MockTraceModelListener();
		model.addModelListener(listener);
		model.notifyObjectAdded(null, null);
		assertTrue(listener.isNotified());
		listener.resetNotification();
	}

	@Test
	public void testNotifyObjectRemoved() {
		model.reset();
		MockTraceModelListener listener = new MockTraceModelListener();
		model.addModelListener(listener);
		try {
			model.notifyObjectRemoved(new MockTraceObject(), new MockTraceObject());
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		assertTrue(listener.isNotified());
		listener.resetNotification();
	}

	@Test
	public void testNotifyObjectCreationComplete() {
		model.reset();
		MockTraceModelListener listener = new MockTraceModelListener();
		model.addModelListener(listener);
		try {
			model.notifyObjectCreationComplete(null);
		} catch (TraceCompilerException e) {
			fail(e.toString());
		}
		assertTrue(listener.isNotified());
		listener.resetNotification();
	}

	@Test
	public void testNotifyExtensionAdded() {
		model.reset();
		TraceModelExtensionListenerMock listener = new TraceModelExtensionListenerMock();
		model.addExtensionListener(listener);
		model.notifyExtensionAdded(null, null);
		assertTrue(listener.isNotified());
		listener.resetNotification();
	}

	@Test
	public void testNotifyExtensionRemoved() {
		model.reset();
		TraceModelExtensionListenerMock listener = new TraceModelExtensionListenerMock();
		model.addExtensionListener(listener);
		model.notifyExtensionRemoved(null, null);
		assertTrue(listener.isNotified());
		listener.resetNotification();
	}

	@Test
	public void testIsValid() {
		model.reset();
		TraceModelResetListenerMock listener = new TraceModelResetListenerMock();
		model.addResetListener(listener);
		try {
			model.setValid(false);
			assertFalse(model.isValid());
			model.setValid(true);
			assertTrue(model.isValid());
			assertTrue(listener.isNotified());
			listener.resetNotification();
			model.setValid(true);
			assertTrue(model.isValid());
			assertFalse(listener.isNotified());
			model.setValid(false);
			model.reset();
			assertTrue(model.isValid()); // Defect should set model to valid at reset
		} catch (TraceCompilerException e) {
			// TODO refactor
			fail(e.toString());
		}
		
	}

	@Test
	public void testSetValid() {
		//tested above
	}

	@Test
	public void testGetFactory() {
		assertSame(factory, model.getFactory().getRuleFactory());
	}

	@Test
	public void testGetVerifier() {
		assertSame(v, model.getVerifier());
	}

	@Test
	public void testStartProcessing() {
		model.reset();
		TraceProcessingListenerMock l = new TraceProcessingListenerMock();
		model.addProcessingListener(l);
		model.startProcessing();
		assertTrue(l.isNotified());
		assertTrue(model.isProcessing());
		assertFalse(model.isComplete()); //defect can not be processing and completed procession at the same time
	}

	@Test
	public void testProcessingComplete() {
		model.processingComplete();
		assertTrue(model.isComplete());
		assertFalse(model.isProcessing());
	}

	@Test
	public void testIsProcessing() {
		//done above
	}

	@Test
	public void testHasTraces() {
		model.reset();
		assertFalse(model.hasTraces());
		TraceGroup g1 = new TraceGroup(model);
		Trace t1 = new Trace(g1);
		assertTrue(model.hasTraces());
	}

	@Test
	public void testGetGroupID() {
		model.reset();
		assertFalse(model.hasTraces());
		TraceGroup g1 = new TraceGroup(model);
		Trace t1 = new Trace(g1);
		try {
			g1.setName("group1");
			g1.setID(1);
		} catch (TraceCompilerException e) {
			// TODO Refactore
			fail(e.toString());
		}
		
		Properties p = new Properties();
		p.put(model.GROUP_PROPERTY_PREFIX + g1.getName() , g1.getID());
		try {
			assertEquals(g1.getID(), model.getGroupID(p, g1)); //TODO something strange about this test failing
			g1.setName("");
			assertEquals(2, model.getGroupID(p, g1));
			p.clear();
			assertEquals(2, model.getGroupID(p, g1));
			//TODO make this api simler and safer
		} catch (TraceCompilerException e) {
			// TODO refactore
			fail(e.toString());
		}
		fail("Not yet implemented");
	}

	@Test
	public void testSaveIDs() {
		model.reset();
		TraceGroup g1 = new TraceGroup(model); //no id
		TraceGroup g2 = new TraceGroup(model); //no name
		TraceGroup g3 = new TraceGroup(model); //no prop
		TraceGroup g4 = new TraceGroup(model); 
		Trace t1 = new Trace(g1);
		Trace t2 = new Trace(g1); //no id
		Trace t3 = new Trace(g2);
		Trace t4 = new Trace(g2); //no prop
		Trace t5 = new Trace(g2); //no name
		
		try {
			
			g1.setName("group1");
			g2.setID(2);
			g4.setName("group4");
			g4.setID(4);
			
			t1.setName("trace1");
			t1.setID(1);
			
			t2.setName("trace2");
			
			t3.setName("trace3");
			t3.setID(3);
			
			t5.setID(5);
			
		} catch (TraceCompilerException e) {
			
			fail(e.toString());
		}
		Properties p = new Properties();
		model.saveIDs(p);
		//[GROUP]=0x0					--- g3
		//[GROUP]group1=0x0				--- g1
		//[GROUP]group4=0x4 			--- g4
		//[TRACE][0x2]_=0x5 			--- g2-t5
		//[TRACE][0x2]_trace3=0x3 		--- g2-t3
		//[TRACE]group1[0x0]_trace2=0x0 --- g1-t2
		//[TRACE]group1[0x0]_trace1=0x1 --- g1-t1
		assertEquals(7, p.size());
		assertEquals(SourceConstants.HEX_PREFIX + g1.getID(), p.getProperty(model.GROUP_PROPERTY_PREFIX + g1.getName()));
		//assertEquals(SourceConstants.HEX_PREFIX + g2.getID(), p.getProperty(model.GROUP_PROPERTY_PREFIX + g2.getName()));
		assertEquals(SourceConstants.HEX_PREFIX + g3.getID(), p.getProperty(model.GROUP_PROPERTY_PREFIX + g3.getName()));
		assertEquals(SourceConstants.HEX_PREFIX + g4.getID(), p.getProperty(model.GROUP_PROPERTY_PREFIX + g4.getName()));
		StringBuffer sb = new StringBuffer();
		String createdTraceName = model.TRACE_PROPERTY_PREFIX + model.createTraceName(sb, g1, t1);
		assertEquals(SourceConstants.HEX_PREFIX + t1.getID(), p.getProperty(createdTraceName));
		createdTraceName = model.TRACE_PROPERTY_PREFIX + model.createTraceName(sb, g1, t2);
		assertEquals(SourceConstants.HEX_PREFIX + t2.getID(), p.getProperty(createdTraceName));
		createdTraceName = model.TRACE_PROPERTY_PREFIX + model.createTraceName(sb, g2, t3);
		assertEquals(SourceConstants.HEX_PREFIX + t3.getID(), p.getProperty(createdTraceName));
		//assertEquals(SourceConstants.HEX_PREFIX + t1.getID(), p.getProperty(model.TRACE_PROPERTY_PREFIX + t4.getName())); this trace seems to have been excluded
		createdTraceName = model.TRACE_PROPERTY_PREFIX + model.createTraceName(sb, g2, t5);
		assertEquals(SourceConstants.HEX_PREFIX + t5.getID(), p.getProperty(createdTraceName));
		//TODO this is probably the most unclear API. please put specs, checks and doc.
		
	}

	@Test
	public void testCreateTraceName() {
		model.reset();
		StringBuffer sb = new StringBuffer();
		TraceGroup g = new TraceGroup(model);
		Trace t = new Trace(g);
		
		try {
			g.setName("testgroup");
			g.setID(1);
			t.setName("testtrace");
			t.setID(222);
		} catch (TraceCompilerException e) {
			// TODO refactore
			fail(e.toString());
		}
		String createdTraceName = model.createTraceName(sb, g, t);
		assertEquals("testgroup[0x1]_testtrace", createdTraceName);
	}

	@Test
	public void testGetFixedIds() {
		model.reset();
		SortedProperties p = new SortedProperties();
		p.put("key1", "value1");
		p.put("key3", "value3");
		p.put("key2", "value2");
		model.setFixedIds(p);
		assertEquals(p, model.getFixedIds());
	}

	@Test
	public void testSetFixedIds() {
		//no need see above
	}

	@Test
	public void testGetNextTraceId() {
		model.reset();
		TraceGroup g = new TraceGroup(model);
		TraceGroup g2 = new TraceGroup(model);
		try {
			g.setName("group1");
			g.setID(1);
			g2.setName("group2");
			g2.setID(2);
			assertEquals(1, model.getNextTraceId(g));
			//add a trace
			Trace t = new Trace(g);
			t.setID(4);
			assertEquals(5, (model.getNextTraceId(g)));
			assertEquals(1, model.getNextTraceId(g2));
			//TODO add more tests to test the case where the next Id comes from the 
			//fixed ids.
			
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
	
	class TraceModelExtensionListenerMock implements TraceModelExtensionListener {
		boolean notified = false;
		
		public void extensionAdded(TraceObject object,
				TraceModelExtension extension) {
			notified = true;
			
		}

		
		public void extensionRemoved(TraceObject object,
				TraceModelExtension extension) {
			notified = true;
			
		}
		
		public boolean isNotified() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
		}
	}
	
	class TraceModelResetListenerMock implements TraceModelResetListener {
		boolean notified = false;
		
		public void modelReset() {
			notified = true;
			
		}

		
		public void modelResetting() {
			notified = true;
			
		}

		
		public void modelValid(boolean valid) throws TraceCompilerException {
			notified = true;
			
		}
		
		public boolean isNotified() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
		}
	}
	
	class TraceProcessingListenerMock implements TraceProcessingListener {
		boolean notified = false;
		
		public void processingComplete(boolean changed) {
			notified = true;
			
		}

		
		public void processingStarted() {
			notified = true;
			
		}
		
		public boolean isNotified() {
			return notified;
		}
		
		public void resetNotification() {
			notified = false;
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

}
