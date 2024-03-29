#pragma once
#include <cstdint>
#include <map>
#include <variant>
#include <vector>

#include <iostream>
enum class EGroundType : uint8_t {
	//0x0X
	WATER1 = 0,
	WATER2 = 1,
	WATER3 = 2,
	WATER4 = 3,
	WATER5 = 4,
	WATER6 = 5,
	WATER7 = 6,
	WATER8 = 7,
	WATER_BEACH = 8,

	// 0x1X
	GRASS = 16 + 0,
	GRASS_MOUNTAIN = 16 + 1,
	GRASS_ISLAND = 16 + 2,
	GRASS_DESERT = 16 + 4,
	GRASS_SWAMP = 16 + 5,
	GRASS_MUD = 16 + 7,
	GRASS_DARK = 16 + 8,
	GRASS_WEIRD = 16 + 9,
	GRASS_DUSTY = 16 + 12,
	GRASS_PAVEMENT = 16 + 13,

	// 0x2X
	MOUNTAIN = 32 + 0,
	MOUNTAIN_GRASS = 32 + 1,
	MOUNTAIN_SNOW = 32 + 3,

	// 0x3X
	BEACH = 48 + 0,

	// 0x4X
	DESERT = 64 + 0,
	DESERT_GRASS = 64 + 1,

	// 0x5X
	SWAMP = 80 + 0,
	SWAMP_GRASS = 80 + 1,

	// 0x6X
	RIVER1 = 96 + 0,
	RIVER2 = 96 + 1,
	RIVER3 = 96 + 2,
	RIVER4 = 96 + 3,

	// 0x7X
	UNIDENTIFIED_GRASS1 = 112 + 0,
	UNIDENTIFIED_GRASS2 = 112 + 1,
	UNIDENTIFIED_GRASS3 = 112 + 2,

	// 0x8X
	SNOW = 128 + 0,
	SNOW_MOUNTAIN = 128 + 1,

	// 0x9X
	MUD = 144 + 0,
	MUD_GRASS = 144 + 1,

	// extra stuff
	GLITCHED = 250,
}; 
class CGroundHierarchy {
public:
	struct SGroundNode {
		SGroundNode* m_prev;
		EGroundType m_groundType;
	};
private:
	/*
		The ground hierarchy tree. each EGroundType has to be placed on EGroundType.m_prev. A visual representation would look like this
		Format: Type (level)

		Water8 (1)
		-Water7 (2)
		-... (Water6 down to Water2)
		-..-Water_1 (8)
		-..--Water_Beach (9)
		-..---Beach (10)
		-..----Grass (11)
		-..-----Grass_Desert (12)
		-..------Desert_Grass (13)
		-..-------Desert (14)
		-..-----Grass_Swamp (12)
		-..------Swamp_Grass (13)
		-..-------Swamp (14)
		-..-----Grass_Mountain
		-..------Mountain_Grass
		-..-------Mountain
		-..--------Mountain_Snow
		-..---------Snow_Mountain
		-..----------Snow
		-..-----Grass_Dark
		-..-----Grass_Island
			etc

	*/
	std::map<EGroundType, SGroundNode> m_mGroundMapLevelTree;
	std::map<EGroundType, uint8_t> m_mGroundMapLevel;

