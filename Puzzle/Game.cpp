#include "Game.h"

#include "ExpressionParser.h"

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

bool clicekd = false;

//Style parameters
template<typename T>
Game::Drawable::Style Game::ClickableStyleNode<T>::_DEFAULT_STYLE{ 0.f, sf::Color::White, sf::Color(211, 211, 211) };

template<typename T>
Game::Drawable::Style Game::ClickableStyleNode<T>::_SELECTED_STYLE{ 5.f, sf::Color::White, sf::Color(211, 211, 211) };

Game::Drawable::Style Game::Node::_AVAILABILITY_STYLE { 5.f, sf::Color(255, 173, 51), sf::Color(211, 211, 211) };

Game::Drawable::Style Game::Button::_SELECTED_STYLE{ 5.f, sf::Color::Green, sf::Color(211, 211, 211) };

float Game::Button::game_outline_thickness = 5.f;
float Game::Button::menu_outline_thickness = 5.f;
sf::Font Game::FONT;
static const sf::Color BG_COLOR = sf::Color::Black;

const float cant_move_animation_time = 0.2f;
const float animation_time = 0.4f;
//

// Game static member
Game::EventQueue* Game::mouse_move_events = nullptr;
Game::EventQueue* Game::mouse_click_events = nullptr;
Game::EventQueue* Game::key_events = nullptr;
Game::AdjacentSet* Game::empty_adjacent = nullptr;

const float Game::Drawable::Animation::one_step_t = 0.008f;
const Game::Drawable::Animation* Game::Node::animations[3][3] = { nullptr };

Game::Lable* Game::last_score = nullptr;
int Game::move_counter = 0;
//

//Table parameters
Game::Node*** Game::table = nullptr;
size_t Game::side_length;
Game::Node* Game::empty_node = nullptr;
Game::Node* Game::last_selected_node = nullptr;
Game::Drawable::Style Game::last_selected_style;
//

//Window parameters
const float Game::window_height = 720;
const float Game::window_width = 1280;
sf::RenderWindow* Game::window;
//

//UI
Game::UserInterface* Game::game_UI = nullptr;
Game::UserInterface* Game::active_UI = nullptr;
std::list<Game::Button*>* Game::buttons = nullptr;
Game::UserInterface* Game::menu_UI = nullptr;
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
	Drawable::Style s = drawable->get_style();
	Drawable* d = drawable;
	
	if (last_selected_node != this)
	{
		selected = true;
		last_selected_node->selected = false;
		last_selected_node->drawable->set_style(last_selected_style);
		last_selected_node = this;
		last_selected_style = s;
	}
	else
		s = last_selected_style;

	drawable->set_style(SELECTED_STYLE());
	

	return [this, d, s]()
	{
		this->selected = false;
		d->set_style(s);
	};
}

std::function<void()> Game::Node::CLICK_ADDITION()
{
	drawable->set_style(DEFAULT_STYLE());

	for (Node* t : get_adjacent())
		t->drawable->set_style(DEFAULT_STYLE());

	return [this]()
	{
		for (Node* t : get_adjacent())
		{
			BEGIN_INTERACTION(t)
			t->drawable->set_style(AVAILABILITY_STYLE());
			END_INTERACTION(t)
		}
		
		last_selected_style = last_selected_node->drawable->get_style();
		last_selected_node->drawable->set_style(SELECTED_STYLE());
		//this->drawable->set_style(SELECTED_STYLE());
	};
}

// YA PROSTO ZVEZDANUSLYA VOOBSHE, HTO YA?
Game::Node::Node(size_t _i, size_t _j, float node_size, const sf::Vector2f& pos) :
	Game::ClickableStyleNode<Node>{sf::Vector2f(node_size, node_size), pos, [](Clickable* t) 
	{ 
		Node* n = dynamic_cast<Node*>(t);
		move_counter++;
		n->try_move();
	}, 
new Drawable(sf::Text(sf::String(std::to_string(_j * side_length + _i + size_t{ 1 })), FONT), sf::Vector2f(node_size, node_size), pos, sf::Vector2f(node_size / size_t{ 3 }, node_size / size_t{ 3 })) },
	i{ _i }, j{ _j }, value{ _j * side_length + _i + size_t{ 1 } }{}

Game::Node::~Node()
{
	Clickable::~Clickable();
	for (auto t : animations)
		delete[] t;
	delete[] animations;
}

