## **8. ТЕОРИЯ ГРАФОВ**

### **I. БАЗОВЫЕ ПОНЯТИЯ, ПРЕДСТАВЛЕНИЯ И ОБХОДЫ**

#### **A. ОСНОВНЫЕ ОПРЕДЕЛЕНИЯ И КЛАССИФИКАЦИЯ**
*   **Типы графов:**
    *   Directed/Undirected (ориентированные/неориентированные)
    *   Weighted/Unweighted (взвешенные/невзвешенные)
    *   Simple/Multigraph (простые/мультиграфы)
    *   Complete/Empty (полные/пустые)
    *   Bipartite/General (двудольные/общие)
    *   Connected/Disconnected (связные/несвязные)
    *   Acyclic/Cyclic (ациклические/циклические)
    *   All Types of Graphs, Basic Graphs
    *   Directed And Undirected Weighted Graph

#### **B. ПРЕДСТАВЛЕНИЯ ГРАФОВ В ПАМЯТИ**
*   **Списки смежности (Adjacency List):**
    *   Graph Adjacency List
    *   Dynamic adjacency lists
    *   Compressed adjacency lists
*   **Матрица смежности (Adjacency Matrix):**
    *   Graph Adjacency Matrix
    *   Bit matrix representation
*   **Список рёбер (Edge List):**
    *   Graph List
    *   Sorted edge list
*   **Специализированные представления:**
    *   **CSR/CSC** (Compressed Sparse Row/Column)
    *   **Forward Star**
    *   **Incidence Matrix**
    *   **Implicit Graph Representation**
*   **Сравнение представлений:**
    *   Space complexity
    *   Time complexity для различных операций

#### **C. ПОИСК В ГЛУБИНУ (DEPTH-FIRST SEARCH - DFS)**
*   **Базовые реализации:**
    *   Depth First Search
    *   Depth First Search 2
    *   Recursive/Iterative DFS
*   **Применения DFS:**
    *   **Проверка на циклы:**
        *   Check Cycle в ориентированных графах
        *   Check Cycle в неориентированных графах
    *   **Топологическая сортировка:**
        *   G Topological Sort
        *   Kahns Algorithm Topo
        *   Kahns Algorithm Long
    *   **Компоненты связности:**
        *   Connected Components (Undirected)
        *   Strongly Connected Components (Directed)
    *   **Копирование графов:**
        *   Deep Clone Graph
    *   **Другие применения:**
        *   Articulation Points (точки сочленения)
        *   Finding Bridges (мосты)
        *   Bipartite checking через DFS
*   **Продвинутые техники DFS:**
    *   **Iterative Deepening DFS (IDDFS)**
    *   **DFS with timestamping** (discovery/finish times)
    *   **DFS Tree/DFS Forest**
    *   **Back Edges, Forward Edges, Cross Edges**

#### **D. ПОИСК В ШИРИНУ (BREADTH-FIRST SEARCH - BFS)**
*   **Базовые реализации:**
    *   Breadth First Search
    *   Breadth First Search 2
*   **BFS для кратчайших путей:**
    *   Breadth First Search Shortest Path
    *   Breadth First Search Shortest Path 2
    *   Breadth First Search Zero One Shortest Path
    *   Unweighted shortest paths
*   **Продвинутые варианты BFS:**
    *   **Bidirectional BFS:**
        *   Bidirectional BFS
        *   Bidirectional Search
        *   Meet-in-the-middle search
    *   **0-1 BFS:**
        *   BFS 0-1 для графов с рёбрами веса 0/1
        *   Использование deque
    *   **BFS 0-k:**
        *   Для графов с ограниченными весами рёбер
    *   **Многопоточный BFS:**
        *   Parallel BFS
        *   Level-synchronous BFS
    *   **BFS на state-space graphs**
*   **Применения BFS:**
    *   Shortest path в невзвешенных графах
    *   Connected components
    *   Bipartite checking
    *   Web crawling

### **II. СВЯЗНОСТЬ В ГРАФАХ**