	CGroundHierarchy() {
		/* The starting "root"/"stem" of tree. This structure looks a lot more like a tree than anything I've had in CS */
		const auto baseLinear = {
			EGroundType::WATER8,
			EGroundType::WATER7,
			EGroundType::WATER6,
			EGroundType::WATER5,
			EGroundType::WATER4,
			EGroundType::WATER3,
			EGroundType::WATER2,
			EGroundType::WATER1,
			EGroundType::WATER_BEACH,
			EGroundType::BEACH,
			EGroundType::GRASS,
		};
		SGroundNode* prev = nullptr;
		for (auto& linearType : baseLinear)
		{
			auto& t = m_mGroundMapLevelTree[linearType];
			t.m_prev = prev;
			t.m_groundType = linearType;

			prev = &t;
		}
		/*
			All nodes that link to GRASS
		*/

		const auto grassDescents = {
			EGroundType::RIVER1,
			EGroundType::RIVER2,
			EGroundType::RIVER3,
			EGroundType::RIVER4,
			EGroundType::GRASS_DUSTY,
			//EGroundType::GRASS_DARK,
			EGroundType::GRASS_WEIRD,
			EGroundType::GRASS_ISLAND,
			EGroundType::GRASS_PAVEMENT,
			EGroundType::GRASS_DESERT,
			EGroundType::GRASS_SWAMP,
			EGroundType::GRASS_MOUNTAIN,
			EGroundType::GRASS_MUD,
		};
		for (auto& grassDescent : grassDescents)
		{

			auto& t = m_mGroundMapLevelTree[grassDescent];
			t.m_prev = prev; // grass is last in list.
			t.m_groundType = grassDescent;
		}

		/*
			All descendant lines from each descendant of grass
		*/
		
		const std::map<EGroundType, std::vector<EGroundType>> grassSubLines = {
			{
				EGroundType::GRASS_DESERT,
				std::vector<EGroundType>{
					EGroundType::DESERT_GRASS,
					EGroundType::DESERT
				}
			},
			{
				EGroundType::GRASS_SWAMP,
				std::vector<EGroundType>{
					EGroundType::SWAMP_GRASS,
					EGroundType::SWAMP
				}
			},
			{
				EGroundType::GRASS_WEIRD,
				std::vector<EGroundType>{
					EGroundType::GRASS_DARK
				}
			},
			{
				EGroundType::GRASS_MOUNTAIN,
				std::vector<EGroundType>{
					EGroundType::MOUNTAIN_GRASS,
					EGroundType::MOUNTAIN,
					EGroundType::MOUNTAIN_SNOW,
					EGroundType::SNOW_MOUNTAIN,
					EGroundType::SNOW,
				}
			},
			{
				EGroundType::GRASS_MUD,
				std::vector<EGroundType>{
					EGroundType::MUD_GRASS,
					EGroundType::MUD
				}
			}
		};
		for (auto& grassSubLine : grassSubLines)
		{
			auto& linkTo = m_mGroundMapLevelTree[grassSubLine.first];
			SGroundNode* prevSub = &linkTo;
			for (auto& lineEl : grassSubLine.second)
			{
				auto& t = m_mGroundMapLevelTree[lineEl];
				t.m_prev = prevSub;
				t.m_groundType = lineEl;

				prevSub = &t;
			}
		}

		auto calcLevel = [](SGroundNode& n) {
			uint8_t lvl = 1;
			SGroundNode* it = &n;
			while (it != nullptr)
			{
				lvl++;
				it = it->m_prev;
			}
			return lvl;
		};
		/* Set all groundMapLevels */
		for (auto& levelItem : m_mGroundMapLevelTree)
		{
			m_mGroundMapLevel[levelItem.first] = calcLevel(levelItem.second);
		}


	}
	static CGroundHierarchy m_sInstance;
public:
	inline static CGroundHierarchy& getInstance() { return m_sInstance; }
	inline uint8_t getGroundLevel(const EGroundType& type) const {
		return m_mGroundMapLevel.at(type);
	}
	inline const SGroundNode& getGroundLevelNode(const EGroundType& type) const {
		return m_mGroundMapLevelTree.at(type);
	}
};
struct SGround {
	/*
		Elevation of the ground. 
		Value Range: 0-225 (0xE1)
		The delta of the elevation of this ground to the neighbor ground with the biggest difference in elevation has to be 5
	*/
	uint8_t m_elevation;
	/*
		The hierarchy of Ground types goes as follows (E.g. Beach can be placed on water, but Grass cannto be placed on water)
		Please take transitional grounds for Grass into consideration. (Grass->[Grass/Mountain->Mountain/Grass]->Mountain)
															 (not all though, see: Grass->Dust)

		Water8->Water7
		Water7->Water6
		Water6->Water...
		...
		Water1->Beach
		Beach->Grass
		Grass->(Dark Grass|Desert|Island Grass|Mountain|Dirt Road|Stone Road|Swamp|River1|River2|River3|River4)
		Mountain->Snow

		River has special regulations:
		- River1-4 can have up to 3 River1-4 around it, all spaced by one Grass ground
		- River4 can mound into the water from one! side, replacing Grass&Beach with Water1
			- The "Pathfinding" can be relatively long with ~10 grounds (estimate) (Editor messes this completely up. Pathfinding doesn't take groundID into consideration (you can break mentioned hierarchy with this)
	*/
	EGroundType m_groundType;
	// set (by compile map function?) through gfxengine in s4editor
	// 
	// first byte (SGround + 0x2) player has to interact with these things to change them
	// MSB - LSB
	// 8th bit - unused?
	// 7th bit - dark ground flag
	// 6th bit - pond water flag
	// 5th bit - ???
	// [4th,1st] bit - light intensity, 0x0 darkest, 0x8 default, 0xF brightest
	// 
	// second byte (SGround + 0x3) quite possibly actively changed mid game
	// MSB - LSB
	// 8th bit - show land mark (usually only shown on the outer edge of your land). Will get "fixed" in game again.
	// 7-4th bit - ?
	// [3rd,1st] bit - darkness intensity (7 = undiscovered, 6 = very very dark, 2 = light dark (render objects again) , 0 = visible)
	uint8_t m_graphics[2];

	inline void setDarkLand(bool dark) {
		if (dark)
			m_graphics[0] |= 0x40; // 0100 0000
		else
			m_graphics[0] &= 0xBF; // 1011 1111
	}
	inline void setPondLand(bool pond) {
		if (pond)
			m_graphics[0] |= 0x20; // 0010 0000
		else
			m_graphics[0] &= 0xDF; // 1101 1111
	}
	inline void setLightIntensity(uint8_t intensity)
	{
		m_graphics[0] &= 0xF0;
		m_graphics[0] |= (0x0F & intensity);
	}
};
static_assert(sizeof(SGround) == 4, "SGround has to be 4 bytes");