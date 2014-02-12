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

//----------
//use these
void bwt(const char* source, size_t sourceLen, char* dest);
void inverseBWT(const char* source, size_t sourceLen, char* dest);
void bwtInFiles(const char* sourceFile, size_t maxMemory, const char* destFile);
void bwtInFiles(const std::string& sourceFile, size_t maxMemory, const std::string& destFile);
//----------


void cerrMemoryUsage(const char* msg);

template <class Alphabet, class IndexType>
void bwt(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest);

template <class Alphabet>
void bwt(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest);

template <class Alphabet, class IndexType>
void bwtInFiles(const std::string& sourceFile, size_t sourceLen, size_t maxAlphabet, size_t maxMemory, const std::string& destFile, bool addSentinel);

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t sourceLen, size_t maxAlphabet, size_t maxMemory, const std::string& destFile, bool addSentinel);

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t maxAlphabet, size_t maxMemory, const std::string& destFile);

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

//vector.clear() doesn't guarantee that memory will be freed
template <class O>
void freeMemory(O& o)
{
	O().swap(o);
}

//used to write/read sequentially into memory without copying the memory (stringbuf copies memory)
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

//memory usage reduction trick from the end of paper's section 5
template <class ItemType, class PriorityType>
class WeirdPriorityQueue
{
private:
	struct WriteType { ItemType item; PriorityType priority; };
public:
	WeirdPriorityQueue(size_t maxPriority, size_t maxBytes) :
		numItems(0),
		k(maxBytes/sizeof(ItemType)),
		currentStart(0),
		currentPos(0),
		currentEnd(k),
		maxPriority(maxPriority),
		currentItems(k),
		used(k, false),
		fileNames(),
		streams()
	{
		size_t maxFile = (maxPriority+k-1)/k;
		for (size_t i = 0; i < maxFile; i++)
		{
			fileNames.push_back(getTempFileName());
			std::fstream* stream = new std::fstream(fileNames.back(), std::ios_base::in | std::ios_base::out | std::ios_base::binary);
			assert(stream->good());
			streams.push_back(stream);
		}
	};
	WeirdPriorityQueue(const WeirdPriorityQueue& second) = delete;
	WeirdPriorityQueue& operator=(const WeirdPriorityQueue& second) = delete;
	WeirdPriorityQueue(WeirdPriorityQueue&& second) = delete;
	WeirdPriorityQueue& operator=(WeirdPriorityQueue&& second) = delete;
	~WeirdPriorityQueue()
	{
		assert(fileNames.size() == streams.size());
		for (size_t i = 0; i < fileNames.size(); i++)
		{
			streams[i]->close();
			delete streams[i];
			remove(fileNames[i].c_str());
		}
	};
	ItemType get()
	{
		assert(currentPos <= k);
		if (currentPos == k)
		{
			for (size_t i = 0; i < k; i++)
			{
				used[i] = false;
			}
			currentPos = 0;
			currentStart += k;
			currentEnd += k;
			size_t fileN = currentStart/k;
			assert(fileN < streams.size());
			streams[fileN]->flush();
			streams[fileN]->seekg(0, std::ios::beg);
			while (true)
			{
				WriteType read;
				assert(streams[fileN]->good());
				streams[fileN]->read((char*)&read, sizeof(WriteType));
				if (streams[fileN]->eof())
				{
					break;
				}
				assert(read.priority >= currentStart);
				assert(read.priority < currentEnd);
				currentItems[read.priority-currentStart] = read.item;
				used[read.priority-currentStart] = true;
			}
		}
		while (!used[currentPos])
		{
			currentPos++;
			if (currentPos == k)
			{
				return get();
			}
		}
		currentPos++;
		numItems--;
		assert(currentPos > 0);
		return currentItems[currentPos-1];
	};
	void insert(ItemType item, PriorityType priority)
	{
		assert(priority < maxPriority);
		assert(priority >= currentStart+currentPos);
		numItems++;
		if (priority < currentEnd)
		{
			currentItems[priority-currentStart] = item;
			used[priority-currentStart] = true;
			return;
		}
		WriteType write;
		write.item = item;
		write.priority = priority;
		size_t fileN = priority/k;
		assert(fileN < streams.size());
		assert(streams[fileN]->good());
		streams[fileN]->write((char*)&write, sizeof(WriteType));
	};
	size_t size() { return numItems; };
	bool empty() { return numItems == 0; };
private:
	size_t numItems;
	size_t k;
	size_t currentStart;
	size_t currentPos;
	size_t currentEnd;
	size_t maxPriority;
	std::vector<ItemType> currentItems;
	std::vector<bool> used;
	std::vector<std::string> fileNames;
	std::vector<std::fstream*> streams;
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
template <class Alphabet, class IndexType>
std::tuple<std::vector<IndexType>, //L-counts
           size_t               //count of LMS-type indices
#ifndef NDEBUG
           ,std::vector<bool> //is S-type
#endif
           > 
preprocess(std::istream& text, size_t textLen, size_t maxAlphabet, std::ostream& LMSLeftOut, std::ostream& charSumOut, std::ostream& LMSIndicesOut, bool addSentinel)
{
	std::tuple<std::vector<IndexType>, 
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
	std::vector<IndexType> sums(maxAlphabet+1, 0);
	std::vector<std::vector<IndexType>> buckets(maxAlphabet+1);
	IndexType lastPossibleLMS = 0;
	IndexType lastCharacterBoundary = 0;
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
				LMSIndicesOut.write((char*)&lastPossibleLMS, sizeof(IndexType));
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
	IndexType lastIndex = textLen-1;
	LMSIndicesOut.write((char*)&lastIndex, sizeof(IndexType));
	assert(std::get<1>(ret) < textLen/2);
	std::get<1>(ret)++;

	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	IndexType charSumSum = 0;
	charSumOut.write((char*)&charSumSum, sizeof(IndexType));
	for (int i = 0; i < maxAlphabet+1; i++)
	{
		charSumSum = charSumSum+sums[i];
		charSumOut.write((char*)&charSumSum, sizeof(IndexType));
		LMSLeftOut.write((char*)buckets[i].data(), buckets[i].size()*sizeof(IndexType));
		freeMemory(buckets[i]);
	}

	return ret;
}

template <class Alphabet, class IndexType>
std::tuple<std::vector<IndexType>, //L-counts
           std::vector<IndexType>, //charSum
           size_t               //count of LMS-type indices
#ifndef NDEBUG
           ,std::vector<bool> //is S-type
#endif
           > 
preprocessLowMemory(std::istream& text, size_t textLen, size_t maxAlphabet, size_t k, std::ostream& LMSLeftOut, std::ostream& LMSIndicesOut, bool addSentinel)
{
	std::tuple<std::vector<IndexType>, 
	           std::vector<IndexType>,
	           size_t
#ifndef NDEBUG
	           ,std::vector<bool>
#endif
	           > ret;
	std::get<0>(ret).resize(maxAlphabet+1, 0);
	std::get<1>(ret).resize(maxAlphabet+2, 0);
	std::get<2>(ret) = 0;
#ifndef NDEBUG
	std::get<3>(ret).resize(textLen, false);
#endif
	std::vector<IndexType> sums(maxAlphabet+1, 0);
	//two passes, first calculate how many times each symbol appears. This is necessary for calculating where each LMS index would go in the LMSleft array
	for (size_t i = 0; i < textLen-1; i++)
	{
		Alphabet symbol;
		text.read((char*)&symbol, sizeof(Alphabet));
		sums[symbol]++;
	}
	if (addSentinel)
	{
		sums[0]++;
	}
	else
	{
		Alphabet symbol;
		text.read((char*)&symbol, sizeof(Alphabet));
		sums[symbol]++;
	}
	for (size_t i = 1; i < maxAlphabet+2; i++)
	{
		std::get<1>(ret)[i] = std::get<1>(ret)[i-1]+sums[i-1];
	}
	freeMemory(sums);
	text.clear();
	text.seekg(0, std::ios::beg);
	assert(text.good());
	WeirdPriorityQueue<IndexType, IndexType> LMSLeft(textLen, k);
	std::vector<IndexType> indicesWritten(maxAlphabet+1, 0);
	IndexType lastPossibleLMS = 0;
	IndexType lastCharacterBoundary = 0;
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
				std::get<3>(ret)[a] = true;
			}
#endif
			if (lastWasLType)
			{
				LMSLeft.insert(lastPossibleLMS, std::get<1>(ret)[currentSymbol]+indicesWritten[currentSymbol]);
				indicesWritten[currentSymbol]++;
				LMSIndicesOut.write((char*)&lastPossibleLMS, sizeof(IndexType));
				assert(std::get<2>(ret) < textLen/2);
				std::get<2>(ret)++;
			}
			lastWasLType = false;
			lastCharacterBoundary = i+1;
		}
		else
		{
			assert(currentSymbol == nextSymbol);
		}
	}
