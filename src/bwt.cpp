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
