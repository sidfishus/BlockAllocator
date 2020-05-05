#pragma once

// A block is arranged in memory as follows:
// [previous block ptr][current ptr][last block byte ptr][count of non-POD ptrs][memory ...............][managed ptr]
//  [managed ptr]

template<typename POLYTYPE>
class tManagedMemoryBlockT
{
//=====================================================================================================================
// PROPERTIES
//=====================================================================================================================
public:
	enum
	{
		eOverheadForManagedObject=sizeof(POLYTYPE*),									// The overhead required for managing an
																									//  object lifespan
	};
	int32_t NumManagedObjects(void) const;												// The number of objects allocated where
																									//  destruction is managed by the the memory
																									//  block
	int32_t NumBytesLeft(void) const;													// The number of bytes left in the memory
																									//  block that can be used for allocation
	int32_t NumBytesUsed(void) const;													// The number of bytes used including the
																									//  managed objects
//=====================================================================================================================
// FUNCTIONS/MODIFIERS
//=====================================================================================================================
	tManagedMemoryBlockT(
	 tManagedMemoryBlockT* const previousblock,
	 const int32_t blocksize,
	 const bool zeroinitialise) throw();												// Zero initialising memory to begin with
																									//  means the performance is improved the
																									//  next time the memory is accessed.
																									//  10-15% speed increase on a quad-core
																									//  Windows Vista machine 8GB RAM.
	~tManagedMemoryBlockT(void);
	void Invariant(void) const;
	bool EnoughSpace(
	 const int32_t size,
	 const unsigned short alignmentpadrequired,
	 const bool ismanaged) const;															// Is there enough space for an object of
																									//  this size/alignment?
	void ManageObjectDestruction(POLYTYPE& managedobject);						// Manage the destruction of this object
	unsigned short AlignmentPadRequired(const unsigned short alignment)
	 const;																						// Padding required to allocate an object
																									//  with this alignment
	void* Use(
	 const int32_t size,
	 const unsigned short alignment,
	 const bool ismanaged);																	// Use this amount of memory with this
																									//  alignment requirement. Returns
																									//  a pointer to the memory
	void ChainAttachBlock(tManagedMemoryBlockT& block) throw();					// Add this block to the end of the previous
																									//  block chain
	tManagedMemoryBlockT* PreviousBlock(void);										// Return the previous block (if any)
private:
//=====================================================================================================================
// PRIVATE
//=====================================================================================================================
	tManagedMemoryBlockT* m_PreviousBlock;										
	char* m_Ptr;																				
	char* const m_EndBytePtr;																
	int32_t m_NumManagedObjects;
	//~V
	char* BeginBytePtr(void);																// The beginning of the memory
	const char* BeginBytePtr(void) const;
	char* EndAllocateableBytePtr(void);													// The last byte in the block of memory + 1
																									//  that is available for allocation. This
																									//  will be different to 'EndBytePtr' where
																									//  there are objects allocated which are
																									//  managed by the allocator
	const char* EndAllocateableBytePtr(void) const;
	const POLYTYPE** PFirstManagedObject(void) const;								// Pointer to the first managed object
	POLYTYPE** PFirstManagedObject(void);
	void DestroyManagedObjects(void);
	//~F
};

//=====================================================================================================================
// IMPLEMENTATION
//=====================================================================================================================

template<typename POLYTYPE>
tManagedMemoryBlockT<POLYTYPE>::tManagedMemoryBlockT(tManagedMemoryBlockT* const previousblock,const int32_t blocksize,
 const bool zeroinitialise) throw():m_PreviousBlock(previousblock),m_Ptr(BeginBytePtr()),
//warning C4355: 'this' : used in base member initializer list
#pragma warning(suppress:4355)
 m_EndBytePtr(reinterpret_cast<char*>(this)+blocksize),m_NumManagedObjects(0)
{
	_ASSERTE(blocksize>0);
	if(zeroinitialise)
	{
		// Zero initialise the memory (not 'this'!)
		memset(BeginBytePtr(),0,blocksize-sizeof(*this));
	}
	Invariant();
}

template<typename POLYTYPE>
tManagedMemoryBlockT<POLYTYPE>::~tManagedMemoryBlockT(void)
{
	Invariant();
	DestroyManagedObjects();
	Invariant();
}

