#ifndef __IMAGELIST_H__
#define __IMAGELIST_H__

#include <vector>
#include <string>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class Image;

class ImageLibrary
{
public:
	//! \brief Initializes a new image library
	ImageLibrary();

	//! \brief Destroys the image library and everything inside it
	~ImageLibrary();

	//! \brief Adds images from a given path to the list
	bool AddPath(const std::string& sPath);

	/*! \brief Sorts all files in the library
	 *
	 *  This works by applying a natural sort on the filename.
	 */
	void Sort();
	
	//! \brief Retrieve the number of images
	int GetSize() const;

	/*! \brief Load a given image number (if necessary) and return it
	 *  \returns Image on success or NULL on failure
	 *
	 *  The resulting image is locked and must be unlocked after use.
	 */
	Image* GetLocked(int n);

protected:
	//! \brief Thread taking care of the preloader actions
	void PreloaderThread();

	/*! \brief Preloads images
	 *  \param iStart First image index to preload
	 *  \param iDirection Direction to go
	 *
	 *  iDirection must be -1 or 1 and indicates whether iStart will be
	 *  incremented or decremented;
	 */
	void Preload(int iStart, int iDirection);

private:
	typedef std::vector<Image*> TImagePtrVector;
	typedef std::list<Image*> TImagePtrList;

	//! \brief Vector containing all available images
	TImagePtrVector m_oImages;

	//! \brief List containing preloaded images
	TImagePtrList m_oPreloadedImages;

	//! \brief Lock protecting the image library
	boost::mutex m_oLock;

	//! \brief Condition variable used to wake the preloader thread
	boost::condition_variable m_oCV;

	//! \brief Preloader thread
	boost::thread m_oPreloadThread;

	//! \brief Should our thread be exiting?
	bool m_bTerminating;

	//! \brief Wrapper for the preloader thread
	static void PreloaderThreadWrapper(void* pMe) {
		((ImageLibrary*)pMe)->PreloaderThread();
	}

	//! \brief Current image being requested
	int m_iCurrentImage;

	/*! \brief Number of images being preloaded per direction
	 *
	 *  For example, using 3 will result in 7 images in memory: the 3
	 *  before the current image, the 3 after the current image, and of
	 *  course the current image itself.
	 */
	static const int m_ciNumPreloadsPerDirection = 3;
};

#endif /* __IMAGELIST_H__ */
