#pragma once

#ifndef __GameUniverseH__
#define __GameUniverseH__

#include <string>
#include "DX11_Core.h"

#include <unordered_map>

class SpaceSystem;
using namespace std;
using namespace std::tr1;


class GameUniverse
{
public:
	GameUniverse(void);
	~GameUniverse(void);

	Result					InitialiseUniverse(void);
	Result					ProcessLoadedSystems(ID3D11Device *device);

	void					AddSystem(SpaceSystem *system);
	SpaceSystem				*GetSystem(string code);

	void					TerminateUniverse(void);


	typedef std::tr1::unordered_map<string, SpaceSystem*> SystemRegister;
	SystemRegister			Systems;									// The collection of all space systems in the game
};


#endif