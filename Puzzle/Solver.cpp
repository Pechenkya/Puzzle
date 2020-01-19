#include "Solver.h"
#include <math.h>
#include <utility>


Solver::Board::~Board()
{

	if (titles)
	{
		for (int i = 0; i < dimension; i++)
			delete[] titles[i];
		delete[] titles;
	}

}

//Board
Solver::Board::Board()
{
	titles = nullptr;
	prev_board = nullptr;
};

Solver::Board::Board(int ** _titles, short unsigned int _dimention)
	: titles{ _titles }, dimension{ _dimention }
{
	prev_board = nullptr;
	moves_made = 0;
	for (int k = 0; k < dimension * dimension; k++)
		if (titles[k % dimension][k / dimension] == dimension * dimension)
		{
			empty_node_x = k % dimension;
			empty_node_y = k / dimension;
			break;
		}

	manhattan_score = this->manhattan();		
};

Solver::Board& Solver::Board::operator=(const Board & obj)
{
	this->dimension = obj.dimension;
	titles = new int*[dimension];
	for (int i = 0; i < dimension; i++)
		titles[i] = new int[dimension];

	for (int i = 0; i < dimension; i++)
		for (int j = 0; j < dimension; j++)
			this->titles[i][j] = obj.titles[i][j];

	this->prev_board = obj.prev_board;

	this->moves_made = obj.moves_made;
	this->empty_node_x = obj.empty_node_x;
	this->empty_node_y = obj.empty_node_y;
	this->manhattan_score = obj.manhattan_score;
	return *this;
}

Solver::Board& Solver::Board::operator=(Board && obj)
{
	this->dimension = obj.dimension;
	titles = obj.titles;
	obj.titles = nullptr;
	this->prev_board = obj.prev_board;
	obj.prev_board = nullptr;
	this->moves_made = obj.moves_made;
	this->empty_node_x = obj.empty_node_x;
	this->empty_node_y = obj.empty_node_y;
	this->manhattan_score = obj.manhattan_score;

	return *this;
}

Solver::Board::Board(const Board & obj)
{
	this->dimension = obj.dimension;
	titles = new int*[dimension];
	for (int i = 0; i < dimension; i++)
		titles[i] = new int[dimension];

	for (int i = 0; i < dimension; i++)
		for (int j = 0; j < dimension; j++)
			this->titles[i][j] = obj.titles[i][j];

	this->prev_board = obj.prev_board;
	

	this->moves_made = obj.moves_made;
	this->empty_node_x = obj.empty_node_x;
	this->empty_node_y = obj.empty_node_y;
	this->manhattan_score = obj.manhattan_score;
}

Solver::Board::Board(Board && obj)
{
	this->dimension = obj.dimension;
	titles = obj.titles;
	obj.titles = nullptr;
	this->prev_board = obj.prev_board;
	obj.prev_board = nullptr;
	this->moves_made = obj.moves_made;
	this->empty_node_x = obj.empty_node_x;
	this->empty_node_y = obj.empty_node_y;
	this->manhattan_score = obj.manhattan_score;

}

int Solver::Board::get_dimension() const
{
	return this->dimension;
}

int Solver::Board::hamming() const
{
	int diff{ 0 };
	for (int j = 0; j < dimension; j++)
		for (int i = 0; i < dimension; i++)
			if (titles[i][j] != j * dimension + i + 1)
				diff++;
	return diff;
}

int Solver::Board::manhattan() const
{
	int diff{ 0 };
	for (int j = 0; j < dimension; j++)
		for (int i = 0; i < dimension; i++)
			if (titles[i][j] != j * dimension + i + 1)
			{
				diff += abs((titles[i][j] - 1) % dimension - i);
				diff += abs((titles[i][j] - 1) / dimension - j);
			}

	diff -= abs((titles[empty_node_x][empty_node_y] - 1) % dimension - empty_node_x);
	diff -= abs((titles[empty_node_x][empty_node_y] - 1) / dimension - empty_node_y);

	return dimension / 2 * diff; // 2? - could be dimension / 2
}

bool Solver::Board::is_goal() const
{
	for (int k = 0; k < dimension * dimension; k++)
		if (titles[k % dimension][k / dimension] != k + 1)
			return false;

	return true;
}

