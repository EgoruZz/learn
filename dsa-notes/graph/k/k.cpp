#ifndef GRAPH_K_CPP
#define GRAPH_K_CPP

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <set>
using namespace std;

// =============================================================
// K. ВЕРОЯТНОСТНЫЕ И ЭВРИСТИЧЕСКИЕ МЕТОДЫ
// =============================================================
// Структура md: A. Вероятностные алгоритмы
//               → B. Метаэвристики
//
// ProbabilisticGraph — наследует SpecialGraphs (j.cpp).
// Реализует: PageRank, Random Walk, Erdős–Rényi,
// Simulated Annealing, Genetic Algorithm.

#ifndef INSIDE_GRAPH_K
#define INSIDE_GRAPH_K
#include "../j/j.cpp"
#undef INSIDE_GRAPH_K
#endif

struct ProbabilisticGraph : SpecialGraphs {

// =============================================================
// A. VEROYATNOSTNIE ALGORITMY
// =============================================================

// --- A.1. PageRank (power method) ---
// adj — списки смежности (направленный граф); d — damping factor.
// Возвращает: vector<double> page_rank.
// O(m · iterations) время.
vector<double> pagerank(const vector<vector<int>>& adj, double d = 0.85,
                        int max_iter = 100, double eps = 1e-6) {
    int n = adj.size();
    vector<double> pr(n, 1.0 / n);
    vector<double> pr_new(n);

    for (int iter = 0; iter < max_iter; iter++) {
        for (int v = 0; v < n; v++)
            pr_new[v] = (1.0 - d) / n;

        for (int v = 0; v < n; v++) {
            if (adj[v].empty()) continue;
            double share = pr[v] / adj[v].size();
            for (int u : adj[v])
                pr_new[u] += d * share;
        }

        double diff = 0;
        for (int v = 0; v < n; v++)
            diff += abs(pr_new[v] - pr[v]);
        pr = pr_new;

        if (diff < eps) break;
    }
    return pr;
}

// --- A.2. Random Walk ---
// adj — списки смежности; steps — число шагов; start — начальная вершина.
// Возвращает: визиты каждой вершины.
vector<int> random_walk(const vector<vector<int>>& adj, int steps,
                        int start = 0, unsigned seed = 42) {
    int n = adj.size();
    mt19937 rng(seed);
    vector<int> visits(n, 0);
    int current = start;

    for (int i = 0; i <= steps; i++) {
        visits[current]++;
        if (adj[current].empty()) break;
        uniform_int_distribution<int> dist(0, adj[current].size() - 1);
        current = adj[current][dist(rng)];
    }
    return visits;
}

// --- A.3. Генерация Erdős–Rényi графа G(n, p) ---
// Возвращает: adjacency list.
vector<vector<int>> erdos_renyi(int n, double p, unsigned seed = 42) {
    mt19937 rng(seed);
    uniform_real_distribution<double> dist(0.0, 1.0);
    vector<vector<int>> adj(n);

    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (dist(rng) < p) {
                adj[i].push_back(j);
                adj[j].push_back(i);
            }
    return adj;
}

// =============================================================
// B. METAHEVristIKI
// =============================================================

// --- B.1. Simulated Annealing ---
// cost(x) — функция стоимости; neighbors(x) — соседние состояния.
// Возвращает: {лучшая_стоимость, лучшее_состояние}.
// Параметры: T0 — начальная температура, alpha — коэффициент охлаждения,
// iterations — число итераций на каждую температуру.
template<typename State>
pair<double, State> simulated_annealing(
    State initial,
    function<double(const State&)> cost,
    function<State(const State&, mt19937&)> neighbors,
    double T0 = 1000.0, double alpha = 0.995,
    int iterations_per_temp = 100, int max_temps = 1000,
    unsigned seed = 42) {
    mt19937 rng(seed);
    uniform_real_distribution<double> prob(0.0, 1.0);

    State current = initial;
    double current_cost = cost(current);
    State best = current;
    double best_cost = current_cost;
    double T = T0;

    for (int temp = 0; temp < max_temps; temp++) {
        for (int iter = 0; iter < iterations_per_temp; iter++) {
            State next = neighbors(current, rng);
            double next_cost = cost(next);
            double delta = next_cost - current_cost;

            if (delta < 0 || prob(rng) < exp(-delta / T)) {
                current = next;
                current_cost = next_cost;
                if (current_cost < best_cost) {
                    best = current;
                    best_cost = current_cost;
                }
            }
        }
        T *= alpha;
        if (T < 1e-10) break;
    }
    return {best_cost, best};
}

// --- B.2. Genetic Algorithm ---
// population_size, generations, crossover_rate, mutation_rate.
// encode/decode — кодирование/декодирование особи.
// Возвращает: {лучший_фитнес, лучшая_особь}.
template<typename Gene>
pair<double, vector<Gene>> genetic_algorithm(
    int pop_size, int gene_size, int generations,
    double crossover_rate, double mutation_rate,
    function<double(const vector<Gene>&)> fitness,
    function<vector<Gene>(mt19937&)> random_individual,
    unsigned seed = 42) {
    mt19937 rng(seed);

    // Инициализация популяции
    vector<vector<Gene>> population(pop_size);
    for (int i = 0; i < pop_size; i++)
        population[i] = random_individual(rng);

    auto evaluate = [&](const vector<Gene>& ind) { return fitness(ind); };

    for (int gen = 0; gen < generations; gen++) {
        // Оценка фитнеса
        vector<double> fits(pop_size);
        for (int i = 0; i < pop_size; i++)
            fits[i] = evaluate(population[i]);

        // Турнирный отбор
        auto tournament = [&](int k = 3) -> int {
            int best_idx = rng() % pop_size;
            for (int i = 1; i < k; i++) {
                int idx = rng() % pop_size;
                if (fits[idx] > fits[best_idx]) best_idx = idx;
            }
            return best_idx;
        };

        vector<vector<Gene>> new_pop(pop_size);
        for (int i = 0; i < pop_size; i++) {
            int p1 = tournament(), p2 = tournament();
            vector<Gene> child(gene_size);

            // Order Crossover (OX) для перестановок
            uniform_real_distribution<double> coin(0.0, 1.0);
            if (coin(rng) < crossover_rate) {
                int a = rng() % gene_size, b = rng() % gene_size;
                if (a > b) swap(a, b);
                // Копируем сегмент из p1
                set<Gene> used;
                for (int j = a; j <= b; j++) {
                    child[j] = population[p1][j];
                    used.insert(population[p1][j]);
                }
                // Заполняем остаток из p2 в порядке
                int pos = (b + 1) % gene_size;
                for (int j = 0; j < gene_size; j++) {
                    Gene g = population[p2][(b + 1 + j) % gene_size];
                    if (used.count(g) == 0) {
                        child[pos] = g;
                        pos = (pos + 1) % gene_size;
                    }
                }
            } else {
                child = population[p1];
            }

            // Мутация: swap двух случайных городов
            if (coin(rng) < mutation_rate) {
                int a = rng() % gene_size, b = rng() % gene_size;
                swap(child[a], child[b]);
            }

            new_pop[i] = child;
        }
        population = new_pop;
    }

    // Лучшая особь
    double best_fit = -1e18;
    vector<Gene> best_ind;
    for (int i = 0; i < pop_size; i++) {
        double f = evaluate(population[i]);
        if (f > best_fit) { best_fit = f; best_ind = population[i]; }
    }
    return {best_fit, best_ind};
}

// --- B.3. Пример: GA для TSP ---
// Ген: перестановка городов; фитнес = −расстояние (максимизируем).
pair<double, vector<int>> tsp_genetic(
    const vector<vector<int>>& dist, int pop_size = 100,
    int generations = 500, unsigned seed = 42) {
    int n = dist.size();

    auto fitness = [&](const vector<int>& tour) -> double {
        double total = 0;
        for (int i = 0; i < n; i++)
            total += dist[tour[i]][tour[(i + 1) % n]];
        return -total;  // maximize negative distance
    };

    auto random_tour = [&](mt19937& rng) -> vector<int> {
        vector<int> tour(n);
        iota(tour.begin(), tour.end(), 0);
        shuffle(tour.begin(), tour.end(), rng);
        return tour;
    };

    return genetic_algorithm<int>(
        pop_size, n, generations, 0.8, 0.1,
        fitness, random_tour, seed);
}

}; // struct ProbabilisticGraph

