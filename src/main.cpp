#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <unistd.h>

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
	std::cerr << "Testing in-memory BWT reversibility with data from file \"" << fileName << "\", size " << size << "+1\n";
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

void doTests()
{
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

}

void testInFileBWT(const std::string& fileName)
{
	std::cerr << "Testing in-file BWT against in-memory BWT with file \"" << fileName << "\"\n";
	std::vector<unsigned char> raw = readVectorFromFile<unsigned char>(fileName, true);
	std::vector<unsigned char> result(raw.size(), 0);
	bwt(raw.data(), raw.size(), 255, result.data());
	freeMemory(raw);
	std::string resultFile = getTempFileName();
	bwtInFiles<unsigned char>(fileName, 255, resultFile);
	std::vector<unsigned char> comp = readVectorFromFile<unsigned char>(resultFile, false);
	if (comp.size() != result.size())
	{
		std::cerr << "BWT in file \"" << fileName << "\" NOT equal to in-memory (wrong size: " << comp.size() << " vs " << result.size() << ")\n";; 
	}
	if (std::equal(result.begin(), result.end(), comp.begin()))
	{
		std::cerr << "BWT in file \"" << fileName << "\" equal to in-memory\n"; 
	}
	else
	{
		std::cerr << "BWT in file \"" << fileName << "\" NOT equal to in-memory (wrong values)"; 
	}
	remove(resultFile.c_str());
}

void doInFileTests()
{
	testReversibilityWithFile("minigenome");
	testReversibilityWithFile("genome3");
	testInFileBWT("minigenome");
}

void bwtFromFileInMemory(const std::string& inFile, const std::string& outFile)
{
	std::vector<unsigned char> src = readVectorFromFile<unsigned char>(inFile, true);
	std::vector<unsigned char> dst(src.size(), 0);
	bwt<unsigned char>(src.data(), src.size(), 255, dst.data());
	writeVectorToFile(dst, outFile);
}

void inverseBwtFromFileInMemory(const std::string& inFile, const std::string& outFile)
{
	std::vector<unsigned char> src = readVectorFromFile<unsigned char>(inFile, false);
	std::vector<unsigned char> dst(src.size(), 0);
	inverseBWT<unsigned char>(src.data(), src.size(), 255, dst.data());
	writeVectorToFile(dst, outFile);
}

void printHelp()
{
	std::cerr << "Usage:\n";
	std::cerr << "\tbwt [MODE] -i in_file_name -o out_file_name\n\n";
	std::cerr << "Modes:\n";
	std::cerr << "\t-t run tests\n";
	std::cerr << "\t-m in-memory BWT\n";
	std::cerr << "\t-f in-files BWT\n";
	std::cerr << "\t-u inverse BWT\n";
}

int main(int argc, char** argv)
{
	enum Mode { TEST, IN_MEMORY, IN_FILE, INVERSE, HELP };
	Mode mode = HELP;
	int gotOption;
	std::string inFile;
	std::string outFile;
	while ((gotOption = getopt(argc, argv, "tmfhui:o:")) != -1)
	{
		switch(gotOption)
		{
			case 't':
				mode = TEST;
				break;
			case 'm':
				mode = IN_MEMORY;
				break;
			case 'f':
				mode = IN_FILE;
				break;
			case 'h':
				mode = HELP;
				break;
			case 'u':
				mode = INVERSE;
				break;
			case 'i':
				inFile = optarg;
				break;
			case 'o':
				outFile = optarg;
				break;
			default:
				break;
		}
	}
	if ((inFile == "" || outFile == "") && (mode == IN_MEMORY || mode == IN_FILE || mode == INVERSE))
	{
		mode = HELP;
	}
	switch(mode)
	{
		case TEST:
			std::cerr << "Running tests\n";
			doTests();
			doInFileTests();
			break;
		case IN_MEMORY:
			std::cerr << "Running in-memory BWT from file " << inFile << " to " << outFile << "\n";
			bwtFromFileInMemory(inFile, outFile);
			break;
		case IN_FILE:
			std::cerr << "Running in-file BWT from file " << inFile << " to " << outFile << "\n";
			bwtInFiles<unsigned char>(inFile, 255, outFile);
			break;
		case INVERSE:
			std::cerr << "Running inverse BWT from file " << inFile << " to " << outFile << "\n";
			inverseBwtFromFileInMemory(inFile, outFile);
			break;
		case HELP:
		default:
			printHelp();
			break;
	}
}