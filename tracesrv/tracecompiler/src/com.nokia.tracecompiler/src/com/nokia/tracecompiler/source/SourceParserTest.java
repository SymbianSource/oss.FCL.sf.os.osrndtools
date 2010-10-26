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
* Test Class for SourceParser: Parser for C++ source files
* 
* Note : more test cases are needed to cover various c++ syntax test cases.
*
*/

package com.nokia.tracecompiler.source;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.List;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.StringDocumentFactory;

public class SourceParserTest {
	
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
	public void testSourceParserSourceDocumentFactoryString() {
		//constructor SourceParser(SourceDocumentFactory, String)
		SourceDocumentFactory factory = new StringDocumentFactory();
		//Note: Normally the parser would work the same whether it's created from string or file
		//so to allow various use cases to be added in the future, we are using SourceParser with
		//string source data.
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceParser parser = new SourceParser(factory, data);
			assertNotNull(parser);
			assertEquals(data.length(), parser.getDataLength());
			assertEquals(data, parser.getData(0, parser.getDataLength()));
			
			//TODO defect if we pass non-valid index, the code just throws StringIndexOutofBoundException
			//while the function claims to return null if nothing can be found
			//assertNull(parser.getData(10000000, parser.getDataLength()));
			//assertNull(parser.getData(0, 1000000));
			
		}
	}

	@Test
	public void testSourceParserSourceDocumentFactorySourceDocumentInterface() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			assertEquals(data.length(), parser.getDataLength());
			assertEquals(data, parser.getData(0, parser.getDataLength()));
		}
	}

	@Test
	public void testGetSource() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			assertSame(sourceDocument, parser.getSource());
			//see defect above about invalid index
		}
	}

	@Test
	public void testGetDataIntInt() {
			String data = MockTracesUseCases.testCases[1];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			assertEquals("CHelloTraceFn()", parser.getData(15, 15));
			//see defect above about invalid index
	}

	@Test
	public void testGetDataInt() {
		String data;
		
		for (int j = 0; j < MockTracesUseCases.testCases.length; j++) {
			data = MockTracesUseCases.testCases[j];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			for (int i = 0; i < data.length(); i++) {
				assertEquals(data.charAt(i), parser.getData(i));
			}
			//see defect above about invalid index
			//assertNull(parser.getData(10000000));
		}
		
	}

	@Test
	public void testGetDataLength() {
		
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			assertEquals(data.length(), parser.getDataLength());
		}	
	}

	@Test
	public void testGetLineNumber() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		assertEquals(1, parser.getLineNumber(0));
		assertEquals(1, parser.getLineNumber(20));
		assertEquals(1, parser.getLineNumber(30));
		assertEquals(2, parser.getLineNumber(31));
		
		assertEquals(2, parser.getLineNumber(32));
		assertEquals(2, parser.getLineNumber(33));
		
		assertEquals(2, parser.getLineNumber(34));
		assertEquals(8, parser.getLineNumber(220));
		
		//TODO defect, the function is claiming to return -1 but it's returning 0
		//reason: SourParser line 211 retval = source.getLineOfOffset(offset) + 1;
		//is adding one without checking what it has from source.getLineOfOffset(offset) 
		//assertEquals(-1, parser.getLineNumber(100000));
		//add more tests
		
	}

	@Test
	public void testStartStringSearch() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		SourceStringSearch startStringSearch = parser.startStringSearch("OstTrace", 0, data.length() -1, SourceParser.SKIP_ALL );
		
		assertEquals(127, startStringSearch.findNext()); //line 7
		assertEquals(218, startStringSearch.findNext()); //line 8
		
		assertEquals(-1, startStringSearch.findNext());
		
		//search for something that does not exist
		startStringSearch = parser.startStringSearch("Rubish", 0, data.length() -1, SourceParser.SKIP_ALL );
		assertEquals(-1, startStringSearch.findNext());
		
		//search case insensitive
		startStringSearch = parser.startStringSearch("osttrace", 0, data.length() -1, SourceParser.IGNORE_CASE | SourceParser.SKIP_ALL );
		assertEquals(127, startStringSearch.findNext()); //line 7
		assertEquals(218, startStringSearch.findNext()); //line 8
		
		//search 
		startStringSearch = parser.startStringSearch("OstTrace", 0, data.length() -1, SourceParser.SKIP_COMMENTS );
		assertEquals(127, startStringSearch.findNext()); //line 7
		assertEquals(218, startStringSearch.findNext()); //line 8
		
		
		//TODO add more test cases here
		
		
	}

	@Test
	public void testTokenizeParameters() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		SourceStringSearch startStringSearch = parser.startStringSearch("OstTrace", 0, data.length() -1, SourceParser.SKIP_ALL );
		
		int index = startStringSearch.findNext();
		List<String> listOfParameters = new ArrayList<String>();;
		try {
			int offsetOfEndOfParameters = parser.tokenizeParameters(index, listOfParameters, true);
			assertEquals(4, listOfParameters.size());
			assertEquals("TRACE_NORMAL", listOfParameters.get(0));
			assertEquals("CHELLOTRACEFN_CHELLOTRACEFN_CTOR", listOfParameters.get(1));
			assertEquals("\"[0x%08x] Constructor\"", listOfParameters.get(2));
			assertEquals("this", listOfParameters.get(3));
			assertEquals(216, offsetOfEndOfParameters);
			
		} catch (SourceParserException e) {
			fail("could not determine parameterlist" +e.toString());
		}
		
		//TODO add more test cases here
	}

	@Test
	public void testParseFunctionParameters() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		List<SourceParameter> parameterList = new ArrayList<SourceParameter>();
		try {
			parser.parseFunctionParameters(100, parameterList);
			assertEquals(0, parameterList.size());
			data = MockTracesUseCases.testCases[4];
			sourceDocument = factory.createDocument(data);
			parser = new SourceParser(factory, sourceDocument);
			assertNotNull(parser);
			parser.parseFunctionParameters(0, parameterList);
			assertEquals(1, parameterList.size());
			SourceParameter p = parameterList.get(0);
			assertEquals("aActive", p.getName());
			assertEquals(1,p.getPointerCount());
			assertEquals("CActive", p.getType());
			
			
			//TODO more test cases are needed here
			
			
		} catch (SourceParserException e) {
			fail("Could not build list of parameters. "  + e.toString());
		} 
	}

	@Test
	public void testParseReturnValues() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		SourceContext context = parser.getContext(130);
		List<SourceReturn> returnList = new ArrayList<SourceReturn>();
		parser.parseReturnValues(context, returnList);
		
		//TODO defect the constructor has no return so simply should return empty list
		//assertEquals(0, returnList.size());
		
		
		data = MockTracesUseCases.testCases[3];
		sourceDocument = factory.createDocument(data);
		parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		context = parser.getContext(130);
		returnList = new ArrayList<SourceReturn>();
		parser.parseReturnValues(context, returnList);
		assertEquals(1, returnList.size());
		SourceReturn r = returnList.get(0);
		assertEquals(239, r.getOffset());
		assertEquals(3, r.getLength()); //ret
		assertEquals("ret",parser.getData(239, 3));
		
		data = MockTracesUseCases.testCases[4];
		sourceDocument = factory.createDocument(data);
		parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		context = parser.getContext(100);
		returnList = new ArrayList<SourceReturn>();
		parser.parseReturnValues(context, returnList);
		assertEquals(1, returnList.size());
		r = returnList.get(0);
		assertEquals(249, r.getOffset());
		assertEquals(7, r.getLength()); //CActive
		assertEquals("aActive",parser.getData(249, 7));
		
		//TODO more test cases are needed here	
		
	}

	@Test
	public void testIsInExcludedArea() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		assertFalse(parser.isInExcludedArea(1));
		assertTrue(parser.isInExcludedArea(32));
		assertTrue(parser.isInExcludedArea(74));
		assertFalse(parser.isInExcludedArea(127));
		
		//TODO more test cases are needed here
		
	}

	@Test
	public void testGetExcludedArea() {
		String data = MockTracesUseCases.testCases[5];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		List<SourceExcludedArea> excludedAreas = parser.getExcludedAreas();
		assertEquals(5, excludedAreas.size());
		assertNull(parser.getExcludedArea(20));
		assertNull(parser.getExcludedArea(200));
		
		//TODO a lot more tests are needed here
	}

	@Test
	public void testCreateIterator() {
		String data = MockTracesUseCases.testCases[3];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		SourceIterator iter = parser.createIterator(0, SourceParser.SKIP_ALL);
		assertNotNull(iter);
		
		//TODO add more tests by trying the iteration
		
	}

	@Test
	public void testGetContext() {
		//done above
	}

	@Test
	public void testFindStartOfLine() {
		String data = MockTracesUseCases.testCases[1];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		try {
			int startOfLine = parser.findStartOfLine(300, false, true);
			assertEquals(218, startOfLine);
		} catch (SourceParserException e) {
			fail("Failed to find start of the line " + e.toString());
		}
		
	}

	@Test
	public void testFindExcludedAreaIndex() {
		String data = MockTracesUseCases.testCases[5];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		assertEquals( -2, parser.findExcludedAreaIndex(20));
		assertEquals(0, parser.findExcludedAreaIndex(10));
		assertEquals(-5, parser.findExcludedAreaIndex(300));
	}

	@Test
	public void testFindExcludedAreas() {
		String data = MockTracesUseCases.testCases[5];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		try {
			parser.findExcludedAreas();
			List<SourceExcludedArea> list = parser.getExcludedAreas();
			assertEquals(5, list.size());
			//TODO add more assertions here
			
		} catch (SourceParserException e) {
			fail("failed to find excluded areas. " + e.toString());
		}
		
	}

	@Test
	public void testGetExcludedAreas() {
		//done above
	}

	@Test
	public void testIsExcluded() {
		//test 
		//SourceParser.SKIP_STRING SourceExcludedArea.STRING > string
		//SourceParser.SKIP_COMMENTS SourceExcludedArea.MULTILINE_COMMENT > comment
		//SourceParser.SKIP_COMMENTS SourceExcludedArea.LINE_COMMENT > linecomment
		//SourceParser.SKIP_PREPROCESSOR SourceExcludedArea.PREPROCESSOR_DEFINITION > preProcessor
		
		assertTrue(SourceParser.isExcluded(SourceExcludedArea.LINE_COMMENT, SourceParser.SKIP_COMMENTS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.LINE_COMMENT, SourceParser.SKIP_STRINGS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.LINE_COMMENT, SourceParser.SKIP_PREPROCESSOR));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.LINE_COMMENT, SourceParser.SKIP_WHITE_SPACES));
		
		assertTrue(SourceParser.isExcluded(SourceExcludedArea.MULTILINE_COMMENT, SourceParser.SKIP_COMMENTS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.MULTILINE_COMMENT, SourceParser.SKIP_PREPROCESSOR));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.MULTILINE_COMMENT, SourceParser.SKIP_STRINGS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.MULTILINE_COMMENT, SourceParser.SKIP_WHITE_SPACES));
		
		assertTrue(SourceParser.isExcluded(SourceExcludedArea.STRING, SourceParser.SKIP_STRINGS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.STRING, SourceParser.SKIP_PREPROCESSOR));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.STRING, SourceParser.SKIP_COMMENTS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.STRING, SourceParser.SKIP_WHITE_SPACES));
		
		assertTrue(SourceParser.isExcluded(SourceExcludedArea.PREPROCESSOR_DEFINITION, SourceParser.SKIP_PREPROCESSOR));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.PREPROCESSOR_DEFINITION, SourceParser.SKIP_COMMENTS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.PREPROCESSOR_DEFINITION, SourceParser.SKIP_STRINGS));
		assertFalse(SourceParser.isExcluded(SourceExcludedArea.PREPROCESSOR_DEFINITION, SourceParser.SKIP_WHITE_SPACES));
		
	}

	@Test
	public void testAddLocation() {
		
		//TODO there is no way to find out if a locaion has been added or not
		//these locations in the parser are not used anywhere so no test is required
		//and they need to be deleted.
	}

	@Test
	public void testRemoveLocation() {
		//TODO there is no way to find out if a locaion has been added or not
		//these locations in the parser are not used anywhere so no test is required
		//and they need to be deleted.
	}

	@Test
	public void testGetDocumentFramework() {
		String data = MockTracesUseCases.testCases[3];
		SourceDocumentFactory factory = new StringDocumentFactory();
		SourceDocumentInterface sourceDocument = factory.createDocument(data);
		SourceParser parser = new SourceParser(factory, sourceDocument);
		assertNotNull(parser);
		assertSame(factory, parser.getDocumentFramework());
	}

}
