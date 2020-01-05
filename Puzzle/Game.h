#pragma once
#include <vector>
#include <deque>
#include <array>
#include <condition_variable>
#include <SFML/Window/Event.hpp>

//Predeclaration
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
//

class Game
{
	struct Node
	{
	private:
		struct Animation
		{
			static const float one_step_t;

			Animation(std::string expr_x, std::string expr_y, float t1, float t2);
			std::pair<sf::Vector2f, float>* movement_table;
			const size_t step_count;
		};

		sf::RectangleShape* rectangle;
		sf::Text* text;
		mutable std::mutex interaction_mutex;

		void play_animation(const Animation& animation);
		Node* swap_with_empty();
		bool contains(const float& pos_x, const float& pos_y) const;

		// Animations
		static const Animation* animations[3][3];
		//
	public:
		int value;
		const sf::Vector2f position;
		const sf::Vector2f text_position;
		const size_t i, j;

		Node(size_t _i, size_t _j, float node_size, sf::Vector2f pos);
		~Node();

		void try_move();
		void select();
		void draw(sf::RenderWindow& window) const;

		void set_default_outline();

		static void initialize_nodes();
		static void reset_nodes();
	};

	struct EventQueue
	{
		sf::Event pop();
		void push(const sf::Event& event);

		void disable();
		void enable();
	private: 
		bool enabled = true;
		std::deque <sf::Event> events;
		std::condition_variable qcv;
		std::mutex qm;
		std::mutex interaction_mutex;
		std::unique_lock<std::mutex> ql{ qm, std::defer_lock };
	};

	struct AdjacentSet
	{
		struct AdjItr
		{
			inline AdjItr(AdjacentSet* c, size_t _i);

			inline AdjItr& operator++();
			inline bool operator!=(const AdjItr& a);
			inline Node* operator*();
		private:
			size_t i;
			AdjacentSet* container;
		};

		void add(Node* n);
		void clear();

		AdjItr begin();
		AdjItr end();

	private:
		size_t index{ 0 };
		std::array<Node*, 4> arr;
	};

	struct PositionTree
	{
		Node* match(float x, float y);
		PositionTree();
		~PositionTree();
	private:
		std::pair<float, float>* node_bounds_x;
		std::pair<float, float>* node_bounds_y;
		int get_index(std::pair<float, float>* bounds_array, float pos, int a = 0, int b = side_length - 1);
	};

	struct Button
	{
	private:
		sf::RectangleShape* rectangle;
		sf::Text* text;
		mutable std::mutex interaction_mutex;
		bool pressed = false;
	public:
		bool visible = true;

		std::mutex press_mutex;
		std::unique_lock<std::mutex> press_lock{ press_mutex, std::defer_lock };
		std::condition_variable press_condition;

		Button(float button_size, sf::Vector2f pos, std::string lable);

		const sf::Vector2f position;
		const sf::Vector2f text_position;
		void draw(sf::RenderWindow& window) const;

		void select();
		bool contains(const float& pos_x, const float& pos_y) const;
		void set_pressed();
		void set_unpressed();
		bool is_pressed();

	
	};

public:
	static void initialize_game(size_t sl = 4);
	static bool start_game();

	
	static void solve_game();

	//Computer control
	
	static void move(size_t i, size_t j);
	// TODO get_table()
	// TODO bool solved() ?
	//

private:
	//Base game propeties
	static const float window_width;
	static const float window_height;
	static size_t side_length;

	static int score_counter;
	//

	static Node*** table; // I tut ya zvezdanulsya

	//Drawing thread resources
	static sf::Font FONT;
	static sf::RenderWindow* window;
	static PositionTree* pos_tree;
	static EventQueue* mouse_move_events;
	static EventQueue* mouse_click_events;
	static AdjacentSet* empty_adjacent;

	static void draw_process(); //separate thread
	//

	//Node
	static void update_empty_adj();
	static Game::AdjacentSet& get_adjacent();
	static Node* empty_node;
	//

	//Puzzle movement
	static void click_process(); //separate thread
	//

	//UI
	static void initialize_buttons();
	static Button* ready_button;
	static Button* reset_button;
	static Button* buttons[2];

	static bool check_buttons(const sf::Event& event);
	static void set_buttons_invisible();

	static void reset();
	//
};