#ifndef NDEBUG
	std::get<3>(ret)[textLen-1] = true;
#endif
	LMSLeft.insert(textLen-1, std::get<1>(ret)[nextSymbol]+indicesWritten[nextSymbol]);
	IndexType lastIndex = textLen-1;
	LMSIndicesOut.write((char*)&lastIndex, sizeof(IndexType));
	assert(std::get<2>(ret) < textLen/2);
	std::get<2>(ret)++;

	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	while (!LMSLeft.empty())
	{
		IndexType index = LMSLeft.get();
		LMSLeftOut.write((char*)&index, sizeof(IndexType));
	}

	return ret;
}

//high memory alternative used for in-memory BWT
//sorts (with step 3) the LMS-type substrings
template <class Alphabet, class IndexType, bool isStep7>
void step2or7(const Alphabet* const text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize, Alphabet* const result, const std::vector<IndexType>& charSum, const std::vector<IndexType>& Lsum)
{
	if (isStep7)
	{
		assert(result != nullptr);
	}
	std::vector<std::vector<IndexType>> buckets(maxAlphabet+1); //A_l in paper
	std::vector<IndexType> numbersOutputted(maxAlphabet+1);
	assert(maxAlphabet+1 < std::numeric_limits<int>::max());
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		IndexType index;
		LMSLeft.read((char*)&index, sizeof(IndexType));
		buckets[text[index]+1].push_back(index);
	}
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		buckets[i].reserve(buckets[i].size()+Lsum[i]);
	}
	for (int bucket = 0; bucket < maxAlphabet+1; bucket++)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			IndexType j = buckets[bucket][i];
			IndexType jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			if (text[jminus1] >= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				if (isStep7)
				{
					IndexType writeIndex = jminus1-1;
					if (jminus1 == 0)
					{
						writeIndex = textLen-1;
					}
					assert(charSum[text[jminus1]]+numbersOutputted[text[jminus1]] < textLen);
					assert(numbersOutputted[text[jminus1]] < charSum[text[jminus1]+1]-charSum[text[jminus1]]);
					result[charSum[text[jminus1]]+numbersOutputted[text[jminus1]]] = text[writeIndex];
					numbersOutputted[text[jminus1]]++;
				}
			}
			else
			{
				out.write((char*)&j, sizeof(IndexType));
			}
		}
		freeMemory(buckets[bucket]);
	}
}

