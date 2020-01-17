#include "Game.h"

#include "ExpressionParser.h"
//#include "Sort.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <cmath>
#include <thread>

#include <SFML/System/String.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#define BEGIN_INTERACTION(NODE_POINTER) NODE_POINTER->interaction_mutex.lock();

#define END_INTERACTION(NODE_POINTER) NODE_POINTER->interaction_mutex.unlock();

#define INTERACTION_RETURN(RETURN_TOKEN)    \
{											\
	this->interaction_mutex.unlock();		\
	return RETURN_TOKEN;					\
}                                           \

//Style parameters
const sf::Color Game::Node::_AVAILABILITY_COLOR = sf::Color(255, 173, 51);

template<typename T>
const sf::Color Game::ClickableStyleNode<T>::_OUTLINE_COLOR = sf::Color::White;

template<typename T>
const sf::Color Game::ClickableStyleNode<T>::_NODE_COLOR = sf::Color(211, 211, 211);

template<typename T>
float Game::ClickableStyleNode<T>::_OUTLINE_THICKNESS = 5.f;

const sf::Color Game::Button::_OUTLINE_COLOR = sf::Color(0, 211, 211);
float Game::Button::_OUTLINE_THICKNESS = 5.f;
sf::Font Game::FONT;
static const sf::Color BG_COLOR = sf::Color::Black;

const float cant_move_animation_time = 0.2f;
const float animation_time = 0.4f;
//

// Game static member
Game::EventQueue* Game::mouse_move_events = nullptr;
Game::EventQueue* Game::mouse_click_events = nullptr;
Game::PositionTree* Game::pos_tree = nullptr;
Game::AdjacentSet* Game::empty_adjacent = nullptr;

const float Game::Node::Animation::one_step_t = 0.008f;
const Game::Node::Animation* Game::Node::animations[3][3] = { nullptr };

int Game::score_counter = 0;
//

//Table parameters
Game::Node*** Game::table = nullptr;
size_t Game::side_length;
Game::Node* Game::empty_node = nullptr;
//

//Window parameters
const float Game::window_height = 720;
const float Game::window_width = 1280;
sf::RenderWindow* Game::window;
//

//UI
std::list<Game::Clickable*>* Game::UI = nullptr;
Game::Button* Game::ready_button = nullptr;
Game::Button* Game::reset_button = nullptr;
Game::Button* Game::buttons[2] = { nullptr };
//


bool Game::clickable_comp_x(const Clickable* a, const Clickable* b)
{
	if (a->position.x < b->position.x)
		return true;
	else if (a->position.x > b->position.x)
		return false;
	else
		return clickable_comp_y(a, b);
}

bool Game::clickable_comp_y(const Clickable* a, const Clickable* b)
{
	if (a->position.y < b->position.y)
		return true;
	else
		return false;
}

std::function<void()> Game::Node::SELECT_ADDITION()
{
	sf::Color default_color = rectangle->getOutlineColor();
	float default_thickness = rectangle->getOutlineThickness();
	sf::RectangleShape* rect = rectangle;

	rect->setOutlineColor(OUTLINE_COLOR());
	rect->setOutlineThickness(OUTLINE_THICKNESS());

	return [rect, default_color, default_thickness]()
	{
		rect->setOutlineColor(default_color);
		rect->setOutlineThickness(default_thickness);
	};
}

Game::Node::Node(size_t _i, size_t _j, float node_size, const sf::Vector2f& pos) :
	Game::ClickableStyleNode<Node>{ sf::Text(sf::String(std::to_string(_j * side_length + _i + size_t{ 1 })), FONT), sf::Vector2f(node_size, node_size),
	pos, sf::Vector2f(pos.x + node_size / size_t{ 3 }, pos.y + node_size / size_t{ 3 }) }, i{ _i }, j{ _j }, value{ _j * side_length + _i + size_t{ 1 } }
{
	rectangle->setFillColor(NODE_COLOR());
	rectangle->setOutlineColor(OUTLINE_COLOR());
}

