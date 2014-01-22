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

std::vector<size_t> step7(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft, char* result, const std::vector<size_t>& charSums)
{
	return step7((const unsigned char*)text, textLen, LMSLeft, (unsigned char*)result, charSums);
}

std::vector<size_t> step8(const char* text, size_t textLen, const std::vector<size_t>& LMSRight, char* result, const std::vector<size_t>& charSums)
{
	return step8((const unsigned char*)text, textLen, LMSRight, (unsigned char*)result, charSums);
}

std::vector<size_t> charSums(const char* text, size_t textLen)
{
	return charSums((const unsigned char*)text, textLen);
}

std::pair<std::vector<size_t>, std::vector<size_t>> step4(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	return step4((const unsigned char*)text, textLen, LMSLeft);
}

std::vector<size_t> charSums(const unsigned char* text, size_t textLen)
{
	std::vector<size_t> sums(256, 0);
	for (size_t i = 0; i < textLen; i++)
	{
		sums[text[i]]++;
	}
	std::vector<size_t> res(256, 0);
	for (int i = 1; i < 256; i++)
	{
		res[i] = res[i-1]+sums[i-1];
	}
	return res;
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
			buckets[text[*LMSPosition-1]].push_back(*LMSPosition-1);
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

//charSums is a vector that tells how many characters in text are less than that char
//eg. charSums['m'] == number of characters which are less than m
//amammmasasmasassaara\0 => charSums['m'] == 10, charSums['r'] == 15
std::vector<size_t> step7(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft, unsigned char* result, const std::vector<size_t>& charSums)
{
	std::vector<size_t> buckets[256]; //A_l in paper
	std::vector<size_t> ret; //A_lms,right in paper
	auto LMSPosition = LMSLeft.begin(); //LMSLeft is A_lms,left in paper
	assert(LMSPosition != LMSLeft.end());
	std::vector<size_t> numbersWritten(256, 0);
	for (int bucket = 0; bucket < 256; bucket++)
	{
		while (LMSPosition != LMSLeft.end() && text[*LMSPosition] == bucket)
		{
			size_t posminus1 = *LMSPosition-1;
			assert(*LMSPosition != 0); //first character can't be a LMS type suffix... right?
			buckets[text[posminus1]].push_back(posminus1);
			size_t charToWrite = posminus1-1;
			if (posminus1 == 0)
			{
				charToWrite = textLen;
			}
			result[charSums[text[posminus1]]+numbersWritten[text[posminus1]]] = text[charToWrite];
			numbersWritten[text[posminus1]]++;
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
				size_t charToWrite = jminus1-1;
				if (jminus1 == 0)
				{
					charToWrite = textLen;
				}
				result[charSums[text[jminus1]]+numbersWritten[text[jminus1]]] = text[charToWrite];
				numbersWritten[text[jminus1]]++;
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
			buckets[text[*LMSPosition-1]].push_back(*LMSPosition-1);
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

std::vector<size_t> step8(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight, unsigned char* result, const std::vector<size_t>& charSums)
{
	std::vector<size_t> buckets[256]; //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<size_t> ret; //A_lms,left in paper, built in reverse order
	auto LMSPosition = LMSRight.rbegin(); //note reverse, LMSRight is in proper order but we're travelling it in reverse
	std::vector<size_t> numbersWritten(256, 0);
	for (int bucket = 255; bucket >= 0; bucket--)
	{
		while (LMSPosition != LMSRight.rend() && text[*LMSPosition] == bucket)
		{
			size_t posminus1 = *LMSPosition-1;
			assert(*LMSPosition != 0); //first character can't be a LMS type suffix... right?
			buckets[text[posminus1]].push_back(posminus1);
			size_t charToWrite = posminus1-1;
			if (posminus1 == 0)
			{
				charToWrite = textLen;
			}
			result[charSums[text[posminus1]+1]-numbersWritten[text[posminus1]]-1] = text[charToWrite];
			numbersWritten[text[posminus1]]++;
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
				size_t charToWrite = jminus1-1;
				if (jminus1 == 0)
				{
					charToWrite = textLen;
				}
				result[charSums[text[jminus1]+1]-numbersWritten[text[jminus1]]-1] = text[charToWrite];
				numbersWritten[text[jminus1]]++;
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

bool LMSSubstringsAreEqual(const unsigned char* text, size_t textLen, size_t str1, size_t str2, const std::vector<bool>& LMSSubstringBorder)
{
	size_t start = str1;
	do
	{
		str1++;
		str2++;
		str1 %= textLen;
		str2 %= textLen;
		if (LMSSubstringBorder[str1] ^ LMSSubstringBorder[str2])
		{
			return false;
		}
		if (LMSSubstringBorder[str1] && LMSSubstringBorder[str2])
		{
			return true;
		}
		if (text[str1] != text[str2])
		{
			return false;
		}
	} while (str1 != start);
	assert(false);
}

//return.first is S', return.second is R
std::pair<std::vector<size_t>, std::vector<size_t>> step4(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft)
{
	std::pair<std::vector<size_t>, std::vector<size_t>> ret;
	std::vector<bool> LMSSubstringBorder(textLen, false);
	for (auto i = LMSLeft.begin(); i != LMSLeft.end(); i++)
	{
		LMSSubstringBorder[*i] = true;
	}
	std::vector<bool> differentThanLast(LMSLeft.size(), true); //B in paper
	for (size_t i = 1; i < LMSLeft.size(); i++)
	{
		differentThanLast[i] = !LMSSubstringsAreEqual(text, textLen, LMSLeft[i-1], LMSLeft[i], LMSSubstringBorder);
	}
	//construct R
	for (size_t i = 0; i < LMSLeft.size(); i++)
	{
		if (differentThanLast[(i+1)%LMSLeft.size()])
		{
			size_t pos = LMSLeft[i];
			do
			{
				pos++;
				pos %= textLen;
			} while (!LMSSubstringBorder[pos]);
			assert(pos != 0);
			ret.second.push_back(pos);
		}
	}
	std::vector<size_t> sparseSPrime((textLen+1)/2, 0); //not sure if needs to round up, do it just in case
	size_t currentName = 0;
	for (size_t i = 0; i < LMSLeft.size(); i++)
	{
		if (differentThanLast[i])
		{
			currentName++;
		}
		sparseSPrime[LMSLeft[i]/2] = currentName;
	}
	for (auto i = sparseSPrime.begin(); i != sparseSPrime.end(); i++)
	{
		if (*i != 0)
		{
			ret.first.push_back(*i);
		}
	}
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
