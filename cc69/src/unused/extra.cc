#include "extra.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <math.h>
#include "app.h"
#include "events.h"

#include "candata.h"

Extra::Extra()
{
}

Extra::~Extra()
{
}

void
Extra::Init()
{
	m_fX = 0.0f;
	m_fY = 0.0f;
	m_fZ = -200.0f;

	m_fRx = 0.0f;
	m_fRy = 0.0f;
	m_fRz = 0.0f;
}

void
Extra::Cleanup()
{
}

void
Extra::Run()
{
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

	/*
	 * Create our viewing frustrum; this is based on OpenGL transformation FAQ
	 * entry 9.085 'How can I make a call to glFrustum() that matches my call to
	 * gluPerspective()?'.
	 */
	float fov = 45.0f;
	float fNear = 10.0f;
	float fFar = 2000.0f;
	float fAspect = (float)g_oApp.GetScreenWidth() / (float)g_oApp.GetScreenHeight();

	float fTop = tan(fov * M_PI / 360.0f) * fNear;
	glFrustum(fAspect * -fTop, fAspect * fTop, -fTop, fTop, fNear, fFar);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

	m_bLeaving = false;
	while(!m_bLeaving) {
		HandleEvents();
		Render();
		SDL_Delay(1000 / g_oApp.GetDesiredFPS());
	}
}

void
Extra::HandleEvents()
{
	Events& oEvents = g_oApp.GetEvents();
	oEvents.Process();

	if (oEvents.CheckAndResetExit()) {
		m_bLeaving = true;
	} else if (oEvents.CheckAndResetStop()) {
		m_bLeaving = true;
	} else if (oEvents.CheckAndResetStop()) {
		m_bLeaving = true;
	} else if (oEvents.CheckAndResetPlus()) {
		m_fRx += 5.0f;
	} else if (oEvents.CheckAndResetMinus()) {
		m_fRx -= 5.0f;
	} else if (oEvents.CheckAndResetLeft()) {
		m_fRy -= 5.0f;
	} else if (oEvents.CheckAndResetRight()) {
		m_fRy += 5.0f;
	}

	m_fRy += 1.0f;
	if (m_fRy >= 360.0f)
		m_fRy = 0.0f;
}

void
Extra::Render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
  glTranslatef(m_fX, m_fY, m_fZ);
	glRotatef(m_fRx, 1, 0, 0);
	glRotatef(m_fRy, 0, 1, 0);
	glRotatef(m_fRy, 0, 0, 1);

	glColor3f(0.8f, 0.8f, 0.8f);

	float fShininess[] = { 50.0f };
	float fSpec[] = { 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, fShininess);
	glMaterialfv(GL_FRONT, GL_SPECULAR, fSpec);

	glEnable(GL_COLOR_MATERIAL);

	// Can
	//glScalef(10.0f, 10.0f, 10.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	for (unsigned int i = 0; i < sizeof(surfaces) / sizeof(surfaces[0]); i++) {
		int idx = surfaces[i];
		float x = vertices[3 * idx];
		float y = vertices[3 * idx + 1];
		float z = vertices[3 * idx + 2];
		glVertex3f(x, y, z);
	}
	glEnd();

	SDL_GL_SwapBuffers();
}

/* vim:set ts=2 sw=2: */