Game::Node::~Node()
{
	if (this->active)
	{
		delete this->rectangle;
		delete this->text;
	}
}

void Game::Node::try_move()
{
	BEGIN_INTERACTION(this)

	rectangle->setOutlineThickness(0.f);

	bool is_adjacent = false;
	for (Node* t : get_adjacent())
	{
		t->rectangle->setOutlineThickness(0.f);
		t->rectangle->setOutlineColor(OUTLINE_COLOR());

		if (this == t)
			is_adjacent = true;
	}
	Node* prev_node = this;

	if (is_adjacent)
		prev_node = swap_with_empty();
	END_INTERACTION(this)

	prev_node->play_animation(*animations[prev_node->i - i + 1][prev_node->j - j + 1]);

	for (Node* t : get_adjacent())
	{
		BEGIN_INTERACTION(t)
		t->rectangle->setOutlineColor(AVAILABILITY_COLOR());
		t->rectangle->setOutlineThickness(OUTLINE_THICKNESS());
		END_INTERACTION(t)
	}

	if (!is_adjacent)
	{
		BEGIN_INTERACTION(prev_node)
		prev_node->rectangle->setOutlineColor(OUTLINE_COLOR());
		prev_node->rectangle->setOutlineThickness(OUTLINE_THICKNESS());
		END_INTERACTION(prev_node)
	}
}

void Game::Node::ON_CLICK()
{
	try_move();
}

void Game::Node::play_animation(const Animation & animation)
{
	for (int i = 0; i < animation.step_count; i++)
	{
		rectangle->move(animation.movement_table[i].first);
		text->move(animation.movement_table[i].first);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(animation.movement_table[i].second * 1000)));
	}
	rectangle->setPosition(this->position);
	text->setPosition(this->text_position);
}

Game::Node* Game::Node::swap_with_empty()
{
	empty_node->rectangle = this->rectangle;
	this->rectangle = nullptr;
	empty_node->text = this->text;
	this->text = nullptr;
	empty_node->value = this->value;
	this->value = side_length * side_length;
	empty_node->active = true;
	this->active = false;
	Node* prev_node = empty_node;
	empty_node = this;

	update_empty_adj();

	return prev_node;
}

void Game::Node::initialize_nodes()
{
	float table_side_size = std::min(window_height, window_width);
	float padding = (std::max(window_height, window_width) - table_side_size) / 2;
	sf::Vector2f starting_position;
	if (window_height < window_width)
	{
		starting_position.x = padding + table_side_size * 0.1;
		starting_position.y = table_side_size * 0.1;
	}
	else
	{
		starting_position.x = table_side_size * 0.1;
		starting_position.y = padding + table_side_size * 0.1;
	}

	table = new Node**[side_length];
	for (int i = 0; i < side_length; i++)
		table[i] = new Node*[side_length];

	float offset = (table_side_size * 0.1) / (side_length - 1);  // Blin, opyat' zvezdanulsya
	float node_size = (table_side_size * 0.8 * 0.875) / side_length;
	_OUTLINE_THICKNESS = offset / 3.f;

	sf::Vector2f position = starting_position;
	for (int j = 0; j < side_length; j++)
	{
		for (int i = 0; i < side_length; i++)
		{
			table[i][j] = create_clickable<Node>(i, j, node_size, position);
			position.x += offset + node_size;
		}
		position.x = starting_position.x;
		position.y += offset + node_size;
	}
	empty_node = table[side_length - 1][side_length - 1];
	empty_node->active = false;

	delete empty_node->rectangle;
	empty_node->rectangle = nullptr;
	delete empty_node->text;
	empty_node->text = nullptr;

	//Animations
	float shift1 = offset / 3.f;
	float a1 = (64.f*shift1) / (pow(cant_move_animation_time, 3));
	float b1 = (-1.f*a1) * cant_move_animation_time;
	float c1 = (32.f * shift1) / (3.f * cant_move_animation_time);


	std::string move_equation = "t^2*(" + std::to_string(a1) + ")+t*(" + std::to_string(b1) + ")+(" + std::to_string(c1) + ")";
	animations[1][1] = new Animation(move_equation, "0", 0.f, cant_move_animation_time);

	float shift = node_size + offset;
	
	float a = 0.9f * animation_time;
	float k = ((2 / (a * animation_time - a * a)) * (shift + (offset / 3) - (a / animation_time)*shift));  //Zvezdanut'sya mojno
	float v0 = shift / animation_time + (k * animation_time) / 2;

	move_equation = "-" + std::to_string(k) + "*t+" + std::to_string(v0);

	animations[1][2] = new Animation("0", move_equation, 0.f, animation_time);
	animations[2][1] = new Animation(move_equation, "0", 0.f, animation_time);
	animations[1][0] = new Animation("0", "-1*(" + move_equation + ")", 0.f, animation_time);
	animations[0][1] = new Animation("-1*(" + move_equation + ")", "0", 0.f, animation_time);
	//
	update_empty_adj();
	for (Node* t : get_adjacent())
	{
		BEGIN_INTERACTION(t)
		t->rectangle->setOutlineColor(AVAILABILITY_COLOR());
		t->rectangle->setOutlineThickness(OUTLINE_THICKNESS());
		END_INTERACTION(t)
	}
}

