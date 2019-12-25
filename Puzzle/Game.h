#pragma once


namespace std
{
	template< class T >
	struct allocator;

	template<
		class T,
		class Allocator = allocator<T>
	> class vector;

	template<
		class T,
		class Allocator = std::allocator<T>
	> class deque;

	class mutex;
}

namespace sf
{
	class RectangleShape;
	class Vector2f;
	class Window;
	class Text;
	class Event;
}

class Game
{
	struct Node
	{
	private:
		sf::RectangleShape* rectangle;
		sf::Text* text;
	public:
		int value;
		const int i, j;
		Node(int _i, int _j) : i{ _i }, j{ _j } {};

		bool contains(const sf::Vector2f& pos);
		void draw(const sf::Window& window);
	};

	struct EventQueue
	{
		sf::Event pop();
		void push(const sf::Event& event);

	private: 
		std::deque <sf::Event>* events;
	};

public:
	static void initialize_game(unsigned int sl);
	static bool start_game();

private:
	//Base game propeties
	static unsigned int side_length;
	static Node** table;
	//

	//Drawing thread values
	static sf::Window* window; // Could be redundant
	static Node* selected_node;
	static EventQueue mouse_move_events;
	static void draw_process();
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

