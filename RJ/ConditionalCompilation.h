#pragma once

#ifndef __ConditionalCompilationH__
#define __ConditionalCompilationH__

#include <iostream>
#include <string>
#include <sstream>


// Defines a conditional-compilation member requirement, which can then be applied to
// methods within the same compilation unit.  Relevant to class objects.  Only supports
// basic function signatures: those without any parameter lists.
#define CC_DEFINE_OBJ_MEMBER_REQUIREMENT(requirement_name, member)  \
template <typename T> \
struct requirement_name { \
private: \
	struct yes { int x; }; \
	struct no { yes x[4]; }; \
	template <typename U> \
	static typename std::enable_if<sizeof(static_cast<U*>(0)->##member##(), void(), int()) == sizeof(int), yes>::type test(int); \
	template <typename> \
	static no test(...); \
public: \
	enum { value = sizeof(test<T>(0)) == sizeof(yes) }; \
};

// Defines a conditional-compilation member requirement, which can then be applied to
// methods within the same compilation unit.  Relevant to pointer objects.  Only supports
// basic function signatures: those without any parameter lists.
#define CC_DEFINE_PTR_MEMBER_REQUIREMENT(requirement_name, member)  \
template <typename T> \
struct requirement_name { \
private: \
	struct yes { int x; }; \
	struct no { yes x[4]; }; \
	template <typename U> \
	static typename std::enable_if<sizeof(static_cast<U>(0)->##member##(), void(), int()) == sizeof(int), yes>::type test(int); \
	template <typename> \
	static no test(...); \
public: \
	enum { value = sizeof(test<T>(0)) == sizeof(yes) }; \
};


// Conditionally-compiles a method only if T_ exposes the relevant member
#define CC_REQUIRES_MEMBER(requirement_name, T_, return_value) \
	typename std::enable_if<requirement_name<T_>::value, return_value>::type

#if 0

// Defines a conditional-compilation member requirement, which can then be applied to
// methods within the same compilation unit.  Relevant to class objects.
#define CC_DEFINE_OBJ_MEMBER_REQUIREMENT(requirement_name, T, member)  \
	template <typename T> \
	struct requirement_name { \
	private: \
		template <typename U> \
			static decltype(std::declval<U>().##member, void(), std::true_type())	test(int); \
		template <typename> \
			static std::false_type													test(...); \
	public: \
		typedef decltype(test<T>(0)) type; \
		enum { value = type::value }; \
	}; 

// Defines a conditional-compilation member requirement, which can then be applied to
// methods within the same compilation unit.  Relevant to pointer types.
#define CC_DEFINE_PTR_MEMBER_REQUIREMENT(requirement_name, T, member)  \
	template <typename T> \
	struct requirement_name { \
	private: \
		template <typename U> \
			static decltype(std::declval<U>()->##member, void(), std::true_type())	test(int); \
		template <typename> \
			static std::false_type													test(...); \
	public: \
		typedef decltype(test<T>(0)) type; \
		enum { value = type::value }; \
	}; 

// Conditional compilation of a method only when the given requirement is met
#define CC_REQUIRES_MEMBER(requirement_name) \
	template<typename requirement_name##U = T, typename requirement_name##B = std::enable_if<requirement_name<T>::value, int>::type>

// Conditional compulation of a method only when the given set of requirements (combined
// using CC_REQ_AND) is met
#define CC_REQUIRES_MEMBERS(requirements) \
	template<##requirements##>

// Single conditional compilation requirement, for use in multi-requirement expressions
#define CC_REQ(requirement_name) \
	typename requirement_name##U = T, typename requirement_name##B = std::enable_if<requirement_name<T>::value, int>::type

// Combines two conditional compilation requiremnets together
#define CC_REQ_AND(req1, req2) \
	req1, req2






#endif


#endif