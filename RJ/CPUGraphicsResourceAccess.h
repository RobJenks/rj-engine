#pragma once

enum CPUGraphicsResourceAccess
{
	None		= 0,				// CPU has no access to the resource at all
	Read		= (1 << 0),			// CPU can read from the resource only
	Write		= (1 << 1),			// CPU can write to the resource only
	ReadWrite	= (Read | Write)	// CPU has both read & write access to the resource
};