void Game::Node::select_up()
{
	auto post_callback = last_selected_node->key_select();

	if (last_selected_node->j > 0)
	{
		last_selected_node = table[last_selected_node->i][last_selected_node->j - 1];
		if (!last_selected_node->is_active() && last_selected_node->j > 0)
			last_selected_node = table[last_selected_node->i][last_selected_node->j - 1];
		else if(!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i][side_length - 1];
	}
	else
	{
		last_selected_node = table[last_selected_node->i][side_length - 1];
		if (!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i][last_selected_node->j - 1];
	}

	post_callback();
}

void Game::Node::select_down()
{
	auto post_callback = last_selected_node->key_select();

	if (last_selected_node->j < side_length - 1)
	{
		last_selected_node = table[last_selected_node->i][last_selected_node->j + 1];
		if (!last_selected_node->is_active() && last_selected_node->j < side_length - 1)
			last_selected_node = table[last_selected_node->i][last_selected_node->j + 1];
		else if (!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i][0];
	}
	else
	{
		last_selected_node = table[last_selected_node->i][0];
		if (!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i][last_selected_node->j + 1];
	}

	post_callback();
}

void Game::Node::select_left()
{
	auto post_callback = last_selected_node->key_select();

	if (last_selected_node->i > 0)
	{
		last_selected_node = table[last_selected_node->i - 1][last_selected_node->j];
		if (!last_selected_node->is_active() && last_selected_node->i > 0)
			last_selected_node = table[last_selected_node->i - 1][last_selected_node->j];
		else if (!last_selected_node->is_active())
			last_selected_node = table[side_length - 1][last_selected_node->j];
	}
	else
	{
		last_selected_node = table[side_length - 1][last_selected_node->j];
		if (!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i - 1][last_selected_node->j];
	}

	post_callback();
}

void Game::Node::select_right()
{
	auto post_callback = last_selected_node->key_select();

	if (last_selected_node->i < side_length - 1)
	{
		last_selected_node = table[last_selected_node->i + 1][last_selected_node->j];
		if (!last_selected_node->is_active() && last_selected_node->i < side_length - 1)
			last_selected_node = table[last_selected_node->i + 1][last_selected_node->j];
		else if (!last_selected_node->is_active())
			last_selected_node = table[0][last_selected_node->j];
	}
	else
	{
		last_selected_node = table[0][last_selected_node->j];
		if (!last_selected_node->is_active())
			last_selected_node = table[last_selected_node->i + 1][last_selected_node->j];
	}

	post_callback();
}

std::function<void()> Game::Node::key_select()
{
	last_selected_node->drawable->set_style(last_selected_style);
	last_selected_node->selected = false;

	return []() {
		last_selected_node->selected = true;
		last_selected_style = last_selected_node->drawable->get_style();
		last_selected_node->drawable->set_style(SELECTED_STYLE());
	};
}

void Game::Node::try_move()
{
	BEGIN_INTERACTION(this)

	bool is_adjacent = false;
	for (Node* t : get_adjacent())
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
	END_INTERACTION(this)

	prev_node->drawable->play_animation(*animations[prev_node->i - i + 1][prev_node->j - j + 1]);
	prev_node->drawable->set_position(prev_node->position);
	last_selected_node = prev_node;
}

void Game::Drawable::play_animation(const Animation & animation)
{
	for (int i = 0; i < animation.step_count; i++)
	{
		if (rectangle->getOutlineThickness() != 0.f)
			std::cout << "prekol" << std::endl;

		rectangle->move(animation.movement_table[i].first);
		text->move(animation.movement_table[i].first);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(animation.movement_table[i].second * 1000)));
	}
}

Game::Node* Game::Node::swap_with_empty()
{
	empty_node->drawable->set_position(position);
	std::swap(empty_node->drawable, this->drawable);
	std::swap(empty_node->value, value);
	std::swap(empty_node->active, active);
	Node* prev_node = empty_node;
	empty_node = this;

	update_empty_adj();

	return prev_node;
}

