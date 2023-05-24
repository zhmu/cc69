#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "runnable.h"

class Image;

class Clock : public IRunnable
{
public:
	Clock();
	~Clock();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

protected:
	void HandleEvents();
	void Render();

	void DrawHandle(float fValue, float fThickness, float fRadius);

private:
	bool m_bLeaving;

	//! \brief Images
	Image* m_poBackgroundImage;
	Image* m_poImage;
	Image* m_poTallImage;
	Image* m_poShortImage;
	Image* m_poTalk1Image;
	Image* m_poTalk2Image;
	Image* m_poTalk3Image;

	float m_fHour;
	float m_fMinute;
	float m_fSecond;

	bool m_bTurbo;
};

#endif
