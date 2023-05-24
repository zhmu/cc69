#include "game.h"
#include <assert.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "app.h"
#include "events.h"
#include "image.h"
#include "messagewindow.h"
#include "random.h"

Game::Game()
	: m_poBackgroundImage(NULL), m_aiGameField(NULL), m_aiBlocksPerRow(NULL),
	  m_poScoreWindow(NULL), m_poNextBlockWindow(NULL), m_poStatusWindow(NULL)
{
	m_fX = 10.0f; m_fY = 10.0f;
	m_iSpeed = 10; m_iSpeedCounter = 0;
}

Game::~Game()
{
	// If this fires, you didn't call Cleanup()
	assert(m_aiGameField == NULL);
	assert(m_aiBlocksPerRow == NULL);
	assert(m_poBackgroundImage == NULL);
	assert(m_poScoreWindow == NULL);
	assert(m_poNextBlockWindow == NULL);
	assert(m_poStatusWindow == NULL);
}

void
Game::Init()
{
	std::string sBlockFile(g_oApp.GetDataPath() + "/game_blocks.png");
	SDL_Surface* pBlocks = IMG_Load(sBlockFile.c_str());
	if (pBlocks == NULL) {
		std::cerr << "fatal: cannot open game block image\n";
		exit(EXIT_FAILURE);
	}

	m_poBackgroundImage = new Image(g_oApp.GetDataPath() + "/game_background.jpg");
	if (!m_poBackgroundImage->Load()) {
		std::cerr << "fatal: cannot open game background image\n";
		exit(EXIT_FAILURE);
	}

	/* XXX Maybe this shouldn't be hardcoded here */
	int iRows = 2;
	int iCols = (m_ciNumBlocks + 1) / iRows;

	// Ensure our image is sane
	m_fBlockHeight = (float)(pBlocks->w / iCols);
	m_fBlockWidth = (float)(pBlocks->h / iRows);
	assert(m_fBlockWidth == m_fBlockHeight);
	
	// Image checks out; make it a texture
	if(!m_oBlockTexture.ConvertSurface(pBlocks))
		assert(0);
	SDL_FreeSurface(pBlocks);

	// Figure our the block coordinates within the texture
	for (int i = 0; i < m_ciNumBlocks; i++) {
		float fRow = (float)(i / iCols);
		float fCol = (float)(i % iCols);
		m_oBlock.push_back(Block(
			*this,
			(fCol * m_fBlockWidth)        / m_oBlockTexture.GetTextureWidth(),
			(fRow * m_fBlockHeight)       / m_oBlockTexture.GetTextureHeight(),
			((fCol + 1) * m_fBlockWidth)  / m_oBlockTexture.GetTextureWidth(),
			((fRow + 1) * m_fBlockHeight) / m_oBlockTexture.GetTextureHeight()
		));
	}

#define CREATE_SHAPE(a, b, c, d) \
	{ \
		Shape& oShape = *m_oShape.insert(m_oShape.end(), Shape(&m_oBlock[m_oShape.size()])); \
		oShape.AddPoint a; oShape.AddPoint b; \
		oShape.AddPoint c; oShape.AddPoint d; \
	} 

	// Construct our shapes
	CREATE_SHAPE((-2,  0), (-1,  0), ( 0,  0), ( 1,  0)); // I
	CREATE_SHAPE(( 0, -1), ( 0,  0), ( 1,  0), ( 2,  0)); // J
	CREATE_SHAPE((-2,  0), (-1,  0), ( 0,  0), ( 0, -1)); // L
	CREATE_SHAPE(( 1, -1), ( 1,  0), ( 0,  0), ( 0, -1)); // O
	CREATE_SHAPE((-1,  0), ( 0,  0), ( 0, -1), ( 1, -1)); // S
	CREATE_SHAPE((-1,  0), ( 0,  0), ( 1,  0), ( 0, -1)); // T
	CREATE_SHAPE((-1, -1), ( 0, -1), ( 0,  0), ( 1,  0)); // Z

	// Windows
	m_poScoreWindow = new MessageWindow(m_fBlockWidth * 10.0f, m_fBlockHeight, 3);
	m_poScoreWindow->Create();
	m_poScoreWindow->SetPosition(m_fX + m_fBlockWidth * (m_ciNumCols + 1), m_fY);
	m_poNextBlockWindow = new MessageWindow(m_fBlockWidth * 10.0f, m_fBlockHeight * 4.0f, 3);
	m_poNextBlockWindow->Create();
	m_poNextBlockWindow->SetPosition(m_fX + m_fBlockWidth * (m_ciNumCols + 1), m_fY + m_fBlockHeight * 1.5f);
	m_poNextBlockWindow->SetTextPosition(-1, 0.5f * m_fBlockHeight);
	m_poNextBlockWindow->Update("Next block");
	m_poStatusWindow = new MessageWindow(m_fBlockWidth * 10.0f, m_fBlockHeight * 2.0f, 3);
	m_poStatusWindow->SetPosition(m_fX + m_fBlockWidth * (m_ciNumCols + 1), m_fY + m_fBlockHeight * 6.0f);
	m_poStatusWindow->Create();

	/*
	 * Initialize the game field. Our game field actually looks like this:
	 *
   *  0,0           m_ciNumCols+2,0
   *     \         /
	 *      X.......X
	 *      X.......X
	 *      X.......X
	 *      XXXXXXXXX <-- m_ciNumCols+2, m_ciNumRows+1
	 *     /
	 * 0,m_ciNumRows+1
	 *
	 * Where . are the visible pieces and the X is the off-screen border ensuring
	 * no pieces can leave the playfield.
	 */ 
	m_aiGameField = new Block*[(m_ciNumCols + 2) * (m_ciNumRows + 1)];
	Restart();
}

