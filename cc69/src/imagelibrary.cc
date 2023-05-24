#include "imagelibrary.h"
#include "image.h"
#include <boost/filesystem.hpp>
#include <boost/static_assert.hpp>

#include <stdio.h>

ImageLibrary::ImageLibrary()
	: m_oPreloadThread(PreloaderThreadWrapper, this),
		m_iCurrentImage(-1)
{
}

ImageLibrary::~ImageLibrary()
{
	// Ask the thread to terminate and wait until it is gone
	m_bTerminating = true;
	m_oCV.notify_one();
	m_oPreloadThread.join();

	// Throw away all images we have
	for (TImagePtrVector::iterator it = m_oImages.begin(); it != m_oImages.end(); it++)
		delete *it;
}

int
ImageLibrary::GetSize() const
{
	return m_oImages.size();
}

bool
ImageLibrary::AddPath(const std::string& sPath)
{
	try {
		boost::filesystem::path p(sPath);

		if (!is_directory(p))
			return false;

		// Obtain a list of all items
		typedef std::vector<boost::filesystem::path> TPathVector;
		TPathVector oPaths;
		copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(oPaths));

		// Add everything we see; it's up to the image to figure out whether it's sane input
		for (TPathVector::const_iterator it = oPaths.begin(); it != oPaths.end(); it++) {
			if (is_directory(*it)) {
				// It's a directory -> recurse
				AddPath(std::string(it->string()));
			} else {
				// Add the image
				m_oImages.push_back(new Image(it->string()));
			}
		}
	} catch(boost::filesystem::filesystem_error& oError) {
		/* XXX I wonder when this happens? */
		std::cout << "ImageLibrary::AddPath() failed: " << oError.what() << "\n";
	}
}

Image*
ImageLibrary::GetLocked(int n)
{
	assert(n >= 0 && n < m_oImages.size());

	// Obtain the image lock; this prevents the preloader from messing with it
	Image* pImage = m_oImages[n];
	pImage->Lock();

	/*
	 * Update our previous and current image values, but only on a change.
	 */
	{
		boost::unique_lock<boost::mutex> oLock(m_oLock);
		if (m_iCurrentImage != n) {
			// Update our image and awake our preloader thread
			m_iCurrentImage = n;
			m_oCV.notify_one();
		}

		/*
		 * If the preloader list is getting very large, throw away some images that
		 * are no longer necessary.
		 */
		int iPreloadOverflow = m_oPreloadedImages.size() - 2 * (m_ciNumPreloadsPerDirection + 1);
		TImagePtrList::iterator it(m_oPreloadedImages.begin());
		while (iPreloadOverflow > 0 && it != m_oPreloadedImages.end()) {
			if ((*it)->CancelPreload()) {
				it = m_oPreloadedImages.erase(it);
				iPreloadOverflow--;
			} else {
				it++;
			}
		}
		assert(iPreloadOverflow <= 0);
	}

	// Load the image, if necessary
	if (!pImage->IsLoaded() && !pImage->IsCorrupt() && pImage->Load()) {
		// Add the item to the cache; this ensures it will be cleaned up as necessary
		boost::unique_lock<boost::mutex> oLock(m_oLock);
		m_oPreloadedImages.push_back(pImage);
	}
	return pImage;
}

void
ImageLibrary::Preload(int iCurrent, int iDirection)
{
	assert(iDirection == -1 || iDirection == 1);
	assert(iCurrent >= 0 && iCurrent < m_oImages.size());

	int iLoops = 0;
	int iLeft = m_ciNumPreloadsPerDirection;
	while (iLeft > 0 && iLoops < 2) {
		// Figure out which index to preload
		iCurrent += iDirection;
		if (iCurrent < 0) {
			iCurrent = m_oImages.size() - 1; iLoops++;
		} else if (iCurrent >= m_oImages.size()) {
			iCurrent = 0; iLoops++;
		}

		// Obtain the image context and see what we can do
		Image* pImage = m_oImages[iCurrent];
		pImage->Lock();
		if (!pImage->IsCorrupt()) {
			if (!pImage->IsLoaded()) {
				if (pImage->Load()) {
					/*
					 * Loading worked; add the image to the preloaded images list. This
					 * list is used to get rid of old stuff when we are exceeding our
					 * preload list.
					 */
					boost::unique_lock<boost::mutex> oLock(m_oLock);
					m_oPreloadedImages.push_back(pImage);
					iLeft--;
				} /* else image was corrupt; do not update count */
			} else {
				/*
				 * Image was already preloaded - we must move it to the end of the
				 * preloaded images list to prevent it from being cleaned up.
				 */
				boost::unique_lock<boost::mutex> oLock(m_oLock);
				TImagePtrList::iterator it = std::find(m_oPreloadedImages.begin(), m_oPreloadedImages.end(), pImage);
				assert(it != m_oPreloadedImages.end());
				m_oPreloadedImages.erase(it);
				m_oPreloadedImages.push_back(pImage);
				iLeft--;
			}
		} /* else corrupt image; these do not count */
		pImage->Unlock();
	}
}

void
ImageLibrary::PreloaderThread()
{
	m_bTerminating = false;

	for (;;) {
		int iCurrentImage;

		{
			// Wait until we are woken
			boost::unique_lock<boost::mutex> oLock(m_oLock);
			m_oCV.wait(oLock);
			if (m_bTerminating)
				break;

			// See what we have to handle
			iCurrentImage = m_iCurrentImage;
		}

		/*
		 * The idea of the preloader is the following:
		 *
		 *        current image
		 *             v
		 * ...+-----+-----+-----+....
		 *    |     |     |     |
		 *    | n-1 |  n  | n+2 |
		 *    |     |     |     |
		 * ...+-----+-----+-----+....
		 *
		 * Because we don't know which direction the user will go to, we need to
		 * preload several images around the current one. For now, we will assume
		 * 'n' has been loaded already and we need to fill in the blanks.
		 */
		Preload(iCurrentImage,  1);
		Preload(iCurrentImage, -1);
	}
}

void
ImageLibrary::Sort()
{
	std::sort(m_oImages.begin(), m_oImages.end(), Image::CompareByFilename);
}

/* vim:set ts=2 sw=2: */