void Game::Node::initialize_nodes(std::list<Clickable*>* UI)
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
	_SELECTED_STYLE.outline_thickness = offset / 3.f;

	sf::Vector2f position = starting_position;
	for (int j = 0; j < side_length; j++)
	{
		for (int i = 0; i < side_length; i++)
		{
			table[i][j] = create_clickable<Node>(UI, i, j, node_size, position);
			position.x += offset + node_size;
		}
		position.x = starting_position.x;
		position.y += offset + node_size;
	}
	empty_node = table[side_length - 1][side_length - 1];
	empty_node->active = false;
	empty_node->drawable->set_visible(false);

	last_selected_node = table[0][0];
	last_selected_style = DEFAULT_STYLE();

	//Animations
	float shift1 = offset / 3.f;
	float a1 = (64.f*shift1) / (pow(cant_move_animation_time, 3));
	float b1 = (-1.f*a1) * cant_move_animation_time;
	float c1 = (32.f * shift1) / (3.f * cant_move_animation_time);


	std::string move_equation = "t^2*(" + std::to_string(a1) + ")+t*(" + std::to_string(b1) + ")+(" + std::to_string(c1) + ")";
	animations[1][1] = new Drawable::Animation(move_equation, "0", 0.f, cant_move_animation_time);

	float shift = node_size + offset;
	
	float a = 0.9f * animation_time;
	float k = ((2 / (a * animation_time - a * a)) * (shift + (offset / 3) - (a / animation_time)*shift));  //Zvezdanut'sya mojno
	float v0 = shift / animation_time + (k * animation_time) / 2;

	move_equation = "-" + std::to_string(k) + "*t+" + std::to_string(v0);

	animations[1][2] = new Drawable::Animation("0", move_equation, 0.f, animation_time);
	animations[2][1] = new Drawable::Animation(move_equation, "0", 0.f, animation_time);
	animations[1][0] = new Drawable::Animation("0", "-1*(" + move_equation + ")", 0.f, animation_time);
	animations[0][1] = new Drawable::Animation("-1*(" + move_equation + ")", "0", 0.f, animation_time);
	//
	update_empty_adj();
	for (Node* t : get_adjacent())
	{
		t->drawable->set_style(AVAILABILITY_STYLE());
	}
}

void Game::Node::reset_nodes()
{
	float table_side_size = std::min(window_height, window_width);
	float node_size = (table_side_size * 0.8 * 0.875) / side_length;
	for (int k = 0; k < side_length * side_length; k++)
	{
		Node* n = table[k % side_length][k / side_length];

		n->value = k + 1;
		n->drawable->set_text(std::to_string(n->value));
		n->drawable->set_style(DEFAULT_STYLE());
		n->active = true;
		n->drawable->set_visible(true);
	}

	empty_node = table[side_length - 1][side_length - 1];
	empty_node->active = false;
	empty_node->drawable->set_visible(false);

	update_empty_adj();
	for (Node* t : get_adjacent())
		t->drawable->set_style(AVAILABILITY_STYLE());
}


Game::Drawable::Drawable(const sf::Text & caption, const sf::Vector2f & _size, const sf::Vector2f & pos, const sf::Vector2f & text_pos) 
	: position{ pos },  text_position { text_pos }, rect_position{ sf::Vector2f() }
{
	{
		rectangle = new sf::RectangleShape(_size);
		rectangle->setPosition(pos);
		rectangle->setOutlineThickness(0.f);
		text = new sf::Text(caption);
		text->setPosition(pos + text_pos);
		text->setCharacterSize(static_cast<int>(_size.y / 3));
		text->setFillColor(BG_COLOR);
	}
}

Game::Drawable::~Drawable()
{
	delete rectangle;
	delete text;
}

Game::Drawable::Style Game::Drawable::get_style()
{
	return Style{ rectangle->getOutlineThickness(), rectangle->getOutlineColor(), rectangle->getFillColor() };
}

void Game::Drawable::set_style(const Style& s)
{
	rectangle->setOutlineThickness(s.outline_thickness);
	rectangle->setOutlineColor(s.outline_color);
	rectangle->setFillColor(s.fill_color);
}

void Game::Drawable::set_position(const sf::Vector2f p)
{
	position = p;
	rectangle->setPosition(position + rect_position);
	text->setPosition(position + text_position);
}

bool Game::Drawable::is_visible() const
{
	return visible;
}

void Game::Drawable::set_visible(bool u_suq_dyq)
{
	visible = u_suq_dyq;
}

void Game::Drawable::draw(sf::RenderWindow & window) const
{
	if (!visible)
		return;

	window.draw(*rectangle);
	window.draw(*text);
}

void Game::Drawable::set_text(const sf::String & t)
{
	text->setString(t);
}

