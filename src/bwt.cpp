#include <cassert>
#include <algorithm>

#include "bwt.h"

//find LMS-type suffixes in text, return a vector of their indices
std::vector<size_t> step1(const char* text, size_t textLen)
{
	assert(textLen > 0);
	std::vector<size_t> buckets[256];
	bool isSType = true; //last character is always s-type
	bool equalIsSType = false; //if text[i] == text[i-1], is text[i-1] s-type?
	//i > 0 because 0 can never be LMS-type
	for (size_t i = textLen-1; i > 0; i--)
	{
		bool nextIsSType;
		if (text[i-1] < text[i])
		{
			nextIsSType = true;
		}
		else if (text[i-1] > text[i])
		{
			nextIsSType = false;
		}
		else
		{
			nextIsSType = equalIsSType;
		}
		if (isSType && !nextIsSType)
		{
			buckets[(unsigned char)text[i]].push_back(i);
		}
		if (text[i-1] != text[i])
		{
			equalIsSType = text[i-1] < text[i];
		}
		isSType = nextIsSType;
	}
	std::vector<size_t> ret;
	//indices were inserted in reverse order, reverse the vector to get them in right order
	for (int i = 0; i < 256; i++)
	{
		std::reverse(buckets[i].begin(), buckets[i].end());
		ret.insert(ret.end(), buckets[i].begin(), buckets[i].end());
	}
	return ret;
}