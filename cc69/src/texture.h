#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/gl.h>

typedef struct SDL_Surface SDL_Surface;

class Texture
{
public:
	//! \brief Create an empty texture object
	Texture();

	//! \brief Destroys the texture object
	~Texture();

	/*! \brief Converts a SDL surface
	 *  \param pSurface Surface to convert
	 *  \returns true on success
	 *
	 *  This function will _not_ create the actual texture itself - this
	 *  will be done on the first GetTextureID() call in the texture
	 *  object.
	 */
	bool ConvertSurface(SDL_Surface* pSurface);

	/*! \brief Updates part of a texture
	 *  \param pSurface Source surface to use
	 *  \param dstX Destination X coordinate in texture
	 *  \param dstY Destination Y coordinate in texture
	 *  \param srcX Source X coordinate in texture
	 *  \param srcY Source Y coordinate in texture
	 *  \param w Width to update, in pixels
	 *  \param h Height to update, in pixels
	 *
	 *  This function will not do any conversions; the surface must be
	 *  32-bit.
	 */
	void Update(SDL_Surface* pSurface, int dstX, int dstY, int srcX, int srcY, int w, int h);

	/*! \brief Retrieve the OpenGL texture ID
	 *  \returns OpenGL texture ID
	 *
	 *  This function will create the texture as necessary; it should
	 *  only be called from the main thread that does all OpenGL
	 *  interactions.
	 */
	GLuint GetTextureID();

	/*! \brief Unloads the texture
	 *
	 *  This will destroy the pending texture or the OpenGL texture, depending
	 *  on what is loaded.
	 */
	void Unload();

	//! \brief Retrieve the image height, in pixels
	unsigned int GetHeight() const { return m_iHeight; }

	//! \brief Retrieve the image width, in pixels
	unsigned int GetWidth() const { return m_iWidth; }

	//! \brief Retrieve the texture height, in pixels
	unsigned int GetTextureHeight() const { return m_iTextureHeight; }

	//! \brief Retrieve the image width, in pixels
	unsigned int GetTextureWidth() const { return m_iTextureWidth; }

	/*! \brief Normalized texture height
	 *
	 *  The normalized height is 1.0f; but due to constraints a texture must be a
	 *  power of two - this means extra pixels may be added and thus we may
	 *  need to return less than 1.0f.
	 */
	float GetNormalizedHeight() const { return m_fNormalizedHeight; }

	/*! \brief Normalized texture width
	 *
	 *  The normalized width is 1.0f; but due to constraints a texture must be a
	 *  power of two - this means extra pixels may be added and thus we may
	 *  need to return less than 1.0f.
	 */
	float GetNormalizedWidth() const { return m_fNormalizedWidth; }


protected:
	/*! \brief Rounds a number up to the next power of two
	 *
	 *  This function will not process numbers that already are a power of
	 *  two.
	 */
	static uint32_t RoundUp2(uint32_t n);

private:
	//! \brief Normalized width is the scaled width for OpenGL
	float m_fNormalizedWidth;

	//! \brief Normalized height is the scaled height for OpenGL
	float m_fNormalizedHeight;

	//! \brief Pointer to texture data, if necessary
	uint32_t* m_pTextureData;

	//! \brief Original image height, in pixels
	int m_iHeight;

	//! \brief Original image width, in pixels
	int m_iWidth;

	//! \brief Texture height, in pixels
	int m_iTextureHeight;

	//! \brief Texture width, in pixels
	int m_iTextureWidth;

	//! \brief Texture format
	GLenum m_eTextureFormat;

	//! \brief OpenGL texture, if any
	GLuint m_iTexture;

	/*! \brief OpenGL undefined texture ID
	 *
	 *  This value is used for the 'default texture' and will never be returned by
	 *  glGenTextures() - this means it is adequate to use as a 'invalid id' value.
	 */
	static const GLuint m_ciUndefinedTexture = 0;
};

#endif /* __TEXTURE_H__ */
