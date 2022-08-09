#pragma once
#include <cstdint>
#include "SGround.h"
#include "SGroundData.h"
#include "EObjectType.h"
class IMapGeneratorHost {
public:
	/*
		S4Editor probably does not need logic for this. Only returns 2
	*/
	virtual uint32_t unknown0x0(int32_t unused) = 0;
	/*
		S4Editor works without this function. Probably sets up some data in S4_Main or some re-rendering thingies
	*/
	virtual void unknown0x4(int32_t unused) = 0;
	/*
		S4Editor map will mess up big time if this function is not called at the end of the map generation
		Basically this will set all the things you have done at this point to the map. Before this step, you cannot place
		objects. As placeObject() is basically the equivalent of placing an object in the editor (you can even Ctrl+U in the editor), the object
		to be placed has to have the corresponding grounds required to place it compiled.
	*/
	virtual void compileMap() = 0;
	/*
		Gets Map Size to be generated
	*/
	virtual uint32_t getMapSize() = 0;
	/*
		Get Map array of Ground data (ground ID, height...)
	*/
	virtual SGround* getGroundMapPtr() = 0;
	/*
		Get Map array of Resoruce data(???,resource type, resource count (0-0xF I believe))
	*/
	virtual SGroundData* getGroundDataMapPtr() = 0;
	/*
		Places an object at a certain position.
	*/
	virtual void placeObject(uint32_t x, uint32_t y, EObjectType objectID) = 0;
	/*
		Sets the starting pos of a party, should be done while initiation phase (for preview)
	*/
	virtual void setStartPos(uint32_t party, uint32_t x, uint32_t y) = 0;
	/*
		In S4Editor, only returns 1, probably bool
	*/
	virtual uint8_t unknown0x20() = 0;


};