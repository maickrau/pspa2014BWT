#include <iostream>
#include <algorithm>

#include "bwt.h"

void testEqualityAndWhine(const char* step, const char* text, size_t textlen, const std::vector<size_t>& result, const std::vector<size_t>& wanted)
{
	if (std::equal(result.begin(), result.end(), wanted.begin()))
	{
		std::cout << step << " equal\n";
	}
	else
	{
		std::cout << step << " NOT EQUAL\n";
		std::cout << text << " " << textlen << "\n";
		std::cout << "wanted: ";
		for (auto i = wanted.begin(); i != wanted.end(); i++)
		{
			std::cout << *i << " ";
		}
		std::cout << "\n";
		std::cout << "result: ";
		for (auto i = result.begin(); i != result.end(); i++)
		{
			std::cout << *i << " ";
		}
		std::cout << "\n";
	}
}

void testStep1(const char* text, size_t textlen, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step1(text, textlen);
	testEqualityAndWhine("step 1", text, textlen, result, wantedResult);
}

void testStep2(const char* text, size_t textlen, const std::vector<size_t>& step1result, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step2(text, textlen, step1result);
	testEqualityAndWhine("step 2", text, textlen, result, wantedResult);
}

void testStep3(const char* text, size_t textlen, const std::vector<size_t>& step2result, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step3(text, textlen, step2result);
	testEqualityAndWhine("step 3", text, textlen, result, wantedResult);
}

void testStep6(const char* text, size_t textlen, const std::vector<size_t>& BWTprime, const std::vector<size_t>& R, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step6(BWTprime, R);
	testEqualityAndWhine("step 6", text, textlen, result, wantedResult);
}

void testCharSums(const char* text, size_t textlen, const std::vector<size_t>& wantedCharSums)
{
	std::vector<size_t> result = charSums(text, textlen);
	testEqualityAndWhine("charsums", text, textlen, result, wantedCharSums);
}

void testStep7(const char* text, size_t textlen, const char* wantedText, const std::vector<size_t>& step6result, const std::vector<size_t>& charSums, const std::vector<size_t>& wantedResult)
{
	char* resultString = new char[textlen](); //initialized to \0
	std::vector<size_t> result = step7(text, textlen, step6result, resultString, charSums);
	//no strcmp/strncmp because there will be \0 inside result
	bool right = true;
	for (size_t i = 0; i < textlen; i++)
	{
		if (resultString[i] != wantedText[i])
		{
			right = false;
			break;
		}
	}
	if (!right)
	{
		std::cout << "Step 7 BWT-written result not correct, expected:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << wantedText[i];
		}
		std::cout << "\"\ngot:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << resultString[i];
		}
		std::cout << "\"\n";
	}
	delete [] resultString;
	testEqualityAndWhine("step 7", text, textlen, result, wantedResult);
}

void testStep8(const char* text, size_t textlen, const char* wantedText, const std::vector<size_t>& step6result, const std::vector<size_t>& charSums)
{
	char* resultString = new char[textlen](); //initialized to \0
	step8(text, textlen, step6result, resultString, charSums);
	//no strcmp/strncmp because there will be \0 inside result
	bool right = true;
	for (size_t i = 0; i < textlen; i++)
	{
		if (resultString[i] != wantedText[i])
		{
			right = false;
			break;
		}
	}
	if (!right)
	{
		std::cout << "Step 8 BWT-written result not correct, expected:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << wantedText[i];
		}
		std::cout << "\"\ngot:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << resultString[i];
		}
		std::cout << "\"\n";
	}
	else
	{
		std::cout << "step 8 equal";
	}
	delete [] resultString;
}

void test()
{
	//test each step on paper's example
	//indices corrected to start at 0 insted of 1
	const char* testString = "amammmasasmasassaara\0";
	int testStringLen = 21; //includes trailing \0
	std::vector<size_t> wantedCharSums(256, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 256; a++)
		{
			wantedCharSums[a]++;
		}
	}
	testCharSums(testString, testStringLen, wantedCharSums);
	std::vector<size_t> wanted { 20, 2, 6, 8, 11, 13, 16 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 1, 3, 18, 7, 12, 9, 14 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 20, 16, 2, 6, 11, 8, 13 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wanted5 { 1, 5, 0, 2, 4, 3, 3 };
	std::vector<size_t> wantedR { 2, 20, 6, 13, 11, 16 };
	std::vector<size_t> wanted6 { 20, 16, 2, 6, 11, 13, 13 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);
	std::vector<size_t> wanted7 { 1, 3, 18, 12, 12, 9, 14 };
	testStep7(testString, testStringLen, "\0r\0\0\0\0\0\0\0\0amsmaasaaaa", wanted6, wantedCharSums, wanted7);
	testStep8(testString, testStringLen, "a\0s\0mammss\0\0\0\0\0\0\0\0\0\0\0", wanted7, wantedCharSums);
}

int main(int argc, char** argv)
{
	test();
}