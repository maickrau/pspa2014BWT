#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include "bwt.h"

void testEqualityAndWhine(const char* step, const char* text, size_t textlen, const std::vector<size_t>& result, const std::vector<size_t>& wanted)
{
	if (std::equal(result.begin(), result.end(), wanted.begin()))
	{
		std::cerr << step << " equal\n";
	}
	else
	{
		std::cerr << step << " NOT EQUAL\n";
		std::cerr << text << " " << textlen << "\n";
		std::cerr << "wanted: ";
		for (auto i = wanted.begin(); i != wanted.end(); i++)
		{
			std::cerr << *i << " ";
		}
		std::cerr << "\n";
		std::cerr << "result: ";
		for (auto i = result.begin(); i != result.end(); i++)
		{
			std::cerr << *i << " ";
		}
		std::cerr << "\n";
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
		std::cerr << step << " text equal\n";
	}
	else
	{
		std::cerr << step << " text NOT EQUAL\n";
		std::cerr << "expected:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
//			std::cerr << text1[i];
			std::cerr << (int)(unsigned char)text1[i] << " ";
		}
		std::cerr << "\"\ngot:\n\"";
		for (size_t i = 0; i < textlen; i++)
		{
//			std::cerr << text2[i];
			std::cerr << (int)(unsigned char)text2[i] << " ";
		}
		std::cerr << "\"\n ";
		std::vector<size_t> brokes;
		for (size_t i = 0; i < textlen; i++)
		{
			if (!equal[i])
			{
				std::cerr << "^";
				brokes.push_back(i);
			}
			else
			{
				std::cerr << " ";
			}
		}
		std::cerr << "\n";
		for (size_t i = 0; i < brokes.size(); i++)
		{
			std::cerr << brokes[i] << " ";
		}
		std::cerr << "\n";
	}
}
/*
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
	std::vector<size_t> wantedCharSums(257, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 257; a++)
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
	std::vector<size_t> wantedCharSums(257, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 257; a++)
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
	std::vector<size_t> wantedCharSums(257, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 257; a++)
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
	std::vector<size_t> wantedCharSums(257, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 257; a++)
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

void testSteps5()
{
	const char* testString = "cabadabab\0";
	int testStringLen = 10; //includes trailing \0
	std::vector<size_t> wantedCharSums(257, 0);
	for (int i = 0; i < testStringLen; i++)
	{
		for (int a = testString[i]+1; a < 257; a++)
		{
			wantedCharSums[a]++;
		}
	}
	testCharSums(testString, testStringLen, wantedCharSums);
	std::vector<size_t> wanted { 9, 1, 3, 5, 7 };
	testStep1(testString, testStringLen, wanted);
	std::vector<size_t> wanted2 { 8, 2, 6, 0, 4 };
	testStep2(testString, testStringLen, wanted, wanted2);
	std::vector<size_t> wanted3 { 9, 7, 1, 5, 3 };
	testStep3(testString, testStringLen, wanted2, wanted3);
	std::vector<size_t> wantedR { 1, 9, 3, 7, 5 };
	std::vector<size_t> wanted4 { 2, 4, 3, 1, 0 };
	testStep4(testString, testStringLen, wanted3, wanted4, wantedR);
	std::vector<size_t> wanted5 { 1, 3, 0, 4, 2 };
	testStep5(testString, testStringLen, wanted4, wanted5);
	std::vector<size_t> wanted6 { 9, 7, 5, 1, 3 };
	testStep6(testString, testStringLen, wanted5, wantedR, wanted6);

//	std::vector<size_t> wanted7 { 14, 5, 1, 11 };
//	testStep7(testString, testStringLen, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", wanted6, wantedCharSums, wanted7);
//	testStep8(testString, testStringLen, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", wanted7, wantedCharSums);

}
*/
void testBWT2()
{
	const char testString[7] { 2, 3, 4, 3, 5, 1, 0 };
	const char expected[7] { 1, 5, 0, 2, 4, 3, 3 };
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
	const unsigned char testString[10] { 202, 255, 73, 151, 76, 166, 226, 235, 123, 0 };
	const unsigned char expected[10] { 123, 255, 151, 235, 73, 76, 0, 166, 226, 202 };
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
	std::cerr << "testing reversibility with \"" << string << "\"\n";
	size_t stringLen = strlen(string)+1; //includes trailing \0
	char* inverse = new char[stringLen]();
	char* result = new char[stringLen]();
	bwt(string, stringLen, inverse);
/*	std::cerr << "\"";
	for (size_t i = 0; i < stringLen; i++)
	{
		std::cerr << inverse[i];
	}
	std::cerr << "\"\n";*/
	inverseBWT(inverse, stringLen, result);
	testEqualityAndWhine("BWT reversibility", string, result, stringLen);
	delete [] result;
	delete [] inverse;
}

