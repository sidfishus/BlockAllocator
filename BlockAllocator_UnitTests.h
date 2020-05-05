#pragma once

#include "stdafx.h"
#include "BlockAllocator.h"
#include <iostream>
#include "IUnitTest.h"

template<typename POLYTYPE>
class tBlockAllocatorT<POLYTYPE>::UnitTest : public IUnitTest
{
	enum eTestNumber
	{
		eTestFirst=0,
		//
		eUseUpAllBlocksTest=0,
		//
		TestCount,
	};
	bool UseUpAllBlocksTest();
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

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::UnitTest::GetTestName(const unsigned short testnum,const unsigned short testnamecount,
 WCHAR* const testname) const
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eUseUpAllBlocksTest:
		wcscpy_s(testname,testnamecount,L"UseUpAllBlocks");
		break;
	}
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::UnitTest::GetTestDescription(const unsigned short testnum,
 const unsigned short descrcount,WCHAR* const descr) const
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eUseUpAllBlocksTest:
		wcscpy_s(descr,descrcount,
		 L"Test the mechanism which throws away the smallest block when all block spaces are used up");
		break;
	}
}

template<typename POLYTYPE>
bool tBlockAllocatorT<POLYTYPE>::UnitTest::DoTest(const unsigned short testnum)
{
	switch(testnum)
	{
	default:
		PANIC;
		// No return
	case eUseUpAllBlocksTest:
		return UseUpAllBlocksTest();
	}
}

template<typename POLYTYPE>
bool tBlockAllocatorT<POLYTYPE>::UnitTest::UseUpAllBlocksTest()
{
	typedef tBlockAllocatorT<POLYTYPE> _tAllocator;
	typedef tManagedMemoryBlockT<POLYTYPE> _tMemBlock;
	_tAllocator allocator(1000);
	allocator.CreateFirstBlock();
	_ASSERTE(allocator.m_NumBlocks==1);
	if(allocator.m_NumBlocks!=1)
	{
		return false;
	}
	const size_t sizeexpected=1000-sizeof(_tMemBlock);
	_ASSERTE(allocator.m_BlockSizes[0]==sizeexpected);
	if(allocator.m_BlockSizes[0]!=sizeexpected)
	{
		return false;
	}
	// Allocate memory so we use up all blocks
	// This is on purpose using more memory than in the loop
	allocator.AllocateUnmanaged<char[700]>();
	_ASSERTE(allocator.m_NumBlocks==1);
	if(allocator.m_NumBlocks!=1)
	{
		return false;
	}
	for(int blockcount=1;blockcount<_tAllocator::eMaxNumBlocks;)
	{
		allocator.AllocateUnmanaged<char[600]>();
		++blockcount;
		const int expectedsize=400-sizeof(_tMemBlock);
		_ASSERTE(allocator.m_BlockSizes[blockcount-1]==expectedsize);
		_ASSERTE(allocator.m_NumBlocks==blockcount);
		if(allocator.m_NumBlocks!=blockcount)
		{
			return false;
		}
	}
	// The number of blocks should be the maximum allowed now
	_ASSERTE(allocator.m_NumBlocks==_tAllocator::eMaxNumBlocks);
	if(allocator.m_NumBlocks!=_tAllocator::eMaxNumBlocks)
	{
		return false;
	}
	unsigned short oldshortestsize;
	_tMemBlock* oldblock;
	{
		const unsigned char smallestblockidx=allocator.SmallestBlockIdx();
		_ASSERTE(smallestblockidx>=0);
		if(smallestblockidx<0)
		{
			return false;
		}
		oldblock=allocator.m_Blocks[smallestblockidx];
		const int expectedsmallestsize=300-sizeof(_tMemBlock);
		oldshortestsize=allocator.m_BlockSizes[smallestblockidx];
		_ASSERTE(oldshortestsize==expectedsmallestsize);
		if(oldshortestsize!=expectedsmallestsize)
		{
			return false;
		}
	}
	// Allocate another block. This should cause the smallest one to be removed
	allocator.AllocateUnmanaged<char[600]>();
	// Still should have the max number of blocks
	_ASSERTE(allocator.m_NumBlocks==_tAllocator::eMaxNumBlocks);
	if(allocator.m_NumBlocks!=_tAllocator::eMaxNumBlocks)
	{
		return false;
	}
	// The smallest block size should have updated now
	const unsigned short newsmallestblockidx=allocator.SmallestBlockIdx();
	const unsigned short newshortestsize=allocator.m_BlockSizes[newsmallestblockidx];
	const unsigned short expectedsmallestsize=400-sizeof(_tMemBlock);
	_ASSERTE(newshortestsize==expectedsmallestsize);
	if(newshortestsize!=expectedsmallestsize)
	{
		return false;
	}
	// Confirm the old block is no longer in the block list
	for(int i=0;i<allocator.m_NumBlocks;++i)
	{
		_ASSERTE(allocator.m_Blocks[i]!=oldblock);
		if(allocator.m_Blocks[i]==oldblock)
		{
			return false;
		}
	}
	return true;
}