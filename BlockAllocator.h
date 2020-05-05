#pragma once

#include "LazyObject.h"
#include "ManagedMemoryBlock.h"
#include "PsyncLib.h"
#include "PolyWrap.h"
#include "RefCount.h"
#include "ProxyRefCounter.h"

template<typename POLYTYPE>
class tBlockAllocatorT
{
//=====================================================================================================================
// PRIVATE
//=====================================================================================================================
	enum
	{
		eMaxNumBlocks=4,																		// Maximum number of memory blocks.
		eBlockCutOffPointBytes=64,															// When a block becomes equal or less to
																									//  this value it's removed.
	};
	typedef tManagedMemoryBlockT<POLYTYPE> _tMemoryBlock;
	template<typename TYPE>
	class _tPolyWrap : public tPolyWrapT<POLYTYPE,TYPE> {};
	//
	unsigned char m_NumBlocks;																// The number of memory blocks in use.
	int32_t m_BlockSizes[eMaxNumBlocks];												// The size remaining of each block. Holding
																									//  these in this class as well as the
																									//  memory block class is for performance
																									//  reasons due to the array being contiguous
																									//  and therefore fast to access in a loop
																									//  and it being slow to access each memory
																									//  block
	_tMemoryBlock* m_Blocks[eMaxNumBlocks];											// The memory blocks. These are parallel
																									//  with m_BlockSizes
	const int32_t m_InitialSize;															// The size for the first block
	const int32_t m_SubsequentBlockSize;												// The size of subsequent blocks
	tRefCount m_RefCount;																	// A resource helper. Debug aid. Is used
																									//  only when POLYTYPE is a resource managing
																									//  object. i.e.
																									//  tBlockAllocatorRefCounted
	//~V
	tBlockAllocatorT(void);
	tBlockAllocatorT(const tBlockAllocatorT&);
	tBlockAllocatorT& operator=(const tBlockAllocatorT&);
	int32_t NextBlockSize(void) const;													// Size of next block
	int32_t NextBlockSize(
	 const int32_t size,
	 const int32_t alignment,
	 const bool ismanaged) const;															// Size of the next block or the minimum
																									//  size to fit an object of this size and
																									//  alignment
	void CreateAnotherBlock(
	 const int32_t nbytes,
	 const bool zeroinitialise);															// Create another block of memory (or the
																									//  first)
	void* _Allocate(
	 const bool manage, //todo param needed?
	 const int32_t size,
	 const unsigned short alignment,
	 unsigned char& blockidx);
	template<typename TYPE>
	TYPE& _Allocate(
	 const bool ismanaged,
	 unsigned char& blockidx,
	 const int32_t size);																	// Allocate an object of this type. Returns
																									//  the block that was used to allocate this
																									//  object
	template<typename TYPE>
	TYPE& _AllocateAndConstruct(void);													// Allocate and construct an object of this
																									//  type
	template<typename TYPE>
	TYPE& _AllocateAndConstructPoly(const int32_t size);							// Allocate and construct an object that
																									//  derives from POLYTYPE
	int32_t AlignmentPaddingForBlocksize(int32_t blocksize) const;				// The alignment padding required for a
																									//  block of this size
	char SmallestBlockIdx(void) const;													// The index of the smallest block or -1
																									//  if there are no blocks in use
	char SmallestBlockIdx(void);
	const _tMemoryBlock* SmallestBlock(void) const;									// The smallest block or NULL if there are
																									//  no blocks in use
	_tMemoryBlock* SmallestBlock(void);
	void RemoveBlock(const unsigned char idx) throw();								// Remove the block at this index
	void HoldOntoBlock(const unsigned char idx);										// Hold on to the block at this index
	bool IsLastBlock(const unsigned char idx) const;								// Is the block at idx the last one?
	unsigned char LastBlockIdx(void) const;											// The index of the last block
	void* Use(
	 const unsigned char blockidx,
	 const size_t numbytes,
	 const unsigned short alignment,
	 const bool ismanaged);																	// Attempt to use 'num bytes' from this
																									//  memory block. May fail due to not enough
																									//  space
	_tMemoryBlock& Block(const unsigned char idx);									// Block at this index.
	const _tMemoryBlock& Block(const unsigned char idx) const;
	const int32_t& BlockSize(const unsigned char idx) const;						// Block size at this index
	int32_t& BlockSize(const unsigned char idx);
	bool IsValidBlockIdx(const unsigned char idx) const;							// Is this a valid block idx?
	bool SpaceForAnotherBlock(void) const;												// Is there space for another block?
	void ManageObjectDestruction(
	 const unsigned char blockidx,
	 POLYTYPE& managedobject);																// Manage the destruction of this object
	void UpdateBlockSize(const unsigned char blockidx);							// Update the block size
	void DeleteBlock(_tMemoryBlock& block);											// Delete a block and it's children
	//~F
public:
	class UnitTest;
//=====================================================================================================================
// FUNCTIONS/MODIFIERS
//=====================================================================================================================
	tBlockAllocatorT(
	 const int32_t initialsize,
	 const int32_t subsequentblocksize=0 /* 0 means use initial size */);
	~tBlockAllocatorT(void);
	void Clear();
	void Invariant(void) const;
	void CreateFirstBlock(void);															// It's better to allocate outside of
																									//  critical loops.
	template<typename TYPE>
	TYPE& AllocateUnmanaged(void);														// Allocated but not constructed. Useful for
																									//  POD types. For example:
																									// int (&x)[10]=AllocateUnmanaged<int[10]>();
	template<typename TYPE>
	tLazyT<TYPE,POLYTYPE>& Allocate(void);												// Allocation and construction managed by the
																									//  caller. Destruction will happen
																									//  automatically if the object is
																									//  constructed
	template<typename TYPE>
	TYPE& AllocateAndConstructPoly(void);
	template<typename TYPE>
	TYPE& AllocateAndConstructPoly(const int32_t size);							// Objects must derive from POLYTYPE
	template<typename TYPE>
	TYPE& AllocateAndConstructPoly(
	 typename const TYPE::tCtorArgs& args);											// Objects must derive from POLYTYPE
	template<typename TYPE>
	TYPE& AllocateAndConstructPoly(
	 typename const TYPE::tCtorArgs& args,
	 const int32_t size);																	// Objects must derive from POLYTYPE
	template<typename TYPE>
	TYPE& AllocateAndConstruct(void);													// Where objects do not derive from POLYTYPE
	template<typename TYPE>
	TYPE& AllocateAndConstruct(typename const TYPE::tCtorArgs& args);			// Where objects do not derive from POLYTYPE
	//
};

