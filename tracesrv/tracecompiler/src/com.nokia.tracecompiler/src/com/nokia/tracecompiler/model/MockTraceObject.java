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
* Mock TraceObject for testing
*
*/

package com.nokia.tracecompiler.model;

import java.util.Iterator;


/**
 * Class to mock the instantiation of TraceObject which is abstract but there is no reason to justify it except
 * probably making sure it's not instantiable, that could be achieved by other means
 *
 */
public class MockTraceObject extends TraceObject {

	@Override
	public void addExtension(TraceModelExtension extension) {
		super.addExtension(extension);
	}

	@Override
	public <T extends TraceModelExtension> T getExtension(Class<T> extClass) {
		return super.getExtension(extClass);
	}

	@Override
	public <T extends TraceModelExtension> Iterator<T> getExtensions(
			Class<T> extClass) {
		return super.getExtensions(extClass);
	}

	@Override
	public int getID() {
		return super.getID();
	}

	@Override
	public TraceModel getModel() {
		return super.getModel();
	}

	@Override
	public String getName() {
		return super.getName();
	}

	@Override
	void internalSetID(int id) {
		super.internalSetID(id);
	}

	@Override
	public boolean isComplete() {
		return super.isComplete();
	}

	@Override
	protected void notifyOnDelete(TraceObject object) {
		super.notifyOnDelete(object);
	}

	@Override
	public void removeExtension(TraceModelExtension extension) {
		super.removeExtension(extension);
	}

	@Override
	public <T extends TraceModelExtension> void removeExtensions(
			Class<T> extClass) {
		super.removeExtensions(extClass);
	}

	@Override
	void reset() {
		super.reset();
	}

	@Override
	void setComplete() throws TraceCompilerException {
		super.setComplete();
	}

	@Override
	public void setID(int id) throws TraceCompilerException {
		super.setID(id);
	}

	@Override
	void setModel(TraceModel model) {
		super.setModel(model);
	}

	@Override
	public void setName(String name) throws TraceCompilerException {
		super.setName(name);
	}
}
