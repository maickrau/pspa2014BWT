#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>
#include <utility>
#include <cassert>
#include <limits>
#include <iostream>

template <class Alphabet>
std::vector<size_t> charSums(const Alphabet* text, size_t textLen, size_t maxAlphabet)
{
	std::vector<size_t> sums(maxAlphabet+1, 0);
	for (size_t i = 0; i < textLen; i++)
	{
		assert(text[i] < maxAlphabet+1);
		sums[text[i]]++;
	}
	std::vector<size_t> res(maxAlphabet+2, 0);
	assert(maxAlphabet+2 < std::numeric_limits<int>::max());
	for (int i = 1; i < maxAlphabet+2; i++)
	{
		assert(res[i-1] < std::numeric_limits<size_t>::max()-sums[i-1]);
		res[i] = res[i-1]+sums[i-1];
	}
	return res;
}

//find LMS-type suffixes in text, return a vector of their indices
template <class Alphabet>
std::vector<size_t> step1(const Alphabet* text, size_t textLen, size_t maxAlphabet, std::vector<bool>& isS)
{
	assert(textLen > 0);
	std::vector<size_t> buckets[maxAlphabet+1];
	bool isSType = true; //last character is always s-type
	bool equalIsSType = false; //if text[i] == text[i-1], is text[i-1] s-type?
	//i > 0 because 0 can never be LMS-type
	for (size_t i = textLen-1; i > 0; i--)
	{
		isS[i] = isSType;
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
		isS[i-1] = isSType;
	}
	std::vector<size_t> ret; //A_lms, left in paper
	//indices were inserted in reverse order, reverse the vector to get them in right order
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	for (int i = 0; i < maxAlphabet+1; i++)
	{
		std::reverse(buckets[i].begin(), buckets[i].end());
		ret.insert(ret.end(), buckets[i].begin(), buckets[i].end());
	}
	return ret;
}

//sorts (with step 3) the LMS-type substrings
template <class Alphabet>
std::vector<size_t> step2(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSLeft)
{
	std::vector<size_t> bucketsS[maxAlphabet+1];
	std::vector<size_t> buckets[maxAlphabet+1]; //A_l in paper
	std::vector<size_t> ret; //A_lms,right in paper
	auto LMSPosition = LMSLeft.begin(); //LMSLeft is A_lms,left in paper
	assert(LMSPosition != LMSLeft.end());
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	while (LMSPosition != LMSLeft.end())
	{
		assert(*LMSPosition < textLen);
		assert(text[*LMSPosition] <= maxAlphabet);
		bucketsS[text[*LMSPosition]].push_back(*LMSPosition);
		LMSPosition++;
	}
	for (int bucket = 0; bucket < maxAlphabet+1; bucket++)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				ret.push_back(j);
			}
		}
		for (size_t i = 0; i < bucketsS[bucket].size(); i++)
		{
			size_t j = bucketsS[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				if (text[jminus1] == bucket)
				{
					std::cerr << text << "\n" << j << ", " << jminus1 << ", " << bucket << ", " << (int)text[jminus1] << "\n";
				}
				assert(text[jminus1] > bucket);
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				bucketsS[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(false);
				ret.push_back(j);
			}
		}
	}
	return ret;
}

