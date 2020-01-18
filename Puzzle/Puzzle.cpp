#include "Game.h"
#include <iostream>



int main()
{
	while (true)
	{
		Game::initialize_game(4);
		Game::start_game();
		Game::clear();
	}
	return 0;
}