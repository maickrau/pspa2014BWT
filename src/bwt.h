#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>
#include <utility>

template <class Alphabet>
std::vector<size_t> charSums(const Alphabet* text, size_t textLen, Alphabet maxAlphabet)
{
	std::vector<size_t> sums(maxAlphabet, 0);
	for (size_t i = 0; i < textLen; i++)
	{
		sums[text[i]]++;
	}
	std::vector<size_t> res(maxAlphabet, 0);
	for (int i = 1; i < maxAlphabet; i++)
	{
		res[i] = res[i-1]+sums[i-1];
	}
	return res;
}

extern std::vector<size_t> charSums(const char* text, size_t textLen);
extern std::vector<size_t> charSums(const unsigned char* text, size_t textLen);

template <class Alphabet>
std::vector<size_t> step1(const Alphabet* text, size_t textLen, Alphabet maxAlphabet);
extern std::vector<size_t> step1(const char* text, size_t textLen);
extern std::vector<size_t> step1(const unsigned char* text, size_t textLen);

template <class Alphabet>
std::vector<size_t> step2(const Alphabet* text, size_t textLen, Alphabet maxAlphabet, const std::vector<size_t>& LMSLeft);
extern std::vector<size_t> step2(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft);
extern std::vector<size_t> step2(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft);

template <class Alphabet>
std::vector<size_t> step3(const Alphabet* text, size_t textLen, Alphabet maxAlphabet, const std::vector<size_t>& LMSRight);
extern std::vector<size_t> step3(const char* text, size_t textLen, const std::vector<size_t>& LMSRight);
extern std::vector<size_t> step3(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight);

template <class Alphabet>
std::pair<std::vector<size_t>, std::vector<size_t>> step4(const Alphabet* text, size_t textLen, Alphabet maxAlphabet, const std::vector<size_t>& LMSLeft);
extern std::pair<std::vector<size_t>, std::vector<size_t>> step4(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft);
extern std::pair<std::vector<size_t>, std::vector<size_t>> step4(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft);

std::vector<size_t> step5(const std::vector<size_t>& Sprime);

std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R);

template <class Alphabet>
std::vector<size_t> step7(const Alphabet* text, size_t textLen, Alphabet maxAlphabet, const std::vector<size_t>& LMSLeft, Alphabet* result, const std::vector<size_t>& charSums);
extern std::vector<size_t> step7(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft, unsigned char* result, const std::vector<size_t>& charSums);
extern std::vector<size_t> step7(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft, char* result, const std::vector<size_t>& charSums);

template <class Alphabet>
std::vector<size_t> step8(const Alphabet* text, size_t textLen, Alphabet maxAlphabet, const std::vector<size_t>& LMSRight, Alphabet* result, const std::vector<size_t>& charSums);
extern std::vector<size_t> step8(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight, unsigned char* result, const std::vector<size_t>& charSums);
extern std::vector<size_t> step8(const char* text, size_t textLen, const std::vector<size_t>& LMSRight, char* result, const std::vector<size_t>& charSums);

template <class Alphabet>
void bwt(const Alphabet* source, size_t sourceLen, Alphabet* dest)
{
	auto charSum = charSums(source, sourceLen);
	auto first = step1(source, sourceLen);
	auto second = step2(source, sourceLen, first);
	auto third = step3(source, sourceLen, second);
	auto fourth = step4(source, sourceLen, third);
	auto fifth = step5(fourth.first);
	auto sixth = step6(fifth, fourth.second);
	auto seventh = step7(source, sourceLen, sixth, dest, charSum);
	step8(source, sourceLen, seventh, dest, charSum);
}

#endif