#pragma once
#include <SDL.h>
#include <vector>

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
	Vector2f pos;
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

	void MoveSnake(float deltaTime);
	void CheckCollisions();
	void Reset();

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;

	bool mIsRunning;
	State mState;
	int mScore;
	int mHiScore;

	std::vector<Segment> mSnake;
	Vector2i mFruitPos;
	Vector2i mNewDirection;

	Uint32 mTicksCount;
};