//sorts (with step 2) the LMS-type substrings
template <class Alphabet>
std::vector<size_t> step3(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSRight)
{
	std::vector<size_t> bucketsL[maxAlphabet+1];
	std::vector<size_t> buckets[maxAlphabet+1]; //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<size_t> ret; //A_lms,left in paper, built in reverse order
	auto LMSPosition = LMSRight.rbegin(); //note reverse, LMSRight is in proper order but we're travelling it in reverse
	assert(maxAlphabet < std::numeric_limits<int>::max());
	while (LMSPosition != LMSRight.rend())
	{
		assert(*LMSPosition < textLen);
		assert(text[*LMSPosition] <= maxAlphabet);
		bucketsL[text[*LMSPosition]].push_back(*LMSPosition);
		LMSPosition++;
	}
	for (int bucket = maxAlphabet; bucket >= 0; bucket--)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			assert(text[j]+1 < maxAlphabet+1);
			if (text[jminus1] <= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(j != 0);
				ret.push_back(j);
			}
		}
		for (size_t i = 0; i < bucketsL[bucket].size(); i++)
		{
			size_t j = bucketsL[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] <= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				assert(text[jminus1] < bucket);
				buckets[text[jminus1]].push_back(jminus1);
				bucketsL[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(false);
				ret.push_back(j);
			}
		}	
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
}

template <class Alphabet>
int compareLMSSubstrings(const Alphabet* text, size_t textLen, size_t str1, size_t str2, const std::vector<bool>& LMSSubstringBorder, const std::vector<bool>& isSType)
{
	size_t start = str1;
	do
	{
		if (text[str1] > text[str2])
		{
			return 1;
		}
		if (text[str1] < text[str2])
		{
			return -1;
		}
		if (isSType[str1] && !isSType[str2])
		{
			return 1;
		}
		if (!isSType[str1] && isSType[str2])
		{
			return -1;
		}
		if (LMSSubstringBorder[str1] && !LMSSubstringBorder[str2])
		{
			return -1;
		}
		if (!LMSSubstringBorder[str1] && LMSSubstringBorder[str2])
		{
			return 1;
		}
		if (LMSSubstringBorder[str1] && LMSSubstringBorder[str2] && str1 != start)
		{
			return 0;
		}
		str1++;
		str2++;
		str1 %= textLen;
		str2 %= textLen;
	} while (str1 != start);
	assert(false);
}

template <class Alphabet>
int compareLMSSuffixes(const Alphabet* text, size_t textLen, size_t str1, size_t str2)
{
	do
	{
		if (text[str1] > text[str2])
		{
			return 1;
		}
		if (text[str1] < text[str2])
		{
			return -1;
		}
		str1++;
		str2++;
		if (str1 == textLen && str2 < textLen)
		{
			return -1;
		}
		if (str1 < textLen && str2 == textLen)
		{
			return 1;
		}
		if (str1 == textLen && str2 == textLen)
		{
			return 0;
		}
	} while (true);
	assert(false);
}

//return.first is S', return.second is R
template <class Alphabet>
std::pair<std::vector<size_t>, std::vector<size_t>> step4(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSLeft, const std::vector<bool>& isSType)
{
	std::pair<std::vector<size_t>, std::vector<size_t>> ret;
	std::vector<bool> LMSSubstringBorder(textLen, false);
	for (auto i = LMSLeft.begin(); i != LMSLeft.end(); i++)
	{
		assert(*i < textLen);
		LMSSubstringBorder[*i] = true;
	}
/*	std::vector<bool> differentThanLast(LMSLeft.size(), true); //B in paper
	for (size_t i = 1; i < LMSLeft.size(); i++)
	{
		differentThanLast[i] = compareLMSSubstrings(text, textLen, LMSLeft[i-1], LMSLeft[i], LMSSubstringBorder, isSType) != 0;
	}*/
	//construct R
	for (size_t i = 0; i < LMSLeft.size(); i++)
	{
//		if (differentThanLast[(i+1)%LMSLeft.size()])
//		{
			size_t pos = LMSLeft[i];
			do
			{
				pos++;
				pos %= textLen;
			} while (!LMSSubstringBorder[pos]);
			assert(pos != 0);
			ret.second.push_back(pos);
//		}
	}
	std::vector<size_t> sparseSPrime((textLen+1)/2, 0); //not sure if needs to round up, do it just in case
	size_t currentName = 0;
	for (size_t i = 0; i < LMSLeft.size(); i++)
	{
//		if (differentThanLast[(i)%LMSLeft.size()])
//		{
			currentName++;
//		}
		assert(LMSLeft[i]/2 < (textLen+1)/2);
		sparseSPrime[LMSLeft[i]/2] = currentName;
	}
	for (auto i = sparseSPrime.begin(); i != sparseSPrime.end(); i++)
	{
		if (*i != 0)
		{
			ret.first.push_back(*i-1);
		}
	}
	return ret;
}

