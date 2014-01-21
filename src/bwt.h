#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>

std::vector<size_t> step1(const char* text, size_t textLen);
std::vector<size_t> step1(const unsigned char* text, size_t textLen);
std::vector<size_t> step2(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft);
std::vector<size_t> step2(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft);

#endif