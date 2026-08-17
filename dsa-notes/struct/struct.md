# **СТРУКТУРЫ ДАННЫХ**

> Разделы I–XI: `struct/a/` ... `struct/k/` — каждый раздел — конспект `X.md` + реализация `X.cpp`. Классы образуют цепочку наследования: `LinearStructures` (a.cpp) ← `SearchTrees` (b.cpp) ← `SetStructures` (c.cpp) ← `Heaps` (d.cpp) ← `RangeQueries` (e.cpp) ← `SqrtStructures` (f.cpp) ← `PersistentStructures` (g.cpp) ← `ConcurrentStructures` (h.cpp) ← `SyntaxStructures` (i.cpp) ← `CachingStructures` (j.cpp) ← `TheoreticalStructures` (k.cpp): каждый конспект использует материал предыдущих (арены и курсоры из I, представления деревьев из II, амортизационный анализ из V и analysis.md). Имена классов финализируются при написании каждого конспекта.
>
> **Связи с другими ветками:** хеш-таблицы и Bloom Filter — `hashing.md`; строковые контейнеры, rope, trie, суффиксные структуры, парсеры строк — `string.md`; представления графов (Adjacency, CSR, Forward Star), Incidence Matrix, структуры динамической связности — `graph.md`; пространственные структуры (R-Tree, KD-Tree, QuadTree/Octree) — `geometry.md`; монотонная очередь и оптимизации ДП — `dynamic.md` (J); амортизированный анализ, массив с удвоением — `analysis.md`; бинарный поиск как приём над структурами — `technique.md`; структуры на числовых множествах (решёта, факторизация) — `math/number-theory`.

---

## **I. ЛИНЕЙНЫЕ СТРУКТУРЫ**
> `a/a.md` · `a/a.cpp` · `struct LinearStructures` — базовый класс всей ветки struct

### **A. МАССИВЫ И ПОСЛЕДОВАТЕЛЬНЫЕ КОНТЕЙНЕРЫ**
*   **МАССИВЫ (ARRAYS)**
    *   **Статические массивы**
        *   C-style arrays
        *   `std::array` (фиксированный размер)
    *   **Динамические массивы**
        *   `std::vector` - амортизированный анализ, управление capacity, рост (фактор роста `2`/`1.5` — выбор компромисса память/скорость); амортизация `push_back` — связь `analysis.md`
        *   Small Vector Optimization (SSO для векторов)
        *   `std::string` как динамический массив символов, SSO — связь `string.md`
    *   **Порядок и память**
        *   Row-major / column-major: индексация многомерных массивов, кэш-локализация
        *   Аллокация пулом страниц / расположение в памяти (locality of reference)
    *   **Алгоритмы на массивах** (с сохранением стиля «одна задача — один обобщённый метод»):
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
    *   Для игровых движков (шахматы, шашки): доска фиксированного размера `n×n` как последовательность машинных слов
    *   Побитовые операции над состояниями: сдвиги, маски, `popcount`, атаки/перемещения как битовые операции; обобщение на произвольный размер поля `n` и число битов на клетку `b` (не только 1)

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
        *   Список с хвостовым указателем (`tail`): константные `push_back` и `append` — базовая оптимизация
    *   **Оптимизированные списки:**
        *   XOR Linked List - память-эффективная версия
        *   Unrolled Linked List - кэш-эффективная версия (блок размера `B` вместо одного узла; выбор `B` как параметр)
    *   **Курсорные списки (арена-память):**
        *   Cursor Linked List: узлы в массиве, ссылки — индексы (а не указатели) — компактность, сериализуемость, никапсуляция в один вектор
        *   Пул/арена аллокации узлов (pool allocator) как общий приём для всех структур с указателями (мост в II, IV)
    *   **Сложные операции:**
        *   Merge Two Lists
        *   Is Palindrome
        *   Reverse K Group
        *   Rotate To The Right
        *   Swap Nodes
    *   **Продвинутые структуры:**
        *   Skip List (вероятностная структура — см. `hashing.md` IV, базовые операции здесь не дублируются)
        *   Self-Organizing List (перемещение к началу/транспозиция)

### **C. СТЕКИ (LIFO - LAST IN, FIRST OUT)**
*   **Базовые реализации стека:**
    *   Stack
    *   Stack With Singly Linked List
    *   Stack With Doubly Linked List
    *   Стек на общем массиве (two stacks in one array) — торговля памятью между независимыми стеками
