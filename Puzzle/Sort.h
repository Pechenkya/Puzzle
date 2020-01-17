#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <utility> //std::pair
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm> //C++ quick sort
#include <iterator>


//////////////////////////////// ASISTING FUNCTIONS //////////////////////////////////////
template<typename T>
void my_swap(T& elem1, T& elem2)
{
	T temp_data{};
	temp_data = elem1;
	elem1 = elem2;
	elem2 = temp_data;
}


template<typename T>
void shuffle(std::vector <T>& vec)
{
	for (size_t i{ 0 }; i < vec.size(); i++)
	{
		my_swap(vec.at(i), vec.at(rand() % (vec.size() - i) + i));
	}
}


int find_max_length(std::vector <std::string> & vec)
{
	int max_length{};
	for (auto& t : vec)
	{
		if (t.length() > max_length)
			max_length = t.length();
	}
	return max_length;
}


void fill_string_with_spaces(std::vector <std::string> & vec, int max_length)
{
	for (auto& t : vec)
	{
		while (t.length() < max_length)
		{
			t += ' ';
		}
	}
}

void remove_spaces(std::vector <std::string> & vec, int max_length)
{
	for (auto& t : vec)
	{
		for (int i = max_length; i > 0; i--)
		{
			if (t[i] == ' ')
				t.erase(t.end());
			else
				break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////// STRING OPERATOR OVERLOADING /////////////////////////////////
bool operator==(const std::string & s1, const std::string & s2)
{
	if (s1.length() != s2.length())
		return false;

	for (size_t i{ 0 }; i < s1.length(); i++)
	{
		if (tolower(s1[i]) != tolower(s2[i]))
			return false;
	}

	return true;
}

bool operator!=(const std::string & s1, const std::string & s2)
{
	return !(s1 == s2);
}


bool operator>(const std::string & s1, const std::string & s2)
{
	if (s1.length() <= s2.length())
	{
		for (size_t i{ 0 }; i < s2.length(); i++)
		{
			if (tolower(s1[i]) > tolower(s2[i]))
				return true;
			else if (tolower(s1[i]) != tolower(s2[i]))
				return false;
		}
	}
	else if (s1.length() > s2.length())
	{
		for (size_t i{ 0 }; i < s1.length(); i++)
		{
			if (tolower(s1[i]) > tolower(s2[i]))
				return true;
			else if (tolower(s1[i]) != tolower(s2[i]))
				return false;
		}
	}
	return false;
}

bool operator<(const std::string & s1, const std::string & s2)
{
	if (s1 == s2)
		return false;


	return !(s1 > s2);
}

bool operator<=(const std::string & s1, const std::string & s2)
{
	return !(s1 > s2);
}

bool operator>=(const std::string & s1, const std::string & s2)
{
	return !(s1 < s2);
}
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////// SELECTION SORT //////////////////////////////////////////
template<typename T>
void selection_sort(std::vector <T> & vec, size_t low, size_t high)
{

	for (size_t id{ low }; id <= high; id++)
	{
		size_t min_id = id;
		for (size_t idf{ id + 1 }; idf <= high; idf++)
		{
			if (vec.at(idf) < vec.at(min_id))
				min_id = idf;
		}

		if (min_id != id)
		{
			my_swap(vec.at(min_id), vec.at(id));
		}
	}

}
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////// INSERTION SORT //////////////////////////////////////////
template<typename T>
void insertion_sort(std::vector <T> & vec, int low, int high)
{

	for (int id{ low + 1 }; id <= high; id++)
	{
		T key = vec.at(id);
		int id2{ id - 1 };

		while (id2 >= low && key < vec.at(id2))
		{
			my_swap(vec.at(id2), vec.at(id2 + 1));
			id2--;
		}
	}

}
//////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////// TWO WAY PARTITION ////////////////////////////////////
template<typename T>
std::pair<size_t, size_t> two_way_partition(std::vector <T> & vec, size_t low, size_t high)
{
	T pivot = vec.at((low + high) / 2);
	int left_border = low - 1;
	int right_border = high + 1;

	while (left_border <= right_border)
	{
		do
		{
			left_border++;
		} while (vec.at(left_border) < pivot);

		do
		{
			right_border--;
		} while (vec.at(right_border) > pivot);

		if (left_border >= right_border) break;

		my_swap(vec.at(left_border), vec.at(right_border));
	}

	return std::make_pair(left_border, right_border);
}



template<typename T>
void two_way_quick_sort(std::vector <T> & vec, size_t low, size_t high)
{
	if (high <= low)
		return;

	if (high - low <= 7)
	{
		insertion_sort(vec, low, high);
		return;
	}

	std::pair<size_t, size_t> pivot = two_way_partition(vec, low, high);

	if (pivot.second > low)
		two_way_quick_sort(vec, low, pivot.second);

	if (pivot.first < high)
		two_way_quick_sort(vec, pivot.first, high);
}
//////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////// THREE WAY PARTITION ////////////////////////////////////
template<typename T>
std::pair<size_t, size_t> three_way_partition(std::vector <T> & vec, size_t low, size_t high)
{

	size_t left_border = low;
	size_t right_border = high;
	size_t copy_counter{ low };
	T pivot = vec.at((low + high) / 2);

	while (copy_counter <= right_border)
	{
		T comparing_element = vec.at(copy_counter);
		if (comparing_element < pivot)
		{
			my_swap(vec.at(left_border), vec.at(copy_counter));
			copy_counter++;
			left_border++;
		}
		else if (comparing_element > pivot)
		{
			my_swap(vec.at(copy_counter), vec.at(right_border));
			right_border--;
		}
		else
		{
			copy_counter++;
		}
	}

	return std::make_pair(left_border, right_border);

}

template<typename T>
void three_way_quick_sort(std::vector <T> & vec, size_t low, size_t high)
{
	if (high <= low)
		return;

	if (high - low <= 7)
	{
		insertion_sort(vec, low, high);
		return;
	}

	std::pair <size_t, size_t> borders = three_way_partition(vec, low, high);

	if (borders.first > low)
		three_way_quick_sort(vec, low, borders.first - 1);

	if (borders.second < high)
		three_way_quick_sort(vec, borders.second + 1, high);
}
//////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////// STRING SORT ///////////////////////////////////////////////
char char_pivot(std::vector <std::string> & vec, const size_t & begin, const int& end, const int& d)
{
	return vec[begin][d];
}

std::pair<size_t, size_t> string_partition(std::vector <std::string> & vec, int begin, int end, char& pivot, int d)
{

	int left_border = begin;
	int right_border = end;
	int copy_counter = begin + 1;

	while (copy_counter <= right_border)
	{
		char comp_char = tolower(vec[copy_counter][d]);
		if (comp_char < pivot)
		{
			my_swap(vec.at(left_border), vec.at(copy_counter));
			copy_counter++;
			left_border++;
		}
		else if (comp_char > pivot)
		{
			my_swap(vec.at(copy_counter), vec.at(right_border));
			right_border--;
		}
		else
		{
			copy_counter++;
		}
	}

	return std::make_pair(left_border, right_border);

}

void string_quick_sort(std::vector<std::string> & vec, int begin, int end, int d, const int& K)
{
	if (end <= begin || d >= K)
		return;

	char pivot = char_pivot(vec, begin, end, d);

	std::pair<int, int> borders = string_partition(vec, begin, end, pivot, d);

	string_quick_sort(vec, begin, borders.first - 1, d, K);
	if (pivot != ' ') string_quick_sort(vec, borders.first, borders.second, d + 1, K);
	string_quick_sort(vec, borders.second + 1, end, d, K);
}

void string_quick_sort(std::vector<std::string> & vec)
{
	int K = find_max_length(vec);
	fill_string_with_spaces(vec, K);
	string_quick_sort(vec, 0, vec.size() - 1, 0, K);
	remove_spaces(vec, K);
}
//////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////// MERGE SORT ////////////////////////////////////////////////
template<typename T>
void merge_sort(std::vector <T> & temp_vec, size_t low, size_t high, std::vector <T> & vec)
{
	if (high - low <= 7)
	{
		insertion_sort(vec, low, high);
		return;
	}

	size_t median{ (low + high) / 2 };

	// SWAPING VECTORS Method
	merge_sort(vec, low, median, temp_vec);
	merge_sort(vec, median + 1, high, temp_vec);

	//// USING ONE AS PRIMARY, ONE AS SECONDARY
	//merge_sort(temp_vec, low, median, vec);
	//merge_sort(temp_vec, median + 1, high, vec);


	//for (size_t i{ low }; i <= high; i++)
	//	temp_vec.at(i) = vec.at(i);
	//

	for (size_t l{ low }, r{ median + 1 }, i{ low }; l <= median || r <= high; i++)
	{
		if (l > median)
			vec.at(i) = temp_vec.at(r++);
		else if (r > high)
			vec.at(i) = temp_vec.at(l++);
		else if (temp_vec.at(l) <= temp_vec.at(r))
			vec.at(i) = temp_vec.at(l++);
		else
			vec.at(i) = temp_vec.at(r++);
	}


}


template<typename T>
void merge_sort(std::vector <T> & vec)
{
	// FOR SWAPING VECTORS Method
	std::vector<T> temp_vec = vec;

	// FOR USING ONE AS PRIMARY, ONE AS SECONDARY
	//std::vector<T> temp_vec(vec.size()); 

	merge_sort(temp_vec, 0, temp_vec.size() - 1, vec);
}
//////////////////////////////////////////////////////////////////////////////////////////



///////////////////// DUAL PIVOT SORT(By Vladimir Yaroslavski) ///////////////////////////
template<typename T>
void get_medians(std::vector<T> & vec, const int& begin, const int& end, const int& length)
{
	int third = length / 3;
	int median1 = begin + third;
	int median2 = end - third;

	if (median1 <= begin)
		median1 = begin + 1;

	if (median2 >= end)
		median2 = end - 1;

	if (vec.at(median1) < vec.at(median2))
	{
		my_swap(vec.at(median1), vec.at(begin));
		my_swap(vec.at(median2), vec.at(end));
	}
	else
	{
		my_swap(vec.at(median1), vec.at(end));
		my_swap(vec.at(median2), vec.at(begin));
	}
}



template<typename T>
int dual_pivot_partition(std::vector<T> & vec, const int& begin, const int& end, const T & pivot1, const T & pivot2, int& less, int& greater)
{
	for (int k = less; k <= greater; k++)
	{
		if (vec.at(k) < pivot1)
		{
			my_swap(vec.at(k), vec.at(less++));
		}
		else if (vec.at(k) > pivot2)
		{
			while (k < greater && vec.at(greater) > pivot2)
				greater--;

			my_swap(vec.at(k), vec.at(greater--));

			if (vec.at(k) < pivot1)
				my_swap(vec.at(k), vec.at(less++));
		}
	}

	my_swap(vec.at(less - 1), vec.at(begin));
	my_swap(vec.at(greater + 1), vec.at(end));

	return greater - less;
}


template<typename T>
void dual_pivot_quick_sort(std::vector<T> & vec, int begin, int end)
{
	int length = end - begin;

	//For range of 1-28 elements using insertion sort
	if (length < 27)
	{
		insertion_sort(vec, begin, end);
		return;
	}

	//Getting medians
	get_medians(vec, begin, end, length);

	//Pivots
	T pivot1 = vec.at(begin);
	T pivot2 = vec.at(end);

	//Pointers
	int less = begin + 1;
	int greater = end - 1;

	//Sorting
	int dist = dual_pivot_partition(vec, begin, end, pivot1, pivot2, less, greater);

	//Sorting subarrays
	dual_pivot_quick_sort(vec, begin, less - 2);
	dual_pivot_quick_sort(vec, greater + 2, end);

	//For equal elements
	if (dist > length - 13 && pivot1 != pivot2)
	{
		for (int k = less; k <= greater; k++)
		{
			if (vec.at(k) == pivot1)
			{
				my_swap(vec.at(k), vec.at(less++));
			}
			else if (vec.at(k) == pivot2)
			{
				my_swap(vec.at(k), vec.at(greater--));

				if (vec.at(k) == pivot1)
					my_swap(vec.at(k), vec.at(less++));
			}
		}
	}

	//Sorting subarray (P1 < --- < P2)
	if (pivot1 < pivot2)
		dual_pivot_quick_sort(vec, less, greater);
}


template<typename T>
void dual_pivot_quick_sort(std::vector<T> & vec)
{
	dual_pivot_quick_sort(vec, 0, vec.size() - 1);
}
//////////////////////////////////////////////////////////////////////////////////////////