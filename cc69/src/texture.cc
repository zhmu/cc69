#include "texture.h"
#include <assert.h>
#include <SDL/SDL.h>

Texture::Texture()
	: m_iTexture(m_ciUndefinedTexture),
	  m_fNormalizedHeight(1.0f), m_fNormalizedWidth(1.0f),
		m_pTextureData(NULL)
{
}

Texture::~Texture()
{
	Unload();
}

uint32_t
Texture::RoundUp2(uint32_t n)
{
	/*
	 * This function is inspired by the Wikipedia article on 'Powers of two', which
	 * is in turn inspired by "Hacker's Delight" by Henry S. Warren Jr.
	 */

	// If the value is already a power of two, we're done
	if ((n & (n - 1)) == 0)
		return n;

	// Round up to the next power of two
	n--;
	n |= (n >> 1);
	n |= (n >> 2);
	n |= (n >> 4);
	n |= (n >> 8);
	n |= (n >> 16);
	n++;
	return n;
}

bool
Texture::ConvertSurface(SDL_Surface* pSurface)
{
	// Ensure nothing has yet been loaded
	assert(m_iTexture == m_ciUndefinedTexture);
	assert(m_pTextureData == NULL);

	// We'll only handle RGB and RGBA images; anything else will likely break
	if (pSurface->format->BytesPerPixel != 3 && pSurface->format->BytesPerPixel != 4)
		return false;

	// Ensure have sane input
	if (pSurface->h <= 0 || pSurface->w <= 0)
		return false;
	GLint iColours = pSurface->format->BytesPerPixel;

	// Convert the image
	uint32_t iWidth = RoundUp2(pSurface->w);
	uint32_t iHeight = RoundUp2(pSurface->h);

	/*
	 * If image sizes aren't a power of two, we'll have to expand it. This is
	 * tricky because we should make everything transparant that isn't in the
	 * original image, which we can only do if there is an alpha channel.
	 *
	 * To prevent this, we'll just add an alpha channel regardless. Since we want
	 * to throw the SDL surface away, we'll just always re-create the image so
	 * that it can be fed to glTexImage2D() without any problems.
	 */
	m_pTextureData = new uint32_t[iHeight * iWidth];
	unsigned int y = 0;
	uint32_t* pImagePtr = m_pTextureData;
	uint8_t* pixels = (uint8_t*)pSurface->pixels;
	for (/* nothing */; y < pSurface->h; y++) {
		// Copy the current row; RGBA it as needed
		if (iColours == 4) {
			memcpy(pImagePtr, pixels, pSurface->w * sizeof(uint32_t));
			pixels += pSurface->w * sizeof(uint32_t);
			pImagePtr += pSurface->w;
		} else /* iColours == 3 */ {
			for (unsigned int x = 0; x < pSurface->w; x++) {
				uint32_t value = *pixels++;
				value |= *pixels++ << 8;
				value |= *pixels++ << 16;
				*pImagePtr++ = value | 0xff000000 /* alpha */;
			}
		}

		// Skip anything extra on the scanline
		pixels += (pSurface->pitch - (pSurface->w * iColours));

		// Pad the image with fully transparant pixels
		memset(pImagePtr, 0, (iWidth - pSurface->w) * sizeof(uint32_t));
		pImagePtr += (iWidth - pSurface->w);
	}

	// Pad the image with fully transparant pixels
	memset(pImagePtr, 0, (iHeight - pSurface->h) * iWidth * sizeof(uint32_t));

	/*
	 * Image all set - however, we cannot create it, since it's illegal to create
	 * a texture from one thread and use it in another. This is solved by storing
	 * the necessary information here and finally creating the image in the
	 * GetTextureID() function, where it's guaranteed to be safe.
	 */
	m_iTextureHeight = iHeight;
	m_iTextureWidth = iWidth;
	m_eTextureFormat = (pSurface->format->Rmask == 0xff) ? GL_RGBA : GL_BGRA;

	/*
	 * Calculate the normalized height and width; this is the value we need to use when
	 * displaying textures, it cuts off the padding we added.
	 */
	m_fNormalizedHeight = (float)pSurface->h / (float)iHeight;
	m_fNormalizedWidth  = (float)pSurface->w / (float)iWidth;

	// Image is loaded (but the texture is not yet created)
	m_iHeight = pSurface->h;
	m_iWidth = pSurface->w;
	return true;
}

GLuint
Texture::GetTextureID()
{
	// If the texture isn't created, we must do so here
	if (m_iTexture == m_ciUndefinedTexture) {
		assert(m_pTextureData != NULL);

		// Construct the texture
		assert(m_iTexture == m_ciUndefinedTexture);
		glGenTextures(1, &m_iTexture);
		assert(m_iTexture != m_ciUndefinedTexture);
		glBindTexture(GL_TEXTURE_2D, m_iTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, m_eTextureFormat, m_iTextureWidth, m_iTextureHeight, 0, m_eTextureFormat, GL_UNSIGNED_BYTE, (void*)m_pTextureData);

		/*
		 * XXX We use the fact that it is possible to free memory allocated by
		 * other threads; I'm not sure whether this is required behaviour.
		 */
		delete[] m_pTextureData;
		m_pTextureData = NULL;
	}
	return m_iTexture;
}

void
Texture::Unload()
{
	// Throw away the texture, if any
	if (m_iTexture != m_ciUndefinedTexture) {
		glDeleteTextures(1, &m_iTexture);
		m_iTexture = m_ciUndefinedTexture;
	}

	// Throw away the texture data, if any (see XXX in GetTextureID)
	delete[] m_pTextureData;
	m_pTextureData = NULL;

}

void
Texture::Update(SDL_Surface* pSurface, int dstX, int dstY, int srcX, int srcY, int w, int h)
{
	assert(pSurface->format->BytesPerPixel == 4);
	assert((pSurface->format->Rmask == 0xff) ? GL_RGBA : GL_BGRA == m_eTextureFormat);

	// Cut the section from the texture
	uint8_t* pImageData = new uint8_t[h * w * sizeof(uint32_t)];
	uint8_t* pDest = pImageData;
	uint8_t* pSource = (uint8_t*)pSurface->pixels + srcY * pSurface->pitch + srcX * sizeof(uint32_t);
	for (int ypos = 0; ypos < h; ypos++) {
		memcpy(pDest, pSource, w * sizeof(uint32_t)); 
		pDest += w * sizeof(uint32_t);
		pSource += pSurface->pitch;
	}

	glBindTexture(GL_TEXTURE_2D, GetTextureID());
	glTexSubImage2D(GL_TEXTURE_2D, 0, dstX, dstY, w, h, m_eTextureFormat, GL_UNSIGNED_BYTE, (void*)pImageData);
	delete[] pImageData;
}

/* vim:set ts=2 sw=2: */