#### **A. КОМПОНЕНТЫ СВЯЗНОСТИ**
*   **Для неориентированных графов:**
    *   Connected Components
    *   Counting connected components
    *   **Biconnected Components** (двусвязные компоненты):
        *   Двусвязность через точки сочленения
        *   Алгоритм Тарьяна для biconnected components
    *   **Edge Biconnected Components**
*   **Для ориентированных графов:**
    *   **Strongly Connected Components (SCC):**
        *   Kosaraju's Algorithm
        *   Tarjan's Algorithm
        *   Strongly Connected Components
        *   Condensation graph (конденсация графа)
    *   **Weakly Connected Components**
*   **Поиск компонент:**
    *   DFS/BFS based approaches
    *   Union-Find для динамической связности

#### **B. МОСТЫ И ТОЧКИ СОЧНЕНИЯ (BRIDGES AND ARTICULATION POINTS)**
*   **Мосты (Bridges):**
    *   Finding Bridges
    *   Алгоритм Тарьяна для поиска мостов
    *   Мосты в ориентированных графах
*   **Точки сочленения (Articulation Points):**
    *   Articulation Points
    *   Алгоритм Тарьяна для точек сочленения
    *   Critical nodes в сетях
*   **Online алгоритмы для мостов:**
    *   Динамическое поддержание мостов за O(log n)
    *   Link-Cut Trees для динамических мостов
    *   Поиск мостов за O(1) в режиме онлайн (fully dynamic)

#### **C. К-СВЯЗНОСТЬ И МИНИМАЛЬНЫЕ РАЗРЕЗЫ**
*   **Вершинная связность (Vertex Connectivity):**
    *   k-вершинная связность
    *   Теорема Менгера
    *   Алгоритмы поиска минимального вершинного разреза
*   **Рёберная связность (Edge Connectivity):**
    *   k-рёберная связность
    *   Минимальный разрез по рёбрам
    *   **Алгоритм Каргера:**
        *   Karger's Algorithm (рандомизированный)
        *   Karger-Stein Algorithm
        *   Для нахождения минимального разреза
*   **Глобальная минимальная связность:**
    *   Нахождение k-связности графа
    *   Проверка k-вершинной/рёберной связности

#### **D. ДВУДОЛЬНЫЕ ГРАФЫ И РАСКРАСКИ**
*   **Проверка двудольности:**
    *   Check Bipatrite
    *   DFS/BFS based bipartite checking
*   **Раскраски графов:**
    *   Coloring
    *   **Vertex Coloring:**
        *   Greedy coloring
        *   Welsh-Powell algorithm
        *   Brelaz's heuristic
        *   Chromatic number
    *   **Edge Coloring:**
        *   Покраска рёбер дерева
        *   Vizing's theorem
        *   Edge chromatic number
    *   **Особые раскраски:**
        *   Acyclic coloring
        *   Star coloring
        *   Harmonious coloring
*   **Теоремы о раскрасках:**
    *   Four Color Theorem (для планарных графов)
    *   Five Color Theorem
    *   Brooks' Theorem

#### **E. ДИНАМИЧЕСКАЯ СВЯЗНОСТЬ**
*   **Incremental Connectivity:**
    *   Добавление рёбер
    *   Union-Find с откатами
*   **Decremental Connectivity:**
    *   Удаление рёбер
    *   Spanning forest maintenance
*   **Fully Dynamic Connectivity:**
    *   Добавление и удаление рёбер
    *   Holmgren's algorithm
    *   Euler Tour Trees (ETT)
    *   Link-Cut Trees

#### **F. 2-SAT И КОНДЕНСАЦИЯ ГРАФА ИМПЛИКАЦИЙ**
*   **2-SAT Problem:**
    *   Сведение к ориентированному графу
    *   Построение графа импликаций
    *   Алгоритм за O(N+M)
*   **Решение 2-SAT:**
    *   Поиск SCC в графе импликаций
    *   Построение решения
    *   Нахождение всех решений
*   **Применения 2-SAT:**
    *   Scheduling problems
    *   Circuit design
    *   Resource allocation

### **III. КРАТЧАЙШИЕ ПУТИ В ГРАФАХ**

