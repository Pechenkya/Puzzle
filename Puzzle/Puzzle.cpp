#include "Game.h"
#include <iostream>



int main()
{
	std::cout << sizeof(Solver::Board);
	Game::initialize_game();
	Game::start_game();
	return 0;
}