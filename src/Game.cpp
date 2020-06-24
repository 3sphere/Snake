#include "Game.h"
#include "Constants.h"
#include <random>
#include <iostream>
#include <algorithm>

Game::Game() : mWindow(nullptr), mRenderer(nullptr), mIsRunning(true), mState(GAME_ACTIVE), mScore(0), mHiScore(0), mTicksCount(0), mMoveTimer(TIME_PER_CELL), mResetTimer(4.0f) {}

bool Game::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Failed to initialise SDL. Error: %s\n", SDL_GetError());
		return false;
	}

	mSoundManager.Init();

	mTextGenerator.Init();

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

	mSoundManager.LoadSoundFromFile(SOUND_ID::EAT_FRUIT, "Sounds/eat_fruit.wav");
	mSoundManager.LoadSoundFromFile(SOUND_ID::GAME_OVER, "Sounds/game_over.wav");

	mTextGenerator.LoadFont("Fonts/data-latin.ttf", 20);

	mSnake.reserve(GRID_WIDTH * GRID_HEIGHT);
	Segment head{ GRID_WIDTH / 2, GRID_HEIGHT / 2, 0, 0 };
	mSnake.push_back(std::move(head));
	
	InitFruit();

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
	if (state[SDL_SCANCODE_SPACE])
	{
		mState = GAME_ACTIVE;
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

	if (mState == GAME_ACTIVE)
	{
		mMoveTimer -= deltaTime;
		if (mMoveTimer < 0.0f)
		{
			for (Segment& segment : mSnake)
			{
				segment.pos.x += segment.direction.x;
				segment.pos.y += segment.direction.y;
			}
			//Update the snake's segments (excluding the head)
			for (std::vector<Segment>::reverse_iterator iter = mSnake.rbegin(); iter < mSnake.rend() - 1; ++iter)
			{
				iter->direction = (iter + 1)->direction;
			}

			mSnake[0].direction = mNewDirection;
			
			CheckCollisions();

			mMoveTimer = TIME_PER_CELL;
		}
	}
	
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

	//render fruit
	SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
	SDL_Rect fruitRect{
		mFruitPos.x * CELL_SIZE,
		mFruitPos.y * CELL_SIZE,
		CELL_SIZE,
		CELL_SIZE
	};
	SDL_RenderFillRect(mRenderer, &fruitRect);

	if (mState == GAME_OVER)
	{
		std::string restartText = "Press space to restart";
		mTextGenerator.Render(mRenderer, std::move(restartText), 0.32 * SCREEN_WIDTH, 0.4 * SCREEN_WIDTH);
	}

	//render snake
	SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
	for (Segment& segment : mSnake)
	{
		//Linearly interpolate between each segment's current grid position and next grid position to give smooth grid motion
		float screenPosX = (static_cast<float>(segment.pos.x) + ((TIME_PER_CELL - mMoveTimer) / TIME_PER_CELL) * static_cast<float>(segment.direction.x)) * static_cast<float>(CELL_SIZE);
		float screenPosY = (static_cast<float>(segment.pos.y) + ((TIME_PER_CELL - mMoveTimer) / TIME_PER_CELL) * static_cast<float>(segment.direction.y)) * static_cast<float>(CELL_SIZE);
		SDL_Rect rect{
			static_cast<int>(screenPosX),
			static_cast<int>(screenPosY),
			CELL_SIZE,
			CELL_SIZE
			};
		SDL_RenderFillRect(mRenderer, &rect);
	}

	//This removes the jagged edges that are seen when a snake with more than one segment changes direction
	if (mSnake.size() > 1)
	{
		SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
		SDL_Rect rect{ 0,0,CELL_SIZE,CELL_SIZE };
		for (decltype(mSnake.size()) i = 0; i < mSnake.size() - 1; ++i)
		{
			rect.x = mSnake[i].pos.x * CELL_SIZE;
			rect.y = mSnake[i].pos.y * CELL_SIZE;
			SDL_RenderFillRect(mRenderer, &rect);
		}
	}

	std::string scoreText = "Score: " + std::to_string(mScore);
	mTextGenerator.Render(mRenderer, scoreText, 0, 0);
	scoreText.clear();
	scoreText = " High score: " + std::to_string(mHiScore);
	mTextGenerator.Render(mRenderer, scoreText, 0.75 * SCREEN_WIDTH, 0);

	//Swap front and back buffers
	SDL_RenderPresent(mRenderer);
}

void Game::CheckCollisions()
{
	//Check if snake head has collided with a wall, its tail, or the fruit
	//wall
	Segment head = mSnake[0];
	if (head.pos.x < 0 || head.pos.x > GRID_WIDTH - 1 || head.pos.y < 0 || head.pos.y > GRID_HEIGHT - 1)
	{
		mSoundManager.PlaySound(SOUND_ID::GAME_OVER);
		mState = GAME_OVER;
		Reset();
		return;
	}

	//tail
	for (std::vector<Segment>::size_type i = 2; i < mSnake.size(); ++i)
	{
		if (head.pos.x == mSnake[i].pos.x && head.pos.y == mSnake[i].pos.y)
		{
			mSoundManager.PlaySound(SOUND_ID::GAME_OVER);
			mState = GAME_OVER;
			Reset();
			return;
		}
	}

	// fruit
	if (head.pos.x == mFruitPos.x && head.pos.y == mFruitPos.y)
	{
		mSoundManager.PlaySound(SOUND_ID::EAT_FRUIT);
		Segment newSegment{ mSnake.back().pos.x, mSnake.back().pos.y, 0, 0 };
		mSnake.push_back(std::move(newSegment));
		InitFruit();
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
	Segment head{ GRID_WIDTH / 2, GRID_HEIGHT / 2, 0, 0 };
	mSnake.push_back(std::move(head));
	mScore = 0;
	mMoveTimer = TIME_PER_CELL;
}

void Game::InitFruit()
{
	// Initialise the fruit's position to a random location, making sure it doesn't spawn on top of
	// the snake
	bool validPos = true;
	std::uniform_int_distribution<unsigned> uX(0, GRID_WIDTH-1);
	std::uniform_int_distribution<unsigned> uY(0, GRID_HEIGHT-1);
	std::default_random_engine e(SDL_GetTicks());
	do
	{
		validPos = true;
		int newX = uX(e);
		int newY = uY(e);
		auto found = std::find_if(mSnake.begin(), mSnake.end(), [newX, newY](Segment& s) {return s.pos.x == newX && s.pos.y == newY; });
		if (found != mSnake.end())
		{
			validPos = false;
		}
		else
		{
			mFruitPos.x = newX;
			mFruitPos.y = newY;
		}
	} while (!validPos);
}