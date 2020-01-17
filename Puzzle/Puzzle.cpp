#include "Game.h"
#include <iostream>



int main()
{
	while (true)
	{
		Game::initialize_game();
		Game::start_game();
		Game::clear();
	}
	return 0;
}