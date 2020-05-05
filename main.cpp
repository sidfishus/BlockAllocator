// BlockAllocator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "UnitTests.h"
#include "BlockAllocator_UnitTests.h"
#include "PsyncArray_UnitTests.h"


int _tmain(int argc, _TCHAR* argv[])
{
	tUnitTestExecutor test;
	WCHAR failmsg[512];
	{
		IUnitTest& unittest=*(new tBlockAllocator::UnitTest());
		const int testnumfailed=test.DoUnitTest(unittest,_countof(failmsg),failmsg);
		if(testnumfailed!=-1)
		{
			std::cout<<failmsg<<"\n";
		}
	}

	{
		IUnitTest& unittest=*(new tPArray_UnitTest());
		const int testnumfailed=test.DoUnitTest(unittest,_countof(failmsg),failmsg);
		if(testnumfailed!=-1)
		{
			std::cout<<failmsg<<"\n";
		}
	}
	return 0;
}

