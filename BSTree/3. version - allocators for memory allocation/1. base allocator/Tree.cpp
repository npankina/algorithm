﻿/*
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

#include "Tree.h"

//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>::~BSTree()
{
	Clear(root_); // Очищаем все узлы дерева при удалении объекта
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>::BSTree() : root_(nullptr), duplicate_elements_count_(0), total_elements_count_(0)
{}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>::BSTree(const BSTree &other) 
: root_(nullptr), duplicate_elements_count_(other.duplicate_elements_count_),
  total_elements_count_(other.total_elements_count_)
{ // copy ctor
	
	root_ = Copy(other);
	duplicate_elements_count_ = other.duplicate_elements_count_;
	total_elements_count_ = other.total_elements_count_;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>::BSTree(BSTree &&temp) noexcept 
: root_(temp.root_), duplicate_elements_count_(temp.duplicate_elements_count_), 
  total_elements_count_(temp.total_elements_count_)
{ // move ctor
	temp.root_ = nullptr;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>& BSTree<T, Alloc>::operator=(const BSTree &other) // copy assign
{ //  Очищает текущее дерево и копирует узлы из другого дерева.
	if (this != &other)
	{
		Clear(root_);
		root_ = Copy(other.root_);
	}

	return *this;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
BSTree<T, Alloc>& BSTree<T, Alloc>::operator=(BSTree &&temp) noexcept // move assign
{ // Очищает текущее дерево и перемещает данные из временного дерева, обнуляя временное дерево.
	if (this != &temp)
	{
		Clear(root_); //  Освобождаем старые ресурсы текущего объекта
		root_ = temp.root_; // Перемещаем указатель на корень дерева
		temp.root_ = nullptr; // Обнуляем указатель временного объекта, чтобы предотвратить удаление
	}

	return *this;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
void BSTree<T, Alloc>::Insert(T value)
{	// Проверка допустимости значения (например, если вы не хотите вставлять нулевые значения)
	if (value == T())
		throw std::invalid_argument("Значение не может быть пустым или нулевым");

	root_ = Insert(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
bool BSTree<T, Alloc>::Search(T value) const
{
	return Search(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
void BSTree<T, Alloc>::Remove(T value)
{ /* Обработка ошибок:
	 1. Обработка случая, когда узел не найден.
	 2. Обработка попыток удаления узлов с некорректными значениями.*/

	if (!Search(value))
		throw std::runtime_error("Элемент не найден в дереве");

	root_ = Remove(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
void BSTree<T, Alloc>::Print_in_order() const
{
	Inorder(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Node* BSTree<T, Alloc>::Insert(Node *node, const T &value)
{/* Алгоритм:
	1. Если корневой узел пуст, создается новый узел и устанавливается как корень.
	2. В противном случае начинается рекурсивная вставка. 
	2.1. Если новое значение меньше текущего узла, рекурсивно вызываем Insert для левого поддерева, иначе для правого.
	3. Операция продолжается до тех пор, пока не будет найдено подходящее пустое место для нового узла.
  */
	
	try
	{
		if (node == nullptr) // Выделяем память для нового узла с использованием аллокатора
		{
			Node * new_node = alloc_.Allocate(1);
			alloc_.Construct(new_node, Node(value));
			return new_node;
		}

		if (value < node->data_)
			node->left_ = Insert(node->left_, value);
		else if (value > node->data_)
			node->right_ = Insert(node->right_, value);
		else // Если значение уже существует, увеличиваем счётчик дубликатов
			node->count_++;
	}
	catch (const std::bad_alloc &err)
	{
		std::cerr << "Ошибка выделения памяти: " << err.what() << std::endl;
		throw;
	}

	return node;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
bool BSTree<T, Alloc>::Search(Node *node, T value) const
{ /* Алгоритм:
	1. Если узел пустой или значение найдено, возвращаем true или false соответственно.
	2. Если искомое значение меньше текущего узла, продолжаем поиск в левом поддереве, если больше — в правом.
  */
	if (node == nullptr)
		return false;

	if (node->data_ == value)
		return true;

	if (value < node->data_)
		return Search(node->left_, value);
	else
		return Search(node->right_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Node* BSTree<T, Alloc>::Remove(typename BSTree<T, Alloc>::Node* node, T value)
{ /* Алгоритм:
	1. Если узел пустой, элемент не найден.
	2. Если значение меньше текущего узла, продолжаем удаление в левом поддереве. Если больше — в правом.
	3. Если значение совпадает с текущим узлом, узел удаляется.
	4. Узел с нулем потомков просто удаляется.
	5. Узел с одним потомком заменяется этим потомком.
	6. Узел с двумя потомками заменяется минимальным элементом правого поддерева.
  */
	if (node == nullptr)
		return nullptr;

	if (value < node->data_)
		node->left_ = Remove(node->left_, value); // Вызов Remove рекурсивно продолжается для левого поддерева
	else if (value > node->data_)
		node->right_ = Remove(node->right_, value); // --||-- для правого поддерева
	else // узел найден, и начинается процесс удаления
	{
		if (node->count_ > 1)  // Если есть дубликаты, уменьшаем счётчик и не удаляем узел
		{
			--node->count_;
			--total_elements_count_; // Уменьшаем общее количество элементов
			return node;
		}

		// Стандартная процедура удаления узла
		--total_elements_count_;
		Node *temp = nullptr;
		if (node->left_ == nullptr)
			temp  = node->right_; // у узла нет левого потомка, просто заменяем этот узел его правым потомком (узел с одним потомком).
		else if (node->right_ == nullptr)
			temp = node->left_;
		delete node;
		return temp; // возвращаем указатель на потомка, который теперь займет место удалённого узла.
	
		// Узел с двумя потомками: находим минимальный элемент в правом поддереве
		Node *min_node = Find_min(node->right_);
		node->data_ = min_node->data_;
		node->count_ = min_node->count_; // Переносим счётчик дубликатов
		node->right_ = Remove(node->right_, min_node->data_); // рекурсивно удаляем минимальный узел, так как его значение уже скопировано в текущий узел.
	}

	return node;
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Node* BSTree<T, Alloc>::Find_min(Node *node) const
{ /* Алгоритм:
	1. Если у текущего узла есть левый потомок, продолжаем двигаться влево.
	2. Если у текущего узла нет левого потомка, это и есть минимальный узел, так как все меньшие значения находятся слева.
  */
	while (node->left_ != nullptr) // Пока есть левый потомок, идем в левое поддерево
		node = node->left_;
	return node;  // Возвращаем узел с минимальным значением
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Node* BSTree<T, Alloc>::Copy(Node *node)
{/* Алгоритм:
	1. Рекурсивно создаем копию дерева, начиная с корня.
	2. Создаем новые узлы, рекурсивно копируя левое и правое поддеревья. */

	if (node == nullptr)
		return nullptr; // Если узел пустой, возвращаем nullptr

	Node *new_node = nullptr;

	try
	{
		new_node = new Node(node->data_);  // Создаем новый узел

		// Копируем левое и правое поддеревья рекурсивно
		new_node->left_ = Copy(node->left_);
		new_node->right_ = Copy(node->right_);

		new_node->count_ = node->count_; // Переносим счётчик дубликатов
	}
	catch (const std::bad_alloc &err)
	{
		std::cerr << "Ошибка выделения памяти при копировании узла: " << err.what() << std::endl;
		Clear(new_node);
		
		throw;
	}

	return new_node; // Возвращаем указатель на новый узел
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
void BSTree<T, Alloc>::Clear(Node *node)
{
	if (node != nullptr)
	{
		Clear(node->left_);
		Clear(node->right_);
		
		// Уничтожаем узел и освобождаем память
		alloc_.Destroy(node);
		alloc_.Deallocate(node, 1);
	}
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
void BSTree<T, Alloc>::Inorder(Node *node) const
{ /* Алгоритм:
	1. Рекурсивно обходим левое поддерево.
	2. Печатаем значение текущего узла.
	3. Рекурсивно обходим правое поддерево.
  */
	if (node == nullptr)
		return;

	Inorder(node->left_);
	std::cout << node->data_ << " ";
	Inorder(node->rigth_);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Iterator BSTree<T, Alloc>::begin() const
{
	return Iterator(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Iterator BSTree<T, Alloc>::end() const
{
	return Iterator(nullptr);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Const_Iterator BSTree<T, Alloc>::сbegin() const
{
	return Const_Iterator(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T, typename Alloc>
typename BSTree<T, Alloc>::Const_Iterator BSTree<T, Alloc>::сend() const
{
	return Const_Iterator(nullptr);
}
//----------------------------------------------------------------------------------------------------