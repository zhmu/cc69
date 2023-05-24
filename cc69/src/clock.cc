#include "clock.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <assert.h>
#include "app.h"
#include "image.h"
#include "events.h"

Clock::Clock()
	: m_poBackgroundImage(NULL), m_poImage(NULL), m_poTallImage(NULL), m_poTalk1Image(NULL), m_poTalk2Image(NULL), m_poTalk3Image(NULL), m_bTurbo(false)
{
}

Clock::~Clock()
{
	assert(m_poImage == NULL);
	assert(m_poTallImage == NULL);
	assert(m_poBackgroundImage == NULL);
  assert(m_poTalk1Image == NULL);
  assert(m_poTalk2Image == NULL);
  assert(m_poTalk3Image == NULL);
}

void
Clock::Init()
{
	m_poBackgroundImage = new Image(g_oApp.GetDataPath() + "/clock_background.png");
	if (!m_poBackgroundImage->Load()) {
		std::cerr << "fatal: cannot load clock background image\n";
		exit(EXIT_FAILURE);
	}
	m_poImage = new Image(g_oApp.GetDataPath() + "/clock_icon.png");
	if (!m_poImage->Load()) {
		std::cerr << "fatal: cannot load clock icon image\n";
		exit(EXIT_FAILURE);
	}
	m_poTallImage = new Image(g_oApp.GetDataPath() + "/clock_hand.png");
	if (!m_poTallImage->Load()) {
		std::cerr << "fatal: cannot load clock hand image\n";
		exit(EXIT_FAILURE);
	}
  m_poTalk1Image = new Image(g_oApp.GetDataPath() + "/clock_talk1.png");
  if (!m_poTalk1Image->Load()) {
    std::cerr << "fatal: cannot load talk 1 image\n";
    exit(EXIT_FAILURE);
  }
  m_poTalk2Image = new Image(g_oApp.GetDataPath() + "/clock_talk2.png");
  if (!m_poTalk2Image->Load()) {
    std::cerr << "fatal: cannot load talk 2 image\n";
    exit(EXIT_FAILURE);
  }
  m_poTalk3Image = new Image(g_oApp.GetDataPath() + "/clock_talk3.png");
  if (!m_poTalk3Image->Load()) {
    std::cerr << "fatal: cannot load talk 3 image\n";
    exit(EXIT_FAILURE);
  }
}

void
Clock::Cleanup()
{
	delete m_poImage;
	delete m_poTallImage;
	delete m_poBackgroundImage;
  delete m_poTalk1Image;
  delete m_poTalk2Image;
  delete m_poTalk3Image;
  
	m_poImage = NULL;
	m_poTallImage = NULL;
	m_poBackgroundImage = NULL;
  m_poTalk1Image = NULL;
  m_poTalk2Image = NULL;
  m_poTalk3Image = NULL;
}

void
Clock::HandleEvents()
{
	Events& oEvents = g_oApp.GetEvents();
	oEvents.Process();

	if (oEvents.CheckAndResetExit()) {
		m_bLeaving = true;
	} else if (oEvents.CheckAndResetStart()) {
		m_bTurbo = true;
	}
}

