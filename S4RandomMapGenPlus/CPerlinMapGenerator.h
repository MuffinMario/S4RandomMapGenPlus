#pragma once
#include "IMapGenerator.h"
#include "IMapGeneratorHost.h"
#include "SRandomMapParams.h"
#include <cstdint>
#include <map>

#include "PerlinNoise.hpp"
#include <functional>

class CPerlinMapGenerator :public IMapGenerator {
	const siv::PerlinNoise::seed_type m_seed;
	const siv::PerlinNoise m_perlinNoiseGen;
	IMapGeneratorHost* m_pMapGeneratorHost;
	
	const SRandomMapParams m_params;

	uint32_t m_mapSize;
	SGround* m_pGroundMap;

	double flattenPerlinEdges(uint32_t x, uint32_t y) {
#undef min
		auto distanceToEdge = std::min(std::min(x, m_mapSize-x), std::min(y, m_mapSize - y));
		if (distanceToEdge < 50)
		{
			return -1.3 * (50.0-distanceToEdge) / 50;
		}
		else {
			return 0.0;
		}
	}
	void promoteGround(uint32_t x, uint32_t y, EGroundType promoteNode) {
		auto& hierarch = CGroundHierarchy::getInstance();

		auto thisNodeLevel = hierarch.getGroundLevel(promoteNode);
		groundAt(x, y)->m_groundType = promoteNode;
		groundAt(x, y)->m_elevation = thisNodeLevel <= 9 ? 0 : 1; // water is elev 0, else 1

		uint8_t lowestLevelAccepted = hierarch.getGroundLevel(promoteNode) - 1;
		auto& currentNode = hierarch.getGroundLevelNode(promoteNode);

		auto fpThisHierarchyCheckGround = std::bind(&CPerlinMapGenerator::hierarchyCheckGround, *this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, currentNode, lowestLevelAccepted);
		foreachNeighborGround(x, y, fpThisHierarchyCheckGround);
	}
	void hierarchyCheckGround(
		const uint32_t neighborX, const uint32_t neighborY, 
		const SGround* pNeighborGround,
		const CGroundHierarchy::SGroundNode& lowestGroundAccepted, uint8_t lowestLevelAccepted) {
		auto& hierarch = CGroundHierarchy::getInstance();
		auto neighborLevel = hierarch.getGroundLevel(pNeighborGround->m_groundType);
		if (neighborLevel < lowestLevelAccepted)
		{
			promoteGround(neighborX, neighborY, lowestGroundAccepted.m_prev->m_groundType); // level > 1 => prev != nullptr
		}
	}
	void foreachNeighborGround(uint32_t x, uint32_t y, std::function<void(const uint32_t, const uint32_t,const SGround*)> f) {
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
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, currentNode,lowestLevelAccepted);
				foreachNeighborGround(x, y, fpThisHierarchyCheckGround);
			}
		}
	}
	inline SGround* groundAt(uint32_t x, uint32_t y) {
		return &m_pGroundMap[x + y * m_mapSize];
	};
public:
	CPerlinMapGenerator(std::shared_ptr<IMapDecorator> pMapDecorator, IMapGeneratorHost* pMapGeneratorHost, const SRandomMapParams& params)
		:
		IMapGenerator(pMapDecorator),
		m_seed(params.m_seed), m_perlinNoiseGen(m_seed), m_pMapGeneratorHost(pMapGeneratorHost), m_params(params),
		m_mapSize(pMapGeneratorHost->getMapSize()), m_pGroundMap(pMapGeneratorHost->getGroundMapPtr())
	{

	}
	virtual void generateGround() {
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

		for (uint32_t x = 20; x < m_mapSize-20; x++)
		{
			for (uint32_t y = 20; y < m_mapSize-20; y++)
			{
				auto pGround = groundAt(x, y);
				auto scale = 0.01;

				auto bignoise = m_perlinNoiseGen.octave2D(x * scale, y * scale, 8, 0.4);
				auto smallnoise = 0.0;// m_perlinNoiseGen.noise2D(x * scale * 0.2, y * scale * 0.2);
				auto noise = bignoise + smallnoise;
				noise += flattenPerlinEdges(x, y);
				if (noise > -0.2)
				{
					pGround->m_elevation = 1;
					pGround->m_groundType = EGroundType::GRASS;
					auto hillnoise = m_perlinNoiseGen.noise2D(x, y);
					auto height = std::clamp(static_cast<uint32_t>(((noise+0.2) * (1.0+0.2)/(1.0) * std::abs(hillnoise)) * (double)5 + 1), 1U, 6U);
					pGround->m_elevation = height;
				}
				else if (noise > -0.25)
				{
					pGround->m_elevation = 1;
					pGround->m_groundType = EGroundType::BEACH;
				}

				scale = 0.02;
				bignoise = m_perlinNoiseGen.octave2D(x * scale, y * scale,3);
				auto newnoise = bignoise + smallnoise;

				if (noise > -0.2 && 
					newnoise > 0.2)
				{
					pGround->m_groundType = EGroundType::MOUNTAIN;
					auto height = std::clamp(static_cast<uint32_t>((newnoise - 0.2) * (double)0xE1 + 1), 1U, 0xE1U);
					if(height > pGround->m_elevation)
						pGround->m_elevation = height;
				}

			}
		}
		auto types = {
			std::make_pair<EGroundType,uint8_t>(EGroundType::BEACH, 1),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER_BEACH,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER1,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER2,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER3,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER4,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER5,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER6,0),
			std::make_pair<EGroundType,uint8_t>(EGroundType::WATER7,0)
		};

		tidyGroundMap();

	}
	virtual void generateObjects() {

	}
	virtual void generateResources() {

	}
};