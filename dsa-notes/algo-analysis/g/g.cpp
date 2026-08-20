#ifndef ALGO_ANALYSIS_G_CPP
#define ALGO_ANALYSIS_G_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <bitset>
#include <climits>
using namespace std;

// =============================================================
// G. NP-ТЕОРИЯ И КЛАССЫ СЛОЖНОСТИ
// =============================================================
// Структура md: A. Основные классы
//               → B. NP-полные задачи
//               → C. Методы работы
//
// NPTheory — наследует SpaceComplexity (f.cpp).
// P/NP/NP-complete, SAT, TSP, Vertex Cover, Clique,
// приближения, FPT, meet-in-the-middle.

#ifndef INSIDE_ALGO_ANALYSIS_G
#define INSIDE_ALGO_ANALYSIS_G
#include "../f/f.cpp"
#endif

struct NPTheory : SpaceComplexity {

// =============================================================
// B. NP-ПОЛНЫЕ ЗАДАЧИ (модели)
// =============================================================

// --- B.1. 3-SAT: brute-force проверка ---
// data: вектор троек (a, b, c) — литералы клоз.
// Пример: (1, -2, 3) означает (x1 ∨ ¬x2 ∨ x3).
// Возвращает true, если существует оценка.
bool sat_3_brute(const vector<tuple<int,int,int>>& clauses, int n_vars) {
    for (int mask = 0; mask < (1 << n_vars); mask++) {
        bool all_sat = true;
        for (auto& [a, b, c] : clauses) {
            bool va = (a > 0) ? (bool)(mask & (1 << (a - 1))) : !(mask & (1 << (-a - 1)));
            bool vb = (b > 0) ? (bool)(mask & (1 << (b - 1))) : !(mask & (1 << (-b - 1)));
            bool vc = (c > 0) ? (bool)(mask & (1 << (c - 1))) : !(mask & (1 << (-c - 1)));
            if (!(va || vb || vc)) { all_sat = false; break; }
        }
        if (all_sat) return true;
    }
    return false;
}

// --- B.2. TSP: brute-force (перебор перестановок) ---
double tsp_brute(const vector<vector<double>>& dist) {
    int n = (int)dist.size();
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i;
    double best = 1e18;
    do {
        double cost = 0;
        for (int i = 0; i < n; i++)
            cost += dist[perm[i]][perm[(i + 1) % n]];
        best = min(best, cost);
    } while (next_permutation(perm.begin(), perm.end()));
    return best;
}

// --- B.3. TSP: жадный (nearest neighbor) ---
double tsp_greedy(const vector<vector<double>>& dist) {
    int n = (int)dist.size();
    vector<bool> visited(n, false);
    visited[0] = true;
    double total = 0;
    int current = 0;
    for (int i = 1; i < n; i++) {
        int next = -1;
        double best = 1e18;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && dist[current][j] < best) {
                best = dist[current][j];
                next = j;
            }
        }
        visited[next] = true;
        total += best;
        current = next;
    }
    total += dist[current][0];
    return total;
}

// --- B.4. Vertex Cover: 2-approximation ---
// Жадный: находим ребро, добавляем оба конца, удаляем все смежные.
int vertex_cover_2approx(const vector<pair<int,int>>& edges, int n) {
    vector<bool> covered(n, false);
    int count = 0;
    for (auto& [u, v] : edges) {
        if (!covered[u] && !covered[v]) {
            covered[u] = true;
            covered[v] = true;
            count += 2;
        }
    }
    return count;
}

// =============================================================
// C. FPT И ЭКСПОНЕНЦИАЛЬНЫЕ АЛГОРИТМЫ
// =============================================================

// --- C.1. Vertex Cover: FPT O(2^k · n) ---
// Рекурсивный: если есть ребро (u,v) без покрытия — пробуем добавить u или v.
int vertex_cover_fpt(const vector<pair<int,int>>& edges, int n, int k,
                     vector<bool>& covered) {
    // Находим непокрытое ребро
    int u = -1, v = -1;
    for (auto& [eu, ev] : edges) {
        if (!covered[eu] && !covered[ev]) { u = eu; v = ev; break; }
    }
    if (u == -1) return true;  // все рёбра покрыты
    if (k == 0) return false;  // нет ресурсов

    // Вариант 1: добавить u
    covered[u] = true;
    if (vertex_cover_fpt(edges, n, k - 1, covered)) return true;
    covered[u] = false;

    // Вариант 2: добавить v
    covered[v] = true;
    if (vertex_cover_fpt(edges, n, k - 1, covered)) return true;
    covered[v] = false;

    return false;
}