*   **Реализации через другие структуры:**
    *   Stack Using Two Queues
    *   Stack On Pseudo Stack
*   **Специализированные стеки:**
    *   Min-Stack (поддержка минимума за O(1))
    *   Max-Stack (поддержка максимума за O(1))
    *   All-Operations-Stack (все операции за O(1))
    *   Persistent Stack (версионный стек — см. раздел VII)
*   **Монотонный стек:**
    *   Идея: стек, где элементы строго монотонны по ключу; каждый элемент входит и выходит ровно один раз — суммарно O(n)
    *   Следующий меньший/больший элемент, предыдущие меньшие/большие (обобщение на произвольный предикат сравнения)
    *   Largest Rectangle Histogram (мост: координатная полоса, `n` произвольное)
    *   Stock Span Problem (глубина стека как метрика)
*   **Алгоритмы на стеке:**
    *   Balanced Parentheses (обобщение на `k` типов скобок)
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
    *   Depth-First Search (DFS) — связь `graph.md`

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
    *   *(более полные реализации в разделе IV. КУЧИ)*
*   **Двусторонние очереди (DEQUE):**
    *   Double Ended Queue
    *   Deque Doubly
    *   `std::deque` — устройство: карта блоков + блоки фиксированной длины (амортизация и кэш-локализация)
    *   Array-based Deque
    *   Circular Deque
    *   Catenable Deque
    *   Deque с ограниченными операциями
*   **Стеко-очереди (STEQUE):**
    *   Stack-ended Queue
    *   Комбинация стека и очереди
*   **Монотонная очередь:**
    *   Идея: deque с убывающим/возрастающим порядком значений; каждый элемент входит/выходит один раз — O(n)
    *   Скользящее окно максимума/минимума (обобщение Sliding Window Maximum из II.A на направление и длину окна `k`)
    *   Оптимизация интервального ДП (размен, jump game с окном) — связь `dynamic.md` (раздел J)
*   **Специализированные очереди:**
    *   Catenable Queue (эффективное объединение)
    *   Calendar Queue (для дискретных событий)
    *   Bucket Queue (ограниченные диапазоны приоритетов — мост в IV)
    *   Banker's Queue / Physicist's Queue
    *   Hood-Melville Queue
    *   Bootstrapped Queue

### **E. СПЕЦИАЛИЗИРОВАННЫЕ БУФЕРЫ**
*   **Кольцевые буферы (RING/CIRCULAR BUFFERS):**
    *   Fixed-size circular buffer
    *   Overwriting circular buffer
    *   Producer-Consumer паттерны (мост в `hashing.md` IV — конкурентные)
*   **Буферы скользящего окна:**
    *   Time-based sliding windows
    *   Count-based sliding windows
    *   Moving Average вычисления (скользящее среднее, экспоненциальное сглаживание)
*   **Буферы для потоков данных:**
    *   Reservoir Sampling (случайная выборка из потока — см. `hashing.md` IV)
    *   Bloom Filter (см. `hashing.md` IV)

### **F. ГИБРИДНЫЕ, ФУНКЦИОНАЛЬНЫЕ И ЛЕНИВЫЕ СТРУКТУРЫ**
*   **Finger Tree:**
    *   Обобщённые последовательности
    *   Поддержка конкатенации, разделения, индексации (амортизированно O(log n))
*   **RRB-Vector (Relaxed Radix Balanced Vector):**
    *   Персистентные векторы
    *   Эффективное копирование с изменением (мост в раздел VII — персистентность)
*   **Zipper:**
    *   Функциональные структуры данных
    *   "Фокус" на элементе с доступом к контексту (обобщение: зиппер на дереве и на списке)
*   **Chunked Sequence:**
    *   Блочное хранение для лучшей кэш-локализации
    *   Обобщение на двунаправленную и произвольную вставку (rope — см. `string.md`)
*   **Ленивые вычисления** (Lazy Evaluation)
    *   **Thunk** - отложенное вычисление
    *   **Promise** - обещание значения
    *   **Stream/Generator** - ленивые последовательности
*   **Мемоизация и кэширование**
    *   **Memoization деревья** - кэширование результатов функций
    *   **Таблицы динамического программирования** как деревья

