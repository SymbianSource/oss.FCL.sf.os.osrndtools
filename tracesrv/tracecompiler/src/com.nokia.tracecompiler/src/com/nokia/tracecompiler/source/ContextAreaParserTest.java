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
* Test Class for SourceAreaParser
* 
* Note : more test cases are needed to cover various c++ syntax test cases.
*
*/

package com.nokia.tracecompiler.source;

import static org.junit.Assert.*;

import java.util.Iterator;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.StringDocumentFactory;

public class ContextAreaParserTest {

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
	public void testContextAreaParser() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			ContextAreaParser context = new ContextAreaParser(parser);
			assertNotNull(context);
			assertEquals(0, context.getContextList().size());
		}
	}

	@Test
	public void testReset() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			ContextAreaParser context = new ContextAreaParser(parser);
			assertNotNull(context);
			assertEquals(0, context.getContextList().size());
			try {
				context.parseAll();
				//TODO the test cases have to be made more complex.
				switch (i) {
				case 0:
					assertEquals(0, context.getContextList().size());
					break;
				case 1:
					assertEquals(1, context.getContextList().size());
					break;
				case 2:
					assertEquals(1, context.getContextList().size());
					break;
				case 3:
					assertEquals(1, context.getContextList().size());
					break;
				case 4:
					assertEquals(1, context.getContextList().size());
					break;
				case 5:
					assertEquals(1, context.getContextList().size());
					break;
				default:
					break;
				}
				
				context.reset();
				assertEquals(0, context.getContextList().size());
				
				
			} catch (SourceParserException e) {
				fail("Failed to parse test case " + e.toString());
			}
		}
		
	}

	@Test
	public void testParseAndGet() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			ContextAreaParser context = new ContextAreaParser(parser);
			assertNotNull(context);
			assertEquals(0, context.getContextList().size());
			try {
				SourceContext currentContext = context.parseAndGet(data.length() / 2); //the middle but this can also be improved on case by case basis
				//TODO the test cases have to be made more complex.
				switch (i) {
				case 0:
					assertEquals(null, currentContext);
					break;
				case 1:
					assertEquals("CHelloTraceFn", currentContext.getFunctionName());
					break;
				case 2:
					assertEquals("Simple", currentContext.getFunctionName());
					break;
				case 3:
					assertEquals("OutputsTIntReturnValue", currentContext.getFunctionName());
					break;
				case 4:
					assertEquals("OutputsUnknownPtrType", currentContext.getFunctionName());
					break;
				case 5:
					assertEquals("CHelloTraceFn", currentContext.getFunctionName());
					break;
				default:
					break;
				}
				context.reset();
				assertEquals(0, context.getContextList().size());
			} catch (SourceParserException e) {
				fail("Failed to parse test case " + e.toString());
			}
		}
	}

	@Test
	public void testParseAndGetAll() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			ContextAreaParser context = new ContextAreaParser(parser);
			assertNotNull(context);
			assertEquals(0, context.getContextList().size());
			try {
				Iterator<SourceContext> contexts = context.parseAndGetAll(); //the middle but this can also be improved on case by case basis
				//TODO the test cases have to be made more complex.
				//TODO there is no need to return Itertor because SourceContext list is an ArrayList
				switch (i) {
				case 0:
					assertFalse(contexts.hasNext());
					break;
				case 1:
					assertEquals("CHelloTraceFn", contexts.next().getFunctionName());
					break;
				case 2:
					assertEquals("Simple", contexts.next().getFunctionName());
					break;
				case 3:
					assertEquals("OutputsTIntReturnValue", contexts.next().getFunctionName());
					break;
				case 4:
					assertEquals("OutputsUnknownPtrType", contexts.next().getFunctionName());
					break;
				case 5:
					assertEquals("CHelloTraceFn", contexts.next().getFunctionName());
					break;
				default:
					break;
				}				
				assertFalse(contexts.hasNext());
				
				context.reset();
				assertEquals(0, context.getContextList().size());
			} catch (SourceParserException e) {
				fail("Failed to parse test case " + e.toString());
			}
		}
	}

	@Test
	public void testGetContextList() {
		//test above
	}

	@Test
	public void testFind() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			ContextAreaParser context = new ContextAreaParser(parser);
			assertNotNull(context);
			assertEquals(0, context.getContextList().size());
			try {
				context.parseAll();
				int currentContextIndex = context.find(data.length() / 2); //the middle but this can also be improved on case by case basis
				//TODO the test cases have to be made more complex.
				switch (i) {
				case 0:
					assertEquals(-1, currentContextIndex);
					break;
				case 1:
					assertEquals(0, currentContextIndex);
					break;
				case 2:
					assertEquals(0, currentContextIndex);
					break;
				case 3:
					assertEquals(0, currentContextIndex);
					break;
				case 4:
					assertEquals(0, currentContextIndex);
					break;
				case 5:
					assertEquals(0, currentContextIndex);
					break;
				default:
					break;
				}
				context.reset();
				assertEquals(0, context.getContextList().size());
			} catch (SourceParserException e) {
				fail("Failed to parse test case " + e.toString());
			}
		}
	}

	@Test
	public void testParseAll() {
		//tested above
	}

}
