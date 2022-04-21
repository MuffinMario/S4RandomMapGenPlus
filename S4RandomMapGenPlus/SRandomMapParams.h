#pragma once
#include <cstdint>

/* Albeit a lot of fields being adressed by their singular byte, this struct has an interesting spacing of 4 bytes per field? */
struct SRandomMapParams {
	// [0x0] ?? is 0xB8
	uint32_t _unknownDWord;
	// [0x4] Seed 0-999.999
	uint32_t m_seed;
	// [0x8] unused due to Host containing getMapSize() function.
	uint32_t m_mapSize;
	// [0xC] 
	uint32_t m_landMassPercentage;
	// [0x10] 
	uint8_t m_resourceCount; // resource count, 5 = low, 0xa = medium, 0xF = high
	// [0x14]
	uint32_t m_mirrorType;// 0 = no mirror
	// [0x18-0x1F] unknown, always has been 8 zero bytes and are not used
	uint8_t _unknown[8];
	// [0x20]
	uint32_t m_partyCount;
	// [0x24]
	uint32_t m_dontPlaceObjects;
};