---

## **II. ДЕРЕВЬЯ ПОИСКА**
> `b/b.md` · `b/b.cpp` · `struct SearchTrees : LinearStructures`

### **A. ПРЕДСТАВЛЕНИЯ ДЕРЕВЬЕВ В ПАМЯТИ**
*   Узлы с указателями (динамическая память) и арены/пулы узлов (фиксированный массив узлов, индексы вместо указателей) — мост из I.B
*   Parent Array: хранение родителей в массиве — для подъёмов, LCA-заделов
*   First-child / next-sibling представление для n-арных деревьев
*   Представление полного бинарного дерева массивом (куча-индексация `i → 2i, 2i+1`) — мост в IV и V (Segment Tree)
*   Нитяные деревья (Threaded Binary Trees): ссылки на inorder-предшественника/преемника в «пустых» указателях — обход без стека
*   Morris Traversal: обход бинарного дерева за O(n) времени и O(1) памяти (временная нить)
*   Выбор представления как параметр задачи: статика (массивы) против динамики (указатели)

### **B. БАЗОВЫЕ БИНАРНЫЕ ДЕРЕВЬЯ**
*   Basic Binary Tree
*   Binary Tree Traversals:
    *   Inorder, Preorder, Postorder (рекурсивно/итеративно со стеком, Morris)
    *   Level-order (BFS)
    *   Inorder Tree Traversal 2022
*   Binary Tree Mirror
*   Symmetric Tree
*   Diameter Of Binary Tree
*   Binary Tree Node Sum
*   Binary Tree Path Sum
*   Distribute Coins
*   Lowest Common Ancestor (подходы: подъёмы, двоичные подъёмы — задел под `graph.md`)
*   Serialize Deserialize Binary Tree
*   Merge Two Binary Trees

### **C. БИНАРНЫЕ ДЕРЕВЬЯ ПОИСКА (BST)**
*   Binary Search Tree
*   Binary Search Tree Recursive
*   Floor And Ceiling
*   Is Sorted
*   Is Sum Tree
*   Maximum Sum Bst
*   Операции: поиск, вставка, удаление (случаи 0/1/2 детей), обход `rotate` как примитив (задел на D)

### **D. СБАЛАНСИРОВАННЫЕ ДЕРЕВЬЯ**
*   AVL-дерево (баланс-фактор, повороты LL/RR/LR/RL, высота сбалансированного дерева `O(log n)`)
*   Красно-черное дерево (инварианты, вставка/удаление по случаям, сравнение с AVL на практике)
*   Splay-дерево (амортизация через splay-операцию, кэш-локальность обращения)
*   AA-дерево
*   Scapegoat Tree (простота: перестройка при нарушении α-баланса)
*   Общее: параметр баланса, повороты как примитив, доказательство высоты через рекуррентность (`h = φ`-границы)

### **E. МНОГОПУТЕВЫЕ ДЕРЕВЬЯ**
*   B-дерево (порядок `t`: узлы от `t−1` до `2t−1` ключей, рост наверх, высота `O(log_t n)`)
*   B+-дерево (данные в листьях, связанные листья — база индексов БД, мост в X)
*   B*-дерево
*   2-3 Tree
*   2-3-4 Tree (эквивалент красно-черного, обобщение до B-дерева произвольного порядка)

### **F. ДЕКАРТОВЫ И РАНДОМИЗИРОВАННЫЕ ДЕРЕВЬЯ**
*   Декартово дерево (Treap): BST + Куча (приоритеты)
    *   Обычный Treap - BST + Куча
    *   Неявный Treap (implicit key) - позиция в последовательности как ключ, операции над отрезками (split по размеру + merge)
    *   Операция `reverse` на отрезке
    *   Декомпозиция/merge/split как фундаментальные примитивы (мост в VII — персистентный treap)
*   Randomized Binary Search Tree (рандомизированная вставка по весам)

### **G. СПЕЦИАЛИЗИРОВАННЫЕ ДЕРЕВЬЯ**
*   Wavelet Tree (дерево по битам значений: запросы k-ой статистики и частот на отрезке за O(log σ))
*   Wavelet Matrix (сукцинктный аналог Wavelet Tree: поразрядные битовые векторы с rank/select из III.B — та же k-я статистика при памяти ≈ n·log σ бит)
*   Fenwick Tree (см. раздел V.B)
*   Segment Tree (см. раздел V.D)
*   Order-Statistic Tree: дерево с размерами поддеревьев (select/rank — `std::pb_ds` tree_order_statistics, мост в E и F)

