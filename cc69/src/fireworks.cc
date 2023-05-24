#include "fireworks.h"
#include "random.h"
#include "app.h"

Fireworks::Fireworks()
	: ParticleEngine<FireworkParticle>(100)
{
}

void
Fireworks::Initialize()
{
	ParticleEngine<FireworkParticle>::Initialize();
	Reset();
}

void
Fireworks::Reset()
{
	ParticleEngine<FireworkParticle>::Reset();

	float fW = g_oApp.GetScreenWidth() / 2.0f;
	float fInitialX = Random::Get(-fW, fW);
	float fInitialSpeed = (fW - fInitialX) / 100.0f;

	for (int i = 0; i < 100; i++) {
		FireworkParticle& oParticle = AddParticle();
		oParticle.Initialize();

		oParticle.SetPosition(fInitialX, 0.0f, 0.0f);
		if (fInitialX >= 0.0f)
			oParticle.SetSpeed(-2.5f, -5.0f, 0.0f);
		else
			oParticle.SetSpeed(2.5f, -5.0f, 0.0f);
	}
}

void
Fireworks::OnAllParticlesDead()
{
	Reset();
}

void
FireworkParticle::Evolve()
{
	Particle::Evolve();

	// When showing the explosion, slightly fade it out each frame
	if (m_iState == 1)
		SetAlpha(m_fA - 0.02f);

	if (!IsDead())
		return;

	m_iState++;
	switch(m_iState) {
		case 1: { // explosion
			SetGravity(0.04f);
			float x = Random::Get(-2.0f, 2.0f);
			float y = Random::Get(-2.0f, 2.0f);
			SetSpeed( x, y, 0.0f);
			SetSize(3.0f);
			SetColor(
				Random::Get(0.0f, 1.0f),
				Random::Get(0.0f, 1.0f),
				Random::Get(0.0f, 1.0f)
			);
			SetLife(10.0f, 0.2f);
			break;
		}
	}
}

void
FireworkParticle::Initialize()
{
//	SetPosition(0.0f, 0.0f, 0.0f);
//	SetSpeed(2.5f, -5.0f, 0.0f);
	SetColor(1.0f, 1.0f, 1.0f);
	SetSize(5.0f);
	SetAlpha(1.0f);
	SetLife(100.0f, 1.0f);
	SetGravity(0.03f);
	m_iState = 0;
}

/* vim:set ts=2 sw=2: */