// --- C.2. Meet-in-the-Middle: Subset Sum ---
// Разбиваем на 2 половины, генерируем все суммы, ищем互补.
bool meet_in_the_middle_subset_sum(const vector<int>& a, int target) {
    int n = (int)a.size();
    int mid = n / 2;
    vector<long long> left_sums;
    for (int mask = 0; mask < (1 << mid); mask++) {
        long long sum = 0;
        for (int i = 0; i < mid; i++)
            if (mask & (1 << i)) sum += a[i];
        left_sums.push_back(sum);
    }
    sort(left_sums.begin(), left_sums.end());

    int right_size = n - mid;
    for (int mask = 0; mask < (1 << right_size); mask++) {
        long long sum = 0;
        for (int i = 0; i < right_size; i++)
            if (mask & (1 << i)) sum += a[mid + i];
        long long need = target - sum;
        if (binary_search(left_sums.begin(), left_sums.end(), need))
            return true;
    }
    return false;
}

}; // struct NPTheory

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_G_MAIN
int main() {
    NPTheory np;
    srand(42);

    cout << "=== B. NP-ПОЛНЫЕ ЗАДАЧИ ===" << endl;

    cout << "--- 3-SAT: brute-force (O(2ⁿ)) ---" << endl;
    {
        // (x1 ∨ x2 ∨ x3) ∧ (¬x1 ∨ x2 ∨ ¬x3) ∧ (x1 ∨ ¬x2 ∨ x3)
        vector<tuple<int,int,int>> clauses = {{1,2,3}, {-1,2,-3}, {1,-2,3}};
        bool sat = np.sat_3_brute(clauses, 3);
        cout << "  3 переменных, 3 клозы: " << (sat ? "SAT" : "UNSAT") << endl;

        // UNSAT: (x1) ∧ (¬x1)
        vector<tuple<int,int,int>> unsat = {{1,1,1}, {-1,-1,-1}};
        cout << "  UNSAT пример: " << (np.sat_3_brute(unsat, 1) ? "SAT" : "UNSAT") << endl;

        // Замер времени
        for (int n : {10, 15, 20}) {
            vector<tuple<int,int,int>> random_clauses;
            for (int i = 0; i < 3 * n; i++) {
                int a = (rand() % n) + 1, b = (rand() % n) + 1, c = (rand() % n) + 1;
                if (rand() % 2) a = -a;
                if (rand() % 2) b = -b;
                if (rand() % 2) c = -c;
                random_clauses.push_back({a, b, c});
            }
            auto start = chrono::high_resolution_clock::now();
            bool result = np.sat_3_brute(random_clauses, n);
            auto end = chrono::high_resolution_clock::now();
            auto t = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "  n=" << n << ", " << random_clauses.size() << " клоз: "
                 << (result ? "SAT" : "UNSAT") << " (" << t << " ms)" << endl;
        }
    }

    cout << "\n--- TSP: brute-force vs greedy ---" << endl;
    {
        int n = 8;
        vector<vector<double>> dist(n, vector<double>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                dist[i][j] = (i == j) ? 0 : 1 + rand() % 100;

        auto start1 = chrono::high_resolution_clock::now();
        double opt = np.tsp_brute(dist);
        auto end1 = chrono::high_resolution_clock::now();
        auto t1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1).count();

        auto start2 = chrono::high_resolution_clock::now();
        double greedy = np.tsp_greedy(dist);
        auto end2 = chrono::high_resolution_clock::now();
        auto t2 = chrono::duration_cast<chrono::microseconds>(end2 - start2).count();

        cout << "  n=" << n << " городов:" << endl;
        cout << "    Optimal (brute-force): " << opt << " (" << t1 << " ms)" << endl;
        cout << "    Greedy (nearest): " << greedy << " (" << t2 << " μs)" << endl;
        cout << "    Ratio: " << greedy / opt << "x" << endl;
    }

    cout << "\n--- Vertex Cover: 2-approx ---" << endl;
    {
        // Путь: 0-1-2-3-4 (4 ребра)
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,4}};
        int n = 5;
        int approx = np.vertex_cover_2approx(edges, n);
        cout << "  Путь 0-1-2-3-4:" << endl;
        cout << "    2-approx: " << approx << " вершин (оптимум: 2)" << endl;
    }

    cout << "\n=== C. FPT И ЭКСПОНЕНЦИАЛЬНЫЕ АЛГОРИТМЫ ===" << endl;

    cout << "--- Vertex Cover: FPT O(2^k · n) ---" << endl;
    {
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,4},{0,4}};
        int n = 5;
        for (int k = 1; k <= 4; k++) {
            vector<bool> covered(n, false);
            bool found = np.vertex_cover_fpt(edges, n, k, covered);
            if (found) {
                int count = 0;
                for (bool c : covered) if (c) count++;
                cout << "  k=" << k << ": найден покрытие размера " << count << endl;
            } else {
                cout << "  k=" << k << ": нет покрытия" << endl;
            }
        }
    }

    cout << "\n--- Meet-in-the-Middle: Subset Sum ---" << endl;
    {
        vector<int> a = {3, 7, 1, 8, 4, 6, 2, 5};
        for (int target : {10, 15, 20, 25}) {
            auto start = chrono::high_resolution_clock::now();
            bool found = np.meet_in_the_middle_subset_sum(a, target);
            auto end = chrono::high_resolution_clock::now();
            auto t = chrono::duration_cast<chrono::microseconds>(end - start).count();
            cout << "  target=" << target << ": " << (found ? "YES" : "NO") << " (" << t << " μs)" << endl;
        }
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_G_CPP
