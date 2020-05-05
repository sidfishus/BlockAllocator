#pragma once

// Memmove deals with overlapping memory regions safely

template<typename TYPE>
void ShiftLeft(
 TYPE* thearray,
 const uint32_t arraysize,
 const uint32_t indextomovefrom);

//=====================================================================================================================
// IMPLEMENTATION
//=====================================================================================================================

template<typename TYPE>
void ShiftLeft(TYPE* thearray,const uint32_t arraysize,const uint32_t indextomovefrom)
{
	_ASSERTE(thearray);
	_ASSERTE(indextomovefrom<arraysize);
	const uint32_t count=arraysize-indextomovefrom;
	TYPE* const source=thearray+indextomovefrom;
	TYPE* const dest=source-1;
	const uint32_t memorysize=sizeof(TYPE)*count; 
	memmove(dest,source,memorysize);
}