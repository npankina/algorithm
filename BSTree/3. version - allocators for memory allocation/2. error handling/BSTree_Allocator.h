#pragma once
#include <iostream>
#include <memory>
#include <limits>
#include <new>  // Для std::bad_alloc

template <typename T>
class BSTree_Allocator
{
public:
	using value_type = T;
	
	BSTree_Allocator() noexcept {}
	
	template <typename U> // Шаблонный конструктор для копирования аллокатора
	BSTree_Allocator(const BSTree_Allocator<U> &) noexcept {} 

	
	T* Allocate(std::size_t n) // Метод для выделения памяти
	{
		if (n == 0)
			return nullptr;

		if (n > std::size_t(-1) / sizeof(T)) // Проверка на переполнение
			throw std::bad_alloc(); // Бросаем исключение, если размер слишком большой

		T *ptr = nullptr;
		try
		{
			ptr = static_cast<T*>(::operator new(n * sizeof(T)));  // Выделение памяти 
		}
		catch (const std::bad_alloc &err)
		{
			std::cerr << "Ошибка выделения памяти: " << err.what() << std::endl;
			throw;
		}

		return ptr;
	}

	void Deallocate(T* p, std::size_t n) noexcept // Метод для освобождения памяти
	{
		::operator delete(p);
	}

	template <typename U, typename... Args>
	void Construct(U *p, Args&&... args) // Метод для конструирования объекта в выделенной памяти
	{
		try
		{
			new (p) U(std::forward<Args>(args)...);
		}
		catch (...) {
			std::cerr << "Ошибка при конструировании объекта!" << std::endl;
			throw;
		}
	}

	template <typename U>
	void Destroy(U *p) // Метод для разрушения объекта
	{
		try
		{
			p->~U();
		}
		catch (...) {
			std::cerr << "Ошибка при разрушении объекта!" << std::endl;
			throw;
		}
	}

	std::size_t Max_size() const noexcept
	{ // Метод для получения максимального количества элементов, которые могут быть выделены
		return std::numeric_limits<std::size_t>::max() / sizeof(T);
	}

	template <typename U>
	struct rebind // Шаблон для создания аллокатора для другого типа
	{
		using other = BSTree_Allocator<U>;
	};
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