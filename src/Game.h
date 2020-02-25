#pragma once
#include <SDL.h>
#include <vector>
#include "SoundManager.h"
#include "TextGenerator.h"

enum State {GAME_ACTIVE, GAME_OVER};

struct Vector2f
{
	float x, y;
};

struct Vector2i
{
	int x, y;
};

struct Segment
{
	Vector2i pos;
	Vector2i direction;
};

class Game
{
public:
	Game();
	bool Init();
	void Run();
	void Shutdown();

private:
	void ProcessInput();
	void Update();
	void Render();

	void CheckCollisions();
	void Reset();
	void InitFruit();

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;

	SoundManager mSoundManager;
	TextGenerator mTextGenerator;

	bool mIsRunning;
	State mState;
	int mScore;
	int mHiScore;

	std::vector<Segment> mSnake;
	Vector2i mFruitPos;
	Vector2i mNewDirection;

	Uint32 mTicksCount;
	float mMoveTimer;
	float mResetTimer;
};