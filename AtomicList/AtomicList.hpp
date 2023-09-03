#pragma once
#include <atomic>
#include <memory>
#include <initializer_list>
#include <iostream>
#include <syncstream>
#include <string>
#include <algorithm>


template<typename Type>
class AtomicList
{
	struct Node
	{
		Type data;
		Node* prev;
		Node* next;
	};

	friend std::ostream& operator << (std::ostream& cout, const AtomicList& list)
	{
		if (list.m_size == 0)
		{
			return cout;
		}

		std::osyncstream sync_cout(cout);

		for ( Node* iter_node = list.m_head.load(); iter_node != nullptr; iter_node = iter_node->next)
		{
			sync_cout << std::to_string( iter_node->data ) + "\n";
		}

		return cout;
	}
	friend class Testing;
public:
	AtomicList()
		:
		m_size(0),
		m_head(),
		m_tail()
	{

	}
	AtomicList(const std::initializer_list<Type>& values);
	~AtomicList();
	inline size_t size() const
	{
		return m_size;
	}
	inline bool empty() const
	{
		return m_size <= 0;
	}
	inline Type& front()
	{
		Node* head = m_head.load();
		if (head == nullptr)
		{
			return {};
		}

		return head->data;
	}

	inline const Type& front() const
	{
		Node* head = m_head.load();
		if (head == nullptr)
		{
			return {};
		}

		return head->data;
	}

	inline Type& back()
	{
		Node* tail = m_tail.load();
		if (tail == nullptr)
		{
			return {};
		}

		return tail->data;
	}

	inline const Type& back() const
	{
		Node* tail = m_tail.load();
		if (tail == nullptr)
		{
			return {};
		}

		return tail->data;
	}
	void push_front(const Type& value);
	void push_back(const Type& value);
	void pop_front();
	void pop_back();


private:
	std::atomic_size_t m_size;
	std::atomic<Node*> m_head;
	std::atomic<Node*> m_tail;
};

template<typename Type>
inline AtomicList<Type>::AtomicList(const std::initializer_list<Type>& values)
{
	if (values.size() == 0)
	{
		return;
	}

	m_size = values.size();
	auto begin = values.begin();
	auto end = values.end();

	Node* node = new Node{ *begin++, nullptr, nullptr };
	m_head = node;
	m_tail = node;

	for (; begin != end; ++begin)
	{
		Node* old_tail = m_tail.load();
		old_tail->next = new Node{ *begin, old_tail, nullptr };
		m_tail = old_tail->next;
	}
}

template<typename Type>
inline AtomicList<Type>::~AtomicList()
{
	if (m_size == 0)
	{
		return;
	}

	Node* iter_node = m_head.load();

	while (iter_node != nullptr)
	{
		Node* remove_node = iter_node;
		iter_node = iter_node->next;

		delete remove_node;
	}
}

template<typename Type>
inline void AtomicList<Type>::push_front(const Type& value)
{
	Node* current_head = m_head.load();

	if (m_size == 0)
	{
		Node* new_node = new Node{ value, nullptr, nullptr };
		while (!m_head.compare_exchange_strong(current_head, new_node));

		m_tail = m_head.load();
		++m_size;
		return;
	}

	Node* new_head = new Node{ value, nullptr, current_head };
	current_head->prev = new_head;

	while (!m_head.compare_exchange_strong(current_head, new_head));

	++m_size;
}

template<typename Type>
inline void AtomicList<Type>::push_back(const Type& value)
{
	Node* current_tail = m_tail.load();

	if (m_size == 0)
	{
		Node* new_node = new Node{ value, nullptr, nullptr };
		while (!m_head.compare_exchange_strong(current_tail, new_node));

		m_head = m_tail.load();
		++m_size;

		return;
	}

	Node* new_tail = new Node{ value, current_tail, nullptr };
	current_tail->next = new_tail;

	while (!m_tail.compare_exchange_strong(current_tail, new_tail));

	++m_size;
}

template<typename Type>
inline void AtomicList<Type>::pop_front()
{
	if (m_size == 0)
	{
		return;
	}

	Node* remove_head = m_head.load();
	Node* new_head = remove_head->next;

	if (new_head != nullptr)
	{
		new_head->prev = nullptr;
	}

	while (!m_head.compare_exchange_strong(remove_head, new_head));

	--m_size;
	delete remove_head;
}

template<typename Type>
inline void AtomicList<Type>::pop_back()
{
	if (m_size == 0)
	{
		return;
	}

	Node* remove_tail = m_tail.load();
	Node* new_tail = remove_tail->prev;

	if (new_tail != nullptr)
	{
		new_tail->next = nullptr;
	}

	while (!m_tail.compare_exchange_strong(remove_tail, new_tail));

	--m_size;
	delete remove_tail;
}

class Testing
{
public:
	template<typename Type>
	static bool check_if_any(const AtomicList<Type>& test_list, const std::initializer_list<Type>& expected)
	{
		if (test_list.m_size != expected.size())
		{
			return false;
		}

		bool check = std::ranges::any_of(expected, [&test_list](const Type& exp_value)
		{
			for (auto iter_node = test_list.m_head.load(); iter_node != nullptr; iter_node = iter_node->next)
			{
				if (iter_node->data == exp_value)
				{
					return true;
				}
			}

			return false;
		});

		return check;
	}
};