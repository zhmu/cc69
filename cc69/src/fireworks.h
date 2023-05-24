#ifndef __FIREWORKS_H__
#define __FIREWORKS_H__

#include "particleengine.h"
#include "particle.h"

class FireworkParticle : public Particle
{
public:
	virtual void Initialize();
	virtual void Evolve();

private:
	//! \brief State
	int m_iState;
};

class Fireworks : public ParticleEngine<FireworkParticle>
{
public:
	Fireworks();
	void Initialize();

protected:
	void Reset();
	void OnAllParticlesDead();
};

#endif /* __FIREWORKS_H__ */
