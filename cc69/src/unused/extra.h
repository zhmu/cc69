#ifndef __EXTRA_H__
#define __EXTRA_H__

#include "runnable.h"

class Extra : public IRunnable
{
public:
	Extra();
	~Extra();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

protected:
	void HandleEvents();
	void Render();

	//! \brief Are we there yet?
	bool m_bLeaving;

	//! \brief Rotation angles
	float m_fRx, m_fRy, m_fRz;

	//! \brief Viewer position
	float m_fX, m_fY, m_fZ;
};

#endif /* __EXTRA_H__ */
