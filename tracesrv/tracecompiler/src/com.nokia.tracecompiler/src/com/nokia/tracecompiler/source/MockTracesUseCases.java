package com.nokia.tracecompiler.source;

final public class MockTracesUseCases {

	
	//TODO please check if '\n' shouldn't be replaced by SourceConstants.LINE_FEED
	public static final String[] testCases = 
	{
		"", //0
		
		"CHelloTraceFn::CHelloTraceFn()" + '\n'
		+ "// a comment with OstTrace keyword in it." + '\n'
		+ "/*" + '\n'
		+ " * a multiline comment with OstTrace keyword" + '\n'
		+ " */" + '\n'
		+ "{" + '\n'
	    + "OstTrace1( TRACE_NORMAL, CHELLOTRACEFN_CHELLOTRACEFN_CTOR, \"[0x%08x] Constructor\", this );" +'\n' 
	    + "OstTrace1( TRACE_NORMAL, CHELLOTRACEFN_CHELLOTRACEFN_CTOR_TEXTWITHTAB, \"[0x%08x] Constructor -   Trace text with tabs\", this );" + '\n'
	    + "}", //1
	    
	    "void CHelloTraceFn::Simple()" + '\n'
	    + "{" + '\n'
	    + "OstTraceFunctionEntry1( CHELLOTRACEFN_SIMPLE_ENTRY, this );" + '\n'
	    + "OstTraceFunctionEntryExt( CHELLOTRACEFN_SIMPLE_ENTRY_EXT_NO_PARAM, this );" + '\n'
	    + "// ..." + '\n'
	    + "OstTraceFunctionExit1( CHELLOTRACEFN_SIMPLE_EXIT, this );" + '\n'
		+ "}", //2
		
		"TInt CHelloTraceFn::OutputsTIntReturnValue()" + '\n'
	    + "{" + '\n'
	    + "OstTraceFunctionEntry1( CHELLOTRACEFN_OUTPUTSTINTRETURNVALUE_ENTRY, this );" + '\n'
	    + "TInt ret = KErrNone;" + '\n'
	    + "// ..." + '\n'
	    + "OstTraceFunctionExitExt( CHELLOTRACEFN_OUTPUTSTINTRETURNVALUE_EXIT, this, ret );" + '\n'
	    + "return ret;" + '\n'
	    + "}", //3
	    
	    "CActive* CHelloTraceFn::OutputsUnknownPtrType(CActive* aActive)" + '\n'
	    + "{" + '\n'
	    + "OstTraceFunctionEntryExt( CHELLOTRACEFN_OUTPUTSUNKNOWNPTRTYPE_ENTRY, this );" + '\n'
	    + "// ..." + '\n'
	    + "OstTraceFunctionExitExt( CHELLOTRACEFN_OUTPUTSUNKNOWNPTRTYPE_EXIT, this, (TUint) aActive );" + '\n'
	    + "return aActive;" + '\n'
		+ "}", //4
		
		"#include \"blah.h\"" + '\n'
		+ "CHelloTraceFn::CHelloTraceFn()" + '\n'
		+ "// a comment with OstTrace keyword in it." + '\n'
		+ "/*" + '\n'
		+ " * a multiline comment with OstTrace keyword" + '\n'
		+ " */" + '\n'
		+ "{" + '\n'
	    + "OstTrace1( TRACE_NORMAL, CHELLOTRACEFN_CHELLOTRACEFN_CTOR, \"[0x%08x] Constructor\", this );" +'\n' 
	    + "OstTrace1( TRACE_NORMAL, CHELLOTRACEFN_CHELLOTRACEFN_CTOR_TEXTWITHTAB, \"[0x%08x] Constructor -   Trace text with tabs\", this );" + '\n'
	    + "}" //5
		
	    //TODO add more test cases here
	 };

	

}
