#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include <tchar.h>

using namespace std;
using namespace std::tr1;

#include "CompilerSettings.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipTile.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipObjectClass.h"
#include "Model.h"
#include "Equipment.h"
#include "Engine.h"
#include "Resource.h"
#include "SkinnedModel.h"
#include "ActorBase.h"
#include "SpaceProjectileDefinition.h"
#include "StaticTerrainDefinition.h"
class ImmediateRegion;
class SystemRegion;
class UserInterface;

#include "GameDataExtern.h"

namespace D {

	// Primary data collections
	SimpleShipRegister				SimpleShips;				// The details of all simple ship classes in the game	
	SSLoadoutRegister				SSLoadouts;					// The details of all simple ship loadouts in the game
	EquipRegister					Equipment;					// The details of all equipment items in the game

	ComplexShipRegister				ComplexShips;				// The details of all complex ships in the game
	ComplexShipSectionRegister		ComplexShipSections;		// The details of all complex ship sections in the game
	ComplexShipTileRegister			ComplexShipTiles;			// The details of all complex ship tiles in the game
	ComplexShipTileClassRegister	ComplexShipTileClasses;		// The details of all complex ship tile classes in the game
	ComplexShipObjectClassRegister	ComplexShipObjectClasses;	// Details of all object classes in the game
	StaticTerrainRegister			StaticTerrainDefinitions;	// Details of all static terrain classes in the game
	ResourceRegister				Resources;					// Details of all resources in the game
	ProjectileRegister				Projectiles;				// Details of all projectile types in the game
	SkinnedModelRegister			SkinnedModels;				// Details of all skinned models in the game
	ActorRegister					Actors;						// Details of all actor types in the game


	// The user interface
	UserInterface *					UI;

	// Primary game regions
	namespace Regions
	{
		ImmediateRegion *			Immediate;
		SystemRegion *				System;
	}

	// Game data location
	/* Std PC    const char *DATA = "C:\\Users\\Rob\\Documents\\Visual Studio 2008\\Projects\\RJ\\RJ\\Data"; */
	/* AW PC  const char *DATA = "C:\\Documents and Settings\\Rob\\My Documents\\RJ\\Data\\"; */
	/* Work PC */ const char *DATA = "C:\\Users\\rjenks\\Documents\\Visual Studio 2013\\Projects\\RJ\\RJ\\Data";

	// Data read/edit/load constants
	const char *NODE_GameData = "gamedata";
	const char *NODE_FileIndex = "include";
	const char *NODE_SimpleShip = "simpleship";
	const char *NODE_SimpleShipLoadout = "simpleshiploadout";
	const char *NODE_Engine = "engine";
	const char *NODE_System = "system";
	const char *NODE_FireEffect = "fireeffect";
	const char *NODE_ParticleEmitter = "particleemitter";
	const char *NODE_UILayout = "uilayout";
	const char *NODE_Image2DGroup = "image2dgroup";
	const char *NODE_ComplexShip = "complexship";
	const char *NODE_ComplexShipSection = "complexshipsection";
	const char *NODE_ComplexShipSectionInstance = "complexshipsectioninstance";
	const char *NODE_ComplexShipTileClass = "complexshiptileclass";
	const char *NODE_ComplexShipTileDefinition = "complexshiptiledefinition";
	const char *NODE_ComplexShipTile = "complexshiptile";
	const char *NODE_ComplexShipTileBaseData = "basetiledata";
	const char *NODE_ComplexShipElement = "complexshipelement";	
	const char *NODE_Hardpoint = "hardpoint";
	const char *NODE_Model = "model";
	const char *NODE_UIManagedControlDefinition = "uimanagedcontroldefinition";
	const char *NODE_Resource = "resource";
	const char *NODE_SpaceProjectileDefinition = "spaceprojectiledefinition";
	const char *NODE_SkinnedModel = "skinnedmodel";
	const char *NODE_ActorAttributeGeneration = "actorattributegeneration";
	const char *NODE_ActorBase = "actorbase";
	const char *NODE_StaticTerrain = "staticterrain";
	const char *NODE_StaticTerrainDefinition = "staticterraindefinition";
	const char *NODE_Faction = "faction";

	// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
	const char *FILE_ComplexShipRegister = "Ships\\ComplexShipRegister.xml";

	// Terminates all data registers in sequence.  Call at shutdown of the application
	void TerminateAllDataRegisters(void)
	{
		// Terminate any class-specific data registers first
		
		// Terminate each central (D::*) data register in turn
		TerminateAllSimpleShipRegisterData();
		TerminateAllComplexShipRegisterData();
		TerminateAllComplexShipSectionRegisterData();
		TerminateAllComplexShipTileRegisterData();
		TerminateAllComplexShipTileClassRegisterData();
		TerminateAllComplexShipObjectClassRegisterData();

		TerminateAllSSLoadoutRegisterData();
		TerminateAllEquipmentRegisterData();
		TerminateAllSkinnedModelRegisterData();
		TerminateAllActorRegisterData();
		TerminateAllResourceRegisterData();
		TerminateAllProjectileRegisterData();
		TerminateAllStaticTerrainRegisterData();
	}

