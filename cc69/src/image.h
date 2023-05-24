#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <string>
#include <boost/thread/mutex.hpp>
#include "texture.h"

//! \brief Contains a single image
class Image
{
public:
	//! \brief Constructs an image object
	Image(const std::string& sPath);

	//! \brief Destroys the image
	~Image();

	//! \brief Loads the image
	bool Load();

	//! \brief Unloads the image
	void Unload();

	//! \brief Is the image loaded?
	bool IsLoaded() const { return m_bLoaded; }

	//! \brief Is the image corrupt?
	bool IsCorrupt() const { return m_bCorrupt; }

	//! \brief Retrieve the texture ID
	GLuint GetTextureID() { return m_oTexture.GetTextureID(); }

	//! \brief Retrieve the normalized height
	float GetNormalizedHeight() const { return m_oTexture.GetNormalizedHeight(); }

	//! \brief Retrieve the normalized width
	float GetNormalizedWidth() const { return m_oTexture.GetNormalizedWidth(); }

	//! \brief Retrieve the image height, in pixels
	unsigned int GetHeight() const { return m_oTexture.GetHeight(); }

	//! \brief Retrieve the image width, in pixels
	unsigned int GetWidth() const { return m_oTexture.GetWidth(); }

	/*! \brief Locks the image object
	 *
	 *  Locking the image ensures no one will attempt to read or unload it;
	 *  this must be called whenever the object is rendered.
	 */
	void Lock() { m_oLock.lock(); }

	//! \brief Unlocks the image object
	void Unlock() { m_oLock.unlock(); }

	/*! \brief Cancels a preloaded image
	 *  \returns true on success
	 *
	 *  This function will fail if the item is already locked.
	 */
	bool CancelPreload();

	//! \brief Retrieve the filename of the image
	const std::string& GetFilename() const {return m_sFilename; }

	//! \brief Renders the image full-screen
	void RenderFullScreen();

	/*! \brief Compares two images by filename
	 *  \params pA First image
	 *  \params pB Second image
	 *  \returns true if pA < pB, false otherwise
	 */
	static bool CompareByFilename(const Image* pA, const Image* pB);

private:
	//! \brief Full path to the image
	std::string m_sFilename;

	//! \brief Is an image loaded or preloaded?
	bool m_bLoaded;

	//! \brief Is the image corrupt?
	bool m_bCorrupt;

	//! \brief Texture object
	Texture m_oTexture;

	//! \brief Lock to prevent multiple accessors
	boost::mutex m_oLock;
};

#endif /*  __IMAGE_H__ */

