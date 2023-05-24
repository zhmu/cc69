#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <list>
#include <string>

class Gateware;

class Events
{
public:
	Events();
	~Events();

	/*! \brief Initialize the event subsystem
	 *  \param sDevice Gateware device to use
	 *
	 *  Note that the inability to use the gateware device is not fatal.
	 */
	bool Init(const std::string& sDevice);

	//! \brief Cleans the event subsystem up
	void Cleanup();

	//! \brief Process pending events, if any
	void Process();

	//! \brief Resets all pending events
	void Reset();

	/*! \brief Sets the start/stop leds
	 *  \param iStart Start led value
	 *  \param iStop Stop led value
	 *
	 *  As for value, m_ciLedXXX is to be used.
	 */
	void SetLEDs(int iStart, int iStop);

	//! \brief Led: off
	static const int m_ciLedOff = 0;

	//! \brief Led: on
	static const int m_ciLedOn = 1;

	//! \brief Check and reset for the ... event
#define MAKE_CHECK_AND_RESET(x) \
	bool CheckAndReset##x() { \
		bool b = m_b##x; \
		m_b##x = false; \
		return b; \
	}

	MAKE_CHECK_AND_RESET(Left)
	MAKE_CHECK_AND_RESET(Right)
	MAKE_CHECK_AND_RESET(Start)
	MAKE_CHECK_AND_RESET(Stop)
	MAKE_CHECK_AND_RESET(Exit)
	MAKE_CHECK_AND_RESET(Plus)
	MAKE_CHECK_AND_RESET(Minus)

#undef MAKE_CHECK_AND_RESET

	//! \brief Rodent events
	class Point {
	public:
		Point() : m_iX(0), m_iY(0) { }
		Point(int iX, int iY) : m_iX(iX), m_iY(iY) { }

		int GetX() const { return m_iX; }
		int GetY() const { return m_iY; }
	private:
		int m_iX, m_iY;
	};

	/*! \brief Get and remove a rodent event
	 *  \param oPoint Point to fill out
	 *  \returns true on success
	 */
	bool GetAndRemoveMouseEvent(Point& oPoint);

private:

	//! \brief Has the 'left' event occured
	bool m_bLeft;

	//! \brief Has the 'right' event occured
	bool m_bRight;

	//! \brief Has the 'start' event occured
	bool m_bStart;

	//! \brief Has the 'stop' event occured
	bool m_bStop;

	//! \brief Has the 'plus' event occured
	bool m_bPlus;

	//! \brief Has the 'minus' event occured
	bool m_bMinus;

	//! \brief Has the 'exit' event occured
	bool m_bExit;

	//! \brief Previous 'start' button status
	bool m_bPreviousStart;

	//! \brief Previous 'stop' button status
	bool m_bPreviousStop;

	//! \brief Previous handwheel value
	int m_iPreviousHandwheel;

	//! \brief Gateware interface
	Gateware* m_poGateware;

	//! \brief Pending mouse events
	std::list<Point> m_oMouseEvents;
};

#endif /* __EVENTS_H__ */
