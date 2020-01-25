#include "Game.h"
#include <iostream>




int main()
{
	Game::initialize_game(4);
	Game::start_game();
	Game::clear();
	int i;
	std::cin >> i;
	return 0;
}