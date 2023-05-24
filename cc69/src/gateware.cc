#include "gateware.h"
#include <assert.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

Gateware::Gateware()
	: m_iFD(-1)
{
}

Gateware::~Gateware()
{
	assert(m_iFD == -1);
}

int
Gateware::SendCommandInternal(Message& oMsg, Message* pReply, bool bHaveReply)
{
	uint8_t buf[m_ciMaxMessageLength];
	int len = 1;
	buf[0] = (oMsg.code << 5) | oMsg.pid;
	buf[1] = oMsg.length;
	if (oMsg.length > 0) {
		memcpy(&buf[2], &oMsg.data[0], oMsg.length);
		len += 1 + oMsg.length;
	}
	if (write(m_iFD, buf, len) != len)
		assert(0);

#ifdef GW_DEBUG
	fprintf(stderr, "send %u bytes:", len);
	for (int n = 0; n < len; n++)
		fprintf(stderr, " %02x", buf[n]);
	fprintf(stderr, "\n");
#endif

	// If there is no reply at all, bail immediately
	if (!bHaveReply)
		return 0;

	// Fetch all resulting bytes
	int state = 0, left = -1;
	while (1) {
		char ch;
		if (read(m_iFD, &ch, sizeof(ch)) != sizeof(ch))
			err(1, "short read");
#ifdef GW_DEBUG
		fprintf(stderr, "state %u data %02x\n", state, ch);
#endif
		switch(state) {
			case 0: // pid + code
				if (pReply == NULL)
					return ch;
				pReply->pid = ch & 0x1f;
				pReply->code = ch >> 5;
				state++;
				break;
			case 1: // length
				pReply->length = ch + 1;
				left = ch + 1;
				state++;
				break;
			default: // data, state = 2+
				pReply->data[state - 2] = ch;
				state++; left--;
				break;
		}
		if (left == 0) {
#ifdef GW_DEBUG
			fprintf(stderr, "packet complete\n");
#endif
			return 1;
		}
	}

	/* NOTREACHED */
	return 0;
}

bool
Gateware::SendCommand(Message& oMsg, Message& oReply)
{
	return !!SendCommandInternal(oMsg, &oReply);
}

bool
Gateware::SendCommand(Message& oMsg)
{
	return !!SendCommandInternal(oMsg, NULL);
}

void
Gateware::SetLedPwm(int iStart, int iStop)
{
	Message oMsg;
	oMsg.pid = m_ciPidMisc;
	oMsg.code = m_ciTXCommand;
	oMsg.length = 4;
	oMsg.data[0] = 1; /* CMD_MISCP_WRITE_CONTROL */
	oMsg.data[1] = 0;
	oMsg.data[2] = (iStart > 0) ? (0x80 | iStart) : 0;
	oMsg.data[3] = (iStop > 0) ? (0x80 | iStop) : 0;
	SendCommandInternal(oMsg, NULL, false);
}

void
Gateware::SetLed(int iLed, bool bOn)
{
	Message oMsg;
	oMsg.pid = m_ciPidMisc;
	oMsg.code = m_ciTXCommand;
	oMsg.length = 4;
	oMsg.data[0] = 1; /* CMD_MISCP_WRITE_CONTROL_STATUS */
	oMsg.data[1] = (bOn ? 0x80 : 0) | iLed;
	oMsg.data[2] = 0;
	oMsg.data[3] = 0;
	SendCommandInternal(oMsg, NULL, false);
}

void
Gateware::SetStopLED(bool bOn)
{
	SetLed(1, bOn);
}

void
Gateware::SetStartLED(bool bOn)
{
	SetLed(2, bOn);
}

bool
Gateware::ResetPeripheral(int iPid)
{
	Message oMsg;
	oMsg.pid = iPid;
	oMsg.code = m_ciTXSetReset;
	oMsg.length = 0;
	SendCommandInternal(oMsg, NULL, false);

	oMsg.pid = iPid;
	oMsg.code = m_ciTXReleaseReset;
	oMsg.length = 0;
	return SendCommand(oMsg);
}

bool
Gateware::GetButtonAndHandwheelStatus(bool& bStartPressed, bool& bStopPressed, int& iHandwheel)
{
	// By default, nothing is hit
	bStartPressed = false; bStopPressed = false; iHandwheel = 0;

	// Fetch the status
	Message oMsg, oReply;
	oMsg.pid = m_ciPidMisc;
	oMsg.code = m_ciTXCommand;
	oMsg.length = 1;
	oMsg.data[0] = 0; /* CMD_MISCP_READ_STATUS */
	if (!SendCommand(oMsg, oReply))
		return false;

#ifdef GW_DEBUG
	fprintf(stderr, "got data: pid = %u, code = %u, len = %u, data ->",
		oReply.pid, oReply.code, oReply.length);
	for (int n = 0; n < oReply.length; n++) {
		printf(" %02x", oReply.data[n]);
	}
	printf("\n");
#endif
	
	bStopPressed = oReply.data[0] & 1;
	bStartPressed = oReply.data[0] & 2;
	iHandwheel = ((unsigned int)oReply.data[2] << 8) | oReply.data[1];
	return true;
}

bool
Gateware::Init(const std::string& sDevice)
{
	struct termios opt;
	m_iFD = open(sDevice.c_str(), O_RDWR | O_NOCTTY);
	if (m_iFD < 0)
		return false;

	if (tcgetattr(m_iFD, &opt) < 0) {
		Cleanup();
		return false;
	}

  /* 115200 baud */
  cfsetispeed(&opt, B115200);
  cfsetospeed(&opt, B115200);

  /* 8 bits, no parity, 1 stop bit */
  opt.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
  opt.c_cflag |= CS8;

  /* no hardware/software flow control */
  opt.c_cflag &= ~CRTSCTS;
  opt.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* raw input */
  opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  opt.c_oflag &= ~OPOST;

  if (tcsetattr(m_iFD, TCSANOW, &opt) < 0) {
		Cleanup();
		return false;
	}

	// Synchronize access to the gateware
	char sync[m_ciMaxMessageLength];
	memset(sync, 0xff, sizeof(sync));
	if (write(m_iFD, sync, sizeof(sync)) != sizeof(sync)) {
		Cleanup();
		return false;
	}

	// Reset the misc peripheral; we don't care about anything else now
	if (!ResetPeripheral(m_ciPidMisc)) {
		Cleanup();
		return false;
	}

	// All set
	return true;
}

void
Gateware::Cleanup()
{
	assert(m_iFD >= 0);
	close(m_iFD);
	m_iFD = -1;
}

/* vim:set ts=2 sw=2: */