void
Clock::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.8f);
	m_poBackgroundImage->RenderFullScreen();
	glPopMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendColor(0.25f, 0.25f, 0.25f, 1.0f);

  float fZ = -0.6f;
  float fTexHeight = m_poImage->GetNormalizedHeight();
  float fTexWidth  = m_poImage->GetNormalizedWidth();

	float fRadius = g_oApp.GetScreenWidth() * 0.25f;
	float fAngle = 0.0f;
	for (int n = 0; n < 12; n++) {
		float fX = g_oApp.GetScreenWidth() / 2;
		float fY = g_oApp.GetScreenHeight() / 2;
		fX += fRadius * cos(fAngle);
		fY += fRadius * sin(fAngle);

		glPushMatrix();
		glTranslatef(fX - m_poImage->GetWidth() / 2.0f, fY - m_poImage->GetWidth() / 2.0f, 0.0f);

		glBindTexture(GL_TEXTURE_2D, m_poImage->GetTextureID());
		glBegin(GL_QUADS);
		glTexCoord2f(     0.0f,       0.0f); glVertex3f(0, 0, fZ);
		glTexCoord2f(fTexWidth,       0.0f); glVertex3f(m_poImage->GetWidth(), 0, fZ);
		glTexCoord2f(fTexWidth, fTexHeight); glVertex3f(m_poImage->GetWidth(), m_poImage->GetHeight(), fZ);
		glTexCoord2f(     0.0f, fTexHeight); glVertex3f(0, m_poImage->GetHeight(), fZ);
		glEnd();
		glPopMatrix();

		fAngle += M_PI / 6;
	}

	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, 0.0f, -0.4f);

  // Two seconds after starting show talk picture 1 for 2 seconds
	if ((int)m_fMinute == 55 && m_fSecond >= 44.0f && m_fSecond < 46.0f)
		m_poTalk1Image->RenderFullScreen();

	// When talk picture 1 is done, show talk picture 2 for 2 seconds
	if ((int)m_fMinute == 55 && m_fSecond >= 46.0f && m_fSecond < 48.0f)
		m_poTalk2Image->RenderFullScreen();

	// When talk picture 2 is done, let the clock go faster
	if ((int)m_fMinute == 55 && m_fSecond > 48.0f)
		m_bTurbo = true;

	// When it's 9:02:10 stop with faster time and show talk picture 3
	if ((int)m_fMinute == 2 && m_fSecond > 10.0f) {
		m_bTurbo = false;
		m_poTalk3Image->RenderFullScreen();
	}
	glPopMatrix();

	// Draw handles	
	glTranslatef(0, 0, -0.2f);
	DrawHandle(m_fHour * 5.0f, 50.0f, fRadius);
	glTranslatef(0, 0, 0.05f);
	DrawHandle(m_fMinute, 30.0f, fRadius * 0.75f);
	glTranslatef(0, 0, 0.05f);

	DrawHandle(m_fSecond, 20.0f, fRadius);

	glDisable(GL_BLEND);
	SDL_GL_SwapBuffers();
}

void
Clock::DrawHandle(float fValue, float fThickness, float fRadius)
{
	fValue = fmod(fValue + 45.0f, 60.0f);
	assert(fValue >= 0.0f && fValue <= 60.0f);
	float fTexHeight = m_poTallImage->GetNormalizedHeight();
	float fTexWidth  = m_poTallImage->GetNormalizedWidth();

	glPushMatrix();

	glTranslatef(
	 g_oApp.GetScreenWidth() / 2.0f,
	 g_oApp.GetScreenHeight() / 2.0f,
	 0.0f
	);
	glRotatef(fValue / (60.0f / 360.0f), 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, m_poTallImage->GetTextureID());

	float H = fThickness;
	float W = fRadius;
	glBegin(GL_QUADS);
	glTexCoord2f(     0.0f,       0.0f); glVertex3f(0,  -H, 0);
	glTexCoord2f(fTexWidth,       0.0f); glVertex3f(W, -H, 0);
	glTexCoord2f(fTexWidth, fTexHeight); glVertex3f(W, H, 0);
	glTexCoord2f(     0.0f, fTexHeight); glVertex3f(0,  H, 0);
	glEnd();

	glPopMatrix();
}

void
Clock::Run()
{
	boost::posix_time::time_duration start_time(boost::posix_time::microsec_clock::local_time().time_of_day());

	// Start at 8:55:42 AM
	start_time -= boost::posix_time::hours(8);
	start_time -= boost::posix_time::minutes(55);
	start_time -= boost::posix_time::seconds(42);

	m_bLeaving = false;
	while(!m_bLeaving) {
		boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration duration( time.time_of_day() );
		duration = duration - start_time;

		if (m_bTurbo)
			start_time -= boost::posix_time::seconds(1);

		m_fSecond = duration.seconds();
		m_fSecond += duration.fractional_seconds() / 1000000.0f;
		m_fMinute = duration.minutes();
		m_fMinute += duration.seconds() / 60.0f;
		m_fHour = duration.hours();
		if (m_fHour >= 12.0f)
			m_fHour -= 12.0f;
		m_fHour += (m_fMinute / 60.0f);

		// Stop at 9:02:12 AM
		if (m_fHour >= 9 && m_fMinute >= 2 && m_fSecond >= 12)
			m_bLeaving = true;

		HandleEvents();
		Render();
		SDL_Delay(1000 / g_oApp.GetDesiredFPS());
	}
}

/* vim:set ts=2 sw=2: */
