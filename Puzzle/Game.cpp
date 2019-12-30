#include "Game.h"
#include "ExpressionParser.h"
#include <algorithm>
#include <iostream>
#include <utility>
#include <SFML/System/String.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "integration.h"
#include <cmath>

#define BEGIN_INTERACTION(NODE_POINTER) NODE_POINTER->interaction_mutex.lock()

#define END_INTERACTION(NODE_POINTER) NODE_POINTER->interaction_mutex.unlock()

#define INTERACTION_RETURN  \
interaction_mutex.unlock(); \
return						\

//Style parameters
const sf::Color BG_COLOR = sf::Color::Black;
const sf::Color NODE_COLOR = sf::Color::White;
const sf::Color OUTLINE_COLOR = sf::Color::Yellow;
const sf::Color AVAILABILITY_COLOR = sf::Color::Blue;
float OUTLINE_THICKNESS = 5.f;
sf::Font Game::FONT;

const float cant_move_animation_time = 0.2f;
const float animation_time = 0.4f;
//

Game::EventQueue* Game::mouse_move_events = nullptr;
Game::EventQueue* Game::mouse_click_events = nullptr;
Game::PositionTree* Game::pos_tree = nullptr;
Game::AdjacentSet* Game::empty_adjacent = nullptr;
const float Game::Node::Animation::one_step_t = 0.008f;

const Game::Node::Animation* Game::Node::animations[3][3] = { nullptr };

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

Game::Node::Node(size_t _i, size_t _j, float node_size, sf::Vector2f pos) : i{ _i }, j{ _j }, 
	position{ pos }, text_position{ sf::Vector2f(pos.x + node_size / 3, pos.y + node_size / 3) }
{
	value = j * side_length + i + 1;
	if (value == side_length * side_length)
	{
		rectangle = nullptr;
		text = nullptr;
	}
	else
	{
		rectangle = new sf::RectangleShape(sf::Vector2f(node_size, node_size));
		rectangle->setPosition(pos);
		rectangle->setFillColor(NODE_COLOR);
		rectangle->setOutlineColor(OUTLINE_COLOR);
		rectangle->setOutlineThickness(0.f);
		text = new sf::Text(sf::String(std::to_string(value)), FONT);
		text->setPosition(text_position);
		text->setCharacterSize(static_cast<int>(node_size / 3));
		text->setFillColor(BG_COLOR);
	}
}

Game::Node::~Node()
{
	delete this->rectangle;
	delete this->text;
}

void Game::Node::try_move()
{
	BEGIN_INTERACTION(this);

	rectangle->setOutlineThickness(0.f);

	bool is_adjacent = false;
	for (Node* t : get_adjacent())
	{
		t->rectangle->setOutlineThickness(0.f);
		t->rectangle->setOutlineColor(OUTLINE_COLOR);

		if (this == t)
			is_adjacent = true;
	}
	Node* prev_node = this;

	if (is_adjacent)
		prev_node = swap_with_empty();
	END_INTERACTION(this);

	prev_node->play_animation(*animations[prev_node->i - i + 1][prev_node->j - j + 1]);

	for (Node* t : get_adjacent())
	{
		BEGIN_INTERACTION(t);
		t->rectangle->setOutlineColor(AVAILABILITY_COLOR);
		t->rectangle->setOutlineThickness(OUTLINE_THICKNESS);
		END_INTERACTION(t);
	}

	BEGIN_INTERACTION(prev_node);
	prev_node->rectangle->setOutlineThickness(OUTLINE_THICKNESS);
	END_INTERACTION(prev_node);
}

void Game::Node::select()
{
	BEGIN_INTERACTION(this);
	sf::Color default_color = rectangle->getOutlineColor();
	float default_thickness = rectangle->getOutlineThickness();
	sf::RectangleShape* rect = rectangle;
	END_INTERACTION(this);

	rect->setOutlineColor(OUTLINE_COLOR);
	rect->setOutlineThickness(OUTLINE_THICKNESS);


	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		BEGIN_INTERACTION(this);

		if (this == empty_node) { INTERACTION_RETURN; }

		if (!contains(e.mouseMove.x, e.mouseMove.y))
		{
			END_INTERACTION(this);
			break;
		}

		END_INTERACTION(this);
	}

	rect->setOutlineColor(default_color);
	rect->setOutlineThickness(default_thickness);
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
	OUTLINE_THICKNESS = offset / 3.f;

	sf::Vector2f position = starting_position;
	for (int j = 0; j < side_length; j++)
	{
		for (int i = 0; i < side_length; i++)
		{
			table[i][j] = new Node(i, j, node_size, position);
			position.x += offset + node_size;
		}
		position.x = starting_position.x;
		position.y += offset + node_size;
	}
	empty_node = table[side_length - 1][side_length - 1];

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
}

