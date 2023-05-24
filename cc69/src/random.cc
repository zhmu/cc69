#include "random.h"
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

boost::mt19937 Random::m_oGenerator;

float
Random::Get(float fMin, float fMax)
{
	typedef boost::uniform_real<float> TDistribution;
	TDistribution u(fMin, fMax);
	boost::variate_generator<TGenerator&, TDistribution> oGen(m_oGenerator, u);
	return oGen();
}

int
Random::Get(int iMin, int iMax)
{
	typedef boost::uniform_int<int> TDistribution;
	TDistribution u(iMin, iMax);
	boost::variate_generator<TGenerator&, TDistribution> oGen(m_oGenerator, u);
	return oGen();
}

/* vim:set ts=2 sw=2: */
