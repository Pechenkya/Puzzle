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

//Style parameters
const sf::Color BG_COLOR = sf::Color::Black;
const sf::Color NODE_COLOR = sf::Color::White;
const sf::Color OUTLINE_COLOR = sf::Color::Yellow;
float OUTLINE_THICKNESS = 5.f;
sf::Font Game::FONT;

float K = 100.f;
//

std::mutex* Game::th_mutex = new std::mutex;

Game::EventQueue* Game::mouse_move_events = nullptr;
Game::EventQueue* Game::mouse_click_events = nullptr;
Game::PositionTree* Game::pos_tree = nullptr;

long long int Game::check_counter = 0;

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
	rectangle->setOutlineThickness(0.f);

	bool is_adjacent = false;
	for (auto t : get_adjacent(empty_node))
	{
		if (this == t)
		{
			is_adjacent = true;
			break;
		}
	}
	Node* prev_node = this;

	if (is_adjacent)
		prev_node = swap_with_empty();

	prev_node->play_animation(*animations[prev_node->i - i + 1][prev_node->j - j + 1]);
}

void Game::Node::select()
{
	sf::Color default_color = rectangle->getOutlineColor();
	float default_thickness = rectangle->getOutlineThickness();
	sf::RectangleShape* rect = rectangle;

	rect->setOutlineColor(OUTLINE_COLOR);
	rect->setOutlineThickness(OUTLINE_THICKNESS);

	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		th_mutex->lock();
		th_mutex->unlock();

		if (this == empty_node)
			return;

		if (!contains(e.mouseMove.x, e.mouseMove.y))
			break;
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
	float V = 0.15f*K + (node_size + offset) / 0.3f;
	animations[1][1] = new Animation("t*cos(50*t)", "t*cos(50*t)", -0.5f, 0.5f);
	//std::string move_equation = std::to_string(V) + "-t";
	animations[1][2] = new Animation("0", "4-2*t", 0.f, 0.3f);
	animations[2][1] = new Animation("4+2*t", "0", 0.f, 0.3f);
	animations[1][0] = new Animation("0", "-4+2*t", 0.f, 0.3f);
	animations[0][1] = new Animation("-4-2*t", "0", 0.f, 0.3f);
	//
}

bool Game::Node::contains(const float& pos_x, const float& pos_y) const
{
	return !((pos_x > rectangle->getPosition().x + rectangle->getSize().x || pos_x < rectangle->getPosition().x)
		|| (pos_y > rectangle->getPosition().y + rectangle->getSize().y || pos_y < rectangle->getPosition().y));
}

void Game::Node::draw(sf::RenderWindow & window) const
{
	if (this != empty_node)
	{
		window.draw(*rectangle);
		window.draw(*text);
	}
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
}

void Game::draw_process()
{		
	while (window->isOpen())
	{
		th_mutex->lock();
		th_mutex->unlock();

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

std::vector<Game::Node*> Game::get_adjacent(const Game::Node* this_node)
{
	std::vector<Game::Node*> adjacents;
	if (this_node->i != 0)
		adjacents.push_back(table[this_node->i - 1][this_node->j]);

	if(this_node->i != side_length - 1)
		adjacents.push_back(table[this_node->i + 1][this_node->j]);

	if (this_node->j != 0)
		adjacents.push_back(table[this_node->i][this_node->j - 1]);

	if (this_node->j != side_length - 1)
		adjacents.push_back(table[this_node->i][this_node->j + 1]);

	return adjacents;
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
			th_mutex->lock();
			node->try_move();
			th_mutex->unlock();
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
	delete []node_bounds_x;
	delete []node_bounds_y;
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

Game::Node::Animation::Animation(std::string expr_x, std::string expr_y, float t1, float t2)
{
	expression_tree expr_tree_x(expr_x, 't');
	expression_tree expr_tree_y(expr_y, 't');

	float diff = t2 - t1;
	step_count = diff / 0.008 + 1;
	float one_step_t = 0.008;
	movement_table = new std::pair<sf::Vector2f, float>[step_count];
	for (int i = 0; i < step_count; i++)
	{
		movement_table[i].first = sf::Vector2f(expr_tree_x.calculate(t1 + i * one_step_t), expr_tree_y.calculate(t1 + i * one_step_t));
		movement_table[i].second = one_step_t;
	}
}
