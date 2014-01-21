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

void test()
{
	//test each step on paper's example
	//indices corrected to start at 0 insted of 1
	std::vector<size_t> wanted { 20, 2, 6, 8, 11, 13, 16 };
	testStep1("amammmasasmasassaara\0", 21, wanted);
	std::vector<size_t> wanted2 { 1, 3, 18, 7, 12, 9, 14 };
	testStep2("amammmasasmasassaara\0", 21, wanted, wanted2);
	std::vector<size_t> wanted3 { 20, 16, 2, 6, 11, 8, 13 };
	testStep3("amammmasasmasassaara\0", 21, wanted2, wanted3);
}

int main(int argc, char** argv)
{
	test();
}