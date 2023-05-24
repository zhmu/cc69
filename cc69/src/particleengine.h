#ifndef __PARTICLE_ENGINE_H__
#define __PARTICLE_ENGINE_H__

#include <list>
#include "image.h"

class Image;

template<class PARTICLE> class ParticleEngine
{
public:
	/*! \brief Initializes the particle engine
	 *  \param iCount Number of particles to maintain
	 */
	ParticleEngine(int iCount);

	//! \brief Destroys the particle engine
	~ParticleEngine();

	//! \brief Evolves the particle engine to the next phase
	virtual void Evolve();

	//! \brief Initializes the particle system
	virtual void Initialize();

	//! \brief Renders all particles
	void Render();

protected:
	//! \brief Resets all particles to dead state
	virtual void Reset();

	/*! \brief Called when all particles are dead
	 *
	 *  This is only called after Evolve() kills the last particle.
	 */
	virtual void OnAllParticlesDead() { }

	//! \brief Adds a new particle
	PARTICLE& AddParticle();

private:
	typedef std::list<PARTICLE> TParticleList;

	//! \brief Active particles
	TParticleList m_oActiveParticle;

	//! \brief Dead particles
	TParticleList m_oDeadParticle;

	//! \brief Particle image
	Image* m_poImage;
};

#include "particleengine.inl"

#endif /* __PARTICLE_ENGINE_H__ */
