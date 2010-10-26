/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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
*/

How to execute test:

1. Compile PIEngineTest from /group
        sbs -c TARGET

2. Add '#include "PIEngineTest.iby" to piprofiler.iby or other suitable place

3. Configure test module to STIF .ini file ( /epoc32/wins/c/TestFramework )
        [New_Module]
        ModuleName= TestScripter
        TestCaseFile= z:\testframework\PIEngineTest.cfg
        [End_Module]

4. Create image

5. Run test from STIF UI

