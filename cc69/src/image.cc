#include "image.h"
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "app.h"
#include "stringlibrary.h"

Image::Image(const std::string& sFilename)
	: m_sFilename(sFilename), m_bLoaded(false), m_bCorrupt(false)
{
}

bool
Image::Load()
{
	assert(!m_bLoaded); // you shan't load twice
	assert(!m_bCorrupt); // don't load what you know will fail

	SDL_Surface* pSurface = IMG_Load(m_sFilename.c_str());
	if (pSurface == NULL) {
		m_bCorrupt = true;
		return false;
	}
	if (m_oTexture.ConvertSurface(pSurface))
		m_bLoaded = true;
	else
		m_bCorrupt = true;

	// Throw away the SDL image; we no longer need it
	SDL_FreeSurface(pSurface);
	return !m_bCorrupt;
}

void
Image::Unload()
{
	// Throw away the texture, if any
	m_oTexture.Unload();

	// Nothing is loaded now, for sure
	m_bLoaded = false;
}

Image::~Image()
{
	// No need to unload the texture; the object does this by itself
}

bool
Image::CancelPreload()
{
	if (!m_oLock.try_lock())
		return false;

	Unload();

	m_oLock.unlock();
	return true;
}

bool
Image::CompareByFilename(const Image* pA, const Image* pB)
{
	return StringLibrary::NaturalCompare(pA->GetFilename(), pB->GetFilename()) < 0;
}

void
Image::RenderFullScreen()
{
	float fTexHeight = GetNormalizedHeight();
	float fTexWidth  = GetNormalizedWidth();
	glBindTexture(GL_TEXTURE_2D, GetTextureID());
	glBegin(GL_QUADS);
	glTexCoord2f(     0.0f,       0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(fTexWidth,       0.0f); glVertex3f(g_oApp.GetScreenWidth(), 0.0f, 0.0f);
	glTexCoord2f(fTexWidth, fTexHeight); glVertex3f(g_oApp.GetScreenWidth(), g_oApp.GetScreenHeight(), 0.0f);
	glTexCoord2f(     0.0f, fTexHeight); glVertex3f(0.0f, g_oApp.GetScreenHeight(), 0.0f);
	glEnd();
}

/* vim:set ts=2 sw=2: */
