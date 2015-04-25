#pragma once

#ifndef __RJDEBUG__
#define __RJDEBUG__

#include "DX11_Core.h"

#include "DataOutput.h"
#include "SimpleShipDetails.h"
#include "Model.h"
#include "Engine.h"

//void __WriteDebugShipData()
//{
//	// Create the debug ship
//	SimpleShipDetails *s = new SimpleShipDetails();
//	s->Name = "Testing Ship";
//	s->Model = new Model();
//	s->Model->SetFilename("\\Data\\Ships\\bigship1.x");
//	D3DXVECTOR3 cam(0.5f, 2.5f, -3.0f);
//	s->CameraPosition = cam;
//
//	// Open a document and save it
//	TiXmlElement *data = IO::Data::NewGameDataXMLNode();
//	Result r1 = IO::Data::SaveSimpleShip(data, s);
//	Result r2 = IO::Data::SaveXMLDocument(data, "C:\\Users\\Rob\\Documents\\Visual Studio 2008\\Projects\\RJ\\RJ\\Data\\ships.xml");
//	
//}

//void __WriteDebugEngineData()
//{
//	Engine *e = new Engine();
//	e->Name = "Basic Ion Engine";
//	e->SetMaxHealth(1200.0f);
//	e->MaxThrust = 200.0f;
//	e->Acceleration = 20.0f;
//	
//	TiXmlElement *data = IO::Data::NewGameDataXMLNode();
//	Result r1 = IO::Data::SaveEngine(data, e);
//	Result r2 = IO::Data::SaveXMLDocument(data, "C:\\Users\\Rob\\Documents\\Visual Studio 2008\\Projects\\RJ\\RJ\\Data\\engines.xml");
//}





#endif