void
Game::Cleanup()
{
	m_oBlockTexture.Unload();
	delete m_aiGameField;
	delete m_aiBlocksPerRow;
	delete m_poBackgroundImage;
	delete m_poScoreWindow;
	delete m_poNextBlockWindow;
	delete m_poStatusWindow;
	m_aiGameField = NULL;
	m_aiBlocksPerRow = NULL;
	m_poBackgroundImage = NULL;
	m_poScoreWindow = NULL;
	m_poNextBlockWindow = NULL;
	m_poStatusWindow = NULL;
}

void
Game::Restart()
{
	for (int i = 0; i < (m_ciNumCols + 2) * (m_ciNumRows + 1); i++)
		m_aiGameField[i] = NULL;

	// Mark left and right columns as occupied
	Block* pBlock = &m_oBlock.front();
	for (int i = 0; i < m_ciNumRows + 1; i++) {
		BlockAt(0, i) = pBlock;
		BlockAt(m_ciNumCols + 1, i) = pBlock;
	}
	// Mark bottom row as occupied
	for (int i = 0; i < m_ciNumCols + 2; i++)
		BlockAt(i, m_ciNumRows) = pBlock;

	// There are no blocks anywhere
	m_aiBlocksPerRow = new int[m_ciNumRows];
	for (int i = 0; i < m_ciNumRows; i++)
		m_aiBlocksPerRow[i] = 0;

	// We're playing a game
	m_iShapeX = m_ciNumCols / 2;
	m_bPlaying = true;
	m_bPlayerInControl = true;
	m_uiScore = 0;
	IncrementScore(0); // to force an update
	m_poStatusWindow->Update("Playing - [STOP] to quit");

	// Reset any pending event
	g_oApp.GetEvents().Process();
	g_oApp.GetEvents().Reset();

#if 0
	/* XXX HACK HACK HACK */
	for (int i = 1; i < m_ciNumCols; i++) {
		BlockAt(i, m_ciNumRows - 4) = pBlock;
		BlockAt(i, m_ciNumRows - 3) = pBlock;
		BlockAt(i, m_ciNumRows - 2) = pBlock;
		BlockAt(i, m_ciNumRows - 1) = pBlock;
	}
	m_aiBlocksPerRow[m_ciNumRows - 4] = m_ciNumCols - 1;
	m_aiBlocksPerRow[m_ciNumRows - 2] = m_ciNumCols - 1;
	m_aiBlocksPerRow[m_ciNumRows - 3] = m_ciNumCols - 1;
	m_aiBlocksPerRow[m_ciNumRows - 1] = m_ciNumCols - 1;
#endif

	// Select a new shape
	m_iNextShapeIndex = -1; // no next shape yet
	NewShape();
	g_oApp.GetEvents().SetLEDs(Events::m_ciLedOff, Events::m_ciLedOn);
}

Game::Block*&
Game::BlockAt(int iX, int iY)
{
	assert(iX >= 0 && (iX < m_ciNumCols + 2));
	assert(iY >= 0 && (iY < m_ciNumRows + 1));
	return m_aiGameField[(iY * (m_ciNumCols + 2)) + iX];
}

