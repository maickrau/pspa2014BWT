#ifndef bwt_h
#define bwt_h

#include <vector>
#include <cstdlib>

std::vector<size_t> charSums(const char* text, size_t textLen);
std::vector<size_t> charSums(const unsigned char* text, size_t textLen);

std::vector<size_t> step1(const char* text, size_t textLen);
std::vector<size_t> step1(const unsigned char* text, size_t textLen);
std::vector<size_t> step2(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft);
std::vector<size_t> step2(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft);
std::vector<size_t> step3(const char* text, size_t textLen, const std::vector<size_t>& LMSRight);
std::vector<size_t> step3(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSRight);
std::vector<size_t> step6(const std::vector<size_t>& BWTprime, const std::vector<size_t>& R);
std::vector<size_t> step7(const unsigned char* text, size_t textLen, const std::vector<size_t>& LMSLeft, unsigned char* result, const std::vector<size_t>& charSums);
std::vector<size_t> step7(const char* text, size_t textLen, const std::vector<size_t>& LMSLeft, char* result, const std::vector<size_t>& charSums);

#endif