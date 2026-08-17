**H. ПОТОКИ В СЕТЯХ (NETWORK FLOW)**

> **Архитектура:** `NetworkFlow` (h.cpp) — наследует `MatchingAlgorithms` (g.cpp). Реализует: Ford-Fulkerson `O(E·|f*|)`, Edmonds-Karp `O(VE²)`, Dinic `O(V²E)` / `O(√V·E)` для unit capacities, Min-Cost Max-Flow (Successive Shortest Path с потенциалами). Все алгоритмы параметризованы типом потока `F` и стоимости `C`.
>
> **Связи:** DFS/BFS — `graph/a`; shortest paths (Dijkstra/Bellman-Ford) — `graph/c`; приоритетные очереди — `struct.md` (IV); двудольные паросочетания — `graph/g` (A); min-cut — `graph/b` (C) (Stoer-Wagner); Floyd-Warshall — `graph/c` (B).

---

# **A. МАКСИМАЛЬНЫЙ ПОТОК (MAX-FLOW)**

## **1. Основные понятия**

### **1.1. Сеть (Network)**
* **Определение:** ориентированный граф `G = (V, E)` с source `s`, sink `t`, и функцией `capacity(u,v) ≥ 0`
* **Поток:** функция `flow(u,v)`, удовлетворяющая: `0 ≤ flow(u,v) ≤ capacity(u,v)` (ограничение); `Σ_{u} flow(u,v) = Σ_{w} flow(v,w)` для всех `v ≠ s, t` (баланс)
* **Величина потока:** `|f| = Σ_{v} flow(s,v) − Σ_{v} flow(v,s)` (суммарный поток из `s`)

### **1.2. Residual graph**
* **Определение:** `G_f` — граф остаточных ёмкостей: `c_f(u,v) = c(u,v) − f(u,v)` (прямое ребро) и `c_f(v,u) = f(u,v)` (обратное ребро)
* **Augmenting path:** путь от `s` до `t` в `G_f` с положительными остаточными ёмкостями
* **Свойство:** `|f|` максимальное ⟺ нет augmenting path в `G_f` (теорема о max-flow min-cut)

## **2. Ford-Fulkerson Method**

### **2.1. Алгоритм**
1. Начать с `f = 0` (нулевой поток)
2. Найти augmenting path в residual graph (DFS)
3. Увеличить поток вдоль пути на `min(остаточные_ёмкости)`
4. Повторять until augmenting path нет

### **2.2. Сложность**
* `O(E · |f*|)` где `|f*|` — величина максимального потока
* При целочисленных capacities: каждый augmenting path увеличивает поток ≥ 1 →最多 `|f*|` итераций

### **2.3. Корректность**
* Следствие max-flow min-cut: поток не может превышать capacity любого разреза; Ford-Fulkerson находит potok не меньше min-cut

## **3. Edmonds-Karp Algorithm**

### **3.1. Идея**
* Ford-Fulkerson + BFS для поиска кратчайшего augmenting path (по числу рёбер)
* **Ключевое свойство:** расстояние от `s` до `t` в residual graph неубывает

### **3.2. Сложность**
* `O(VE²)`: каждое ребро может стать «критическим»最多 `O(V)` раз; BFS `O(E)`

## **4. Dinic's Algorithm**

### **4.1. Идея (Level graph + blocking flow)**
1. **Level graph:** BFS от `s` → построение графа уровней (только рёбра следующего уровня)
2. **Blocking flow:** DFS в level graph находим максимальный поток, который невозможно увеличить без нарушения уровней
3. Повторять пока `t` достижим из `s` в level graph

### **4.2. Сложность**
* `O(V²E)` в общем случае
* `O(√V · E)` для unit capacities (все capacities = 1) — оптимально

### **4.3. Практическая эффективность**
* Один из самых быстрых на практике
* Хорошо работает на разреженных графах

## **5. Push-Relabel (Goldberg-Tarjan)**

### **5.1. Идея (проталкивание предпотока)**
* **Предпоток:** `Σ_{u} f(u,v) ≥ Σ_{w} f(v,w)` для `v ≠ s` (бывает «избыток»)
* **Высота:** `h(v)` — «уровень» вершины; `h(s) = |V|`, `h(t) = 0`
* **Push:** протолкнуть избыток из `v` в `u` если `h(v) = h(u) + 1`
* **Relabel:** увеличить `h(v)` если push невозможен