const Game::Block*&
Game::BlockAt(int iX, int iY) const
{
	return (const Block*&)(const_cast<Game*>(this))->BlockAt(iX, iY);
}

void
Game::HandleEvents()
{
	Events& oEvents = g_oApp.GetEvents();
	oEvents.Process();

	if (oEvents.CheckAndResetExit()) {
		m_bLeaving = true;
		return;
	}

	if (!m_bPlayerInControl) {
		if (oEvents.CheckAndResetStart()) {
			Restart();
		} else if (oEvents.CheckAndResetStop()) {
			m_bLeaving = true;
		}
		return;
	}

	if (oEvents.CheckAndResetMinus()) {
		TryToMoveCurrentShape(-1);
	} else if (oEvents.CheckAndResetPlus()) {
		TryToMoveCurrentShape(1);
	} else if (oEvents.CheckAndResetRight()) {
		TryToRotateCurrentShape(1);
	} else if (oEvents.CheckAndResetLeft()) {
		TryToRotateCurrentShape(-1);
	} else if (oEvents.CheckAndResetStop()) {
		// Stop cancels playing
		GameOver();
	}
}

void
Game::Block::Render()
{
	float W = m_oGame.GetBlockWidth();
	float H = m_oGame.GetBlockHeight();
	float Z = 0;

	glBindTexture(GL_TEXTURE_2D, m_oGame.GetBlockTexture().GetTextureID());
	glBegin(GL_QUADS);
	glTexCoord2f(m_fLeft,  m_fTop); glVertex3f( 0, 0, Z);
	glTexCoord2f(m_fRight, m_fTop); glVertex3f( W, 0, Z);
	glTexCoord2f(m_fRight, m_fBottom); glVertex3f( W, H, Z);
	glTexCoord2f(m_fLeft,  m_fBottom); glVertex3f( 0, H, Z);
	glEnd();
}

Game::Shape::Shape(Block* pBlock)
	: m_pBlock(pBlock)
{
}

Game::Shape::Point::Point(const Point& oPoint, int iRotate)
{
	switch (iRotate) {
		case 0:
			m_iX = oPoint.m_iX;
			m_iY = oPoint.m_iY;
			break;
		case 1:
			m_iX = -oPoint.m_iY;
			m_iY =  oPoint.m_iX;
			break;
		case 2:
			m_iX = -oPoint.m_iX;
			m_iY = -oPoint.m_iY;
			break;
		case 3:
			m_iX =  oPoint.m_iY;
			m_iY = -oPoint.m_iX;
			break;
		default:
			assert(0);
	}
}

void
Game::Shape::Render(Block& oBlock, int iRotate)
{
	for (TPointList::iterator it = m_oPoint.begin(); it != m_oPoint.end(); it++) {
		Point oPoint(*it, iRotate);

		glPushMatrix();
		glTranslatef(oPoint.GetX() * oBlock.GetGame().GetBlockWidth(), oPoint.GetY() * oBlock.GetGame().GetBlockHeight(), 0);
		oBlock.Render();
		glPopMatrix();
	}
}

bool
Game::Shape::CheckCollision(const Game& oGame, int iRotate) const
{
	for (TPointList::const_iterator it = m_oPoint.begin(); it != m_oPoint.end(); it++) {
		Point oPoint(*it, iRotate);
		int iX = oGame.GetCurrentShapeX() + oPoint.GetX();
		int iY = oGame.GetCurrentShapeY() + oPoint.GetY();
		// XXX Kludge: some shapes may end at below 0 or greater than the maximum
		// depending on their rotation
		if (iX < 0 || iY < 0)
			return true;
		if (iX >= m_ciNumCols + 2)
			return true;
		const Block*& pBlock = oGame.BlockAt(iX, iY);
		if (pBlock != NULL)
			return true;
	}

	return false;
}

void
Game::Shape::Place(Game& oGame) const
{
	for (TPointList::const_iterator it = m_oPoint.begin(); it != m_oPoint.end(); it++) {
		Point oPoint(*it, oGame.GetCurrentShapeRotation());
		int iY = oGame.GetCurrentShapeY() + oPoint.GetY();
		Block*& pBlock = oGame.BlockAt(oGame.GetCurrentShapeX() + oPoint.GetX(), iY);
		assert(pBlock == NULL);

		// Place the block and increment the blocks per row-counter
		pBlock = m_pBlock;
		oGame.IncrementBlocksPerRow(iY);
	}
}

