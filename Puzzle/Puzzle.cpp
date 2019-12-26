#include "Game.h"
#include <iostream>

std::mutex m;
std::mutex l;

void thread1()
{
	l.lock();
	for(int i = 0; i < 100; ++i)
	{
		std::cout << "1" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	l.unlock();
}

void thread2()
{

	l.lock();
	for (int i = 0; i < 100; ++i)
	{
		std::cout << "2" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	l.unlock();
}


int main()
{
	Game::initialize_game();
	Game::start_game();
	//l.lock();
	//l.unlock();
	//l.lock();
	//l.unlock();
	//std::thread t1(thread1);
	//std::thread t2(thread2);

	//t1.join();
	//t2.join();
	return 0;
}