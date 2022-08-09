#include "pch.h"
#include "IMapGeneratorHost.h"
#include "SRandomMapParams.h"
#include "CPerlinMapGenerator.h"
#include "CBasicMapDecorator.h"
#include <map>
#include <memory>

constexpr int MAPGENERATOR_ORIGINAL_VERSION = 5;

// given from interface
IMapGeneratorHost* g_pMapGeneratorHost;

std::unique_ptr<CPerlinMapGenerator> g_pPerlinMapGenerator(nullptr);
std::shared_ptr<CBasicMapDecorator> g_pBasicMapDecorator(nullptr);
int __declspec(dllexport) __stdcall GetRandomMapGeneratorInterfaceVersion(void) {
	return MAPGENERATOR_ORIGINAL_VERSION;
}
void __declspec(dllexport) __stdcall InitRandomMap(class IMapGeneratorHost* host, struct SRandomMapParams* params){
	g_pMapGeneratorHost = host;

	g_pBasicMapDecorator.reset(new CBasicMapDecorator());
	g_pPerlinMapGenerator.reset(new CPerlinMapGenerator(g_pBasicMapDecorator, host, *params));
	//0x200 is unused, host function calls funtion with mapsize instead
	host->unknown0x4(0x200);
}
bool __declspec(dllexport) __stdcall GenerateRandomMap(class IMapGeneratorHost* host){
	//SGround* pGroundMap = host->getGroundMapPtr();
	//uint32_t mapSize = host->getMapSize();
	//auto groundAt = 
	//	[&pGroundMap](uint32_t x, uint32_t y, uint32_t mapsize) {
	//	return &pGroundMap[x + y * mapsize];
	//};
	//auto foreachEdgePoint = 
	//	[&pGroundMap,&groundAt,&mapSize](uint32_t xFrom, uint32_t yFrom, uint32_t xTo, uint32_t yTo, const EGroundType& type,uint8_t elevation) {
	//	// ^
	//	for (uint32_t x = xFrom; x <= xTo; x++)
	//	{
	//		SGround* pGround = groundAt(x, yFrom, mapSize);
	//		pGround->m_groundType = type;
	//		pGround->m_elevation = elevation;
	//	}
	//	// v
	//	for (uint32_t x = xFrom; x <= xTo; x++)
	//	{
	//		SGround* pGround = groundAt(x, yTo, mapSize);
	//		pGround->m_groundType = type;
	//		pGround->m_elevation = elevation;
	//	}
	//	// <-
	//	for (uint32_t y = yFrom; y <= yTo; y++)
	//	{
	//		SGround* pGround = groundAt(xFrom, y, mapSize);
	//		pGround->m_groundType = type;
	//		pGround->m_elevation = elevation;
	//	}
	//	// ->
	//	for (uint32_t y = yFrom; y <= yTo; y++)
	//	{
	//		SGround* pGround = groundAt(xTo, y, mapSize);
	//		pGround->m_groundType = type;
	//		pGround->m_elevation = elevation;
	//	}
	//};
	//auto types = {
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::BEACH, 1),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER_BEACH,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER1,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER2,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER3,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER4,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER5,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER6,0),
	//	std::make_pair<EGroundType,uint8_t>(EGroundType::WATER7,0)
	//};
	//uint32_t posOff = 0;
	//for (auto& pair : types) 
	//{
	//	foreachEdgePoint(49 - posOff, 49 - posOff, mapSize - 50 + posOff, mapSize - 50 + posOff, pair.first, pair.second);
	//	posOff++;
	//}  
//	for (uint32_t x = 50; x < host->getMapSize() - 50; x++) 
//	{
//		for (uint32_t y = 50; y < host->getMapSize()-50; y++)
//		{
//#undef min
//			uint32_t distanceToWater = std::min(std::min(x - 50, mapSize-50 - x), std::min(y - 50, mapSize-50 - y));
//			pGroundMap[x + y * host->getMapSize()].m_groundType = EGroundType::GRASS;
//			if (distanceToWater < 5)
//				pGroundMap[x + y * host->getMapSize()].m_elevation = 1;
//			else
//				pGroundMap[x + y * host->getMapSize()].m_elevation = std::min((distanceToWater-4)*2, 0xE1U);
//		}
//	}
	g_pPerlinMapGenerator->generateGround();
	host->setStartPos(1, host->getMapSize() / 4, host->getMapSize() / 4);

	// Compile map, so we can place objects
	host->compileMap();

	g_pPerlinMapGenerator->generateObjects();
	g_pPerlinMapGenerator->generateResources();
	for (uint32_t x = host->getMapSize() / 2; x < host->getMapSize() -50; x++)
	{
		host->placeObject(x, host->getMapSize() / 2, x%2?EObjectType::TropischeDattelpalme1:EObjectType::TropischeDattelpalme2);
	}


	return true;
}
void  __declspec(dllexport) __stdcall CreatePreview(class IMapGeneratorHost* host, unsigned short* us1, int i1){
	return; // UNUSED IN EDITOR, MAYBE ADD THIS BACK LATER? HISTORY EDITION GAME HAS MAPGENERATOR LIBRARY STATICALLY BOUND
}