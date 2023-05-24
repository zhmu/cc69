#ifndef __GAME_H__
#define __GAME_H__

#include <list>
#include <vector>
#include "runnable.h"
#include "texture.h"

class Image;
class MessageWindow;

class Game : public IRunnable
{
public:
	Game();
	~Game();

	virtual void Init();
	virtual void Cleanup();
	virtual void Run();

	//! \brief Retrieve the block width
	float GetBlockWidth() { return m_fBlockWidth; }

	//! \brief Retrieve the block height 
	float GetBlockHeight() { return m_fBlockHeight; }

	//! \brief Retrieve the block texture
	Texture& GetBlockTexture() { return m_oBlockTexture; }

private:
	class Block;

protected:
	void HandleEvents();
	void HandleLogic();
	void Render();

	void NewShape();
	void RenderCurrentShape();
	void TryToMoveCurrentShape(int iX);
	void TryToRotateCurrentShape(int iDirection);
	void PlaceCurrentShape();
	void DropCurrentShape();
	void GameOver();
	bool MarkCompletedLines();
	bool HandleCompletedLines();
	void Restart();

	bool CheckCurrentShapeColission(int iDirection);

	//! \brief Type of a block in the playfield
	typedef int TBlockNum;

	//! \brief Retrieve a block in the playfield
	Block*& BlockAt(int iX, int iY);
	const Block*& BlockAt(int iX, int iY) const;

	//! \brief Retrieve the current shape's X
	int GetCurrentShapeX() const { return m_iShapeX; }

	//! \brief Retrieve the current shape's Y
	int GetCurrentShapeY() const { return m_iShapeY; }

	//! \brief Retrieve the current shape's rotation
	int GetCurrentShapeRotation() const { return m_iShapeRotation; }

	//! \brief Increments the blocks-per-row counter for a given row
	void IncrementBlocksPerRow(int iRow);

	//! \brief Increments the score and updates the window
	void IncrementScore(int iValue);

private:
	//! \brief Texture containg the block images
	Texture m_oBlockTexture;

	//! \brief Background image
	Image* m_poBackgroundImage;

	//! \brief Are we still here?
	bool m_bLeaving;

	//! \brief Score
	unsigned int m_uiScore;

	//! \brief Game speed
	int m_iSpeed;

	//! \brief Current speed counter
	int m_iSpeedCounter;

	//! \brief Number of blocks
	static const int m_ciNumBlocks = 7;

	//! \brief Number of shapes 
	static const int m_ciNumShapes = 7;

	//! \brief Describes a block within the texture
	class Block {
	public:
		Block(Game& oGame, float fLeft, float fTop, float fRight, float fBottom)
		 : m_oGame(oGame), m_fLeft(fLeft), m_fTop(fTop), m_fRight(fRight), m_fBottom(fBottom)
		{
		}

		void Render();
		Game& GetGame() { return m_oGame; }

		//! \brief Copy constructor, used by std::vector::push_back()
		Block& operator=(const Block& oA) {
			m_oGame = oA.m_oGame;
			m_fLeft = oA.m_fLeft;
			m_fTop = oA.m_fTop;
			m_fBottom = oA.m_fBottom;
			m_fRight = oA.m_fRight;
			return *this;
		}

	private:
		Block(); // not to be used

		//! \brief Game we belong to
		Game& m_oGame;

		//! \brief Block coordinates within the texture
		float m_fLeft, m_fTop, m_fRight, m_fBottom;
	};
	typedef std::vector<Block> TBlockVector;
	friend class Game::Block;

	//! \brief Describes a shape
	class Shape {
	public:
		/*! \brief Constructs a new shape
		 *  \param pBlock Blocks the shape consists of
		 */
		Shape(Block* pBlock);

		//! \brief Adds a point to the shape
		void AddPoint(int iX, int iY) {
			m_oPoint.push_back(Point(iX, iY));
		}

		/*! \brief Renders the shape using given blocks */
		void Render(Block& oBlock, int iRotate);

		//! \brief Directions
		static const int m_ciDirEast = 0;
		static const int m_ciDirWest = 1;
		static const int m_ciDirNorth = 2;
		static const int m_ciDirSouth = 3;
		static const int m_ciDirSouthEast = 4;

		//! \brief Check for a collision
		bool CheckCollision(const Game& oGame, int iRotate) const;

		//! \brief Places the shape within the game
		void Place(Game& oGame) const;

	private:
		//! \brief Point within this block
		class Point {
		public:
			Point(int iX, int iY) : m_iX(iX), m_iY(iY) { }
			Point(const Point& oPoint, int iRotate);

			int GetX() const { return m_iX; }
			int GetY() const { return m_iY; }
		private:
			int m_iX, m_iY;
		};
		typedef std::list<Point> TPointList;

		//! \brief List of points
		TPointList m_oPoint;

		//! \brief Block used to draw the shape
		Block* m_pBlock;
	};
	typedef std::vector<Shape> TShapeVector;

	//! \brief Block height, in pixels
	float m_fBlockHeight;

	//! \brief Block width, in pixels
	float m_fBlockWidth;

	//! \brief Blocks
	TBlockVector m_oBlock;

	//! \brief Shapes
	TShapeVector m_oShape;

	//! \brief Playfield coordinates
	float m_fX, m_fY;

	//! \brief Coordinate of current shape
	int m_iShapeX, m_iShapeY;

	//! \brief Current shape
	int m_iShapeIndex;

	//! \brief Next shape
	int m_iNextShapeIndex;

	//! \brief Shape rotation
	int m_iShapeRotation;

	//! \brief Are we playing?
	bool m_bPlaying;

	//! \brief Are controls allowed?
	bool m_bPlayerInControl;

	//! \brief Number of rows
	static const int m_ciNumRows = 21;

	//! \brief Number of columns 
	static const int m_ciNumCols = 10;

	//! \brief Game field
	Block** m_aiGameField;

	//! \brief Number of blocks per row
	int* m_aiBlocksPerRow;

	//! \brief Lines-removing animation counter
	int m_iRemovingLines;

	//! \brief Amount of frames to remove the row
	static const int m_ciRemoveTime = 20; /* 0.3 sec at 60 fps */

	//! \brief Score window
	MessageWindow* m_poScoreWindow;

	//! \brief Next block window
	MessageWindow* m_poNextBlockWindow;

	//! \brief Status window
	MessageWindow* m_poStatusWindow;
};

#endif /* __GAME_H__ */
