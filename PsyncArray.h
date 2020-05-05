#pragma once

template<typename BASECLASS,typename TYPE>
class tPArrayT : public BASECLASS
{
public:
	class iterator;
	struct tCtorArgs;
	friend class tPArray_UnitTest;
	tPArrayT(const tCtorArgs& args);
	~tPArrayT(void);
	iterator begin();
	iterator end();
	int32_t NumReserved(void) const;														// Maximum number of elements
	int32_t Size(void) const;																// Number of elements constructed
	TYPE& Construct(void);																	// Construct another element
	TYPE& Construct(typename const TYPE::tCtorArgs& args);						// Construct another element
	void Clear(void);																			// Remove all the constructed elements
	TYPE& operator[](const int32_t index);												// Item at this index/offset
	const TYPE& operator[](const int32_t index) const;
private:
	const int32_t m_NumElements;															// Maximum number of elements
	int32_t m_NumConstructed;																// Number of elements constructed
	//~V
	TYPE& BeginPtr(void);
	const TYPE& BeginPtr(void) const;
	TYPE& EndPtr(void);																		// The end of the constructed array
	const TYPE& EndPtr(void) const;
	TYPE& EndOfArrayPtr(void);																// The end of the array
	const TYPE& EndOfArrayPtr(void) const;
	iterator CreateIterator(TYPE& item);
	void Invariant(void) const;
	TYPE& ElementAt_NoCheck(const int32_t index);									// The element at this index without
																									//  checking if it's constructed
	const TYPE& ElementAt_NoCheck(const int32_t index) const;
	//~F
};

template<typename BASECLASS,typename TYPE>
struct tPArrayT<BASECLASS,TYPE>::tCtorArgs
{
	int32_t NumElements;
};

//=====================================================================================================================
// ARRAY CREATION FUNCTIONS
//=====================================================================================================================

// Template create an array from an allocator
template<typename BASECLASS,typename TYPE,typename ALLOCATOR>
tPArrayT<BASECLASS,TYPE>& PMakeArrayT(
 ALLOCATOR& allocator,
 const int32_t numelements);

//=====================================================================================================================
// ITERATOR CLASS DECLARATION
//=====================================================================================================================

template<typename BASECLASS,typename TYPE>
class tPArrayT<BASECLASS,TYPE>::iterator
{
	TYPE* m_PItem;
#ifdef _DEBUG
	tPArrayT<BASECLASS,TYPE>* m_Container;
#endif
	//~V
	void Invariant(void) const;
	//~F
public:
	iterator(void);
	iterator(TYPE& item
#ifdef _DEBUG
	 ,tPArrayT<BASECLASS,TYPE>& container
#endif
	);
	iterator operator++(void);
	iterator operator++(int);
	iterator operator--(void);
	iterator operator--(int);
	bool operator==(const iterator& rhs) const;
	bool operator!=(const iterator& rhs) const;
	TYPE& operator*(void);
};

//=====================================================================================================================
// IMPLEMENTATION
//=====================================================================================================================

template<typename BASECLASS,typename TYPE,typename ALLOCATOR>
tPArrayT<BASECLASS,TYPE>& PMakeArrayT(ALLOCATOR& allocator,const int32_t numelements)
{
	_ASSERTE(numelements>0);
	typedef tPArrayT<BASECLASS,TYPE> _tPArray;
	const int32_t size=sizeof(_tPArray)+(sizeof(TYPE)*numelements);
	const _tPArray::tCtorArgs args=
	{
		numelements,
	};
	// The BASECLASS is used as the POLYTYPE for the allocator
	return AllocateAndConstructPoly<BASECLASS,_tPArray>(allocator,size,args);
}

template<typename BASECLASS,typename TYPE>
tPArrayT<BASECLASS,TYPE>::tPArrayT(const tCtorArgs& args):m_NumElements(args.NumElements),m_NumConstructed(0)
{
	_ASSERTE(m_NumElements>0);
	Invariant();
}

template<typename BASECLASS,typename TYPE>
tPArrayT<BASECLASS,TYPE>::~tPArrayT(void)
{
	Clear();
}