#### **A. ОДНОИСТОЧНИКОВЫЕ КРАТЧАЙШИЕ ПУТИ (SSSP)**
*   **Для невзвешенных графов:**
    *   BFS (уже в разделе I)
*   **Для неотрицательных весов (Dijkstra):**
    *   **Алгоритм Дейкстры:**
        *   Dijkstra
        *   Dijkstra 2
        *   Dijkstra Algorithm
        *   Dijkstra Alternate
        *   Dijkstra Binary Grid
        *   Биекционная Дейкстра:
            *   Bi Directional Dijkstra
    *   **Реализации:**
        *   Priority Queue based
        *   Fibonacci Heap optimization
        *   Dial's implementation (для малых целых весов)
*   **Для произвольных весов (Bellman-Ford):**
    *   **Алгоритм Форда-Беллмана:**
        *   Standard Bellman-Ford
        *   Shortest Path Faster Algorithm (SPFA)
        *   Detection of negative cycles
    *   **Алгоритм Левита:**
        *   Levit's Algorithm (для смешанных графов)
        *   Использование трёх очередей
*   **Для DAG (Directed Acyclic Graph):**
    *   Topological sort + DP
    *   Longest path в DAG

#### **B. КРАТЧАЙШИЕ ПУТИ МЕЖДУ ВСЕМИ ПАРАМИ (APSP)**
*   **Алгоритм Флойда-Уоршелла:**
    *   Floyd Warshall
    *   Transitive closure
    *   Detection of negative cycles
*   **Алгоритм Джонсона:**
    *   Johnson's Algorithm
    *   Для разреженных графов с отрицательными весами
*   **Специальные алгоритмы:**
    *   **Matrix Multiplication method:**
        *   Min-plus matrix multiplication
        *   Repeated squaring
    *   **Подсчёт путей фиксированной длины:**
        *   Через возведение матрицы смежности в степень
*   **Для разреженных графов:**
    *   Run Dijkstra from each vertex
    *   Optimization tricks

#### **C. ЭВРИСТИЧЕСКИЕ АЛГОРИТМЫ ПОИСКА ПУТИ**
*   **A* Search Algorithm:**
    *   A-star
    *   Bidirectional A Star
    *   Multi Heuristic Astar
    *   Admissible/Consistent heuristics
*   **Greedy Best First Search:**
    *   Greedy Best First
*   **IDEA* (Iterative Deepening A*)**
*   **D* и D* Lite** (для динамических сред)

#### **D. СПЕЦИАЛЬНЫЕ ВИДЫ ПУТЕЙ**
*   **k-кратчайших путей:**
    *   Yen's algorithm
    *   Eppstein's algorithm
*   **Наиболее надежные пути**
*   **Пути с ограничениями** (по времени, ресурсам)

### **IV. ОСТОВНЫЕ ДЕРЕВЬЯ (SPANNING TREES)**

#### **A. МИНИМАЛЬНОЕ ОСТОВНОЕ ДЕРЕВО (MST)**
*   **Алгоритм Крускала:**
    *   Minimum Spanning Tree Kruskal
    *   Minimum Spanning Tree Kruskal2 (with DSU)
    *   Test Min Spanning Tree Kruskal
    *   Union-Find optimization
*   **Алгоритм Прима:**
    *   Prim
    *   Minimum Spanning Tree Prims
    *   Minimum Spanning Tree Prims2
    *   Test Min Spanning Tree Prim
    *   Fibonacci Heap optimization
*   **Алгоритм Борувки:**
    *   Boruvka
    *   Minimum Spanning Tree Boruvka
    *   Parallel Boruvka
*   **Сравнение алгоритмов:**
    *   Time/space complexity
    *   Применимость для разных типов графов

#### **B. ВТОРОЕ ПО МИНИМАЛЬНОСТИ ОСТОВНОЕ ДЕРЕВО**
*   **Second-best MST:**
    *   Алгоритм за O(M log N)
    *   На основе MST + LCA
*   **Minimum Bottleneck Spanning Tree (MBST)**
*   **Minimum Spanning Forest** (для несвязных графов)