bool Game::Node::contains(const float& pos_x, const float& pos_y) const
{
	return !((pos_x > rectangle->getPosition().x + rectangle->getSize().x || pos_x < rectangle->getPosition().x)
		|| (pos_y > rectangle->getPosition().y + rectangle->getSize().y || pos_y < rectangle->getPosition().y));
}

void Game::Node::draw(sf::RenderWindow & window) const
{
	BEGIN_INTERACTION(this);

	if (rectangle && text)
	{
		window.draw(*rectangle);
		window.draw(*text);
	}

	END_INTERACTION(this);
}

void Game::initialize_game(size_t sl)
{
	window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "Puzzle", sf::Style::Titlebar | sf::Style::Close);
	FONT.loadFromFile("ArialRegular.ttf");
	side_length = sl;

	Node::initialize_nodes();
	
	mouse_move_events = new EventQueue();
	mouse_click_events = new EventQueue();
	pos_tree = new PositionTree();
	empty_adjacent = new AdjacentSet();
	update_empty_adj();
}

void Game::draw_process()
{		
	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		Node* node = pos_tree->match(e.mouseMove.x, e.mouseMove.y);
		if (node && node != empty_node)
			node->select();
	}
}

bool Game::start_game()
{
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
		window->clear();

		for (int k = 0; k < side_length * side_length; k++)
			table[k % side_length][k / side_length]->draw(*window);

		window->display();
	}

	draw_thread.join();
	click_thread.join();
	return true;
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
		Node* node = pos_tree->match(e.mouseButton.x, e.mouseButton.y);

		if (node && node != empty_node)
		{
			mouse_click_events->disable();
			mouse_move_events->disable();
			node->try_move();
			mouse_move_events->enable();

			sf::Event e;
			e.mouseMove.x = sf::Mouse::getPosition().x;
			e.mouseMove.y = sf::Mouse::getPosition().y;
			mouse_move_events->push(e);

			mouse_click_events->enable();
		}
	}
}

sf::Event Game::EventQueue::pop()
{
	bool empty = events.empty();
	if (empty)
	{
		ql.lock();
		qcv.wait(ql);
	}

	qm2.lock();
	sf::Event event = events.back();
	events.pop_back();

	qm2.unlock();

	if (empty)
		ql.unlock();

	return event;
}

void Game::EventQueue::push(const sf::Event & event)
{
	if (!enabled)
		return;

	qm2.lock();
	bool empty = events.empty();
	events.push_front(event);
	if (empty)
		qcv.notify_all();

	qm2.unlock();
}

void Game::EventQueue::disable()
{
	enabled = false;
	events.clear();
}

void Game::EventQueue::enable()
{
	enabled = true;
}

Game::Node* Game::PositionTree::match(float x, float y)
{
	int x_index = get_index(node_bounds_x, x);
	if (x_index == -1)
		return nullptr;

	int y_index = get_index(node_bounds_y, y);
	if (y_index == -1)
		return nullptr;

	return table[x_index][y_index];
}

Game::PositionTree::PositionTree()
{
	float node_size = (std::min(window_height, window_width) * 0.8 * 0.875) / side_length;
	node_bounds_x = new std::pair<float, float>[side_length];
	node_bounds_y = new std::pair<float, float>[side_length];

	for (int i = 0; i < side_length; i++)
	{
		node_bounds_x[i] = std::make_pair(table[i][0]->position.x, table[i][0]->position.x + node_size);
		node_bounds_y[i] = std::make_pair(table[0][i]->position.y, table[0][i]->position.y + node_size);
	}
}

Game::PositionTree::~PositionTree()
{
	delete[] node_bounds_x;
	delete[] node_bounds_y;
}

int Game::PositionTree::get_index(std::pair<float, float>* bounds_array, float pos, size_t a, size_t b)
{
	if (a >= b)
	{
		if (pos > bounds_array[a].second || pos < bounds_array[a].first)
			return -1;
		else
			return a;
	}

	size_t m = (a + b) / 2;

	if (pos < bounds_array[m].first)
		return get_index(bounds_array, pos, a, m - 1);
	else if (pos > bounds_array[m].second)
		return get_index(bounds_array, pos, m + 1, b);
	else
		return m;
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
