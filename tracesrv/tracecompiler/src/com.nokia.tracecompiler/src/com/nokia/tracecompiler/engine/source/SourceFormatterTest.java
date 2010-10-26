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
 *
 */


package com.nokia.tracecompiler.engine.source;

import static org.junit.Assert.*;

import java.util.ArrayList;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nokia.tracecompiler.engine.TraceCompilerEngineGlobals;
import com.nokia.tracecompiler.engine.rules.osttrace.OstTraceFormatRule;
import com.nokia.tracecompiler.model.Trace;
import com.nokia.tracecompiler.model.TraceCompilerException;
import com.nokia.tracecompiler.model.TraceGroup;
import com.nokia.tracecompiler.model.TraceModel;
import com.nokia.tracecompiler.model.TraceModelExtension;
import com.nokia.tracecompiler.model.TraceObjectFactory;
import com.nokia.tracecompiler.plugin.TraceAPIFormatter.TraceFormatType;

public class SourceFormatterTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TraceCompilerEngineGlobals.start();		
	}

	@Test
	public void testFormatTraceTraceTraceFormatType() throws TraceCompilerException {
				
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();
		OstTraceFormatRule ostFormatRule = new OstTraceFormatRule(); 
		
		// note these tests use OstFormatRule
		model.addExtension(ostFormatRule);
		
		TraceObjectFactory factory = model.getFactory();

		final int id = 999;
		String name = "hello_hello";  		
		TraceModelExtension[] extensions = null;

		TraceGroup traceGroup = factory.createTraceGroup(id, name, extensions );		
		Trace trace = factory.createTrace(traceGroup, id, name, "traceText", extensions);
		
		String str= SourceFormatter.formatTrace(trace, TraceFormatType.EMPTY_MACRO);
		assertTrue(str.equals(""));		
				
		str = SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_ACTIVATION);
		assertTrue(str.equals("BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID )\r\n"));		

		str = SourceFormatter.formatTrace(trace, TraceFormatType.HEADER);		
		assertTrue(str.equals("OstTraceGen0( TUint32 aTraceID )\r\n"));
		
		str= SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_BUFFER);	
		assertTrue(str.equals("OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA%, %LENGTH% );\r\n"));
		
		str= SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_PACKED);
		assertTrue(str.equals("BTraceFilteredContext12( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA% );\r\n"));
				
		
		// after we remove the format rule - we should get no data
		model.removeExtension(ostFormatRule);		
	
		str= SourceFormatter.formatTrace(trace, TraceFormatType.EMPTY_MACRO);
		assertTrue(str.equals(""));		
	
		str = SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_ACTIVATION);
		assertTrue(str.equals(""));		

		str = SourceFormatter.formatTrace(trace, TraceFormatType.HEADER);		
		assertTrue(str.equals(""));
		
		str= SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_BUFFER);	
		assertTrue(str.equals(""));
		
		str= SourceFormatter.formatTrace(trace, TraceFormatType.TRACE_PACKED);
		assertTrue(str.equals(""));		
	}

	@Test
	public void testFormatTraceTraceTraceFormattingRuleTraceFormatTypeIteratorOfStringBoolean() throws TraceCompilerException {
				
		// note these tests use OstFormatRule specified in setupBeforeClass
		final int id = 999;
		String name = "hello_hello";  		
		TraceModelExtension[] extensions = null;

		OstTraceFormatRule ostFormatRule = new OstTraceFormatRule(); 
		TraceModel model = TraceCompilerEngineGlobals.getTraceModel();		
		TraceObjectFactory factory = model.getFactory();
		
		TraceGroup traceGroup = factory.createTraceGroup(id, name, extensions );		
		Trace trace = factory.createTrace(traceGroup, id, name, "traceText", extensions);

		ArrayList<String> tags = new ArrayList<String>();
		tags.add("tag_1");
		tags.add("tag_2");
		tags.add("tag_3");
		tags.add("tag_4");
		
		String str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.EMPTY_MACRO, tags.iterator(), false);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.EMPTY_MACRO, tags.iterator(), true);
		assertTrue(str.equals(""));		

		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_ACTIVATION, tags.iterator(), false);
		assertTrue(str.equals("BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID )\r\n"));		
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_ACTIVATION, tags.iterator(), true);
		assertTrue(str.equals("BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID )\r\n"));		
			
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.HEADER, tags.iterator(), true);
		assertTrue(str.equals("OstTraceGen0( TUint32 aTraceID )\r\n"));		
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.HEADER, tags.iterator(), false);
		assertTrue(str.equals("OstTraceGen0( TUint32 aTraceID )\r\n"));
				
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_BUFFER, tags.iterator(), true);
		assertTrue(str.equals("OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA%, %LENGTH% );\r\n"));
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_BUFFER, tags.iterator(), false);
		assertTrue(str.equals("OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA%, %LENGTH% );\r\n"));

		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_PACKED, tags.iterator(), true);
		assertTrue(str.equals("BTraceFilteredContext12( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA% );\r\n"));
		str = SourceFormatter.formatTrace(trace, ostFormatRule, TraceFormatType.TRACE_PACKED, tags.iterator(), false);
		assertTrue(str.equals("BTraceFilteredContext12( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, %DATA% );\r\n"));
		
		// try with no format rule	
		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.EMPTY_MACRO, tags.iterator(), true);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.EMPTY_MACRO, tags.iterator(), false);
		assertTrue(str.equals(""));		
		
		str = SourceFormatter.formatTrace(trace, null,  TraceFormatType.TRACE_ACTIVATION, tags.iterator(), true);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, null,  TraceFormatType.TRACE_ACTIVATION, tags.iterator(), false);
		assertTrue(str.equals(""));		

		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.HEADER, tags.iterator(), true);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.HEADER, tags.iterator(), false);
		assertTrue(str.equals(""));		

		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.TRACE_BUFFER, tags.iterator(), true);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.TRACE_BUFFER, tags.iterator(), false);
		assertTrue(str.equals(""));		

		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.TRACE_PACKED, tags.iterator(), true);
		assertTrue(str.equals(""));		
		str = SourceFormatter.formatTrace(trace, null, TraceFormatType.TRACE_PACKED, tags.iterator(), false);
		assertTrue(str.equals(""));				
	}
}
