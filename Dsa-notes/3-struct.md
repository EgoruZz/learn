# Содержание
- [3. Структуры данных](#3-структуры-данных)
  - [I. Линейные структуры](#i-линейные-структуры)
    - [A. Массивы и последовательные контейнеры](#a-массивы-и-последовательные-контейнеры)
    - [B. Списки и цепочки](#b-списки-и-цепочки)
    - [C. Стеки (LIFO - Last In, First Out)](#c-стеки-lifo---last-in-first-out)
    - [D. Очереди (FIFO - First In, First Out)](#d-очереди-fifo---first-in-first-out)
    - [E. Специализированные буферы](#e-специализированные-буферы)
    - [F. Гибридные, функциональные и ленивые структуры](#f-гибридные-функциональные-и-ленивые-структуры)
  - [II. Конкурентные (параллельные) структуры](#ii-конкурентные-параллельные-структуры)
  - [III. Деревья поиска](#iii-деревья-поиска)
  - [IV. Структуры для множеств](#iv-структуры-для-множеств)
  - [V. Кучи (Priority Queues)](#v-кучи-priority-queues)
  - [VI. Структуры для запросов на отрезках](#vi-структуры-для-запросов-на-отрезках)
    - [A. Префиксные суммы](#a-префиксные-суммы)
    - [B. Дерево Фенвика (Fenwick Tree / Binary Indexed Tree)](#b-дерево-фенвика-fenwick-tree--binary-indexed-tree)
    - [C. Разреженная таблица (Sparse Table)](#c-разреженная-таблица-sparse-table)
    - [D. Дерево отрезков (Segment Tree)](#d-дерево-отрезков-segment-tree)
    - [E. Деревья разбора и синтаксические структуры](#e-деревья-разбора-и-синтаксические-структуры)
    - [F. Продвинутые деревья](#f-продвинутые-деревья)
  - [VII. Корневые структуры (Sqrt Decomposition)](#vii-корневые-структуры-sqrt-decomposition)
  - [VIII. Персистентные структуры](#viii-персистентные-структуры)
  - [IX. Пространственно-эффективные структуры](#ix-пространственно-эффективные-структуры)
  - [X. Прикладные и специализированные структуры](#x-прикладные-и-специализированные-структуры)
  - [XI. Теоретические и экзотические структуры](#xi-теоретические-и-экзотические-структуры)

# **3. СТРУКТУРЫ ДАННЫХ**

## **I. ЛИНЕЙНЫЕ СТРУКТУРЫ**
### **A. МАССИВЫ И ПОСЛЕДОВАТЕЛЬНЫЕ КОНТЕЙНЕРЫ**
*   **МАССИВЫ (ARRAYS)**
    *   **Статические массивы**
        *   C-style arrays
        *   `std::array` (фиксированный размер)
    *   **Динамические массивы**
        *   `std::vector` - амортизированный анализ, управление capacity
        *   Small Vector Optimization (SSO для векторов)
    *   **Алгоритмы на массивах:**
        *   Equilibrium Index In Array
        *   Find Triplets With 0 Sum
        *   Index 2D Array In 1D
        *   Kth Largest Element
        *   Median Two Array
        *   Monotonic Array
        *   Pairs With Given Sum
        *   Permutations
        *   Prefix Sum
        *   Product Sum
        *   Rotate Array
        *   Sudoku Solver
        *   **Дополнительно:**
            *   Kadane's Algorithm (максимальная подмассивная сумма)
            *   Dutch National Flag (сортировка 3 цветов)
            *   Maximum Subarray Sum (разделяй и властвуй)
            *   Sliding Window Maximum
            *   Rain Water Trapping
*   **BIT BOARD**
    *   Для игровых движков (шахматы, шашки)
    *   Побитовые операции над состояниями

### **B. СПИСКИ И ЦЕПОЧКИ**
*   **LINKED LIST:**
    *   **Односвязные списки:**
        *   Singly Linked List
        *   From Sequence
        *   Print Reverse
        *   Middle Element Of Linked List
        *   Has Loop
        *   Floyds Cycle Detection
        *   `std::forward_list`
    *   **Двусвязные списки:**
        *   Doubly Linked List
        *   Doubly Linked List Two
        *   `std::list`
    *   **Кольцевые списки:**
        *   Circular Linked List
    *   **Оптимизированные списки:**
        *   XOR Linked List - память-эффективная версия
        *   Unrolled Linked List - кэш-эффективная версия
    *   **Сложные операции:**
        *   Merge Two Lists
        *   Is Palindrome
        *   Reverse K Group
        *   Rotate To The Right
        *   Swap Nodes
    *   **Продвинутые структуры:**
        *   Skip List (вероятностная структура)
        *   Self-Organizing List (перемещение к началу/транспозиция)

### **C. СТЕКИ (LIFO - LAST IN, FIRST OUT)**
*   **Базовые реализации стека:**
    *   Stack
    *   Stack With Singly Linked List
    *   Stack With Doubly Linked List
*   **Реализации через другие структуры:**
    *   Stack Using Two Queues
    *   Stack On Pseudo Stack
*   **Специализированные стеки:**
    *   Min-Stack (поддержка минимума за O(1))
    *   Max-Stack (поддержка максимума за O(1))
    *   All-Operations-Stack (все операции за O(1))
    *   Persistent Stack (версионный стек)
*   **Алгоритмы на стеке:**
    *   Balanced Parentheses
    *   Infix To Postfix Conversion
    *   Infix To Prefix Conversion
    *   Postfix Evaluation
    *   Prefix Evaluation
    *   Largest Rectangle Histogram
    *   Lexicographical Numbers
    *   Next Greater Element
    *   Stock Span Problem
    *   Dijkstras Two Stack Algorithm
*   **Применения:**
    *   Call Stack (рекурсия)
    *   Undo/Redo в редакторах
    *   Backtracking алгоритмы
    *   Depth-First Search (DFS)

### **D. ОЧЕРЕДИ (FIFO - FIRST IN, FIRST OUT)**
*   **Базовые реализации очереди:**
    *   Queue By List
    *   Linked Queue
*   **Кольцевые очереди:**
    *   Circular Queue
    *   Circular Queue Linked List
*   **Реализации через другие структуры:**
    *   Queue By Two Stacks
    *   Queue On Pseudo Stack
*   **Приоритетные очереди:**
    *   Priority Queue Using List
    *   *(более полные реализации в разделе V. КУЧИ)*
*   **Двусторонние очереди (DEQUE):**
    *   Double Ended Queue
    *   Deque Doubly
    *   `std::deque`
    *   Array-based Deque
    *   Circular Deque
    *   Catenable Deque
    *   Deque с ограниченными операциями
*   **Стеко-очереди (STEQUE):**
    *   Stack-ended Queue
    *   Комбинация стека и очереди
*   **Специализированные очереди:**
    *   Catenable Queue (эффективное объединение)
    *   Calendar Queue (для дискретных событий)
    *   Bucket Queue (ограниченные диапазоны приоритетов)
    *   Banker's Queue / Physicist's Queue:
    *   Hood-Melville Queue
    *   Bootstrapped Queue

### **E. СПЕЦИАЛИЗИРОВАННЫЕ БУФЕРЫ**
*   **Кольцевые буферы (RING/CIRCULAR BUFFERS):**
    *   Fixed-size circular buffer
    *   Overwriting circular buffer
    *   Producer-Consumer паттерны
*   **Буферы скользящего окна:**
    *   Time-based sliding windows
    *   Count-based sliding windows
    *   Moving Average вычисления
*   **Буферы для потоков данных:**
    *   Reservoir Sampling (случайная выборка из потока)
    *   Bloom Filter (в разделе хэширования)

### **F. ГИБРИДНЫЕ, ФУНКЦИОНАЛЬНЫЕ И ЛЕНИВЫЕ СТРУКТУРЫ**
*   **Finger Tree:**
    *   Обобщенные последовательности
    *   Поддержка конкатенации, разделения, индексации
*   **RRB-Vector (Relaxed Radix Balanced Vector):**
    *   Персистентные векторы
    *   Эффективное копирование с изменением
*   **Zipper:**
    *   Функциональные структуры данных
    *   "Фокус" на элементе с доступом к контексту
*   **Chunked Sequence:**
    *   Блочное хранение для лучшей кэш-локализации
*   **Ленивые вычисления** (Lazy Evaluation)
    *   **Thunk** - отложенное вычисление
    *   **Promise** - обещание значения
    *   **Stream/Generator** - ленивые последовательности
*   **Мемоизация и кэширование**
    *   **Memoization деревья** - кэширование результатов функций
    *   **Таблицы динамического программирования** как деревья

## **II. КОНКУРЕНТНЫЕ (ПАРАЛЛЕЛЬНЫЕ) СТРУКТУРЫ**
*   **Lock-based структуры:**
    *   Mutex-protected Queue/Stack
    *   Reader-Writer Lock структуры
    *   Condition Variable очереди
*   **Lock-free структуры:**
    *   Lock-free Stack (Treiber)
    *   Lock-free Queue (Michael-Scott)
    *   Lock-free Deque
    *   Atomic операции, memory ordering
*   **Wait-free структуры:**
    *   Wait-free Queue
    *   Wait-free Stack
*   **Concurrent Containers:**
    *   `concurrent_queue`, `concurrent_stack`
    *   `concurrent_vector`, `concurrent_hash_map`

## **III. ДЕРЕВЬЯ ПОИСКА**
*   **Базовые бинарные деревья:**
    *   Basic Binary Tree
    *   Binary Tree Traversals:
        *   Inorder, Preorder, Postorder
        *   Level-order (BFS)
        *   Inorder Tree Traversal 2022
    *   Binary Tree Mirror
    *   Symmetric Tree
    *   Diameter Of Binary Tree
    *   Binary Tree Node Sum
    *   Binary Tree Path Sum
    *   Distribute Coins
    *   Lowest Common Ancestor
    *   Serialize Deserialize Binary Tree
    *   Merge Two Binary Trees
*   **Бинарные деревья поиска (BST):**
    *   Binary Search Tree
    *   Binary Search Tree Recursive
    *   Floor And Ceiling
    *   Is Sorted
    *   Is Sum Tree
    *   Maximum Sum Bst
*   **Сбалансированные деревья:**
    *   AVL-дерево
    *   Красно-черное дерево
    *   Splay-дерево
    *   AA-дерево
    *   Scapegoat Tree
*   **Многопутевые деревья:**
    *   B-дерево
    *   B+-дерево
    *   B*-дерево
    *   2-3 Tree
    *   2-3-4 Tree
*   **Специализированные деревья:**
    *   Декартово дерево (Treap):
        *   Обычный Treap - BST + Куча
        *   Неявный ключ (implicit key)
        *   Операция `reverse` на отрезке
    *   Wavelet Tree
    *   Fenwick Tree (в разделе VI)
    *   Segment Tree (в разделе VI)
*   **Деревья в STL:**
    *   `std::map` (красно-черное дерево)
    *   `std::set`, `std::multiset`
    *   `std::multimap`

## **IV. СТРУКТУРЫ ДЛЯ МНОЖЕСТВ**
*   **Система непересекающихся множеств (DSU/Union-Find):**
    *   Disjoint Set
    *   Alternate Disjoint Set
    *   **Оптимизации:**
        *   Union by Rank/Size
        *   Path Compression
        *   Persistent DSU
        *   Rollback DSU
    *   **Варианты:**
        *   Partially Persistent DSU
        *   Dynamic Connectivity
*   **Битсеты:**
    *   `std::bitset`
    *   Dynamic Bitset
    *   **Операции:**
        *   Bit manipulation
        *   Set operations (union, intersection, difference)
        *   Bit Count (popcount)

## **V. КУЧИ (PRIORITY QUEUES)**
*   **Бинарная куча:**
    *   Heap
    *   Heap Generic
    *   Max Heap
    *   Min Heap
    *   `std::priority_queue`
*   **Биномиальная куча:**
    *   Binomial Heap
*   **Фибоначчиева куча:**
    *   Fibbonaci Heap
*   **Парная куча (Pairing Heap)**
*   **Левосторонняя куча (Leftist Heap)**
*   **Косая куча (Skew Heap):**
    *   Skew Heap
*   **Рандомизированная куча:**
    *   Randomized Heap
*   **Для целочисленных ключей (Radix Heap):**
*   **Двусторонняя куча (Double-ended Priority Queue):**
    *   Min-Max Heap
    *   Interval Heap
    *   Deap

## **VI. СТРУКТУРЫ ДЛЯ ЗАПРОСОВ НА ОТРЕЗКАХ**
### **A. ПРЕФИКСНЫЕ СУММЫ**
*   Prefix Sum (1D)
*   2D Prefix Sum
*   3D Prefix Sum
*   Difference Array (разностный массив)
*   Prefix Sum with Updates

### **B. ДЕРЕВО ФЕНВИКА (FENWICK TREE / BINARY INDEXED TREE)**
*   Standard Fenwick Tree
*   Maximum Fenwick Tree
*   Minimum Fenwick Tree
*   **Варианты:**
    *   2D Fenwick Tree
    *   Fenwick Tree with Range Updates
    *   Fenwick Tree on Fenwick Tree
*   **Операции:**
    *   Point Update, Range Query
    *   Range Update, Point Query
    *   Range Update, Range Query

### **C. РАЗРЕЖЕННАЯ ТАБЛИЦА (SPARSE TABLE)**
*   Sparse Table для RMQ (Range Minimum Query)
*   Sparse Table для GCD
*   Sparse Table для любых идемпотентных операций
*   Двумерная Sparse Table
*   Disjoint Sparse Table

### **D. ДЕРЕВО ОТРЕЗКОВ (SEGMENT TREE)**
*   **Базовые реализации:**
    *   Segment Tree (на массиве)
    *   Segment Tree (на указателях)
    *   Non Recursive Segment Tree
    *   Segment Tree Other
*   **Отложенные операции (Lazy Propagation):**
    *   Add on segment, sum on segment
    *   Assign on segment
    *   Add and Assign combination
*   **Массовые операции:**
    *   Range updates
    *   Range queries (sum, min, max, gcd)
*   **Segment Tree Beats:**
    *   `min=` operation
    *   `max=` operation
    *   `%=` operation (modulo)
    *   Возведение в степень на отрезке
    *   Bitwise operations (AND, OR, XOR)
*   **Персистентное дерево отрезков:**
    *   Persistent Segment Tree
    *   Внешнеплоское дерево (PST)
*   **Segment Tree с комбинированными обновлениями**
    *   Поддержка `add` и `set` одновременно
    *   Приоритет операций (например, `set` перекрывает `add`)
    *   **Операции с историей** - отслеживание изменений

### **E. ДЕРЕВЬЯ РАЗБОРА И СИНТАКСИЧЕСКИЕ СТРУКТУРЫ**
*   **AST (Abstract Syntax Tree - Абстрактное синтаксическое дерево)**
    *   **Бинарные AST** - для арифметических выражений
    *   **N-арные AST** - для сложных конструкций
    *   **Узлы**: операторы (`+`, `-`, `*`, `/`, `^`), функции (`sin`, `cos`, `log`)
    *   **Листья**: константы, переменные, литералы
    *   **Свойства**: приоритет операторов, ассоциативность
*   **Parse Tree (Дерево разбора)**
    *   Полное дерево с нетерминалами и терминалами
    *   **Конкретное синтаксическое дерево** - включает все детали грамматики
    *   **Канонические формы**: приведение к нормальной форме
*   **Expression Tree (Дерево выражений)**
    *   Специализированное AST для математических выражений
    *   **Оптимизации**: constant folding, common subexpression elimination
    *   **Дифференцирование**: построение производной выражения
    *   **Упрощение**: алгебраические упрощения
*   **Построение и обход деревьев разбора**
    *   **Рекурсивный спуск** (Recursive Descent Parser)
    *   **LL-парсеры** - Left-to-right, Leftmost derivation
    *   **LR-парсеры** - Left-to-right, Rightmost derivation
    *   **Shift-Reduce парсеры**
    *   **Операторный парсер** (Pratt Parser) - для разных приоритетов
*   **Трансформации и вычисления**
    *   **Visitor Pattern** для обхода AST
    *   **Вычисление выражений** (интерпретация)
    *   **Компиляция в байт-код** или машинный код
    *   **Дифференциальное выполнение** (automatic differentiation)
    *   **Символьные вычисления** (computer algebra)
*   **Валидация и проверка корректности**
    *   **Проверка типов** (type checking)
    *   **Статический анализ** выражений
    *   **Обнаружение ошибок**: деление на ноль, переполнение
    *   **Верификация** математических выражений
*   **Специализированные структуры для выражений**
    *   **DAG выражений** - Directed Acyclic Graph для общих подвыражений
    *   **Трехадресный код** - промежуточное представление
    *   **SSA форма** (Static Single Assignment)
    *   **Таблица символов** (Symbol Table) для переменных и функций
*   **Применения в компиляторах и интерпретаторах**
    *   **Front-end компилятора**: лексический анализ → синтаксический анализ → AST
    *   **Макропроцессоры** и препроцессоры
    *   **Шаблонизаторы** и системы генерации кода
    *   **Доменно-специфичные языки** (DSL)
    *   **Конфигурационные языки** (JSON, YAML, XML парсеры)

### **F. ПРОДВИНУТЫЕ ДЕРЕВЬЯ:**
*   **Range Tree:**
    *   1D Range Tree
    *   2D Range Tree
    *   Для многомерных запросов
*   **Interval Tree:**
    *   Для работы с интервалами
    *   Статические и динамические интервалы
*   **Segment Tree with Fractional Cascading**
*   **Li Chao Tree** (для линейных функций)
*   **Merge Sort Tree** (для запросов k-той статистики)

## **VII. КОРНЕВЫЕ СТРУКТУРЫ (SQRT DECOMPOSITION)**
*   **Корневая декомпозиция:**
    *   Блоки фиксированного размера
    *   Оптимальный размер блока (√n)
*   **Алгоритм Мо (Mo's Algorithm):**
    *   Стандартный алгоритм Мо
    *   Алгоритм Мо с обновлениями (Mo with updates)
    *   Алгоритм Мо на деревьях
*   **Дерево отрезков с корневой декомпозицией:**
    *   Segment Tree + Sqrt Decomposition
    *   Оптимизация для смешанных запросов
*   **Sqrt Decomposition по блокам:**
    *   Разбиение на тяжелые и легкие вершины
    *   Heavy-Light Decomposition (HLD)

## **VIII. ПЕРСИСТЕНТНЫЕ СТРУКТУРЫ**
*   **Персистентное дерево отрезков:**
    *   Persistent Segment Tree
*   **Персистентное декартово дерево:**
    *   Persistent Treap
*   **Структуры с откатами:**
    *   Rollback DSU
    *   Persistent Stack
    *   Persistent Queue
*   **Fully Persistent структуры:**
    *   Любая версия доступна для модификации
*   **Confluently Persistent структуры:**
    *   Слияние версий
*   **Functional структуры:**
    *   Persistent Vector
    *   Persistent Map/Set
    *   Persistent Queue (реализация на двух стеках)

## **IX. ПРОСТРАНСТВЕННО-ЭФФЕКТИВНЫЕ СТРУКТУРЫ**
*   **Succinct Data Structures:**
    *   Суффиксный массив + LCP
    *   Wavelet Matrix
    *   Fully Indexable Dictionary (FID)
    *   Rank/Select структуры
*   **Compressed Data Structures:**
    *   Compressed Segment Tree
    *   Sparse Table with compression
*   **Implicit Data Structures:**
    *   Implicit Treap
    *   Implicit Cartesian Tree

## **X. ПРИКЛАДНЫЕ И СПЕЦИАЛИЗИРОВАННЫЕ СТРУКТУРЫ**
*   **Для потоков данных:**
    *   **Count-Min Sketch**
    *   **HyperLogLog**
    *   **Bloom Filter** (в разделе хэширования)
    *   **Reservoir Sampling**
*   **Для кэширования:**
    *   LRU Cache (Least Recently Used)
    *   LFU Cache (Least Frequently Used)
    *   ARC Cache (Adaptive Replacement Cache)
    *   LIRS Cache
*   **Для графов:**
    *   Adjacency Matrix
    *   Adjacency List
    *   Incidence Matrix
    *   CSR (Compressed Sparse Row)
    *   Forward Star
*   **Для баз данных:**
    *   B+Tree (индексы)
    *   Hash Index
    *   Bitmap Index
    *   R-Tree (для пространственных данных)
    *   GiST (Generalized Search Tree)

## **XI. ТЕОРЕТИЧЕСКИЕ И ЭКЗОТИЧЕСКИЕ СТРУКТУРЫ**
*   **VEB-Tree (van Emde Boas Tree):**
    *   Для целых чисел из ограниченного универсума
    *   Все операции за O(log log U)
*   **Y-Fast Trie**
*   **X-Fast Trie**
*   **Fusion Tree:**
    *   Для целых чисел, операции за O(log_w n)
*   **Dynamic Connectivity структуры:**
    *   Holm-de Lichtenberg-Thorup
    *   ETT (Euler Tour Tree)
*   **Retroactive Data Structures:**
    *   Поддержка операций во времени
