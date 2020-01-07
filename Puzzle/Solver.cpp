#include "Solver.h"
#include <math.h>
#include <utility>

//List Iterator
template<typename T>
Solver::NeighborList<T>::ListItr::ListItr(T ** obj)
{
	current = obj;
};

template<typename T>
typename Solver::NeighborList<T>::ListItr& Solver::NeighborList<T>::ListItr::operator++()
{
	current = current + 1;
	return *this;
};

template<typename T>
typename Solver::NeighborList<T>::ListItr& Solver::NeighborList<T>::ListItr::operator++(int)
{
	current = current + 1;
	return *this;
};

template<typename T>
T& Solver::NeighborList<T>::ListItr::operator*()
{
	return **current;
}

template<typename T>
bool Solver::NeighborList<T>::ListItr::operator==(const ListItr & obj)
{
	return this->current == obj.current;
}
//



//Neighbour List
template<typename T>
Solver::NeighborList<T>::NeighborList()
{
	size = 0;
};

template<typename T>
Solver::NeighborList<T>::NeighborList(NeighborList && obj)
{
	this->size = obj.size;
	for (int i = 0; i < size; i++)
	{
		this->neighbour_list[i] = obj.neighbour_list[i];
		obj.neighbour_list[i] = nullptr;
	}
}

template<typename T>
Solver::NeighborList<T>::~NeighborList()
{
	for (int i = 0; i < 4; i++)
		delete neighbour_list[i];
}

template<typename T>
int Solver::NeighborList<T>::get_size()
{
	return this->size;
}

template<typename T>
void Solver::NeighborList<T>::add_neighbour(T obj)
{
	this->neighbour_list[size++] = new T(obj);
}

template<typename T>
typename Solver::NeighborList<T>::ListItr Solver::NeighborList<T>::begin()
{
	return ListItr(&neighbour_list[0]);
}

template<typename T>
typename Solver::NeighborList<T>::ListItr Solver::NeighborList<T>::end()
{
	return ListItr(&neighbour_list + size);
}
//

Solver::Board::Board(){};

//Board
Solver::Board::Board(int ** _titles, int _dimention)
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
}

int Solver::Board::get_dimension()
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
	return diff;
}

bool Solver::Board::is_goal()
{
	for (int k = 0; k < dimension * dimension; k++)
		if (titles[k % dimension][k / dimension] != k + 1)
			return false;

	return true;
}

bool Solver::Board::equals(Board obj)
{
	for (int k = 0; k < dimension * dimension; k++)
		if (this->titles[k % dimension][k / dimension] != obj.titles[k % dimension][k / dimension])
			return false;

	return true;
}

std::vector<Solver::Board> Solver::Board::neighbours()
{
	std::vector<Board> neighbour_list;
	Board neighbour = *this;
	
	int i = this->empty_node_x;
	int j = this->empty_node_y;

	if (i < dimension - 1)
	{
		swap(neighbour.titles[i][j], neighbour.titles[i + 1][j]);
		neighbour.empty_node_x = i + 1;
		neighbour.moves_made++;
		neighbour.prev_board = new Board(*this);
		if(this->prev_board && !neighbour.equals(*this->prev_board))
			neighbour_list.push_back(neighbour);
		else if(!this->prev_board)
			neighbour_list.push_back(neighbour);

		neighbour = *this;
	}
	if (i > 0)
	{
		swap(neighbour.titles[i][j], neighbour.titles[i - 1][j]);
		neighbour.empty_node_x = i - 1;
		neighbour.moves_made++;
		neighbour.prev_board = new Board(*this);
		if (this->prev_board && !neighbour.equals(*this->prev_board))
			neighbour_list.push_back(neighbour);
		else if (!this->prev_board)
			neighbour_list.push_back(neighbour);

		neighbour = *this;
	}
	if (j < dimension - 1)
	{
		swap(neighbour.titles[i][j], neighbour.titles[i][j + 1]);
		neighbour.empty_node_y = j + 1;
		neighbour.moves_made++;
		neighbour.prev_board = new Board(*this);
		if (this->prev_board && !neighbour.equals(*this->prev_board))
			neighbour_list.push_back(neighbour);
		else if (!this->prev_board)
			neighbour_list.push_back(neighbour);

		neighbour = *this;
	}
	if (j > 0)
	{
		swap(neighbour.titles[i][j], neighbour.titles[i][j - 1]);
		neighbour.empty_node_y = j - 1;
		neighbour.moves_made++;
		neighbour.prev_board = new Board(*this);
		if (this->prev_board && !neighbour.equals(*this->prev_board))
			neighbour_list.push_back(neighbour);
		else if (!this->prev_board)
			neighbour_list.push_back(neighbour);

		neighbour = *this;
	}

	return std::move(neighbour_list);
}

bool Solver::Board::operator<(const Board & obj)
{
	return this->manhattan() + this->moves_made < obj.manhattan() + obj.moves_made;
}

std::pair<int, int> Solver::Board::get_empty_pos()
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
		Board temp = priority_queue.remove_min();
		std::vector<Board> vec = temp.neighbours();
		for (Board t : vec)
		{
			priority_queue.insert(t);
			if (t.is_goal())
			{
				result_board = t;
				solved = true;
			}
		}

	}
}

std::vector<std::pair<int, int>> Solver::solution()
{
	std::vector<std::pair<int, int>> solution;
	Board* curr = &result_board;
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
}

void Solver::swap(int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}