// =============================================================
// MAIN
// =============================================================
#ifdef GRAPH_K_MAIN
int main() {
    ProbabilisticGraph pg;

    cout << "=== PageRank ===" << endl;
    // Простой DAG: 0→1, 0→2, 1→2, 2→3
    vector<vector<int>> adj(4);
    adj[0] = {1, 2}; adj[1] = {2}; adj[2] = {3};
    auto pr = pg.pagerank(adj);
    for (int i = 0; i < 4; i++)
        cout << "PR(" << i << ") = " << pr[i] << endl;

    cout << "\n=== Random Walk ===" << endl;
    vector<vector<int>> adj2 = {{1,2},{0,2},{0,1}};
    auto visits = pg.random_walk(adj2, 1000, 0);
    int total = accumulate(visits.begin(), visits.end(), 0);
    for (int i = 0; i < 3; i++)
        cout << "v" << i << ": " << visits[i] << "/" << total
             << " (" << (double)visits[i]/total << ")" << endl;

    cout << "\n=== Erdős–Rényi ===" << endl;
    auto er = pg.erdos_renyi(10, 0.3);
    int edges = 0;
    for (int i = 0; i < 10; i++) edges += er[i].size();
    cout << "G(10, 0.3): " << edges/2 << " edges" << endl;

    cout << "\n=== Simulated Annealing (minimize x² + y²) ===" << endl;
    using State = pair<double,double>;
    function<double(const State&)> cost_sa = [](const State& s) {
        return s.first*s.first + s.second*s.second;
    };
    function<State(const State&, mt19937&)> neighbors_sa =
        [](const State& s, mt19937& rng) -> State {
            normal_distribution<double> dist(0.0, 0.5);
            return {s.first + dist(rng), s.second + dist(rng)};
        };
    auto [sa_cost, sa_state] = pg.simulated_annealing(
        State{5.0, 5.0}, cost_sa, neighbors_sa, 100, 0.99, 100, 500);
    cout << "Best: (" << sa_state.first << ", " << sa_state.second
         << ") cost=" << sa_cost << endl;

    cout << "\n=== Genetic Algorithm (TSP, 5 cities) ===" << endl;
    vector<vector<int>> tsp_dist = {
        {0, 10, 15, 20, 25},
        {10, 0, 35, 25, 20},
        {15, 35, 0, 30, 15},
        {20, 25, 30, 0, 10},
        {25, 20, 15, 10, 0}
    };
    auto [ga_fit, ga_tour] = pg.tsp_genetic(tsp_dist, 50, 300);
    cout << "Best tour cost: " << -ga_fit << endl;
    cout << "Tour: ";
    for (int v : ga_tour) cout << v << " ";
    cout << ga_tour[0] << endl;

    return 0;
}
#endif

#endif // GRAPH_K_CPP
