#include "menu.h"
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <assert.h>
#include "app.h"
#include "events.h"
#include "fireworks.h"
#include "image.h"
#include "messagewindow.h"

Fireworks* m_poFireworks;

Menu::Menu()
	: m_poBackgroundImage(NULL)
{
	for (int i = 0; i < m_ciNumOptions; i++)
		m_poOption[i] = NULL;
	m_poFireworks = NULL;
}

Menu::~Menu()
{
	assert(m_poFireworks == NULL);
	assert(m_poBackgroundImage == NULL);
	for (int i = 0; i < m_ciNumOptions; i++)
		assert(m_poOption[i] == NULL);
}

void
Menu::Init()
{
	// Create the options
	float fOptionWidth = g_oApp.GetScreenWidth() * 0.75f;
	float fOptionHeight = (g_oApp.GetScreenWidth() * 0.5f) / (m_ciNumOptions + 2);
	for (int i = 0; i < m_ciNumOptions; i++) {
		m_poOption[i] = new MessageWindow(fOptionWidth, fOptionHeight, 10.0f);
		m_poOption[i]->Create();
		m_poOption[i]->SetPosition(
		 (g_oApp.GetScreenWidth() - fOptionWidth) / 2.0f,
		 (g_oApp.GetScreenHeight() * 0.5f) + (fOptionHeight * 1.25f) * i
		);
	}
	m_poOption[0]->Update("Plaatjes kijken");
	m_poOption[1]->Update("Spelletje spelen");
	m_poOption[2]->Update("Doei!");

	// Load the background image
	m_poBackgroundImage = new Image(g_oApp.GetDataPath() + "/menu_background.jpg");
	if (!m_poBackgroundImage->Load()) {
		std::cerr << "fatal: cannot load menu background image\n";
		exit(EXIT_FAILURE);
	}

	m_poFireworks = new Fireworks();
	m_poFireworks->Initialize();

	g_oApp.GetEvents().SetLEDs(false, false);
}

void
Menu::Cleanup()
{
	for (int i = 0; i < m_ciNumOptions; i++) {
		delete m_poOption[i];
		m_poOption[i] = NULL;
	}
	delete m_poBackgroundImage;
	m_poBackgroundImage = NULL;
	delete m_poFireworks;
	m_poFireworks = NULL;
}

void
Menu::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);

	// Draw background
	m_poBackgroundImage->RenderFullScreen();

	// Draw the options
	for (int i = 0; i < m_ciNumOptions; i++)
		m_poOption[i]->Render();

	glTranslatef(g_oApp.GetScreenWidth() / 2.0f, g_oApp.GetScreenHeight() / 2.0f, 0.2f);
	m_poFireworks->Render();

	// Render
	SDL_GL_SwapBuffers();
}

void
Menu::HandleEvents()
{
	Events& oEvents = g_oApp.GetEvents();
	oEvents.Process();

	m_poFireworks->Evolve();

	if (oEvents.CheckAndResetExit()) {
		m_iSelectedOption = 2;
		return;
	}

	// Let's assume all buttons are equal
	float fButtonHeight = m_poOption[0]->GetHeight();
	float fButtonWidth = m_poOption[0]->GetWidth();

	Events::Point oPoint;
	while (oEvents.GetAndRemoveMouseEvent(oPoint)) {
		for (int i = 0; i < m_ciNumOptions; i++) {
			float fX = m_poOption[i]->GetX();
			float fY = m_poOption[i]->GetY();
			if (oPoint.GetX() >= fX && (oPoint.GetX() < (fX + fButtonWidth)) &&
			    oPoint.GetY() >= fY && (oPoint.GetY() < (fY + fButtonHeight))) {
				// Got it
				m_iSelectedOption = i;
				break;
			}
		}
	}
}

void
Menu::Run()
{
	m_iSelectedOption = -1;
	while(m_iSelectedOption < 0) {
		HandleEvents();
		Render();
		SDL_Delay(1000 / g_oApp.GetDesiredFPS());
	}
}

/* vim:set ts=2 sw=2: */
