#pragma once

#include <type_traits>
#include "EmptyClass.h"
#include "IPoly.h"

// An object where the construction can be delayed. Useful for creating an instance of a class but not instanciating
//  it until later on.
// Template base class is so you can create polymorphic and non-polymorphic instances of this class
template<typename TYPE,typename BASECLASS>
class tLazyT : public BASECLASS
{
//=====================================================================================================================
// PRIVATE
//=====================================================================================================================
	typedef typename std::tr1::aligned_storage<sizeof(TYPE),
	 std::tr1::alignment_of<TYPE>::value>::type tMemory;
	bool m_Constructed;
	tMemory m_Memory;																			// Memory aligned to the correct
																									//  size/boundary of TYPE. You cannot simply
																									//  use char array[sizeof(TYPE)] to do this
																									//  due to alignment issues.
	//~V
	void Invariant();
	template<typename OTHER>
	void _Construct(OTHER rhs);
	tLazyT(const tLazyT&);
	tLazyT& operator=(const tLazyT&);
	//~F
//=====================================================================================================================
// PROPERTIES
//=====================================================================================================================
public:
	bool IsConstructed(void) const;
//=====================================================================================================================
// FUNCTIONS/MODIFIERS
//=====================================================================================================================
	tLazyT(void);
	~tLazyT(void);
	void Construct(void);																	// Construct the object
	void Construct(typename const TYPE::tCtorArgs& args);							// Construct the object
	void operator=(const TYPE& rhs);														// Assign the object
	void Clear(void);																			// Destruct the object
	TYPE& operator*(void);																	// Dereference to get at the object
	//~PF
};

template<typename TYPE,typename BASECLASS>
tLazyT<TYPE,BASECLASS>::tLazyT(void):m_Constructed(false)
{
	Invariant();
}

template<typename TYPE,typename BASECLASS>
tLazyT<TYPE,BASECLASS>::~tLazyT(void)
{
	Clear();
}

template<typename TYPE,typename BASECLASS>
void tLazyT<TYPE,BASECLASS>::Construct(typename const TYPE::tCtorArgs& args)
{
	_Construct(args);
}

template<typename TYPE,typename BASECLASS>
void tLazyT<TYPE,BASECLASS>::operator=(const TYPE& rhs)
{
	_Construct(rhs);
}

template<typename TYPE,typename BASECLASS>
bool tLazyT<TYPE,BASECLASS>::IsConstructed(void) const
{
	return m_Constructed;
}

template<typename TYPE,typename BASECLASS>
void tLazyT<TYPE,BASECLASS>::Clear(void)
{
	Invariant();
	if(m_Constructed)
	{
		operator*().~TYPE();
		m_Constructed=false;
	}
	Invariant();
}

template<typename TYPE,typename BASECLASS>
TYPE& tLazyT<TYPE,BASECLASS>::operator*(void)
{
	Invariant();
	_ASSERTE(m_Constructed);
	return reinterpret_cast<TYPE&>(m_Memory);
}

template<typename TYPE,typename BASECLASS>
void tLazyT<TYPE,BASECLASS>::Invariant()
{
#ifdef _DEBUG
#endif
}

template<typename TYPE,typename BASECLASS>
void tLazyT<TYPE,BASECLASS>::Construct(void)
{
	Invariant();
	Clear();
	::new(static_cast<void*>(&m_Memory)) TYPE();
	m_Constructed=true;
	Invariant();
}

template<typename TYPE,typename BASECLASS>
template<typename OTHER>
void tLazyT<TYPE,BASECLASS>::_Construct(OTHER rhs)
{
	Invariant();
	Clear();
	::new(static_cast<void*>(&m_Memory)) TYPE(rhs);
	m_Constructed=true;
	Invariant();
}

// These would be template 'using' declarations in C++ 11.
#define tLazy(TYPE) tLazyT<TYPE,tEmptyClass>
#define tPolyLazy(TYPE) tLazyT<TYPE,IPoly>