void Game::Node::reset_nodes()
{
	float table_side_size = std::min(window_height, window_width);
	float node_size = (table_side_size * 0.8 * 0.875) / side_length;
	for (int k = 0; k < side_length * side_length - 1; k++)
	{
		Node* n = table[k % side_length][k / side_length];
		BEGIN_INTERACTION(n)
		delete n->text;
		n->text = new sf::Text(sf::String(std::to_string(k + 1)), FONT);
		n->text->setPosition(n->text_position);
		n->text->setCharacterSize(static_cast<int>(node_size / 3));
		n->text->setFillColor(BG_COLOR);
		delete n->rectangle;
		n->rectangle = new sf::RectangleShape(sf::Vector2f(node_size, node_size));
		n->rectangle->setPosition(n->position);
		n->rectangle->setFillColor(NODE_COLOR());
		n->rectangle->setOutlineColor(OUTLINE_COLOR());
		n->rectangle->setOutlineThickness(0.f);
		n->value = k + 1;
		n->active = true;
		END_INTERACTION(n)
	}

	Node* n = table[side_length - 1][side_length - 1];
	BEGIN_INTERACTION(n)
	delete n->rectangle;
	n->rectangle = nullptr;
	delete n->text;
	n->text = nullptr;
	n->value = side_length * side_length;
	END_INTERACTION(n)
	empty_node = n;

	update_empty_adj();
	for (Node* t : get_adjacent())
	{
		BEGIN_INTERACTION(t)
		t->rectangle->setOutlineColor(AVAILABILITY_COLOR());
		t->rectangle->setOutlineThickness(OUTLINE_THICKNESS());
		END_INTERACTION(t)
	}
}

bool Game::Clickable::is_active() const
{
	return active;
}

void Game::Clickable::draw(sf::RenderWindow & window) const
{
	if (!active)
		return;

	BEGIN_INTERACTION(this)

	if (rectangle && text)
	{
		window.draw(*rectangle);
		window.draw(*text);
	}

	END_INTERACTION(this)
}

void Game::Clickable::click()
{
	ON_CLICK();
}

template<typename T>
const sf::Color& Game::ClickableStyleNode<T>::NODE_COLOR()
{
	return T::_NODE_COLOR;
}

template<typename T>
const sf::Color& Game::ClickableStyleNode<T>::OUTLINE_COLOR()
{
	return T::_OUTLINE_COLOR;
}