### **H. ДЕРЕВЬЯ В STL**
*   `std::map` (красно-черное дерево)
*   `std::set`, `std::multiset`
*   `std::multimap`
*   `std::priority_queue` — см. IV

---

## **III. СТРУКТУРЫ ДЛЯ МНОЖЕСТВ**
> `c/c.md` · `c/c.cpp` · `struct SetStructures : SearchTrees`

### **A. СИСТЕМА НЕПЕРЕСЕКАЮЩИХСЯ МНОЖЕСТВ (DSU/UNION-FIND)**
*   Disjoint Set
*   Alternate Disjoint Set
*   **Оптимизации:**
    *   Union by Rank/Size
    *   Path Compression
    *   Persistent DSU (мост в VII)
    *   Rollback DSU (откат объединений со стеком изменений — мост в VII)
*   **Варианты:**
    *   Partially Persistent DSU
    *   Dynamic Connectivity (мост в XI)
    *   DSU с данными в компоненте: размер, сумма/произведение, экстремумы — переносимые инварианты
    *   DSU с модификацией весов (weighted union-find: потенциалы к корню)
    *   DSU оффлайн с временными метками рёбер (параллельные запросы по времени)

### **B. БИТСЕТЫ**
*   `std::bitset`
*   Dynamic Bitset
*   **Операции:**
    *   Bit manipulation (установка/сброс/инверсия бита — машинное слово и границы `n ≤ 64·k`)
    *   Set operations (union, intersection, difference — по словам)
    *   Bit Count (popcount, `__builtin_popcountll`, префиксные суммы битов — rank)
    *   Перебор единиц/нулей (следующий установленный бит за `O(1)` словом)
    *   **Rank/Select** (сукцинктное хранение битового вектора): `rank(i)` — число единиц в префиксе (префиксные блоки/суперблоки + `popcount` слова, O(1) на запрос); `select(k)` — позиция k-й единицы (по суперблокам с указателями + двоичный поиск); обобщение: мультиплексируемые слова, `popcount`-примитивы
    *   **FID (Fully Indexable Dictionary)** — битовый вектор с rank/select как базовый примитив сукцинктных структур; принцип: память ≈ информации (энтропия данных), оценка в битах на элемент
*   Применение: сжатие множеств, динамические булевы маски, рюкзак через `dp |= dp << x` (связь `dynamic.md` J.2)

---

## **IV. КУЧИ (PRIORITY QUEUES)**
> `d/d.md` · `d/d.cpp` · `struct Heaps : SetStructures`

*   **Бинарная куча:**
    *   Heap (массив + индексная арифметика, sift-up/sift-down)
    *   Построение кучи за O(n) (снизу вверх по уровням) — амортизация и суммарная оценка
    *   Heap Generic
    *   Max Heap / Min Heap (параметризация компаратора — произвольный ключ и приоритет)
    *   Куча с позиционным массивом (index heap): decrease-key/increase-key за O(log n) — для Дейкстры/Прима (связь `graph.md`)
    *   `std::priority_queue`
*   **Биномиальная куча:**
    *   Binomial Heap (лес биномиальных деревьев, слияние O(log n), бинарное представление размера)
*   **Фибоначчиева куча:**
    *   Fibbonaci Heap (каскадное сокращение, decrease-key O(1) амортизированно; потенциальная функция — связь `analysis.md`)
*   **Парная куча (Pairing Heap)** — практическая альтернатива Фибоначчиевой
*   **Левосторонняя куча (Leftist Heap)** — merge за O(log n), ранг по правому пути
*   **Косая куча (Skew Heap):**
    *   Skew Heap (амортизированный merge, обмен правого/левого на каждом шаге)
*   **Рандомизированная куча:**
    *   Randomized Heap (merge с монеткой приоритетов — мост в `hashing.md` IV)
*   **Для целочисленных ключей (Radix Heap):**
    *   Radix Heap / Bucket Heap: приоритеты из малого диапазона — O(1) амортизированно (мост из I.D Bucket Queue)
