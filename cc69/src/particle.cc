#include "particle.h"
#include <GL/gl.h>

void
Particle::Evolve()
{
	m_fX += m_fXi;
	m_fY += m_fYi;
	m_fZ += m_fZi;
	m_fLifetime -= m_fDecay;
	m_fYi += m_fGravity;
}

void
Particle::Render() const
{
	glColor4f(m_fR, m_fG, m_fB, m_fA);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(m_fX + m_fSize, m_fY + m_fSize, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(m_fX - m_fSize, m_fY + m_fSize, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(m_fX + m_fSize, m_fY - m_fSize, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(m_fX - m_fSize, m_fY - m_fSize, 0.0f);
	glEnd();
}

/* vim:set ts=2 sw=2: */