bool Solver::Board::equals(Board obj) const
{
	for (int k = 0; k < dimension * dimension; k++)
		if (this->titles[k % dimension][k / dimension] != obj.titles[k % dimension][k / dimension])
			return false;

	return true;
}

std::vector<Solver::Board*> Solver::Board::neighbours() const 
{
	std::vector<Board*> neighbour_list;
	
	int i = this->empty_node_x;
	int j = this->empty_node_y;

	if (i < dimension - 1 && !(prev_board && i + 1 == prev_board->empty_node_x))
	{
		neighbour_list.emplace_back(new Board(*this));
		swap(neighbour_list.back()->titles[i][j], neighbour_list.back()->titles[i + 1][j]);
		neighbour_list.back()->empty_node_x = i + 1;
		neighbour_list.back()->moves_made++;
		neighbour_list.back()->prev_board = this;
		neighbour_list.back()->manhattan_score = neighbour_list.back()->manhattan() + neighbour_list.back()->moves_made;
	}
	if (i > 0 && !(prev_board && i - 1 == prev_board->empty_node_x))
	{
		neighbour_list.emplace_back(new Board(*this));
		swap(neighbour_list.back()->titles[i][j], neighbour_list.back()->titles[i - 1][j]);
		neighbour_list.back()->empty_node_x = i - 1;
		neighbour_list.back()->moves_made++;
		neighbour_list.back()->prev_board = this;
		neighbour_list.back()->manhattan_score = neighbour_list.back()->manhattan() + neighbour_list.back()->moves_made;
	}
	if (j < dimension - 1 && !(prev_board && j + 1 == prev_board->empty_node_y))
	{
		neighbour_list.emplace_back(new Board(*this));
		swap(neighbour_list.back()->titles[i][j], neighbour_list.back()->titles[i][j + 1]);
		neighbour_list.back()->empty_node_y = j + 1;
		neighbour_list.back()->moves_made++;
		neighbour_list.back()->prev_board = this;
		neighbour_list.back()->manhattan_score = neighbour_list.back()->manhattan() + neighbour_list.back()->moves_made;
	}
	if (j > 0 && !(prev_board && j - 1 == prev_board->empty_node_y))
	{
		neighbour_list.emplace_back(new Board(*this));
		swap(neighbour_list.back()->titles[i][j], neighbour_list.back()->titles[i][j - 1]);
		neighbour_list.back()->empty_node_y = j - 1;
		neighbour_list.back()->moves_made++;
		neighbour_list.back()->prev_board = this;
		neighbour_list.back()->manhattan_score = neighbour_list.back()->manhattan() + neighbour_list.back()->moves_made;
	}

	return neighbour_list;
}

bool Solver::Board::operator<(const Board & obj)
{
	return this->manhattan_score < obj.manhattan_score;
}

std::pair<int, int> Solver::Board::get_empty_pos() const
{
	return std::make_pair(empty_node_x, empty_node_y);
}
//


//Solver
Solver::Solver(int ** _titles, int _dimention)
{
	Board initial_board(_titles, _dimention);
	int** titles = new int*[_dimention];
	priority_queue.insert(initial_board);
}

void Solver::solve()
{ 
	if ((*priority_queue.begin()).is_goal())
	{
		solved = true;
		result_board = priority_queue.remove_min();
	}
	
	while(!solved)
	{
		Board* temp = priority_queue.remove_min();
		std::vector<Board*> vec = temp->neighbours();
		for (Board* t : vec)
		{
			if (t->is_goal())
			{
				result_board = t;
				solved = true;
				break;
			}
			priority_queue.insert(t);
		}

	}
}

int Solver::moves_made()
{
	return result_board->moves_made;
}

std::vector<std::pair<int, int>> Solver::solution()
{
	std::vector<std::pair<int, int>> solution;
	const Board* curr = result_board;
	while (curr)
	{
		solution.push_back(curr->get_empty_pos());
		curr = curr->prev_board;
	}
	solution.pop_back();
	std::reverse(solution.begin(), solution.end());
	return solution;
}

Solver::~Solver()
{
	if(result_board->moves_made != 0)
		delete result_board;
}

void Solver::swap(int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}
