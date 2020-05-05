#pragma once

struct IUnitTest
{
	virtual unsigned short GetFirstTest(void) const=0;								// The first test number
	virtual unsigned short GetTestCount(void) const=0;								// The number of tests
	virtual bool DoTest(const unsigned short testnum)=0;							// Perform this test. False return means
																									//  test failed
	virtual void GetTestName(
	 const unsigned short testnum,
	 const unsigned short testnamecount,
	 WCHAR* const testname) const=0;														// Return the test name
	virtual void GetTestDescription(
	 const unsigned short testnum,
	 const unsigned short descrcount,
	 WCHAR* const descr) const=0;															// Return the description of this test
};