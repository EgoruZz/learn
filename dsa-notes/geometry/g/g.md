**G. ПРОСТРАНСТВЕННЫЕ СТРУКТУРЫ**

> **Архитектура:** `SpatialStructures` (g.cpp) — наследует `ComputationalGeometry` (f.cpp). Реализует: KD-tree (k-NN, nearest neighbor), Quadtree, ближайшую пару точек, диаграмму Вороного (базовая). Все методы параметризованы и работают через `Point` и `double`.
>
> **Связи:** `ComputationalGeometry` (f) — point_in_polygon, convex hull; `VectorAlgebra` (b) — расстояния; `struct.md` (IV) — priority queue; `struct.md` (V) — дерево отрезков (для Range Tree).

---

# **1. KD-tree**
*   Двоичное дерево разбиения по координатам (чередование осей); построение `O(n log n)`
*   **k-ближайших соседей (k-NN):** `k_nearest(root, target, k)` — через приоритетную очередь `O(√n)` в среднем
*   **Nearest neighbor:** `nearest_neighbor(root, target)` — рекурсивный обход с отсечением `O(log n)` в среднем

# **2. Quadtree**
*   Разбиение пространства на 4 части; для разреженных данных; range queries

# **3. Ближайшая пара точек**
*   Divide & Conquer `O(n log n)`: разбиение по медиане x; проверка полосы шириной `2d`

# **4. Диаграмма Вороного**
*   Разбиение по ближайшему центру `O(n log n)`; двойственный граф Делоне
*   **Метрики:** Евклидова, L₁ (Манхэттен), L∞ (Чебышёв)
