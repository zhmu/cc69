#include "events.h"
#include <SDL/SDL.h>
#include <assert.h>
#include "app.h"
#include "gateware.h"

Events::Events()
	: m_poGateware(NULL), m_iPreviousHandwheel(0)
{
	Reset();
}

Events::~Events()
{
	assert(m_poGateware == NULL);
}

bool
Events::Init(const std::string& sDevice)
{
	m_poGateware = new Gateware();
	if (!m_poGateware->Init(sDevice)) {
		delete m_poGateware;
		m_poGateware = NULL;
		return false;
	}

	// Fetch the initial button and handwheel status
	return m_poGateware->GetButtonAndHandwheelStatus(m_bPreviousStart, m_bPreviousStop, m_iPreviousHandwheel);
}

void
Events::Cleanup()
{
	if (m_poGateware != NULL)
		m_poGateware->Cleanup();
	delete m_poGateware;
	m_poGateware = NULL;
}

void
Events::Reset()
{
	m_bLeft = false;
	m_bRight = false;
	m_bStart = false;
	m_bPlus = false;
	m_bMinus = false;
	m_bExit = false;
	m_bPreviousStart = false;
	m_bPreviousStop = false;
	m_oMouseEvents.clear();
}

bool
Events::GetAndRemoveMouseEvent(Point& oPoint)
{
	if (m_oMouseEvents.empty())
		return false;

	oPoint = m_oMouseEvents.front();
	m_oMouseEvents.pop_front();
	return true;
}

void
Events::SetLEDs(int iStart, int iStop)
{
	if (m_poGateware == NULL)
		return;

	m_poGateware->SetStartLED(iStart != m_ciLedOff);
	m_poGateware->SetStopLED(iStop != m_ciLedOff);
}

void
Events::Process()
{
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					case SDLK_ESCAPE:
						m_bExit = true;
						break;
					case SDLK_LEFT:
						m_bLeft = true;
						break;
					case SDLK_TAB:
						m_bStart = true;
						break;
					case SDLK_RIGHT:
						m_bRight = true;
						break;
					case SDLK_DOWN:
						m_bStop = true;
						break;
					case SDLK_LEFTBRACKET:
						m_bMinus = true;
						break;
					case SDLK_RIGHTBRACKET:
						m_bPlus = true;
						break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if ((float)event.motion.x >= (float)g_oApp.GetScreenWidth() * 0.75f)
					m_bRight = true;
				if ((float)event.motion.x <= (float)g_oApp.GetScreenWidth() * 0.25f)
					m_bLeft = true;
				m_oMouseEvents.push_back(Point((int)event.motion.x, (int)event.motion.y));
				break;
			case SDL_QUIT:
				m_bExit = true;
				break;
		}
	}

	if (m_poGateware == NULL)
		return;

	bool bStart, bStop;
	int iHandwheel;
	if (m_poGateware->GetButtonAndHandwheelStatus(bStart, bStop, iHandwheel)) {
		if (m_bPreviousStart && !bStart) {
			m_bPreviousStart = false;
			m_bStart = true;
		} else {
			m_bPreviousStart = bStart;
			m_bStart = false;
		}
		if (m_bPreviousStop && !bStop) {
			m_bPreviousStop = false;
			m_bStop = true;
		} else {
			m_bPreviousStop = bStop;
			m_bStop = false;
		}
		if (iHandwheel != m_iPreviousHandwheel) {
			m_bMinus |= iHandwheel < m_iPreviousHandwheel;
			m_bPlus |= iHandwheel > m_iPreviousHandwheel;
			m_iPreviousHandwheel = iHandwheel;
		}
	}
}

/* vim:set ts=2 sw=2: */
