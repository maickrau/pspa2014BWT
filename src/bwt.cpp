#include <cassert>
#include <algorithm>

#include "bwt.h"

std::vector<size_t> step1(const char* text, size_t textLen)
{
	return step1<unsigned char>((const unsigned char*)text, textLen, 255);
}

std::vector<size_t> step1(const unsigned char* text, size_t textLen)
{
	return step1<unsigned char>((const unsigned char*)text, textLen, 255);
}

std::vector<size_t> step2(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step2<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft);
}

std::vector<size_t> step2(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step2<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft);
}

std::vector<size_t> step3(const char* text, size_t textLen, const std::vector<size_t>& LMSRight)
{
	return step3<unsigned char>((const unsigned char*)text, textLen, 255, LMSRight);
}

std::vector<size_t> step3(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight)
{
	return step3<unsigned char>((const unsigned char*)text, textLen, 255, LMSRight);
}

std::pair<std::vector<size_t>, std::vector<size_t>> step4(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step4<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft);
}

std::pair<std::vector<size_t>, std::vector<size_t>> step4(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step4<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft);
}

std::vector<size_t> step7(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft, unsigned char* result, const std::vector<size_t>& charSums)
{
	return step7<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft, (unsigned char*)result, charSums);
}

std::vector<size_t> step7(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft, char* result, const std::vector<size_t>& charSums)
{
	return step7<unsigned char>((const unsigned char*)text, textLen, 255, LMSLeft, (unsigned char*)result, charSums);
}

std::vector<size_t> step8(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight, unsigned char* result, const std::vector<size_t>& charSums)
{
	return step8<unsigned char>((const unsigned char*)text, textLen, 255, LMSRight, (unsigned char*)result, charSums);
}

std::vector<size_t> step8(const char* text, size_t textLen, const std::vector<size_t>& LMSRight, char* result, const std::vector<size_t>& charSums)
{
	return step8<unsigned char>((const unsigned char*)text, textLen, 255, LMSRight, (unsigned char*)result, charSums);
}

std::vector<size_t> charSums(const char* text, size_t textLen)
{
	return charSums<unsigned char>((const unsigned char*)text, textLen, 255);
}

std::vector<size_t> charSums(const unsigned char* text, size_t textLen)
{
	return charSums<unsigned char>((const unsigned char*)text, textLen, 255);
}

void bwt(const char* source, size_t sourceLen, char* dest)
{
	bwt<unsigned char>((const unsigned char*)source, sourceLen, 255, (unsigned char*)dest);
}

//do a counting sort on the rotated strings and pick the last element
std::vector<size_t> bwtDirectly(const std::vector<size_t>& data)
{
	auto max = std::max_element(data.begin(), data.end());
	assert(*max <= data.size());
	std::vector<size_t> location(*max, 0);
	for (size_t i = 0; i < data.size(); i++)
	{
		location[data[i]] = i+1; //use indexes that start from 1 so 0 is reserved as "unused"
	}
	std::vector<size_t> ret;
	for (auto i = location.begin(); i != location.end(); i++)
	{
		if (*i != 0)
		{
			size_t loc = *i-2;
			if (loc == 1)
			{
				loc = data.size()-1;
			}
			ret.push_back(data[loc]);
		}
	}
	return ret;
}

std::vector<size_t> step5(const std::vector<size_t>& Sprime)
{
	std::vector<bool> isUnique(Sprime.size(), true);
	bool canCalculateDirectly = true;
	for (auto i = Sprime.begin(); i != Sprime.end(); i++)
	{
		if (!isUnique[*i])
		{
			canCalculateDirectly = false;
			break;
		}
		isUnique[*i] = false;
	}
	if (canCalculateDirectly)
	{
		return bwtDirectly(Sprime);
	}
	return std::vector<size_t>();
//	return bwt(Sprime.data(), Sprime.size());
}

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R)
{
	std::vector<size_t> ret;
	for (auto i = BWTprime.begin(); i != BWTprime.end(); i++)
	{
		ret.push_back(R[*i]);
	}
	return ret;
}