#### **C. ТЕОРЕТИЧЕСКИЕ ОСНОВЫ**
*   **Матричная теорема Кирхгофа:**
    *   Kirchhoff's Matrix Tree Theorem
    *   Подсчёт остовных деревьев
    *   Laplacian matrix
*   **Код Прюфера:**
    *   Prüfer sequence
    *   Взаимно однозначное соответствие деревьев и последовательностей
    *   Применения в подсчёте
*   **Формула Кэли:**
    *   Cayley's formula: n^(n-2) деревьев на n вершинах
    *   Обобщения формулы Кэли

#### **D. ОБРАТНЫЕ ЗАДАЧИ И ОПТИМИЗАЦИИ**
*   **Обратная задача MST:**
    *   Задан граф и дерево, сделать его MST с минимальными изменениями
*   **Steiner Tree Problem:**
    *   Минимальное дерево, соединяющее заданное подмножество вершин
    *   Приближённые алгоритмы
*   **Degree-constrained MST**
*   **Capacitated MST**

### **V. ЭЙЛЕРОВЫ И ГАМИЛЬТОНОВЫ ГРАФЫ**

#### **A. ЭЙЛЕРОВЫ ПУТИ И ЦИКЛЫ**
*   **Основные определения:**
    *   Эйлеров цикл/путь
    *   Эйлеров/полуэйлеров граф
*   **Критерии эйлеровости:**
    *   Для неориентированных графов
    *   Для ориентированных графов
