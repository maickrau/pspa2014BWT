#include <cassert>
#include <algorithm>

#include "bwt.h"

std::vector<size_t> step1(const char* text, size_t textLen)
{
	return step1((const unsigned char*)text, textLen);
}

std::vector<size_t> step2(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step2((const unsigned char*)text, textLen, LMSLeft);
}

std::vector<size_t> step3(const char* text, size_t textLen, const std::vector<size_t>& LMSRight)
{
	return step3((const unsigned char*)text, textLen, LMSRight);
}

//find LMS-type suffixes in text, return a vector of their indices
std::vector<size_t> step1(const unsigned char* text, size_t textLen)
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
			buckets[text[i]].push_back(i);
		}
		if (text[i-1] != text[i])
		{
			equalIsSType = text[i-1] < text[i];
		}
		isSType = nextIsSType;
	}
	std::vector<size_t> ret; //A_lms, left in paper
	//indices were inserted in reverse order, reverse the vector to get them in right order
	for (int i = 0; i < 256; i++)
	{
		std::reverse(buckets[i].begin(), buckets[i].end());
		ret.insert(ret.end(), buckets[i].begin(), buckets[i].end());
	}
	return ret;
}

//does whatever, read the paper and go figure
std::vector<size_t> step2(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	std::vector<size_t> buckets[256]; //A_l in paper
	std::vector<size_t> ret; //A_lms,right in paper
	auto LMSPosition = LMSLeft.begin(); //LMSLeft is A_lms,left in paper
	assert(LMSPosition != LMSLeft.end());
	for (int bucket = 0; bucket < 256; bucket++)
	{
		while (LMSPosition != LMSLeft.end() && text[*LMSPosition] == bucket)
		{
			buckets[bucket].push_back(*LMSPosition);
			LMSPosition++;
		}
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1; //is this right?
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				ret.push_back(j);
			}
		}
	}
	return ret;
}

//same as step 2 in right-to-left order, see step 2 for documentation
std::vector<size_t> step3(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight)
{
	std::vector<size_t> buckets[256]; //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<size_t> ret; //A_lms,left in paper, built in reverse order
	auto LMSPosition = LMSRight.rbegin(); //note reverse, LMSRight is in proper order but we're travelling it in reverse
	for (int bucket = 255; bucket >= 0; bucket--)
	{
		while (LMSPosition != LMSRight.rend() && text[*LMSPosition] == bucket)
		{
			buckets[bucket].push_back(*LMSPosition);
			LMSPosition++;
		}
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1; //is this right?
			}
			assert(j <= textLen);
			if (text[jminus1] <= text[j])
			{
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				ret.push_back(j);
			}
		}
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
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