void
Game::IncrementBlocksPerRow(int iRow)
{
	// Update the per-row count; this lets us check quickly for filled rows
	assert(iRow >= 0 && iRow < m_ciNumRows);
	m_aiBlocksPerRow[iRow]++;
	assert(m_aiBlocksPerRow[iRow] >= 0 && m_aiBlocksPerRow[iRow] <= m_ciNumCols);
}

void
Game::RenderCurrentShape()
{
	Block& oBlock = m_oBlock[m_iShapeIndex];
	Shape& oShape = m_oShape[m_iShapeIndex]; 

	/*
	 * The -1 is necessary because the first column is not drawn (it's only used
	 * for collision checking
	 */
	glTranslatef((m_iShapeX - 1) * m_fBlockWidth, m_iShapeY * m_fBlockHeight, 0.0f);
	oShape.Render(oBlock, m_iShapeRotation);
}

void
Game::TryToMoveCurrentShape(int iX)
{
	Shape& oShape = m_oShape[m_iShapeIndex]; 

	m_iShapeX += iX;
	if (oShape.CheckCollision(*this, m_iShapeRotation)) {
		// Collision; cancel the movement
		m_iShapeX -= iX;
	}
}

void
Game::TryToRotateCurrentShape(int iDirection)
{
	Shape& oShape = m_oShape[m_iShapeIndex]; 
	int iPreviousRotation = m_iShapeRotation;

	if (iDirection > 0)
		m_iShapeRotation = (m_iShapeRotation + 1) % 4;
	else
		m_iShapeRotation = (m_iShapeRotation == 0) ? 3 : m_iShapeRotation - 1;
	if (oShape.CheckCollision(*this, m_iShapeRotation)) {
		// Collision; cancel the movement
		m_iShapeRotation = iPreviousRotation;
	}
}

void
Game::DropCurrentShape()
{
	Shape& oShape = m_oShape[m_iShapeIndex]; 

	for(;;) {
		int iPreviousY = m_iShapeY;
		m_iShapeY++;
		if (oShape.CheckCollision(*this, m_iShapeRotation)) {
			m_iShapeY = iPreviousY;
			oShape.Place(*this);
			if (!MarkCompletedLines())
				NewShape();
			return;
		}
	}
}

void
Game::NewShape()
{
	m_iShapeY = 1;
	if (m_iNextShapeIndex >= 0)
		m_iShapeIndex = m_iNextShapeIndex;
	else
		m_iShapeIndex = Random::Get(0, m_oShape.size() - 1);
	m_iShapeRotation = 0;
	m_iRemovingLines = 0;
	m_bPlayerInControl = true;
	m_iNextShapeIndex = Random::Get(0, m_oShape.size() - 1);

	/*
	 * If this new shape causes an immediate collision, see if we can
	 * help the player a bit by moving the shape around horizonally. If
	 * this fails, it's game over.
	 */
	Shape& oShape = m_oShape[m_iShapeIndex]; 
	if (oShape.CheckCollision(*this, m_iShapeRotation)) {
		m_iShapeX = 0;
		while(m_iShapeX < m_ciNumCols - 3 /* longest shape is I */) {
			if (!oShape.CheckCollision(*this, m_iShapeRotation))
				return;
			m_iShapeX++;
		}

		// Too bad, no escape from this hell
		GameOver();
	}
}

bool
Game::MarkCompletedLines()
{
	int iNumLines = 0;

	// First of all, flag all lines and increment score
	for (int iRow = m_ciNumRows - 1; iRow >= 0; iRow--) {
		if (m_aiBlocksPerRow[iRow] != m_ciNumCols)
			continue;
		m_aiBlocksPerRow[iRow] = -1;
		iNumLines++;
	}

	if (iNumLines == 0)
		return false;

	IncrementScore(powf(2, iNumLines - 1) * 100);
	m_bPlayerInControl = false;
	m_iRemovingLines = 1;
	return true;
}

bool
Game::HandleCompletedLines()
{
	if (m_iRemovingLines <= 0)
		return false;

	if (m_iRemovingLines <= m_ciRemoveTime) {
		m_iRemovingLines++;
		return true;
	}

	// Now remove all completed lines
	for (int iRow = m_ciNumRows - 1; iRow >= 0; /* nothing */) {
		if (m_aiBlocksPerRow[iRow] != -1) {
			iRow--;
			continue;
		}
	
		// This line is filled; we need to remove it
		memmove(&m_aiBlocksPerRow[1], &m_aiBlocksPerRow[0], iRow * sizeof(int));
		memmove(&m_aiGameField[m_ciNumCols + 2], &m_aiGameField[0], (m_ciNumCols + 2) * iRow * sizeof(Block*));

		// Don't decrement iRow; this row must be handled again
	}

	// All lines are gone; we're done removing them
	NewShape();
	return false;
}

