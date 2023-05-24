#include "app.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <err.h>
#include <getopt.h>
#include <iostream>
#include "clock.h"
#include "events.h"
//#include "extra.h"
#include "game.h"
#include "menu.h"
#include "imagelibrary.h"
#include "photoviewer.h"
//#include "musicplayer.h"

App g_oApp;

App::App()
	: m_iDesiredFPS(60), m_poEvents(NULL), m_poPhotoViewer(NULL), m_poGame(NULL),
	  m_poMenu(NULL), m_poClock(NULL), m_poMusicPlayer(NULL), m_poExtra(NULL),
		m_poCurrentApp(NULL)
{
}

App::~App()
{
	assert(m_poPhotoViewer == NULL);
	assert(m_poGame == NULL);
	assert(m_poMenu == NULL);
	assert(m_poClock == NULL);
	assert(m_poImageLibrary == NULL);
	assert(m_poMusicPlayer == NULL);
	assert(m_poEvents == NULL);
}

void
App::Usage()
{
	std::cerr << "usage: cc69 [-h?] [-s h,w] [-g device] -d datapath path ...\n";
	std::cerr << " -h, -?          this help\n";
	std::cerr << " -s h,w          override window size to h x w (defaults to full screen)\n";
	std::cerr << " -g device       gateware device to use\n";
	std::cerr << " -d datapath     directory containing application data\n";
	std::cerr << "\n";
	std::cerr << "path ... is the list of directories containing photo's\n";
	exit(EXIT_SUCCESS);
}

void
App::Init(int argc, char** argv)
{
	// Kickstart SDL and the image library
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_VIDEO) < 0)
		errx(1, "cannot initialize video: %s", SDL_GetError());
#if SDL_IMAGE_PATCHLEVEL > 6
	if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0)
		errx(1, "cannot initialize image support: %s", IMG_GetError());
#endif
	if (Mix_Init(MIX_INIT_MP3) < 0 /* XXX */)
		errx(1, "cannot initialize music support: %s", Mix_GetError());
	if (TTF_Init() < 0)
		errx(1, "cannot initialize font support: %s", TTF_GetError());
	
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	assert(info != NULL);
	m_iWidth = info->current_w;
	m_iHeight = info->current_h;

	// Parse the commandline options
	std::string sGatewareDevice;
	int c;
	while ((c = getopt(argc, argv, "?hg:s:d:")) != -1) {
		switch(c) {
			case 'h':
			case '?':
				Usage();
				/* NOTREACHED */
			case 's': {
				char* ptr;
				int h = strtol(optarg, &ptr, 10);
				int w = *ptr == ',' ? strtol(ptr + 1, &ptr, 10) : 0;
				if (h <= 0 || w <= 0 || *ptr != '\0') {
					std::cerr << "error: cannot parse size\n";
					Usage();
					/* NOTREACHED */
				}
				m_iHeight = h; m_iWidth = w;
				break;
			}
			case 'd':
				m_sDataPath = optarg;
				break;
			case 'g':
				sGatewareDevice = optarg;
				break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0) {
		std::cerr << "error: no paths to view\n";
		Usage();
		/* NOTREACHED */
	}
	if (m_sDataPath.empty()) {
		std::cerr << "error: -d is mandatory\n";
		exit(EXIT_FAILURE);
	}
	{
		std::string sFont(m_sDataPath + "/font.ttf");
		m_pFont = TTF_OpenFont(sFont.c_str(), 12);
	}
	if (m_pFont == NULL) {
		std::cerr << "error: cannot load font\n";
		exit(EXIT_FAILURE);
	}

	// Initialize the events subsystem
	m_poEvents = new Events();
	m_poEvents->Init(sGatewareDevice);

	// Create a new image library
	m_poImageLibrary = new ImageLibrary();

	// Add the paths one by one
	for (int i = 0; i < argc; i++) {
		m_poImageLibrary->AddPath(argv[i]);
	}
	m_poImageLibrary->Sort();

	// If the library is empty, error out
	if (m_poImageLibrary->GetSize() == 0) {
		std::cerr << "error: no images found\n";
		exit(EXIT_FAILURE);
	}

	// Initialize our audio
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		errx(1, "cannot set up audio: %s", Mix_GetError());

	// Initialize our video mode
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	if (SDL_SetVideoMode(m_iWidth, m_iHeight, info->vfmt->BitsPerPixel, SDL_OPENGL) < 0)
		errx(1, "cannot set video mode: %s", SDL_GetError());

	// Initialize the viewport and use a generic [0,0] .. [w,h] coordinate system
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, m_iWidth, m_iHeight, 0.0f, -1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Reset the model view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Textures, flat shading and Z-buffering
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	// Create our embedded parts; we'll only initialize them upoon running to
	// avoid resource use
	m_poPhotoViewer = new PhotoViewer();
	m_poGame = new Game();
	m_poMenu = new Menu();
	m_poClock = new Clock();
	//m_poMusicPlayer = new MusicPlayer();
	//m_poExtra = new Extra();

	// Ignore SDL mouse events by default; they are re-enabled when the
	// mouse button is down
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

	// Disable the mouse cursor
	SDL_ShowCursor(SDL_DISABLE);
}

void
App::Cleanup()
{
	delete m_poImageLibrary;
	delete m_poPhotoViewer;
	delete m_poGame;
	delete m_poClock;
	delete m_poMenu;
	//delete m_poMusicPlayer;
	//delete m_poExtra;
	m_poImageLibrary = NULL;
	m_poGame = NULL;
	m_poPhotoViewer = NULL;
	m_poMenu = NULL;
	m_poClock = NULL;
	m_poMusicPlayer = NULL;
	m_poExtra = NULL;

	m_poEvents->Cleanup();
	delete m_poEvents;
	m_poEvents = NULL;

	Mix_Quit();
	TTF_Quit();
#if SDL_IMAGE_PATCHLEVEL > 6
	IMG_Quit();
#endif
	SDL_Quit();
}

ImageLibrary&
App::GetImageLibrary()
{
	assert(m_poImageLibrary != NULL);
	return *m_poImageLibrary;
}

Events&
App::GetEvents()
{
	assert(m_poEvents != NULL);
	return *m_poEvents;
}

void
App::Launch()
{
	assert(m_poCurrentApp != NULL);
	m_poCurrentApp->Init();
	m_poCurrentApp->Run();
	m_poCurrentApp->Cleanup();
}

void
App::Run()
{
	m_poCurrentApp = m_poClock;
	Launch();
	while (1) {
		// Ensure there are no lingering events
		m_poEvents->Reset();

		// Launch the menu
		m_poCurrentApp = m_poMenu;
		Launch();

		// Reset events, then execute the user's choice
		m_poEvents->Reset();
		switch(m_poMenu->GetSelectedOption()) {
			case Menu::m_ciOptionPhoto:
				m_poCurrentApp = m_poPhotoViewer;
				Launch();
				break;
			case Menu::m_ciOptionGame:
				m_poCurrentApp = m_poGame;
				Launch();
				break;
			case Menu::m_ciOptionQuit:
				return;
		}
	}
}

int
main(int argc, char* argv[])
{
	g_oApp.Init(argc, argv);
	g_oApp.Run();
	g_oApp.Cleanup();
	return 0;
}

/* vim:set ts=2 sw=2: */
