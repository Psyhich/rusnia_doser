#include <random>
#include <algorithm>
#include <climits>

#include "utils.h"

std::string DecodeURL(const std::string &stringToDecode) noexcept
{
	std::string returnString;
	returnString.reserve(stringToDecode.size());

	char parsedChar;
	std::size_t index;
	unsigned int valueToParse;

	char buffer[3];
	buffer[2] = '\0';

	for (index = 0; index < stringToDecode.length(); index++)
	{
		if (int(stringToDecode[index]) == 37)
		{
			buffer[0] = stringToDecode[index + 1];
			buffer[1] = stringToDecode[index + 2];

			sscanf(buffer, "%x", &valueToParse);
			parsedChar = static_cast<char>(valueToParse);
			returnString += parsedChar;
			index += 2;
		}
		else
		{
			returnString.push_back(stringToDecode[index]);
		}
	}

	returnString.shrink_to_fit();
	return returnString;
}

void FillWithRandom(std::vector<char> &bufferToFill) noexcept
{
	std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char> randomEngine;
	std::generate(
		std::begin(bufferToFill), 
		std::end(bufferToFill), 
		std::ref(randomEngine));
}

std::string GetRandomIP() noexcept
{
	std::uniform_int_distribution<int> dist(0,255);
	std::random_device rd;

	std::string ipString{std::to_string(dist(rd))};
	for(short i = 0; i < 3; i++)
	{
		ipString += '.';
		ipString += std::to_string(dist(rd));
	}

	return ipString;
}

int GetRandomPort() noexcept
{
	std::uniform_int_distribution<int> dist(0, 65535);
	std::random_device rd;
	return dist(rd);
}
