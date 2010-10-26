/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). 
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
* Test Class for SourceExcludedArea
*
*/
package com.nokia.tracecompiler.source;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class SourceExcludedAreaTest {

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
	public void testSourceExcludedArea() {
		SourceExcludedArea sxArea = new SourceExcludedArea(null, 0, SourceExcludedArea.CHARACTER);
		assertNotNull(sxArea);
		//TODO there is a problem with this constructor
		//the type passed in is not checked at all
		//Also because SourceExcludedArea is subclassing SourceLocationBase, anyone can change the offset and the length
		//of the area
		//this class could override these methods to do nothing to protect these fields being changed.
		
	}

	@Test
	public void testGetType() {
		SourceExcludedArea sxArea = new SourceExcludedArea(null, 20, SourceExcludedArea.CHARACTER);
		assertNotNull(sxArea);
		assertEquals(SourceExcludedArea.MULTILINE_COMMENT, sxArea.getType());
		assertEquals(20, sxArea.getOffset());
	}

}
