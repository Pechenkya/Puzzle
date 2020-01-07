#pragma once
#include "Heap.h"

namespace std
{
	template<
		class T1,
		class T2
	> struct pair;
}

class Solver
{
	template<typename T>
	struct NeighborList
	{
		struct ListItr 
		{
			ListItr(T** obj);
		private:
			T** current;
		public:
			ListItr& operator++();
			ListItr& operator++(int);
			T& operator*();
			bool operator==(const ListItr& obj);
		};
		NeighborList();
		NeighborList(NeighborList && obj);
		~NeighborList();
	private:
		int size { 0 };
		T* neighbour_list[4] = {nullptr};
	public:
		int get_size();
		void add_neighbour(T obj);
		ListItr begin();
		ListItr end();
	};

	struct Board
	{
	//private:
		int dimension;
		int** titles;
		int empty_node_x;
		int empty_node_y;
	//public:
		Board* prev_board;
		int moves_made;

		Board();
		Board(int** _titles, int _dimention);
		Board& operator=(const Board& obj);
		Board(const Board& obj);

		int get_dimension();
		int hamming() const;
		int manhattan() const;
		bool is_goal();
		bool equals(Board obj);
		std::vector<Board> neighbours();

		bool operator<(const Board& obj);

		std::pair<int, int> get_empty_pos();
	};

public:
	Solver(int** _titles, int _dimention);
	void solve();
	std::vector<std::pair<int, int>> solution();
	~Solver();

private:
	Heap<Board> priority_queue;
	Board result_board;
	bool solved = false;
	static void swap(int& a, int& b);
};


