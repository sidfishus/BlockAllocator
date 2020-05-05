// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#define NOMINMAX
#include <windows.h>
#include <type_traits>

#include <stdlib.h>
#include <iostream>
#include <new>
#include <memory>
#include <limits>

using std::tr1::aligned_storage;
using std::tr1::alignment_of;
using std::numeric_limits;

#define PANIC

#include "IPoly.h"
#include "EmptyClass.h"
#include "ProxyRefCounter.h"

#define tProxyRefCounter(TYPE) tProxyRefCounterT<IPoly,TYPE>

#include "BlockAllocator.h"

#include "PsyncArray.h"

typedef tBlockAllocatorT<tBlockAllocatorRefCounter> tBlockAllocator;

typedef tBlockAllocatorRefCounter tPolyBaseClass;

#define tPArray(TYPE) tPArrayT<tPolyBaseClass,TYPE>

template<typename TYPE,typename ALLOCATOR>
tPArray(TYPE)& PMakeArray(ALLOCATOR& allocator,const int32_t numelements)
{
	return PMakeArrayT<tPolyBaseClass,TYPE,ALLOCATOR>(allocator,numelements);
}



// TODO: reference additional headers your program requires here
