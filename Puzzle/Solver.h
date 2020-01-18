#pragma once
#include "AdvancedDS.h"

namespace std
{
	template<
		class T1,
		class T2
	> struct pair;
}

class Solver
{
public:
	struct Board
	{
	private:
		short unsigned int dimension;
		int** titles;
		short unsigned int empty_node_x;
		short unsigned int empty_node_y;
		short int manhattan_score;
	public:
		const Board* prev_board;
		short unsigned int moves_made;

		~Board();
		Board();
		Board(int** _titles, short unsigned int _dimention);
		Board& operator=(const Board& obj);
		Board & operator=(Board && obj);
		Board(const Board& obj);
		Board(Board&& obj);

		int get_dimension() const;
		int hamming() const;
		int manhattan() const;
		bool is_goal() const;
		bool equals(Board obj) const;
		std::vector<Board*> neighbours() const;

		bool operator<(const Board& obj);

		std::pair<int, int> get_empty_pos() const;
	};

public:
	Solver(int** _titles, int _dimention);
	void solve();
	int moves_made();
	std::vector<std::pair<int, int>> solution();
	~Solver();

private:
	PQueue<Board> priority_queue;
	Board* result_board;
	bool solved = false;
	static void swap(int& a, int& b);
};