bool isReversible(const char* string, size_t stringLen)
{
	char* inverse = new char[stringLen]();
	char* result = new char[stringLen]();
	bwt(string, stringLen, inverse);
	inverseBWT(inverse, stringLen, result);
	bool equal = true;
	for (size_t i = 0; i < stringLen; i++)
	{
		if (result[i] != string[i])
		{
			equal = false;
			break;
		}
	}
	delete [] result;
	delete [] inverse;
	return equal;
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
	std::cerr << "Testing reversibility of " << iterations << " random strings with length " << size << "\n";
	unsigned char* string = new unsigned char[size];
	unsigned char* inverse = new unsigned char[size];
	unsigned char* result = new unsigned char[size];
	for (int iteration = 0; iteration < iterations; iteration++)
	{
		for (size_t i = 0; i < size-1; i++)
		{
			string[i] = (rand() % 255)+1; //anything except \0
		}
		string[size-1] = '\0';
		bwt(string, size, 255, inverse);
		inverseBWT(inverse, size, 255, result);
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
			std::cerr << "random string reversibility not equal: \n\"";
			for (size_t i = 0; i < size; i++)
			{
				std::cerr << string[i];
			}
			std::cerr << "\"\n(";
			for (size_t i = 0; i < size; i++)
			{
				std::cerr << (int)(unsigned char)string[i] << " ";
			}
			std::cerr << ")\n";
			auto charSum1 = charSums(string, size, 255);
			auto charSum2 = charSums(inverse, size, 255);
			auto charSum3 = charSums(result, size, 255);
			for (size_t i = 0; i < 255; i++)
			{
				std::cerr << i << "\n";
				if (charSum1[i] != charSum2[i])
				{
					std::cerr << "string/inverse charsums not equal\n";
				}
				if (charSum1[i] != charSum3[i])
				{
					std::cerr << "string/result charsums not equal\n";
				}
			}
			throw 1;
		}
	}
	delete [] string;
	delete [] inverse;
	delete [] result;
}

// big broken string was found by testReversibilityWithLotsOfRandoms and used to break the algorithm
void testReversibilityWithBigBroken()
{
	const unsigned char bigBroken[500] = { 81, 175, 211, 203, 219, 40, 199, 87, 191, 57, 77, 103, 74, 133, 121, 189, 193, 134, 184, 171, 186, 166, 139, 29, 189, 153, 107, 240, 24, 3, 75, 105, 49, 157, 179, 139, 196, 123, 97, 4, 51, 174, 106, 124, 178, 227, 184, 115, 232, 112, 31, 34, 150, 169, 62, 83, 193, 40, 194, 217, 169, 13, 66, 89, 170, 116, 228, 237, 110, 196, 240, 160, 114, 218, 156, 36, 189, 84, 23, 37, 68, 180, 70, 89, 220, 3, 171, 158, 42, 236, 119, 83, 249, 56, 171, 35, 171, 15, 143, 26, 211, 128, 57, 196, 90, 212, 104, 150, 168, 126, 58, 107, 50, 254, 195, 141, 1, 237, 43, 169, 217, 33, 251, 82, 88, 39, 116, 131, 180, 131, 28, 135, 3, 211, 203, 219, 40, 51, 240, 207, 48, 169, 58, 224, 167, 252, 109, 167, 233, 24, 80, 194, 56, 203, 148, 144, 113, 135, 146, 37, 10, 45, 44, 139, 255, 246, 229, 39, 168, 85, 117, 215, 253, 174, 183, 164, 42, 163, 75, 146, 186, 27, 212, 242, 101, 104, 2, 213, 238, 19, 121, 248, 63, 164, 131, 62, 26, 105, 228, 193, 189, 89, 152, 59, 8, 206, 222, 176, 241, 169, 67, 171, 195, 150, 157, 167, 253, 30, 251, 235, 48, 116, 99, 110, 25, 102, 44, 50, 78, 16, 243, 138, 104, 11, 196, 238, 89, 163, 159, 74, 76, 97, 116, 142, 246, 145, 180, 115, 174, 175, 94, 222, 35, 65, 203, 59, 166, 118, 236, 243, 133, 223, 252, 109, 105, 193, 91, 65, 227, 121, 138, 174, 89, 254, 60, 79, 15, 239, 193, 188, 158, 159, 26, 64, 95, 101, 123, 5, 218, 103, 119, 223, 197, 242, 76, 173, 179, 38, 238, 150, 31, 247, 68, 246, 117, 254, 70, 131, 109, 134, 191, 138, 164, 216, 202, 3, 188, 196, 134, 151, 170, 124, 245, 238, 111, 192, 155, 34, 101, 9, 56, 131, 1, 250, 122, 244, 249, 63, 247, 229, 68, 182, 112, 104, 14, 185, 106, 202, 125, 112, 97, 39, 235, 213, 21, 217, 149, 47, 123, 249, 56, 178, 252, 183, 44, 245, 43, 164, 179, 34, 138, 246, 215, 121, 94, 229, 50, 72, 47, 46, 183, 15, 211, 34, 227, 231, 251, 120, 149, 245, 240, 76, 167, 108, 3, 82, 97, 173, 246, 20, 206, 255, 138, 38, 120, 103, 138, 41, 174, 184, 86, 228, 198, 41, 134, 41, 143, 1, 32, 36, 245, 16, 112, 28, 124, 241, 109, 92, 30, 99, 239, 236, 98, 121, 145, 89, 223, 27, 129, 14, 82, 214, 113, 151, 254, 246, 191, 13, 246, 94, 176, 235, 109, 159, 134, 104, 16, 243, 68, 46, 213, 51, 153, 56, 43, 42, 16, 10, 195, 145, 150, 148, 103, 8, 43, 229, 125, 105, 113, 243, 198, 33, 222, 178, 191, 101, 154, 207, 215, 221, 252, 172, 143, 149, 99, 185, 62, 115, 66, 0};
	unsigned char* inverse = new unsigned char[500]();
	unsigned char* result = new unsigned char[500]();
	bwt(bigBroken, 500, 255, inverse);
	inverseBWT(inverse, 500, 255, result);
	testEqualityAndWhine("BWT reversibility with big broken", (char*)bigBroken, (char*)result, 500);
	delete [] result;
	delete [] inverse;
}

