#include <GL/gl.h>
#include "app.h"

template<class PARTICLE>
ParticleEngine<PARTICLE>::ParticleEngine(int iCount)
	: m_poImage(NULL)
{
	for (int i = 0; i < iCount; i++)
		m_oDeadParticle.push_back(PARTICLE());
}

template<class PARTICLE>
ParticleEngine<PARTICLE>::~ParticleEngine()
{
	delete m_poImage;
}

template<class PARTICLE> void
ParticleEngine<PARTICLE>::Initialize()
{
	m_poImage = new Image(g_oApp.GetDataPath() + "/particle.png");
	if (!m_poImage->Load()) {
		std::cerr << "fatal: cannot load particle image\n";
		exit(EXIT_FAILURE);
	}
}

template<class PARTICLE> void
ParticleEngine<PARTICLE>::Evolve()
{
	typename TParticleList::iterator it = m_oActiveParticle.begin();
	while (it != m_oActiveParticle.end()) {
		PARTICLE& oParticle = *it;

		oParticle.Evolve();
		if (oParticle.IsDead()) {
			it = m_oActiveParticle.erase(it);
			m_oDeadParticle.push_back(oParticle);
		} else {
			it++;
		}
	}

	if (m_oActiveParticle.empty())
		OnAllParticlesDead();
}

template<class PARTICLE> PARTICLE&
ParticleEngine<PARTICLE>::AddParticle()
{
	assert(!m_oDeadParticle.empty());
	PARTICLE& oParticle = m_oDeadParticle.front();
	m_oDeadParticle.pop_front();
	m_oActiveParticle.push_back(oParticle);
	return oParticle;
}

template<class PARTICLE> void
ParticleEngine<PARTICLE>::Reset()
{
	for (typename TParticleList::iterator it = m_oActiveParticle.begin(); it != m_oActiveParticle.end(); it++) {
		m_oDeadParticle.push_back(*it);
	}
	m_oActiveParticle.clear();
}

template<class PARTICLE> void
ParticleEngine<PARTICLE>::Render()
{
	glBindTexture(GL_TEXTURE_2D, m_poImage->GetTextureID());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (typename TParticleList::iterator it = m_oActiveParticle.begin(); it != m_oActiveParticle.end(); it++) {
		PARTICLE& oParticle = *it;
		oParticle.Render();
	}

	// Reset the color so our next renderer needn't bother
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}

/* vim:set ts=2 sw=2: */
