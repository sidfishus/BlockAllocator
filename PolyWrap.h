#pragma once

// Wrap an object which is not polymorphic and allow it to be treated in a polymorphic way. POLYTYPE must have a
//  virtual destructor for this to work
template<typename POLYTYPE,typename TYPE>
class tPolyWrapT : public POLYTYPE
{
	TYPE m_Wrapped;
	//~V
	tPolyWrapT(const tPolyWrapT&);														// I'm not sure what it would mean to copy
																									//  this, so prevent until it's required
	tPolyWrapT& operator=(const tPolyWrapT&);
public:
	tPolyWrapT(void);
	tPolyWrapT(typename const TYPE::tCtorArgs& args);
	TYPE& operator*(void);																	// Dereference to access the object
};

template<typename POLYTYPE,typename TYPE>
tPolyWrapT<POLYTYPE,TYPE>::tPolyWrapT(void)
{
}

template<typename POLYTYPE,typename TYPE>
tPolyWrapT<POLYTYPE,TYPE>::tPolyWrapT(typename const TYPE::tCtorArgs& args):m_Wrapped(args)
{
}

template<typename POLYTYPE,typename TYPE>
TYPE& tPolyWrapT<POLYTYPE,TYPE>::operator*(void)
{
	return m_Wrapped;
}