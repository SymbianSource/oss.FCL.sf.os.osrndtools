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
* Mock TraceModelExtension for testing
*
*/

package com.nokia.tracecompiler.model;

/**
 * Class to Mock a TraceModel Listener, all what we are interested in here is
 * the notification bit
 * 
 */

public class MockTraceModelListener implements TraceModelListener {

	boolean notified = false;

	public void objectAdded(TraceObject owner, TraceObject object) {
		notified = true;
	}

	public void objectCreationComplete(TraceObject object) throws TraceCompilerException {
		notified = true;

	}

	public void objectRemoved(TraceObject owner, TraceObject object) throws TraceCompilerException {
		notified = true;
	}

	public void propertyUpdated(TraceObject object, int property) throws TraceCompilerException {
		notified = true;
	}

	public boolean isNotified() {
		return notified;
	}

	public void resetNotification() {
		notified = false;
	}
}
