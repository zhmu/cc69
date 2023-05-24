#ifndef __MUSICPLAYER_H__
#define __MUSICPLAYER_H__

#include <SDL/SDL.h> // for Uint8
#include "runnable.h"

typedef struct _Mix_Music Mix_Music;

class MusicPlayer : public IRunnable
{
public:
	MusicPlayer();
	~MusicPlayer();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

protected:
	static void PostMixWrapper(void* pUser, Uint8* pStream, int iLength) {
		((MusicPlayer*)pUser)->PostMix(pStream, iLength);
	}
	void PostMix(Uint8* pStream, int iLength);

	void HandleEvents();
	void Render();

	//! \brief Number of mixer values
	static const int m_ciNumValues = 100;

	//! \brief Size of the cube
	static const float m_fCubeSize = 100.0f;

	//! \brief Mixer values
	float m_fMixerValues[m_ciNumValues][m_ciNumValues];

	//! \brief Are we there yet?
	bool m_bLeaving;

	//! \brief Currently loaded music
	Mix_Music* m_pMusic;

	//! \brief Viewer position
	float m_fX, m_fY, m_fZ;
};

#endif /* __MUSICPLAYER_H__ */
