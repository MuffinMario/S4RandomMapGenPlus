#pragma once
#include "IMapGenerator.h"
#include "IMapGeneratorHost.h"
#include "SRandomMapParams.h"
#include <cstdint>
#include <map>

#include "PerlinNoise.hpp"
#include <functional>
#include <math.h>

class CPerlinMapGenerator :public IMapGenerator {
	/* Map Random Params*/
	const siv::PerlinNoise::seed_type m_seed;
	const siv::PerlinNoise m_perlinNoiseGen;

	/* Map Interface */
	IMapGeneratorHost* m_pMapGeneratorHost;
	
	/* Map Gen Params*/
	const SRandomMapParams m_params;

	/* Map Data */
	uint32_t m_mapSize;
	SGround* m_pGroundMap;
	SGroundData* m_pGroundDataMap;


	/* Constants */
	constexpr static inline uint32_t MAX_ELEVATION = 0xE1;

	constexpr static inline uint32_t MAX_HEIGHT_DIFFERENCE_NEIGHBOR = 4;
	constexpr static inline uint32_t MAPGEN_EDGE_DISTANCE = 50;

	constexpr static inline double  BASENOISE_SCALE = 0.005;
	constexpr static inline double  BIOMENOISE_SCALE = 0.005;
	constexpr static inline double  MOUNTAINNOISE_SCALE = 0.01;
	constexpr static inline double  GRASSNOISE_SCALE = 0.01;
	constexpr static inline double  TREENOISE_SCALE = 0.05;

	constexpr static inline double MIN_BASENOISE_GRASS = -0.2;
	constexpr static inline double MIN_BASENOISE_BEACH = -0.25;
	constexpr static inline double MIN_BASENOISE_MOUNTAIN = MIN_BASENOISE_GRASS;

	constexpr static inline double MIN_MOUNTAINNOISE_MOUNTAIN = 0.3;
	constexpr static inline double MIN_MOUNTAINNOISE_SNOW = 0.7;

	const double MIN_DARKFOREST_NOISE = 0.7;
	const double MIN_FIRFOREST_NOISE = 0.4;
	const double MIN_JUNGLEFOREST_NOISE = 0;
	const double MIN_EUROFOREST1_NOISE = -0.5;
	const double MIN_EUROFOREST2_NOISE = -1.0;

	// the highest spawnrate to say yup this is "biome" grass field
	constexpr static inline double GRASS_FIELD_MAX_SPAWNRATE = 10;

	/*
		Lessens Perlin noise values if they're close to the edge
	*/
	double flattenPerlinEdges(uint32_t x, uint32_t y)const {
#undef min
		auto distanceToEdge = std::min(std::min(x, m_mapSize-x), std::min(y, m_mapSize - y));
		if (distanceToEdge < MAPGEN_EDGE_DISTANCE)
		{
			return -1.3 * (static_cast<double>(MAPGEN_EDGE_DISTANCE) -distanceToEdge) / MAPGEN_EDGE_DISTANCE;
		}
		else {
			return 0.0;
		}
	}

