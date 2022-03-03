#include "utils.h"

std::string decodeURL(const std::string &stringToDecode)
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
