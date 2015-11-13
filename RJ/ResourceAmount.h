#pragma once

#ifndef __ResourceAmountH__
#define __ResourceAmountH__

#include <stddef.h>

class Resource;

// This class has no special alignment requirements
class ResourceAmount
{
public:
	const Resource *									Type;
	float												Amount;

	ResourceAmount(void)								{ Type = NULL; Amount = 0.0f; }
	ResourceAmount(const Resource *res, float amount)	{ Type = res; Amount = amount; }

	~ResourceAmount(void) { }
};



#endif