	/*
		If a hierarchial ground is close thats 2 levels+ above, it will promote this node to the one level previous
	*/
	void promoteGround(uint32_t x, uint32_t y, EGroundType promoteNode) {
		auto& hierarch = CGroundHierarchy::getInstance();

		auto thisNodeLevel = hierarch.getGroundLevel(promoteNode);
		groundAt(x, y)->m_groundType = promoteNode;
		groundAt(x, y)->m_elevation = thisNodeLevel <= 9 ? 0 : groundAt(x,y)->m_elevation; // water is elev 0, else 1

		uint8_t lowestLevelAccepted = hierarch.getGroundLevel(promoteNode) - 1;
		auto& currentNode = hierarch.getGroundLevelNode(promoteNode);

		auto fpThisHierarchyCheckGround = std::bind(&CPerlinMapGenerator::hierarchyCheckGround, *this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, currentNode, lowestLevelAccepted);
		foreachNeighborGround(x, y, fpThisHierarchyCheckGround);
	}
	/*
		hierarchy ground check
	*/
	void hierarchyCheckGround(
		const uint32_t neighborX, const uint32_t neighborY,
		SGround* pNeighborGround,
		const CGroundHierarchy::SGroundNode& currentNode, uint8_t lowestLevelAccepted)
	{
		auto& hierarch = CGroundHierarchy::getInstance();
		auto neighborLevel = hierarch.getGroundLevel(pNeighborGround->m_groundType);
		auto grassLevel = hierarch.getGroundLevel(EGroundType::GRASS);
		if (neighborLevel < lowestLevelAccepted)
		{
			promoteGround(neighborX, neighborY, currentNode.m_prev->m_groundType); // level > 1 => prev != nullptr
		}

		/* W8-...W1-BW-B-G are linear
			

			it becomes more complex with different branching lines after G

			  / GW - GD
			G - GM - MG - M
			  \ GI
			  \ GD - GD - D
			  \ ...

			  GD,M,GI,GDi,GP are all branches that can transition without a G between each other

			  so e.g. GD-GW-GM-MG-M    is accepted, but
					  GD-GW-GD-DG-D    is not. You need to have a Grass between them.
				   so GD-GW-G -GD-DG-D is accepted.

			Approach to fix e.g.

			    bracket = position checking their neighbors (all around them, except topleft, bottomright)
			M	M	M	M	M	M	M	M	M
			M	M	M	M	M	M	M	M	M
			M	M	M	(M)	M	M	M	M	M
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD

			M and GD are sister branches so the deal is to demote the end of each branch to its closest to Grass (GM,GW)

			1 update neighbor GD to GW, "demoting" GD
			M	M	M	M	M	M	M	M	M
			M	M	M	M	M	M	M	M	M
			M	M	M	M	M	M	M	M	M
			M	M	M	(GW)GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			
			2 update all unfitting neighbors, "demoting" all here
			M	M	M	M	M	M	M	M	M
			M	M	M	M	M	M	M	M	M
			M	M	M	(GM)(GM)M	M	M	M
			M	M	(GM)GW	GD	GD	GD	GD	GD
			M	M	(GM)GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD


			a pattern emerged, "cascading" all of the blocks. These are all "demoting" since we are dealing with branch-differences.
			If we e.g. have another column of W8 on the left side, they'd be promoted instead, as they follow a linear path (W8-...B-G-GM-MG-M)
			Basically, we need to demote all non-linear paths until they are either "fine" (GW-GM; sister branches) 
			or build a linear path again ([D]-M ->(demote to highest lv intersecting linear (grass)) D-[G] ->(demote to closest?) [GD]-G ) Here we have a problem. 
			Should we instead do (D-M -> (remote to D.prev) D-MD -> repeat and either demote/promote depending on neighbors?)
			'-> Risk of endless recursion? Only demote different branches. Only promote linear?
				[MG] - G - D -D -> 
				MG - [GM] - D - D -> 
				MG - GM - [G] - D ->
				fin?
				if we do nothing at this cascade, this will happen at D :
				MG - GM - G - [D] -> ? 
				MG - GM - [DG] - D ->
				MG - [GD] - DG - D ->
				[G] - GD - DG - D ->
				fin
				These cases are rather rare, but if they happen a few more times than we like, there will be a fair chunk of performance loss
				But for the current implementation, I don't see a better way to do this 100% correct.
				
				Same example with sister branches:
				MG - G - GD -GD -> 
				MG - GM - D - D -> 
				MG - GM - G - D -> ? 
				if we do nothing at this cascade, this will happen at D :
				MG - GM - DG - D ->
				MG - GD - DG - D ->
				G - GD - DG - D 
				These cases are rather rare, but if they happen a few more times than we like, there will be a fair chunk of performance loss
				But for the current implementation, I don't see a better way to do this 100% correct.

			M	M	M	M	M	M	M	M	M
			M	M	M	(MG)(MG)M	M	M	M
			M	M	(MG)GM	GM	(MG)M	M	M
			M	(MG)GM	GW	GD	GD	GD	GD	GD
			M	(MG)GM	GD	GD	GD	GD	GD	GD
			M	M	(MG)GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD
			M	M	M	GD	GD	GD	GD	GD	GD

		*/

		//// MOVE all TO CONST AREA
		//auto& hierarch = CGroundHierarchy::getInstance();
		//const auto GRASS_LEVEL = hierarch.getGroundLevel(EGroundType::GRASS);

		//// TODO: MAKE THIS INTO A TABLE areLine(a,b) => bool INSTEAD OF CALCULATING IT EVERY TIME
		//static const auto areInLine = [&hierarch](const EGroundType& a, const EGroundType& b) {
		//	if (a == b)
		//		return true;

		//	auto nodeA = hierarch.getGroundLevel(a);
		//	auto nodeB = hierarch.getGroundLevel(b);
		//	EGroundType highestLevelType, lowestLevelType;
		//	if (nodeA > nodeB)
		//	{
		//		highestLevelType = a;
		//		lowestLevelType = b;
		//	}
		//	else {
		//		highestLevelType = b;
		//		lowestLevelType = a;
		//	}
		//	auto* highestLevelNode = &hierarch.getGroundLevelNode(highestLevelType);
		//	while (highestLevelNode != nullptr)
		//	{
		//		// yes this also again checks a==b
		//		if (highestLevelNode->m_groundType == lowestLevelType)
		//			return true;
		//		highestLevelNode = highestLevelNode->m_prev;
		//	}
		//	return false;
		//};

		////const uint32_t neighborX, const uint32_t neighborY,
		////	SGround* pNeighborGround,
		////	const CGroundHierarchy::SGroundNode& currentNode, uint8_t lowestLevelAccepted)
		//if (areInLine(currentNode.m_groundType, pNeighborGround->m_groundType)) {
		//	// they are in line, so promote the neighbor if hes lower
		//	auto neighborLevel = hierarch.getGroundLevel(pNeighborGround->m_groundType);
		//	if (neighborLevel < lowestLevelAccepted)
		//	{
		//		promoteGround(neighborX, neighborY, currentNode.m_prev->m_groundType); // w8 impossible in this block => always mprev != nullptr
		//	}
		//	else if (neighborLevel > GRASS_LEVEL)
		//	{
		//		int32_t levelDiffs = static_cast<int32_t>(neighborLevel) - static_cast<int32_t>(lowestLevelAccepted+1);
		//		if (levelDiffs > 1) // no fluid transition, demote neighbor to levelDiffs-1 lower
		//		{
		//			EGroundType demoteType;
		//			auto* demoteIt = &hierarch.getGroundLevelNode(pNeighborGround->m_groundType);
		//			for (int i = 1; i < levelDiffs; i++)
		//			{
		//				demoteIt = demoteIt->m_prev;
		//			}
		//			demoteType = demoteIt->m_groundType;
		//			promoteGround(neighborX, neighborY, demoteType); // w8 impossible in this block => always mprev != nullptr

		//		}
		//	}
		//}
		//else { 
		//	//they are NOT in line. only possible level grass + 1 and higher. re-mote neighbor to current.mprev
		//	std::cout << std::dec << "(checking: " << neighborX << "/" << neighborY << ") " << std::hex << (int)currentNode.m_groundType << " not in line with neighbor " << (int)pNeighborGround->m_groundType << "Setting neighbor to " << (int)currentNode.m_prev->m_groundType << std::endl;
		//	promoteGround(neighborX, neighborY, currentNode.m_prev->m_groundType); // w8 impossible in this block => always mprev != nullptr
		//}
	}
	/*
		Check heights are okay (they aren't)
	*/
	void checkHeight(
		const uint32_t neighborX, const uint32_t neighborY,
		SGround* pNeighborGround,
		const uint8_t highestElevationAccepted) 
	{
		if (pNeighborGround->m_elevation > highestElevationAccepted)
		{
			pNeighborGround->m_elevation = highestElevationAccepted;
			auto fpThisHierarchyCheckHeight = std::bind(&CPerlinMapGenerator::checkHeight, *this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, pNeighborGround->m_elevation + 4);
			foreachNeighborGround(neighborX, neighborY, fpThisHierarchyCheckHeight);
		}
	}
	void foreachNeighborGround(uint32_t x, uint32_t y, std::function<void(const uint32_t, const uint32_t,SGround*)> f) {
		// left neighbor
		if (x > 0)
			f(x - 1, y, groundAt(x - 1, y));

		//right neighbor
		if (x < m_mapSize - 1)
			f(x + 1, y, groundAt(x + 1, y));

		// above, right (just above) neighbor
		if (y > 0)
			f(x, y - 1, groundAt(x, y - 1));

		// above, left  neighbor
		if (y > 0 && x > 0)
			f(x - 1, y - 1, groundAt(x - 1, y - 1));

		// below, left (just below) neighbor
		if (y < m_mapSize - 1)
			f(x, y + 1, groundAt(x, y + 1));

		// below, right 
		if (y < (m_mapSize - 1) && x < (m_mapSize - 1))
			f(x + 1, y + 1, groundAt(x + 1, y + 1));

	}
	void tidyGroundMap() {
		auto& hierarch = CGroundHierarchy::getInstance();
		for (uint32_t y = 0; y < m_mapSize; y++)
		{
			for (uint32_t x = 0; x < m_mapSize; x++) {
				auto pGround = groundAt(x, y);
				uint8_t lowestLevelAccepted = hierarch.getGroundLevel(pGround->m_groundType) - 1;
				auto& currentNode = hierarch.getGroundLevelNode(pGround->m_groundType);
				auto fpThisHierarchyCheckGround = std::bind(&CPerlinMapGenerator::hierarchyCheckGround, *this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, currentNode, lowestLevelAccepted);
				auto fpThisCheckHeight = std::bind(&CPerlinMapGenerator::checkHeight, *this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, pGround->m_elevation + 4);
				foreachNeighborGround(x, y, fpThisHierarchyCheckGround);
				foreachNeighborGround(x, y, fpThisCheckHeight);
			}
		}
	}
	inline SGround* groundAt(uint32_t x, uint32_t y) const {
		return &m_pGroundMap[x + y * m_mapSize];
	};
	inline SGroundData* groundDataAt(uint32_t x, uint32_t y) const {
		return &m_pGroundDataMap[x + y * m_mapSize];
	};

