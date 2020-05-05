#pragma once

class tRefCount
{
	long m_Count;
public:
	tRefCount(void);
	void AddRef(void);
	void Release(void);
	long Count(void) const;
};

inline tRefCount::tRefCount(void):m_Count(0)
{
}

inline void tRefCount::AddRef(void)
{
	InterlockedIncrement(&m_Count);
}

inline void tRefCount::Release(void)
{
	InterlockedDecrement(&m_Count);
}

inline long tRefCount::Count(void) const
{
	return m_Count;
}