### **5.2. Сложность**
* `O(V³)` — классическая
* `O(V²√E)` — highest-label с gap heuristic
* FIFO push-relabel: `O(V³)` в худшем, хорошо на практике

---

# **B. МИНИМАЛЬНЫЙ РАЗРЕЗ (MIN-CUT)**

## **1. Теорема о максимальном потоке и минимальном разрезе**
* **Разрез (Cut):** разделение `V` на `S` и `T = V \ S` с `s ∈ S`, `t ∈ T`
* **Capacity разреза:** `cap(S,T) = Σ_{u∈S, v∈T} c(u,v)`
* **Теорема:** `max |f| = min cap(S,T)` — максимальный поток = минимальный разрез

## **2. Построение min-cut из max-flow**
* После нахождения max-flow: найти все вершины, достижимые из `s` в residual graph → `S`; остальные → `T`
* Рёбра из `S` в `T` с `c(u,v) > 0` и `f(u,v) = c(u,v)` — ребра min-cut

## **3. Приложения**
* **Image Segmentation:** минимум cost разреза между foreground/background
* **Project Selection:** минимум cost невыбранных проектов + штраф за невыбранные пары
* **Bipartite Matching:** max-flow = max matching (source → L, R → sink)

---

# **C. ПОТОК МИНИМАЛЬНОЙ СТОИМОСТИ (MIN-COST FLOW)**

## **1. Постановка задачи**
* Сеть с capacities `c(u,v)` и стоимостями `cost(u,v)` за единицу потока
* **Задача:** найти поток величины `k` с минимальной суммарной стоимостью: `Σ f(u,v)·cost(u,v)` → min

## **2. Successive Shortest Path**

### **2.1. Идея**
1. Начать с нулевого потока
2. Итеративно: найти кратчайший augmenting path в residual graph (по стоимостям) → увеличить поток
3. Повторять until поток = `k` или augmenting path нет

### **2.2. Потенциалы (для обработки отрицательных рёбер)**
* **Потенциалы:** `π(v)` — перезахват весов: `cost'(u,v) = cost(u,v) + π(u) − π(v) ≥ 0`
* **Обновление:** после каждого BFS/Dijkstra: `π(v) += dist(v)` (новый потенциал)
* **Результат:** `dist'(s,t) + π(t) − π(s)` = реальная стоимость augmenting path

### **2.3. Сложность**
* `O(F · (E + V log V))` где `F` — искомая величина потока

## **3. Cycle Canceling**
* **Идея:** начать с любого потока; искать отрицательные циклы в residual graph → увеличивать поток вдоль цикла → уменьшать стоимость
* **Сложность:** `O(V · E² · log V)` — медленнее successive shortest path

---

# **D. СПЕЦИАЛЬНЫЕ ВИДЫ ПОТОКОВ**

## **1. Циркуляция (Circulation with Demands)**
* **Задача:** для каждой вершины `v`: `Σ in(v) − Σ out(v) = d(v)` (demand: `d(v) > 0` — спрос, `d(v) < 0` — предложение)
* **Сведение к max-flow:** добавить `s` и `t`; для `d(v) > 0` → ребро `(s, v)` с capacity `d(v)`; для `d(v) < 0` → ребро `(v, t)` с capacity `−d(v)`; max-flow = Σ `d(v)`

## **2. Потоки с ограничениями (lower bounds)**
* **Задача:** `l(u,v) ≤ f(u,v) ≤ c(u,v)` (нижняя и верхняя границы)
* **Сведение:** трансформация через `f'(u,v) = f(u,v) − l(u,v)` → стандартный max-flow с дополнительными условиями

## **3. Мультипотоки (Multicommodity Flow)**
* Несколько пар (исток, сток) с разными потоками
* NP-hard в общем случае; полиномиально для планарных графов

---

# **E. ПРИМЕНЕНИЯ ПОТОКОВ**

## **1. Задача о назначениях (Assignment Problem)**
* Сведение к min-cost max-flow: source → L (cap=1, cost=0), L → R (cap=1, cost=cost[i][j]), R → sink (cap=1, cost=0)
* Hungarian algorithm — частный случай min-cost flow

## **2. Maximum Bipartite Matching**
* Сведение к max-flow: source → L (cap=1), L → R (cap=1), R → sink (cap=1)
* Max-flow = max matching

## **3. Baseball Elimination**
* Определение: может ли команда выиграть лигу
* Сведение к max-flow: source → game nodes → team nodes → sink

## **4. Airline Scheduling**
* Назначение рейсов экипажам через min-cost flow
