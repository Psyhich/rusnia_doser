#include <random>

#include "utils.h"

void FillWithRandom(std::vector<char> &bufferToFill) noexcept
{
	std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char> randomEngine;
	std::generate(
		std::begin(bufferToFill), 
		std::end(bufferToFill), 
		std::ref(randomEngine));
}