*   **Алгоритмы поиска:**
    *   **Алгоритм Флёри** (Fleury's algorithm)
    *   **Алгоритм Хьерхольцера** (Hierholzer's algorithm)
    *   Рекурсивные алгоритмы
*   **Применения:**
    *   **Задача китайского почтальона:**
        *   Chinese Postman Problem
        *   Для неориентированных/ориентированных графов
    *   DNA sequencing
    *   Network routing

#### **B. ГАМИЛЬТОНОВЫ ПУТИ И ЦИКЛЫ**
*   **Основные определения:**
    *   Гамильтонов цикл/путь
    *   Гамильтонов/полугамильтонов граф
*   **Достаточные условия:**
    *   Теорема Дирака
    *   Теорема Оре
    *   Теорема Поша
    *   Теорема Бонди-Хватала
*   **Алгоритмы поиска:**
    *   **Алгоритм Робертса-Флореса** (backtracking)
    *   **Алгоритм Хелда-Карпа** (DP with bitmask)
    *   Метод ветвей и границ
*   **NP-полнота задачи**
*   **Приближённые алгоритмы**
*   **Частные случаи:**
    *   Турниры (ориентированные полные графы)
    *   Планарные графы
    *   Решётки (grid graphs)

#### **C. ОБОБЩЁННЫЕ ЗАДАЧИ**
*   **Longest Path Problem:**
    *   В произвольных графах (NP-hard)
    *   В DAG (полиномиально)
*   **Traveling Salesman Problem (TSP):**
    *   Классическая формулировка
    *   Метрический TSP
    *   Алгоритмы: DP (Held-Karp), Approximation (Christofides)

### **VI. ДЕРЕВЬЯ И ИХ ДЕКОМПОЗИЦИИ**

#### **A. ОСНОВНЫЕ АЛГОРИТМЫ НА ДЕРЕВЬЯХ**
*   **Обходы деревьев:**
    *   DFS/BFS обходы
    *   Pre-order, In-order, Post-order
    *   Level-order
*   **Базовые запросы:**
    *   Even Tree
    *   Minimum Path Sum
    *   Diameter of tree
    *   Центр(ы) дерева
*   **Подсчёт поддеревьев:**
    *   Количество поддеревьев с условиями
    *   DP на деревьях

#### **B. НАИМЕНЬШИЙ ОБЩИЙ ПРЕДОК (LOWEST COMMON ANCESTOR - LCA)**
*   **Бинарные подъёмы (Binary Lifting):**
    *   O(N log N) предподсчёт, O(log N) запрос
    *   Поддержка различных запросов на пути
*   **Сведение к RMQ:**
    *   Euler Tour + RMQ
    *   **Алгоритм Фарах-Колтона и Бендера:**
        *   RMQ за O(N) предподсчёт, O(1) запрос
        *   ±1 RMQ для деревьев
*   **Метод эйлерова обхода:**
    *   Euler Tour Technique
    *   Преобразование дерева в массив
*   **Tarjan's Offline LCA:**
    *   O(N + Q) для оффлайн запросов
    *   Union-Find based
*   **Динамическое LCA:**
    *   Link-Cut Trees
    *   Для динамически изменяющихся деревьев

#### **C. ДЕКОМПОЗИЦИИ ДЕРЕВЬЕВ**
*   **Heavy-Light Decomposition (HLD):**
    *   Разбиение дерева на тяжёлые/лёгкие пути
    *   Запросы на путях за O(log² N)
    *   Дерево отрезков на HLD
*   **Центроидная декомпозиция:**
    *   Centroid Decomposition
    *   Разбиение дерева на центроиды
    *   Запросы на расстояниях
*   **Лестничная декомпозиция:**
    *   Ladder Decomposition
    *   Для LCA и запросов на путях
*   **Моноидная декомпозиция**

#### **D. ПРОДВИНУТЫЕ АЛГОРИТМЫ НА ДЕРЕВЬЯХ**
*   **DSU on Tree (Sack):**
    *   Merge small-to-large technique
    *   Запросы на поддеревьях за O(N log N)
*   **Rerooting DP:**
    *   Переподвешивание корня
    *   DP на дереве с пересчётом при смене корня
*   **Virtual Trees:**
    *   Построение virtual tree по набору вершин
    *   Сжатие путей
*   **Tree Isomorphism:**
    *   AHU algorithm
    *   Tree canonization

### **VII. ПАРОСОЧЕТАНИЯ В ГРАФАХ**

#### **A. ДВУДОЛЬНЫЕ ПАРОСОЧЕТАНИЯ**
*   **Теорема Холла (Hall's Marriage Theorem)**
*   **Алгоритм Куна (Kuhn's Algorithm):**
    *   DFS-based matching
    *   O(VE) complexity
*   **Алгоритм Хопкрофта-Карпа:**
    *   Hopcroft-Karp Algorithm
    *   O(E√V) complexity
    *   BFS для поиска augmenting paths
*   **Maximum matching в двудольных графах:**
    *   Gale Shapley Bigraph (Stable Marriage)
    *   Weighted bipartite matching

#### **B. ПАРОСОЧЕТАНИЯ В ОБЩИХ ГРАФАХ**
*   **Теорема Татта (Tutte's Theorem)**
*   **Алгоритм Эдмондса (Blossom Algorithm):**
    *   Edmonds' Blossom Algorithm
    *   O(V³) implementation
    *   Blossom contraction
*   **Матрица Татта (Tutte Matrix):**
    *   Алгебраический подход
    *   Рандомизированный алгоритм
*   **Maximum matching за O(√V E)**

#### **C. ВЗВЕШЕННЫЕ ПАРОСОЧЕТАНИЯ**
*   **Взвешенное паросочетание в двудольных графах:**
    *   Hungarian Algorithm
    *   O(V³) implementation
*   **Взвешенное паросочетание в общих графах:**
    *   Edmonds' algorithm for weighted matching
    *   Blossom V algorithm

#### **D. СМЕЖНЫЕ ПОНЯТИЯ**
*   **Вершинные покрытия:**
    *   Matching Min Vertex Cover
    *   Greedy Min Vertex Cover
    *   König's theorem (для двудольных графов)
*   **Покрытие путями в DAG:**
    *   Dilworth's theorem
    *   Минимальное покрытие путями
*   **Maximum Independent Set:**
    *   В двудольных графах
    *   В общих графах (NP-hard)

### **VIII. ПОТОКИ В СЕТЯХ (NETWORK FLOW)**

#### **A. МАКСИМАЛЬНЫЙ ПОТОК (MAX-FLOW)**
*   **Алгоритм Форда-Фалкерсона:**
    *   Ford-Fulkerson method
    *   Edmonds-Karp variant: O(VE²)
    *   Edmonds Karp Multiple Source And Sink
*   **Алгоритм Диница:**
    *   Dinic's Algorithm
    *   O(V²E) или O(√V E) для unit capacities
    *   Level graph + blocking flow
*   **Push-Relabel Алгоритмы:**
    *   **Метод проталкивания предпотока:**
        *   Goldberg-Tarjan algorithm
        *   O(V³) или O(V²√E)
    *   **Highest-label push-relabel**
    *   **FIFO push-relabel**
*   **Максимальный поток с динамическими обновлениями**

#### **B. МИНИМАЛЬНЫЙ РАЗРЕЗ (MIN-CUT)**
*   **Теорема о максимальном потоке и минимальном разрезе**
*   **Minimum Cut:**
    *   Global min-cut (Stoer-Wagner algorithm)
    *   s-t min-cut
*   **Приложения min-cut:**
    *   Image segmentation
    *   Project selection

#### **C. ПОТОК МИНИМАЛЬНОЙ СТОИМОСТИ (MIN-COST FLOW)**
*   **Successive Shortest Path:**
    *   With potentials (Johnson's reweighting)
    *   O(F E log V) где F - величина потока
*   **Cycle Canceling:**
    *   Klein's cycle canceling algorithm
    *   Negative cycle detection
*   **Minimum Cost Maximum Flow**
*   **Cost Scaling Algorithms**

#### **D. СПЕЦИАЛЬНЫЕ ВИДЫ ПОТОКОВ**
*   **Циркуляция:**
    *   Circulation with demands
    *   Циркуляция минимальной стоимости
*   **Мультипотоки (Multicommodity Flow):**
    *   Maximum concurrent flow
    *   Minimum cost multicommodity flow
*   **Потоки с ограничениями:**
    *   Edge capacities
    *   Vertex capacities
    *   Lower bounds on flow
*   **Потоки в планарных графах:**
    *   Min-cut в планарных графах за O(N log N)

#### **E. ПРИМЕНЕНИЯ ПОТОКОВ**
*   **Задача о назначениях (Assignment Problem):**
    *   Сведение к min-cost flow
    *   Hungarian algorithm как частный случай
*   **Maximum Bipartite Matching** (как частный случай max-flow)
*   **Minimum Path Cover** в DAG
*   **Baseball Elimination**
*   **Airline Scheduling**

### **IX. ПЛАНАРНЫЕ ГРАФЫ И АЛГОРИТМЫ**

#### **A. ОСНОВНЫЕ СВОЙСТВА ПЛАНАРНЫХ ГРАФОВ**
*   **Определения и критерии:**
    *   Euler's formula: V - E + F = 2
    *   Kuratowski's theorem
    *   Planarity testing algorithms
*   **Представления планарных графов:**
    *   Planar embedding
    *   Dual graph
    *   Straight-line drawing
*   **Разделение планарных графов:**
    *   Planar separator theorem (Lipton-Tarjan)
    *   Applications in divide-and-conquer

#### **B. АЛГОРИТМЫ НА ПЛАНАРНЫХ ГРАФАХ**
*   **Кратчайшие пути:**
    *   Dijkstra с special heaps для планарных графов
    *   Multiple-source shortest paths
*   **Минимальный разрез в планарных графах**
*   **Maximum flow в планарных графах:**
    *   Сведение к shortest path в dual graph
*   **Planar embedding algorithms**

#### **C. ГРАФЫ НА ПОВЕРХНОСТЯХ**
*   **Графы на торе и других поверхностях**
*   **Графы с ограниченным genus**

### **X. СПЕЦИАЛЬНЫЕ КЛАССЫ ГРАФОВ И АЛГОРИТМЫ**

#### **A. ГРАФЫ С ОГРАНИЧЕННЫМИ ПАРАМЕТРАМИ**
*   **Графы с ограниченной treewidth:**
    *   Tree decomposition
    *   DP on tree decomposition
    *   Courcelle's theorem
*   **Графы с ограниченной clique-width**
*   **Графы с ограниченной степенью (degree-bounded)**
*   **Интервальные графы:**
    *   Maximum clique/independent set
    *   Interval scheduling
*   **Графы перестановок (permutation graphs)**

#### **B. ГРАФЫ В СТРОКАХ И СЕQUENCE ANALYSIS**
*   **Суффиксные структуры:**
    *   **Алгоритм Укконена** для построения суффиксного дерева
    *   Suffix automaton
    *   Suffix array + LCP
*   **De Bruijn Graphs** для genome assembly
*   **Overlap graphs**

#### **C. ГИПЕРГРАФЫ И МУЛЬТИГРАФЫ**
*   **Гиперграфы:**
    *   Представления
    *   Transversal/hitting set
*   **Мультиграфы**
*   **Directed hypergraphs**

### **XI. ВЕРОЯТНОСТНЫЕ И ЭВРИСТИЧЕСКИЕ МЕТОДЫ**

#### **A. ВЕРОЯТНОСТНЫЕ АЛГОРИТМЫ**
*   **Random Walks:**
    *   Markov Chain
    *   Stationary distribution
    *   Mixing time
*   **PageRank:**
    *   Page Rank algorithm
    *   Random surfer model
    *   Applications in web search
*   **Случайные графы:**
    *   Erdős–Rényi model
    *   Random Graph Generator
    *   Properties of random graphs

#### **B. МЕТАЭВРИСТИКИ И ПОИСК С ОГРАНИЧЕНИЯМИ**
*   **Ant Colony Optimization:**
    *   Ant Colony Optimization Algorithms
    *   Pheromone update strategies
*   **Genetic Algorithms для графов**
*   **Simulated Annealing**
*   **Tabu Search**

#### **C. ЛОКАЛЬНЫЙ ПОИСК И ЖАДНЫЕ АЛГОРИТМЫ**
*   **Greedy algorithms для графовых задач**
*   **Local search heuristics**
*   **Approximation algorithms:**
    *   Vertex Cover (2-approximation)
    *   Metric TSP (1.5-approximation)
    *   Set Cover (log n-approximation)

### **XII. ПРИКЛАДНЫЕ АЛГОРИТМЫ НА ГРАФАХ**

#### **A. DATA MINING И АНАЛИЗ СЕТЕЙ**
*   **Frequent Pattern Mining:**
    *   Frequent Pattern Graph Miner
    *   Subgraph isomorphism
*   **Community Detection:**
    *   Girvan-Newman algorithm
    *   Modularity optimization
    *   Louvain method
*   **Centrality Measures:**
    *   Betweenness centrality
    *   Closeness centrality
    *   Eigenvector centrality
*   **Link Prediction**

#### **B. СОЦИАЛЬНЫЕ СЕТИ И WEB ГРАФЫ**
*   **Small-world networks**
*   **Scale-free networks**
*   **Web graph algorithms:**
    *   HITS algorithm
    *   SALSA

#### **C. ЛИНЕЙНАЯ АЛГЕБРА ДЛЯ ГРАФОВ**
*   **Спектральная теория графов:**
    *   Собственные значения матрицы смежности
    *   Lanczos Eigenvectors algorithm
    *   Spectral clustering
*   **Laplacian matrix:**
    *   Fiedler vector
    *   Graph partitioning
*   **Incidence matrix applications**

#### **D. ГРАФОВЫЕ БАЗЫ ДАННЫХ И СИСТЕМЫ**
*   **Graph databases:**
    *   Query languages (Cypher, Gremlin)
    *   Indexing techniques
*   **Distributed graph processing:**
    *   Pregel model
    *   Graph processing frameworks

### **XIII. ДИНАМИЧЕСКИЕ И ОНЛАЙН АЛГОРИТМЫ**

#### **A. ДИНАМИЧЕСКИЕ ГРАФОВЫЕ АЛГОРИТМЫ**
*   **Dynamic Connectivity:**
    *   Fully dynamic connectivity
    *   Euler Tour Trees
    *   Link-Cut Trees
*   **Dynamic Shortest Paths:**
    *   Incremental/decremental
    *   Fully dynamic
*   **Dynamic MST**

#### **B. ОНЛАЙН АЛГОРИТМЫ**
*   **Online Matching**
*   **Online Steiner Tree**
*   **Competitive analysis**
