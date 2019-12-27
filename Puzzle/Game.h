#pragma once
#include <vector>
#include <deque>
#include <condition_variable>
#include <SFML/Window/Event.hpp>

namespace std
{
	class mutex;
}

namespace sf
{
	class RectangleShape;
	template<typename T>
	class Vector2;
	typedef Vector2<float> Vector2f;
	class RenderWindow;
	class Text;
	class Font;
	template<
		class T1,
		class T2
	> struct pair;
}

class Game
{
	struct Node
	{
		struct Animation
		{

		};

	private:
		sf::RectangleShape* rectangle;
		sf::Text* text;
	public:
		int value;
		const sf::Vector2f position;
		const size_t i, j;
		Node(size_t _i, size_t _j, float node_size, sf::Vector2f pos);
		~Node();

		void move_animation(sf::Vector2f pos);
		void swap(Node* node);
		void set_selected();
		void remove_outline();
		bool contains(const sf::Vector2f& pos) const;
		void draw(sf::RenderWindow& window) const;
	};

	struct EventQueue
	{
		sf::Event pop();
		void push(const sf::Event& event);

	private: 
		std::deque <sf::Event> events;
		std::condition_variable qcv;
		std::mutex qm;
		std::mutex qm2;
		std::unique_lock<std::mutex> ql{ qm, std::defer_lock };
	};

	struct PositionTree
	{
		Node* match(float x, float y);
		PositionTree();
		~PositionTree();
	private:
		std::pair<float, float>* node_bounds_x;
		std::pair<float, float>* node_bounds_y;
		int get_index(std::pair<float, float>* bounds_array, float pos, size_t a = 0, size_t b = side_length - 1);
	};

public:
	static void initialize_game(size_t sl = 4);
	static bool start_game();
	


private:
	//Base game propeties
	static const float window_width;
	static const float window_height;
	static size_t side_length;
	static Node*** table; // I tut ya zvezdanulsya
	//

	// TEST
	static long long int check_counter;
	//

	//Drawing thread values
	static sf::Font FONT;
	static sf::RenderWindow* window;
	static Node* selected_node;
	static void draw_process();
	static PositionTree* pos_tree;
	static EventQueue* mouse_move_events;
	static EventQueue* mouse_click_events;
	//

	//Node
	static std::vector<Node*> get_adjacent(const Node& this_node);
	static Node* empty_node;
	//

	//Puzzle movement
	static std::mutex interaction_mutex;
	static void click_process();
	//
};

