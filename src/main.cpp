#include <iostream>
#include <algorithm>

#include "bwt.h"

void testStep1(const char* text, size_t textlen, const std::vector<size_t> wantedResult)
{
	std::cout << text << " " << textlen << "\n";
	std::cout << "wanted: ";
	for (auto i = wantedResult.begin(); i != wantedResult.end(); i++)
	{
		std::cout << *i << " ";
	}
	std::cout << "\n";
	std::vector<size_t> result = step1(text, textlen);
	std::cout << "result: ";
	for (auto i = result.begin(); i != result.end(); i++)
	{
		std::cout << *i << " ";
	}
	std::cout << "\n";
	if (std::equal(result.begin(), result.end(), wantedResult.begin()))
	{
		std::cout << "equal\n";
	}
	else
	{
		std::cout << "NOT EQUAL\n";
	}
}

void test()
{
	//test each step on paper's example
	//indices corrected to start at 0 insted of 1
	std::vector<size_t> wanted { 20, 2, 6, 8, 11, 13, 16 };
	testStep1("amammmasasmasassaara\0", 21, wanted);
}

int main(int argc, char** argv)
{
	test();
}