template<typename POLYTYPE>
void tManagedMemoryBlockT<POLYTYPE>::ChainAttachBlock(tManagedMemoryBlockT& block) throw()
{
	Invariant();
	for(tManagedMemoryBlockT* iterblock=this;;/* Increment done inside the loop */)
	{
		if(!iterblock->m_PreviousBlock)
		{
			// Found the end - attach this one.
			iterblock->m_PreviousBlock=&block;
			break;
		}
		iterblock=iterblock->m_PreviousBlock;
	}
	Invariant();
}

template<typename POLYTYPE>
int32_t tManagedMemoryBlockT<POLYTYPE>::NumManagedObjects(void) const
{
	return m_NumManagedObjects;
}

template<typename POLYTYPE>
const POLYTYPE** tManagedMemoryBlockT<POLYTYPE>::PFirstManagedObject(void) const
{
	const POLYTYPE** const pfirst=reinterpret_cast<const POLYTYPE**>(m_EndBytePtr-(1*sizeof(POLYTYPE*)));
	// Sanity check
	_ASSERTE(reinterpret_cast<const char*>(pfirst)<m_EndBytePtr);
	return pfirst;
}

template<typename POLYTYPE>
POLYTYPE** tManagedMemoryBlockT<POLYTYPE>::PFirstManagedObject(void)
{
	return const_cast<POLYTYPE**>(static_cast<const tManagedMemoryBlockT&>(*this).PFirstManagedObject());
}

template<typename POLYTYPE>
const char* tManagedMemoryBlockT<POLYTYPE>::BeginBytePtr(void) const
{
	// The first byte of memory is 'this' + the sizeof this struct.
	return reinterpret_cast<const char*>(this)+sizeof(*this);
}

template<typename POLYTYPE>
char* tManagedMemoryBlockT<POLYTYPE>::BeginBytePtr(void)
{
	return const_cast<char*>(static_cast<const tManagedMemoryBlockT&>(*this).BeginBytePtr());
}

template<typename POLYTYPE>
char* tManagedMemoryBlockT<POLYTYPE>::EndAllocateableBytePtr(void)
{
	return const_cast<char*>(static_cast<const tManagedMemoryBlockT&>(*this).EndAllocateableBytePtr());
}

template<typename POLYTYPE>
const char* tManagedMemoryBlockT<POLYTYPE>::EndAllocateableBytePtr(void) const
{
	char* rv=m_EndBytePtr-(NumManagedObjects()*sizeof(POLYTYPE*));
	// Sanity check
	_ASSERTE(rv>=m_Ptr && rv<=m_EndBytePtr);
	return rv;
}

template<typename POLYTYPE>
int32_t tManagedMemoryBlockT<POLYTYPE>::NumBytesLeft(void) const
{
	const int32_t rv=static_cast<int32_t>(EndAllocateableBytePtr()-m_Ptr);
	// At no point can there be a negative number of bytes left
	_ASSERTE(rv>=0);
	return rv;
}

template<typename POLYTYPE>
int32_t tManagedMemoryBlockT<POLYTYPE>::NumBytesUsed(void) const
{
	const int32_t rv=static_cast<int32_t>((m_Ptr-BeginBytePtr())+(m_NumManagedObjects*sizeof(POLYTYPE*)));
	// Never should the number of bytes used be less than 0
	_ASSERTE(rv>=0);
	return rv;
}

template<typename POLYTYPE>
void tManagedMemoryBlockT<POLYTYPE>::ManageObjectDestruction(POLYTYPE& managedobject)
{
	Invariant();
	// Get the next managed object to use
	POLYTYPE*& pnewmanaged=*(PFirstManagedObject()-m_NumManagedObjects);
	pnewmanaged=&managedobject;
	++m_NumManagedObjects;
	Invariant();
}

template<typename POLYTYPE>
unsigned short tManagedMemoryBlockT<POLYTYPE>::AlignmentPadRequired(const unsigned short alignment) const
{
	return static_cast<unsigned short>(reinterpret_cast<uintptr_t>(m_Ptr)%alignment);
}

template<typename POLYTYPE>
void tManagedMemoryBlockT<POLYTYPE>::DestroyManagedObjects(void)
{
	Invariant();
	POLYTYPE** pmanagedobject=PFirstManagedObject();
	for(int32_t i=0;i<m_NumManagedObjects;++i)
	{
		// Call the virtual destructor
		(*pmanagedobject)->~POLYTYPE();
		// Work backwards
		--pmanagedobject;
	}
	m_NumManagedObjects=0;
	Invariant();
}