void doTests()
{
//	testSteps();
//	testSteps2();
//	testSteps3();
//	testSteps4();

//	std::cerr << "0\n";
//	testSteps5();
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
	testReversibility("cabadabab");

	testReversibilityWithBigBroken();

	testReversibilityWithLotsOfRandoms(200, 10);
	testReversibilityWithLotsOfRandoms(200, 50);
	testReversibilityWithLotsOfRandoms(10, 5000);
	testReversibilityWithLotsOfRandoms(1, 100000);

//	testReversibilityWithLotsOfRandoms(60, 10000);
/*	testReversibilityWithLotsOfRandoms(6, 100000);
	testReversibilityWithLotsOfRandoms(1, 250000);
	testReversibilityWithLotsOfRandoms(1, 250000);
	testReversibilityWithLotsOfRandoms(1, 275000);
	testReversibilityWithLotsOfRandoms(1, 300000);
	testReversibilityWithLotsOfRandoms(1, 400000);
	testReversibilityWithLotsOfRandoms(1, 500000);
	testReversibilityWithLotsOfRandoms(1, 1000000);
	testReversibilityWithLotsOfRandoms(1, 5000000);*/
}

void bwtFromFileInMemory(const char* fileName)
{
	std::ifstream file(fileName, std::ifstream::binary);
	file.seekg(0, std::ifstream::end);
	size_t size = file.tellg();
	std::cerr << size;
	file.seekg(0, std::ifstream::beg);
	char* source = new (std::nothrow) char[size+1](); //+1 for trailing \0
	char* dest = new (std::nothrow) char[size+1]();
	if (source == NULL || dest == NULL)
	{
		file.close();
		std::cerr << "Could not allocate enough memory for in-memory BWT";
		delete [] source;
		delete [] dest;
		return;
	}
	file.read(source, size);
	file.close();
	bwt(source, size+1, dest); //+1 for trailing \0
	std::cout.write(dest, size+1); //+1 because there's a \0 in the middle
	delete [] source;
	delete [] dest;
}

void inverseBwtFromFileInMemory(const char* fileName)
{
	std::ifstream file(fileName, std::ifstream::binary);
	file.seekg(0, std::ifstream::end);
	size_t size = file.tellg();
	std::cerr << size;
	file.seekg(0, std::ifstream::beg);
	char* source = new (std::nothrow) char[size](); //no +1 because there's a \0 in the middle
	char* dest = new (std::nothrow) char[size]();
	if (source == NULL || dest == NULL)
	{
		file.close();
		std::cerr << "Could not allocate enough memory for in-memory BWT";
		delete [] source;
		delete [] dest;
		return;
	}
	file.read(source, size);
	file.close();
	inverseBWT(source, size, dest);
	std::cout.write(dest, size);
	delete [] source;
	delete [] dest;
}

void testReversibilityWithFile(const char* fileName)
{
	std::ifstream file(fileName, std::ifstream::binary);
	file.seekg(0, std::ifstream::end);
	size_t size = file.tellg();
//	std::cerr << size;
	file.seekg(0, std::ifstream::beg);
	char* source = new (std::nothrow) char[size+1](); //+1 for trailing \0
	file.read(source, size);
	file.close();
	std::cerr << "Testing reversibility of file \"" << fileName << "\", size " << size << "+1\n";
	if (isReversible(source, size+1))
	{
		std::cerr << "file \"" << fileName << "\" reversible\n";
	}
	else
	{
		std::cerr << "file \"" << fileName << "\" NOT reversible\n";
	}
	delete [] source;
}

int main(int argc, char** argv)
{
	doTests();
	testReversibilityWithFile("minigenome");
//	bwtFromFileInMemory("minigenome");
//	inverseBwtFromFileInMemory("result.out");
}