#pragma once

template<typename BASECLASS,typename OBJECT>
class tProxyRefCounterT : public BASECLASS
{
	OBJECT* m_Object;
public:
	tProxyRefCounterT(void);
	~tProxyRefCounterT(void);
	void ProxyRefCounterSetObject(OBJECT& object);
};

template<typename BASECLASS,typename OBJECT>
tProxyRefCounterT<BASECLASS,OBJECT>::tProxyRefCounterT(void):m_Object(NULL)
{
}

template<typename BASECLASS,typename OBJECT>
tProxyRefCounterT<BASECLASS,OBJECT>::~tProxyRefCounterT(void)
{
	if(m_Object)
	{
		m_Object->Release();
		m_Object=NULL;
	}
}

template<typename BASECLASS,typename OBJECT>
void tProxyRefCounterT<BASECLASS,OBJECT>::ProxyRefCounterSetObject(OBJECT& object)
{
	m_Object=&object;
	m_Object->AddRef();
}