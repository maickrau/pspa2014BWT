#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>
#include <utility>
#include <cassert>
#include <limits>
#include <iostream>

template <class O>
void freeMemory(O& o)
{
	O().swap(o);
}

template <class Alphabet>
std::vector<size_t> charSums(const Alphabet* text, size_t textLen, size_t maxAlphabet)
{
	std::vector<size_t> sums(maxAlphabet+1, 0);
	for (size_t i = 0; i < textLen; i++)
	{
		assert(text[i] <= maxAlphabet);
		sums[text[i]]++;
	}
	std::vector<size_t> ret(maxAlphabet+2, 0);
	for (size_t i = 1; i < maxAlphabet+2; i++)
	{
		ret[i] = ret[i-1]+sums[i-1];
	}
	return ret;
}

template <class Alphabet>
std::tuple<std::vector<size_t>, //A_LMS,left
           std::vector<size_t>, //charsums
           std::vector<size_t>, //L-counts
           std::vector<size_t>  //LMS indices in order they appear in text
#ifndef NDEBUG
           ,std::vector<bool> //is S-type
#endif
           > 
preprocess(const Alphabet* text, size_t textLen, size_t maxAlphabet)
{
	std::tuple<std::vector<size_t>, 
	           std::vector<size_t>, 
	           std::vector<size_t>, 
	           std::vector<size_t> 
#ifndef NDEBUG
	           ,std::vector<bool>
#endif
	           > ret;
	std::get<1>(ret).resize(maxAlphabet+2, 0);
	std::get<2>(ret).resize(maxAlphabet+1, 0);
#ifndef NDEBUG
	std::get<4>(ret).resize(textLen, false);
#endif
	std::vector<size_t> sums(maxAlphabet+1, 0);
	std::vector<std::vector<size_t>> buckets(maxAlphabet+1);
	bool isSType = true; //last character is always s-type
	bool equalIsSType = false; //if text[i] == text[i-1], is text[i-1] s-type?
	//i > 0 because 0 can never be LMS-type
	for (size_t i = textLen-1; i > 0; i--)
	{
		assert(text[i] <= maxAlphabet);
		sums[text[i]]++;
#ifndef NDEBUG
		std::get<4>(ret)[i] = isSType;
#endif
		if (!isSType)
		{
			std::get<2>(ret)[text[i]]++;
		}
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
			std::get<3>(ret).push_back(i);
		}
		if (text[i-1] != text[i])
		{
			equalIsSType = text[i-1] < text[i];
		}
		isSType = nextIsSType;
#ifndef NDEBUG
		std::get<4>(ret)[i-1] = isSType;
#endif
	}
	sums[text[0]]++;
	//indices were inserted in reverse order, reverse the vector to get them in right order
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	for (int i = 0; i < maxAlphabet+1; i++)
	{
		if (i > 0)
		{
			std::get<1>(ret)[i] = std::get<1>(ret)[i-1]+sums[i-1];
		}
		std::reverse(buckets[i].begin(), buckets[i].end());
		std::get<0>(ret).insert(std::get<0>(ret).end(), buckets[i].begin(), buckets[i].end());
		freeMemory(buckets[i]);
	}
	std::get<1>(ret)[maxAlphabet+1] = std::get<1>(ret)[maxAlphabet]+sums[maxAlphabet];
	return ret;
}

