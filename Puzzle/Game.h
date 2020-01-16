#pragma once
#include <vector>
#include <deque>
#include <array>
#include <list>
#include <functional>
#include <condition_variable>
#include <SFML/Window/Event.hpp>
//#include <SFML/Graphics/Color.hpp>
#include "Solver.h"

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
	class Color;
	template<
		class T1,
		class T2
	> struct pair;


}
//

class Game
{
	struct Clickable
	{
	protected:
		sf::RectangleShape* rectangle;
		sf::Text* text;

		mutable std::mutex interaction_mutex;

		bool active;

		virtual std::function<void()> SELECT_ADDITION() = 0; // function itself executes before selection, returned function executes at selection end
	public:
		Clickable(const sf::Text& caption, const sf::Vector2f& size, const sf::Vector2f& pos, const sf::Vector2f& text_pos);

		const sf::Vector2f size;
		const sf::Vector2f position;
		const sf::Vector2f text_position;

		virtual bool is_active() const;
		virtual bool contains(const float& pos_x, const float& pos_y) const;
		virtual void select();
		virtual void draw(sf::RenderWindow& window) const;
	};

	template<typename T>
	struct ClickableStyleNode : public Clickable
	{
	public:
		// Polymorphic style getters
		static const sf::Color& NODE_COLOR();
		static const sf::Color& OUTLINE_COLOR();
		static float OUTLINE_THICKNESS();
		//

		void set_default_outline();

		ClickableStyleNode(const sf::Text& caption, const sf::Vector2f& size, const sf::Vector2f& pos, const sf::Vector2f& text_pos);

	protected:

		// Default clickable object style (can be overriden in derived classes)
		static const sf::Color _NODE_COLOR;
		static const sf::Color _OUTLINE_COLOR;
		static float _OUTLINE_THICKNESS;
		//
	};

	struct Node : public ClickableStyleNode<Node>
	{
	private:
		struct Animation
		{
			static const float one_step_t;

			Animation(std::string expr_x, std::string expr_y, float t1, float t2);
			std::pair<sf::Vector2f, float>* movement_table;
			const size_t step_count;
		};

		void play_animation(const Animation& animation);
		Node* swap_with_empty();
		
		// Animations
		static const Animation* animations[3][3];
		//
	protected:
		// Default Node object style (can be overriden in derived classes)
		static const sf::Color _AVAILABILITY_COLOR;
		//

		std::function<void()> SELECT_ADDITION() override;

	public:
		int value;
		const size_t i, j;

		Node(size_t _i, size_t _j, float node_size, const sf::Vector2f& pos);
		~Node();

		void try_move();

		static void initialize_nodes();
		static void reset_nodes();

		// Polymorphic style getters
		static const sf::Color& AVAILABILITY_COLOR();
		//
	};

	struct Button : public ClickableStyleNode<Button>
	{
	private:
		bool pressed = false;
	public:
		bool visible = true;
		std::mutex press_mutex;
		std::unique_lock<std::mutex> press_lock{ press_mutex, std::defer_lock };
		std::condition_variable press_condition;

		Button(float button_size_x, float button_size_y, sf::Vector2f pos, std::string lable);
		void draw(sf::RenderWindow& window) const override;
		void set_pressed();
		void set_released();
		bool is_pressed();


		static void initialize_buttons();
	protected:
		// Default Button object style (can be overriden in derived classes)
		static const sf::Color _OUTLINE_COLOR;
		static float _OUTLINE_THICKNESS;
		//

		std::function<void()> SELECT_ADDITION() override;
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
		Clickable* match(float x, float y);
		PositionTree();
		~PositionTree();
	private:
		std::vector<std::pair<float, std::vector<const Clickable*>>> tree;
		int get_index_x(float pos, int a, int b);
		Clickable* get_index_y(float pos, int a, int b, std::vector<const Clickable*>& x_vec);

	};

	bool clickable_comp_x(const Game::Clickable* a, const Game::Clickable* b);
	bool clickable_comp_y(const Game::Clickable* a, const Game::Clickable* b);



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
	static std::list<Clickable*>* UI;

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
	static Button* ready_button;
	static Button* reset_button;
	static Button* buttons[2];

	template<typename T, typename ...ArgsT>
	static T* create_clickable(ArgsT&&... args);

	static bool check_buttons(const sf::Event& event);
	static void set_buttons_invisible();

	static void reset();
	//

	//Solver help methods
	static int** represent_to_int();
	//
};

//Zabiv
template<typename T, typename ...ArgsT>
T* Game::create_clickable(ArgsT&& ...args)
{
	Clickable* new_obj = new T(std::forward<ArgsT>(args)...);
	UI.push_back(new_obj);
	return static_cast<T*>(new_obj);
}
