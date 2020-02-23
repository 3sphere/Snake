#include <SDL.h>
#include "Game.h"

int main(int args, char** argc)
{
	Game game;

	bool success = game.Init();

	if (success)
	{
		game.Run();
	}

	game.Shutdown();

	return 0;
}