#include <iostream>
#include <algorithm>
#include <cstring>

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

void testEqualityAndWhine(const char* step, const char* text1, const char* text2, size_t textlen)
{
	bool right = true;
	std::vector<bool> equal(textlen, true);
	//no strcmp/strncmp because there will be \0 inside the strings
	for (size_t i = 0; i < textlen; i++)
	{
		if (text1[i] != text2[i])
		{
			right = false;
			equal[i] = false;
		}
	}
	if (right)
	{
		std::cout << step << " text equal\n";
	}
	else
	{
		std::cout << step << " text NOT EQUAL\n";
		std::cout << "expected:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << text1[i];
		}
		std::cout << "\"\ngot:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
			std::cout << text2[i];
		}
		std::cout << "\"\n ";
		for (size_t i = 0; i < textlen; i++)
		{
			if (!equal[i])
			{
				std::cout << "^";
			}
			else
			{
				std::cout << " ";
			}
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

void testStep4(const char* text, size_t textlen, const std::vector<size_t>& step3result, const std::vector<size_t>& wanted4, const std::vector<size_t>& wantedR)
{
	auto result = step4(text, textlen, step3result);
	testEqualityAndWhine("step 4 S\'", text, textlen, result.first, wanted4);
	testEqualityAndWhine("step 4 R", text, textlen, result.second, wantedR);
}

void testStep5(const char* text, size_t textlen, const std::vector<size_t>& Sprime, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step5(Sprime);
	testEqualityAndWhine("step 5", text, textlen, result, wantedResult);
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

void testBWTDirectly(const char* text, size_t textlen, const std::vector<size_t>& data, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = bwtDirectly(data);
	testEqualityAndWhine("bwt directly", text, textlen, result, wantedResult);
}

void testStep7(const char* text, size_t textlen, const char* wantedText, const std::vector<size_t>& step6result, const std::vector<size_t>& charSums, const std::vector<size_t>& wantedResult)
{
	char* resultString = new char[textlen](); //initialized to \0
	std::vector<size_t> result = step7(text, textlen, step6result, resultString, charSums);
	testEqualityAndWhine("step 7", wantedText, resultString, textlen);
	delete [] resultString;
	testEqualityAndWhine("step 7", text, textlen, result, wantedResult);
}

void testStep8(const char* text, size_t textlen, const char* wantedText, const std::vector<size_t>& step6result, const std::vector<size_t>& charSums)
{
	char* resultString = new char[textlen](); //initialized to \0
	step8(text, textlen, step6result, resultString, charSums);
	testEqualityAndWhine("step 8", wantedText, resultString, textlen);
	delete [] resultString;
}

void testSteps()
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
	std::vector<size_t> testBWT { 5, 2, 6, 4, 1, 3, 0 };
	std::vector<size_t> testBWTResult { 3, 4, 5, 1, 6, 0, 2 };
	testBWTDirectly(testString, testStringLen, testBWT, testBWTResult);
	std::vector<size_t> wanted { 20, 2, 6, 8, 11, 13, 16 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 1, 3, 18, 7, 12, 9, 14 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 20, 16, 2, 6, 11, 8, 13 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wantedR { 2, 20, 6, 13, 11, 16 };
	std::vector<size_t> wanted4 { 2, 3, 4, 3, 5, 1, 0 };
	testStep4(testString, testStringLen, wanted3, wanted4, wantedR);
	std::vector<size_t> wanted5 { 1, 5, 0, 2, 4, 3, 3 };
	testStep5(testString, testStringLen, wanted4, wanted5);
	std::vector<size_t> wanted6 { 20, 16, 2, 6, 11, 13, 13 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);
	std::vector<size_t> wanted7 { 1, 3, 18, 12, 12, 9, 14 };
	testStep7(testString, testStringLen, "\0r\0\0\0\0\0\0\0\0amsmaasaaaa", wanted6, wantedCharSums, wanted7);
	testStep8(testString, testStringLen, "a\0s\0mammss\0\0\0\0\0\0\0\0\0\0\0", wanted7, wantedCharSums);
}

void testSteps2()
{
	char testString[7] { 2, 3, 4, 3, 5, 1, 0 };
	int testStringLen = 7;
	std::vector<size_t> wantedCharSums(256, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 256; a++)
		{
			wantedCharSums[a]++;
		}
	}
	testCharSums(testString, testStringLen, wantedCharSums);
	std::vector<size_t> testBWT { 1, 0 };
	std::vector<size_t> testBWTResult { 1, 0 };
	testBWTDirectly(testString, testStringLen, testBWT, testBWTResult);

	std::vector<size_t> wanted { 6, 3 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 2, 4 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 6, 3 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wantedR { 3, 6 };
	std::vector<size_t> wanted4 { 1, 0 };
	testStep4(testString, testStringLen, wanted3, wanted4, wantedR);
	std::vector<size_t> wanted5 { 1, 0 };
	testStep5(testString, testStringLen, wanted4, wanted5);
	std::vector<size_t> wanted6 { 6, 3 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);
	std::vector<size_t> wanted7 { 2, 4 };
	char result7[7] { 0, 5, 0, 0, 0, 3, 3 };
	char result8[7] { 1, 0, 0, 2, 4, 0, 0 };
	testStep7(testString, testStringLen, result7, wanted6, wantedCharSums, wanted7);
	testStep8(testString, testStringLen, result8, wanted7, wantedCharSums);
}

void testSteps3()
{
	const char* testString = "asfjnsdfjklsjkl\0";
	int testStringLen = 16; //includes trailing \0
	std::vector<size_t> wantedCharSums(256, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 256; a++)
		{
			wantedCharSums[a]++;
		}
	}
	testCharSums(testString, testStringLen, wantedCharSums);
	std::vector<size_t> wanted { 15, 6, 2, 12 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 14, 5, 1, 11 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 15, 6, 2, 12 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wantedR { 2, 12, 6, 15 };
	std::vector<size_t> wanted4 { 2, 1, 3, 0 };
	testStep4(testString, testStringLen, wanted3, wanted4, wantedR);
	std::vector<size_t> wanted5 { 3, 2, 0, 1 };
	testStep5(testString, testStringLen, wanted4, wanted5);
	std::vector<size_t> wanted6 { 15, 6, 2, 12 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);
	std::vector<size_t> wanted7 { 14, 5, 1, 11 };
	testStep7(testString, testStringLen, "\0\0\0\0\0\0\0\0\0\0k\0\0nal", wanted6, wantedCharSums, wanted7);
	testStep8(testString, testStringLen, "l\0sdssffjj\0kj\0\0\0", wanted7, wantedCharSums);
}

void testSteps4()
{
	const char* testString = "egargdfafddfhahdfdhf\0";
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
	std::vector<size_t> wanted { 20, 2, 7, 13, 5, 9, 15, 17 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 6, 8, 16, 1, 12, 14, 18, 3 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 20, 7, 13, 2, 9, 5, 15, 17 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wantedR { 2, 9, 15, 5, 13, 17, 20 };
	std::vector<size_t> wanted4 { 3, 5, 1, 4, 2, 5, 6, 0 };
	testStep4(testString, testStringLen, wanted3, wanted4, wantedR);
	std::vector<size_t> wanted5 { 6, 5, 4, 0, 1, 3, 2, 5 };
	testStep5(testString, testStringLen, wanted4, wanted5);
	std::vector<size_t> wanted6 { 20, 17, 13, 2, 9, 5, 15, 17 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);

	std::vector<size_t> wanted7 { 14, 5, 1, 11 };
	testStep7(testString, testStringLen, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", wanted6, wantedCharSums, wanted7);
//	testStep8(testString, testStringLen, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", wanted7, wantedCharSums);

}

void testBWT2()
{
	char testString[7] { 2, 3, 4, 3, 5, 1, 0 };
	char expected[7] { 1, 5, 0, 2, 4, 3, 3 };
	int testStringLen = 7;
	char* result = new char[testStringLen]();
	bwt(testString, testStringLen, result);
	testEqualityAndWhine("BWT test 2", expected, result, testStringLen);
	delete [] result;
}

void testBWT()
{
	const char* testString = "amammmasasmasassaara\0";
	const char* expected = "ars\0mammssamsmaasaaaa";
	int testStringLen = 21; //includes trailing \0
	char* result = new char[testStringLen]();
	bwt(testString, testStringLen, result);
	testEqualityAndWhine("BWT test", expected, result, testStringLen);
	delete [] result;
}

void testBWT3()
{
	const char* testString = "egargdfafddfhahdfdhf\0";
	const char* expected = "ffhgfghdf\0hdadderfada";
	int testStringLen = 21; //includes trailing \0
	char* result = new char[testStringLen]();
	bwt(testString, testStringLen, result);
	testEqualityAndWhine("BWT test 3", expected, result, testStringLen);
	delete [] result;
}

void testBWT4()
{
	unsigned char testString[10] { 202, 255, 73, 151, 76, 166, 226, 235, 123, 0 };
	unsigned char expected[10] { 123, 255, 151, 235, 73, 76, 0, 166, 226, 202 };
	int testStringLen = 10; //includes trailing \0
	unsigned char* result = new unsigned char[testStringLen]();
	bwt(testString, testStringLen, 255, result);
	testEqualityAndWhine("BWT test 4", (char*)expected, (char*)result, testStringLen);
	delete [] result;
}

void testBWT5()
{
	const char* testString = "cabadabab\0";
	const char* expected = "bbdcbaaa\0a";
	int testStringLen = 10; //includes trailing \0
	char* result = new char[testStringLen]();
	bwt(testString, testStringLen, result);
	testEqualityAndWhine("BWT test 5", expected, result, testStringLen);
	delete [] result;
}

void testReversibility(const char* string)
{
	std::cout << "testing reversibility with \"" << string << "\"\n";
	size_t stringLen = strlen(string)+1; //includes trailing \0
	char* inverse = new char[stringLen]();
	char* result = new char[stringLen]();
	bwt(string, stringLen, inverse);
/*	std::cout << "\"";
	for (size_t i = 0; i < stringLen; i++)
	{
		std::cout << inverse[i];
	}
	std::cout << "\"\n";*/
	inverseBWT(inverse, stringLen, result);
	testEqualityAndWhine("BWT reversibility", string, result, stringLen);
	delete [] result;
	delete [] inverse;
}

void testInverse()
{
	const char* testString = "ars\0mammssamsmaasaaaa";
	const char* expected = "amammmasasmasassaara\0";
	int testStringLen = 21; //includes trailing \0
	char* result = new char[testStringLen]();
	inverseBWT(testString, testStringLen, result);
	testEqualityAndWhine("BWT inverse test", expected, result, testStringLen);
	delete [] result;
}

void testReversibilityWithLotsOfRandoms(int iterations, size_t size)
{
	std::cout << "Testing reversibility of " << iterations << " random strings with length " << size << "\n";
	char* string = new char[size];
	char* inverse = new char[size];
	char* result = new char[size];
	for (int iteration = 0; iteration < iterations; iteration++)
	{
		for (size_t i = 0; i < size; i++)
		{
			string[i] = (rand() % 255)+1; //anything except \0
		}
		string[size-1] = '\0';
		bwt(string, size, inverse);
		inverseBWT(inverse, size, result);
		bool equal = true;
		for (size_t i = 0; i < size; i++)
		{
			if (string[i] != result[i])
			{
				equal = false;
				break;
			}
		}
		if (!equal)
		{
			std::cout << "random string reversibility not equal: \n\"";
			for (size_t i = 0; i < size; i++)
			{
				std::cout << string[i];
			}
			std::cout << "\"\n(";
			for (size_t i = 0; i < size; i++)
			{
				std::cout << (int)(unsigned char)string[i] << " ";
			}
			std::cout << ")\n";
		}
	}
	delete [] string;
	delete [] inverse;
	delete [] result;
}

int main(int argc, char** argv)
{
//	testSteps();
//	testSteps2();
//	testSteps3();
//	testSteps4();
	testBWT();
	testBWT2();
	testBWT3();
	testBWT4();
	testBWT5();
	testInverse();
	testReversibility("hello world");
	testReversibility("asfjnsdfjklsjkl");
	testReversibility("egargdfafddfhahdfdhf");
	testReversibility("qa<dgsd");
	testReversibility("erhetahsga");
	testReversibility("23t34qy3qt3qat4a");
	testReversibility("aaaaaaaaaaaaaaa");
	testReversibility("rtj7yt5uj56uy5twesdgs<df");
	testReversibilityWithLotsOfRandoms(200, 10);
	testReversibilityWithLotsOfRandoms(200, 50);
	testReversibilityWithLotsOfRandoms(10, 5000);
}