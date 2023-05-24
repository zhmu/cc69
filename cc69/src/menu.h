#ifndef __MENU_H__
#define __MENU_H__

#include "runnable.h"

class MessageWindow;
class Image;

class Menu : public IRunnable
{
public:
	Menu();
	~Menu();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

	int GetSelectedOption() const { return m_iSelectedOption; }

	static const int m_ciOptionPhoto = 0;
	static const int m_ciOptionGame = 1;
	static const int m_ciOptionQuit = 2;

protected:
	void HandleEvents();
	void Render();

private:
	//! \brief Number of menu options
	static const int m_ciNumOptions = 3;

	//! \brief Option chosen, -1 if none yet
	int m_iSelectedOption;

	//! \brief The options
	MessageWindow* m_poOption[m_ciNumOptions];

	//! \brief Background image 
	Image* m_poBackgroundImage;
};

#endif /* __MENU_H__ */