//low memory alternative used for in-file BWT
//sorts (with step 3) the LMS-type substrings
template <class Alphabet, class IndexType, bool isStep7>
void step2or7LowMemory(const Alphabet* const text, size_t textLen, size_t maxAlphabet, size_t k, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize, WeirdPriorityQueue<Alphabet, IndexType>* result, const std::vector<IndexType>& charSum, const std::vector<IndexType>& Lsum)
{
	if (isStep7)
	{
		assert(result != nullptr);
	}
	std::vector<IndexType> numbersArrayed(maxAlphabet+1, 0);
	WeirdPriorityQueue<IndexType, IndexType> priorities(textLen, k);
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		IndexType index;
		LMSLeft.read((char*)&index, sizeof(IndexType));
		IndexType pos = charSum[text[index]]+Lsum[text[index]]+numbersArrayed[text[index]];
		priorities.insert(index, pos);
		numbersArrayed[text[index]]++;
	}
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		numbersArrayed[i] = 0;
	}
	while (!priorities.empty())
	{
		IndexType j = priorities.get();
		IndexType jminus1 = j-1;
		if (j == 0)
		{
			jminus1 = textLen-1;
		}
		assert(j < textLen);
		assert(text[jminus1] <= maxAlphabet);
		assert(text[j] <= maxAlphabet);
		if (text[jminus1] >= text[j])
		{
			IndexType pos = charSum[text[jminus1]]+numbersArrayed[text[jminus1]];
			priorities.insert(jminus1, pos);
			numbersArrayed[text[jminus1]]++;
			if (isStep7)
			{
				IndexType writeIndex = jminus1-1;
				if (jminus1 == 0)
				{
					writeIndex = textLen-1;
				}
				result->insert(text[writeIndex], pos);
			}
		}
		else
		{
			out.write((char*)&j, sizeof(IndexType));
		}
	}
}

//high memory alternative for in-memory BWT
//sorts (with step 2) the LMS-type substrings
template <class Alphabet, class IndexType, bool isStep8>
void step3or8(const Alphabet* const text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSRight, size_t LMSRightSize, Alphabet* const result, const std::vector<IndexType>& charSum, const std::vector<IndexType>& Lsum)
{
	if (isStep8)
	{
		assert(result != nullptr);
	}
	std::vector<std::vector<IndexType>> buckets(maxAlphabet+1); //A_s in paper, note that the contents are in reverse order, eg. bucket['a'][0] is the rightmost item in bucket a, not leftmost
	std::vector<IndexType> numbersOutputted(maxAlphabet+1);
	assert(maxAlphabet < std::numeric_limits<int>::max());
	for (size_t i = 0; i < LMSRightSize; i++)
	{
		IndexType index;
		LMSRight.read((char*)&index, sizeof(IndexType));
		assert(index < textLen);
		assert(text[index] > 0);
		buckets[text[index]-1].push_back(index);
	}
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		std::reverse(buckets[i].begin(), buckets[i].end());
		buckets[i].reserve(buckets[i].size()+(charSum[i+1]-charSum[i]-Lsum[i]));
	}
	for (int bucket = maxAlphabet; bucket >= 0; bucket--)
	{
		//can't use iterators because indices may be pushed into current bucket, and that can invalidate iterators
		for (size_t i = 0; i < buckets[bucket].size(); i++)
		{
			IndexType j = buckets[bucket][i];
			IndexType jminus1 = j-1;
			if (j == 0)
			{
				jminus1 = textLen-1;
			}
			assert(j <= textLen);
			assert(text[j] < maxAlphabet+1);
			if (text[jminus1] <= text[j])
			{
				assert(text[jminus1] < maxAlphabet+1);
				buckets[text[jminus1]].push_back(jminus1);
				if (isStep8)
				{
					IndexType writeIndex = jminus1-1;
					if (jminus1 == 0)
					{
						writeIndex = textLen-1;
					}
					assert(numbersOutputted[text[jminus1]] < charSum[text[jminus1]+1]);
					assert(numbersOutputted[text[jminus1]]+1 <= charSum[text[jminus1]+1]);
					assert(numbersOutputted[text[jminus1]]+charSum[text[jminus1]] < charSum[text[jminus1]+1]);
					assert(charSum[text[jminus1]+1]-numbersOutputted[text[jminus1]]-1 < textLen);
					result[charSum[text[jminus1]+1]-numbersOutputted[text[jminus1]]-1] = text[writeIndex];
					numbersOutputted[text[jminus1]]++;
				}
			}
			else
			{
				assert(j != 0);
				if (!isStep8) //don't return anything for step 8
				{
					out.write((char*)&j, sizeof(IndexType));
				}
			}
		}
		freeMemory(buckets[bucket]);
	}
}