*   **Двусторонняя куча (Double-ended Priority Queue):**
    *   Min-Max Heap
    *   Interval Heap
    *   Deap
*   Общее: операции (insert/extract/merge/decrease-key), представление (массив/указатели/индексы), амортизированные оценки, применимость по типам ключей

---

## **V. СТРУКТУРЫ ДЛЯ ЗАПРОСОВ НА ОТРЕЗКАХ**
> `e/e.md` · `e/e.cpp` · `struct RangeQueries : Heaps`

### **A. ПРЕФИКСНЫЕ СУММЫ**
*   Prefix Sum (1D)
*   2D Prefix Sum (включения-исключения прямоугольника)
*   3D Prefix Sum (обобщение на `k` измерений: включения-исключения по `2^k` слагаемых)
*   Difference Array (разностный массив: точечный запрос = префикс; отрезковые обновления за O(1))
*   Prefix Sum with Updates (обновления + запросы — переход к Fenwick V.B)

### **B. ДЕРЕВО ФЕНВИКА (FENWICK TREE / BINARY INDEXED TREE)**
*   Standard Fenwick Tree
*   Maximum Fenwick Tree
*   Minimum Fenwick Tree
*   **Варианты:**
    *   2D Fenwick Tree (вложенные деревья, обобщение на `k` измерений)
    *   Fenwick Tree with Range Updates
    *   Fenwick Tree on Fenwick Tree
*   **Операции:**
    *   Point Update, Range Query
    *   Range Update, Point Query
    *   Range Update, Range Query
*   Поиск k-го по сумме за O(log n) (спуск по степеням двойки)

### **C. РАЗРЕЖЕННАЯ ТАБЛИЦА (SPARSE TABLE)**
*   Sparse Table для RMQ (Range Minimum Query)
*   Sparse Table для GCD
*   Sparse Table для любых идемпотентных операций (параметризация операции `f` и требования ассоциативность + идемпотентность)
*   Двумерная Sparse Table
*   Disjoint Sparse Table (и неидемпотентные операции)
*   Sparse Table с координатным сжатием (для разреженных точек: сжатие координат сводит таблицу к числу уникальных позиций)

### **D. ДЕРЕВО ОТРЕЗКОВ (SEGMENT TREE)**
*   **Базовые реализации:**
    *   Segment Tree (на массиве)
    *   Segment Tree (на указателях)
    *   Non Recursive Segment Tree
    *   Segment Tree Other
    *   Динамическое дерево отрезков (по указателям, вершины создаются по требованию — без координатного сжатия, мост в VII)
    *   Сжатое по координатам дерево отрезков (coordinate compression: сортировка уникальных координат + индексация — статический аналог динамического, мост в VII)
*   **Отложенные операции (Lazy Propagation):**
    *   Add on segment, sum on segment
    *   Assign on segment
    *   Add and Assign combination
*   **Массовые операции:**
    *   Range updates
    *   Range queries (sum, min, max, gcd) — параметризация нейтрального элемента и операции свертки
*   **Segment Tree Beats:**
    *   `min=` operation
    *   `max=` operation
    *   `%=` operation (modulo)
    *   Возведение в степень на отрезке
    *   Bitwise operations (AND, OR, XOR)
*   **Персистентное дерево отрезков:**
    *   Persistent Segment Tree
    *   Внешнеплоское дерево (PST) (мост в VII)
*   **Segment Tree с комбинированными обновлениями**
    *   Поддержка `add` и `set` одновременно
    *   Приоритет операций (например, `set` перекрывает `add`)
    *   **Операции с историей** - отслеживание изменений (мост в VII — персистентность)
*   2D Segment Tree (дерево деревьев, запросы по прямоугольнику)

### **E. ПРОДВИНУТЫЕ ДЕРЕВЬЯ:**
*   **Range Tree:**
    *   1D Range Tree
    *   2D Range Tree
    *   Для многомерных запросов (обобщение до `k` измерений: `O(log^k n)`)
*   **Interval Tree:**
    *   Для работы с интервалами
    *   Статические и динамические интервалы
*   **Segment Tree with Fractional Cascading** (ускорение спуска по деревьям)
*   **Li Chao Tree** (для линейных функций: вставка прямой, максимум/минимум в точке)
*   **Merge Sort Tree** (для запросов k-той статистики, через слияние отсортированных пунктов)

