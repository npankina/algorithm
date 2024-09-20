#include "Tree.h"

//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>::~BSTree()
{
	Clear(root_); // Очищаем все узлы дерева при удалении объекта
}
//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>::BSTree() : root_(nullptr), duplicate_elements_count_(0), total_elements_count_(0)
{}
//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>::BSTree(const BSTree &other) 
: root_(nullptr), duplicate_elements_count_(other.duplicate_elements_count_),
  total_elements_count_(other.total_elements_count_)
{ // copy ctor
	
	root_ = Copy(other);
	duplicate_elements_count_ = other.duplicate_elements_count_;
	total_elements_count_ = other.total_elements_count_;
}
//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>::BSTree(BSTree &&temp) noexcept 
: root_(temp.root_), duplicate_elements_count_(temp.duplicate_elements_count_), 
  total_elements_count_(temp.total_elements_count_)
{ // move ctor
	temp.root_ = nullptr;
}
//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>& BSTree<T>::operator=(const BSTree &other) // copy assign
{ //  Очищает текущее дерево и копирует узлы из другого дерева.
	if (this != &other)
	{
		Clear(root_);
		root_ = Copy(other.root_);
	}

	return *this;
}
//----------------------------------------------------------------------------------------------------
template <typename T>
BSTree<T>& BSTree<T>::operator=(BSTree &&temp) noexcept // move assign
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
template <typename T>
void BSTree<T>::Insert(T value)
{	// Проверка допустимости значения (например, если вы не хотите вставлять нулевые значения)
	if (value == T())
		throw std::invalid_argument("Значение не может быть пустым или нулевым");

	root_ = Insert(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
bool BSTree<T>::Search(T value) const
{
	return Search(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
void BSTree<T>::Remove(T value)
{ /* Обработка ошибок:
	 1. Обработка случая, когда узел не найден.
	 2. Обработка попыток удаления узлов с некорректными значениями.*/

	if (!Search(value))
		throw std::runtime_error("Элемент не найден в дереве");

	root_ = Remove(root_, value);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
void BSTree<T>::Print_in_order() const
{
	Inorder(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
typename BSTree<T>::Node* BSTree<T>::Insert(Node *node, T value, size_t depth)
{/* Алгоритм:
	1. Если корневой узел пуст, создается новый узел и устанавливается как корень.
	2. В противном случае начинается рекурсивная вставка. 
	2.1. Если новое значение меньше текущего узла, рекурсивно вызываем Insert для левого поддерева, иначе для правого.
	3. Операция продолжается до тех пор, пока не будет найдено подходящее пустое место для нового узла.
  */
	
	if (depth > MAX_DEPTH)
		throw std::runtime_error("Слишком глубокая рекурсия");

	try
	{
		if (node == nullptr) // Если достигли пустого места, создаем новый узел
			return new Node(value); // Здесь может возникнуть ошибка выделения памяти

		if (value < node->data_)
			node->left_ = Insert(node->left_, value);
		else if (value > node->data_)
			node->right_ = Insert(node->right_, value);
		else // Если значение уже существует, увеличиваем счётчик дубликатов
			node->count_++;
	}
	catch (const std::bad_alloc &err)
	{
		sdt::cerr << "Ошибка выделения памяти: " << err.what() << std::endl;
		throw;
	}

	return node;
}
//----------------------------------------------------------------------------------------------------
template <typename T>
bool BSTree<T>::Search(Node *node, T value) const
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
template <typename T>
typename BSTree<T>::Node* BSTree<T>::Remove(typename BSTree<T>::Node* node, T value)
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
template <typename T>
typename BSTree<T>::Node* BSTree<T>::Find_min(Node *node) const
{ /* Алгоритм:
	1. Если у текущего узла есть левый потомок, продолжаем двигаться влево.
	2. Если у текущего узла нет левого потомка, это и есть минимальный узел, так как все меньшие значения находятся слева.
  */
	while (node->left_ != nullptr) // Пока есть левый потомок, идем в левое поддерево
		node = node->left_;
	return node;  // Возвращаем узел с минимальным значением
}
//----------------------------------------------------------------------------------------------------
template <typename T>
typename BSTree<T>::Node* BSTree<T>::Copy(Node *node)
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
template <typename T>
void BSTree<T>::Clear(Node *node)
{
	if (node != nullptr)
	{
		Clear(node->left_);
		Clear(node->right_);
		delete node;
	}
}
//----------------------------------------------------------------------------------------------------
template <typename T>
void BSTree<T>::Inorder(Node *node) const
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
template <typename T>
typename BSTree<T>::Iterator BSTree<T>::begin() const
{
	return Iterator(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
typename BSTree<T>::Iterator BSTree<T>::end() const
{
	return Iterator(nullptr);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
typename BSTree<T>::Const_Iterator BSTree<T>::сbegin() const
{
	return Const_Iterator(root_);
}
//----------------------------------------------------------------------------------------------------
template <typename T>
typename BSTree<T>::Const_Iterator BSTree<T>::сend() const
{
	return Const_Iterator(nullptr);
}
//----------------------------------------------------------------------------------------------------