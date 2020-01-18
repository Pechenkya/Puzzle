#pragma once
#include <vector>
#include <deque>
#include <array>
#include <list>
#include <functional>
#include <condition_variable>
#include <SFML/Window/Event.hpp>
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
	struct UserInterface;

	struct Drawable
	{
	protected:
		sf::RectangleShape* rectangle;
		sf::Text* text;

		mutable std::mutex interaction_mutex;

		bool active;
	public:

		Drawable(const sf::Text& caption, const sf::Vector2f& _size, const sf::Vector2f& pos, const sf::Vector2f& text_pos);

		const sf::Vector2f size;
		const sf::Vector2f position;
		const sf::Vector2f text_position;

		virtual bool is_active() const;
		virtual bool contains(const float& pos_x, const float& pos_y) const;

		virtual void draw(sf::RenderWindow& window) const;
	};

	struct Clickable : Drawable
	{
	protected:

		virtual std::function<void()> SELECT_ADDITION() = 0; // function itself executes before selection, returned function executes at selection end
		const std::function<void(Clickable*)> ON_CLICK;
	public:
		Clickable(const sf::Text& caption, const sf::Vector2f& _size, const sf::Vector2f& pos, const sf::Vector2f& text_pos, const std::function<void(Clickable*)> f);

		virtual void select();

		void click();

		virtual void set_default_style() = 0;

		void deactivate();
		void activate();
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

		// Default clickable object style (can be overriden in derived classes)
		static const sf::Color _NODE_COLOR;
		static const sf::Color _OUTLINE_COLOR;
		static float _OUTLINE_THICKNESS;
		//

		void set_default_style() override;

		ClickableStyleNode(const sf::Text& caption, const sf::Vector2f& size, const sf::Vector2f& pos, const sf::Vector2f& text_pos, const std::function<void(Clickable*)> f);

	protected:
		virtual std::function<void()> SELECT_ADDITION() = 0; // function itself executes before selection, returned function executes at selection end

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
		std::function<void()> SELECT_ADDITION() override;
		void try_move();

	public:
		size_t value;
		const size_t i, j;

		Node(size_t _i, size_t _j, float node_size, const sf::Vector2f& pos);
		~Node();


		static void initialize_nodes(std::list<Clickable*>* UI);
		static void reset_nodes();
		static void set_adjasents();

		// Polymorphic style getters
		static const sf::Color& AVAILABILITY_COLOR();
		//

		// Default Node object style (can be overriden in derived classes)
		static const sf::Color _AVAILABILITY_COLOR;
		//
	};

	struct Button : public ClickableStyleNode<Button>
	{
	public:
		Button(float button_size_x, float button_size_y, const sf::Vector2f& pos, std::string lable, const std::function<void(Clickable*)> f);
		~Button();

		void set_pressed();

		static void initialize_buttons(std::list<Clickable*>* UI);
		static void deactivate_buttons();
		static void activate_buttons();

		// Default Button object style (can be overriden in derived classes)
		static const sf::Color _OUTLINE_COLOR;
		static float _OUTLINE_THICKNESS;
		//

		static float game_outline_thickness;
		static float menu_outline_thickness;

	protected:
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
		Clickable* match(float x, float y) const;
		PositionTree(const std::list<Clickable*>* UI);
	private:
		std::vector<std::pair<float, std::vector<const Clickable*>>> tree;
		int get_index_x(float pos, int a, int b) const;
		Clickable* get_index_y(float pos, int a, int b, const std::vector<const Clickable*>& x_vec, float x_pos) const;

	};

	struct UserInterface
	{
		std::list <Clickable*>* UI;
		PositionTree* pos_tree;

		UserInterface(std::list <Clickable*>* _UI);
		~UserInterface();

		void deactivate();
		void activate();
		void draw(sf::RenderWindow& window) const;
	};

	struct Lable
	{
		bool active;
		sf::Text* text;

		Lable();
		Lable(const sf::Vector2f& pos, const std::string& caption);
		void set_caption(const std::string& caption);
		void draw(sf::RenderWindow& window) const;

	};

	static bool clickable_comp_x(const Clickable* a, const Clickable* b);
	static bool clickable_comp_y(const Clickable* a, const Clickable* b);

public:
	static void initialize_game(size_t sl = 4);
	static bool start_game();
	static void clear(); //TODO
	
	static void solve_game();

	//Computer control
	
	static void move(size_t i, size_t j);

private:
	static UserInterface* active_UI;

	//Main menu
	static void initialize_menu();

	static UserInterface* menu_UI;
	//


	//Base game propeties
	static const float window_width;
	static const float window_height;
	static size_t side_length;

	static Lable* last_score;
	static int move_counter;
	//

	static Node*** table; // I tut ya zvezdanulsya
	static UserInterface* game_UI;

	//Drawing thread resources
	static sf::Font FONT;
	static sf::RenderWindow* window;
	static std::mutex window_mutex;
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

	//Game UI
	static std::list<Button*>* buttons;
	static bool check_buttons(const sf::Event& event);

	static void restart_threads();
	static void reset();
	//

	template<typename T, typename ...ArgsT>
	static T* create_clickable(std::list<Clickable*>* UI_list, ArgsT&&... args);

	//Solver help methods
	static int** represent_to_int();
	//
};

//Zabiv
template<typename T, typename ...ArgsT>
T* Game::create_clickable(std::list<Clickable*>* UI_list, ArgsT&& ...args)
{
	Clickable* new_obj = new T(std::forward<ArgsT>(args)...);
	UI_list->push_back(new_obj);
	return static_cast<T*>(new_obj);
}