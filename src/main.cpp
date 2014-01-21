#include <iostream>
#include <algorithm>

#include "bwt.h"

bool testStep1(const char* text, size_t textlen, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step1(text, textlen);
	if (std::equal(result.begin(), result.end(), wantedResult.begin()))
	{
		std::cout << "step 1 equal\n";
		return true;
	}
	else
	{
		std::cout << "step 1 NOT EQUAL\n";
		std::cout << text << " " << textlen << "\n";
		std::cout << "wanted: ";
		for (auto i = wantedResult.begin(); i != wantedResult.end(); i++)
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
		return false;
	}
}

bool testStep2(const char* text, size_t textlen, const std::vector<size_t>& step1result, const std::vector<size_t>& wantedResult)
{
	std::vector<size_t> result = step2(text, textlen, step1result);
	if (std::equal(result.begin(), result.end(), wantedResult.begin()))
	{
		std::cout << "step 2 equal\n";
		return true;
	}
	else
	{
		std::cout << "step 2 NOT EQUAL\n";
		std::cout << text << " " << textlen << "\n";
		std::cout << "wanted: ";
		for (auto i = wantedResult.begin(); i != wantedResult.end(); i++)
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
		return false;
	}
}

void test()
{
	//test each step on paper's example
	//indices corrected to start at 0 insted of 1
	std::vector<size_t> wanted { 20, 2, 6, 8, 11, 13, 16 };
	testStep1("amammmasasmasassaara\0", 21, wanted);
	std::vector<size_t> wanted2 { 1, 3, 18, 7, 12, 9, 14 };
	testStep2("amammmasasmasassaara\0", 21, wanted, wanted2);
}

int main(int argc, char** argv)
{
	test();
}