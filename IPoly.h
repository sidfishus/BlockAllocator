#pragma once

#include "PolyWrap.h"

// A polymorphic object with a virtual destructor. Base class for polymorphic objects
class IPoly
{
public:
	virtual ~IPoly(void)
	{
	}
};

#define tPolyWrap(TYPE) tPolyWrapT<IPoly,TYPE>