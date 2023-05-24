#include "musicplayer.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>
#include <assert.h>
#include "app.h"
#include "events.h"

static float m_fRotate = 25.0f;

MusicPlayer::MusicPlayer()
	: m_pMusic(NULL)
{
}

MusicPlayer::~MusicPlayer()
{
	assert(m_pMusic == NULL);
}

void
MusicPlayer::Init()
{
	std::string sMusicFile(g_oApp.GetDataPath() + "/deuntje.mp3");
	m_pMusic = Mix_LoadMUS(sMusicFile.c_str());
	assert(m_pMusic != NULL);
	Mix_SetPostMix(MusicPlayer::PostMixWrapper, (void*)this);

	int freq, channels;
	Uint16 format;
	if (Mix_QuerySpec(&freq, &format, &channels) == 0)
		assert(0);
	assert(channels == 2);
	assert(format == AUDIO_S16SYS);

	m_fX = 0.0f;
	m_fY = 0.0f;
	m_fZ = -600.0f;
}

void
MusicPlayer::Cleanup()
{
	Mix_SetPostMix(NULL, NULL);
	Mix_FreeMusic(m_pMusic);
	m_pMusic = NULL;
}

void
MusicPlayer::PostMix(Uint8* pStream, int iLength)
{
	int iOriginalLength = iLength;
	iLength /= 4;
	assert(iLength > m_ciNumValues);

	// First of all, shift the current values a bit
	memcpy(&m_fMixerValues[0][0], &m_fMixerValues[1][0], m_ciNumValues * (m_ciNumValues - 1) * sizeof(float));
	
	// Add the new values
	int iValuesPerStep = iLength / m_ciNumValues;
	int16_t* pStreamData = (int16_t*)pStream;
	for (int i = 0; i < m_ciNumValues; i++) {
#if 0
		float v = 0.0f;
		for (int n = 0; n < iValuesPerStep; n++, pStreamData += 2) {
			v += *pStreamData;
		}
		v /= iValuesPerStep; /* average, -32767 ... 32768 */
#else
		float v = *pStreamData;
		pStreamData += iValuesPerStep * 2;
#endif
	//	v = (v + 32767.0f) / (65536.0f / m_ciNumValues); /* 0 .. m_fCubeSize - 1 */
		v /= (65536.0f / m_ciNumValues);
		m_fMixerValues[m_ciNumValues - 1][i] = v;
	}
#if 0
	int iStepSize = iLength / m_ciNumValues;
	for (int i = 0; i < m_ciNumValues; i++) {
		float avg = 0;
		for (int j = 0; j < iStepSize; j++) {
			int16_t val = *((int16_t*)pStream + ((i * (int)iStepSize) + j) * 2);
			avg += val;
		}
		avg /= iStepSize;
		avg += 32767.0f; // drop the sign

		/* Scale */
		avg /= (65536.0f / m_ciNumValues);
		m_fMixerValues[m_ciNumValues - 1][i] = avg;
	}	
#endif
}

void
MusicPlayer::Run()
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

	Mix_PlayMusic(m_pMusic, 1);
	Mix_VolumeMusic(SDL_MIX_MAXVOLUME);
	while(!m_bLeaving) {
		HandleEvents();
		Render();
	//	SDL_Delay(1000 / g_oApp.GetDesiredFPS());
	}
}

void
MusicPlayer::HandleEvents()
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
		m_fRotate += 5.0f;
	} else if (oEvents.CheckAndResetMinus()) {
		m_fRotate -= 5.0f;
	} else if (oEvents.CheckAndResetLeft()) {
		m_fZ -= 5.0f;
	} else if (oEvents.CheckAndResetRight()) {
		m_fZ += 5.0f;
	}
}

void
MusicPlayer::Render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
  glTranslatef(m_fX, m_fY, m_fZ);
	glRotatef(m_fRotate, 1, 0, 0);
	glRotatef(40.0f, 0, 1, 0);

	glColor3f(1.0f, 1.0f, 1.0f);

#define CUBE_SIZE m_ciNumValues
#define CUBE_FACTOR 2

	//glutWireCube(CUBE_SIZE * CUBE_FACTOR);
//	int m = idx + 1;
	int m = 0;
	do {
		for(int n = 0; n < CUBE_SIZE - 1; n++) {
			float v1 = m_fMixerValues[m    ][n    ];
			float v2 = m_fMixerValues[m    ][n + 1];
			float v3 = m_fMixerValues[(m + 1) % CUBE_SIZE][n    ];
			float v4 = m_fMixerValues[(m + 1) % CUBE_SIZE][n + 1];

			//glBegin(GL_QUAD_STRIP);
			glBegin(GL_QUAD_STRIP);

#define V(x,y,z) \
	glColor3f(0.5f + (y) / (0.5f * CUBE_SIZE), 1.0f - ((y) / (0.5f * CUBE_SIZE)), 0); \
	glVertex3f( \
		((x) * CUBE_FACTOR) - (CUBE_FACTOR * CUBE_SIZE / 2.0f),	\
		((y) * CUBE_FACTOR), \
		((z) * CUBE_FACTOR) - (CUBE_FACTOR * CUBE_SIZE / 2.0f) 	\
	)

			V(n,     v1, m);
			V(n + 1, v2, m);
			V(n    , v3, m + 1);
			V(n + 1, v4, m + 1);

			glEnd();
		}
		m = (m + 1) % CUBE_SIZE;
	} while(m != 0);

#if 0
	int m = 0;
	do {
		for (int n = 0; n < m_ciNumValues - 1; n++) {
			float v1 = m_fMixerValues[m][n];
			float v2 = m_fMixerValues[m][n + 1];
			float v3 = m_fMixerValues[(m + 1) % m_ciMixerDepth][n];
			float v4 = m_fMixerValues[(m + 1) % m_ciMixerDepth][n + 1];

#define V(x,y,z) \
  glColor3f(0.5f + (y) / (0.5f * m_ciNumValues), 1.0f - ((y) / (0.5f * m_ciNumValues)), 0); \
  glVertex3f( \
    ((x) * CUBE_FACTOR) - (CUBE_FACTOR * m_ciNumValues / 2.0f), \
    ((y) * CUBE_FACTOR), \
    ((z) * CUBE_FACTOR) - (CUBE_FACTOR * m_ciNumValues / 2.0f)  \
  )

			glBegin(GL_LINES);
      V(n,     v1, m);
      V(n + 1, v2, m);
      V(n    , v3, m + 1);
			V(n + 1, v4, m + 1);
			glEnd();
		}
		m = (m + 1) % m_ciMixerDepth;
	} while (m != 0);
#endif

#if 0
	glTranslatef(-m_ciNumValues, -(m_ciNumValues / 2.0f), 0.0f);
	glBegin(GL_LINES);

#define DRAW(dx, dy) \
	float fXFactor = 5;
	float fYFactor = 10;
	int iStep = 1;
	for (int x = 0; x < m_ciNumValues - iStep; x += iStep) {
		for (int y = 0; y < m_ciNumValues - iStep; y += iStep) {
			// bottom left
			DRAW(0, 0);

			// top left
			DRAW(0, iStep);

			// top right
			DRAW(iStep, iStep);

			// bottom right
			DRAW(iStep, 0);
		}
	}
#endif
	
	glEnd();

	SDL_GL_SwapBuffers();
}

/* vim:set ts=2 sw=2: */