template<typename BASECLASS,typename TYPE>
void tPArrayT<BASECLASS,TYPE>::Clear(void)
{
	Invariant();
	for(TYPE* elemptr=&BeginPtr();elemptr!=&EndPtr();++elemptr)
	{
		// Destruct the element
		elemptr->~TYPE();
	}
	m_NumConstructed=0;
	Invariant();
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::BeginPtr(void)
{
	return const_cast<TYPE&>(static_cast<const tPArrayT&>(*this).BeginPtr());
}

template<typename BASECLASS,typename TYPE>
const TYPE& tPArrayT<BASECLASS,TYPE>::BeginPtr(void) const
{
	return *reinterpret_cast<const TYPE*>(reinterpret_cast<const char*>(this)+sizeof(*this));
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::EndPtr(void)
{
	return const_cast<TYPE&>(static_cast<const tPArrayT&>(*this).EndPtr());
}

template<typename BASECLASS,typename TYPE>
const TYPE& tPArrayT<BASECLASS,TYPE>::EndPtr(void) const
{
	return *reinterpret_cast<const TYPE*>(&(BeginPtr())+m_NumConstructed);
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::EndOfArrayPtr(void)
{
	return const_cast<TYPE&>(static_cast<const tPArrayT&>(*this).EndOfArrayPtr());
}

template<typename BASECLASS,typename TYPE>
const TYPE& tPArrayT<BASECLASS,TYPE>::EndOfArrayPtr(void) const
{
	return *reinterpret_cast<const TYPE*>(&(BeginPtr())+m_NumElements);
}

template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::CreateIterator(TYPE& item)
{
	Invariant();
#ifdef _DEBUG
	return iterator(item,*this);
#else
	return iterator(item);
#endif
}

template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::begin(void)
{
	return CreateIterator(BeginPtr());
}

template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::end(void)
{
	return CreateIterator(EndPtr());
}

template<typename BASECLASS,typename TYPE>
void tPArrayT<BASECLASS,TYPE>::Invariant(void) const
{
#ifdef _DEBUG
	_ASSERTE(m_NumElements>0);
	_ASSERTE(m_NumConstructed>=0);
	// Can't construct more elements than are in the array
	_ASSERTE(m_NumConstructed<=m_NumElements);
	_ASSERTE(reinterpret_cast<const char*>(&(BeginPtr()))==
	 reinterpret_cast<const char*>(reinterpret_cast<const char*>(this)+sizeof(*this)));
	_ASSERTE(m_NumConstructed<m_NumElements || &(EndPtr())==&(EndOfArrayPtr()));
#endif
}

template<typename BASECLASS,typename TYPE>
tPArrayT<BASECLASS,TYPE>::iterator::iterator(void):m_PItem(NULL),m_Container(NULL)
{
	Invariant();
}

template<typename BASECLASS,typename TYPE>
tPArrayT<BASECLASS,TYPE>::iterator::iterator(TYPE& item
#ifdef _DEBUG
 ,tPArrayT<BASECLASS,TYPE>& container
#endif
 ):m_PItem(&item)
#ifdef _DEBUG
 ,m_Container(&container)
#endif
{
	Invariant();
}

template<typename BASECLASS,typename TYPE>
bool tPArrayT<BASECLASS,TYPE>::iterator::operator==(const iterator& rhs) const
{
	_ASSERTE(rhs.m_Container==m_Container);
	return rhs.m_PItem==m_PItem;
}

template<typename BASECLASS,typename TYPE>
bool tPArrayT<BASECLASS,TYPE>::iterator::operator!=(const iterator& rhs) const
{
	_ASSERTE(rhs.m_Container==m_Container);
	return rhs.m_PItem!=m_PItem;
}

template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::iterator::operator++(void)
{
	Invariant();
	++m_PItem;
	Invariant();
	return *this;
}

// Postincrement
template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::iterator::operator++(int)
{
	Invariant();
	iterator copy(*this);
	++m_PItem;
	Invariant();
	return copy;
}

template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::iterator::operator--(void)
{
	Invariant();
	--m_PItem;
	Invariant();
	return *this;
}

// Post decrement
template<typename BASECLASS,typename TYPE>
typename tPArrayT<BASECLASS,TYPE>::iterator tPArrayT<BASECLASS,TYPE>::iterator::operator--(int)
{
	Invariant();
	iterator copy(*this);
	--m_PItem;
	Invariant();
	return copy;
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::iterator::operator*(void)
{
	_ASSERTE(m_PItem);
#ifdef _DEBUG
	// Can't be outside the bounds of the array
	_ASSERTE(m_PItem>=&(m_Container->BeginPtr()) && m_PItem<&(m_Container->EndPtr()));
#endif
	return *m_PItem;
}

template<typename BASECLASS,typename TYPE>
void tPArrayT<BASECLASS,TYPE>::iterator::Invariant(void) const
{
#ifdef _DEBUG
	m_Container->Invariant();
#endif
}

template<typename BASECLASS,typename TYPE>
int32_t tPArrayT<BASECLASS,TYPE>::NumReserved(void) const
{
	return m_NumElements;
}

template<typename BASECLASS,typename TYPE>
int32_t tPArrayT<BASECLASS,TYPE>::Size(void) const
{
	return m_NumConstructed;
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::ElementAt_NoCheck(const int32_t index)
{
	return const_cast<TYPE&>(static_cast<const tPArrayT&>(*this).ElementAt_NoCheck(index));
}

template<typename BASECLASS,typename TYPE>
const TYPE& tPArrayT<BASECLASS,TYPE>::ElementAt_NoCheck(const int32_t index) const
{
	_ASSERTE(index<m_NumElements);
	return *(&(BeginPtr())+index);
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::Construct(void)
{
	Invariant();
	_ASSERTE(m_NumConstructed<m_NumElements);
	TYPE& item(ElementAt_NoCheck(m_NumConstructed));
	::new(static_cast<void*>(&item)) TYPE();
	++m_NumConstructed;
	Invariant();
	return item;
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::Construct(typename const TYPE::tCtorArgs& args)
{
	Invariant();
	_ASSERTE(m_NumConstructed<m_NumElements);
	TYPE& item(ElementAt_NoCheck(m_NumConstructed));
	::new(static_cast<void*>(&item)) TYPE(args);
	++m_NumConstructed;
	Invariant();
	return item;
}

template<typename BASECLASS,typename TYPE>
TYPE& tPArrayT<BASECLASS,TYPE>::operator[](const int32_t index)
{
	return const_cast<TYPE&>(static_cast<const tPArrayT&>(*this).operator[](index));
}

template<typename BASECLASS,typename TYPE>
const TYPE& tPArrayT<BASECLASS,TYPE>::operator[](const int32_t index) const
{
	_ASSERTE(index>=0);
	_ASSERTE(index<m_NumConstructed);
	_ASSERTE(index<m_NumElements);
	return ElementAt_NoCheck(index);
}