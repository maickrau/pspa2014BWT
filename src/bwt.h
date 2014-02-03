#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>
#include <utility>
#include <cassert>
#include <limits>
#include <ios>
#include <iostream>
#include <streambuf>
#include <fstream>

std::string getTempFileName();

template <class Type>
std::vector<Type> readVectorFromFile(const std::string& sourceFile, bool addSentinel)
{
	std::ifstream file(sourceFile, std::ios::binary);
	file.seekg(0, std::ios::end);
	assert(file.tellg() % sizeof(Type) == 0);
	size_t fileLen = file.tellg()/sizeof(Type);
	file.clear();
	file.seekg(0, std::ios::beg);
	if (addSentinel)
	{
		fileLen++;
	}
	std::vector<Type> ret(fileLen, 0);
	if (addSentinel)
	{
		fileLen--;
	}
	file.read((char*)ret.data(), fileLen*sizeof(Type));
	file.close();
	return ret;
}

template <class Type>
void writeVectorToFile(const std::vector<Type> vec, const std::string& destFile)
{
	std::ofstream file(destFile, std::ios::binary);
	file.write((char*)vec.data(), vec.size()*sizeof(Type));
	file.close();
}

template <class Alphabet>
size_t getFileLengthInAlphabets(const std::string& sourceFile)
{
	std::ifstream file(sourceFile, std::ios::binary);
	file.seekg(0, std::ios::end);
	assert(file.tellg() % sizeof(Alphabet) == 0);
	size_t fileLen = file.tellg()/sizeof(Alphabet);
	file.close();
	return fileLen;
}

template <class O>
void freeMemory(O& o)
{
	O().swap(o);
}

template <class Alphabet>
class MemoryStreambuffer : public std::streambuf
{
public:
	MemoryStreambuffer(Alphabet* memory, size_t size) : std::streambuf() 
	{ 
		this->setg((char*)memory, (char*)memory, (char*)(memory+size)); 
		this->setp((char*)memory, (char*)(memory+size)); 
	};
protected:
	std::streampos seekpos(std::streampos sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
	{
		this->setg(eback(), eback()+sp, egptr());
		return sp;
	}
};

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

//returns random access written results in the tuple, and sequential access written results with streams
template <class Alphabet>
std::tuple<std::vector<size_t>, //L-counts
           size_t               //count of LMS-type indices
#ifndef NDEBUG
           ,std::vector<bool> //is S-type
#endif
           > 
preprocess(std::istream& text, size_t textLen, size_t maxAlphabet, std::ostream& LMSLeftOut, std::ostream& charSumOut, std::ostream& LMSIndicesOut, bool addSentinel)
{
	std::tuple<std::vector<size_t>, 
	           size_t
#ifndef NDEBUG
	           ,std::vector<bool>
#endif
	           > ret;
	std::get<0>(ret).resize(maxAlphabet+1, 0);
	std::get<1>(ret) = 0;
#ifndef NDEBUG
	std::get<2>(ret).resize(textLen, false);
#endif
	std::vector<size_t> sums(maxAlphabet+1, 0);
	std::vector<std::vector<size_t>> buckets(maxAlphabet+1);
	size_t lastPossibleLMS = 0;
	size_t lastCharacterBoundary = 0;
	bool lastWasLType = false;
	Alphabet currentSymbol;
	Alphabet nextSymbol;
	text.read((char*)&nextSymbol, sizeof(Alphabet));
	for (size_t i = 0; i < textLen-1; i++)
	{
		currentSymbol = nextSymbol;
		if (addSentinel && i == textLen-2)
		{
			nextSymbol = 0;
		}
		else
		{
			text.read((char*)&nextSymbol, sizeof(Alphabet));
		}
		assert(currentSymbol <= maxAlphabet);
		sums[currentSymbol]++;
		if (currentSymbol > nextSymbol)
		{
			//L-type, and all in [lastCharacterBoundary, i] are L-type of the same letter
			assert(i >= lastCharacterBoundary);
			std::get<0>(ret)[currentSymbol] += (i-lastCharacterBoundary)+1;
			lastPossibleLMS = i+1;
			lastCharacterBoundary = i+1;
			lastWasLType = true;
		}
		else if (currentSymbol < nextSymbol)
		{
			//S-type, and all in [lastCharacterBoundary, i] are S-type of the same letter
#ifndef NDEBUG
			for (size_t a = lastCharacterBoundary; a <= i; a++)
			{
				std::get<2>(ret)[a] = true;
			}
#endif
			if (lastWasLType)
			{
				buckets[currentSymbol].push_back(lastPossibleLMS);
				LMSIndicesOut.write((char*)&lastPossibleLMS, sizeof(size_t));
				assert(std::get<1>(ret) < textLen/2);
				std::get<1>(ret)++;
			}
			lastWasLType = false;
			lastCharacterBoundary = i+1;
		}
		else
		{
			assert(currentSymbol == nextSymbol);
		}
	}
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		assert(std::get<0>(ret)[i] <= sums[i]);
	}
#ifndef NDEBUG
	std::get<2>(ret)[textLen-1] = true;
#endif
	sums[nextSymbol]++;
	buckets[nextSymbol].push_back(textLen-1);
	size_t lastIndex = textLen-1;
	LMSIndicesOut.write((char*)&lastIndex, sizeof(size_t));
	assert(std::get<1>(ret) < textLen/2);
	std::get<1>(ret)++;

	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	size_t charSumSum = 0;
	charSumOut.write((char*)&charSumSum, sizeof(size_t));
	for (int i = 0; i < maxAlphabet+1; i++)
	{
		charSumSum = charSumSum+sums[i];
		charSumOut.write((char*)&charSumSum, sizeof(size_t));
		LMSLeftOut.write((char*)buckets[i].data(), buckets[i].size()*sizeof(size_t));
		freeMemory(buckets[i]);
	}
	return ret;
}

