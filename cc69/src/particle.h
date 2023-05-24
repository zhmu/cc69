#ifndef __PARTICLE_H__
#define __PARTICLE_H__

//! \brief A single particle
class Particle {
public:

	//! \brief Is the particle dead?
	bool IsDead() const { return m_fLifetime <= 0.0f; }

	//! \brief Evolves the particle to the next level
	virtual void Evolve();

	//! \brief Renders the particle
	void Render() const;

	//! \brief Change the particle size
	void SetSize(float fSize) {
		m_fSize = fSize;
	}

	//! \brief Change the particle position
	void SetPosition(float fX, float fY, float fZ) {
		m_fX = fX; m_fY = fY; m_fZ = fZ;
	}

	//! \brief Change the particle color
	void SetColor(float fR, float fG, float fB) {
		m_fR = fR; m_fG = fG; m_fB = fB;
	}

	//! \brief Change the particle alpha value
	void SetAlpha(float fA) {
		m_fA = fA;
	}

	//! \brief Change the particle speed
	void SetSpeed(float fX, float fY, float fZ) {
		m_fXi = fX; m_fYi = fY; m_fZi = fZ;
	}

	//! \brief Change the particle gravity
	void SetGravity(float fGravity) {
		m_fGravity = fGravity;
	}

	//! \brief Change the particle lifetime and decay
	void SetLife(float fLifetime, float fDecay) {
		m_fLifetime = fLifetime; m_fDecay = fDecay;
	}

protected:
	//! \brief X coordinate
	float m_fX;

	//! \brief Y coordinate
	float m_fY;

	//! \brief Z coordinate
	float m_fZ;

	//! \brief Red color value
	float m_fR;

	//! \brief Green color value
	float m_fG;

	//! \brief Blue color value
	float m_fB;

	//! \brief Alpha value
	float m_fA;

	//! \brief X direction
	float m_fXi;

	//! \brief Y direction
	float m_fYi;

	//! \brief Z direction
	float m_fZi;

	//! \brief Lifetime of this particle
	float m_fLifetime;

	//! \brief Decay value (how fast it will decay)
	float m_fDecay;

	//! \brief Particle gravity (how much it will pull down)
	float m_fGravity;

	//! \brief Particle size
	float m_fSize;
};

#endif /* __PARTICLE_H__ */