const sf::Color& Game::Node::AVAILABILITY_COLOR()
{
	return _AVAILABILITY_COLOR;
}

template<typename T>
float Game::ClickableStyleNode<T>::OUTLINE_THICKNESS()
{
	return T::_OUTLINE_THICKNESS;
}

template<typename T>
Game::ClickableStyleNode<T>::ClickableStyleNode(const sf::Text& caption, const sf::Vector2f& size, const sf::Vector2f& pos, const sf::Vector2f& text_pos) : Clickable(caption, size, pos, text_pos) 
{
	rectangle->setFillColor(NODE_COLOR());
	rectangle->setOutlineColor(OUTLINE_COLOR());
}

template<typename T>
void Game::ClickableStyleNode<T>::set_default_outline()
{
	rectangle->setOutlineThickness(0.f);
	rectangle->setOutlineColor(OUTLINE_COLOR());
}

void Game::initialize_game(size_t sl)
{
	if(!window)
		window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "Puzzle", sf::Style::Titlebar | sf::Style::Close);
	FONT.loadFromFile("ArialRegular.ttf");
	side_length = sl;

	UI = new std::list<Clickable*>();

	mouse_move_events = new EventQueue();
	mouse_click_events = new EventQueue();
	empty_adjacent = new AdjacentSet();
	Node::initialize_nodes();
	Button::initialize_buttons();
	UI->sort(&Game::clickable_comp_x);
	pos_tree = new PositionTree();
}

void Game::draw_process()
{		
	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		if (check_buttons(e))
			continue;

		Clickable* node = pos_tree->match(e.mouseMove.x, e.mouseMove.y);
		if (node && node->is_active())
			node->select();
	}
}

bool Game::start_game()
{
	std::thread solve_thread(solve_game);
	std::thread draw_thread(draw_process);
	std::thread click_thread(click_process);
	while (window->isOpen())
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();
			else if (event.type == sf::Event::MouseMoved)
				mouse_move_events->push(event);
			else if (event.type == sf::Event::MouseButtonPressed)
				mouse_click_events->push(event);
		}

		if (reset_button->is_pressed())
		{
			reset_button->set_released();
			reset();
		}

		if (ready_button->is_pressed())
		{
			mouse_click_events->disable();
			mouse_move_events->disable();
			Button::deactivate_buttons();
		}

		window->clear();

		for (int k = 0; k < side_length * side_length; k++)
			table[k % side_length][k / side_length]->draw(*window);

		for (auto t : buttons)
			t->draw(*window);

		window->display();
	}
	solve_thread.detach();
	draw_thread.detach();
	click_thread.detach();
	return true;
}

void Game::solve_game()
{
	ready_button->press_lock.lock();
	ready_button->press_condition.wait(ready_button->press_lock);
	int** int_table = represent_to_int();
	Solver solver(int_table, side_length);
	solver.solve();
	for (auto t : solver.solution())
		move(t.first, t.second);


	//window->close();
	//reset();
	//restart_threads();
	
}

void Game::move(size_t i, size_t j)
{
	if (table[i][j] != empty_node)
	{
		table[i][j]->click();
		if (table[i][j] != empty_node)
			table[i][j]->set_default_outline();
	}
}

void Game::update_empty_adj()
{
	empty_adjacent->clear();

	size_t j = empty_node->j;
	size_t i = empty_node->i;

	if (i < side_length - 1)
		empty_adjacent->add(table[i + 1][j]);
	if (i > 0)
		empty_adjacent->add(table[i - 1][j]);
	if (j < side_length - 1)
		empty_adjacent->add(table[i][j + 1]);
	if (j > 0)
		empty_adjacent->add(table[i][j - 1]);
}

Game::AdjacentSet& Game::get_adjacent()
{
	return *empty_adjacent;
}