void Game::Clickable::click()
{
	BEGIN_INTERACTION(this)
	auto post_callback = CLICK_ADDITION();
	END_INTERACTION(this)

	ON_CLICK(this);

	BEGIN_INTERACTION(this)
	post_callback();
	END_INTERACTION(this)
}

bool Game::Clickable::is_active() const
{
	return active;
}

bool Game::Clickable::contains(const float & pos_x, const float & pos_y) const
{
	return drawable->contains(pos_x, pos_y);
}

void Game::Clickable::draw(sf::RenderWindow & window) const
{	
	BEGIN_INTERACTION(this)
	drawable->draw(window);
	END_INTERACTION(this)
}

void Game::Clickable::deactivate()
{
	active = false;
}

void Game::Clickable::activate()
{
	active = true;
}

template<typename T>
const Game::Drawable::Style& Game::ClickableStyleNode<T>::DEFAULT_STYLE()
{
	return T::_DEFAULT_STYLE;
}

template<typename T>
const Game::Drawable::Style& Game::ClickableStyleNode<T>::SELECTED_STYLE()
{
	return T::_SELECTED_STYLE;
}

const Game::Drawable::Style& Game::Node::AVAILABILITY_STYLE()
{
	return _AVAILABILITY_STYLE;
}

template<typename T>
Game::ClickableStyleNode<T>::ClickableStyleNode(const sf::Vector2f& _size, const sf::Vector2f& pos, const std::function<void(Clickable*)> f, Drawable* d) :
	Clickable(_size, pos, f, d)
{
	drawable->set_style(DEFAULT_STYLE());
}

void Game::initialize_game(size_t sl)
{
	if(!window)
		window = new sf::RenderWindow(sf::VideoMode(window_width, window_height), "Puzzle", sf::Style::Titlebar | sf::Style::Close);
	FONT.loadFromFile("ArialRegular.ttf");
	side_length = sl;

	mouse_move_events = new EventQueue();
	mouse_click_events = new EventQueue();
	key_events = new EventQueue();
	empty_adjacent = new AdjacentSet();
	buttons = new std::list<Button*>();
	float padding = (std::max(window_height, window_width) - std::min(window_height, window_width)) / 2;
	sf::Vector2f pos = sf::Vector2f(padding * 0.03f, padding * 0.03f);
	last_score = new Lable(pos, "Last Score:");

	std::list<Clickable*>* UI = new std::list<Clickable*>();
	Node::initialize_nodes(UI);
	Button::initialize_buttons(UI);

	game_UI = new UserInterface(UI);

	initialize_menu();

	active_UI = menu_UI;
}

void Game::draw_process()
{		
	while (window->isOpen())
	{
		const sf::Event& e = mouse_move_events->pop();

		Clickable* node = active_UI->pos_tree->match(e.mouseMove.x, e.mouseMove.y);
		if (node && node->is_active())
			node->select();
	}
}

bool Game::start_game()
{
	std::thread draw_thread(draw_process);
	std::thread click_thread(click_process);
	std::thread keyboard_thread(keyboard_process);
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
			else if (event.type == sf::Event::KeyPressed)
				key_events->push(event);
		}

		window->clear();

		active_UI->draw(*window);
		last_score->draw(*window);

		window->display();
	}
	restart_threads();
	keyboard_thread.join();
	draw_thread.join();
	click_thread.join();
	return true;
}

void Game::solve_game()
{
	int** int_table = represent_to_int();
	Solver solver(int_table, side_length);
	solver.solve();
	for (auto t : solver.solution())
		move(t.first, t.second);
	int score;
	if (solver.moves_made() == 0)
		score = 0;
	else
		score = (solver.moves_made()) * 100 / (move_counter);
	last_score->set_caption("Last Score: " + std::to_string(score));
	last_score->active = true;
	reset();
}

void Game::move(size_t i, size_t j)
{
	if (table[i][j] != empty_node)
		table[i][j]->click();
}

