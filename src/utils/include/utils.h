#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
#include <climits>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <random>

void FillWithRandom(std::vector<char> &bufferToFill) noexcept;

template<typename Iter>
void FillWithRandom(Iter start, const Iter end) noexcept
{
	std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char> randomEngine;
	std::generate(start, end, std::ref(randomEngine));
}

#endif // UTILS_HPP