void Game::click_process()
{
	while (window->isOpen())
	{
		const sf::Event& e = mouse_click_events->pop();
		Clickable* node = pos_tree->match(e.mouseButton.x, e.mouseButton.y);

		if (node && node->is_active())
		{
			mouse_click_events->disable();
			mouse_move_events->disable();
			node->click();
			mouse_move_events->enable();

			sf::Event e;
			e.mouseMove.x = sf::Mouse::getPosition(*window).x;
			e.mouseMove.y = sf::Mouse::getPosition(*window).y;
			mouse_move_events->push(e);
			mouse_move_events->push(e);

			mouse_click_events->enable();
		}
	}
}

void Game::Button::initialize_buttons()
{
	float table_side_size = std::min(window_height, window_width);
	float padding = (std::max(window_height, window_width) - table_side_size) / 2;
	float button_size = table_side_size * 0.075;
	_OUTLINE_THICKNESS = table_side_size * 0.005;

	sf::Vector2f button_position;
	if (window_height < window_width)
	{
		button_position.x = padding + table_side_size - table_side_size * 0.0875;
		button_position.y = table_side_size * 0.5 - table_side_size * 0.0375;
	}
	else
	{
		button_position.x = table_side_size * 0.5 - table_side_size * 0.0375;
		button_position.y = padding + table_side_size - table_side_size * 0.0875;
	}

	ready_button = create_clickable<Button>(button_size, button_size, button_position, "S"); //S means Solve
	buttons[0] = ready_button;

	if (window_height < window_width)
	{
		button_position.x = padding + table_side_size * 0.0125;
		button_position.y = table_side_size * 0.5 - table_side_size * 0.0375;
	}
	else
	{
		button_position.x = table_side_size * 0.5 - table_side_size * 0.0375;
		button_position.y = padding + table_side_size * 0.0125;
	}

	reset_button = create_clickable<Button>(button_size, button_size, button_position, "R"); //R means Reset
	buttons[1] = reset_button;
}

void Game::Button::deactivate_buttons()
{
	for (auto t : buttons)
		t->active = false;
}

bool Game::check_buttons(const sf::Event& event)
{
	for (Button* t : buttons)
		if (t->contains(event.mouseMove.x, event.mouseMove.y))
		{
			t->select();
			return true;
		}
	return false;
}

void Game::restart_threads()
{
	mouse_click_events->enable();
	mouse_move_events->enable();
	sf::Event e;
	e.mouseMove.x = -1.f;
	e.mouseMove.y = -1.f;
	mouse_move_events->push(e);
	mouse_move_events->push(e);
	e.mouseButton.x = -1.f;
	e.mouseButton.y = -1.f;
	mouse_click_events->push(e);
}

void Game::reset()
{
	score_counter = 0;
	Node::reset_nodes();
}

void Game::clear()
{
	//TODO

	/*if(window->isOpen())
		window->close();
	restart_threads();
	delete pos_tree;
	delete mouse_move_events;
	delete mouse_click_events;
	delete empty_adjacent;

	for (auto t : *UI)
		delete t;

	delete UI;*/
}

int** Game::represent_to_int()
{
	int** int_table = new int*[side_length];
	for (int i = 0; i < side_length; i++)
		int_table[i] = new int[side_length];

	for (int j = 0; j < side_length; j++)
		for (int i = 0; i < side_length; i++)
			int_table[i][j] = table[i][j]->value;

	return int_table;
}

sf::Event Game::EventQueue::pop()
{
	BEGIN_INTERACTION(this)
	bool empty = events.empty();

	if (empty)
	{
		END_INTERACTION(this)
		ql.lock();
		qcv.wait(ql);
		BEGIN_INTERACTION(this)
	}

	sf::Event event = events.back();
	events.pop_back();

	END_INTERACTION(this)

	if (empty)
		ql.unlock();

	return event;
}

