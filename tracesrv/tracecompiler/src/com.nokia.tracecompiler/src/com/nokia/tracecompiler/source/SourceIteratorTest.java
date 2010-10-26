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
* Test Class for SourceIterator
*
*/

//TODO more tests are needed here

package com.nokia.tracecompiler.source;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.StringDocumentFactory;

public class SourceIteratorTest {

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
	public void testSourceIterator() {
		//TODO various test cases need to be added here
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
			assertNotNull(sIter);
		}
	}

	@Test
	public void testHasNext() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		assertTrue(sIter.hasNext());
		sIter = new SourceIterator(parser, data.length() -1, SourceParser.SKIP_ALL);
		//TODO defect this should return false
		assertFalse(sIter.hasNext());
	}

	@Test
	public void testNext() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		try {
			assertEquals('C', sIter.next()); // "CHelloTraceFn::CHelloTraceFn()" + '\n'
			assertEquals('H', sIter.next());
			assertEquals('e', sIter.next());
			assertEquals('l', sIter.next());
			assertEquals('l', sIter.next());
			assertEquals('o', sIter.next());
			assertEquals('T', sIter.next());
			assertEquals('r', sIter.next());
			assertEquals('a', sIter.next());
			assertEquals('c', sIter.next());
			assertEquals('e', sIter.next());
			assertEquals('F', sIter.next());
			assertEquals('n', sIter.next());
			assertEquals(':', sIter.next());
			assertEquals(':', sIter.next());
			assertEquals('C', sIter.next());
			assertEquals('H', sIter.next());
			assertEquals('e', sIter.next());
			assertEquals('l', sIter.next());
			assertEquals('l', sIter.next());
			assertEquals('o', sIter.next());
			assertEquals('T', sIter.next());
			assertEquals('r', sIter.next());
			assertEquals('a', sIter.next());
			assertEquals('c', sIter.next());
			assertEquals('e', sIter.next());
			assertEquals('F', sIter.next());
			assertEquals('n', sIter.next());
			assertEquals('(', sIter.next());
			assertEquals(')', sIter.next());
			assertFalse(sIter.hasSkipped());
			assertEquals('/', sIter.next()); //TODO this is a genuine defect, it should have returned { here and 
			//all the characters between ) and { should have been skipped.
			//for the moment it's getting the next index after skipping just one area so if there are contiguous skippable areas
			// it return the beginning of the next one
			assertTrue(sIter.hasSkipped());
			assertEquals('{', sIter.next());
			assertTrue(sIter.hasSkipped());
		} catch (SourceParserException e) {
			fail("failed to get next from source iterator. " + e.toString());
		}
	}

	@Test
	public void testNextIndex() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		assertEquals(0, sIter.nextIndex());
		sIter = new SourceIterator(parser, 29, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		assertEquals(29, sIter.nextIndex());
		try {
			sIter.next();
			assertEquals(73, sIter.nextIndex()); //check it's { after skipping all areas but see defect above
			sIter.next();
			assertEquals('{', sIter.next());
		} catch (SourceParserException e) {
			fail("failed to get next from source iterator. " + e.toString());
		}
		
	}

	@Test
	public void testCurrentIndex() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		assertEquals(0, sIter.currentIndex());
		assertEquals(0, sIter.previousIndex());
		try {
			sIter.next();
			assertEquals(0, sIter.currentIndex()); //check it's { after skipping all areas but see defect above
			assertEquals(0, sIter.previousIndex());
			sIter.next();
			assertEquals(1, sIter.currentIndex());
			assertEquals(0, sIter.previousIndex());
			sIter = new SourceIterator(parser, 29, SourceParser.SKIP_ALL);
			assertEquals(29, sIter.currentIndex());
			assertEquals(29, sIter.previousIndex());
			sIter.next();
			assertEquals(29, sIter.currentIndex());
			assertEquals(29, sIter.previousIndex()); //TODO this is strange behaviour, it should be 28
			sIter.next();
			assertEquals(73, sIter.currentIndex());
			assertEquals(29, sIter.previousIndex());
			assertEquals('{', sIter.next());
			assertEquals(125, sIter.currentIndex());
			assertEquals(73, sIter.previousIndex());
		} catch (SourceParserException e) {
			fail("failed to get next from source iterator. " + e.toString());
		}
	}

	@Test
	public void testPreviousIndex() {
		//see above
	}

	@Test
	public void testPeek() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		SourceIterator sIter = new SourceIterator(parser, 0, SourceParser.SKIP_ALL);
		assertNotNull(sIter);
		try {
			assertEquals('C', sIter.peek());
			sIter = new SourceIterator(parser, 29, SourceParser.SKIP_ALL);
			sIter.next();
			assertEquals('/', sIter.peek());
			sIter = new SourceIterator(parser, 30, SourceParser.SKIP_ALL);
			sIter.next();
			assertEquals('{', sIter.peek());
		} catch (SourceParserException e) {
			fail("failed to peek from source iterator. " + e.toString());
		}
	}

	@Test
	public void testHasSkipped() {
		//see above
	}

}