//sorts (with step 3) the LMS-type substrings
//call with result == nullptr to do step 2, otherwise step 7
template <class Alphabet, bool isStep7>
void step2or7(const Alphabet* const text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize, Alphabet* const result, const std::vector<size_t>& charSum, const std::vector<size_t>& Lsum)
{
	if (isStep7)
	{
		assert(result != nullptr);
	}
	std::vector<std::vector<size_t>> bucketsS(maxAlphabet+1);
	std::vector<size_t*> buckets(maxAlphabet+1, nullptr); //A_l in paper
	std::vector<size_t> bucketsSize(maxAlphabet+1, 0);
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		buckets[i] = new size_t[Lsum[i]];
	}
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		size_t index;
		LMSLeft.read((char*)&index, sizeof(size_t));
		bucketsS[text[index]].push_back(index);
	}
	for (int bucket = 0; bucket < maxAlphabet+1; bucket++)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < bucketsSize[bucket]; i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			if (isStep7)
			{
				assert(charSum[text[j]]+i < textLen);
				assert(i < charSum[text[j]+1]-charSum[text[j]]);
				result[charSum[text[j]]+i] = text[jminus1];
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]][bucketsSize[text[jminus1]]] = jminus1;
				bucketsSize[text[jminus1]]++;
			}
			else
			{
				out.write((char*)&j, sizeof(size_t));
			}
		}
		delete [] buckets[bucket];
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
				buckets[text[jminus1]][bucketsSize[text[jminus1]]] = jminus1;
				bucketsSize[text[jminus1]]++;
			}
			else
			{
				assert(false);
			}
		}
		freeMemory(bucketsS[bucket]);
	}
}

