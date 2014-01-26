#include <cassert>
#include <algorithm>

#include "bwt.h"

void bwt(const char* source, size_t sourceLen, char* dest)
{
	bwt<unsigned char>((const unsigned char*)source, sourceLen, 255, (unsigned char*)dest);
}

//do a counting sort on the rotated strings and pick the last element
std::vector<size_t> bwtDirectly(const std::vector<size_t>& data)
{
	auto max = std::max_element(data.begin(), data.end());
	assert(*max == data.size()-1);
	std::vector<size_t> location(*max+1, 0);
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
			if (*i == 1)
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
	assert(false); //modification to the algorithm, all names are unique so can always calculate directly
	size_t* rawResult = new size_t[Sprime.size()]();
	auto max = std::max_element(Sprime.begin(), Sprime.end());
	bwt(Sprime.data(), Sprime.size(), *max, rawResult);
	std::vector<size_t> result;
	for (size_t i = 0; i < Sprime.size(); i++)
	{
		result.push_back(rawResult[i]);
	}
	delete [] rawResult;
	return result;
}

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R)
{
	std::vector<size_t> ret;
	for (auto i = BWTprime.begin(); i != BWTprime.end(); i++)
	{
		assert(*i < R.size());
		ret.push_back(R[*i]);
	}
	return ret;
}

void inverseBWT(const char* source, size_t sourceLen, char* dest)
{
	inverseBWT<unsigned char>((const unsigned char*)source, sourceLen, 255, (unsigned char*)dest);
}
