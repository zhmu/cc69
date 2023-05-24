#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <assert.h>
#include <err.h>
#include "app.h"
#include "events.h"
#include "game.h"
#include "imagelibrary.h"
#include "image.h"
#include "photoviewer.h"
#include "messagewindow.h"

PhotoViewer::PhotoViewer()
	: m_iCurrentImage(0), m_iPreviousImage(0), m_fZoom(0.0f), m_iDirection(1),
		m_iSlideShowInterval(0), m_iSlideShowCounter(0), m_iAnimation(0),
		m_iAnimationEffect(0), m_poMessageWindow(NULL),
		m_bMessageWindowVisible(false), m_bLeaving(false)
{
}

PhotoViewer::~PhotoViewer()
{
	delete m_poMessageWindow;
}

void
PhotoViewer::Init()
{
	float fMessageWindowWidth = g_oApp.GetScreenWidth() * 0.25f;
	float fMessageWindowHeight = g_oApp.GetScreenHeight() / 10.0f;
	m_poMessageWindow = new MessageWindow(fMessageWindowWidth, fMessageWindowHeight, 10.0f);
	m_poMessageWindow->Create();
	m_poMessageWindow->SetPosition(
		(g_oApp.GetScreenWidth() - fMessageWindowWidth) / 2.0f,
		g_oApp.GetScreenHeight() - fMessageWindowHeight * 1.5f
	);
	g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
}

void
PhotoViewer::Cleanup()
{
}

void
PhotoViewer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	/*
	 * If we are animating, fetch the previous image as well. This is
	 * set by Next() when transitioning, so we are certain that this
	 * image is not corrupt (it has already been shown)
	 */
	Image* pPreviousImage = NULL;
	if (m_iAnimation && m_iPreviousImage >= 0) {
		pPreviousImage = g_oApp.GetImageLibrary().GetLocked(m_iPreviousImage);
		assert(!pPreviousImage->IsCorrupt());
	}

	// Fetch the current image; this needs to skip over any corrupt images
	Image* pImage = NULL;
	int iOriginalImage = m_iCurrentImage;
	while(true) {
		pImage = g_oApp.GetImageLibrary().GetLocked(m_iCurrentImage);
		if (!pImage->IsCorrupt())
			break;

		// Oops, this photo was corrupted!
		pImage->Unlock();
		Next(m_iDirection, false);
		if (iOriginalImage == m_iCurrentImage) {
			// Uh-uh, there are no non-corrupt photo's here.
			return;
		}
	}

	// If we have a previous image, we need to do an animation
	if (pPreviousImage != NULL) {
		glLoadIdentity();
		switch(m_iAnimation) {
			case 1: // horizontal left slide in
				glTranslatef(m_fAnimationCounter, 0.0f, 0.0f);
				RenderImage(pPreviousImage);

				glLoadIdentity();
				glTranslatef(m_fAnimationCounter - g_oApp.GetScreenWidth(), 0.0f, 0.0f);
				m_fAnimationCounter += 25.0f;
				if (m_fAnimationCounter >= g_oApp.GetScreenWidth())
					m_iAnimation = 0;
				break;
			case 2: // horizontal right slide in
				glTranslatef(-m_fAnimationCounter, 0.0f, 0.0f);
				RenderImage(pPreviousImage);

				glLoadIdentity();
				glTranslatef(g_oApp.GetScreenWidth() - m_fAnimationCounter, 0.0f, 0.0f);
				m_fAnimationCounter += 25;
				if (m_fAnimationCounter >= g_oApp.GetScreenWidth())
					m_iAnimation = 0;
				break;
			case 3: { // blend
				/* Fade from here... */
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);

				glColor4f(1.0f, 1.0f, 1.0f, m_fAnimationCounter);
				RenderImage(pPreviousImage);
				/* ... to the new image */
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f - m_fAnimationCounter);

				/* Force the new image on top of the other */
				glTranslatef(0.0f, 0.0f, 0.1f);

				m_fAnimationCounter += 0.08;
				if (m_fAnimationCounter >= 1.0f)
					m_iAnimation = 0;
				break;
			}
		}
	}

	// Update the message window
	std::string sImageFile(pImage->GetFilename());
	size_t sFinalSlash = sImageFile.find_last_of('/');
	if (sFinalSlash != std::string::npos)
		sImageFile = sImageFile.substr(sFinalSlash + 1);
	m_poMessageWindow->Update(sImageFile);

	// Render the new image
	RenderImage(pImage);

	// Done with the images
	pImage->Unlock();
	if (pPreviousImage != NULL)
		pPreviousImage->Unlock();

	glDisable(GL_BLEND);

	// Show the message window, if necessary
	if (m_bMessageWindowVisible)
		m_poMessageWindow->Render();

	// Render
	SDL_GL_SwapBuffers();
}

