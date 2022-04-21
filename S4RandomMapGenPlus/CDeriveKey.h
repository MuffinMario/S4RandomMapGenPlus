#pragma once
#include <string>
#include <stdexcept>
#include "SRandomMapParams.h"
#include "CBase32Hex.h"
class CDeriveKeyKeyException : public std::runtime_error {
public:
	CDeriveKeyKeyException(const char* err) :std::runtime_error(err){}
};
enum class EResourceAmount : uint8_t {
	RESOURCES_LITTLE,
	RESOURCES_MEDIUM,
	RESOURCES_HIGH,
};
enum class EMapSize : uint8_t {
	MAP_256x256,
	MAP_320x320,
	MAP_384x384,
	MAP_448x448,
	MAP_512x512,
	MAP_576x576,
	MAP_640x640,
	MAP_704x704,
	MAP_768x768,
	MAP_832x832,
	MAP_896x896,
	MAP_960x960,
	MAP_1024x1024
};
enum class ELandMassPercentage : uint8_t {
	LANDMASS_10_PERCENT,
	LANDMASS_20_PERCENT,
	LANDMASS_30_PERCENT,
	LANDMASS_40_PERCENT,
	LANDMASS_50_PERCENT,
	LANDMASS_60_PERCENT,
	LANDMASS_70_PERCENT,
	LANDMASS_80_PERCENT,
	LANDMASS_90_PERCENT,
};
struct SRandomMapData {
	// resource amount
	EResourceAmount m_resourceAmount{EResourceAmount::RESOURCES_LITTLE};
	// Amount of players on map
	uint8_t m_playerCount{1};
	// determines if the map shall be mirrored on its short diagonal (topleft-bottomright)
	bool m_mirrorShortDiagonal{false};
	// map size
	EMapSize m_mapSize{EMapSize::MAP_256x256};
	// land mass percentage
	ELandMassPercentage m_landMassPercentage{ELandMassPercentage::LANDMASS_10_PERCENT};
	// determines if the map shall be mirrored on its long diagonal (bottomleft-topright)
	bool m_mirrorLongDiagonal{false};
	// seed of the map, 4 characters of base32hex
	unsigned int m_seed{ 0 };
	// in the original game/editor, the last character has to be 0, else it will tell you it's incorrect.
	uint8_t m_hasToBeZero{ 0 };
};
namespace KeyOffsets {
	template<size_t CharOffset, size_t BitOffset, size_t BitCount>
	class KeyOffset {
	public:
		constexpr size_t getCharOffset() { return CharOffset; }
		constexpr size_t getBitOffset() { return BitOffset; }
		constexpr size_t getBitCount() { return BitCount; }
	};
	constexpr KeyOffset<0, 3, 2> resourceAmount;
	constexpr KeyOffset<0, 0, 3> playerCount;
	constexpr KeyOffset<1, 4, 1> mirrorShortDiagonal;
	constexpr KeyOffset<1, 0, 4> mapSize;
	constexpr KeyOffset<2, 1, 4> landMassPercentage;
	constexpr KeyOffset<2, 0, 1> mirrorLongDiagonal;
	constexpr KeyOffset<3, 0, 20> seed;// useless since multiple bits, should be different class (not worth the effort)
	constexpr KeyOffset<7, 0, 5> hasToBeZero;
}
class CDeriveKey {
	constexpr const static size_t KEY_SIZE = 8;
	std::string m_key;

	template<typename T,size_t CharOffset, size_t BitOffset, size_t BitCount>
	T decode(KeyOffsets::KeyOffset<CharOffset, BitOffset, BitCount> keyOffset)
	{
		// decode char to byte -> shift to offset -> mask to bit count
		constexpr auto charOffset = keyOffset.getCharOffset();
		constexpr auto bitOffset = keyOffset.getBitOffset();
		constexpr auto bitCount = keyOffset.getBitCount();

		uint8_t decodedBit = CBase32Hex::decode(m_key.at(charOffset));
		uint8_t decodedBitShifted = decodedBit >> bitOffset;
		uint8_t bitmask = (1 << bitCount) - 1;
			return static_cast<T>(
				decodedBitShifted & bitmask
				);
	}
public:
	CDeriveKey(const std::string& key);
	SRandomMapData derive();
};