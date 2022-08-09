#pragma once
#include "IMapGenerator.h"
#include "IMapGeneratorHost.h"
#include "SRandomMapParams.h"
#include <cstdint>
#include <map>

#include "PerlinNoise.hpp"
#include <functional>

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

	constexpr static inline double MIN_BASENOISE_GRASS = -0.2;
	constexpr static inline double MIN_BASENOISE_BEACH = -0.25;
	constexpr static inline double MIN_BASENOISE_MOUNTAIN = MIN_BASENOISE_GRASS;

	constexpr static inline double MIN_MOUNTAINNOISE_MOUNTAIN = 0.2;
	constexpr static inline double MIN_MOUNTAINNOISE_SNOW = 0.7;

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
		const CGroundHierarchy::SGroundNode& lowestGroundAccepted, uint8_t lowestLevelAccepted)
	{
		auto& hierarch = CGroundHierarchy::getInstance();
		auto neighborLevel = hierarch.getGroundLevel(pNeighborGround->m_groundType);
		if (neighborLevel < lowestLevelAccepted)
		{
			promoteGround(neighborX, neighborY, lowestGroundAccepted.m_prev->m_groundType); // level > 1 => prev != nullptr
		}
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
		for (uint32_t x = 0; x < m_mapSize; x++) {
			for (uint32_t y = 0; y < m_mapSize; y++)
			{
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

		constexpr auto scale = 0.01;

		const auto bignoise = m_perlinNoiseGen.octave2D(x * scale, y * scale, 8, 0.4);
		const auto smallnoise = 0.0;// m_perlinNoiseGen.noise2D(x * scale * 0.2, y * scale * 0.2);
		const auto noise = bignoise + smallnoise + flattenPerlinEdges(x, y);
		return noise;
	}
	inline auto getMountainNoiseAt(uint32_t x, uint32_t y) const
	{
		constexpr auto bigscale = 0.02;
		const auto bignoise = m_perlinNoiseGen.octave2D(x * bigscale, y * bigscale, 3);
		return bignoise;
	}
	/*
		Chance grass will spawn 1 to 21
	*/
	inline uint32_t getGrassSpawnChanceAt(uint32_t x, uint32_t y) const
	{
		constexpr auto smallerscale = 0.01;
		const auto noise = m_perlinNoiseGen.noise2D(x * smallerscale, y * smallerscale);
		return static_cast<uint32_t>(10.0 * (noise + 1.0)) + 1;
	}
	/*
		Chance grass will spawn 1 to 21
	*/
	inline uint32_t getTreeSpawnChanceAt(uint32_t x, uint32_t y) const
	{
		constexpr auto smallerscale = 0.05;
		const auto noise = m_perlinNoiseGen.noise2D((x+1024) * smallerscale, (y+1024) * smallerscale);
		auto between = [](double v, double min, double max) {return v >= min && v <= max; };
		bool bTreeRegion = 
			noise > 0.3 ||

			between(noise, -0.8,-0.7); //small
		return bTreeRegion ? 1 : 10000;
	}


	inline bool isGrass(const SGround* pGround) const {
		const auto type = pGround->m_groundType;
		return type == EGroundType::GRASS ||
			type == EGroundType::GRASS_DARK ||
			type == EGroundType::GRASS_ISLAND;
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

public:
	CPerlinMapGenerator(std::shared_ptr<IMapDecorator> pMapDecorator, IMapGeneratorHost* pMapGeneratorHost, const SRandomMapParams& params)
		:
		IMapGenerator(pMapDecorator),
		m_seed(params.m_seed), m_perlinNoiseGen(m_seed), m_pMapGeneratorHost(pMapGeneratorHost), m_params(params),
		m_mapSize(pMapGeneratorHost->getMapSize()), m_pGroundMap(pMapGeneratorHost->getGroundMapPtr()),m_pGroundDataMap(pMapGeneratorHost->getGroundDataMapPtr())
	{

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
			[this](uint32_t xFrom, uint32_t yFrom, uint32_t xTo, uint32_t yTo, const EGroundType& type,uint8_t elevation) {
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
					pGround->m_elevation = 1;
					pGround->m_groundType = EGroundType::GRASS;
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
		const EObjectType darkerTreeTypes[] = {
			EObjectType::GrosserOlivenbaum,
			EObjectType::KleinerOlivenbaum,
			EObjectType::Pinie,
			EObjectType::GemeinePinie
		};
		for (uint32_t y = 0; y < m_mapSize; y++)
		{
			for (uint32_t x = 0; x < m_mapSize; x++)
			{
				SGround* pGround = groundAt(x, y);
				if (isGrass(pGround))
				{

					uint32_t treeSpawnChance = getTreeSpawnChanceAt(x, y);
					bool bSpawnTree = shouldSpawnTree(treeSpawnChance);
					if (bSpawnTree)
					{
						placeRandomObject(x,y,darkerTreeTypes);
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
			EResourceType::COAL_ORE,
			EResourceType::IRON_ORE,
			EResourceType::GOLD_ORE,
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
								pGroundData->m_resource.setResourceCount(8 + rand() % 8 );
							}
						}
					}
				}
			}
		}
				
	}
};