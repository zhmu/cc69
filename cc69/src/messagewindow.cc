#include "messagewindow.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include "app.h"

MessageWindow::MessageWindow(int iWidth, int iHeight, int iCornerRadius)
	: m_pSurface(NULL), m_iWidth(iWidth), m_iHeight(iHeight), m_iRadius(iCornerRadius),
	  m_iTextX(-1), m_iTextY(-1)
{
}

MessageWindow::~MessageWindow()
{
	if (m_pSurface != NULL)
		SDL_FreeSurface(m_pSurface);
}

void
MessageWindow::Create()
{
	assert(m_pSurface == NULL);

	m_pSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, m_iWidth, m_iHeight, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
	assert(m_pSurface != NULL);

	// Create the background
	SDL_FillRect(m_pSurface, NULL, m_iBackgroundColor);

#define CLEAR_PIXEL(x, y) \
	{ uint32_t* p = (uint32_t*)((uint8_t*)m_pSurface->pixels + (y) * m_pSurface->pitch + (x) * sizeof(uint32_t)); *p = 0; }

	/*
	 * Round the edges of the box we just created; 
	 */
	int num = 20;
	float angle = 0.0f;
	SDL_LockSurface(m_pSurface);
	for (int i = 0; i < num; i++) {
		float angle_rad = angle * (M_PI / 180.0f);
		angle += (90.0f / (float)num); 

		// Calculate the point in the circle
		float fX = (float)m_iRadius * cos(angle_rad);
		float fY = (float)m_iRadius * sin(angle_rad);

		// Top left
		{
			int x = m_iRadius - fX;
			int y = m_iRadius - fY;
			for (/* nothing */; x >= 0; x--)
				CLEAR_PIXEL(x,y);
		}

		// Top right
		{
			int x = m_iWidth - m_iRadius + fX;
			int y = m_iRadius - fY;
			for (/* nothing */; x < m_iWidth; x++)
				CLEAR_PIXEL(x,y);
		}

		// Bottom left 
		{
			int x = m_iRadius - fX;
			int y = m_iHeight - m_iRadius + fY;
			for (/* nothing */; x >= 0; x--)
				CLEAR_PIXEL(x,y);
		}

		// Bottom right
		{
			int x = m_iWidth - m_iRadius + fX;
			int y = m_iHeight - m_iRadius + fY;
			for (/* nothing */; x < m_iWidth; x++)
				CLEAR_PIXEL(x,y);
		}
	}
	SDL_UnlockSurface(m_pSurface);
#undef CLEAR_PIXEL

	if (!m_oTexture.ConvertSurface(m_pSurface))
		assert(0);
}

void
MessageWindow::SetPosition(float fX, float fY)
{
	m_fX = fX; m_fY = fY;
}

void
MessageWindow::Update(const std::string& sText)
{
	// If our message is identical, don't bother doing anything
	if (m_sMessage == sText)
		return;
	m_sMessage = sText;

	// Clear our surface's contents
	SDL_Rect oSurfaceRect;
	oSurfaceRect.x = m_iRadius;
	oSurfaceRect.y = m_iRadius;
	oSurfaceRect.w = m_iWidth - 2 * m_iRadius;
	oSurfaceRect.h = m_iHeight - 2 * m_iRadius;
	SDL_FillRect(m_pSurface, &oSurfaceRect, m_iBackgroundColor);

	/*
	 * Render the text to a temporary surface and copy that to
	 * our surface - this ensures that the pixel formats match
	 */
	SDL_Color oColor;
	oColor.r = (m_iTextColor >> 16) & 0xff;
	oColor.g = (m_iTextColor >>  8) & 0xff;
	oColor.b = (m_iTextColor      ) & 0xff;
	SDL_Surface* pFontSurface = TTF_RenderText_Blended(g_oApp.GetFont(), sText.c_str(), oColor);
	SDL_Rect oRect;
	if (m_iTextX < 0)
		oRect.x = (m_iWidth - pFontSurface->w) / 2;
	else
		oRect.x = m_iTextX;
	if (m_iTextY < 0)
		oRect.y = (m_iHeight - pFontSurface->h) / 2;
	else
		oRect.y = m_iTextY;
	assert(pFontSurface != NULL);
	SDL_BlitSurface(pFontSurface, NULL, m_pSurface, &oRect);
	SDL_FreeSurface(pFontSurface);

	// Update our texture
	m_oTexture.Update(m_pSurface, oSurfaceRect.x, oSurfaceRect.y, oSurfaceRect.x, oSurfaceRect.y, oSurfaceRect.w, oSurfaceRect.h);
}

void
MessageWindow::Render()
{
	float fLeft, fTop, fRight, fBottom, fDepth;
	fLeft = m_fX;
	fTop = m_fY;
	fRight = fLeft + m_iWidth;
	fBottom = fTop + m_iHeight;
	fDepth = 0.2f;

	// Enable alphablending to make the message window transparant
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);

	float fTexHeight = m_oTexture.GetNormalizedHeight();
	float fTexWidth  = m_oTexture.GetNormalizedWidth();
	glBindTexture(GL_TEXTURE_2D, m_oTexture.GetTextureID());
	glBegin(GL_QUADS);
	glTexCoord2f(     0.0f,       0.0f); glVertex3f( fLeft,  fTop,    fDepth);
	glTexCoord2f(fTexWidth,       0.0f); glVertex3f( fRight, fTop,    fDepth);
	glTexCoord2f(fTexWidth, fTexHeight); glVertex3f( fRight, fBottom, fDepth);
	glTexCoord2f(     0.0f, fTexHeight); glVertex3f( fLeft,  fBottom, fDepth);
	glEnd();

	glDisable(GL_BLEND);
}

/* vim:set ts=2 sw=2: */
