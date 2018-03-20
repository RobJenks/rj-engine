#pragma once


// Type for all post-process commands (Assimp and custom)
typedef unsigned long PostProcess;


// Assimp post-processing constants end with a final value of "aiProcess_Debone = 0x4000000".  Custom constants
// will therefore begin from 0x8000000

enum class CustomPostProcess
{
	InvertU = 0x8000000,		// Replace U coords of UV mapping with (1.0f - U) 
	InvertV = 0x10000000		// Replace V coords of UV mapping with (1.0f - V)



};
