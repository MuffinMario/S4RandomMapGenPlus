
#include "pch.h"
#include "CDeriveKey.h"

CDeriveKey::CDeriveKey(const std::string& key) : m_key(key) {
	if (key.length() != KEY_SIZE)
		throw CDeriveKeyKeyException("Key has to be exactly 8 symbols long");
}

SRandomMapData CDeriveKey::derive() {
	SRandomMapData randomMapData;
	randomMapData.m_resourceAmount = decode<EResourceAmount>(KeyOffsets::resourceAmount);
	randomMapData.m_playerCount = decode<uint8_t>(KeyOffsets::playerCount);
	randomMapData.m_mirrorShortDiagonal = decode<bool>(KeyOffsets::mirrorShortDiagonal);
	randomMapData.m_mapSize = decode<EMapSize>(KeyOffsets::mapSize);
	randomMapData.m_landMassPercentage = decode<ELandMassPercentage>(KeyOffsets::landMassPercentage);
	randomMapData.m_mirrorLongDiagonal = decode<bool>(KeyOffsets::mirrorLongDiagonal);
	randomMapData.m_hasToBeZero = decode<uint8_t>(KeyOffsets::hasToBeZero);

	std::string seed = m_key.substr(3, 5);
	//std::reverse(seed.begin(), seed.end());
	auto seedBytes = CBase32Hex::decode(seed);
	if (seedBytes.size() != 3) // 999.999 is max number input possible, 2^20-1 = ~1.024m is max number possible with 4 B32 numbers -> 2^16 < 2^20 < 2^24 => 3 byte  
		throw CDeriveKeyKeyException("Unexpected error. Seeds can not be decoded to exactly 4 bytes");
	// assume little endianness system
	// safe 
	randomMapData.m_seed = 0U; // default initalized to 0U already, but oh well
	for (size_t i = 0; i < 3; i++)
	{
		randomMapData.m_seed |= seedBytes[i] << (i * 8);
	}

	return randomMapData;
}
