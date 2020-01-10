#pragma once
#pragma once
#include <vector>
#include <iterator>

template<typename T>
class PQueue
{
private:
	std::vector<T*> heap;
	std::vector<T*> trash;
	int size;

	bool is_empty();
	void swim(int k);
	void sink(int k);

	void exch(int a, int b);
	bool less(int a, int b);

public:
	struct HeapItr;

	void insert(const T& elem);
	void insert(T* elem);


	template<typename... Args>
	void emplace(Args&&... args);

	std::vector <T> return_heap();
	std::vector <T> return_sorted_heap();
	std::vector <T> remove_elements();

	template<typename Container>
	void fill(Container list);

	T* remove_min();

	HeapItr begin();
	HeapItr end();
	const HeapItr const_begin();
	const HeapItr const_end();

	PQueue();
	~PQueue();
};

template<typename T>
struct PQueue<T>::HeapItr
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
PQueue<T>::HeapItr::HeapItr(T* &new_node)
{
	this_node = &new_node;
}

template<typename T>
typename PQueue<T>::HeapItr& PQueue<T>::HeapItr::operator++()
{
	this->this_node = ++this->this_node;
	return *this;
}

template<typename T>
typename PQueue<T>::HeapItr& PQueue<T>::HeapItr::operator++(int)
{
	this->this_node = ++this->this_node;
	return *this;
}

template<typename T>
typename PQueue<T>::HeapItr& PQueue<T>::HeapItr::operator--()
{
	this->this_node = --this->this_node;
	return *this;
}

template<typename T>
typename PQueue<T>::HeapItr& PQueue<T>::HeapItr::operator--(int)
{
	this->this_node = --this->this_node;
	return *this;
}

template<typename T>
typename PQueue<T>::HeapItr PQueue<T>::HeapItr::operator+(int num)
{
	return HeapItr(*(this->this_node + num));
}

template<typename T>
bool PQueue<T>::HeapItr::operator==(const PQueue<T>::HeapItr& obj)
{
	return *this->this_node == *obj.this_node;
}

template<typename T>
bool PQueue<T>::HeapItr::operator!=(const PQueue<T>::HeapItr& obj)
{
	return !(*this == obj);
}

template<typename T>
T& PQueue<T>::HeapItr::operator*()
{
	return *(*this_node);
}

template<typename T>
typename PQueue<T>::HeapItr PQueue<T>::PQueue::begin()
{
	return PQueue<T>::HeapItr(this->heap[1]);
}

template<typename T>
typename PQueue<T>::HeapItr PQueue<T>::end()
{
	PQueue<T>::HeapItr temp_itr = PQueue<T>::HeapItr(this->heap[size]);
	//++temp_itr;
	return temp_itr;
}

template<typename T>
const typename PQueue<T>::HeapItr PQueue<T>::const_begin()
{
	return PQueue<T>::HeapItr(this->heap[1]);
}

template<typename T>
const typename PQueue<T>::HeapItr PQueue<T>::const_end()
{
	PQueue<T>::HeapItr temp_itr = PQueue<T>::HeapItr(this->heap[size]);
	++temp_itr;
	return temp_itr;
}

template<typename T>
bool PQueue<T>::is_empty()
{
	return this->heap.size() == 1;
}

template<typename T>
void PQueue<T>::swim(int k)
{
	while (k > 1 && !less(k / 2, k))
	{
		exch(k, k / 2);
		k = k / 2;
	}
}

template<typename T>
template<typename Container>
void PQueue<T>::fill(Container list)
{
	for (auto t : list)
	{
		this->insert(t);
	}
}

template<typename T>
void PQueue<T>::sink(int k)
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
void PQueue<T>::exch(int a, int b)
{
	T* temp = this->heap[a];
	this->heap[a] = this->heap[b];
	this->heap[b] = temp;
}

template<typename T>
bool PQueue<T>::less(int a, int b)
{
	return *(this->heap[a]) < *(this->heap[b]);
}

template<typename T>
void PQueue<T>::insert(const T& elem)
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
void PQueue<T>::insert(T* elem)
{
	if (this->size < this->heap.size() - 1)
	{
		heap[++size] = elem;
		swim(size);
	}
	else
	{
		heap.push_back(elem);
		++size;
		swim(size);
	}

}

template<typename T>
template<typename... Args>
void PQueue<T>::emplace(Args&&... args)
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
std::vector<T> PQueue<T>::return_heap()
{
	std::vector<T> temp;
	for (int i = 1; i <= this->size; i++)
	{
		temp.push_back(*(this->heap[i]));
	}
	return temp;
}

template<typename T>
std::vector <T> PQueue<T>::return_sorted_heap()
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
std::vector<T> PQueue<T>::remove_elements()
{
	std::vector <T> temp;
	for (int i = 1; i <= this->size;)
	{
		temp.push_back(this->remove_max());
	}

	return temp;
}


template<typename T>
T* PQueue<T>::remove_min()
{
	T* min = heap[1];
	exch(1, size--);
	sink(1);
	heap.pop_back();
	trash.push_back(min);
	return min;
}

template<typename T>
PQueue<T>::PQueue()
{
	heap.push_back(nullptr);
	size = 0;
}

template<typename T>
PQueue<T>::~PQueue()
{
	for (auto t : heap)
	{
		delete t;
	}

	for (auto t : trash)
	{
		delete t;
	}
}
