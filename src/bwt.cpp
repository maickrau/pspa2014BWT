#include <cassert>
#include <algorithm>

#include "bwt.h"

size_t nextTempFileName = 0;

std::string getTempFileName()
{
	std::string tryThis;
	bool usable;
	do
	{
		usable = true;
		tryThis = "tmp" + std::to_string(nextTempFileName) + ".tmp";
		nextTempFileName++;
		std::ifstream fileExists(tryThis);
		if (fileExists)
		{
			usable = false;
			fileExists.close();
			continue;
		}
		std::ofstream canBeCreated(tryThis);
		if (!canBeCreated)
		{
			usable = false;
		}
		else
		{
			canBeCreated.close();
		}
	} while (!usable);
	return tryThis;
}

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
	freeMemory(isUnique);
	if (canCalculateDirectly)
	{
		return bwtDirectly(Sprime);
	}
	std::vector<size_t> result(Sprime.size(), 0);
	auto max = std::max_element(Sprime.begin(), Sprime.end());
	bwt(Sprime.data(), Sprime.size(), *max, result.data());
	return result;
}

void step5InFile(const std::string& SprimeFile, const std::string& outFile, size_t SprimeSize)
{
	std::vector<bool> isUnique(SprimeSize, true);
	bool canCalculateDirectly = true;
	size_t index;
	std::ifstream file(SprimeFile, std::ios::binary);
	size_t max = 0;
	for (size_t i = 0; i < SprimeSize; i++)
	{
		file.read((char*)&index, sizeof(size_t));
		if (!isUnique[index])
		{
			canCalculateDirectly = false;
		}
		isUnique[index] = false;
		if (index > max)
		{
			max = index;
		}
	}
	file.close();
	freeMemory(isUnique);
	if (canCalculateDirectly)
	{
		std::vector<size_t> SprimeVec = readVectorFromFile<size_t>(SprimeFile, false);
		std::vector<size_t> result = bwtDirectly(SprimeVec);
		writeVectorToFile(result, outFile);
		return;
	}
	bwtInFiles<size_t>(SprimeFile, max, outFile, false);
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

void alternateStep6a(std::ostream& SAinverse, std::istream& BWTprime, size_t BWTprimeSize)
{
	std::vector<size_t> charNum(1, 0);
	size_t maxAlphabet = 0;
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		size_t read;
		BWTprime.read((char*)&read, sizeof(size_t));
		if (read > maxAlphabet)
		{
			maxAlphabet = read;
			charNum.resize(maxAlphabet+1);
		}
		charNum[read]++;
	}
	BWTprime.clear();
	BWTprime.seekg(0);
	std::vector<size_t> charSum(maxAlphabet+2, 0);
	for (size_t i = 1; i < maxAlphabet+1; i++)
	{
		charSum[i] = charSum[i-1]+charNum[i-1];
	}
	charSum[maxAlphabet+1] = charSum[maxAlphabet]+charNum[maxAlphabet];
	std::vector<size_t> LFinverse(BWTprimeSize, 0);
	std::vector<size_t> usedSlots(maxAlphabet+1, 0);
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		size_t read;
		BWTprime.read((char*)&read, sizeof(size_t));
		assert(usedSlots[read] < charSum[read+1]-charSum[read]);
		LFinverse[charSum[read]+usedSlots[read]] = i;
		usedSlots[read]++;
	}
	for (size_t i = 0; i <= maxAlphabet; i++)
	{
		assert(usedSlots[i] == charSum[i+1]-charSum[i]);
	}
	size_t index = 0;
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		SAinverse.write((char*)&LFinverse[index], sizeof(size_t));
		index = LFinverse[index];
	}
}

void inverseBWT(const char* source, size_t sourceLen, char* dest)
{
	inverseBWT<unsigned char>((const unsigned char*)source, sourceLen, 255, (unsigned char*)dest);
}
