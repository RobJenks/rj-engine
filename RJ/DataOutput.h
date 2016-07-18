#ifndef __DataOutputH__
#define __DataOutputH__

#include "DX11_Core.h"
#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "Utility.h"

class iStaticObject;
class iActiveObject;
class iEnvironmentObject;
class SimpleShip;
class iSpaceObjectEnvironment;
class ComplexShip;
class ComplexShipSection;
class ComplexShipElement;
class TileConnections;
class Hardpoint;
class OrientedBoundingBox;
class BoundingObject;
class Engine;
class TileConnections;

// This file contains no objects with special alignment requirements
namespace IO { namespace Data {

	Result SaveXMLDocument(TiXmlElement *root, const string &filename);

	// Common methods to save intermediate class data
	bool SaveObjectData(TiXmlElement *node, iObject *object);									// Saves iObject class data
	bool SaveActiveObjectData(TiXmlElement *node, iActiveObject *object);						// Saves iActiveObject class data
	bool SaveStaticObjectData(TiXmlElement *node, iStaticObject *object);						// Saves iStaticObject class data
	bool SaveSpaceObjectData(TiXmlElement *node, iSpaceObject *object);							// Saves iSpaceObject class data
	bool SaveEnvironmentObjectData(TiXmlElement *node, iEnvironmentObject *object);				// Saves iEnvironmentObject class data
	bool SaveSpaceObjectEnvironmentData(TiXmlElement *node, iSpaceObjectEnvironment *object);	// Saves iSpaceObjectEnvironment data
	bool SaveShipData(TiXmlElement *node, Ship *object);										// Saves base ship class data

	// 'Final' methods, to save objects at the end of the inheriance hierarchy that are instantiated directly
	Result SaveSimpleShip(TiXmlElement *parent, SimpleShip *object);
	Result SaveComplexShip(TiXmlElement *parent, ComplexShip *object);
	Result SaveComplexShipSection(TiXmlElement *parent, ComplexShipSection *object);
	Result SaveComplexShipSectionInstance(TiXmlElement *parent, ComplexShipSection *object);	// Instance of an already-defined section
	Result SaveComplexShipElement(TiXmlElement *parent, const ComplexShipElement &object);
	Result SaveHardpoint(TiXmlElement *parent, Hardpoint *object);
	
	Result SaveCollisionOBB(TiXmlElement *parent, OrientedBoundingBox *obb);
	Result SaveBoundingObject(TiXmlElement *parent, BoundingObject *bound, int id);

	Result SaveTileConnectionState(TiXmlElement *parent, const std::string & element_name, TileConnections *connection_data);

	Result SaveStaticTerrain(TiXmlElement *parent, StaticTerrain *terrain);

	Result SaveEngine(TiXmlElement *parent, Engine *e);

	Result GenerateComplexShipRegisterXMLFile(void);




}}



#endif