//sorts (with step 3) the LMS-type substrings
//call with result == nullptr to do step 2, otherwise step 7
template <class Alphabet>
std::vector<size_t> step2or7(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSLeft, Alphabet* result, const std::vector<size_t>& charSum, const std::vector<size_t>& Lsum)
{
	std::vector<std::vector<size_t>> bucketsS(maxAlphabet+1);
	std::vector<std::vector<size_t>> buckets(maxAlphabet+1); //A_l in paper
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		buckets[i].reserve(Lsum[i]);
	}
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
			if (result != nullptr)
			{
				assert(charSum[text[j]]+i < textLen);
				assert(i < charSum[text[j]+1]-charSum[text[j]]);
				result[charSum[text[j]]+i] = text[jminus1];
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
//call with result == nullptr to do step 3, otherwise 8
template <class Alphabet>
std::vector<size_t> step3or8(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSRight, Alphabet* result, const std::vector<size_t>& charSum, const std::vector<size_t>& Lsum)
{
	std::vector<std::vector<size_t>> bucketsL(maxAlphabet+1);
	std::vector<std::vector<size_t>> buckets(maxAlphabet+1); //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		buckets[i].reserve(charSum[i+1]-charSum[i]-Lsum[i]);
	}
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
			if (result != nullptr)
			{
				assert(i < charSum[text[j]+1]);
				assert(i+1 <= charSum[text[j]+1]);
				assert(i+charSum[text[j]] < charSum[text[j]+1]);
				assert(charSum[text[j]+1]-i-1 < textLen);
				result[charSum[text[j]+1]-i-1] = text[jminus1];
			}
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
bool LMSSubstringsAreEqual(const Alphabet* text, size_t textLen, size_t str1, size_t str2, const std::vector<bool>& LMSSubstringBorder)
{
	if (text[str1] != text[str2])
	{
		return false;
	}
	if (LMSSubstringBorder[str1] ^ LMSSubstringBorder[str2])
	{
		return false;
	}
	str1++;
	str2++;
	str1 %= textLen;
	str2 %= textLen;
	while (true)
	{
		if (text[str1] != text[str2])
		{
			return false;
		}
		if (LMSSubstringBorder[str1] ^ LMSSubstringBorder[str2])
		{
			return false;
		}
		if (LMSSubstringBorder[str1] && LMSSubstringBorder[str2])
		{
			return true;
		}
		str1++;
		str2++;
		str1 %= textLen;
		str2 %= textLen;
	}
	assert(false);
}

//used only in debugging to check that the LMS-substrings are ordered correctly
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

//returns S'
template <class Alphabet>
std::vector<size_t> step4(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& LMSLeft)
{
	std::vector<size_t> ret;
	std::vector<bool> LMSSubstringBorder(textLen, false);
	for (auto i = LMSLeft.begin(); i != LMSLeft.end(); i++)
	{
		assert(*i < textLen);
		LMSSubstringBorder[*i] = true;
	}
	std::vector<size_t> sparseSPrime((textLen+1)/2, 0); //not sure if needs to round up, do it just in case
	size_t currentName = 0;
	for (size_t i = 0; i < LMSLeft.size(); i++)
	{
		if (i == 0 || !LMSSubstringsAreEqual(text, textLen, LMSLeft[i-1], LMSLeft[i], LMSSubstringBorder))
		{
			currentName++;
		}
		assert(LMSLeft[i]/2 < (textLen+1)/2);
		sparseSPrime[LMSLeft[i]/2] = currentName;
	}
	for (auto i = sparseSPrime.begin(); i != sparseSPrime.end(); i++)
	{
		if (*i != 0)
		{
			ret.push_back(*i-1);
		}
	}
	return ret;
}

std::vector<size_t> step5(const std::vector<size_t>& Sprime);

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R);

std::vector<size_t> alternateStep6a(const std::vector<size_t>& BWTprime);

template <class Alphabet>
std::vector<size_t> alternateStep6b(const Alphabet* text, size_t textLen, size_t maxAlphabet, const std::vector<size_t>& SAinverse, const std::vector<size_t>& LMSIndices)
{
	assert(textLen > 0);
	assert(SAinverse.size() == LMSIndices.size());
	std::vector<size_t> ret(SAinverse.size(), 0);
	for (size_t i = 0; i < SAinverse.size(); i++)
	{
		ret[SAinverse[i]] = LMSIndices[SAinverse.size()-i-1];
	}
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
	auto prep = preprocess(source, sourceLen, maxAlphabet);
	auto second = step2or7(source, sourceLen, maxAlphabet, std::get<0>(prep), (Alphabet*)nullptr, std::get<1>(prep), std::get<2>(prep));
	freeMemory(std::get<0>(prep));
	auto third = step3or8(source, sourceLen, maxAlphabet, second, (Alphabet*)nullptr, std::get<1>(prep), std::get<2>(prep));
	freeMemory(second);
#ifndef NDEBUG
	verifyLMSSubstringsAreSorted(source, sourceLen, third, std::get<4>(prep));
#endif
	auto fourth = step4(source, sourceLen, maxAlphabet, third);
	freeMemory(third);
	auto fifth = step5(fourth);
	auto SAinverse = alternateStep6a(fifth);
	freeMemory(fifth);
	auto sixth = alternateStep6b(source, sourceLen, maxAlphabet, SAinverse, std::get<3>(prep));
	freeMemory(std::get<3>(prep));
	freeMemory(SAinverse);
#ifndef NDEBUG
	verifyLMSSuffixesAreSorted(source, sourceLen, sixth);
#endif
	auto seventh = step2or7(source, sourceLen, maxAlphabet, sixth, dest, std::get<1>(prep), std::get<2>(prep));
	freeMemory(sixth);
	step3or8(source, sourceLen, maxAlphabet, seventh, dest, std::get<1>(prep), std::get<2>(prep));
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