//low memory alternative for in-file BWT
//sorts (with step 2) the LMS-type substrings
template <class Alphabet, class IndexType, bool isStep8>
void step3or8LowMemory(const Alphabet* const text, size_t textLen, size_t maxAlphabet, size_t k, std::ostream& out, std::istream& LMSRight, size_t LMSRightSize, WeirdPriorityQueue<Alphabet, IndexType>* result, const std::vector<IndexType>& charSum, const std::vector<IndexType>& Lsum)
{
	if (isStep8)
	{
		assert(result != nullptr);
	}
	//positions start with 0 as the rightmost element because the indices are processed right-to-left
	WeirdPriorityQueue<IndexType, IndexType> priorities(textLen, k);
	std::vector<IndexType> numbersArrayed(maxAlphabet+1, 0);
	for (size_t i = 0; i < LMSRightSize; i++)
	{
		IndexType index;
		LMSRight.read((char*)&index, sizeof(IndexType));
		IndexType reversePos = (charSum[text[index]]+numbersArrayed[text[index]]);
		IndexType pos = textLen-reversePos-1; //positions have 0 as rightmost element
		priorities.insert(index, pos);
		numbersArrayed[text[index]]++;
	}
	for (size_t i = 0; i < maxAlphabet+1; i++)
	{
		numbersArrayed[i] = 0;
	}
	while (!priorities.empty())
	{
		IndexType j = priorities.get();
		IndexType jminus1 = j-1;
		if (j == 0)
		{
			jminus1 = textLen-1;
		}
		if (text[jminus1] <= text[j])
		{
			IndexType reversePos = charSum[text[jminus1]+1]-numbersArrayed[text[jminus1]]-1;
			assert(reversePos < textLen);
			IndexType pos = textLen-reversePos-1; //positions have 0 as rightmost element
			priorities.insert(jminus1, pos);
			numbersArrayed[text[jminus1]]++;
			if (isStep8)
			{
				IndexType writeIndex = jminus1-1;
				if (jminus1 == 0)
				{
					writeIndex = textLen-1;
				}
				result->insert(text[writeIndex], reversePos);
			}
		}
		else
		{
			if (!isStep8) //no output for step 8
			{
				out.write((char*)&j, sizeof(IndexType));
			}
		}
	}
}

//because this only checks equality, it doesn't need to know whether the suffixes are L-type or S-type
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
	}
	assert(false);
}

//because this only checks equality, it doesn't need to know whether the suffixes are L-type or S-type
//Doesn't need information about LMS substring borders to test substring equality. May read values outside the substring, however will not read past the next substring
template <class Alphabet>
bool LMSSubstringsAreEqualNoBorder(const Alphabet* text, size_t textLen, size_t str1, size_t str2)
{
	if (text[str1] != text[str2])
	{
		return false;
	}
	str1++;
	str2++;
	size_t pos = 1;
	size_t numEquals = 0;
	size_t lastPossibleBorderStr1 = 0;
	size_t lastPossibleBorderStr2 = 0;
	bool borderFoundStr1 = false;
	bool borderFoundStr2 = false;
	bool stillEqual = true;
	while (true)
	{
		if (stillEqual)
		{
			if (text[str1] != text[str2])
			{
				stillEqual = false;
				if (lastPossibleBorderStr1 == 0 || lastPossibleBorderStr2 == 0)
				{
					return false;
				}
			}
			else
			{
				numEquals++;
			}
		}
		if (!borderFoundStr1 && text[str1] < text[str1-1])
		{
			lastPossibleBorderStr1 = pos;
			if (str1 == textLen-1)
			{
				borderFoundStr1 = true;
			}
		}
		if (!borderFoundStr2 && text[str2] < text[str2-1])
		{
			lastPossibleBorderStr2 = pos;
			if (str2 == textLen-1)
			{
				borderFoundStr2 = true;
			}
		}
		if (text[str1] > text[str1-1] && lastPossibleBorderStr1 != 0)
		{
			borderFoundStr1 = true;
		}
		if (text[str2] > text[str2-1] && lastPossibleBorderStr2 != 0)
		{
			borderFoundStr2 = true;
		}
		if (borderFoundStr1 && numEquals < lastPossibleBorderStr1)
		{
			return false;
		}
		if (borderFoundStr2 && numEquals < lastPossibleBorderStr2)
		{
			return false;
		}
		if (borderFoundStr2 && borderFoundStr1)
		{
			if (lastPossibleBorderStr2 != lastPossibleBorderStr1)
			{
				return false;
			}
			if (numEquals < lastPossibleBorderStr2)
			{
				return false;
			}
			return true;
		}
		str1++;
		str2++;
		pos++;
	}
	assert(false);
}

//used only in debugging to check that the LMS-substrings are ordered correctly by steps 1-3
//needs to know whether a position is L-type or S-type because the substrings' ordering is important
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

//used only in debugging to check that the LMS-suffixes are ordered correctly by steps 4-6
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

//high memory alternative used for in-memory BWT
//returns whether the output can be directly BWT'd and the max alphabet
template <class Alphabet, class IndexType>
std::tuple<bool, size_t> step4(const Alphabet* text, size_t textLen, size_t maxAlphabet, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize)
{
	std::tuple<bool, size_t> ret;
	std::get<0>(ret) = true;
	std::get<1>(ret) = 0;
	std::vector<bool> LMSSubstringBorder(textLen, false);
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		IndexType index;
		assert(LMSLeft.good());
		LMSLeft.read((char*)&index, sizeof(IndexType));
		LMSSubstringBorder[index] = true;
	}
	LMSLeft.clear();
	LMSLeft.seekg(0);
	assert(LMSLeft.good());
	std::vector<IndexType> sparseSPrime((textLen+1)/2, 0); //not sure if needs to round up, do it just in case
	IndexType currentName = LMSLeftSize+1;
	IndexType oldIndex = 0;
	IndexType index = 0;
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		assert(LMSLeft.good());
		LMSLeft.read((char*)&index, sizeof(IndexType));
		if (i == 0 || !LMSSubstringsAreEqual(text, textLen, oldIndex, index, LMSSubstringBorder))
		{
			currentName--;
		}
		else if (i != 0)
		{
			//lms substrings are equal
			std::get<0>(ret) = false;
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
			IndexType write = *i-currentName;
			if (write > std::get<1>(ret))
			{
				std::get<1>(ret) = write;
			}
			assert(write < LMSLeftSize);
			out.write((char*)&write, sizeof(IndexType));
		}
	}
	return ret;
}

