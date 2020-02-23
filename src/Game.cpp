#include "Game.h"
#include "Constants.h"
#include <iostream>

Game::Game() : mWindow(nullptr), mRenderer(nullptr), mIsRunning(true), mState(GAME_ACTIVE), mScore(0), mHiScore(0), mTicksCount(0) {}

bool Game::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Failed to initialise SDL. Error: %s\n", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!mWindow)
	{
		SDL_Log("Failed to create window. Error: %s\n", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer. Error: %s\n", SDL_GetError());
		return false;
	}

	mSnake.reserve(GRID_WIDTH * GRID_HEIGHT);
	Segment head{ (SCREEN_WIDTH - CELL_SIZE) / 2, (SCREEN_HEIGHT - CELL_SIZE) / 2, 0, 0 };
	mSnake.push_back(std::move(head));
	//
	mFruitPos.x = 0;
	mFruitPos.y = 0;

	return true;
}

void Game::Run()
{
	while (mIsRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}

void Game::ProcessInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	if (state[SDL_SCANCODE_W])
	{
		if (!(mSnake.size() > 1 && mSnake[0].direction.y == 1))
		{
			mNewDirection.x = 0;
			mNewDirection.y = -1;
		}
	}
	else if (state[SDL_SCANCODE_S])
	{
		if (!(mSnake.size() > 1 && mSnake[0].direction.y == -1))
		{
			mNewDirection.x = 0;
			mNewDirection.y = 1;
		}
	}
	else if (state[SDL_SCANCODE_A])
	{
		if (!(mSnake.size() > 1 && mSnake[0].direction.x == 1))
		{
			mNewDirection.x = -1;
			mNewDirection.y = 0;
		}
	}
	else if (state[SDL_SCANCODE_D])
	{
		if (!(mSnake.size() > 1 && mSnake[0].direction.x == -1))
		{
			mNewDirection.x = 1;
			mNewDirection.y = 0;
		}
	}
}

void Game::Update()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mTicksCount = SDL_GetTicks();
	mSnake[0].direction = mNewDirection;
	MoveSnake(deltaTime);
	CheckCollisions();
}

void Game::Render()
{
	//Clear back buffer
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mRenderer);

	//render grid
	SDL_SetRenderDrawColor(mRenderer, 76, 76, 76, 255);
	for (int x = 0; x < SCREEN_WIDTH; x += CELL_SIZE)
	{
		SDL_RenderDrawLine(mRenderer, x, 0, x, SCREEN_HEIGHT);
	}
	for (int y = 0; y < SCREEN_HEIGHT; y += CELL_SIZE)
	{
		SDL_RenderDrawLine(mRenderer, 0, y, SCREEN_WIDTH, y);
	}

	//render snake
	SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
	for (Segment& segment : mSnake)
	{
		SDL_Rect rect{
			static_cast<int>(segment.pos.x),
			static_cast<int>(segment.pos.y),
			CELL_SIZE,
			CELL_SIZE
			};
		SDL_RenderFillRect(mRenderer, &rect);
	}
	
	////snake's outline
	//SDL_SetRenderDrawColor(mRenderer, 20, 100, 0, 255);
	//for (Segment& segment : mSnake)
	//{
	//	SDL_Rect outlineRect{
	//		static_cast<int>(segment.pos.x),
	//		static_cast<int>(segment.pos.y),
	//		CELL_SIZE,
	//		CELL_SIZE
	//	};
	//	SDL_RenderDrawRect(mRenderer, &outlineRect);
	//}

	//render fruit
	SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
	SDL_Rect fruitRect{
		mFruitPos.x,
		mFruitPos.y,
		CELL_SIZE,
		CELL_SIZE
	};
	SDL_RenderFillRect(mRenderer, &fruitRect);

	//Swap front and back buffers
	SDL_RenderPresent(mRenderer);
}

void Game::MoveSnake(float deltaTime)
{
	for (Segment& segment : mSnake)
	{
		segment.pos.x += segment.direction.x * SNAKE_SPEED * deltaTime;
		segment.pos.y += segment.direction.y * SNAKE_SPEED * deltaTime;
	}

	//Update the snake's segments (excluding the head)
	for (std::vector<Segment>::reverse_iterator iter = mSnake.rbegin(); iter < mSnake.rend() - 1; ++iter)
	{
		iter->direction = (iter + 1)->direction;
	}
}

void Game::CheckCollisions()
{
	//Check if snake head has collided with a wall, its tail, or the fruit
	//wall
	Segment head = mSnake[0];
	if (head.pos.x < 0 || head.pos.x > SCREEN_WIDTH - CELL_SIZE || head.pos.y < 0 || head.pos.y > SCREEN_HEIGHT - CELL_SIZE)
	{
		//PlaySound()
		mState = GAME_OVER;
		Reset();

		return;
	}

	//tail
	for (std::vector<Segment>::size_type i = 1; i < mSnake.size(); ++i)
	{
		if (head.pos.x == mSnake[i].pos.x && head.pos.y == mSnake[i].pos.y)
		{
			//PlaySound();
			mState = GAME_OVER;
			Reset();
			return;
		}
	}

	if (head.pos.x == mFruitPos.x && head.pos.y == mFruitPos.y)
	{
		//PlaySound();
		Segment newSegment{ mSnake.back().pos.x, mSnake.back().pos.y, 0, 0 };
		mSnake.push_back(std::move(newSegment));
		//InitFruit();
		++mScore;
		if (mScore > mHiScore)
		{
			mHiScore = mScore;
		}
	}
}

void Game::Reset()
{
	mSnake.clear();
	Segment head{ (SCREEN_WIDTH - CELL_SIZE) / 2, (SCREEN_HEIGHT - CELL_SIZE) / 2, 0, 0 };
	mSnake.push_back(std::move(head));
}