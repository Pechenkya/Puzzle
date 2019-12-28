#pragma once
#include <vector>
#include <string>
#include <functional>

class expression_tree
{
public:
	enum class op_type
	{
		param, sum, dif, prod, div, pow, sqr, sin, cos, tan, cot, log, lgn, num
	};



	struct operation
	{
		operation(std::function<double(std::pair<operation*, operation*>)>, operation* = nullptr, operation* = nullptr);
		std::pair<operation*, operation*> op;
		std::function<double(const  std::pair<operation*, operation*>)> func;
		double get();
	};

private:
	using operands = std::pair<operation*, operation*>;

	static double param(const operands&);
	static double sum(const operands&);
	static double dif(const operands&);
	static double prod(const operands&);
	static double div(const operands&);
	static double pow(const operands&);
	static double sqr(const operands&);
	static double sin(const operands&);
	static double cos(const operands&);
	static double tan(const operands&);
	static double cot(const operands&);
	static double log(const operands&);
	static double lgn(const operands&);

public:
	expression_tree(std::string input, char param = 'x');
	double calculate(double);

private:
	// parse funcs
	operation* parse_subexpr(std::string expression);
	void parse_token(const std::string& str, int& a);

	operation* parse_func(std::string token);

	// helper funcs
	bool is_terminator(char);
	op_type get_func_type(const std::string& token);
	int get_delim_index(const std::string&, char);

	// locals
	operation* root;
	char param_name;

	// statics
	static char st[6];
	static double param_value;
	static std::function<double(const std::pair<operation*, operation*>&)> operations[13];
};

expression_tree::op_type operator++(expression_tree::op_type& p);



