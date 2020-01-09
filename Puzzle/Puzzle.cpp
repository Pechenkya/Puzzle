#include "Game.h"
#include <iostream>



int main()
{
	std::cout << sizeof(Solver::Board);
	Game::initialize_game(3);
	Game::start_game();
	return 0;
}