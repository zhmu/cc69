#ifndef __STRINGLIBRARY_H__
#define __STRINGLIBRARY_H__

#include <string>

class StringLibrary
{
public:
	/*! \brief Naturally compares two strings by name
	 *  \param sA First string
	 *  \param sB Second string
	 *  \returns <0 if sA < sB, >0 if sA > sB, 0 if sA == sB
	 */
	static int NaturalCompare(const std::string& sA, const std::string& sB);

protected:
	/*! \brief Compares a series of digits
	 *  \param pA Source string
	 *  \param pB Destination string string
	 *  \returns 1 if pA < pB, 1 if pA > pB, 0 if pA == pB
	 */
	static int CompareDigits(const char* pA, const char* pB);

	//! \brief Checks whether a given character is a digit
	static bool IsDigit(char c) { return c >= '0' && c <= '9'; }

	//! \brief Checks whether a given character is whitespace
	static bool IsSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

};

#endif /* __STRINGLIBRARY_H__ */