	// Termination function: ship register.  Clears all ship details data.
	void TerminateAllSimpleShipRegisterData(void)
	{
		SimpleShipRegister::const_iterator it_end = SimpleShips.end();
		for (SimpleShipRegister::const_iterator it = SimpleShips.begin(); it != it_end; ++it) 
		{
			if (it->second) 
			{
				// Check whether this ship object points to a standard set of model data.  If it does,
				// repoint away from the model so that it is not impacted by any default destructor data.
				if ((it->second)->GetModel())
					if (((it->second)->GetModel())->IsStandardModel())
						(it->second)->SetModel(NULL);

				// Shut the object down, delete it and deallocate its memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		SimpleShips.clear();
	}

	// Termination function: SS loadout register.  Clears all SS loadout data.
	void TerminateAllSSLoadoutRegisterData(void)
	{
		SSLoadoutRegister::const_iterator it_end = SSLoadouts.end();
		for (SSLoadoutRegister::const_iterator it = SSLoadouts.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		SSLoadouts.clear();
	}

	// Termination function: Equipment register.  Clears all equipment data.
	void TerminateAllEquipmentRegisterData(void)
	{
		EquipRegister::const_iterator it_end = Equipment.end();
		for (EquipRegister::const_iterator it = Equipment.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		Equipment.clear();
	}

	// Termination function: Complex ship register.  Clears all data.
	void TerminateAllComplexShipRegisterData(void)
	{
		ComplexShipRegister::iterator it_end = ComplexShips.end();
		for (ComplexShipRegister::iterator it = ComplexShips.begin(); it != it_end; ++it) {
			if (it->second) {
				// Call the shutdown method; do NOT traverse the associated section details since these will be deleted
				// directly from the ship section register.  However we can remove the sections themselves from these ship templates
				it->second->Shutdown(true, false, false, false);

				// Delete the object and deallocate memory
				delete (it->second);
				it->second = NULL;
			}
		}

		// Empty the register now it is full of null/invalid pointers
		ComplexShips.clear();
	}

	// Termination function: Complex ship section register.  Clears all data.
	void TerminateAllComplexShipSectionRegisterData(void)
	{
		ComplexShipSectionRegister::const_iterator it_end = ComplexShipSections.end();
		for (ComplexShipSectionRegister::const_iterator it = ComplexShipSections.begin(); it != it_end; ++it) {
			if (it->second) {
				// Call the section shutdown method
				it->second->Shutdown();

				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		ComplexShipSections.clear();
	}

	// Termination function: Complex ship tile register.  Clears all data.
	void TerminateAllComplexShipTileRegisterData(void)
	{
		ComplexShipTileRegister::const_iterator it_end = ComplexShipTiles.end();
		for (ComplexShipTileRegister::const_iterator it = ComplexShipTiles.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		ComplexShipTiles.clear();
	}
	
	// Termination function: Complex ship tile class register.  Clears all data.
	void TerminateAllComplexShipTileClassRegisterData(void)
	{
		ComplexShipTileClassRegister::const_iterator it_end = ComplexShipTileClasses.end();
		for (ComplexShipTileClassRegister::const_iterator it = ComplexShipTileClasses.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		ComplexShipTileClasses.clear();
	}

	// Termination function: Complex ship object class register.  Clears all data.
	void TerminateAllComplexShipObjectClassRegisterData(void)
	{
		ComplexShipObjectClassRegister::const_iterator it_end = ComplexShipObjectClasses.end();
		for (ComplexShipObjectClassRegister::const_iterator it = ComplexShipObjectClasses.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		ComplexShipObjectClasses.clear();
	}

	// Termination function: Resource register.  Clears all data.
	void TerminateAllResourceRegisterData(void)
	{
		ResourceRegister::const_iterator it_end = Resources.end();
		for (ResourceRegister::const_iterator it = Resources.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		Resources.clear();
	}

	// Termination function: Projectile register.  Clears all data.
	void TerminateAllProjectileRegisterData(void)
	{
		ProjectileRegister::const_iterator it_end = Projectiles.end();
		for (ProjectileRegister::const_iterator it = Projectiles.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		Projectiles.clear();
	}

	// Termination function: Skinned model data register.  Clears all data.
	void TerminateAllSkinnedModelRegisterData(void)
	{
		SkinnedModelRegister::const_iterator it_end = SkinnedModels.end();
		for (SkinnedModelRegister::const_iterator it = SkinnedModels.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		SkinnedModels.clear();
	}
	
	// Termination function: Actor data register.  Clears all data.
	void TerminateAllActorRegisterData(void)
	{
		ActorRegister::const_iterator it_end = Actors.end();
		for (ActorRegister::const_iterator it = Actors.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		Actors.clear();
	}

	// Termination function: Static terrain data register.  Clears all data.
	void TerminateAllStaticTerrainRegisterData(void)
	{
		StaticTerrainRegister::const_iterator it_end = StaticTerrainDefinitions.end();
		for (StaticTerrainRegister::const_iterator it = StaticTerrainDefinitions.begin(); it != it_end; ++it) {
			if (it->second) {
				// Delete the object and deallocate memory
				delete (it->second);
			}
		}

		// Empty the register now it is full of null/invalid pointers
		StaticTerrainDefinitions.clear();
	}



	/* Managed methods to add items to each primary data collection.  Unregister any iObjects since we do not want them to be simulated in-game */

	void AddStandardSimpleShip(SimpleShip *s)
	{
		if (s && s->GetCode() != NullString && D::SimpleShips.count(s->GetCode()) == 0)
		{
			SimpleShips[s->GetCode()] = s;				// Add the object
			iObject::UnregisterObject(s);				// Unregister as a game object, since we don't want it to be simulated
		}
	}
	void AddStandardSSLoadout(SimpleShipLoadout *l)
	{
		if (l && l->Code != NullString && D::SSLoadouts[l->Code] == 0)
		{
			SSLoadouts[l->Code] = l;					// Add the object
		}
	}
	void AddStandardComplexShip(ComplexShip *s)
	{
		if (s && s->GetCode() != NullString && D::ComplexShips.count(s->GetCode()) == 0)
		{
			ComplexShips[s->GetCode()] = s;				// Add the object
			iObject::UnregisterObject(s);				// Unregister as a game object, since we don't want it to be simulated
		}
	}
	void AddStandardComplexShipSection(ComplexShipSection *s)
	{
		if (s && s->GetCode() != NullString && D::ComplexShipSections.count(s->GetCode()) == 0)
		{
			ComplexShipSections[s->GetCode()] = s;		// Add the object
			iObject::UnregisterObject(s);				// Unregister as a game object, since we don't want it to be simulated
		}
	}
	void AddStandardComplexShipTileDefinition(ComplexShipTileDefinition *t)
	{
		if (t && t->GetCode() != NullString && D::ComplexShipTiles.count(t->GetCode()) == 0)
		{
			ComplexShipTiles[t->GetCode()] = t;			// Add the object
		}
	}
	void AddStandardComplexShipTileClass(ComplexShipTileClass *c)
	{
		if (c && c->GetCode() != NullString && D::ComplexShipTileClasses.count(c->GetCode()) == 0)
		{
			ComplexShipTileClasses[c->GetCode()] = c;	// Add the object
		}
	}
	void AddStandardComplexShipObjectClass(ComplexShipObjectClass *c)
	{
		if (c && c->GetCode() != NullString && D::ComplexShipObjectClasses.count(c->GetCode()) == 0)
		{
			ComplexShipObjectClasses[c->GetCode()] = c;	// Add the object
		}
	}
	void AddStandardResource(Resource *r)
	{
		if (r && r->GetCode() != NullString && D::Resources.count(r->GetCode()) == 0)
		{
			Resources[r->GetCode()] = r;				// Add the object
		}
	}
	void AddStandardResource(SpaceProjectileDefinition *p)
	{
		if (p && p->GetCode() != NullString && D::Projectiles.count(p->GetCode()) == 0)
		{
			Projectiles[p->GetCode()] = p;				// Add the object
		}
	}
	void AddStandardSkinnedModel(SkinnedModel *m)
	{
		if (m && m->GetCode() != NullString && D::SkinnedModels.count(m->GetCode()) == 0)
		{
			SkinnedModels[m->GetCode()] = m;			// Add the object
		}
	}
	void AddStandardActor(ActorBase *a)
	{
		if (a && a->GetCode() != NullString && D::Actors.count(a->GetCode()) == 0)
		{
			Actors[a->GetCode()] = a;					// Add the object
		}
	}
	void AddStandardEquipment(class Equipment *e)
	{
		if (e && e->Code != NullString && D::Equipment.count(e->Code) == 0)
		{
			D::Equipment[e->Code] = e;					// Add the object
		}
	}
	void AddStaticTerrain(StaticTerrainDefinition *d)
	{
		if (d && d->GetCode() != NullString && D::StaticTerrainDefinitions.count(d->GetCode()) == 0)
		{
			D::StaticTerrainDefinitions[d->GetCode()] = d;					// Add the object
		}
	}













}


