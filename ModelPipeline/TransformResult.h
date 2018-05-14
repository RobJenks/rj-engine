#pragma once

struct TransformResult
{
public:

	unsigned int Success;
	unsigned int Failure;

	TransformResult() : Success(0U), Failure(0U) { }
	TransformResult(unsigned int success, unsigned int failure) : Success(success), Failure(failure) { }

	inline void Add(bool success) { *this += TransformResult::Single(success); }

	inline unsigned int Total(void) const { return (Success + Failure); }

	static inline TransformResult Single(bool success) { return (success ? TransformResult(1U, 0U) : TransformResult(0U, 1U)); }
	static inline TransformResult SingleSuccess(void) { return Single(true); }
	static inline TransformResult SingleFailure(void) { return Single(false); }

	inline TransformResult operator+=(const TransformResult & other) 
	{
		Success += other.Success;
		Failure += other.Failure;
		return *this;
	}

};

inline TransformResult operator+(TransformResult lhs, const TransformResult & rhs)
{
	lhs += rhs; 
	return lhs;
}