//low memory alternative used for in-file BWT
//returns whether the output can be directly BWT'd and the max alphabet
template <class Alphabet, class IndexType>
std::tuple<bool, size_t> step4LowMemory(const Alphabet* text, size_t textLen, size_t maxAlphabet, size_t k, std::ostream& out, std::istream& LMSLeft, size_t LMSLeftSize)
{
	std::tuple<bool, size_t> ret;
	std::get<0>(ret) = true;
	std::get<1>(ret) = 0;
	//using an array for sparse S' would use |text|/2*sizeof(text_pointer) bytes, usually |text|*2 bytes
	WeirdPriorityQueue<IndexType, IndexType> sparseSPrime((textLen+1)/2, k);
	IndexType currentName = LMSLeftSize+1;
	IndexType oldIndex = 0;
	IndexType index = 0;
	for (size_t i = 0; i < LMSLeftSize; i++)
	{
		assert(LMSLeft.good());
		LMSLeft.read((char*)&index, sizeof(IndexType));
		if (i == 0 || !LMSSubstringsAreEqualNoBorder(text, textLen, oldIndex, index))
		{
			currentName--;
		}
		else if (i != 0)
		{
			//lms substrings are equal
			std::get<0>(ret) = false;
		}
		assert(index/2 < (textLen+1)/2);
		sparseSPrime.insert(currentName, index/2);
		assert(currentName > 0);
		oldIndex = index;
	}
	while (!sparseSPrime.empty())
	{
		IndexType write = sparseSPrime.get()-currentName;
		if (write > std::get<1>(ret))
		{
			std::get<1>(ret) = write;
		}
		assert(write < LMSLeftSize);
		out.write((char*)&write, sizeof(IndexType));
	}
	return ret;
}

//do a counting sort on the rotated strings and pick the last element
template <class IndexType>
std::vector<IndexType> bwtDirectly(const std::vector<IndexType>& data)
{
	auto max = std::max_element(data.begin(), data.end());
	assert(*max == data.size()-1);
	std::vector<IndexType> location(*max+1, 0);
	for (size_t i = 0; i < data.size(); i++)
	{
		location[data[i]] = i+1; //use indexes that start from 1 so 0 is reserved as "unused"
	}
	std::vector<IndexType> ret;
	for (auto i = location.begin(); i != location.end(); i++)
	{
		if (*i != 0)
		{
			IndexType loc = *i-2;
			if (*i == 1)
			{
				loc = data.size()-1;
			}
			ret.push_back(data[loc]);
		}
	}
	return ret;
}

template <class IndexType>
std::vector<IndexType> step5(const std::vector<IndexType>& Sprime, bool canBWTDirectly, size_t maxAlphabet)
{
	if (canBWTDirectly)
	{
		return bwtDirectly(Sprime);
	}
	std::vector<IndexType> result(Sprime.size(), 0);
	bwt<IndexType>(Sprime.data(), Sprime.size(), maxAlphabet, result.data());
	return result;
}

template <class IndexType>
void step5InFile(const std::string& SprimeFile, const std::string& outFile, size_t maxMemory, size_t SprimeSize, bool canBWTDirectly, size_t maxAlphabet)
{
	if (canBWTDirectly)
	{
		std::vector<IndexType> SprimeVec = readVectorFromFile<IndexType>(SprimeFile, false);
		std::vector<IndexType> result = bwtDirectly(SprimeVec);
		writeVectorToFile(result, outFile);
		return;
	}
	bwtInFiles<IndexType>(SprimeFile, SprimeSize, maxAlphabet, maxMemory, outFile, false);
}

//calculate the inverse suffix array of S' by using BWT'
template <class IndexType>
void alternateStep6a(std::ostream& SAinverse, std::istream& BWTprime, size_t BWTprimeSize)
{
	std::vector<IndexType> charNum(1, 0);
	IndexType maxAlphabet = 0;
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		IndexType read;
		BWTprime.read((char*)&read, sizeof(IndexType));
		if (read > maxAlphabet)
		{
			maxAlphabet = read;
			charNum.resize(maxAlphabet+1);
		}
		charNum[read]++;
	}
	BWTprime.clear();
	BWTprime.seekg(0);
	std::vector<IndexType> charSum(maxAlphabet+2, 0);
	for (IndexType i = 1; i < maxAlphabet+1; i++)
	{
		charSum[i] = charSum[i-1]+charNum[i-1];
	}
	charSum[maxAlphabet+1] = charSum[maxAlphabet]+charNum[maxAlphabet];
	std::vector<IndexType> LFinverse(BWTprimeSize, 0);
	std::vector<IndexType> usedSlots(maxAlphabet+1, 0);
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		IndexType read;
		BWTprime.read((char*)&read, sizeof(IndexType));
		assert(usedSlots[read] < charSum[read+1]-charSum[read]);
		LFinverse[charSum[read]+usedSlots[read]] = i;
		usedSlots[read]++;
	}
	for (size_t i = 0; i <= maxAlphabet; i++)
	{
		assert(usedSlots[i] == charSum[i+1]-charSum[i]);
	}
	IndexType index = 0;
	for (size_t i = 0; i < BWTprimeSize; i++)
	{
		SAinverse.write((char*)&LFinverse[index], sizeof(IndexType));
		index = LFinverse[index];
	}
}

