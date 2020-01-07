#pragma once
#pragma once
#include <vector>
#include <iterator>

template<typename T>
class Heap
{
private:
	std::vector<T*> heap;
	int size;

	bool is_empty();
	void swim(int k);
	void sink(int k);

	void exch(int a, int b);
	bool less(int a, int b);

public:
	struct HeapItr;

	void insert(T elem);

	template<typename... Args>
	void emplace(Args&&... args);

	std::vector <T> return_heap();
	std::vector <T> return_sorted_heap();
	std::vector <T> remove_elements();

	template<typename Container>
	void fill(Container list);

	T remove_min();

	HeapItr begin();
	HeapItr end();
	const HeapItr const_begin();
	const HeapItr const_end();

	Heap();
	~Heap();
};

template<typename T>
struct Heap<T>::HeapItr
{
public:
	T** this_node = nullptr;
	HeapItr() = default;
	HeapItr(T* &new_node);

	HeapItr& operator++();
	HeapItr& operator++(int);
	HeapItr& operator--();
	HeapItr& operator--(int);

	HeapItr operator+(int num);

	bool operator==(const HeapItr& obj);
	bool operator!=(const HeapItr& obj);
	T& operator*();
};

template<typename T>
Heap<T>::HeapItr::HeapItr(T* &new_node)
{
	this_node = &new_node;
}

template<typename T>
typename Heap<T>::HeapItr& Heap<T>::HeapItr::operator++()
{
	this->this_node = ++this->this_node;
	return *this;
}

template<typename T>
typename Heap<T>::HeapItr& Heap<T>::HeapItr::operator++(int)
{
	this->this_node = ++this->this_node;
	return *this;
}

template<typename T>
typename Heap<T>::HeapItr& Heap<T>::HeapItr::operator--()
{
	this->this_node = --this->this_node;
	return *this;
}

template<typename T>
typename Heap<T>::HeapItr& Heap<T>::HeapItr::operator--(int)
{
	this->this_node = --this->this_node;
	return *this;
}

template<typename T>
typename Heap<T>::HeapItr Heap<T>::HeapItr::operator+(int num)
{
	return HeapItr(*(this->this_node + num));
}

template<typename T>
bool Heap<T>::HeapItr::operator==(const Heap<T>::HeapItr& obj)
{
	return *this->this_node == *obj.this_node;
}

template<typename T>
bool Heap<T>::HeapItr::operator!=(const Heap<T>::HeapItr& obj)
{
	return !(*this == obj);
}

template<typename T>
T& Heap<T>::HeapItr::operator*()
{
	return *(*this_node);
}

template<typename T>
typename Heap<T>::HeapItr Heap<T>::Heap::begin()
{
	return Heap<T>::HeapItr(this->heap[1]);
}

template<typename T>
typename Heap<T>::HeapItr Heap<T>::end()
{
	Heap<T>::HeapItr temp_itr = Heap<T>::HeapItr(this->heap[size]);
	//++temp_itr;
	return temp_itr;
}

template<typename T>
const typename Heap<T>::HeapItr Heap<T>::const_begin()
{
	return Heap<T>::HeapItr(this->heap[1]);
}

template<typename T>
const typename Heap<T>::HeapItr Heap<T>::const_end()
{
	Heap<T>::HeapItr temp_itr = Heap<T>::HeapItr(this->heap[size]);
	++temp_itr;
	return temp_itr;
}

template<typename T>
bool Heap<T>::is_empty()
{
	return this->heap.size() == 1;
}

template<typename T>
void Heap<T>::swim(int k)
{
	while (k > 1 && !less(k / 2, k))
	{
		exch(k, k / 2);
		k = k / 2;
	}
}

template<typename T>
template<typename Container>
void Heap<T>::fill(Container list)
{
	for (auto t : list)
	{
		this->insert(t);
	}
}

template<typename T>
void Heap<T>::sink(int k)
{
	while (k * 2 <= size)
	{
		int j = k * 2;
		if (j < size && !less(j, j + 1))
			j++;
		if (less(k, j)) break;
		exch(k, j);
		k = j;
	}
}

template<typename T>
void Heap<T>::exch(int a, int b)
{
	T* temp = this->heap[a];
	this->heap[a] = this->heap[b];
	this->heap[b] = temp;
}

template<typename T>
bool Heap<T>::less(int a, int b)
{
	return *(this->heap[a]) < *(this->heap[b]);
}

template<typename T>
void Heap<T>::insert(T elem)
{
	if (this->size < this->heap.size() - 1)
	{
		heap[++size] = new T(elem);
		swim(size);
	}
	else
	{
		heap.push_back(new T(elem));
		++size;
		swim(size);
	}

}

template<typename T>
template<typename... Args>
void Heap<T>::emplace(Args&&... args)
{
	if (this->size < this->heap.size() - 1)
	{
		heap[++size] = new T(std::forward<Args>(args)...);
		swim(size);
	}
	else
	{
		heap.push_back(new T(std::forward<Args>(args)...));
		++size;
		swim(size);
	}

}

template<typename T>
std::vector<T> Heap<T>::return_heap()
{
	std::vector<T> temp;
	for (int i = 1; i <= this->size; i++)
	{
		temp.push_back(*(this->heap[i]));
	}
	return temp;
}

template<typename T>
std::vector <T> Heap<T>::return_sorted_heap()
{
	std::vector <T> temp;
	for (int i = 1; i <= this->size;)
	{
		temp.push_back(this->remove_max());
	}

	for (auto t : temp)
	{
		this->insert(t);
	}

	return temp;
}

template<typename T>
std::vector<T> Heap<T>::remove_elements()
{
	std::vector <T> temp;
	for (int i = 1; i <= this->size;)
	{
		temp.push_back(this->remove_max());
	}

	return temp;
}


template<typename T>
T Heap<T>::remove_min()
{
	T max = *(heap[1]);
	exch(1, size--);
	sink(1);
	delete heap[size + 1];
	heap[size + 1] = nullptr;
	heap.pop_back();
	return max;
}

template<typename T>
Heap<T>::Heap()
{
	heap.push_back(nullptr);
	size = 0;
}

template<typename T>
Heap<T>::~Heap()
{
	for (auto t : heap)
	{
		delete t;
	}
}
