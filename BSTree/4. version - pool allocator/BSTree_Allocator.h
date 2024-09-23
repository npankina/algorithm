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
#include <iostream>
#include <memory> // Для std::unique_ptr
#include <vector>
#include <list>
#include <stdexcept>  // Для обработки исключений
#include <new>        // Для std::bad_alloc


template <typename T>
class Memory_Pool
{
private:
	struct Block
	{
		T data;
		Block *next;
	};

	Block* free_list_; // Список свободных блоков
	std::vector<std::unique_ptr<Block[]>> blocks_; // Храним массивы выделенных блоков
	size_t block_size_;  // Размер каждого блока
	size_t block_count_; // Количество блоков, выделенных за один раз

public:
	Memory_Pool(size_t block_count = 64) : free_list_(nullptr), block_size_(sizeof(T)), block_count_(block_count) 
	{
		if (block_count == 0)
			throw std::invalid_argument("Block count must be greater than 0.");
	}

	~Memory_Pool() {} // Память автоматически освобождается через умные указатели

	T* Allocate()
	{ // Выделение памяти для одного элемента
		if (free_list_ == nullptr)
			Allocate_Block();
		Block* block = free_list_;
		free_list_ = free_list_->next;
		return reinterpret_cast<T*>(block); // приведение типа указателя Block* к типу указателя T*.
	}

	void Deallocate(T* ptr)
	{ // Освобождение памяти
		if (ptr == nullptr)
			throw std::invalid_argument("Cannot deallocate a null pointer.");

		Block* block = reinterpret_cast<Block*>(ptr);
		block->next = free_list_;
		free_list_ = block;
	}

private:
	void Allocate_Block()
	{ // Выделение нового блока памяти
		try
		{
			std::unique_ptr<Block[]> new_block = std::make_unique<Block[]>(block_count_);

			for (size_t i = 0; i < block_count_ - 1; ++i)
				new_block[i].next = &new_block[i + 1];

			new_block[block_count_ - 1].next = free_list_;
			free_list_ = &new_block[0];
			blocks_.emplace_back(std::move(new_block)); // Храним блоки, чтобы управлять временем жизни
		}
		catch (const std::bad_alloc &err)
		{
			throw std::runtime_error("Memory_Pool: failed to allocate memory block.");
		}
	}
};
//----------------------------------------------------------------------------------------------------
template <typename T>
class BSTree_Allocator // Класс аллокатора, использующий пул памяти
{
public:
	using value_type = T;
	
	BSTree_Allocator() : pool_(std::make_shared<Memory_Pool<T>>()) {}
	
	template <typename U> // Шаблонный конструктор для копирования аллокатора
	BSTree_Allocator(const BSTree_Allocator<U> &other) noexcept : pool_(other.pool_) {} 

	
	T* Allocate(std::size_t n) 
	{ // Метод для выделения памяти
		if (n != 1)
			throw std::bad_alloc();

		return pool_->Allocate();
	}

	void Deallocate(T* p, std::size_t n) noexcept 
	{ // Метод для освобождения памяти
		if (n == 1)
			pool_->Deallocate(p);
	}

	template <typename U, typename... Args>
	void Construct(U *p, Args&&... args)
	{  // Метод для конструирования объекта в выделенной памяти
		try
		{
			new (p) U(std::forward<Args>(args)...); // placement new для конструктора
		}
		catch (const std::exception &err) {
			throw std::runtime_error("BSTree_Allocator: failed to construct object.");
		}
	}

	template <typename U>
	void Destroy(U *p) // Метод для разрушения объекта
	{
		try
		{
			p->~U();
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("BSTree_Allocator: failed to destroy object.");
		}
	}

	
	template <typename U>
	struct rebind 
	{ // Шаблон для создания аллокатора для другого типа
		using other = BSTree_Allocator<U>;
	};

private:
	std::shared_ptr<Memory_Pool<T>> pool_; // Совместное использование пула между аллокаторами
};

template <typename T, typename U>
bool operator==(const BSTree_Allocator<T>&, const BSTree_Allocator<U>&)
{
	return true;
}

template <typename T, typename U>
bool operator!=(const BSTree_Allocator<T>&, const BSTree_Allocator<U>&)
{
	return false;
}