//calculate the order of LMS-suffixes of S by using the inverse suffix array of S' and a list of all LMS indices from step 1
template <class Alphabet, class IndexType>
std::vector<IndexType> alternateStep6b(std::istream& SAinverse, std::istream& LMSIndices, size_t size)
{
	std::vector<IndexType> ret(size, 0);
	for (size_t i = 0; i < size; i++)
	{
		IndexType SA;
		SAinverse.read((char*)&SA, sizeof(IndexType));
		IndexType index;
		LMSIndices.read((char*)&index, sizeof(IndexType));
		ret[SA] = index;
	}
	return ret;
}

//only for debugging
template <class Alphabet, class IndexType>
void verifyLMSSubstringsAreSorted(const Alphabet* source, size_t sourceLen, const std::vector<IndexType>& LMSIndices, const std::vector<bool>& isSType)
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

//only for debugging
template <class Alphabet, class IndexType>
void verifyLMSSuffixesAreSorted(const Alphabet* source, size_t sourceLen, const std::vector<IndexType>& LMSIndices)
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
template <class Alphabet, class IndexType>
void bwt(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	std::vector<IndexType> LMSLeft(sourceLen/2, 0);
	std::vector<IndexType> charSum(maxAlphabet+2, 0);
	std::vector<IndexType> LMSIndices(sourceLen/2, 0);

	MemoryStreambuffer<Alphabet> sourceBuf((Alphabet*)source, sourceLen);
	MemoryStreambuffer<IndexType> LMSLeftBuf(LMSLeft.data(), sourceLen/2);
	MemoryStreambuffer<IndexType> charSumBuf(charSum.data(), maxAlphabet+2);
	MemoryStreambuffer<IndexType> LMSIndicesBuf(LMSIndices.data(), sourceLen/2);

	std::istream sourceReader(&sourceBuf);
	std::istream LMSLeftReader(&LMSLeftBuf);
//	std::istream charSumReader(&charSumBuf);
	std::istream LMSIndicesReader(&LMSIndicesBuf);

	std::ostream LMSLeftWriter(&LMSLeftBuf);
	std::ostream charSumWriter(&charSumBuf);
	std::ostream LMSIndicesWriter(&LMSIndicesBuf);

	auto prep = preprocess<Alphabet, IndexType>(sourceReader, sourceLen, maxAlphabet, LMSLeftWriter, charSumWriter, LMSIndicesWriter, false);

	assert(std::get<1>(prep) <= sourceLen/2);
	LMSLeft.resize(std::get<1>(prep));
	LMSIndices.resize(std::get<1>(prep));

	std::vector<IndexType> second(std::get<1>(prep), 0);
	MemoryStreambuffer<IndexType> secondBuf(second.data(), std::get<1>(prep));
	std::ostream secondWriter(&secondBuf);
	std::istream secondReader(&secondBuf);

	step2or7<Alphabet, IndexType, false>(source, sourceLen, maxAlphabet, secondWriter, LMSLeftReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	freeMemory(LMSLeft);

	std::vector<IndexType> third(std::get<1>(prep), 0);
	MemoryStreambuffer<IndexType> thirdBuf(third.data(), std::get<1>(prep));
	std::ostream thirdWriter(&thirdBuf);
	std::istream thirdReader(&thirdBuf);

	step3or8<Alphabet, IndexType, false>(source, sourceLen, maxAlphabet, thirdWriter, secondReader, std::get<1>(prep), (Alphabet*)nullptr, charSum, std::get<0>(prep));
	freeMemory(second);
#ifndef NDEBUG
	verifyLMSSubstringsAreSorted<Alphabet, IndexType>(source, sourceLen, third, std::get<2>(prep));
#endif

	std::vector<IndexType> fourth(std::get<1>(prep), 0);
	MemoryStreambuffer<IndexType> fourthBuf(fourth.data(), std::get<1>(prep));
	std::ostream fourthWriter(&fourthBuf);

	auto fourthRet = step4<Alphabet, IndexType>(source, sourceLen, maxAlphabet, fourthWriter, thirdReader, std::get<1>(prep));
	freeMemory(third);
	auto fifth = step5<IndexType>(fourth, std::get<0>(fourthRet), std::get<1>(fourthRet));

	MemoryStreambuffer<IndexType> fifthBuf(fifth.data(), std::get<1>(prep));
	std::istream fifthReader(&fifthBuf);
	std::vector<IndexType> SAinverse(std::get<1>(prep), 0);
	MemoryStreambuffer<IndexType> SAinverseBuf(SAinverse.data(), std::get<1>(prep));
	std::ostream SAinverseWriter(&SAinverseBuf);
	std::istream SAinverseReader(&SAinverseBuf);

	alternateStep6a<IndexType>(SAinverseWriter, fifthReader, std::get<1>(prep));
	freeMemory(fifth);
	auto sixth = alternateStep6b<Alphabet, IndexType>(SAinverseReader, LMSIndicesReader, std::get<1>(prep));
	freeMemory(LMSIndices);
	freeMemory(SAinverse);
#ifndef NDEBUG
	verifyLMSSuffixesAreSorted<Alphabet, IndexType>(source, sourceLen, sixth);
#endif
	MemoryStreambuffer<IndexType> sixthBuf(sixth.data(), sixth.size());
	std::istream sixthReader(&sixthBuf);

	std::vector<IndexType> seventh(std::get<1>(prep), 0);
	MemoryStreambuffer<IndexType> seventhBuf(seventh.data(), std::get<1>(prep));
	std::ostream seventhWriter(&seventhBuf);
	std::istream seventhReader(&seventhBuf);

	step2or7<Alphabet, IndexType, true>(source, sourceLen, maxAlphabet, seventhWriter, sixthReader, sixth.size(), dest, charSum, std::get<0>(prep));
	freeMemory(sixth);
	std::ofstream dummyStream;
	step3or8<Alphabet, IndexType, true>(source, sourceLen, maxAlphabet, dummyStream, seventhReader, std::get<1>(prep), dest, charSum, std::get<0>(prep));
}

template <class Alphabet>
void bwt(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	if (sourceLen < 255)
	{
		bwt<Alphabet, unsigned char>(source, sourceLen, maxAlphabet, dest);
	}
	else if (sourceLen < std::numeric_limits<uint16_t>::max())
	{
		bwt<Alphabet, uint16_t>(source, sourceLen, maxAlphabet, dest);
	}
	else if (sourceLen < std::numeric_limits<uint32_t>::max())
	{
		bwt<Alphabet, uint32_t>(source, sourceLen, maxAlphabet, dest);
	}
	else
	{
		bwt<Alphabet, size_t>(source, sourceLen, maxAlphabet, dest);
	}
}

template <class Alphabet, class IndexType>
void bwtInFiles(const std::string& sourceFile, size_t sourceLen, size_t maxAlphabet, size_t maxMemory, const std::string& destFile, bool addSentinel)
{
	cerrMemoryUsage("start of in-file BWT");

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
	std::ofstream LMSIndicesWriter(LMSIndicesFile, std::ios::binary);
	std::ifstream sourceReader(sourceFile, std::ios::binary);

	cerrMemoryUsage("before preprocessing");

	auto prep = preprocessLowMemory<Alphabet, IndexType>(sourceReader, sourceLen, maxAlphabet, maxMemory, LMSLeftWriter, LMSIndicesWriter, addSentinel);
	LMSLeftWriter.close();
	LMSIndicesWriter.close();
	sourceReader.close();

	assert(std::get<2>(prep) <= sourceLen/2);

	std::vector<Alphabet> source;
	source = readVectorFromFile<Alphabet>(sourceFile, addSentinel);

	std::ofstream secondWriter(secondFile, std::ios::binary);
	std::ifstream LMSLeftReader(LMSLeftFile, std::ios::binary);

	cerrMemoryUsage("before step 2");

	step2or7LowMemory<Alphabet, IndexType, false>(source.data(), sourceLen, maxAlphabet, maxMemory, secondWriter, LMSLeftReader, std::get<2>(prep), (WeirdPriorityQueue<Alphabet, IndexType>*)nullptr, std::get<1>(prep), std::get<0>(prep));
	secondWriter.close();
	LMSLeftReader.close();

	std::ofstream thirdWriter(thirdFile, std::ios::binary);
	std::ifstream secondReader(secondFile, std::ios::binary);

	cerrMemoryUsage("before step 3");

	step3or8LowMemory<Alphabet, IndexType, false>(source.data(), sourceLen, maxAlphabet, maxMemory, thirdWriter, secondReader, std::get<2>(prep), (WeirdPriorityQueue<Alphabet, IndexType>*)nullptr, std::get<1>(prep), std::get<0>(prep));
	thirdWriter.close();
	secondReader.close();

#ifndef NDEBUG
	cerrMemoryUsage("before LMS substring order verification");

	std::vector<IndexType> third = readVectorFromFile<IndexType>(thirdFile, false);
	verifyLMSSubstringsAreSorted(source.data(), sourceLen, third, std::get<3>(prep));
	freeMemory(third);
#endif

	std::ofstream fourthWriter(fourthFile, std::ios::binary);
	std::ifstream thirdReader(thirdFile, std::ios::binary);

	cerrMemoryUsage("before step 4");

	auto fourthRet = step4LowMemory<Alphabet, IndexType>(source.data(), sourceLen, maxAlphabet, maxMemory, fourthWriter, thirdReader, std::get<2>(prep));
	fourthWriter.close();
	thirdReader.close();

	freeMemory(source);
	writeVectorToFile(std::get<1>(prep), charSumFile);
	freeMemory(std::get<1>(prep));
	writeVectorToFile(std::get<0>(prep), LCountFile);
	freeMemory(std::get<0>(prep));

	cerrMemoryUsage("before step 5");

	step5InFile<IndexType>(fourthFile, fifthFile, maxMemory, std::get<2>(prep), std::get<0>(fourthRet), std::get<1>(fourthRet));

	std::ofstream SAinverseWriter(SAinverseFile, std::ios::binary);
	std::ifstream fifthReader(fifthFile, std::ios::binary);

	cerrMemoryUsage("before step 6a");

	alternateStep6a<IndexType>(SAinverseWriter, fifthReader, std::get<2>(prep));
	SAinverseWriter.close();
	fifthReader.close();

	std::ifstream SAinverseReader(SAinverseFile, std::ios::binary);
	std::ifstream LMSIndicesReader(LMSIndicesFile, std::ios::binary);

	cerrMemoryUsage("before step 6b");

	auto sixth = alternateStep6b<Alphabet, IndexType>(SAinverseReader, LMSIndicesReader, std::get<2>(prep));
	SAinverseReader.close();
	LMSIndicesReader.close();

	writeVectorToFile(sixth, sixthFile);
#ifndef NDEBUG
#else
	freeMemory(sixth);
#endif

	source = readVectorFromFile<Alphabet>(sourceFile, addSentinel);
#ifndef NDEBUG
	cerrMemoryUsage("before LMS suffix order verification");

	verifyLMSSuffixesAreSorted(source.data(), sourceLen, sixth);
	freeMemory(sixth);
#endif

	std::get<0>(prep) = readVectorFromFile<IndexType>(LCountFile, false);
	std::get<1>(prep) = readVectorFromFile<IndexType>(charSumFile, false);

	std::ofstream seventhWriter(seventhFile, std::ios::binary);
	std::ifstream sixthReader(sixthFile, std::ios::binary);

	cerrMemoryUsage("before step 7");

	WeirdPriorityQueue<Alphabet, IndexType> result(sourceLen, maxMemory/2);

	step2or7LowMemory<Alphabet, IndexType, true>(source.data(), sourceLen, maxAlphabet, maxMemory/2, seventhWriter, sixthReader, std::get<2>(prep), &result, std::get<1>(prep), std::get<0>(prep));
	seventhWriter.close();
	sixthReader.close();

	std::ifstream seventhReader(seventhFile, std::ios::binary);

	cerrMemoryUsage("before step 8");

	std::ofstream dummyStream;
	step3or8LowMemory<Alphabet, IndexType, true>(source.data(), sourceLen, maxAlphabet, maxMemory/2, dummyStream, seventhReader, std::get<2>(prep), &result, std::get<1>(prep), std::get<0>(prep));
	seventhReader.close();

	std::ofstream resultWriter(destFile, std::ios::binary);
	while (!result.empty())
	{
		Alphabet alph = result.get();
		resultWriter.write((char*)&alph, sizeof(Alphabet));
	}
	resultWriter.close();

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

	cerrMemoryUsage("end of BWT");
}

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t sourceLen, size_t maxAlphabet, size_t maxMemory, const std::string& destFile, bool addSentinel)
{
	if (sourceLen < 255)
	{
		bwtInFiles<Alphabet, unsigned char>(sourceFile, sourceLen, maxAlphabet, maxMemory, destFile, addSentinel);
	}
	else if (sourceLen < std::numeric_limits<uint16_t>::max())
	{
		bwtInFiles<Alphabet, uint16_t>(sourceFile, sourceLen, maxAlphabet, maxMemory, destFile, addSentinel);
	}
	else if (sourceLen < std::numeric_limits<uint32_t>::max())
	{
		bwtInFiles<Alphabet, uint32_t>(sourceFile, sourceLen, maxAlphabet, maxMemory, destFile, addSentinel);
	}
	else
	{
		bwtInFiles<Alphabet, size_t>(sourceFile, sourceLen, maxAlphabet, maxMemory, destFile, addSentinel);
	}
}

