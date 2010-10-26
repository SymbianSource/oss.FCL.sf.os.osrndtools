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
* Test Class for FunctioneturnValueParser
*
*/

package com.nokia.tracecompiler.source;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.List;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.document.StringDocumentFactory;

public class FunctionReturnValueParserTest {

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
	public void testFunctionReturnValueParser() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			//constructor
			FunctionReturnValueParser frvParser = new FunctionReturnValueParser(parser);
			assertNotNull(frvParser);
		}
	}

	@Test
	public void testParseReturnValues() {
		for (int i = 0; i < MockTracesUseCases.testCases.length; i++) {
			String data = MockTracesUseCases.testCases[i];
			SourceDocumentFactory factory = new StringDocumentFactory();
			SourceDocumentInterface sourceDocument = factory.createDocument(data);
			SourceParser parser = new SourceParser(factory, sourceDocument);
			
			ContextAreaParser ctxParser = new ContextAreaParser(parser);
			try {
				ctxParser.parseAll();
			} catch (SourceParserException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			List<SourceContext> contextList = ctxParser.getContextList();
			
			for (SourceContext sourceContext : contextList) {
				List<SourceReturn> retList = new ArrayList<SourceReturn>();
				FunctionReturnValueParser frvParser = new FunctionReturnValueParser(parser);
				assertNotNull(frvParser);
				frvParser.parseReturnValues(sourceContext, retList);
				//TODO, it seems that parseReturnValue return a SourceReturn Object even when it shoud not such as
				//in constructors, it return an object with position : length 0, offset : the end of the context.
				//why it does not simply return null to say there is no return statement.
				
				//assess creation of these ret values
				
				switch (i) {
				case 1:
				case 2:
				case 5:
					assertEquals(1, retList.size());
					assertEquals(0, retList.get(0).getLocation().getLength());
					assertEquals(data.length() -1, retList.get(0).getLocation().getOffset());
					break;
				case 3:
					assertEquals(1, retList.size());
					assertEquals(3, retList.get(0).getLocation().getLength());  //ret
					assertEquals(239, retList.get(0).getLocation().getOffset());
					break;
				case 4:
					assertEquals(1, retList.size());
					assertEquals(7, retList.get(0).getLocation().getLength()); //aActive
					assertEquals(249, retList.get(0).getLocation().getOffset());
					break;

				default:
					break;
				}
			}
		}
	}

	@Test
	public void testFindLast() {
		//TODO FindLast is not called by anyone so remove from production code
		//if a user needs to find out about the last return context, (s)he can have from the list passed in
		//parseReturnValues.
		
		//TODO: there a returnextensions list in FunctionReturnValueParser which is not set by anyone
		//so remove from production code.
	}

}
