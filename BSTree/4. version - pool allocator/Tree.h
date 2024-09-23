/*
 * MIT License
 * Copyright (c) 2024 Nathalie Pankin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <stack>
#include <iterator>
#include "BSTree_Allocator.h"


template <typename T, typename Alloc = std::allocator<T>>
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
				throw std::out_of_range("Iteration beyond the tree.");

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

	void Insert(const T &value);
	bool Search(T value) const;
	void Remove(T value);
	void Print_in_order() const;

	Iterator begin() const;
	Iterator end() const;
	Const_Iterator сbegin() const;
	Const_Iterator сend() const;

private:
	Node* Insert(Node *node, const T &value);
	Node* Find_min(Node *node) const;
	Node* Copy(Node *node);
	Node* Remove(Node *node, T value);
	bool Search(Node *node, T value) const;
	void Clear(Node *node);
	void Inorder(Node *node) const;

	using Allocator_type = typename Alloc::template rebind<Node>::other;

	Node *root_;
	Allocator_type alloc_;
	size_t duplicate_elements_count_;
	size_t total_elements_count_;
};