template <class Alphabet>
void bwtInFiles(const std::string& sourceFile, size_t maxAlphabet, size_t maxMemory, const std::string& destFile)
{
	size_t sourceLen = getFileLengthInAlphabets<Alphabet>(sourceFile)+1;
	bwtInFiles<Alphabet>(sourceFile, sourceLen, maxAlphabet, maxMemory, destFile, true);
}

template <class Alphabet, class IndexType>
void inverseBWT(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	auto charSum = charSums(source, sourceLen, maxAlphabet);
	std::vector<IndexType> usedCharacters(maxAlphabet+1, 0);
	std::vector<IndexType> LFmapping(sourceLen, 0);
	IndexType index = 0;
	for (size_t i = 0; i < sourceLen; i++)
	{
		assert(source[i] < maxAlphabet+1);
		assert(charSum[source[i]] < std::numeric_limits<IndexType>::max()-usedCharacters[source[i]]);
		LFmapping[i] = charSum[source[i]]+usedCharacters[source[i]];
		usedCharacters[source[i]]++;
		if (source[i] == 0)
		{
			assert(index == 0);
			index = i;
		}
	}
	IndexType loc = sourceLen-1;
	do
	{
		assert(index < sourceLen);
		dest[loc] = source[index];
		index = LFmapping[index];
		loc--;
	} while (loc != 0);
	dest[loc] = source[index];
}

template <class Alphabet>
void inverseBWT(const Alphabet* source, size_t sourceLen, size_t maxAlphabet, Alphabet* dest)
{
	if (sourceLen < 255)
	{
		inverseBWT<Alphabet, unsigned char>(source, sourceLen, maxAlphabet, dest);
	}
	else if (sourceLen < std::numeric_limits<uint16_t>::max())
	{
		inverseBWT<Alphabet, uint16_t>(source, sourceLen, maxAlphabet, dest);
	}
	else if (sourceLen < std::numeric_limits<uint32_t>::max())
	{
		inverseBWT<Alphabet, uint32_t>(source, sourceLen, maxAlphabet, dest);
	}
	else
	{
		inverseBWT<Alphabet, size_t>(source, sourceLen, maxAlphabet, dest);
	}
}

#endif