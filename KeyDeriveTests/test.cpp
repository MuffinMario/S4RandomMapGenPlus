#include "pch.h"
#include "../S4RandomMapGenPlus/CDeriveKey.h"
#include "../S4RandomMapGenPlus/CBase32Hex.cpp"

// RFC 3548 standard for Base32, also known as Base32Hex
static const char m_saB32Alphabet[32] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'
};
// reverse LUT, not used in string de-/coding
static const std::map<char, uint8_t> m_saAlphabetB32{
	{'0',0},		{'1',1},		{'2',2},		{'3',3},
	{'4',4},		{'5',5},		{'6',6},		{'7',7},
	{'8',8},		{'9',9},		{'A',10},		{'B',11},
	{'C',12},		{'D',13},		{'E',14},		{'F',15},
	{'G',16},		{'H',17},		{'I',18},		{'J',19},
	{'K',20},		{'L',21},		{'M',22},		{'N',23},
	{'O',24},		{'P',25},		{'Q',26},		{'R',27},
	{'S',28},		{'T',29},		{'U',30},		{'V',31}
};

TEST(Base32EncodeTest, EncodeTest1) {
	std::string expected = "00100";
	std::vector<uint8_t> enc{ 0,4 };
	std::string got = CBase32Hex::encode(enc);
	EXPECT_STREQ(expected.c_str(), got.c_str());
}
TEST(Base32EncodeTest, CoverAllVariables0To999999) {
	std::vector<uint8_t> container;
	// represent our number with 4 bytes;
	constexpr size_t BYTES = 4;
	container.resize(BYTES);
	for (uint32_t i = 0; i < 1000000; i++)
	{
		char b32[6] = { 0 }; // N-T
		for (uint32_t j = 0; j < 4; j++) // covers a smaller range than the function
		{
			//bitmask the j*5 lowest bits, apply bitmask on integer, shift integer j*5 bits to the right to become the 5 LSB => index of alphabet
			uint32_t j5BitSegment = ((i & (0x1F << 5 * j)) >> 5 * j);
			b32[j] = m_saB32Alphabet[j5BitSegment];
		}
		b32[4] = '0'; // zero padding
		for (uint32_t j = 0; j < BYTES; j++)
		{
			container[j] = (i >> (j * 8)) & 0xFF;
		}
		auto res = CBase32Hex::encode(container);

		EXPECT_STREQ(res.c_str(), b32);

	}
}
TEST(Base32DecodeTest, CoverAllVariables0To999999) {
	for (uint32_t i = 0; i < 1000000; i++)
	{
		char b32[5] = { 0 }; // N-T
		for (uint32_t j = 0; j < 4; j++)
		{
			//bitmask the j*5 lowest bits, apply bitmask on integer, shift integer j*5 bits to the right to become the 5 LSB => index of alphabet
			uint32_t j5BitSegment = ((i & (0x1F << 5 * j)) >> 5 * j);
			b32[j] = m_saB32Alphabet[j5BitSegment];
		}
		auto res = CBase32Hex::decode(b32);
		EXPECT_EQ(res.size(), 3);

		unsigned int resInt = 0U;
		for (size_t i = 0; i < res.size(); i++)
		{
			resInt |= res[i] << (i * 8);
		}
		EXPECT_EQ(resInt, i);
	}
}
TEST(Base32DecodeTest, CoverAllVariables10000000To10999999) {
	for (uint32_t i = 10000000; i < 11000000; i++)
	{

		char b32[8] = { 0 }; // N-T
		for (uint32_t j = 0; j < 7; j++)
		{
			//bitmask the j*5 lowest bits, apply bitmask on integer, shift integer j*5 bits to the right to become the 5 LSB => index of alphabet
			uint32_t j5BitSegment = ((i & (0x1F << 5 * j)) >> 5 * j);
			b32[j] = m_saB32Alphabet[j5BitSegment];
		}
		auto res = CBase32Hex::decode(b32);
		EXPECT_EQ(res.size(), 5);

		unsigned int resInt = 0U;
		for (size_t i = 0; i < res.size(); i++)
		{
			resInt |= res[i] << (i * 8);
		}
		EXPECT_EQ(resInt, i);
	}
}
TEST(Base32DecodeTest, CoverAllVariables0To31) {
	for (uint32_t i = 0; i < 32; i++)
	{

		char b32[2] = { 0 }; // N-T
		for (uint32_t j = 0; j < 1; j++)
		{
			//bitmask the j*5 lowest bits, apply bitmask on integer, shift integer j*5 bits to the right to become the 5 LSB => index of alphabet
			uint32_t j5BitSegment = ((i & (0x1F << 5 * j)) >> 5 * j);
			b32[j] = m_saB32Alphabet[j5BitSegment];
		}
		auto res = CBase32Hex::decode(b32);

		EXPECT_EQ(res.size(), 1);

		unsigned int resInt = 0U;
		for (size_t i = 0; i < res.size(); i++)
		{
			resInt |= res[i] << (i * 8);
		}
		EXPECT_EQ(resInt, i);
	}
}