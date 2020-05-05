#pragma once

// An empty class. Useful for template classes where the template is the base class and want to specify don't derive
//  from anything
//
// template<typename BASECLASS>
// class tClassT : public BASECLASS
//
// typedef tClassT<tEmptyClass> tClass;
class tEmptyClass
{
};