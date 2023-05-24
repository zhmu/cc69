#ifndef __MESSAGEWINDOW_H__
#define __MESSAGEWINDOW_H__

#include <string>
#include "texture.h"

class MessageWindow
{
public:
	//! \brief Constructor of the message window
	MessageWindow(int iWidth, int iHeight, int iCornerRadius);

	//! \brief Destroys the message window
	~MessageWindow();

	//! \brief Creates our message window
	void Create();

	//! \brief Sets the message window position
	void SetPosition(float fX, float fY);

	/*! \brief Sets the text position within the window
	 *  \param iX X position
	 *  \param iY Y position
	 *  
	 *  The text is centered by default; use '-1' for a coordinate to
	 *  center it along that axis.
	 */
	void SetTextPosition(int iX, int iY) {
		m_iTextX = iX; m_iTextY = iY;
	}

	/*! \brief Updates the message window text
	 *  \param sMessage Message to use
	 */
	void Update(const std::string& sMessage);

	//! \brief Renders the object
	void Render();

	//! \brief Retrieve the X position
	float GetX() const { return m_fX; }

	//! \brief Retrieve the Y position
	float GetY() const { return m_fY; }

	//! \brief Retrieve the window height
	float GetHeight() { return m_iHeight; }

	//! \brief Retrieve the window width 
	float GetWidth() { return m_iWidth; }

private:
	//! \brief Texture used
	Texture m_oTexture;

	//! \brief SDL surface used
	SDL_Surface* m_pSurface;

	//! \brief Current text
	std::string m_sMessage; 

	//! \brief Height of our message window
	int m_iHeight;

	//! \brief Width of our message window
	int m_iWidth;

	//! \brief Round corners radius
	int m_iRadius;

	//! \brief Text X coordinate, or -1 to center
	int m_iTextX;

	//! \brief Text Y coordinate, or -1 to center
	int m_iTextY;

	//! \brief Background color
	static const unsigned int m_iBackgroundColor = 0x80909090;

	//! \brief Text color
	static const unsigned int m_iTextColor = 0x000000;

	//! \brief X coordinate of the message window
	float m_fX;

	//! \brief Y coordinate of the message window
	float m_fY;
};

#endif /* __MESSAGEWINDOW_H__ */