void Game::initialize_menu()
{
	float button_size_x = window_width * 0.125;
	float button_size_y = window_height * 0.125;
	Button::menu_outline_thickness = button_size_y * 0.1;
	Button::_SELECTED_STYLE.outline_thickness = Button::menu_outline_thickness;

	sf::Vector2f button_position;
	button_position.x = window_width / 2 - button_size_x / 2;
	button_position.y = window_height / 2 - button_size_y / 2;

	std::list<Clickable*>* UI = new std::list<Clickable*>();


	create_clickable<Button>(UI, button_size_x, button_size_y, button_position, "Start", [](Clickable* t) {
		Button* b = dynamic_cast<Button*>(t);
		b->set_pressed();
		game_UI->activate();
		menu_UI->deactivate();
		Button::_SELECTED_STYLE.outline_thickness = Button::game_outline_thickness;
		active_UI = game_UI;
	});

	menu_UI = new UserInterface(UI);
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
		Clickable* node = active_UI->pos_tree->match(e.mouseButton.x, e.mouseButton.y);

		if (node)
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

void Game::keyboard_process()
{
	while (window->isOpen())
	{
		const sf::Event& e = key_events->pop();

		if (last_selected_node->is_active() && (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
			|| sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)))
		{
			mouse_click_events->disable();
			mouse_move_events->disable();
			key_events->disable();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				Node::select_up();
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				Node::select_down();
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				Node::select_left();
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				Node::select_right();
			else
				move(last_selected_node->i, last_selected_node->j);

			mouse_move_events->enable();
			sf::Event e;
			e.mouseMove.x = -1.f;
			e.mouseMove.y = -1.f;
			mouse_move_events->push(e);
			mouse_click_events->enable();
			key_events->enable();
		}
	}
}

void Game::Button::initialize_buttons(std::list<Clickable*>* UI)
{
	float table_side_size = std::min(window_height, window_width);
	float padding = (std::max(window_height, window_width) - table_side_size) / 2;
	float button_size_x = table_side_size * 0.125;
	float button_size_y = table_side_size * 0.075;
	game_outline_thickness = table_side_size * 0.005;

	sf::Vector2f button_position;
	button_position.x = window_width - button_size_x - padding * 0.03f;
	button_position.y = padding * 0.03f;

	buttons->push_back(
		create_clickable<Button>(UI, button_size_x, button_size_y, button_position, "Menu", [](Clickable* t) {
		Button* b = dynamic_cast<Button*>(t);
		b->set_pressed();
		menu_UI->activate();
		game_UI->deactivate();
		Button::_SELECTED_STYLE.outline_thickness = Button::menu_outline_thickness;
		active_UI = menu_UI;
	}));

	button_position.y = button_position.y + 3 * game_outline_thickness + button_size_y;

	buttons->push_back(
		create_clickable<Button>(UI, button_size_x, button_size_y, button_position, "Reset", [](Clickable* t) {
		Button* b = dynamic_cast<Button*>(t);
		b->set_pressed();
		active_UI->deactivate();
		reset();
		active_UI->activate();
	}));

	button_position.y = button_position.y + 3 * game_outline_thickness + button_size_y;

	buttons->push_back(
		create_clickable<Button>(UI, button_size_x, button_size_y, button_position, "Solve", [](Clickable* t) {
		Button* b = dynamic_cast<Button*>(t);
		b->set_pressed();
		active_UI->deactivate();
		solve_game();
		active_UI->activate();
	}));
}

void Game::Button::deactivate_buttons()
{
	for (auto t : *buttons)
		t->active = false;
}

void Game::Button::activate_buttons()
{
	for (auto t : *buttons)
		t->active = true;
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
	key_events->push(e);
}

void Game::reset()
{
	move_counter = 0;
	Node::reset_nodes();
}