std::vector<size_t> step5(const std::vector<size_t>& Sprime);

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R);

template <class Alphabet>
std::vector<size_t> step7(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSLeft, Alphabet* result, const std::vector<size_t>& charSums)
{
	assert(charSums.size() > maxAlphabet+1);
	std::vector<size_t> bucketsS[maxAlphabet+1];
	std::vector<size_t> buckets[maxAlphabet+1]; //A_l in paper
	std::vector<size_t> ret; //A_lms,right in paper
	auto LMSPosition = LMSLeft.begin(); //LMSLeft is A_lms,left in paper
	assert(LMSPosition != LMSLeft.end());
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	while (LMSPosition != LMSLeft.end())
	{
		bucketsS[text[*LMSPosition]].push_back(*LMSPosition);
		LMSPosition++;
	}
	for (int bucket = 0; bucket < maxAlphabet+1; bucket++)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(charSums[text[j]]+i < textLen);
			assert(i < charSums[text[j]+1]-charSums[text[j]]);
			result[charSums[text[j]]+i] = text[jminus1];
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				ret.push_back(j);
			}
		}
		for (size_t i = 0; i < bucketsS[bucket].size(); i++)
		{
			size_t j = bucketsS[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				assert(text[jminus1] > bucket);
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				bucketsS[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(false);
				ret.push_back(j);
			}
		}
	}
	return ret;
}

template <class Alphabet>
std::vector<size_t> step8(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSRight, Alphabet* result, const std::vector<size_t>& charSums)
{
	assert(charSums.size() > maxAlphabet+1);
	assert(charSums[maxAlphabet+1] == textLen);
	std::vector<size_t> bucketsL[maxAlphabet+1];
	std::vector<size_t> buckets[maxAlphabet+1]; //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<size_t> ret; //A_lms,left in paper, built in reverse order
	auto LMSPosition = LMSRight.rbegin(); //note reverse, LMSRight is in proper order but we're travelling it in reverse
	assert(maxAlphabet < std::numeric_limits<int>::max());
	while (LMSPosition != LMSRight.rend())
	{
		bucketsL[text[*LMSPosition]].push_back(*LMSPosition);
		LMSPosition++;
	}
	for (int bucket = maxAlphabet; bucket >= 0; bucket--)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			assert(text[j]+1 < maxAlphabet+1);
			assert(i < charSums[text[j]+1]);
			assert(i+1 <= charSums[text[j]+1]);
			assert(i+charSums[text[j]] < charSums[text[j]+1]);
			assert(charSums[text[j]+1]-i-1 < textLen);
			result[charSums[text[j]+1]-i-1] = text[jminus1];
			if (text[jminus1] <= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				buckets[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(j != 0);
				ret.push_back(j);
			}
		}
		for (size_t i = 0; i < bucketsL[bucket].size(); i++)
		{
			size_t j = bucketsL[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] <= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				assert(text[jminus1] < bucket);
				buckets[text[jminus1]].push_back(jminus1);
				bucketsL[bucket][i] = -1; //don't erase() because erase is O(n), just mark as unused
			}
			else
			{
				assert(false);
				ret.push_back(j);
			}
		}	
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
}

