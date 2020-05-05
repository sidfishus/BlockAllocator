#pragma once

#include "PsyncArray.h"
#include "IUnitTest.h"
#include "BlockAllocator.h"
#include "Refcount.h"

class tPArray_UnitTest : public IUnitTest
{
	enum eTestNumber
	{
		eTestFirst=0,
		//
		eCreateAndConstructAndIterate=0,
		//
		TestCount,
	};
	bool CreateAndConstructAndIterate(void);
public:
	unsigned short GetFirstTest(void) const override
	{
		return eTestFirst;
	}
	unsigned short GetTestCount(void) const override
	{
		return TestCount;
	}
	void GetTestName(
	 const unsigned short testnum,
	 const unsigned short testnamecount,
	 WCHAR* const testname) const override;												
	void GetTestDescription(
	 const unsigned short testnum,
	 const unsigned short descrcount,
	 WCHAR* const descr) const override;
	bool DoTest(const unsigned short testnum) override;
};

inline void tPArray_UnitTest::GetTestName(const unsigned short testnum,
 const unsigned short testnamecount,WCHAR* const testname) const
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eCreateAndConstructAndIterate:
		wcscpy_s(testname,testnamecount,L"CreateAndConstructAndIterate");
		break;
	}
}

inline void tPArray_UnitTest::GetTestDescription(const unsigned short testnum,
 const unsigned short descrcount,WCHAR* const descr) const
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eCreateAndConstructAndIterate:
		wcscpy_s(descr,descrcount,
		 L"Create an array, construct some items and then iterate");
		break;
	}
}

inline bool tPArray_UnitTest::CreateAndConstructAndIterate()
{
	class _tStruct
	{
		tRefCount* m_RefCounter;
	public:
		int m_X;
		struct tCtorArgs
		{
			tRefCount& RefCounter;
		};
		_tStruct(const tCtorArgs& args):m_RefCounter(&args.RefCounter),m_X(args.RefCounter.Count())
		{
			m_RefCounter->AddRef();
		}
		~_tStruct(void)
		{
			if(m_RefCounter)
			{
				m_RefCounter->Release();
				m_RefCounter=NULL;
			}
		}
	};

	tRefCount refcounter;

	{
		// Create the allocator
		const size_t maxelements=1000;
		tBlockAllocatorT<tBlockAllocatorRefCounter> allocator((maxelements+10)*sizeof(_tStruct));
		typedef tPArrayT<tBlockAllocatorRefCounter,_tStruct> _tPArray;
		_tPArray& thearray=PMakeArrayT<tBlockAllocatorRefCounter,_tStruct>(allocator,maxelements);
		UNITTEST_ASSERT(thearray.NumReserved()==maxelements);
		UNITTEST_ASSERT(thearray.Size()==0);
		// Construct some elements
		for(int i=0;i<maxelements-1;++i)
		{
			const _tStruct::tCtorArgs args=
			{
				refcounter,
			};
			thearray.Construct(args);
			UNITTEST_ASSERT(thearray.NumReserved()==maxelements);
			UNITTEST_ASSERT(thearray.Size()==i+1);
			UNITTEST_ASSERT(refcounter.Count()==i+1);
		}
		// Iterate and read
		{
			int iterationindex=0;
			for(_tPArray::iterator i=thearray.begin();i!=thearray.end();++i)
			{
				const _tStruct& thestruct=*i;
				UNITTEST_ASSERT(thestruct.m_X==(iterationindex));
				++iterationindex;
			}
		}
		// Iterate and write
		{
			int iterationindex=0;
			for(_tPArray::iterator i=thearray.begin();i!=thearray.end();++i)
			{
				(*i).m_X=iterationindex+10;
				UNITTEST_ASSERT((*i).m_X==iterationindex+10);
				++iterationindex;
			}
		}
		// Test using pointers
		{
			_tStruct* itemp=&thearray[0];
			_tStruct* const end=itemp+thearray.Size();
			for(int index=0;itemp!=end;++index,++itemp)
			{
				UNITTEST_ASSERT(itemp->m_X==index+10);
			}
		}
	}

	// Confirm all structs were released
	UNITTEST_ASSERT(refcounter.Count()==0);
	return true;
}

inline bool tPArray_UnitTest::DoTest(const unsigned short testnum)
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eCreateAndConstructAndIterate:
		return CreateAndConstructAndIterate();
	}
}