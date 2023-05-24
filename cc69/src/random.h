#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <boost/random/mersenne_twister.hpp>

class Random
{
public:
	/*! \brief Retrieve a random number
	 *  \param fMin Minimum value to return
	 *  \param fMax Maximum value to return
	 *  \returns Random number r, where fMin <= r <= fMax
	 */
	static float Get(float fMin, float fMax);
	static int Get(int fMin, int fMax);

private:
	typedef boost::mt19937 TGenerator;
	static TGenerator m_oGenerator;
};

#endif /* __RANDOM_H__ */