void
Game::IncrementScore(int iValue)
{
	m_uiScore += iValue;
	char sScore[64];
	snprintf(sScore, sizeof(sScore), "Score: %u", m_uiScore);
	sScore[sizeof(sScore)] = '\0';
	m_poScoreWindow->Update(sScore);
}

void
Game::HandleLogic()
{
	if (HandleCompletedLines())
		return;

	if (!m_bPlaying || ++m_iSpeedCounter < m_iSpeed)
		return;
	m_iSpeedCounter = 0;

	int iPreviousY = m_iShapeY;
	m_iShapeY++;

	// If this shape causes a collision, place it
	Shape& oShape = m_oShape[m_iShapeIndex]; 
	if (oShape.CheckCollision(*this, m_iShapeRotation)) {
		m_iShapeY = iPreviousY;
		oShape.Place(*this);
		if (!MarkCompletedLines())
			NewShape();
		return;
	}
}

void
Game::GameOver()
{
	m_bPlaying = false;
	m_bPlayerInControl = false;
	g_oApp.GetEvents().SetLEDs(Events::m_ciLedOn, Events::m_ciLedOn);
	m_poStatusWindow->Update("Game over - [START] to play again, [STOP] to quit");
}

void
Game::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	// Start with the background
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.8f);
	m_poBackgroundImage->RenderFullScreen();
	glPopMatrix();

	// Corner off the playfield	
	glEnable(GL_BLEND);
	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	glBlendColor(1.0f, 1.0f, 1.0f, 0.5f);

	glTranslatef(m_fX, m_fY, 0.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	glVertex3f(0.0f, 0.0f, -0.5f);
	glVertex3f(m_ciNumCols * m_fBlockWidth, 0.0f, -0.5f);
	glVertex3f(m_ciNumCols * m_fBlockWidth, m_ciNumRows * m_fBlockHeight, -0.5f);
	glVertex3f(0, m_ciNumRows * m_fBlockHeight, -0.5f);
	glEnd();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Display all our blocks
	for (int y = 0; y < m_ciNumRows; y++) {
		glPushMatrix();
		glTranslatef(0.0f, y * m_fBlockWidth, -0.4f);
		if (m_aiBlocksPerRow[y] == -1) {
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
      glBlendColor(1.0f, 1.0f, 1.0f, 1.0f - ((float)m_iRemovingLines / (float)(m_ciRemoveTime + 1)));
		}
		for (int x = 0; x < m_ciNumCols; x++) {
			Block* pBlock = BlockAt(x + 1, y);
			if (pBlock != NULL)
				pBlock->Render();
			glTranslatef(m_fBlockWidth, 0.0f, 0.0f);
		}
		if (m_aiBlocksPerRow[y] == -1) {
			// Reset blend for the remaining rows
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		glPopMatrix();
	}

	// Show the current block if we're not removing lines
	if (!m_iRemovingLines)
		RenderCurrentShape();

	glLoadIdentity();
	m_poScoreWindow->Render();
	m_poStatusWindow->Render();
	m_poNextBlockWindow->Render();
	glEnable(GL_BLEND);

	if (m_iNextShapeIndex >= 0) {
		Block& oBlock = m_oBlock[m_iNextShapeIndex];
		Shape& oShape = m_oShape[m_iNextShapeIndex];
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
		glLoadIdentity();
		// Center the block within this window as much as we can
		float fX = m_poNextBlockWindow->GetX() + m_poNextBlockWindow->GetWidth() / 2.0f;
		float fY = m_poNextBlockWindow->GetY() + m_poNextBlockWindow->GetHeight() / 2.0f;
		glTranslatef(fX, fY, 0.5f);
		oShape.Render(oBlock, 0);
	}

	glDisable(GL_BLEND);

	// Render
	SDL_GL_SwapBuffers();
}

void
Game::Run()
{
	m_bLeaving = false;
	while(!m_bLeaving) {
		HandleEvents();
		HandleLogic();
		Render();
		SDL_Delay(1000 / g_oApp.GetDesiredFPS());
	}
}

/* vim:set ts=2 sw=2: */