template<typename POLYTYPE>
void tManagedMemoryBlockT<POLYTYPE>::Invariant(void) const
{
#ifdef _DEBUG
	if(m_PreviousBlock)
	{
		m_PreviousBlock->Invariant();
	}
	const char* const beginbyteptr=BeginBytePtr();
	// ==== m_Ptr ======================================================================================================
	_ASSERTE(m_Ptr<=m_EndBytePtr); // Would mean the pointer has gone past the memory
	_ASSERTE(m_Ptr<=EndAllocateableBytePtr()); // Would mean the pointer has gone past the allocateable area
	_ASSERTE(!m_Ptr || m_Ptr>=beginbyteptr); // Ptr could be 0 if it wrapped (unlikely)
	_ASSERTE(!m_Ptr || m_Ptr>reinterpret_cast<const char*>(this));
	// ==== m_EndBytePtr ===============================================================================================
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>reinterpret_cast<const char*>(this)); // Could be 0 it wrapped (unlikely).
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>beginbyteptr);
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>=EndAllocateableBytePtr());
	// ==== EndAllocateableBytePtr =====================================================================================
	_ASSERTE(!EndAllocateableBytePtr() ||
	 EndAllocateableBytePtr()>reinterpret_cast<const char*>(this)); // Could be 0 if wrapped (unlikely)
	_ASSERTE(!EndAllocateableBytePtr() || EndAllocateableBytePtr()>beginbyteptr);
	_ASSERTE(!EndAllocateableBytePtr() || EndAllocateableBytePtr()<=m_EndBytePtr);
	// ==== BeginBytePtr() ============================================================================================
	_ASSERTE(BeginBytePtr()==reinterpret_cast<const char*>(this)+sizeof(*this));
	// ==== m_NumManagedObjects =======================================================================================
	_ASSERTE(m_NumManagedObjects<=NumBytesUsed()); // Can't have more managed objects than bytes used
	// ==== PFirstManagedObject =======================================================================================
	const char* const pfirstmanagedobject=reinterpret_cast<const char*>(PFirstManagedObject());
	_ASSERTE(pfirstmanagedobject>reinterpret_cast<const char*>(this));
	_ASSERTE(pfirstmanagedobject>beginbyteptr);
	// ==== Misc =======================================================================================================
	_ASSERTE(m_EndBytePtr-BeginBytePtr()==NumBytesLeft()+NumBytesUsed()); // Test the counts
	_ASSERTE(NumBytesLeft()>=0);
	_ASSERTE(NumBytesUsed()>=0);
#endif
}

template<typename POLYTYPE>
bool tManagedMemoryBlockT<POLYTYPE>::EnoughSpace(const int32_t size,const unsigned short alignmentpadrequired,
 const bool ismanaged) const
{
	// Why would you allocate <=0 bytes?
	_ASSERTE(size>0);
	const int32_t nbytesleft=NumBytesLeft();
	const int32_t sizeneeded=static_cast<int32_t>(alignmentpadrequired+size+((ismanaged)?sizeof(POLYTYPE*):0));
	return (sizeneeded<=nbytesleft);
}

template<typename POLYTYPE>
void* tManagedMemoryBlockT<POLYTYPE>::Use(const int32_t size,const unsigned short alignment,const bool ismanaged)
{
	Invariant();
	void* rv;
	const unsigned short pad=AlignmentPadRequired(alignment);
	if(EnoughSpace(size,pad,ismanaged))
	{
		if(pad>0)
		{
			// Add the padding to align on correct boundary
			m_Ptr+=pad;
		}
		// Ptr should now be 'alignment' aligned
		_ASSERTE(!(reinterpret_cast<uintptr_t>(m_Ptr)%alignment));
		rv=m_Ptr;
		// Increment the ptr ready for another object
		m_Ptr+=size;
	}
	else
	{
		rv=NULL;
	}
	Invariant();
	return rv;
}

template<typename POLYTYPE>
tManagedMemoryBlockT<POLYTYPE>* tManagedMemoryBlockT<POLYTYPE>::PreviousBlock(void)
{
	return m_PreviousBlock;
}