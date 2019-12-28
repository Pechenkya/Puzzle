#include "ExpressionParser.h"
#include <stack>
#include <cmath>

using operation_t = expression_tree::operation;
using operands = std::pair<expression_tree::operation*, expression_tree::operation*>;
using op_t = expression_tree::op_type;

std::function<double(const operands&)> expression_tree::operations[13]
{
	param, sum, dif, prod, div, pow, sqr, sin, cos, tan, cot, log, lgn
};

char expression_tree::st[6]
{
	'x', '+', '-', '*', '/', '^'
};

double expression_tree::param_value;

expression_tree::expression_tree(std::string input, char param) :param_name{ param }
{
	for (int i = input.length() - 1; i >= 0; --i)
	{
		if (input.at(i) == ' ')
			input.erase(i, 1);
	}
	root = parse_subexpr(input);
}
//
double expression_tree::calculate(double x)
{
	expression_tree::param_value = x;
	return root->get();
}

operation_t* expression_tree::parse_subexpr(std::string expression)
{
	if (expression.front() == '(' && expression.back() == ')')
	{
		expression.pop_back();
		expression.erase(0, 1);
	}

	for (op_t i = op_t::sum; i != op_t::sqr; ++i)
	{
		int op_index = static_cast<int>(i);
		int del = get_delim_index(expression, st[op_index]);

		if (del != 0)
		{
			return new operation_t(operations[op_index],
				parse_subexpr(expression.substr(0, del)),
				parse_subexpr(expression.substr(del + 1, expression.length() - 1)));
		}
	}

	return parse_func(expression);
}

void expression_tree::parse_token(const std::string& str, int& a)
{
	if (str[a] == ')')
	{
		std::stack<bool> s;
		s.push(true);

		a--;

		while (!s.empty())
		{
			if (str[a] == '(') s.pop();
			else if (str[a] == ')') s.push(true);
			a--;
		}
	}

	while (a >= 0)
	{
		if (a == 0 || is_terminator(str[a]) || is_terminator(str[a - 1])) { a--;  break; }
		a--;
	}
}
//
bool expression_tree::is_terminator(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == ',');
}

operation_t* expression_tree::parse_func(std::string token)
{
	if (token.size() == 1 && token[0] == param_name)
		return new operation_t(operations[static_cast<int>(op_t::param)]);

	op_t type = get_func_type(token.substr(0, 3));

	if (token.size() < 6 && type == op_t::num)
	{
		double a = std::stod(token);
		return new operation_t([a](const operands&) { return a; });
	}

	int i = get_delim_index(token.substr(4, token.size() - 5), ',') + 4;

	if (i == 4)
		return new operation_t(operations[static_cast<int>(type)], parse_subexpr(token.substr(4, token.size() - 5)));
	else
		return new	operation_t(operations[static_cast<int>(type)],
			parse_subexpr(token.substr(4, i - 4)),
			parse_subexpr(token.substr(i + 1, token.size() - i - 2)));

}
//
op_t expression_tree::get_func_type(const std::string& token)
{
	if (token == "pow")	return op_t::pow;
	if (token == "sqr")	return op_t::sqr;
	if (token == "sin")	return op_t::sin;
	if (token == "cos")	return op_t::cos;
	if (token == "tan")	return op_t::tan;
	if (token == "cot")	return op_t::cot;
	if (token == "log")	return op_t::log;
	if (token == "lgn")	return op_t::lgn;
	return op_t::num;
}
int expression_tree::get_delim_index(const std::string& str, char delim)
{
	int i = str.length() - 1;

	do
		parse_token(str, i);
	while (str.at(i + 1) != delim && i >= 0);

	return i + 1;
}
//
double expression_tree::param(const operands&)
{
	return param_value;
}
//
double expression_tree::sum(const operands& n)
{

	return n.first->get() + n.second->get();
}
//
double expression_tree::dif(const operands& n)
{
	return n.first->get() - n.second->get();
}
//
double expression_tree::prod(const operands& n)
{
	return n.first->get() * n.second->get();
}
//
double expression_tree::div(const operands& n)
{
	return n.first->get() / n.second->get();
}
//
double expression_tree::pow(const operands& n)
{
	return std::pow(n.first->get(), n.second->get());
}
double expression_tree::sqr(const operands& n)
{
	return std::sqrt(n.first->get());
}
//
double expression_tree::sin(const operands& n)
{
	return std::sin(n.first->get());
}
//
double expression_tree::cos(const operands& n)
{
	return std::cos(n.first->get());
}
//
double expression_tree::tan(const operands& n)
{
	return std::tan(n.first->get());
}
//
double expression_tree::cot(const operands& n)
{
	return 1 / std::tan(n.first->get());
}
//
double expression_tree::log(const operands& n)
{
	return std::log(n.first->get()) / std::log(n.second->get());
}
//
double expression_tree::lgn(const operands& n)
{
	return std::log(n.first->get());
}
expression_tree::operation::operation(std::function<double(std::pair<operation*, operation*>)> f,
	expression_tree::operation* o1, expression_tree::operation* o2) : func{ f }, op{ o1, o2 }{}

double expression_tree::operation::get()
{
	return func(op);
}

expression_tree::op_type operator++(expression_tree::op_type& p)
{
	return p = op_t(static_cast<std::underlying_type<op_t>::type>(p) + 1);
}

