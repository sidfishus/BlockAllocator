#include "stdafx.h"
#include "ManagedMemoryBlock.h"

tManagedMemoryBlockT::tManagedMemoryBlockT(tManagedMemoryBlockT* const previousblock,const int32_t blocksize,
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

tManagedMemoryBlockT::~tManagedMemoryBlockT(void)
{
	Invariant();
	DestroyManagedObjects();
	if(m_PreviousBlock)
	{
		// Call destructor on the previous block, and so on
		m_PreviousBlock->~tManagedMemoryBlockT();
	}
	Invariant();
}

int32_t tManagedMemoryBlockT::NumManagedObjects(void) const
{
	return m_NumManagedObjects;
}

const POLYTYPE** tManagedMemoryBlockT::PFirstManagedObject(void) const
{
	const POLYTYPE** const pfirst=reinterpret_cast<const POLYTYPE**>(m_EndBytePtr-(1*sizeof(POLYTYPE*)));
	// Sanity check
	_ASSERTE(reinterpret_cast<const char*>(pfirst)>=m_Ptr &&
	 reinterpret_cast<const char*>(pfirst)<m_EndBytePtr);
	return pfirst;
}

POLYTYPE** tManagedMemoryBlockT::PFirstManagedObject(void)
{
	return const_cast<POLYTYPE**>(static_cast<const tManagedMemoryBlockT&>(*this).PFirstManagedObject());
}

const char* tManagedMemoryBlockT::BeginBytePtr(void) const
{
	// The first byte of memory is 'this' + the sizeof this struct.
	return reinterpret_cast<const char*>(this)+sizeof(*this);
}

char* tManagedMemoryBlockT::BeginBytePtr(void)
{
	return const_cast<char*>(static_cast<const tManagedMemoryBlockT&>(*this).BeginBytePtr());
}

char* tManagedMemoryBlockT::EndAllocateableBytePtr(void)
{
	return const_cast<char*>(static_cast<const tManagedMemoryBlockT&>(*this).EndAllocateableBytePtr());
}

const char* tManagedMemoryBlockT::EndAllocateableBytePtr(void) const
{
	char* rv=m_EndBytePtr-(NumManagedObjects()*sizeof(POLYTYPE*));
	// Sanity check
	_ASSERTE(rv>=m_Ptr && rv<=m_EndBytePtr);
	return rv;
}

int32_t tManagedMemoryBlockT::NumBytesLeft(void) const
{
	const int32_t rv=static_cast<int32_t>(EndAllocateableBytePtr()-m_Ptr);
	// At no point can there be a negative number of bytes left
	_ASSERTE(rv>=0);
	return rv;
}

int32_t tManagedMemoryBlockT::NumBytesUsed(void) const
{
	const int32_t rv=static_cast<int32_t>((m_Ptr-BeginBytePtr())+(m_NumManagedObjects*sizeof(POLYTYPE*)));
	// Never should the number of bytes used be less than 0
	_ASSERTE(rv>=0);
	return rv;
}

void tManagedMemoryBlockT::ManageObjectDestruction(POLYTYPE& managedobject)
{
	Invariant();
	// Get the next managed object to use
	POLYTYPE*& pnewmanaged=*(PFirstManagedObject()-m_NumManagedObjects);
	pnewmanaged=&managedobject;
	++m_NumManagedObjects;
	Invariant();
}

unsigned short tManagedMemoryBlockT::AlignmentPadRequired(const unsigned short alignment) const
{
	return static_cast<unsigned short>(reinterpret_cast<uintptr_t>(m_Ptr)%alignment);
}

// I'm not sure this is a good idea. Should the memory block deal with deleting itself and it's children?
tManagedMemoryBlockT* tManagedMemoryBlockT::NextBlock(void)
{
	return m_PreviousBlock;
}

void tManagedMemoryBlockT::DestroyManagedObjects(void)
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

void tManagedMemoryBlockT::Invariant(void) const
{
#ifdef _DEBUG
	if(m_PreviousBlock)
	{
		m_PreviousBlock->Invariant();
	}
	// ==== m_Ptr ======================================================================================================
	_ASSERTE(m_Ptr<=m_EndBytePtr); // Would mean the pointer has gone past the memory
	_ASSERTE(m_Ptr<=EndAllocateableBytePtr()); // Would mean the pointer has gone past the allocateable area
	_ASSERTE(!m_Ptr || m_Ptr>=BeginBytePtr()); // Ptr could be 0 if it wrapped (unlikely)
	_ASSERTE(!m_Ptr || m_Ptr>reinterpret_cast<const char*>(this));
	// ==== m_EndBytePtr ===============================================================================================
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>reinterpret_cast<const char*>(this)); // Could be 0 it wrapped (unlikely).
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>BeginBytePtr());
	_ASSERTE(!m_EndBytePtr || m_EndBytePtr>=EndAllocateableBytePtr());
	// ==== EndAllocateableBytePtr =====================================================================================
	_ASSERTE(!EndAllocateableBytePtr() ||
	 EndAllocateableBytePtr()>reinterpret_cast<const char*>(this)); // Could be 0 if wrapped (unlikely)
	_ASSERTE(!EndAllocateableBytePtr() || EndAllocateableBytePtr()>BeginBytePtr());
	_ASSERTE(!EndAllocateableBytePtr() || EndAllocateableBytePtr()<=m_EndBytePtr);
	// ==== BeginBytePtr() ============================================================================================
	_ASSERTE(BeginBytePtr()==reinterpret_cast<const char*>(this)+sizeof(*this));
	// ==== m_NumManagedObjects =======================================================================================
	_ASSERTE(m_NumManagedObjects<=NumBytesUsed()); // Can't have more managed objects than bytes used
	// ==== PFirstManagedObject =======================================================================================
	_ASSERTE(reinterpret_cast<const char*>(PFirstManagedObject())>reinterpret_cast<const char*>(this));
	_ASSERTE(reinterpret_cast<const char*>(PFirstManagedObject())>BeginBytePtr());
	// ==== Misc =======================================================================================================
	_ASSERTE(m_EndBytePtr-BeginBytePtr()==NumBytesLeft()+NumBytesUsed()); // Test the counts
	_ASSERTE(NumBytesLeft()>=0);
	_ASSERTE(NumBytesUsed()>=0);
#endif
}

bool tManagedMemoryBlockT::EnoughSpace(const int32_t size,const unsigned short alignmentpadrequired,
 const bool ismanaged) const
{
	// Why would you allocate <=0 bytes?
	_ASSERTE(size>0);
	const int32_t nbytesleft=NumBytesLeft();
	const int32_t sizeneeded=static_cast<int32_t>(alignmentpadrequired+size+((ismanaged)?sizeof(POLYTYPE*):0));
	return (sizeneeded<=nbytesleft);
}

void* tManagedMemoryBlockT::Use(const int32_t size,const unsigned short alignment,const bool ismanaged)
{
	Invariant();
	void* rv;
	const unsigned short pad=AlignmentPadRequired(alignment);
	if(EnoughSpace(pad,size,ismanaged))
	{
		if(pad>0)
		{
			// Add the padding to align on correct boundary
			m_Ptr+=pad;
		}
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