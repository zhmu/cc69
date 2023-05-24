#ifndef __APP_H__
#define __APP_H__

#include <string>

class PhotoViewer;
class ImageLibrary;
class Game;
class Menu;
class Clock;
class Events;
class MusicPlayer;
class Extra;
class IRunnable;

typedef struct _TTF_Font TTF_Font;

class App
{
public:
	App();
	~App();

	void Init(int argc, char* argv[]);
	void Cleanup();
	void Run();

	//! \brief Should we terminate the current application?
	bool IsExiting();

	//! \brief Retrieve the screen height
	int GetScreenHeight() const { return m_iHeight; }

	//! \brief Retrieve the screen width
	int GetScreenWidth() const { return m_iWidth; }

	//! \brief Retrieve the intended FPS count
	int GetDesiredFPS() const { return m_iDesiredFPS; }

	//! \brief Retrieve the loaded font
	TTF_Font* GetFont() { return m_pFont; }

	//! \brief Retrieve the image library
	ImageLibrary& GetImageLibrary();

	//! \brief Retrieve the events manager
	Events& GetEvents();

	//! \brief Retrieve the root path to the application data
	const std::string& GetDataPath() const { return m_sDataPath; }

protected:
	//! \brief Print some boring instructions
	void Usage();

	//! \brief Launches the currently running application
	void Launch();

private:
	//! \brief Image library
	ImageLibrary* m_poImageLibrary;

	//! \brief Data path
	std::string m_sDataPath;

	//! \brief Font in use
	TTF_Font* m_pFont;

	//! \brief Events subsystem
	Events* m_poEvents;

	//! \brief Our embedded photo viewer
	PhotoViewer* m_poPhotoViewer;

	//! \brief Our embedded game
	Game* m_poGame;

	//! \brief Our embedded menu
	Menu* m_poMenu;

	//! \brief Our embedded clock
	Clock* m_poClock;

	//! \brief Our embedded music player
	MusicPlayer* m_poMusicPlayer;

	//! \brief Our embedded little extra bonus material
	Extra* m_poExtra;

	//! \brief Currently running application
	IRunnable* m_poCurrentApp;

	//! \brief Screen height, in pixel-like units
	int m_iHeight;

	//! \brief Screen width, in pixel-like units
	int m_iWidth;

	//! \brief Desired FPS count
	int m_iDesiredFPS;

};

extern App g_oApp;

#endif /* __APP_H__ */