void
PhotoViewer::RenderImage(Image* pImage)
{
	// Calculate the <left,top> - <right, bottom> points on screen
	float fTop, fLeft, fRight, fBottom, fDepth;
	fTop    = 0;
	fLeft   = 0;
	fRight  = pImage->GetWidth();
	fBottom = pImage->GetHeight();
	fDepth  = 0;

	// If the photo doesn't fit, make it
	if (fRight >= g_oApp.GetScreenWidth() || fBottom >= g_oApp.GetScreenHeight()) {
		/*
		 * We have the following situation:
		 *
		 *                        W          r
		 *                        v         /
		 * +----------------------+.........
		 * |                      |        .
		 * |                      |        .
		 * |  Screen              |        .
		 * |                      |        .
		 * |                      |        .
		 * +----------------------+        .
		 * .                       \       .
		 * .       Photo            H      .
		 * .................................<- b
		 *
		 * And r >= W, b >= H or both; this means we have to scale the photo so
		 * that it will fit nicely. As we scale, we must keep the aspect ratio
		 * of the photo into account because it will look deformed otherwise.
		 */
		if (fRight > fBottom) {
			/*
			 * Because the width is greater than the height, the photo is a landscape
			 * and this means we'll have to scale according to the width.
			 */
			float fRatio = (float)pImage->GetHeight() / (float)pImage->GetWidth();
			fRight = g_oApp.GetScreenWidth();
			fBottom = fRatio * fRight;
		} else {
			/*
			 * The height is greater than the width; this means the photo is in
			 * portrait mode so we'll have to scale according to the height.
			 */
			float fRatio = (float)pImage->GetWidth() / (float)pImage->GetHeight();
			fBottom = g_oApp.GetScreenHeight();
			fRight = fRatio * fBottom;
		}
	}

	// Scaling is done; see if we need to center the photo
	float fWidthLeft = (g_oApp.GetScreenWidth() - (fRight - fLeft)) / 2.0f;
	float fHeightLeft = (g_oApp.GetScreenWidth() - (fBottom - fTop)) / 2.0f;
	fLeft += fWidthLeft; fRight += fWidthLeft;
	fTop += fHeightLeft; fBottom += fHeightLeft;

	// Handle zooming (and clip a bit to prevent things from looking too weird)
	fLeft -= m_fZoom;
	fTop -= m_fZoom;
	fRight += m_fZoom;
	fBottom += m_fZoom;
	if (fLeft > fRight)
		fLeft = fRight;
	if (fBottom < fTop)
		fBottom = fTop;

	// Draw the image
	float fTexHeight = pImage->GetNormalizedHeight();
	float fTexWidth  = pImage->GetNormalizedWidth();
	glBindTexture(GL_TEXTURE_2D, pImage->GetTextureID());
	glBegin(GL_QUADS);
	glTexCoord2f(     0.0f,       0.0f); glVertex3f( fLeft,  fTop,    fDepth);
	glTexCoord2f(fTexWidth,       0.0f); glVertex3f( fRight, fTop,    fDepth);
	glTexCoord2f(fTexWidth, fTexHeight); glVertex3f( fRight, fBottom, fDepth);
	glTexCoord2f(     0.0f, fTexHeight); glVertex3f( fLeft,  fBottom, fDepth);
	glEnd();
}

void
PhotoViewer::HandleEvents()
{
	Events& oEvents = g_oApp.GetEvents();
	oEvents.Process();

	if (oEvents.CheckAndResetExit()) {
		m_bLeaving = true;
	} else if (oEvents.CheckAndResetLeft()) {
		m_iAnimationEffect = 1;
		Next(-1);
		if (m_iSlideShowInterval != 0) {
			m_iSlideShowInterval = 0;
			g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
		}
	} else if (oEvents.CheckAndResetRight()) {
		m_iAnimationEffect = 1;
		Next(1);
		if (m_iSlideShowInterval != 0) {
			m_iSlideShowInterval = 0;
			g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
		}
	} else if (oEvents.CheckAndResetMinus()) {
		m_fZoom -= 5.0f;
	} else if (oEvents.CheckAndResetPlus()) {
		m_fZoom += 5.0f;
	} else if (oEvents.CheckAndResetStop()) {
		if (m_iSlideShowInterval == 0)
			m_bLeaving = true;
		m_iSlideShowInterval = 0;
		g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
	} else if (oEvents.CheckAndResetStart()) {
		m_iSlideShowInterval = g_oApp.GetDesiredFPS(); /* 1 sec per image */
		m_iSlideShowCounter = m_iSlideShowInterval;
		m_iAnimationEffect = 2;
		g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
	}
} 

void
PhotoViewer::Next(int iDirection, bool bUpdatePrev /* = true */)
{
	if (bUpdatePrev) {
		m_iPreviousImage = m_iCurrentImage;
		switch(m_iAnimationEffect) {
			default: m_iAnimation = 0; break;
			case 1: m_iAnimation = (iDirection < 0) ? 1 : 2; break;
			case 2: m_iAnimation = 3; break;
		}
		m_fAnimationCounter = 0;
	}
	switch(iDirection) {
		case -1:
			if (m_iCurrentImage > 0)
				m_iCurrentImage--;
			else
				m_iCurrentImage = g_oApp.GetImageLibrary().GetSize() - 1;
			break;
		case 1:
			m_iCurrentImage = (m_iCurrentImage + 1) % g_oApp.GetImageLibrary().GetSize();
			break;
		default:
			assert(0);
	}
	m_iDirection = iDirection;
}

void
PhotoViewer::Run()
{
	m_bLeaving = false;
	while(!m_bLeaving) {
		HandleEvents();
		Render();
		SDL_Delay(1000 / g_oApp.GetDesiredFPS());

		// Handle the slideshow, but only if there is no animation
		if (m_iSlideShowInterval > 0 && m_iAnimation == 0) {
			// Time to show the next image?
			if (m_iSlideShowCounter-- <= 0) {
				// Yes; do that and re-arm
				Next(1);
				m_iSlideShowCounter = m_iSlideShowInterval;
			}
		}
	}
}

void
PhotoViewer::SlideShow(int iInterval)
{
	m_iSlideShowInterval = iInterval;
	m_iSlideShowCounter = iInterval;
}

/* vim:set ts=2 sw=2: */