void Game::EventQueue::push(const sf::Event & event)
{
	BEGIN_INTERACTION(this)
	if (!enabled) 
		INTERACTION_RETURN()

	bool empty = events.empty();
	events.push_front(event);
	if (empty)
		qcv.notify_all();

	END_INTERACTION(this)
}

void Game::EventQueue::disable()
{
	BEGIN_INTERACTION(this)
	enabled = false;
	END_INTERACTION(this)
}

void Game::EventQueue::enable()
{
	BEGIN_INTERACTION(this)
	enabled = true;
	events.clear();
	END_INTERACTION(this)
}

Game::Clickable* Game::PositionTree::match(float x, float y)
{
	int x_index = get_index_x(x, 0, tree.size() - 1);
	if (x_index == -1)
		return nullptr;

	return get_index_y(y, 0, tree.at(x_index).second.size() - 1, tree.at(x_index).second, x);
}

Game::PositionTree::PositionTree()
{
	std::list<Clickable*>* list = UI;
	const Clickable* temp_clickable = nullptr;

	float t = -1.f;

	for (auto i = UI->cbegin(); i != UI->cend(); ++i)
	{
		temp_clickable = *i;

		if (temp_clickable->position.x == t)
			continue;

		tree.emplace_back();
		tree.back().first = temp_clickable->position.x;
		t = temp_clickable->position.x;

		for (const Clickable* t : *UI)
		{
			if (t->position.x > temp_clickable->position.x)
				break;

			if (t->position.x + t->size.x > temp_clickable->position.x)
				tree.back().second.push_back(t);
		}

		std::sort(tree.back().second.begin(), tree.back().second.end(), &Game::clickable_comp_y);
	}
}

int Game::PositionTree::get_index_x(float pos, int a, int b)
{
	if (a >= b)
	{
		if (b < 0)
			return -1;
		else
			return a;
	}

	int m = (a + b) / 2;

	if (pos < tree[m].first)
		return get_index_x(pos, a, m - 1);
	else if (pos > tree[m + 1].first)
		return get_index_x(pos, m + 1, b);
	else
		return m;
}

Game::Clickable* Game::PositionTree::get_index_y(float pos, int a, int b, std::vector<const Clickable*>& x_vec, float x_pos)
{
	if (a >= b)
	{
		if (b < 0)
			return nullptr;
		else if (x_vec.at(a)->is_active() && x_vec.at(a)->contains(x_pos, pos))
			return const_cast<Clickable*>(x_vec.at(a));
		else
			return nullptr;
	}

	int m = (a + b) / 2;

	if (pos < x_vec[m]->position.y)
		return get_index_y(pos, a, m - 1, x_vec, x_pos);
	else if (pos > x_vec[m + 1]->position.y)
		return get_index_y(pos, m + 1, b, x_vec, x_pos);
	else
	{
		if (x_vec.at(m)->is_active() && x_vec.at(m)->contains(x_pos, pos))
			return const_cast<Clickable*>(x_vec.at(m));
		else
			return nullptr;
	}
}

Game::Node::Animation::Animation(std::string expr_x, std::string expr_y, float t1, float t2) : step_count{ static_cast<size_t>((t2 - t1) / one_step_t + 1) }
{
	expression_tree expr_tree_x(expr_x, 't');
	expression_tree expr_tree_y(expr_y, 't');

	movement_table = new std::pair<sf::Vector2f, float>[step_count];
	for (int i = 0; i < step_count; i++)
	{
		movement_table[i].first = sf::Vector2f(expr_tree_x.calculate(t1 + i * one_step_t) * one_step_t, expr_tree_y.calculate(t1 + i * one_step_t) * one_step_t);
		movement_table[i].second = one_step_t;
	}
}

void Game::AdjacentSet::add(Game::Node* n)
{
	arr.at(index++) = n;
}

void Game::AdjacentSet::clear()
{
	index = 0;
}

Game::AdjacentSet::AdjItr Game::AdjacentSet::begin()
{
	return AdjItr(this, 0);
}