---

## **VI. КОРНЕВЫЕ СТРУКТУРЫ (SQRT DECOMPOSITION)**
> `f/f.md` · `f/f.cpp` · `struct SqrtStructures : RangeQueries`

*   **Корневая декомпозиция:**
    *   Блоки фиксированного размера (`B = ⌈√n⌉` как параметр, настраиваемый под тип операций)
    *   Оптимальный размер блока: `√n` для расщепления (обновление+запрос поровну), иное при асимметрии
    *   Отрезковые запросы: полные блоки целиком + хвосты поштучно — суммарно `O(√n)`
*   **Алгоритм Мо (Mo's Algorithm):**
    *   Стандартный алгоритм Мо (сортировка запросов по блокам левого конца, два указателя)
    *   Алгоритм Мо с обновлениями (Mo with updates) — третья координата времени
    *   Алгоритм Мо на деревьях (эйлеров порядок + LCA-обработка)
    *   Обобщение: функция add/remove как параметр (инвариант окна)
*   **Дерево отрезков с корневой декомпозицией:**
    *   Segment Tree + Sqrt Decomposition
    *   Оптимизация для смешанных запросов
*   **Sqrt Decomposition по блокам:**
    *   Разбиение на тяжелые и легкие вершины (heavy/light: тяжелых ≤ `2·√m`)
    *   Heavy-Light Decomposition (HLD): пути в графе за `O(log² n)` — мост в `graph.md`

---

## **VII. ПЕРСИСТЕНТНЫЕ СТРУКТУРЫ**
> `g/g.md` · `g/g.cpp` · `struct PersistentStructures : SqrtStructures`

*   **Основы персистентности:**
    *   Классификация: частичная (чтение старых версий), полная (модификация любой версии), конфлюэнтная (слияние версий)
    *   Persistent Array: двоичное дерево версий (`path copying`) — базовый примитив всех персистентных структур
    *   Path Copying против Fat Node: обмен памяти и сложности
*   **Персистентное дерево отрезков:**
    *   Persistent Segment Tree (копирование пути при каждом обновлении, `O(log n)` новых вершин)
    *   Приложения: k-я порядковая статистика на отрезке, запросы по истории
*   **Персистентное декартово дерево:**
    *   Persistent Treap (split/merge без модификации старых вершин)
*   **Структуры с откатами:**
    *   Rollback DSU (стек изменений + откат)
    *   Persistent Stack (списки версий)
    *   Persistent Queue (реализация на двух стеках)
*   **Fully Persistent структуры:**
    *   Любая версия доступна для модификации
*   **Confluently Persistent структуры:**
    *   Слияние версий
*   **Functional структуры:**
    *   Persistent Vector (RRB-родственный — мост из I.F)
    *   Persistent Map/Set (персистентный treap / красно-черный с path copying)
    *   Persistent Queue (реализация на двух стеках)

---

## **VIII. КОНКУРЕНТНЫЕ (ПАРАЛЛЕЛЬНЫЕ) СТРУКТУРЫ**
> `h/h.md` · `h/h.cpp` · `struct ConcurrentStructures : PersistentStructures`

*   **Lock-based структуры:**
    *   Mutex-protected Queue/Stack
    *   Reader-Writer Lock структуры
    *   Condition Variable очереди
    *   Спинлоки (backoff), блокирующие vs неблокирующие примитивы
*   **Lock-free структуры:**
    *   Lock-free Stack (Treiber)
    *   Lock-free Queue (Michael-Scott)
    *   Lock-free Deque
    *   Atomic операции, memory ordering (acquire/release/seq_cst), CAS-циклы
    *   ABA-проблема и её решения (tagged pointers, hazard pointers, epoch-based reclamation)
*   **Wait-free структуры:**
    *   Wait-free Queue
    *   Wait-free Stack
*   **Concurrent Containers:**
    *   `concurrent_queue`, `concurrent_stack`
    *   `concurrent_vector`, `concurrent_hash_map`
*   **Специальные примитивы:**
    *   Seqlock (write-lock + версия), RCU (Read-Copy-Update)
    *   Lock-free Skip List (мост из `hashing.md` IV)

---

## **IX. СИНТАКСИЧЕСКИЕ СТРУКТУРЫ (ДЕРЕВЬЯ РАЗБОРА)**
> `i/i.md` · `i/i.cpp` · `struct SyntaxStructures : ConcurrentStructures`

### **1. Нотации и преобразования**
*   **Инфиксная, префиксная, постфиксная нотации:** инфиксная (стандартная), префиксная (Польская), постфиксная (обратная Польская)
*   **Shunting Yard (Dijkstra):** инфикс → постфикс `O(n)` через стек операторов; обработка приоритета и ассоциативности
*   **RPN (обратная польская нотация):** вычисление через стек `O(n)`; каждый оператор применяет два верхних элемента

### **2. Построение и обход деревьев разбора**
*   **AST (Abstract Syntax Tree):**
    *   Бинарные AST — для арифметических выражений; N-арные — для сложных конструкций
    *   Узлы: операторы (`+`, `-`, `*`, `/`, `^`), функции (`sin`, `cos`, `log`); листья: константы, переменные
    *   Свойства: приоритет операторов, ассоциативность
*   **Parse Tree (Дерево разбора):**
    *   Полное дерево с нетерминалами и терминалами; конкретное синтаксическое дерево
    *   Канонические формы: приведение к нормальной форме
*   **Expression Tree (Дерево выражений):**
    *   Специализированное AST для математических выражений
    *   Оптимизации: constant folding, common subexpression elimination
    *   Дифференцирование: построение производной выражения; упрощение: алгебраические упрощения

### **3. Построение деревьев разбора**
*   **Рекурсивный спуск (Recursive Descent Parser):** top-down; LL(k) парсеры; `O(n)` при фиксированном приоритете; построение AST
*   **Parser combinators:** функциональный подход к парсингу; композиция парсеров
*   **Operator precedence parsing (Pratt Parser):** обработка приоритетов операторов через стек; для разных приоритетов и ассоциативности
*   **LL-парсеры:** Left-to-right, Leftmost derivation
*   **LR-парсеры:** Left-to-right, Rightmost derivation
*   **Shift-Reduce парсеры:** стек как рабочая память

### **4. Трансформации и вычисления**
*   **Visitor Pattern** для обхода AST
*   **Вычисление выражений:** интерпретация AST; вычисление с переменными и функциями
*   **Компиляция** в байт-код или машинный код
*   **Дифференциальное выполнение** (automatic differentiation)
*   **Символьные вычисления** (computer algebra)
*   **Syntax highlighting:** основы подсветки через DFA

### **5. Валидация и проверка корректности**
*   **Проверка скобок:** стек `O(n)`; несколько типов скобок; обобщение на произвольные пары символов
*   **Проверка типов** (type checking)
*   **Статический анализ** выражений
*   **Обнаружение ошибок:** деление на ноль, переполнение
*   **Верификация** математических выражений

### **6. Специализированные структуры для выражений**
*   **DAG выражений:** Directed Acyclic Graph для общих подвыражений; экономия памяти
*   **Трехадресный код:** промежуточное представление
*   **SSA форма** (Static Single Assignment)
*   **Таблица символов** (Symbol Table) для переменных и функций

### **7. Применения в компиляторах и интерпретаторах**
*   **Front-end компилятора:** лексический анализ → синтаксический анализ → AST
*   **Макропроцессоры** и препроцессоры
*   **Шаблонизаторы** и системы генерации кода
*   **Доменно-специфичные языки** (DSL)
*   **Конфигурационные языки** (JSON, YAML, XML парсеры)

---

## **X. СТРУКТУРЫ ДЛЯ КЭШИРОВАНИЯ**
> `j/j.md` · `j/j.cpp` · `struct CachingStructures : SyntaxStructures`

*   **Постановка задачи кэширования:**
    *   Кэш размера `k`, поток запросов к элементам из универсума `U`; попадание/промах; цель — максимизировать число попаданий
    *   Классификация политик: детерминированные/рандомизированные, онлайн/оффлайн
    *   Оптимальная оффлайн-политика: алгоритм Белади (Belady) — выброс элемента, следующий запрос к которому дальше всего (доказательство через обмен)
    *   Конкурентное соотношение: для любой онлайн-политики `≥ k/(k+1)` от оффлайн-оптимума; LRU и FIFO — `k`-конкурентны
*   **LRU Cache (Least Recently Used):**
    *   Реализация: хеш-таблица + двусвязный список (чтобы `get`/`put` за O(1)), параметризация размера и узла
    *   Варианты: LRU с батчами, `k`-LRU, приближения (Clock)
*   **LFU Cache (Least Frequently Used):**
    *   Счётчики частот, поддержка минимума частоты (бакеты частот или heap + позиции)
    *   Устаревание частот (память о прошлом), сравнение с LRU на реальных последовательностях
*   **ARC Cache (Adaptive Replacement Cache):**
    *   Два списка T1/T2 с адаптивной настройкой (адаптивный параметр `p`)
    *   Отслеживание «истории» вытесненных элементов (B1/B2)
*   **LIRS Cache:**
    *   Признак «текущей рекурсии» (recency) против частоты: два уровня (HIR/LIR)
    *   Сравнение устойчивости к сканам (однократному чтению большого объёма)
*   **Другие политики (карта, без дублирования):**
    *   FIFO/MRU (простейшие), Clock/Second-Chance (приближение LRU для аппаратуры), 2Q (две очереди: «потенциальные» и «долгожители»)
    *   TTL-кэши (срок жизни записи), размерные кэши (политика «вместимость против ценности» — knapsack-идея)
    *   Write-back / write-through (политики записи, называют кэш-когерентность — мост в VIII)
*   **Применения:**
    *   Кэши в ОС (страницы, TLB), СУБД (буфер-пул страниц), CPU (аппаратные кэши — мост в XI cache-oblivious)
    *   Кэши вычислений: мемоизация (мост из I.F), кэши результатов функций (веб-кэши)
    *   Скользящие кэши в потоковой обработке — связь `hashing.md` (вероятностные счётчики)

---

## **XI. ТЕОРЕТИЧЕСКИЕ И ЭКЗОТИЧЕСКИЕ СТРУКТУРЫ**
> `k/k.md` · `k/k.cpp` · `struct TheoreticalStructures : CachingStructures`

*   **Малый универсум: битовые трюки (A):**
    *   Универсум `U` мал, ключ влезает в машинное слово: сравнение ключей заменяется доступом к биту
    *   Битовое множество и сортировка за `O(n + U/w)` (слово обрабатывается за `O(1)`); битовый слой — `DynamicBitset` (III.B)
*   **VEB-Tree (van Emde Boas Tree, B):**
    *   Для целых чисел из ограниченного универсума `U`
    *   Все операции за O(log log U): суммарные кластеры + минимум/максимум
    *   Нижний уровень — компактный битовый блок; vEB-раскладка массивов (cache-oblivious переиспользование рекурсии, мост в III.B и H)
*   **X-Fast Trie (C):** двоичное trie по битам ключа, уровневые хеши; `next`/`prev` за `O(log log U)` бинарным поиском по уровням; память `O(n log U)`
*   **Y-Fast Trie (D):** X-fast над представителями бакетов + Treap внутри бакета; `O(log log U)` ожидаемо (наводка X-fast + ранг в Treap), память `O(n)`; размер бакета `Θ(log U)` — параметр
*   **Fusion Tree (E):**
    *   Для целых чисел, операции за O(log_w n) (word-level parallelism)
    *   Модель Word-RAM: битовая параллельность, sketch-сжатие ключей, константы предвычислений per word
*   **Структуры динамической связности (F):**
    *   Оффлайн: дерево по времени + Rollback-DSU (мост III.A.6)
    *   ETT (Euler Tour Tree) — путь в эйлеровом обходе, как динамическое дерево
    *   Link-Cut Tree (разрез/сшивание, акцесс-операция, применение: динамические минимумы на путях) — связь `graph.md`
    *   Holm-de Lichtenberg-Thorup (амортизированный `O(log² n)` per update)
    *   Top Tree / Sleator-Tarjan (общий каркас) — упоминание
*   **Retroactive Data Structures (G):**
    *   Поддержка операций во времени (вставка/удаление операций в прошлом)
    *   Частичные (приоритетная очередь) и полные (стек) ретроактивные структуры
*   **Cache-oblivious структуры (H):**
    *   Принцип: алгоритм без знания размера кэша; оценка через идеальную модель кэша
    *   vEB-раскладка массива и рекурсивный поиск; cache-oblivious B-дерево