//sorts (with step 2) the LMS-type substrings
//call with result == nullptr to do step 3, otherwise 8
template <class Alphabet, bool isStep8>
void step3or8(const Alphabet* const text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSRight, size_t LMSRightSize, Alphabet* const result, const std::vector<size_t>& charSum, const std::vector<size_t>& Lsum)
{
	if (isStep8)
	{
		assert(result != nullptr);
	}
	std::vector<std::vector<size_t>> bucketsL(maxAlphabet+1);
	std::vector<size_t*> buckets(maxAlphabet+1, nullptr); //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<size_t> bucketsSize(maxAlphabet+1, 0);
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		buckets[i] = new size_t[charSum[i+1]-charSum[i]-Lsum[i]];
	}
	assert(maxAlphabet < std::numeric_limits<int>::max());
	for (size_t i = 0; i < LMSRightSize; i++)
	{
		size_t index;
		LMSRight.read((char*)&index, sizeof(size_t));
		bucketsL[text[index]].push_back(index);
	}
	for (int bucket = maxAlphabet; bucket >= 0; bucket--)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < bucketsSize[bucket]; i++)
		{
			size_t j = buckets[bucket][i];
			size_t jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			assert(text[j]+1 < maxAlphabet+1);
			if (isStep8)
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
				buckets[text[jminus1]][bucketsSize[text[jminus1]]] = jminus1;
				bucketsSize[text[jminus1]]++;
			}
			else
			{
				assert(j != 0);
				if (!isStep8) //don't return anything for step 8
				{
					out.write((char*)&j, sizeof(size_t));
				}
			}
		}
		delete [] buckets[bucket];
		if (bucketsL[bucket].size() > 0)
		{
			for (size_t i = bucketsL[bucket].size()-1; ; i--)
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
					buckets[text[jminus1]][bucketsSize[text[jminus1]]] = jminus1;
					bucketsSize[text[jminus1]]++;
				}
				else
				{
					assert(false);
				}
				if (i == 0)
				{
					break;
				}
			}
			freeMemory(bucketsL[bucket]);
		}
	}
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
void step4(const Alphabet* text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize)
{
	std::vector<bool> LMSSubstringBorder(textLen, false);
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		size_t index;
		assert(LMSLeft.good());
		LMSLeft.read((char*)&index, sizeof(size_t));
		LMSSubstringBorder[index] = true;
	}
	LMSLeft.clear();
	LMSLeft.seekg(0);
	assert(LMSLeft.good());
	std::vector<size_t> sparseSPrime((textLen+1)/2, 0); //not sure if needs to round up, do it just in case
	size_t currentName = LMSLeftSize+1;
	size_t oldIndex = 0;
	size_t index = 0;
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		assert(LMSLeft.good());
		LMSLeft.read((char*)&index, sizeof(size_t));
		if (i == 0 || !LMSSubstringsAreEqual(text, textLen, oldIndex, index, LMSSubstringBorder))
		{
			currentName--;
		}
		assert(index/2 < (textLen+1)/2);
		sparseSPrime[index/2] = currentName;
		assert(currentName > 0);
		oldIndex = index;
	}
	for (auto i = sparseSPrime.begin(); i != sparseSPrime.end(); i++)
	{
		if (*i != 0)
		{
			assert(*i >= currentName);
			size_t write = *i-currentName;
			assert(write < LMSLeftSize);
			out.write((char*)&write, sizeof(size_t));
		}
	}
}

std::vector<size_t> step5(const std::vector<size_t>& Sprime);
void step5InFile(const std::string& SprimeFile, const std::string& outFile, size_t SprimeSize);

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R);

void alternateStep6a(std::ostream& SAinverse, std::istream& BWTprime, size_t BWTprimeSize);

