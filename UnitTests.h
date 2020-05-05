#pragma once

#include "IUnitTest.h"

#define UNITTEST_ASSERT(val) _ASSERTE((val)); if(!(val)) return false;

class tUnitTestExecutor
{
	void DescribeTest(
	 IUnitTest& test,
	 const unsigned short testnum,
	 const unsigned short descrcount,
	 WCHAR* const descr);
public:
	int DoUnitTest(
	 IUnitTest& test,
	 const unsigned short failmsgcount,
	 WCHAR* failmsg);																			// Returns the first test which failed or -1
};

int tUnitTestExecutor::DoUnitTest(IUnitTest& test,const unsigned short failmsgcount,WCHAR* failmsg)
{
	const char* testname=NULL;
	const unsigned short firsttest=test.GetFirstTest();
	const unsigned short testcount=test.GetTestCount();
	unsigned short testnum=firsttest;
	try
	{
		for(;testnum<(firsttest+testcount);++testnum)
		{
			if(!test.DoTest(testnum))
			{
				WCHAR descr[512];
				DescribeTest(test,testnum,_countof(descr),descr);
				wprintf_s(failmsg,failmsgcount,L"%ls. This test failed by returning false.",descr);
				return testnum;
			}
		}
	}
	catch(...)
	{
		WCHAR descr[512];
		DescribeTest(test,testnum,_countof(descr),descr);
		wprintf_s(failmsg,failmsgcount,L"%ls. This test threw an exception!",descr);
		return testnum;
	}
	return -1;
}

void tUnitTestExecutor::DescribeTest(IUnitTest& test,const unsigned short testnum,const unsigned short descrcount,
 WCHAR* const descr)
{
	WCHAR testname[64];
	test.GetTestName(testnum,_countof(testname),testname);
	WCHAR testdescr[256];
	test.GetTestDescription(testnum,_countof(testdescr),testdescr);
	wprintf_s(descr,descrcount,"Test (number %hd) %ls: %ls",testname,testdescr);
}