Game::AdjacentSet::AdjItr Game::AdjacentSet::end()
{
	return AdjItr(this, index);
}

Game::AdjacentSet::AdjItr::AdjItr(AdjacentSet* c, size_t _i) : container{ c }, i{ _i } {}

Game::AdjacentSet::AdjItr& Game::AdjacentSet::AdjItr::operator++()
{
	i++;
	return *this;
}

bool Game::AdjacentSet::AdjItr::operator!=(const AdjItr& a)
{
	return i != a.i;
}

inline Game::Node* Game::AdjacentSet::AdjItr::operator*()
{
	return container->arr.at(i);
}

Game::Button::Button(float button_size_x, float button_size_y, sf::Vector2f pos, std::string lable)
	: Game::ClickableStyleNode<Button>{ sf::Text(sf::String(lable), FONT), sf::Vector2f(button_size_x, button_size_y), 
		pos, sf::Vector2f(pos.x + button_size_x / 3, pos.y + button_size_y / 3) } 
{
	rectangle->setFillColor(NODE_COLOR());
	rectangle->setOutlineColor(OUTLINE_COLOR());
}

Game::Button::~Button()
{
	delete this->rectangle;
	delete this->text;
}


bool Game::Clickable::contains(const float & pos_x, const float & pos_y) const
{
	return !((pos_x > rectangle->getPosition().x + rectangle->getSize().x || pos_x < rectangle->getPosition().x)
		|| (pos_y > rectangle->getPosition().y + rectangle->getSize().y || pos_y < rectangle->getPosition().y));
}

void Game::Clickable::select()
{
	BEGIN_INTERACTION(this)
	auto post_callback = SELECT_ADDITION();
	END_INTERACTION(this)


	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		BEGIN_INTERACTION(this)

		if (!active) 
			INTERACTION_RETURN()

		if (!contains(e.mouseMove.x, e.mouseMove.y))
		{
			END_INTERACTION(this)
			break;
		}

		END_INTERACTION(this)
	}

	BEGIN_INTERACTION(this)
	post_callback();
	END_INTERACTION(this)
}

void Game::Button::set_pressed()
{
	if (pressed || !active)
		return;

	active = false;
	pressed = true;
	BEGIN_INTERACTION(this)
	rectangle->setOutlineThickness(0.f);
	END_INTERACTION(this)
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	BEGIN_INTERACTION(this)
	rectangle->setOutlineThickness(OUTLINE_THICKNESS());
	END_INTERACTION(this)
	press_condition.notify_one();
}

void Game::Button::set_released()
{
	pressed = false;
	active = true;
}

bool Game::Button::is_pressed()
{
	return pressed;
}

void Game::Button::ON_CLICK()
{
	set_pressed();
}

std::function<void()> Game::Button::SELECT_ADDITION()
{
	sf::Color default_color = rectangle->getOutlineColor();
	float default_thickness = rectangle->getOutlineThickness();
	sf::RectangleShape* rect = rectangle;

	rect->setOutlineColor(OUTLINE_COLOR());
	rect->setOutlineThickness(OUTLINE_THICKNESS());

	return [rect, default_color, default_thickness]()
	{
		rect->setOutlineColor(default_color);
		rect->setOutlineThickness(default_thickness);
	};
}

Game::Clickable::Clickable(const sf::Text& caption, const sf::Vector2f& _size, const sf::Vector2f& pos, const sf::Vector2f& text_pos)
	: position{ pos }, text_position{ text_pos }, size{_size}
{
	{
		rectangle = new sf::RectangleShape(size);
		rectangle->setPosition(pos);
		rectangle->setOutlineThickness(0.f);
		text = new sf::Text(caption);
		text->setPosition(text_position);
		text->setCharacterSize(static_cast<int>(size.y / 3));
		text->setFillColor(BG_COLOR);
	}
}
