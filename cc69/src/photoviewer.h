#ifndef __PHOTOVIEWER_H__
#define __PHOTOVIEWER_H__

#include "runnable.h"

class MessageWindow;

class PhotoViewer : public IRunnable
{
public:
	PhotoViewer();
	~PhotoViewer();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

protected:
	void Render();
	void RenderImage(Image* pImage);
	void HandleEvents();
	void Next(int iDirection, bool bUpdatePrev = true);

	void SlideShow(int iInterval);

private:
	int m_iSlideShowInterval;
	int m_iSlideShowCounter;

	//! \brief Are we leaving yet?
	bool m_bLeaving;

	//! \brief Zoom factor
	float m_fZoom;

	//! \brief Current image being viewed
	int m_iCurrentImage;

	//! \brief Prevous image being viewed
	int m_iPreviousImage;

	//! \brief Current direction
	int m_iDirection;

	//! \brief Animation in use
	int m_iAnimation;

	//! \brief Animation counter
	float m_fAnimationCounter;

	//! \brief Which animation effect to use
	int m_iAnimationEffect;

	//! \brief Message window visible
	bool m_bMessageWindowVisible;

	//! \brief Message window object
	MessageWindow* m_poMessageWindow;
};

#endif /*__PHOTOVIEWER_H__ */