template <class Alphabet>
std::vector<size_t> alternateStep6b(std::istream& SAinverse, std::istream& LMSIndices, size_t size)
{
	std::vector<size_t> ret(size, 0);
	for (size_t i = 0; i < size; i++)
	{
		size_t SA;
		SAinverse.read((char*)&SA, sizeof(size_t));
		size_t index;
		LMSIndices.read((char*)&index, sizeof(size_t));
		ret[SA] = index;
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
	for (size_t i = LMSIndices.size()-1; i > 0; i--)
	{
		int diff = compareLMSSubstrings(source, sourceLen, LMSIndices[i], LMSIndices[i-1], substringBorders, isSType);
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
	std::vector<size_t> LMSLeft(sourceLen/2, 0);
	std::vector<size_t> charSum(maxAlphabet+2, 0);
	std::vector<size_t> LMSIndices(sourceLen/2, 0);

	MemoryStreambuffer<Alphabet> sourceBuf((Alphabet*)source, sourceLen);
	MemoryStreambuffer<size_t> LMSLeftBuf(LMSLeft.data(), sourceLen/2);
	MemoryStreambuffer<size_t> charSumBuf(charSum.data(), maxAlphabet+2);
	MemoryStreambuffer<size_t> LMSIndicesBuf(LMSIndices.data(), sourceLen/2);

	std::istream sourceReader(&sourceBuf);
	std::istream LMSLeftReader(&LMSLeftBuf);
//	std::istream charSumReader(&charSumBuf);
	std::istream LMSIndicesReader(&LMSIndicesBuf);

	std::ostream LMSLeftWriter(&LMSLeftBuf);
	std::ostream charSumWriter(&charSumBuf);
	std::ostream LMSIndicesWriter(&LMSIndicesBuf);

	auto prep = preprocess<Alphabet>(sourceReader, sourceLen, maxAlphabet, LMSLeftWriter, charSumWriter, LMSIndicesWriter, false);

	assert(std::get<1>(prep) <= sourceLen/2);
	LMSLeft.resize(std::get<1>(prep));
	LMSIndices.resize(std::get<1>(prep));

	std::vector<size_t> second(std::get<1>(prep), 0);
	MemoryStreambuffer<size_t> secondBuf(second.data(), std::get<1>(prep));
	std::ostream secondWriter(&secondBuf);
	std::istream secondReader(&secondBuf);

	step2or7<Alphabet, false>(source, sourceLen, maxAlphabet, secondWriter, LMSLeftReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	freeMemory(LMSLeft);

	std::vector<size_t> third(std::get<1>(prep), 0);
	MemoryStreambuffer<size_t> thirdBuf(third.data(), std::get<1>(prep));
	std::ostream thirdWriter(&thirdBuf);
	std::istream thirdReader(&thirdBuf);

	step3or8<Alphabet, false>(source, sourceLen, maxAlphabet, thirdWriter, secondReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	freeMemory(second);
#ifndef NDEBUG
	verifyLMSSubstringsAreSorted(source, sourceLen, third, std::get<2>(prep));
#endif

	std::vector<size_t> fourth(std::get<1>(prep), 0);
	MemoryStreambuffer<size_t> fourthBuf(fourth.data(), std::get<1>(prep));
	std::ostream fourthWriter(&fourthBuf);

	step4(source, sourceLen, maxAlphabet, fourthWriter, thirdReader, std::get<1>(prep));
	freeMemory(third);
	auto fifth = step5(fourth);

	MemoryStreambuffer<size_t> fifthBuf(fifth.data(), std::get<1>(prep));
	std::istream fifthReader(&fifthBuf);
	std::vector<size_t> SAinverse(std::get<1>(prep), 0);
	MemoryStreambuffer<size_t> SAinverseBuf(SAinverse.data(), std::get<1>(prep));
	std::ostream SAinverseWriter(&SAinverseBuf);
	std::istream SAinverseReader(&SAinverseBuf);

	alternateStep6a(SAinverseWriter, fifthReader, std::get<1>(prep));
	freeMemory(fifth);
	auto sixth = alternateStep6b<size_t>(SAinverseReader, LMSIndicesReader, std::get<1>(prep));
	freeMemory(LMSIndices);
	freeMemory(SAinverse);
#ifndef NDEBUG
	verifyLMSSuffixesAreSorted(source, sourceLen, sixth);
#endif
	MemoryStreambuffer<size_t> sixthBuf(sixth.data(), sixth.size());
	std::istream sixthReader(&sixthBuf);

	std::vector<size_t> seventh(std::get<1>(prep), 0);
	MemoryStreambuffer<size_t> seventhBuf(seventh.data(), std::get<1>(prep));
	std::ostream seventhWriter(&seventhBuf);
	std::istream seventhReader(&seventhBuf);

	step2or7<Alphabet, true>(source, sourceLen, maxAlphabet, seventhWriter, sixthReader, sixth.size(), dest, charSum, std::get<0>(prep));
	freeMemory(sixth);
	std::ofstream dummyStream;
	step3or8<Alphabet, true>(source, sourceLen, maxAlphabet, dummyStream, seventhReader, std::get<1>(prep), dest, charSum, std::get<0>(prep));
}

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t maxAlphabet, const std::string& destFile, bool addSentinel)
{
	size_t sourceLen = getFileLengthInAlphabets<Alphabet>(sourceFile);
	if (addSentinel)
	{
		sourceLen++;
	}
	
	std::string LMSLeftFile = getTempFileName();
	std::string charSumFile = getTempFileName();
	std::string LMSIndicesFile = getTempFileName();
	std::string LCountFile = getTempFileName();
	std::string secondFile = getTempFileName();
	std::string thirdFile = getTempFileName();
	std::string fourthFile = getTempFileName();
	std::string fifthFile = getTempFileName();
	std::string SAinverseFile = getTempFileName();
	std::string sixthFile = getTempFileName();
	std::string seventhFile = getTempFileName();

	std::ofstream LMSLeftWriter(LMSLeftFile, std::ios::binary);
	std::ofstream charSumWriter(charSumFile, std::ios::binary);
	std::ofstream LMSIndicesWriter(LMSIndicesFile, std::ios::binary);
	std::ifstream sourceReader(sourceFile, std::ios::binary);

	auto prep = preprocess<Alphabet>(sourceReader, sourceLen, maxAlphabet, LMSLeftWriter, charSumWriter, LMSIndicesWriter, addSentinel);
	LMSLeftWriter.close();
	charSumWriter.close();
	LMSIndicesWriter.close();
	sourceReader.close();

	assert(std::get<1>(prep) <= sourceLen/2);

	std::vector<size_t> charSum = readVectorFromFile<size_t>(charSumFile, false);
	std::vector<Alphabet> source;
	source = readVectorFromFile<Alphabet>(sourceFile, addSentinel);

	std::ofstream secondWriter(secondFile, std::ios::binary);
	std::ifstream LMSLeftReader(LMSLeftFile, std::ios::binary);

	step2or7<Alphabet, false>(source.data(), sourceLen, maxAlphabet, secondWriter, LMSLeftReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	secondWriter.close();
	LMSLeftReader.close();

	std::ofstream thirdWriter(thirdFile, std::ios::binary);
	std::ifstream secondReader(secondFile, std::ios::binary);

	step3or8<Alphabet, false>(source.data(), sourceLen, maxAlphabet, thirdWriter, secondReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	thirdWriter.close();
	secondReader.close();

#ifndef NDEBUG
	std::vector<size_t> third = readVectorFromFile<size_t>(thirdFile, false);
	verifyLMSSubstringsAreSorted(source.data(), sourceLen, third, std::get<2>(prep));
	freeMemory(third);
#endif

	std::ofstream fourthWriter(fourthFile, std::ios::binary);
	std::ifstream thirdReader(thirdFile, std::ios::binary);

	step4(source.data(), sourceLen, maxAlphabet, fourthWriter, thirdReader, std::get<1>(prep));
	fourthWriter.close();
	thirdReader.close();

	freeMemory(source);

	step5InFile(fourthFile, fifthFile, std::get<1>(prep));

	std::ofstream SAinverseWriter(SAinverseFile, std::ios::binary);
	std::ifstream fifthReader(fifthFile, std::ios::binary);

	alternateStep6a(SAinverseWriter, fifthReader, std::get<1>(prep));
	SAinverseWriter.close();
	fifthReader.close();

	std::ifstream SAinverseReader(SAinverseFile, std::ios::binary);
	std::ifstream LMSIndicesReader(LMSIndicesFile, std::ios::binary);

	auto sixth = alternateStep6b<size_t>(SAinverseReader, LMSIndicesReader, std::get<1>(prep));
	SAinverseReader.close();
	LMSIndicesReader.close();

	writeVectorToFile(sixth, sixthFile);
#ifndef NDEBUG
#else
	freeMemory(sixth);
#endif

	source = readVectorFromFile<Alphabet>(sourceFile, addSentinel);
#ifndef NDEBUG
	verifyLMSSuffixesAreSorted(source.data(), sourceLen, sixth);
	freeMemory(sixth);
#endif

	std::vector<Alphabet> dest(sourceLen, 0);

	std::ofstream seventhWriter(seventhFile, std::ios::binary);
	std::ifstream sixthReader(sixthFile, std::ios::binary);

	step2or7<Alphabet, true>(source.data(), sourceLen, maxAlphabet, seventhWriter, sixthReader, std::get<1>(prep), dest.data(), charSum, std::get<0>(prep));
	seventhWriter.close();
	sixthReader.close();

	std::ifstream seventhReader(seventhFile, std::ios::binary);

	std::ofstream dummyStream;
	step3or8<Alphabet, true>(source.data(), sourceLen, maxAlphabet, dummyStream, seventhReader, std::get<1>(prep), dest.data(), charSum, std::get<0>(prep));
	seventhReader.close();

	writeVectorToFile(dest, destFile);

	remove(LMSLeftFile.c_str());
	remove(charSumFile.c_str());
	remove(LMSIndicesFile.c_str());
	remove(LCountFile.c_str());
	remove(secondFile.c_str());
	remove(thirdFile.c_str());
	remove(fourthFile.c_str());
	remove(fifthFile.c_str());
	remove(SAinverseFile.c_str());
	remove(sixthFile.c_str());
	remove(seventhFile.c_str());
}

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t maxAlphabet, const std::string& destFile)
{
	bwtInFiles<Alphabet>(sourceFile, maxAlphabet, destFile, true);
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