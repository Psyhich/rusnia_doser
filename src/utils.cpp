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

