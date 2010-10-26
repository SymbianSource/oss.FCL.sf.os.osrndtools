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
* Mock TraceModel Extension
*
*/

package com.nokia.tracecompiler.model;

import java.util.ArrayList;
import java.util.Iterator;


//class for testing - comparing extensions
public	class MockExtension implements TraceModelExtension {

	private static int ref = 0;
	private int testRef;
	
	public MockExtension() {
		testRef = ++ref;
	}

	public TraceObject getOwner() {
		return null;
	}

	public void setOwner(TraceObject owner) {
	}

	public int getTestRef() {
		return testRef;
	}
	
	public boolean equals(MockExtension other){
		return (testRef == other.testRef);
	}


	public static ArrayList<MockExtension> createExtensionsHelper(int numExtensions) {
		ArrayList<MockExtension> extensions = new ArrayList<MockExtension>();
		for (int i = 0; i < numExtensions; i++) {
			extensions.add(new MockExtension());
		}
		return extensions;
	}

	public static boolean compareExtensionsHelper(Iterator<MockExtension> extensions1,
			Iterator<MockExtension> extensions2) {
		while (extensions1.hasNext()) {
			if (!extensions2.hasNext()) {				
				return false;
			}			
			if(!extensions1.next().equals(extensions2.next())){
				return false;
			}
		}
		return true;
	}

}