void Game::clear()
{
	delete window;
	delete game_UI;
	delete menu_UI;
	delete mouse_move_events;
	delete mouse_click_events;
	delete key_events;
	delete empty_adjacent;
	delete last_score;
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
	{
		interaction_mutex.unlock();
		return;
	}

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

Game::Clickable* Game::PositionTree::match(float x, float y) const 
{
	int x_index = get_index_x(x, 0, tree.size() - 1);
	if (x_index == -1)
		return nullptr;

	return get_index_y(y, 0, tree.at(x_index).second.size() - 1, tree.at(x_index).second, x);
}

Game::PositionTree::PositionTree(const std::list<Clickable*>* UI)
{
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

int Game::PositionTree::get_index_x(float pos, int a, int b) const
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

Game::Clickable* Game::PositionTree::get_index_y(float pos, int a, int b, const std::vector<const Clickable*>& x_vec, float x_pos) const
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

Game::Drawable::Animation::Animation(std::string expr_x, std::string expr_y, float t1, float t2) : step_count{ static_cast<size_t>((t2 - t1) / one_step_t + 1) }
{
	expression_tree expr_tree_x(expr_x, 't');
	expression_tree expr_tree_y(expr_y, 't');

	for (int i = 0; i < step_count; i++)
		movement_table.emplace_back(sf::Vector2f(expr_tree_x.calculate(t1 + i * one_step_t) * one_step_t, expr_tree_y.calculate(t1 + i * one_step_t) * one_step_t), one_step_t);
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

Game::Node* Game::AdjacentSet::AdjItr::operator*()
{
	return container->arr.at(i);
}

Game::Button::Button(float button_size_x, float button_size_y, const sf::Vector2f& pos, std::string lable, const std::function<void(Clickable*)> f)
	: Game::ClickableStyleNode<Button>{ sf::Vector2f(button_size_x, button_size_y), pos, f,
	new Drawable(sf::Text(sf::String(lable), FONT), sf::Vector2f(button_size_x, button_size_y), pos, 
		sf::Vector2f(button_size_x / 3 - (lable.length() - 1) * button_size_x / 45, button_size_y / 3)) } {}

Game::Button::~Button()
{
	Clickable::~Clickable();
}

bool Game::Drawable::contains(const float & pos_x, const float & pos_y) const
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
		{
			interaction_mutex.unlock();
			return;
		}

		if (!contains(e.mouseMove.x, e.mouseMove.y) || !selected)
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
	drawable->set_style(DEFAULT_STYLE());
	std::this_thread::sleep_for(std::chrono::milliseconds(75));
	drawable->set_style(SELECTED_STYLE());
}

void Game::Button::deactivate()
{
	active = false;

	drawable->set_style(DEFAULT_STYLE());
}

void Game::Button::activate()
{
	active = true;
}

std::function<void()> Game::Button::SELECT_ADDITION()
{
	Drawable::Style s = drawable->get_style();
	Drawable* d = drawable;
	selected = true;
	
	drawable->set_style(SELECTED_STYLE());

	return [this, d, s]()
	{
		this->selected = false;
		d->set_style(s);
	};
}

std::function<void()> Game::Button::CLICK_ADDITION()
{
	this->set_pressed();
	Drawable::Style s = drawable->get_style();
	Drawable* d = drawable;

	drawable->set_style(DEFAULT_STYLE());

	return [this, d, s]()
	{
		if(this->is_active())
			d->set_style(s);
	};
	return []() {};
}

Game::Clickable::Clickable(const sf::Vector2f& _size, const sf::Vector2f& pos, const std::function<void(Clickable*)> f, Drawable* d)
	: drawable{ d }, position{ pos }, size{ _size }, ON_CLICK{ f } {}

Game::Clickable::~Clickable()
{
	delete drawable;
}

Game::UserInterface::UserInterface(std::list <Clickable*>* _UI)
{
	UI = _UI;
	UI->sort(&Game::clickable_comp_x);
	pos_tree = new PositionTree(UI);
}

Game::UserInterface::~UserInterface()
{
	for (auto t : *UI)
		delete t;
	delete UI;
	delete pos_tree;
}

void Game::UserInterface::deactivate()
{
	for (auto t : *UI)
		t->deactivate();
}

void Game::UserInterface::activate()
{
	for (Clickable* t : *UI)
	{
		if (t != empty_node)
			t->activate();
	}
}

void Game::UserInterface::draw(sf::RenderWindow & window) const
{
	for (Clickable* t : *UI)
		t->draw(window);
}

Game::Lable::Lable()
{
	active = false;
	text = new sf::Text(sf::String(" "), FONT);
	text->setPosition(sf::Vector2f(10.f, 10.f));
	text->setCharacterSize(static_cast<int>(window_height * 0.125 / 3));
	text->setFillColor(sf::Color::White);
}

Game::Lable::~Lable()
{
	delete text;
}

Game::Lable::Lable(const sf::Vector2f & pos, const std::string & caption)
{
	active = false;
	text = new sf::Text(sf::String(caption), FONT);
	text->setPosition(pos);
	text->setCharacterSize(static_cast<int>(window_height * 0.125 / 3));
	text->setFillColor(sf::Color::White);
}

void Game::Lable::set_caption(const std::string & caption)
{
	text->setString(caption);
}

void Game::Lable::draw(sf::RenderWindow & window) const
{
	if(active)
		window.draw(*text);
}
