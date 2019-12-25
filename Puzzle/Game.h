#pragma once
#include <vector>
#include <deque>
#include <condition_variable>
#include <SFML/Window/Event.hpp>
//#include <mutex>

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
		Node(int _i, int _j, float node_size, sf::Vector2f pos);

		bool contains(const sf::Vector2f& pos);
		void draw(sf::RenderWindow& window);
	};

	struct EventQueue
	{
		sf::Event pop();
		void push(const sf::Event& event);

	private: 
		std::deque <sf::Event> events;
		std::condition_variable cv;
		std::mutex mudax;
		std::unique_lock<std::mutex> lock{ mudax };
	};

public:
	static void initialize_game(unsigned int sl = 4);
	static bool start_game();

private:
	//Base game propeties
	static const float window_width;
	static const float window_height;
	static unsigned int side_length;
	static Node*** table; // I tut ya zvezdanulsya
	//

	//Drawing thread values
	static sf::Font FONT;
	static sf::RenderWindow* window; // Could be redundant
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