//todo better name?
typedef tProxyRefCounter(tRefCount) tBlockAllocatorRefCounter;

//=====================================================================================================================
// HELPER FUNCTIONS
//=====================================================================================================================

template<typename POLYTYPE,typename TYPE>
TYPE& AllocateAndConstructPoly(
 tBlockAllocatorT<POLYTYPE>& allocator,
 const int32_t size,
 typename const TYPE::tCtorArgs& args);

template<typename POLYTYPE,typename TYPE>
TYPE& AllocateAndConstructPoly(
 tBlockAllocatorT<POLYTYPE>& allocator,
 const int32_t size);

//=====================================================================================================================
// IMPLEMENTATION
//=====================================================================================================================

template<typename POLYTYPE>
tBlockAllocatorT<POLYTYPE>::tBlockAllocatorT(const int32_t initialsize,const int32_t subsequentblocksize /*=0*/)
:m_InitialSize(initialsize),m_SubsequentBlockSize((subsequentblocksize)?subsequentblocksize:initialsize),
m_NumBlocks(0)
{
	Invariant();
}

template<typename POLYTYPE>
tBlockAllocatorT<POLYTYPE>::~tBlockAllocatorT(void)
{
	Clear();
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::Clear()
{
	Invariant();
	for(unsigned char blockidx=0;blockidx<m_NumBlocks;++blockidx)
	{
		DeleteBlock(Block(blockidx));
	}
	m_NumBlocks=0;
	// If this is non 0, then we have a resource issue!
	_ASSERTE(m_RefCount.Count()==0);
	if(m_RefCount.Count()!=0)
	{
		char msg[128];
		sprintf_s(msg,_countof(msg),"Resource problem detected! Managed object reference count is %ld.",
		 m_RefCount.Count());
		throw std::bad_alloc("Resource problem detected!");
	}
	Invariant();
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::DeleteBlock(_tMemoryBlock& block)
{
	_tMemoryBlock* pblock=&block;
	do
	{
		_tMemoryBlock& iterblock=*pblock;
		pblock=iterblock.PreviousBlock();
		// Free the resources associated with this block
		iterblock.~_tMemoryBlock();
		// Free the memory associated with this block
		::free(static_cast<void*>(&iterblock));
	}while(pblock);
}

template<typename POLYTYPE>
void* tBlockAllocatorT<POLYTYPE>::_Allocate(const bool manage,const int32_t size,const unsigned short alignment,
 unsigned char& blockidx)
{
	Invariant();
	if(!m_NumBlocks)
	{
		// Initialise for first time
		// Don't zero initialise as that would incur a penalty that we may not want to incur now.
		const bool zeroinitialise=false;
		CreateAnotherBlock(NextBlockSize(size,alignment,manage),zeroinitialise);
	}
	_ASSERTE(m_NumBlocks>0);
	// Try use one of the memory blocks
	// The newly allocated object
	void* allocatedobject=NULL;
	const int32_t minimumbytesrequired=
	 static_cast<int32_t>(size+((manage)?_tMemoryBlock::eOverheadForManagedObject:0));
	for(blockidx=0;blockidx<m_NumBlocks;++blockidx)
	{
		// This does not take in to account alignment, which we can't know unless we ask the memory block which would
		//  slow this down. Which means the call to allocate memory within a block could fail due to alignment padding
		//  being required to fit the new object.
		if(BlockSize(blockidx)>=minimumbytesrequired)
		{
			allocatedobject=Use(blockidx,size,alignment,manage);
			if(allocatedobject)
			{
				break;
			}
		}
	}
	if(!allocatedobject)
	{
		// Allocate another block
		const bool zeroinitialise=false;
		CreateAnotherBlock(NextBlockSize(size,alignment,manage),zeroinitialise);
		blockidx=LastBlockIdx();
		allocatedobject=Use(blockidx,size,alignment,manage);
	}
	_ASSERTE(allocatedobject);
	Invariant();
	return allocatedobject;
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::_Allocate(const bool manage,unsigned char& blockidx,const int32_t size)
{
	// If the size is specified, it must be at least be the size of the object being created
	_ASSERTE(size>=sizeof(TYPE));
	static const unsigned short alignment=static_cast<unsigned short>(alignment_of<TYPE>::value);
	return *reinterpret_cast<TYPE*>(_Allocate(manage,size,alignment,blockidx));
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateUnmanaged(void)
{
	unsigned char unused;
	const bool manage=false;
	const int32_t size=sizeof(TYPE);
	return _Allocate<TYPE>(manage,unused,size);
}

template<typename POLYTYPE>
template<typename TYPE>
tLazyT<TYPE,POLYTYPE>& tBlockAllocatorT<POLYTYPE>::Allocate(void)
{
	return _AllocateAndConstructPoly<tLazyT<TYPE,POLYTYPE> >();
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstructPoly(void)
{
	return AllocateAndConstructPoly(size);
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstructPoly(const int32_t size)
{
	const int32_t size=sizeof(TYPE);
	return _AllocateAndConstructPoly<TYPE>(size);
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::_AllocateAndConstructPoly(const int32_t size)
{
	Invariant();
	unsigned char blockidx;
	static const bool manage=true;
	TYPE& allocatedobject=_Allocate<TYPE>(manage,blockidx,size);
	// Construct the smart pointer (not the wrapped object)
	::new(static_cast<void*>(&allocatedobject)) TYPE();
	// Manage the destruction of the object.
	ManageObjectDestruction(blockidx,allocatedobject);
	Invariant();
	return allocatedobject;
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstructPoly(typename const TYPE::tCtorArgs& args)
{
	const int32_t size=sizeof(TYPE);
	return AllocateAndConstructPoly(args,size);
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstructPoly(typename const TYPE::tCtorArgs& args,const int32_t size)
{
	Invariant();
	unsigned char blockidx;
	static const bool manage=true;
	TYPE& allocatedobject=_Allocate<TYPE>(manage,blockidx,size);
	// Construct the object
	::new(static_cast<void*>(&allocatedobject)) TYPE(args);
	// Manage the destruction of the object
	ManageObjectDestruction(blockidx,allocatedobject);
	Invariant();
	return allocatedobject;
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::_AllocateAndConstruct(void)
{
	Invariant();
	// Wrap the TYPE up as a POLYTYPE
	unsigned char blockidx;
	_tPolyWrap<TYPE>& allocatedobject=_Allocate<_tPolyWrap<TYPE> >(true,blockidx);
	// Construct the smart pointer (not the wrapped object)
	::new(static_cast<void*>(&allocatedobject)) _tPolyWrap<TYPE>();
	// Manage the destruction of the object.
	ManageObjectDestruction(blockidx,allocatedobject);
	Invariant();
	return *allocatedobject;
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstruct(typename const TYPE::tCtorArgs& args)
{
	Invariant();
	// Wrap the TYPE up as a POLYTYPE
	unsigned char blockidx;
	_tPolyWrap<TYPE>& allocatedobject=_Allocate<_tPolyWrap<TYPE> >(true,blockidx);
	// Construct the smart pointer (not the wrapped object)
	::new(static_cast<void*>(&allocatedobject)) TYPE(args);
	// Manage the destruction of the object (using the poly wrapper)
	ManageObjectDestruction(blockidx,allocatedobject);
	Invariant();
	return *allocatedobject;
}

template<typename POLYTYPE>
template<typename TYPE>
TYPE& tBlockAllocatorT<POLYTYPE>::AllocateAndConstruct(void)
{
	return _AllocateAndConstruct<TYPE>();
}

template<typename POLYTYPE>
char tBlockAllocatorT<POLYTYPE>::SmallestBlockIdx(void) const
{
	// Start off at the highest possible number so every block is less than this.
	int32_t smallestsize=numeric_limits<int32_t>::max();
	char smallestblockidx=-1;
	// We're casting the max blocks to a char so this static asserts the max number of blocks will not overflow
	C_ASSERT(eMaxNumBlocks<=CHAR_MAX);
	for(char blockidx=0;blockidx<static_cast<char>(m_NumBlocks);++blockidx)
	{
		// Assuming that a block is never INT32_MAX in size otherwise it will cause this function to fail
		_ASSERTE(BlockSize(blockidx)<numeric_limits<int32_t>::max());
		if(BlockSize(blockidx)<smallestsize)
		{
			// New smallest block
			smallestblockidx=blockidx;
			smallestsize=BlockSize(blockidx);
		}
	}
	return smallestblockidx;
}

template<typename POLYTYPE>
char tBlockAllocatorT<POLYTYPE>::SmallestBlockIdx(void)
{
	return static_cast<const tBlockAllocatorT&>(*this).SmallestBlockIdx();
}

template<typename POLYTYPE>
const typename tBlockAllocatorT<POLYTYPE>::_tMemoryBlock* tBlockAllocatorT<POLYTYPE>::SmallestBlock(void) const
{
	const char idx=SmallestBlockIdx();
	const _tMemoryBlock* rv;
	if(idx>=0)
	{
		rv=&(Block(idx));
	}
	else
	{
		rv=NULL;
	}
	return rv;
}

template<typename POLYTYPE>
typename tBlockAllocatorT<POLYTYPE>::_tMemoryBlock* tBlockAllocatorT<POLYTYPE>::SmallestBlock(void)
{
	return const_cast<_tMemoryBlock*>(static_cast<const tBlockAllocatorT&>(*this).SmallestBlock());
}

// Hold on to this block by adding it to the back of one of the in use memory blocks.
template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::HoldOntoBlock(const unsigned char idx)
{
	// This can't work if there's only one block
	_ASSERTE(m_NumBlocks>1);
	// Work out the parent to use. Which is either:
	//  0 - if idx is the last block, or
	//  idx+1 - otherwise
	const unsigned char parentidx=((IsLastBlock(idx))?0:idx+1);
	_tMemoryBlock& blocktoholdonto=Block(idx);
	Block(parentidx).ChainAttachBlock(blocktoholdonto);
}

template<typename POLYTYPE>
bool tBlockAllocatorT<POLYTYPE>::IsLastBlock(const unsigned char idx) const
{
	return (idx==LastBlockIdx());
}

template<typename POLYTYPE>
unsigned char tBlockAllocatorT<POLYTYPE>::LastBlockIdx(void) const
{
	_ASSERTE(m_NumBlocks>0);
	return m_NumBlocks-1;
}

template<typename POLYTYPE>
void* tBlockAllocatorT<POLYTYPE>::Use(const unsigned char blockidx,const size_t size,const unsigned short alignment,
 const bool ismanaged)
{
	Invariant();
	void* const mem=Block(blockidx).Use(size,alignment,ismanaged);
	if(mem)
	{
		// Update the size remaining for the block we've just allocated from
		UpdateBlockSize(blockidx);
		// If the number of blocks is 1 then we can't get rid of it yet, as nothing could have a reference on it
		//  and thus leak memory - this is handled in the special case further down. Alternatively we could
		//  allocate another block here, but that doesn't seem right. We should only allocate memory when we
		//  need another object, and we have satisfied this call to allocate a new object, hence no need to
		//  allocate more
		if(m_NumBlocks>1 && BlockSize(blockidx)<=eBlockCutOffPointBytes)
		{
			// Attach this block to the end of another block to keep a reference on it
			HoldOntoBlock(blockidx);
			// Get rid of the block.
			RemoveBlock(blockidx);
		}
	}
	Invariant();
	return mem;
}

template<typename POLYTYPE>
typename tBlockAllocatorT<POLYTYPE>::_tMemoryBlock& tBlockAllocatorT<POLYTYPE>::Block(const unsigned char idx)
{
	return const_cast<_tMemoryBlock&>(static_cast<const tBlockAllocatorT&>(*this).Block(idx));
}

template<typename POLYTYPE>
const typename tBlockAllocatorT<POLYTYPE>::_tMemoryBlock& tBlockAllocatorT<POLYTYPE>::Block(const unsigned char idx) const
{
	_ASSERTE(IsValidBlockIdx(idx));
	return *m_Blocks[idx];
}

template<typename POLYTYPE>
const int32_t& tBlockAllocatorT<POLYTYPE>::BlockSize(const unsigned char idx) const
{
	_ASSERTE(IsValidBlockIdx(idx));
	_ASSERTE(m_BlockSizes[idx]>=0);
	return m_BlockSizes[idx];
}

template<typename POLYTYPE>
int32_t& tBlockAllocatorT<POLYTYPE>::BlockSize(const unsigned char idx)
{
	return const_cast<int32_t&>(static_cast<const tBlockAllocatorT&>(*this).BlockSize(idx));
}

template<typename POLYTYPE>
bool tBlockAllocatorT<POLYTYPE>::IsValidBlockIdx(const unsigned char idx) const
{
	return idx<=LastBlockIdx();
}

template<typename POLYTYPE>
bool tBlockAllocatorT<POLYTYPE>::SpaceForAnotherBlock(void) const
{
	return m_NumBlocks<eMaxNumBlocks;
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::CreateFirstBlock(void)
{
	Invariant();
	// It doesn't make sense to call this if a block has already been
	//  allocated
	_ASSERTE(!m_NumBlocks);
	if(!m_NumBlocks)
	{
		CreateAnotherBlock(NextBlockSize(),true);
	}
	Invariant();
}

// Calculate the next block size. Must be big enough to fit an object with the size/alignment
template<typename POLYTYPE>
int32_t tBlockAllocatorT<POLYTYPE>::NextBlockSize(const int32_t size,const int32_t alignment,const bool ismanaged) const
{
	// Size and alignment must both be greater than 0
	_ASSERTE(size>0 && alignment>0);
	Invariant();
	// The minimum size required to fit 'size' as well as the overhead of the allocator, taking in to
	//  account alignment and whether it's a POD
	int32_t minsizerequired=size;
	// Add the overhead of the allocator
	minsizerequired+=sizeof(_tMemoryBlock);
	// Add 'alignment' to the size - in case we need to pad at the beginning of the memory block. Probably won't
	//  happen due to the way malloc aligns memory on the biggest possible alignment, but just to be sure.
	if(alignment>1)
	{
		minsizerequired+=alignment;
	}
	if(ismanaged)
	{
		// If it's not a POD then need to fit in at least one POLYTYPE*
		minsizerequired+=sizeof(POLYTYPE*);
		// Include padding the manage
		minsizerequired+=AlignmentPaddingForBlocksize(minsizerequired);
	}
	Invariant();
	return ((minsizerequired>NextBlockSize())?minsizerequired:NextBlockSize());
}

// Returns the alignment padding needed for this block size
template<typename POLYTYPE>
int32_t tBlockAllocatorT<POLYTYPE>::AlignmentPaddingForBlocksize(int32_t blocksize) const
{
	// Meaningless to call this function for a block size of 0
	_ASSERTE(blocksize>0);
	Invariant();
	// Take into account alignment for managed object pointers at the end so it's always aligned on a
	//  sizeof(POLYTYPE*) memory boundary
	const int32_t polypad=blocksize%sizeof(POLYTYPE*);
	// Sanity check
	_ASSERTE(polypad<sizeof(POLYTYPE*));
	// Check the resulting size is aligned properly
	_ASSERTE(!((blocksize+polypad)%sizeof(POLYTYPE*)));
	//
	return polypad;
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::CreateAnotherBlock(const int32_t nbytes,const bool zeroinitialise)
{
	// Sanity check!
	_ASSERTE(nbytes>sizeof(_tMemoryBlock));
	Invariant();
	// Doesn't make sense to have the maximum number of blocks set to 1 and it will cause problems in this function due
	//  to assumptions it makes
	C_ASSERT(eMaxNumBlocks>1);
	// Create the memory
	void* const newmemory=malloc(nbytes);
	if(!newmemory)
	{
		// Failed to allocate memory. Consider reducing the block size
		char errormsg[56];
		sprintf_s(errormsg,"Failed to allocate a block of %lu bytes.",nbytes);
		throw std::bad_alloc(errormsg);
	}
	_tMemoryBlock* previousblock;
	if(!SpaceForAnotherBlock())
	{
		// Need to get rid of one of the memory blocks. Throw the one away which has the least space remaining
		const char smallestblockidx=SmallestBlockIdx();
		// Should be impossible not to have a block at this point 
		_ASSERTE(smallestblockidx>=0);
		previousblock=&(Block(smallestblockidx));
		// Remove this block
		// If this ever threw an exception then it would cause 'previousblock' to leak. But it won't
		RemoveBlock(smallestblockidx);
	}
	else
	{
		// Special case: if there is one block and it's size is less than the cut off point, remove it from the in-use
		//  list and hold on to it as the back point pointer of the new memory block we're constructing
		if(m_NumBlocks==1 && BlockSize(0)<eBlockCutOffPointBytes)
		{
			// Get rid of the block and connect it to the back of the new one
			previousblock=&(Block(0));
			RemoveBlock(0);
		}
		else
		{
			previousblock=NULL;
		}
	}
	// Construct the new block
	::new(newmemory) _tMemoryBlock(previousblock,nbytes,zeroinitialise);
	// Add the block to our list
	_ASSERTE(SpaceForAnotherBlock());
	const unsigned char newblockidx=m_NumBlocks++;
	m_Blocks[newblockidx]=static_cast<_tMemoryBlock*>(newmemory);
	// Hold the size of the block
	UpdateBlockSize(newblockidx);
	Invariant();
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::RemoveBlock(const unsigned char idx) throw()
{
	_ASSERTE(idx<m_NumBlocks);
	// Is there at least one block above the one we are removing?
	if(!IsLastBlock(idx))
	{
		// Shift the blocks and block sizes above the one we're removing one to the left - overwrite
		ShiftLeft(m_Blocks,m_NumBlocks,idx+1);
		ShiftLeft(m_BlockSizes,m_NumBlocks,idx+1);
	}
	// There is one less block
	--m_NumBlocks;
	_ASSERTE(m_NumBlocks>=0);
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::Invariant(void) const
{
#ifdef _DEBUG
	// I think it would be pointless to use this with block sizes of less than 1kB.
	_ASSERTE(m_InitialSize>=1000 && m_SubsequentBlockSize>=1000);
	// Per block
	for(unsigned char blockidx=0;blockidx<m_NumBlocks;++blockidx)
	{
		Block(blockidx).Invariant();
		_ASSERTE(BlockSize(blockidx)==Block(blockidx).NumBytesLeft());
		if(m_NumBlocks>1)
		{
			// If there's more than one block, then they should all be bigger than the cut off point
			const int32_t size=BlockSize(blockidx);
			_ASSERTE(size>eBlockCutOffPointBytes);
		}
		_ASSERTE(IsValidBlockIdx(blockidx));
	}
	// SmallestBlock
	_ASSERTE((!m_NumBlocks && !SmallestBlock()) || (m_NumBlocks>0 && SmallestBlock()));
	// LastBlockIdx
	_ASSERTE(!m_NumBlocks || (m_NumBlocks>0 && LastBlockIdx()==m_NumBlocks-1));
	// IsValidBlockIdx
	_ASSERTE(!m_NumBlocks || (!IsValidBlockIdx(m_NumBlocks)));
	// m_NumBlocks
	_ASSERTE(m_NumBlocks<=eMaxNumBlocks);
	// SpaceForAnotherBlock
	_ASSERTE((m_NumBlocks<eMaxNumBlocks && SpaceForAnotherBlock()) ||
	 (m_NumBlocks==eMaxNumBlocks && !SpaceForAnotherBlock()));
#endif
}

template<typename POLYTYPE>
int32_t tBlockAllocatorT<POLYTYPE>::NextBlockSize(void) const
{
	int32_t nextblocksize=((!m_NumBlocks)?m_InitialSize:m_SubsequentBlockSize);
	nextblocksize+=AlignmentPaddingForBlocksize(nextblocksize);
	return nextblocksize;
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::ManageObjectDestruction(const unsigned char blockidx,POLYTYPE& managedobject)
{
	Block(blockidx).ManageObjectDestruction(managedobject);
	UpdateBlockSize(blockidx);
}

template<>
inline void tBlockAllocatorT<tBlockAllocatorRefCounter>::ManageObjectDestruction(const unsigned char blockidx,
 tBlockAllocatorRefCounter& managedobject)
{
	Block(blockidx).ManageObjectDestruction(managedobject);
	managedobject.ProxyRefCounterSetObject(m_RefCount);
	UpdateBlockSize(blockidx);
}

template<typename POLYTYPE>
void tBlockAllocatorT<POLYTYPE>::UpdateBlockSize(const unsigned char blockidx)
{
	m_BlockSizes[blockidx]=Block(blockidx).NumBytesLeft();
	_ASSERTE(BlockSize(blockidx)>=0);
}

template<typename POLYTYPE,typename TYPE>
TYPE& AllocateAndConstructPoly(
 tBlockAllocatorT<POLYTYPE>& allocator,
 const int32_t size,
 typename const TYPE::tCtorArgs& args)
{
	return allocator.AllocateAndConstructPoly<TYPE>(args,size);
}

template<typename POLYTYPE,typename TYPE>
TYPE& AllocateAndConstructPoly(
 tBlockAllocatorT<POLYTYPE>& allocator,
 const int32_t size)
{
	return allocator.AllocateAndConstructPoly<TYPE>(size);
}