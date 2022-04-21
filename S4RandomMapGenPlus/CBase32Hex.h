#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <map>
//
// std::runtime_error
// '
// '- CBase32HexException
//    '
//    '- CBase32HexDecodeException
//    '
//    '- CBase32HexEncodeException
//
class CBase32HexException : public std::runtime_error {
public:
	CBase32HexException(const char* err) :std::runtime_error(err) {}
};
class CBase32HexDecodeException : public CBase32HexException {
public:
	CBase32HexDecodeException(const char* err) :CBase32HexException(err) {}
};
class CBase32HexEncodeException : public CBase32HexException {
public:
	CBase32HexEncodeException(const char* err) :CBase32HexException(err) {}
};

class CBase32Hex {
public:
	// RFC 3548 standard for Base32, also known as Base32Hex
	static inline const char m_saB32Alphabet[32] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'
	};
	// reverse LUT, not used in string de-/coding
	static inline const std::map<char, uint8_t> m_smAlphabetB32{
		{'0',0},		{'1',1},		{'2',2},		{'3',3},
		{'4',4},		{'5',5},		{'6',6},		{'7',7},
		{'8',8},		{'9',9},		{'A',10},		{'B',11},
		{'C',12},		{'D',13},		{'E',14},		{'F',15},
		{'G',16},		{'H',17},		{'I',18},		{'J',19},
		{'K',20},		{'L',21},		{'M',22},		{'N',23},
		{'O',24},		{'P',25},		{'Q',26},		{'R',27},
		{'S',28},		{'T',29},		{'U',30},		{'V',31}
	};
	/*
		Decodes a RFC 4648 standard Base32 string to a byte container
	*/
	static std::vector<uint8_t> decode(const std::string& b32Str);
	/*
		Decodes a single RFC 4648 standard Base32 char to an 8-bit byte
	*/
	static uint8_t decode(const char b32Char) {
		auto it = m_smAlphabetB32.find(b32Char);
		if (it != m_smAlphabetB32.end())
			return it->second;
		else
			throw CBase32HexDecodeException("Input char is not in Base32hex Alphabet");
	}
	/*
		Encodes a container of bytes to a RFC 4648 standard Base32 string
	*/
	static std::string encode(const std::vector<uint8_t>& aData);

	/*
		Encode a single 8-bit byte to its RFC 4648 standard Base32 character 
	*/
	static char encode(const uint8_t b32Char) {
		if (b32Char >= sizeof(m_saB32Alphabet))
			throw CBase32HexEncodeException("To be encoded char must be representable in 5 bits");
		return m_saB32Alphabet[b32Char];
	}
};