	inline auto getBaseNoiseAt(uint32_t x, uint32_t y)const
	{
		// also todo move this stuff
		const auto scale = BASENOISE_SCALE * (1024.0 / double(m_mapSize));
		const auto offset = (m_mapSize - 256) / 64 * 1024;//pseudo "new" world gen, so it doesnt look like every map with diff size but same seed is a miniatur/bigger version
		
		const auto bignoise = m_perlinNoiseGen.octave2D((x + offset) * scale, (y + offset) * scale, 8, 0.4);
		const auto smallnoise = 0.0;// m_perlinNoiseGen.noise2D(x * scale * 0.2, y * scale * 0.2);
		const auto noise = bignoise + smallnoise + flattenPerlinEdges(x, y);
		return noise;
	}
	inline auto getBiomeNoiseAt(uint32_t x, uint32_t y) const {
		// also todo move this stuff
		const auto scale = BIOMENOISE_SCALE * (1024.0 / double(m_mapSize));
		const auto offset = (m_mapSize - 256) / 64 * 1024;//pseudo "new" world gen, so it doesnt look like every map with diff size but same seed is a miniatur/bigger version
		
		const auto bignoise = m_perlinNoiseGen.octave2D((x + offset) * scale, (y + offset) * scale, 1, 0.4);
		return bignoise;
	}
	inline auto getMountainNoiseAt(uint32_t x, uint32_t y) const
	{
		const auto MOUNTAINNOISE_SIZE_SCALAR = 1.0;
		//scale half as much as base noise 
		const auto bigscale = MOUNTAINNOISE_SCALE * (1024.0 / (double(m_mapSize) * MOUNTAINNOISE_SIZE_SCALAR));
		const auto bignoise = m_perlinNoiseGen.octave2D(x * bigscale, y * bigscale, 3);
		return bignoise;
	}
	inline enum { 
		DARK_FOREST,
		FIR_FOREST,
		JUNGLE,
		EUROFOREST1,
		EUROFOREST2
	} getBiomeAt(uint32_t x, uint32_t y) const {
		auto biomeNoise = getBiomeNoiseAt(x, y);
		if (biomeNoise > MIN_DARKFOREST_NOISE)
			return DARK_FOREST;
		else if (biomeNoise > MIN_FIRFOREST_NOISE)
			return FIR_FOREST;
		else if (biomeNoise > MIN_JUNGLEFOREST_NOISE)
			return JUNGLE;
		else if (biomeNoise > MIN_EUROFOREST1_NOISE)
			return EUROFOREST1;
		else
			return EUROFOREST2;
	}
	/*
		Chance grass will spawn 1 to 21
	*/
	inline uint32_t getGrassSpawnChanceAt(uint32_t x, uint32_t y) const
	{
		const auto smallerscale = GRASSNOISE_SCALE;
		const auto noise = m_perlinNoiseGen.noise2D(x * smallerscale, y * smallerscale);
		return static_cast<uint32_t>(10.0 * (noise + 1.0)) + 1;
	}
	/*
		Chance grass will spawn 1 to 21
	*/
	inline uint32_t getTreeSpawnChanceAt(uint32_t x, uint32_t y) const
	{
		const auto smallerscale = TREENOISE_SCALE;
		const auto biome = getBiomeAt(x, y);
		const auto noise = m_perlinNoiseGen.noise2D((x+biome*1024) * smallerscale, (y+biome*1024) * smallerscale);
		auto between = [](double v, double min, double max) {return v >= min && v <= max; };

		bool bTreeRegion = false;
		if (biome == JUNGLE || biome == FIR_FOREST)
		{
			bTreeRegion = noise > 0.15;
		}
		else {
			bTreeRegion = noise > 0.3 ||
				between(noise, -0.8, -0.7); //small
		}
		return bTreeRegion ? 1 : 10000;
	}


