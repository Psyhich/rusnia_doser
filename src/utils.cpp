#include <random>
#include <algorithm>
#include <climits>

#include "utils.h"


std::string decodeURL(const std::string &stringToDecode) noexcept
{
	std::string ret;
	char ch;
	int i, ii;

	for (i = 0; i < stringToDecode.length(); i++)
	{
		if (int(stringToDecode[i]) == 37)
		{
			sscanf(stringToDecode.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i += 2;
		}
		else
		{
			ret += stringToDecode[i];
		}
	}
	return ret;
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
