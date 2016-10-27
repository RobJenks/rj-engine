#pragma once

#ifndef __DebugInvocationH__
#define __DebugInvocationH__

class iObject;
class Actor;
class CapitalShipPerimeterBeacon;
class ComplexShip;
class ComplexShipSection;
class LightSource;
class SpaceProjectile;
class Ship;
class SimpleShip;
class SpaceEmitter;


struct DebugInvocation
{
	// Below are typedef examples for the *fn function pointers used in each function signature
	// These have been included directly in the signature since it is not possible to typedef
	// the requried FnPtr type between the type specification in its template parameters and the 
	// function argument string itself
	//
	// typedef RetVal(CInv::*FnPtr0)(void) const;
	// typedef RetVal(CInv::*FnPtr1)(Arg1) const;
	// typedef RetVal(CInv::*FnPtr2)(Arg1, Arg2) const;
	// typedef RetVal(CInv::*FnPtr3)(Arg1, Arg2, Arg3) const;
	// typedef RetVal(CInv::*FnPtr4)(Arg1, Arg2, Arg3, Arg4) const;
	// typedef RetVal(CInv::*FnPtr5)(Arg1, Arg2, Arg3, Arg4, Arg5) const;
	//
	// Example invoking a method "float Actor::SomeFunction(int)" on object "o" with type "const iObject *"
	//				InvokeConst::Invoke<iObject, Actor, float, int>(o, &Actor::SomeFunction, some_int_value)


public:

	/*
	 *	Functions to invoke a const function for class of arbitrary type CInv, on an object of type CObj
	*/

	template <typename CObj, typename CInv, typename RetVal>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (void) const)
	{
		return (((CInv*)obj)->*fn)();
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (Arg1) const, Arg1 _1)
	{
		return (((CInv*)obj)->*fn)(_1);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (Arg1, Arg2) const, Arg1 _1, Arg2 _2)
	{
		return (((CInv*)obj)->*fn)(_1, _2);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (Arg1) const, Arg1 _1, Arg2 _2, Arg3 _3)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (Arg1) const, Arg1 _1, Arg2 _2, Arg3 _3, Arg4 _4)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3, _4);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	static RetVal InvokeConst(const CObj *obj, RetVal(CInv::*fn) (Arg1) const, Arg1 _1, Arg2 _2, Arg3 _3, Arg4 _4, Arg5 _5)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3, _4, _5);
	}



	/*
	 *	Functions to invoke a non-const function for class of arbitrary type CInv, on an object of type CObj
	*/

	template <typename CObj, typename CInv, typename RetVal>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (void))
	{
		return (((CInv*)obj)->*fn)();
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (Arg1), Arg1 _1)
	{
		return (((CInv*)obj)->*fn)(_1);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (Arg1, Arg2), Arg1 _1, Arg2 _2)
	{
		return (((CInv*)obj)->*fn)(_1, _2);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (Arg1), Arg1 _1, Arg2 _2, Arg3 _3)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (Arg1), Arg1 _1, Arg2 _2, Arg3 _3, Arg4 _4)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3, _4);
	}

	template <typename CObj, typename CInv, typename RetVal, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	static RetVal Invoke(CObj *obj, RetVal(CInv::*fn) (Arg1), Arg1 _1, Arg2 _2, Arg3 _3, Arg4 _4, Arg5 _5)
	{
		return (((CInv*)obj)->*fn)(_1, _2, _3, _4, _5);
	}


	/*
	*	Functions to invoke a const function on a base iObject*, with destination invoked type determined based on GetObjectType()
	*/

	// Preprocessor macros to expand out the object type-derivation process
	// @Dependency iObject::ObjectType
#	define INVOKE_SUBCLASS_METHOD(object, method, return_type) \
		switch (object->GetObjectType()) \
		{ \
			case iObject::ObjectType::ActorObject:								return ((Actor*)object)->##method; \
			case iObject::ObjectType::CapitalShipPerimeterBeaconObject:			return ((CapitalShipPerimeterBeacon*)object)->##method; \
			case iObject::ObjectType::ComplexShipObject:						return ((ComplexShip*)object)->##method; \
			case iObject::ObjectType::ComplexShipSectionObject:					return ((ComplexShipSection*)object)->##method; \
			case iObject::ObjectType::LightSourceObject:						return ((LightSource*)object)->##method; \
			case iObject::ObjectType::ProjectileObject:							return ((SpaceProjectile*)object)->##method; \
			case iObject::ObjectType::ShipObject:								return ((Ship*)object)->##method; \
			case iObject::ObjectType::SimpleShipObject:							return ((SimpleShip*)object)->##method; \
			case iObject::ObjectType::SpaceEmitterObject:						return ((SpaceEmitter*)object)->##method; \
			default:															return DefaultValues<return_type>::NullValue(); \
		} 


	// Subclass-invocable functions.  Uses GetObjectType() to call the correct method.  Used to avoid 
	// excessive bloating of object vtable for debug-only functions
	class SubclassInvocations
	{
	public:

		static std::string Invoke_DebugString(const iObject *obj);

	};


};






#endif