	inline bool isGrass(const SGround* pGround) const {
		const auto type = pGround->m_groundType;
		return type == EGroundType::GRASS ||
			type == EGroundType::GRASS_DARK ||
			type == EGroundType::GRASS_ISLAND;
	}
	inline bool isWater(const SGround* pGround) const {
		const auto type = pGround->m_groundType;
		return type <= EGroundType::WATER8;
	}

	inline bool isNearBeach(uint32_t x, uint32_t y, const SGround* pGround)
	{
		auto baseNoise = getBaseNoiseAt(x, y); //redundant; performance issue
		return baseNoise > MIN_BASENOISE_BEACH &&
			baseNoise < MIN_BASENOISE_GRASS + 0.05 &&
			(pGround->m_groundType == EGroundType::GRASS ||
				pGround->m_groundType == EGroundType::BEACH);
	}

	inline bool shouldSpawnTree(uint32_t rate)
	{
		return rand() % rate == 0;
	}
	inline bool shouldSpawnGrass(uint32_t rate)
	{
		return rand() % rate == 0;
	}
	inline bool shouldSpawnBush(uint32_t rate = 40)
	{
		return rand() % rate == 0;
	}
	inline bool shouldSpawnPalm(uint32_t rate = 15)
	{
		return rand() % rate == 0;
	}

