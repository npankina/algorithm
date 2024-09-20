#pragma once
#include <stack>
#include <iterator>
#include <iostream>


template <typename T>
class BSTree
{
private:
	class Node
	{
	public:
		T data_;
		Node *left_;
		Node *right_;
		size_t count_;  // Счётчик дубликатов

		Node(T value) : data_(value), left_(nullptr), right_(nullptr), count_(0) {}
	};
public:
	template <typename Pointer, typename Reference>
	class Tree_Iterator
	{
	public:
		friend class BSTree; // Итератору нужен доступ к внутренним членам дерева

		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = Pointer;
		using reference = Reference;
		using iterator_category = std::bidirectional_iterator_tag;

		Tree_Iterator() : current_(nullptr) {}
		Tree_Iterator(Node *node) : Tree_Iterator()
		{
			// Инициализируем итератор для обхода дерева
			// Начнем с левого потомка корня
			Push_left(node);

			// Устанавливаем текущий узел как вершину стека, если стек не пуст
			if (!Node_Stack.empty())
			{
				current_ = Node_Stack.top();
				Node_Stack.pop();
			}
		}

		reference operator*() const
		{ // Возвращает ссылку на данные текущего узла. Это позволяет получить значение элемента, на который указывает итератор.
			return current_->data_;
		}

		pointer operator->() const
		{ // Возвращает указатель на данные текущего узла. Это позволяет использовать синтаксис -> для доступа к членам данных.
			return &(current_->data_);
		}

		Tree_Iterator& operator++()
		{ /* Алгоритм:
			1. Переходит к следующему узлу в in-order обходе.
			2. Если у текущего узла есть правый потомок, добавляет все его левые потомки в стек. 
			2.1. Затем возвращает следующий узел из стека как текущий.
			3. Если стек пуст, итерация завершена.
		  */

			if (current_ == nullptr)
				throw std::out_of_range("Итерация за пределами дерева");

			if (current_->right_ != nullptr)
				Push_left(current_->right_); // Добавляем все левые узлы правого поддерева

			if (!Node_Stack.empty())
			{
				current_ = Node_Stack.top(); // Берем следующий узел из стека
				Node_Stack.pop();
			}
			else
				current_ = nullptr; // Итерация завершена

			return *this;
		}

		Tree_Iterator operator++(int)
		{ /* Алгоритм:
			1. Постфиксный инкремент требует создания временного итератора, который содержит текущее состояние, 
				чтобы вернуть его до изменения итератора.
			2. Выполняет инкремент аналогично префиксному инкременту, но возвращает временную копию исходного итератора.
		  */
			Tree_Iterator temp = *this; // Создаем копию текущего состояния
			++(*this); // Применяем префиксный инкремент
			return temp; // Возвращаем копию
		}

		bool operator==(const Tree_Iterator &other) const
		{ // Оператор == проверяет, указывают ли два итератора на один и тот же узел (если их текущие узлы совпадают).
			return current_ == other.current_;
		}

		bool operator!=(const Tree_Iterator &other) const
		{ // Оператор != является противоположным оператором, проверяет, что итераторы указывают на разные узлы.
			return !(*this == other);  // Возвращаем противоположное значение
		}

	protected:
		void Push_left(Node *node)
		{ // Рекурсивно добавляет все левые узлы, начиная с переданного узла, в стек. 
		  // Это позволяет подготовить стек для in-order обхода дерева, начиная с самого левого (наименьшего) элемента.
			while (node != nullptr)
			{
				Node_Stack.push(node);
				node = node->left_;
			}
		}

		std::stack<typename BSTree<T>::Node *> Node_Stack;
		typename BSTree<T>::Node *current_;
	};

	class Iterator : public Tree_Iterator<T*, T&>
	{
	public:
		using Base = Tree_Iterator<T*, T&>;
		using Base::Base; // наследуем конструкторы
	};

	class Const_Iterator : public Tree_Iterator<const T*, const T&>
	{
	public:
		using Base = Tree_Iterator<const T*, const T&>;
		using Base::Base; // наследуем конструкторы
	};

public:
	~BSTree();
	BSTree();
	BSTree(const BSTree &other); // copy ctor
	BSTree(BSTree &&temp) noexcept; // move ctor
	BSTree& operator=(const BSTree &other); // copy assign
	BSTree& operator=(BSTree &&temp) noexcept; // move assign

	void Insert(T value);
	bool Search(T value) const;
	void Remove(T value);
	void Print_in_order() const;
	
	Iterator begin() const;
	Iterator end() const;
	Const_Iterator сbegin() const;
	Const_Iterator сend() const;

private:
	Node* Insert(Node *node, T value, size_t depth = 0);
	Node* Find_min(Node *node) const;
	Node* Copy(Node *node);
	Node* Remove(Node *node, T value);
	bool Search(Node *node, T value) const;
	void Clear(Node *node);
	void Inorder(Node *node) const;

	Node *root_;
	size_t duplicate_elements_count_;
	size_t total_elements_count_;

	const size_t MAX_DEPTH = 1000;
};