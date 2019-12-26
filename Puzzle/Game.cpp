#include "Game.h"
#include <algorithm>
#include <SFML/System/String.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

//Style parameters
const sf::Color BG_COLOR = sf::Color::Black;
const sf::Color NODE_COLOR = sf::Color::White;
const sf::Color OUTLINE_COLOR = sf::Color::Yellow;
sf::Font Game::FONT;
//

//Table parameters
Game::Node*** Game::table;
size_t Game::side_length;
//

//Window parameters
const float Game::window_height = 720;
const float Game::window_width = 1280;
sf::RenderWindow* Game::window;
//

Game::Node::Node(size_t _i, size_t _j, float node_size, sf::Vector2f pos) : i{ _i }, j{ _j }
{
	rectangle = new sf::RectangleShape(sf::Vector2f(node_size, node_size));
	rectangle->setPosition(pos);
	rectangle->setFillColor(NODE_COLOR);
	rectangle->setOutlineColor(OUTLINE_COLOR);
	rectangle->setOutlineThickness(0.f);
	value = (_j + 1) * side_length + i + 1;
	text = new sf::Text(sf::String(std::to_string(value)), FONT);
	text->setPosition(sf::Vector2f(pos.x + node_size / 3, pos.y + node_size / 3));
	text->setCharacterSize(static_cast<int>(node_size / 3));
	text->setFillColor(BG_COLOR);
}

bool Game::Node::contains(const sf::Vector2f & pos)
{
	return !((pos.x > rectangle->getPosition().x + rectangle->getSize().x || pos.x < rectangle->getPosition().x)
		|| (pos.y > rectangle->getPosition().y + rectangle->getSize().y || pos.y < rectangle->getPosition().y));
}

void Game::Node::draw(sf::RenderWindow & window)
{
	window.draw(*rectangle);
	window.draw(*text);
}

void Game::initialize_game(size_t sl)
{
	window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "Puzzle", sf::Style::Titlebar | sf::Style::Close);
	FONT.loadFromFile("ArialRegular.ttf");
	side_length = sl;

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

	float offset = (table_side_size * 0.1) / (sl - 1);  // Blin, opyat' zvezdanulsya
	float node_size = (table_side_size * 0.8 * 0.875) / sl;

	sf::Vector2f position = starting_position;
	for (int i = 0; i < sl; i++)
	{
		for (int j = 0; j < sl; j++)
		{
			table[i][j] = new Node(i, j, node_size, position);
			position.x += offset + node_size;
		}
		position.x = starting_position.x;
		position.y += offset + node_size;
	}

	start_game();
}

bool Game::start_game()
{
	while (window->isOpen())
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();
		}

		window->clear();
		for (int i = 0; i < side_length; i++)
			for (int j = 0; j < side_length; j++)
				table[i][j]->draw(*window);
		window->display();
	}
	return true;
}

std::vector<Game::Node*> Game::get_adjacent(const Game::Node & this_node)
{
	std::vector<Game::Node*> adjacents;
	if (this_node.i != 0)
		adjacents.push_back(table[this_node.i - 1][this_node.j]);
	if(this_node.i != side_length - 1)
		adjacents.push_back(table[this_node.i + 1][this_node.j]);
	if (this_node.j != 0)
		adjacents.push_back(table[this_node.i][this_node.j - 1]);
	if (this_node.j != side_length - 1)
		adjacents.push_back(table[this_node.i][this_node.j + 1]);
	return adjacents;
}

sf::Event Game::EventQueue::pop()
{
	if (events.empty())
		cv.wait(lock);

	sf::Event event = events.back();
	events.pop_back();
	return event;
}

void Game::EventQueue::push(const sf::Event & event)
{
	events.push_front(event);
	if (events.size() == 1)
		cv.notify_one();
}
