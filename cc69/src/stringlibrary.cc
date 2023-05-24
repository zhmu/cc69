#include "stringlibrary.h"
#include <assert.h>
#include <string>

int
StringLibrary::CompareDigits(const char* pA, const char* pB)
{
	for (/* nothing */; /* nothing */; pA++, pB++) {
		// Stop on the first non-digit
		if (!IsDigit(*pA) && !IsDigit(*pB))
			return 0;

		// First string without a digit loses; this also handles \0
		if (!IsDigit(*pA))
			return -1;
		if (!IsDigit(*pB))
			return 1;
		// OK, both strings, both digits; may the highest one win
		if (*pA < *pB) {
			return -1;
		} else if (*pA > *pB) {
			return 1;
		}
	}
	/* NOTREACHED */
	assert(0);
	return 0;
}

int
StringLibrary::NaturalCompare(const std::string& sA, const std::string& sB)
{
	const char* pA = sA.c_str();
	const char* pB = sB.c_str();
	for (/* nothing */; /* nothing */; pA++, pB++) {
		// Ignore whitespace
		while (IsSpace(*pA))
			pA++;
		while (IsSpace(*pB))
			pB++;

		// Handle series of digits
		if (IsDigit(*pA) && IsDigit(*pB)) {
			int iResult = CompareDigits(pA, pB);
			if (iResult != 0)
				return iResult;
		}

		// Compare the next char
		if (*pA < *pB)
			return -1;
		else if (*pA > *pB)
			return 1;
		else if (*pA == '\0' /* we know *pA == *pB at this point */)
			return 0;
	}

	/* NOTREACHED */
	assert(0);	
	return 0;
}

/* vim:set ts=2 sw=2: */