	template<size_t size>
	void placeRandomObject(uint32_t x, uint32_t y,const EObjectType (& objectPool)[size])
	{
		uint32_t index = rand() % sizeof(objectPool);
		m_pMapGeneratorHost->placeObject(x, y, objectPool[index]);
	}

	void placeStoneForBuildingPlace(uint32_t x, uint32_t y,uint32_t radius) {

		circle(x, y, radius, m_mapSize, [this](DWORD cx, DWORD cy) {
			bool bPlaceStone = rand() % 6 == 0;
			if (bPlaceStone)
			{
				SGround* pGround = groundAt(cx, cy);
				if (isGrass(pGround))
				{
					uint32_t quantity = 8 + rand() % 9;
					m_pMapGeneratorHost->placeObject(cx, cy,
						static_cast<EObjectType>(
							static_cast<uint32_t>(EObjectType::SteinezumBauen1) + (quantity - 1)
							)
					);
				}
			}
			});
	}

	template <typename F, typename... P>
	void circle(WORD x, WORD y, WORD radius, WORD mapsize, F func) {
		// INT to prevent arithmetic underflow

		int r = radius;
		int xorig = x, yorig = y;

		int its = 2 * r - 1;
		//    basically 
		//
		//      LLX
		//     LLXRR
		//      XRR
		int left = r - 1, right = 0;
		for (int i = its; i > 0; --i) {
#undef max
			int cx = xorig - left;
			int cy = yorig + r - i;

			// this and the following lines will be out of map
			if (cy >= unsigned(mapsize - 1)) break;

			// don't care aobut the ones left from this.
			if (cx < 1) cx = 1;

			while (cx <= xorig + right && cx < unsigned(mapsize - 1)) {
				func(cx, cy);
				++cx;
			}

			if (right == r - 1) --left;
			else ++right;
		}
	}
public:
	CPerlinMapGenerator(std::shared_ptr<IMapDecorator> pMapDecorator, IMapGeneratorHost* pMapGeneratorHost, const SRandomMapParams& params)
		:
		IMapGenerator(pMapDecorator),
		m_seed(params.m_seed), m_perlinNoiseGen(m_seed), m_pMapGeneratorHost(pMapGeneratorHost), m_params(params),
		m_mapSize(pMapGeneratorHost->getMapSize()), m_pGroundMap(pMapGeneratorHost->getGroundMapPtr()),m_pGroundDataMap(pMapGeneratorHost->getGroundDataMapPtr())
	{
		// TODO: replace with mt rng
		srand(params.m_seed);
	}
	virtual void generateGround() {
		/*
			draws
			---------
			|		|
			|		|  |yfrom-yto|
			---------
			|xfrom-xto|
		*/
		auto foreachEdgePoint =
			[this](uint32_t xFrom, uint32_t yFrom, uint32_t xTo, uint32_t yTo, const EGroundType& type, uint8_t elevation) {
			// ^
			for (uint32_t x = xFrom; x <= xTo; x++)
			{
				SGround* pGround = groundAt(x, yFrom);
				pGround->m_groundType = type;
				pGround->m_elevation = elevation;
			}
			// v
			for (uint32_t x = xFrom; x <= xTo; x++)
			{
				SGround* pGround = groundAt(x, yTo);
				pGround->m_groundType = type;
				pGround->m_elevation = elevation;
			}
			// <-
			for (uint32_t y = yFrom; y <= yTo; y++)
			{
				SGround* pGround = groundAt(xFrom, y);
				pGround->m_groundType = type;
				pGround->m_elevation = elevation;
			}
			// ->
			for (uint32_t y = yFrom; y <= yTo; y++)
			{
				SGround* pGround = groundAt(xTo, y);
				pGround->m_groundType = type;
				pGround->m_elevation = elevation;
			}
		};

		/*
			Generate landscape
		*/

		for (uint32_t y = 20; y < m_mapSize - 20; y++)
		{
			for (uint32_t x = 20; x < m_mapSize - 20; x++)
			{
				/*
					Generate beach/sand
				*/
				auto pGround = groundAt(x, y);

				auto baseNoise = getBaseNoiseAt(x, y);
				if (baseNoise > MIN_BASENOISE_GRASS)
				{
					auto biomeNoise = getBiomeNoiseAt(x, y);
					if (/*biomeNoise <= MIN_DARKFOREST_NOISE &&*/ biomeNoise > MIN_JUNGLEFOREST_NOISE)
					{
						// grass until we fix transition ground on branching hierarchy
						pGround->m_groundType = EGroundType::GRASS; //EGroundType::GRASS_DARK;
					}
					else {
						pGround->m_groundType = EGroundType::GRASS;
					}
					auto hillnoise = m_perlinNoiseGen.noise2D(x, y);
					auto height = std::clamp(static_cast<uint32_t>(((baseNoise + 0.2) * (1.0 + 0.2) / (1.0) * std::abs(hillnoise)) * (double)5 + 1), 1U, 6U);
					pGround->m_elevation = height;
				}
				else if (baseNoise > MIN_BASENOISE_BEACH)
				{
					pGround->m_elevation = 1;
					pGround->m_groundType = EGroundType::BEACH;
				}

				/*
					Generate mountains
				*/
				auto mountainNoise = getMountainNoiseAt(x, y);

				if (baseNoise > MIN_BASENOISE_MOUNTAIN
					&& mountainNoise > MIN_MOUNTAINNOISE_MOUNTAIN)
				{
					// Set Height
					auto height = std::clamp(static_cast<uint32_t>((mountainNoise + MIN_BASENOISE_MOUNTAIN) * (double)MAX_ELEVATION + 1), 1U, MAX_ELEVATION);
					if (height > pGround->m_elevation)
						pGround->m_elevation = height;
					// Set Ground 
					if (mountainNoise > MIN_MOUNTAINNOISE_SNOW)
					{
						pGround->m_groundType = EGroundType::SNOW;
					}
					else
					{
						pGround->m_groundType = EGroundType::MOUNTAIN;
					}
				}
				/*
					TODO: generate special areas like beach or whatever
				*/
			}
		}

		tidyGroundMap();

		generateRivers();
	}
	void generateRivers() {
		auto drawLine = [](int32_t x1, int32_t y1, int32_t x2, int32_t y2, const std::function<void(int32_t, int32_t)>& drawPoint) {
			int dx = abs(x2 - x1);
			int dy = abs(y2 - y1);
			int sx = (x1 < x2) ? 1 : -1;
			int sy = (y1 < y2) ? 1 : -1;
			int err = dx - dy;

			while (true) {
				drawPoint(x1, y1);
				if (x1 == x2 && y1 == y2) {
					break;
				}
				int e2 = 2 * err;
				if (e2 > -dy) {
					err -= dy;
					x1 += sx;
				}
				if (e2 < dx) {
					err += dx;
					y1 += sy;
				}

				// settlers 45° skew, a diagonal move that goes ^> or v< needs an additional drawn point
				if (e2 > -dy && e2 < dx) {
					// v<
					if (sy == 1 && sx == -1) {
						drawPoint(x1 + 1, y1);
					}
					// ^>
					else if (sy == -1 && sx == 1) {
						drawPoint(x1, y1 + 1);
					}
				}
			}
		};
		auto placeRiver = [&](int32_t x, int32_t y) {
			SGround* pGround = groundAt(x, y);
			if (pGround->m_groundType == EGroundType::GRASS) {
				bool bIsAllGrassRiver = true;
				const auto isGrassOrRiver = [&bIsAllGrassRiver](uint32_t x, uint32_t y, SGround* ground) {
					bIsAllGrassRiver &= ground->m_groundType == EGroundType::GRASS || ground->m_groundType == EGroundType::RIVER4;
				};
				foreachNeighborGround(x, y, isGrassOrRiver);
				if (bIsAllGrassRiver) {
					pGround->m_groundType = EGroundType::RIVER4;
					// not true pGround->m_elevation = 0;
				}
			}
		};
		//drawLine(m_mapSize - 20, 20, 20, m_mapSize - 20, placeRiver);
		constexpr int chunksize = 48;
		//
		// set a point on every chunk, connect to neighbor dot on random chance
		//	
		// ignore the maps edge chunks
		//				- - - - -
		//            - R R R -
		//			- R R R -
		//		  - R R R -
		//		- - - - -
		// but still create points so rivers can connect to the edges
		const auto ITS = m_mapSize / chunksize;
		struct Point{
			uint32_t x;
			uint32_t y;
		};
		struct Line {
			Point p1;
			Point p2;
		};
		std::vector<std::vector<Point>> vecVecRiverPoints(ITS, std::vector<Point>(ITS));
		/*auto foreachNeighbor2DVec = [](const std::vector<std::vector<Point>>& vec, uint32_t x, uint32_t y, std::function<void(const Point&)>& func) {
			const auto rows = vec.size();
			const auto cols = vec[0].size();
			for (uint32_t xit = x; xit < x + 3; xit++) {
				for (uint32_t yit = y; yit < y + 3; yit++) {
					if (xit > 0 && // 0 => left side
						xit < (rows + 1) &&
						yit > 0 &&
						yit < (cols + 1) && 
						(xit != x || yit != y)) {
						func(vec[xit - 1][yit - 1]);
					}
				}
			}
		};*/
		auto foreachSideNeighbors2DVec = [](std::vector<std::vector<Point>>& vec, uint32_t x, uint32_t y, std::function<void(const Point&)> func) {
			const auto rows = vec.size();
			const auto cols = vec[0].size();
			if (x > 0) {
				func(vec[y][x - 1]);
			}
			if (y > 0) {
				func(vec[y-1][x]);
			}
			if (x < (cols - 1)) {
				func(vec[y][x + 1]);
			}
			if (y < (rows - 1)) {
				func(vec[y + 1][x]);
			}
		};
		for (uint32_t y_chunk = 0; y_chunk < ITS; y_chunk++) {
			for (uint32_t x_chunk = 0; x_chunk < ITS; x_chunk++) {
				//special case edge: draw river position in non-land area pos not between [(20,20),(mapsize-20,mapsize-20)]
				const uint32_t XRANDMAX = x_chunk ? chunksize : std::min(20,chunksize);
				const uint32_t YRANDMAX = y_chunk ? chunksize : std::min(20,chunksize);
				const uint32_t XRANDMIN = x_chunk != (ITS - 1) ? 0 : std::min(20,chunksize);
				const uint32_t YRANDMIN = y_chunk != (ITS - 1) ? 0 : std::min(20,chunksize);
				const uint32_t x = std::max(XRANDMIN, rand() % XRANDMAX) + x_chunk*chunksize;
				const uint32_t y = std::max(YRANDMIN, rand() % YRANDMAX) + y_chunk*chunksize;
				vecVecRiverPoints[x_chunk][y_chunk] = Point{ x,y };
			}
		}
		// connect neighbors in patterns of
		// - X - X - X
		// X - X - X -
		// - X - X - X
		// X - X - X -
		

		std::vector<Line> vecRiverLines;
		for (uint32_t i = 0; i < vecVecRiverPoints.size(); i++) {
			auto& row = vecVecRiverPoints[i];
			for (uint32_t j = i%2; j < row.size(); j+=2) {
				const auto& currentPoint = row[j];
				//m_pMapGeneratorHost->placeObject(currentPoint.x, currentPoint.y, EObjectType::Schilfrohrmittel);
				foreachSideNeighbors2DVec(vecVecRiverPoints, j, i, [&vecRiverLines,&currentPoint](const Point& neighborPoint) {
					if (rand() % 5 == 0) {
						vecRiverLines.push_back({ currentPoint,neighborPoint });
					}//m_pMapGeneratorHost->placeObject(neighborPoint.x, neighborPoint.y, EObjectType::Schilfrohrmittel);
				});
			}
		}
		for (auto& riverLine : vecRiverLines) {
			std::cout << "Riverline: " << riverLine.p1.x << " / " << riverLine.p1.y << " => " << riverLine.p2.x << " / " << riverLine.p2.y << "\n";
			drawLine(riverLine.p1.x, riverLine.p1.y, riverLine.p2.x, riverLine.p2.y, placeRiver);
 		}
	}
	virtual void generateObjects() {
		const EObjectType grassTypes[] = {
			EObjectType::Nesseln,
			EObjectType::HellesGras,
			EObjectType::Pampasgras,
			EObjectType::Loewenzahn,
			EObjectType::Beifuss, //mugwort
			EObjectType::Minze,
			EObjectType::Huflattich, //coltsfood
			EObjectType::Hirschzunge,
			EObjectType::Sauerampfer,
			EObjectType::Distel
		};
		const EObjectType bushTypes[] = {
			EObjectType::Ginsterbusch,
			EObjectType::Magnolie,
			EObjectType::Haselnuss,
			EObjectType::Holunder,
			EObjectType::Liguster,
			EObjectType::Weissdorn,
			EObjectType::Preiselbeere,
			EObjectType::Maulbeere,
			EObjectType::Pfaffenhuetchen
		};
		const EObjectType palmTypes[] = {
			EObjectType::Dattelpalme,
			EObjectType::Kokospalme,
			EObjectType::TropischeDattelpalme1,
			EObjectType::TropischeDattelpalme2,
			EObjectType::TropischePalme
		};
		const EObjectType darkForest1[] = {
			EObjectType::GrosserOlivenbaum,
			EObjectType::KleinerOlivenbaum,
			EObjectType::Pinie,
			EObjectType::GemeinePinie
		};
		const EObjectType darkForest2[] = {
			EObjectType::GrosserOlivenbaum,
			EObjectType::KleinerOlivenbaum,
			EObjectType::Pinie,
			EObjectType::GemeinePinie,

			EObjectType::GrosserOlivenbaum,
			EObjectType::KleinerOlivenbaum,
			EObjectType::Pinie,
			EObjectType::GemeinePinie,

			//half the chance of these, they ugly
			EObjectType::Korkeiche,
			EObjectType::Walnuss,
		};
		const EObjectType firForestTrees[] = {
			EObjectType::Fichte,
			EObjectType::Tanne
		};
		// seriously I am no arborist or whatever
		const EObjectType europeanForest1Trees[] = {
			EObjectType::Eiche,
			EObjectType::Birke,
			EObjectType::Linde,
			EObjectType::Kastanie
		};
		const EObjectType europeanForest2Trees[] = {
			EObjectType::Eiche,
			EObjectType::Buche,
			EObjectType::Silberpappel,
			EObjectType::Kastanie,
			EObjectType::Silberpappel,
			EObjectType::Ahorn
		};


		uint32_t placeStoneFieldChance = 15;
		for (uint32_t y = 0; y < m_mapSize; y+=32)
		{
			for (uint32_t x = 0; x < m_mapSize; x+=32)
			{
				bool bPlaceStoneField = rand() % placeStoneFieldChance == 0;
				placeStoneFieldChance--;
				if (bPlaceStoneField)
				{
					placeStoneForBuildingPlace(x+16, y+16,(15 -placeStoneFieldChance) / 2 +  10);
					placeStoneFieldChance = 15;
				}

			}
		}
		for (uint32_t y = 0; y < m_mapSize; y++)
		{
			for (uint32_t x = 0; x < m_mapSize; x++)
			{
				SGround* pGround = groundAt(x, y);
				if (isGrass(pGround))
				{

					uint32_t treeSpawnChance = getTreeSpawnChanceAt(x, y);
					auto noise = getBiomeNoiseAt(x, y);
					bool bSpawnTree = shouldSpawnTree(treeSpawnChance);
					if (bSpawnTree)
					{
						if (noise > MIN_DARKFOREST_NOISE)
						{
							placeRandomObject(x, y, darkForest1);
						}
						else if (noise > MIN_FIRFOREST_NOISE)
						{
							placeRandomObject(x, y, firForestTrees);
						}
						else if (noise > MIN_JUNGLEFOREST_NOISE)
						{
							placeRandomObject(x, y, darkForest2);
						}
						else if (noise > MIN_EUROFOREST1_NOISE)
						{
							placeRandomObject(x, y, europeanForest1Trees);
						}
						else if (noise > MIN_EUROFOREST2_NOISE)
						{
							placeRandomObject(x, y, europeanForest2Trees);
						}
						continue;
					}
					uint32_t grassSpawnChance = getGrassSpawnChanceAt(x, y);
					bool bSpawnGrass = shouldSpawnGrass(grassSpawnChance);
					if (bSpawnGrass)
					{
						placeRandomObject(x,y,grassTypes);
						continue;
					}
					bool bSpawnBush = shouldSpawnBush();
					if (grassSpawnChance > GRASS_FIELD_MAX_SPAWNRATE &&
						bSpawnBush)
					{
						placeRandomObject(x, y, bushTypes);
						continue;
					}
				}
				if (isNearBeach(x, y,pGround))
				{
					bool bSpawnPalm = shouldSpawnPalm();
					if (bSpawnPalm)
					{
						placeRandomObject(x, y, palmTypes);
					}
				}

			}
		}

	}
	virtual void generateResources() {
		const EResourceType oreTypes[] = {
			// 4 coal 3 iron 1 gold 2 stone 1 sulfur
			EResourceType::COAL_ORE,
			EResourceType::COAL_ORE,
			EResourceType::COAL_ORE,
			EResourceType::COAL_ORE,
			EResourceType::IRON_ORE,
			EResourceType::IRON_ORE,
			EResourceType::IRON_ORE,
			EResourceType::GOLD_ORE,
			EResourceType::STONE_ORE,
			EResourceType::STONE_ORE,
			EResourceType::SULFUR_ORE,
		};
		for (uint32_t y = 0; y < m_mapSize; y += 8)
		{
			for (uint32_t x = 0; x < m_mapSize; x += 8)
			{
				auto bPlaceResourceBlock = rand() % 2;
				if (bPlaceResourceBlock)
				{
					auto resourceType = oreTypes[rand() % sizeof(oreTypes)];
					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							auto* pGround = groundAt(x+i, y+j);
							if (pGround->m_groundType == EGroundType::MOUNTAIN) {
								auto* pGroundData = groundDataAt(x + i, y + j);
								pGroundData->m_resource.setResourceType(resourceType);
								pGroundData->m_resource.setResourceCount(8 + rand() % 8);
							}
							else if (isWater(pGround))
							{

								auto* pGroundData = groundDataAt(x + i, y + j);
								pGroundData->m_resource.setResourceType(EResourceType::FISH);
								pGroundData->m_resource.setResourceCount(8 + rand() % 8);
							}
						}
					}
				}
			}
		}
				
	}
};