template <class Alphabet>
void verifyLMSSubstringsAreSorted(const Alphabet* source, size_t sourceLen, const std::vector<size_t>& LMSIndices, const std::vector<bool>& isSType)
{
	std::vector<bool> substringBorders(sourceLen, false);
	for (auto i = LMSIndices.begin(); i != LMSIndices.end(); i++)
	{
		substringBorders[*i] = true;
	}
	for (size_t i = 0; i < LMSIndices.size()-1; i++)
	{
		int diff = compareLMSSubstrings(source, sourceLen, LMSIndices[i], LMSIndices[i+1], substringBorders, isSType);
		if (diff > 0)
		{
			std::cerr << "substring " << i << "/" << LMSIndices.size() << " " << diff << "\n";
			std::cerr << LMSIndices[i] << " " << LMSIndices[i+1] << "\n";
		}
		assert(diff <= 0);
	}
}

template <class Alphabet>
void verifyLMSSuffixesAreSorted(const Alphabet* source, size_t sourceLen, const std::vector<size_t>& LMSIndices)
{
	for (size_t i = 0; i < LMSIndices.size()-1; i++)
	{
		int diff = compareLMSSuffixes(source, sourceLen, LMSIndices[i], LMSIndices[i+1]);
		if (diff > 0)
		{
			std::cerr << "\"" << source << "\":\n";
			for (size_t a = 0; a < LMSIndices.size(); a++)
			{
				std::cerr << LMSIndices[a] << " ";
			}
			std::cerr << "\nsuffix " << i << "/" << LMSIndices.size() << " " << diff << "\n";
			std::cerr << LMSIndices[i] << " " << LMSIndices[i+1] << "\n";
		}
		assert(diff <= 0);
	}
}

//maxAlphabet is the largest alphabet that appears in the source
//for unsigned char use maxAlphabet == 255
//for larger alphabets (eg. size_t), take the largest number that actually appears and use that
template <class Alphabet>
void bwt(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	std::vector<bool> isSType(sourceLen, false);
	auto charSum = charSums(source, sourceLen, maxAlphabet);
	auto first = step1(source, sourceLen, maxAlphabet, isSType);
	auto second = step2(source, sourceLen, maxAlphabet, first);
	auto third = step3(source, sourceLen, maxAlphabet, second);
	verifyLMSSubstringsAreSorted(source, sourceLen, third, isSType);
	auto fourth = step4(source, sourceLen, maxAlphabet, third, isSType);
	auto fifth = step5(fourth.first);
	auto sixth = step6(fifth, fourth.second);
	std::sort(sixth.begin(), sixth.end(), [&, source, sourceLen](size_t left, size_t right) -> bool { return compareLMSSuffixes(source, sourceLen, left, right) < 0; });
	verifyLMSSuffixesAreSorted(source, sourceLen, sixth);
	std::vector<bool> sevenWrote(sourceLen, false);
	std::vector<bool> eightWrote(sourceLen, false);
	auto seventh = step7(source, sourceLen, maxAlphabet, sixth, dest, charSum);
	step8(source, sourceLen, maxAlphabet, seventh, dest, charSum);
}

extern void bwt(const char* source, size_t sourceLen, char* dest);

template <class Alphabet>
void inverseBWT(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	auto charSum = charSums(source, sourceLen, maxAlphabet);
	std::vector<size_t> usedCharacters(maxAlphabet+1, 0);
	std::vector<size_t> LFmapping(sourceLen, 0);
	size_t index = 0;
	for (size_t i = 0; i < sourceLen; i++)
	{
		assert(source[i] < maxAlphabet+1);
		assert(charSum[source[i]] < std::numeric_limits<size_t>::max()-usedCharacters[source[i]]);
		LFmapping[i] = charSum[source[i]]+usedCharacters[source[i]];
		usedCharacters[source[i]]++;
		if (source[i] == 0)
		{
			assert(index == 0);
			index = i;
		}
	}
	size_t loc = sourceLen-1;
	do
	{
		assert(index < sourceLen);
		dest[loc] = source[index];
		index = LFmapping[index];
		loc--;
	} while (loc != 0);
	dest[loc] = source[index];
}

extern void inverseBWT(const char* source, size_t sourceLen, char* dest);

#endif