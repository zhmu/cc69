#ifndef __GATEWARE_H__
#define __GATEWARE_H__

#include <string>

class Gateware
{
public:
	//! \brief Constructor
	Gateware();

	//! \brief Destructor
	~Gateware();

	/*! \brief Initialize the miniature gateware interface
	 *  \param sDevice Path to the device
	 *  \returns true on success
	 */
	bool Init(const std::string& sDevice);

	//! \brief Cleans the gateware interface up
	void Cleanup();

	/*! \brief Sets the start/stop LED PWM values
	 *  \param iStart Start LED PWM value, if > 0
	 *  \param iStop Stop LED PWM value, if > 0
	 */
	void SetLedPwm(int iStart, int iStop);

	/*! \brief Sets the start LED state
	 *  \param bOn Light LED if true
	 */
	void SetStartLED(bool bOn);

	/*! \brief Sets the stop LED state
	 *  \param bOn Light LED if true
	 */
	void SetStopLED(bool bOn);

	/*! \brief Read the buttons and handwheel
	 *  \param bStartPressed Was start pressed?
	 *  \param bStopPressed Was stop pressed?
	 *  \param iHandwheel Handwheel value
	 *  \returns true on success
	 */
	bool GetButtonAndHandwheelStatus(bool& bStartPressed, bool& bStopPressed, int& iHandwheel);

protected:
	//! \brief Maximum length of a gateware message
	static const int m_ciMaxMessageLength = 16;

	//! \brief Misc peripheral ID
	static const int m_ciPidMisc = 1;

	//! \brief Transmit code: command
	static const int m_ciTXCommand = 0;

	//! \brief Transmit code: set reset
	static const int m_ciTXSetReset = 1;

	//! \brief Transmit code: release reset
	static const int m_ciTXReleaseReset = 2;

	//! \brief A gateware message or response
	class Message {
	public:
		int pid;
		int code;
		int length;
		unsigned char data[m_ciMaxMessageLength];
	};

	/*! \brief Sends a gateware command message
	 *  \param oMsg Message to send
	 *  \param oReply Reply buffer
	 *  \returns true on success
	 */
	bool SendCommand(Message& oMsg, Message& oReply);

	/*! \brief Sends a gateware command message without reply
	 *  \param oMsg Message to send
	 *  \returns true on success
	 */
	bool SendCommand(Message& oMsg);

	/*! \brief Sends a gateware command message
	 *  \param oMsg Message to send
	 *  \param pReply Reply buffer, if any
	 *  \param bExpectReply If false, no reply will come at all
	 *  \returns Received command byte if pReply is NULL, otherwise non-zero on success
	 */
	int SendCommandInternal(Message& oMsg, Message* pReply, bool bExpectReply = true);

	/*! \brief Sets a LED state
	 *  \param iLed LED to set
	 *  \param bOn Light LED if true
	 */
	void SetLed(int iLed, bool bOn);

	/*! \brief Resets a peripheral
	 *  \param iPid Peripheral ID to reset
	 */
	bool ResetPeripheral(int iPid);

	//! \brief File descriptor
	int m_iFD;
};

#endif /* __GATEWARE_H__ */
