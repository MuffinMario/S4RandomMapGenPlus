#include "pch.h"
#include "CBase32Hex.h"
#include <algorithm>
#include <vector>
#include <string>
#include <iterator>
///*
//Decodes a RFC 4648 standard Base32 string to a byte container
//*/
//
//std::vector<uint8_t> CBase32Hex::decode(const std::string& b32Str) {
//	std::basic_string<unsigned char> b32StrEnc; // copy for operations (may operate on data), reinterpret characters as unsigned char
//	b32StrEnc.resize(b32Str.size());
//	std::copy(b32Str.begin(), b32Str.end(), b32StrEnc.data());
//
//	if (!Base32::Unmap32(b32StrEnc.data(), b32StrEnc.size(), reinterpret_cast<unsigned const char*>(m_saB32Alphabet)))
//	{
//		throw CBase32HexDecodeException("Cannot Unmap alphabet to Base32 Input");
//	}
//
//	std::vector<uint8_t> data;
//	data.resize(Base32::GetDecode32Length(b32Str.length()));
//
//	if (!Base32::Decode32(b32StrEnc.data(), b32StrEnc.length(), data.data()))
//	{
//		throw CBase32HexDecodeException("Cannot Decode Base32 Input");
//	}
//	return data;
//}

std::vector<uint8_t> CBase32Hex::decode(const std::string& b32Str)
{
	if (b32Str.length() < 1)
		return std::vector<uint8_t>();

	std::vector<uint8_t> data;
	using uint5_t = uint8_t;
	/*
	* byte offsets ( inverse if result < 5)
	* inverse (  -shifts % 8 )
	* shifts ( shifts_previos + 5 % 8)
	* 0 3 0 1 4 0 2 0
	* 0 3 6 1 4 7 2 0
	* 0 5 2 7 4 1 6 3
	*/
	// constexpr size_t BASE_32_BITCOUNT = 5;
	// constexpr size_t BASE_256_BITCOUNT = 8;
	// non-precomputated
	// signed char leftover = 3; // 8 - 5 from below operations
	//for (i = 1; i < b32Str.length(); i++)
	//{
	//	currentChar = b32Str[i];
	//	currentLUTValue = m_saAlphabetB32.at(currentChar);
	//	// base32 char we are reading will exceed highest current byte
	//	if (leftover < BASE_32_BITCOUNT)
	//	{
	//		*lastData |= currentLUTValue << (BASE_256_BITCOUNT - leftover);
	//		lastData = &data.emplace_back(currentLUTValue >> leftover);
	//		leftover = BASE_256_BITCOUNT - (BASE_32_BITCOUNT -leftover);
	//	}
	//	//base 32 char fits nice into the highest current byte
	//	else {
	//		*lastData |= currentLUTValue << (8 - leftover);
	//		leftover -= BASE_32_BITCOUNT;
	//	}
	//}
	static uint8_t byteOffsets[8] = { 0,3,0,1,4,0,2,0 };
	static uint8_t shifts[8] = { 0,5,2,7,4,1,6,3 };

	size_t i = 0;
	// first iteration outside of loop for caching of previous element ptr
	char currentChar = b32Str[i];
	uint8_t currentLUTValue = m_smAlphabetB32.at(currentChar);
	uint5_t* lastData = &data.emplace_back(m_smAlphabetB32.at(currentChar));

	for (i = 1; i < b32Str.length(); i++)
	{
		currentChar = b32Str[i];
		currentLUTValue = m_smAlphabetB32.at(currentChar);
		
		uint8_t& byteOffset = byteOffsets[i % 8];
		uint8_t& shift = shifts[i % 8];

		*lastData |= currentLUTValue << shift;
		if(byteOffset)
			lastData = &data.emplace_back(currentLUTValue >> byteOffset);
	}
	return data;
}
#undef max
/*
Encodes a container of bytes to a RFC 4648 standard Base32 string
*/
std::string CBase32Hex::encode(const std::vector<uint8_t>& aData) {
	/*
		example data {size = 7} => pad to {size = 10} (padding symbol: 0)
		[00000000 00000000 00000000 00000000 00000000] 
		[00000000 00000000 PPPPPPPP PPPPPPPP PPPPPPPP]
		    2		 3		2			3		2
	*/

	std::string b32Str;
	std::vector<uint64_t> blocks;
	blocks.reserve(aData.size() / 5 + ((aData.size() % 5 != 0) ? 1 : 0));
	for (size_t i = 0; i < aData.size(); i+=5)
	{
		auto& curblock = blocks.emplace_back(0U);
		/* combine next bytes into block of 40 bits*/
		for (uint32_t j = 0; j < 5 && ((i + j) < aData.size()); j++) {
			curblock |= static_cast<uint64_t>(aData[i + j]) << (j * 8);
		}
	}

	for (size_t i = 0; i < blocks.size(); i ++)
	{
		for (size_t j = 0; j < 5; j++)
		{
			//bitmask the j*5 lowest bits, apply bitmask on integer, shift integer j*5 bits to the right to become the 5 LSB => index of alphabet
#pragma warning(push)
#pragma warning(disable: 6297 ) // max(5*j) j=4 => shift 0b0001_1111 << 20, highest activated bit is 24 < 64
			uint64_t shifted1F = static_cast<uint64_t>(0x1F) << static_cast<uint64_t>(5 * j);
			uint32_t j5BitSegment = static_cast<uint32_t>((blocks[i] & shifted1F) >> (5 * j));
#pragma warning(pop)
			b32Str.append(1, m_saB32Alphabet[j5BitSegment]);
		}
	}
	return b32Str;
}
