#ifndef TECHNIQUE_F_CPP
#define TECHNIQUE_F_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <climits>
#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include <random>
using namespace std;

// =============================================================
// VI. ТЕОРИЯ ИГР
// =============================================================
// Структура md: A. Основы (P/N позиции, Гранди, сумма игр)
//               → B. Игры с полной информацией (Minimax, Alpha-Beta, A*/IDA*, MCTS)
//               → C. Комбинаторная теория (Ним, misère, ограниченные ходы, игры на графах)
//               → D. Вероятностные игры (Expectiminimax, MDP, Q-learning, Policy Iteration)
//               → E. Классические задачи (Пятнашки, Nim, Wythoff, стоимость позиций)
//
// GameTheory наследует GreedyAlgorithms (e.cpp). Переиспользует:
//   * сортировки из I — для предобработки позиций;
//   * двоичный поиск из II — для бинарного поиска по ответу;
//   * A*/IDA* из II.F — для поиска в деревьях игр;
//   * жадные из V — для оценки ветвления;
//   * ДП из dynamic.md — для стоимости позиций, MDP;
//   * графы из graph.md — для игр на графах, топологической сортировки.
//
// Собственной арифметики не имеет — использует XOR для nim-sum,
// std::function для параметризации переходов и оценок.

#include "../e/e.cpp"

struct GameTheory : GreedyAlgorithms {

// =============================================================
// A. ОСНОВЫ КОМБИНАТОРНЫХ ИГР
// =============================================================

// --- A.1. P-позиции и N-позиции ---
// Классификация позиций: P (prevailing, выигрышная для предыдущего)
// или N (next, выигрышная для следующего).
// Вычисляется через ДП: терминальные → P; достижимые из N → P;
// все достижимые из P → N.

// --- A.2. Функция Гранди (Шпрага-Гранди) ---
// G(x) = mex{G(y) : y достижима из x}
// mex — minimum excludant, наименьшее неотрицательное целое,
// не принадлежащее множеству.
// Параметризация: функция переходов moves(pos) → vector<Pos>,
// где Pos — тип позиции (int, pair, vector и т.д.).
// Вычисление через ДП с мемоизацией.

// Пример для целочисленных позиций (номеров):
// moves(pos) → вектор достижимых позиций
template<typename Pos>
int grundy_value(Pos pos,
                 const function<vector<Pos>(Pos)>& moves,
                 unordered_map<Pos, int>& memo) {
    if (memo.count(pos)) return memo[pos];
    vector<Pos> reachable = moves(pos);
    if (reachable.empty()) {
        memo[pos] = 0;
        return 0;
    }
    unordered_set<int> values;
    for (const Pos& next : reachable)
        values.insert(grundy_value(next, moves, memo));
    int g = 0;
    while (values.count(g)) g++;
    memo[pos] = g;
    return g;
}

// Версия без мемоизацией (для одноразового вызова):
template<typename Pos>
int grundy_value(Pos pos,
                 const function<vector<Pos>(Pos)>& moves) {
    unordered_map<Pos, int> memo;
    return grundy_value(pos, moves, memo);
}

// --- A.3. Сумма игр: G(A + B) = G(A) XOR G(B) ---
// Для составных игр из независимых подигр.
// nim_sum — XOR размеров куч (классический Ним).
// Параметризация: вектор размеров куч.

// =============================================================
// C. КОМБИНАТОРНАЯ ТЕОРИЯ ИГР
// =============================================================

// --- C.8/C.18. Nim — nim_sum и is_nim_winning ---

// nim_sum: XOR всех элементов вектора.
// Параметризация: произвольный вектор значений.
int nim_sum(const vector<int>& heaps) {
    int result = 0;
    for (int h : heaps) result ^= h;
    return result;
}

// is_nim_winning: позиция выигрышна ↔ nim-sum ≠ 0.
bool is_nim_winning(const vector<int>& heaps) {
    return nim_sum(heaps) != 0;
}

// --- C.9. Misère Nim ---
// Проигрывает тот, кто делает последний ход.
// Стратегия: если все кучи ≤ 1 — parity; иначе — как normal.
bool is_misere_nim_winning(const vector<int>& heaps) {
    int xr = nim_sum(heaps);
    bool all_le_one = true;
    for (int h : heaps)
        if (h > 1) { all_le_one = false; break; }
    if (all_le_one) return xr == 0; // нечётное число куч → проигрыш (нужно оставить 0)
    return xr != 0;
}

// --- C.10. Ограниченный Nim (Bounded Nim) ---
// Каждый ход — от 1 до m элементов из одной кучи.
// G(k) = k mod (m + 1); выигрыш ↔ XOR(k_i mod (m + 1)) ≠ 0.
bool is_bounded_nim_winning(const vector<int>& heaps, int m) {
    int mod = m + 1;
    int xr = 0;
    for (int h : heaps) xr ^= (h % mod);
    return xr != 0;
}

// =============================================================
// B. ИГРЫ С ПОЛНОЙ ИНФОРМАЦИЕЙ
// =============================================================

// --- B.4. Minimax (рекурсивный на дереве игры) ---
// Параметризация:
//   state — текущее состояние (пользовательский тип);
//   children(state) → vector<State> — ходы;
//   evaluation(state) → double — оценка терминального узла;
//   is_terminal(state) → bool — проверка терминальности;
//   max_depth — ограничение глубины;
//   is_max_turn — чей ход (true = max, false = min).

template<typename State>
double minimax(const State& state,
               int depth,
               bool is_max_turn,
               const function<vector<State>(State)>& children,
               const function<double(State)>& evaluation,
               const function<bool(State)>& is_terminal,
               int max_depth = INT_MAX) {
    if (is_terminal(state) || depth >= max_depth)
        return evaluation(state);
    vector<State> kids = children(state);
    if (kids.empty())
        return evaluation(state);
    if (is_max_turn) {
        double best = -1e18;
        for (const State& child : kids)
            best = max(best, minimax(child, depth + 1, false,
                                     children, evaluation, is_terminal, max_depth));
        return best;
    } else {
        double best = 1e18;
        for (const State& child : kids)
            best = min(best, minimax(child, depth + 1, true,
                                     children, evaluation, is_terminal, max_depth));
        return best;
    }
}

// --- B.5. Alpha-Beta Pruning ---
// Отсечение ветвей через α (лучшее max) и β (лучшее min).
// При идеальном порядке ходов: O(b^{d/2}) вместо O(b^d).

template<typename State>
double alpha_beta(const State& state,
                  int depth,
                  bool is_max_turn,
                  double alpha,
                  double beta,
                  const function<vector<State>(State)>& children,
                  const function<double(State)>& evaluation,
                  const function<bool(State)>& is_terminal,
                  int max_depth = INT_MAX) {
    if (is_terminal(state) || depth >= max_depth)
        return evaluation(state);
    vector<State> kids = children(state);
    if (kids.empty())
        return evaluation(state);
    if (is_max_turn) {
        double value = -1e18;
        for (const State& child : kids) {
            double child_val = alpha_beta(child, depth + 1, false,
                                          alpha, beta,
                                          children, evaluation, is_terminal, max_depth);
            value = max(value, child_val);
            alpha = max(alpha, value);
            if (alpha >= beta) break; // β-отсечение
        }
        return value;
    } else {
        double value = 1e18;
        for (const State& child : kids) {
            double child_val = alpha_beta(child, depth + 1, true,
                                          alpha, beta,
                                          children, evaluation, is_terminal, max_depth);
            value = min(value, child_val);
            beta = min(beta, value);
            if (alpha >= beta) break; // α-отсечение
        }
        return value;
    }
}

// Версия, возвращающая лучший ход (для корневого вызова):
template<typename State>
State alpha_beta_best_move(const State& state,
                           bool is_max_turn,
                           const function<vector<State>(State)>& children,
                           const function<double(State)>& evaluation,
                           const function<bool(State)>& is_terminal,
                           int max_depth = INT_MAX) {
    vector<State> kids = children(state);
    State best_move = kids[0];
    if (is_max_turn) {
        double best_val = -1e18;
        for (const State& child : kids) {
            double val = alpha_beta(child, 1, false, best_val, 1e18,
                                    children, evaluation, is_terminal, max_depth);
            if (val > best_val) { best_val = val; best_move = child; }
        }
    } else {
        double best_val = 1e18;
        for (const State& child : kids) {
            double val = alpha_beta(child, 1, true, -1e18, best_val,
                                    children, evaluation, is_terminal, max_depth);
            if (val < best_val) { best_val = val; best_move = child; }
        }
    }
    return best_move;
}

// =============================================================
// E. КЛАССИЧЕСКИЕ ЗАДАЧИ
// =============================================================

// --- E.17. Пятнашки (15-puzzle) через IDA* ---
// 4×4 поле с плитками 1–15 и пустой клеткой.
// Эвристика Манхэттена: сумма |x_i - x_target| + |y_i - y_target|.
// Параметризация: размер поля (rows × cols), целевая конфигурация.

struct FifteenPuzzle {
    int rows, cols;
    vector<vector<int>> board;
    vector<vector<int>> target;
    int empty_r, empty_c;

    FifteenPuzzle(int r, int c, const vector<vector<int>>& b,
                  const vector<vector<int>>& t)
        : rows(r), cols(c), board(b), target(t) {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (board[i][j] == 0) { empty_r = i; empty_c = j; }
    }

    // Эвристика Манхэттена: сумма расстояний плиток до целевых позиций
    int manhattan() const {
        int h = 0;
        vector<pair<int, int>> target_pos(rows * cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                target_pos[target[i][j]] = {i, j};
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                int val = board[i][j];
                if (val != 0) {
                    auto [tr, tc] = target_pos[val];
                    h += abs(i - tr) + abs(j - tc);
                }
            }
        return h;
    }

    bool is_goal() const {
        return board == target;
    }

    // Возможные ходы: 4 направления (up, down, left, right)
    vector<FifteenPuzzle> children() const {
        vector<FifteenPuzzle> result;
        const int dr[] = {-1, 1, 0, 0};
        const int dc[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; d++) {
            int nr = empty_r + dr[d];
            int nc = empty_c + dc[d];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                FifteenPuzzle next = *this;
                swap(next.board[empty_r][empty_c], next.board[nr][nc]);
                next.empty_r = nr;
                next.empty_c = nc;
                result.push_back(next);
            }
        }
        return result;
    }

    bool operator==(const FifteenPuzzle& other) const {
        return board == other.board;
    }
};

// Хеш для unordered_map
struct PuzzleHash {
    size_t operator()(const FifteenPuzzle& p) const {
        size_t h = 0;
        for (int i = 0; i < p.rows; i++)
            for (int j = 0; j < p.cols; j++)
                h = h * 31 + p.board[i][j];
        return h;
    }
};

// IDA* для 15-puzzle
// Возвращает вектор ходов (направлений: 0=up, 1=down, 2=left, 3=right)
// или пустой вектор, если решение не найдено за max_iterations.
vector<int> fifteen_puzzle_solve(FifteenPuzzle puzzle,
                                  int max_iterations = 100) {
    if (puzzle.is_goal()) return {};

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    vector<int> path;

    function<bool(int, int)> dfs = [&](int g, int bound) -> bool {
        FifteenPuzzle state = puzzle;
        // Восстанавливаем состояние из path
        for (int dir : path) {
            int nr = state.empty_r + dr[dir];
            int nc = state.empty_c + dc[dir];
            swap(state.board[state.empty_r][state.empty_c],
                 state.board[nr][nc]);
            state.empty_r = nr;
            state.empty_c = nc;
        }

        int f = g + state.manhattan();
        if (f > bound) return false;
        if (state.is_goal()) return true;

        int min_next = INT_MAX;
        for (int d = 0; d < 4; d++) {
            int nr = state.empty_r + dr[d];
            int nc = state.empty_c + dc[d];
            if (nr >= 0 && nr < state.rows && nc >= 0 && nc < state.cols) {
                // Проверяем, не отменяем ли мы предыдущий ход
                if (!path.empty()) {
                    int last_dir = path.back();
                    int opp = (last_dir ^ 1); // 0↔1, 2↔3
                    if (d == opp) continue;
                }

                path.push_back(d);
                if (dfs(g + 1, bound)) return true;
                path.pop_back();

                // Обновляем min_next для следующего bound
                FifteenPuzzle child = state;
                swap(child.board[state.empty_r][state.empty_c],
                     child.board[nr][nc]);
                child.empty_r = nr;
                child.empty_c = nc;
                min_next = min(min_next, child.manhattan());
            }
        }
        return false;
    };

    int bound = puzzle.manhattan();
    for (int iter = 0; iter < max_iterations; iter++) {
        if (dfs(0, bound)) return path;
        bound++;
    }
    return {}; // не найдено
}

// =============================================================
// C.12 / E.19. Wythoff's Game
// =============================================================
// Два стека (a, b), a ≤ b.
// Ход: взять k > 0 из одного стека ИЛИ k > 0 из обоих.
// P-позиции: a = ⌊k · φ⌋, b = a + k, где φ = (1+√5)/2.
// Эквивалентно: k = b - a; проверяем a == ⌊k · φ⌋.

// Wythoff Nim: проверка, является ли позиция P-позицией.
// Возвращает true, если позиция (a, b) — P-позиция (проигрышная
// для того, кто ходит).
bool wythoff_nim(int a, int b) {
    if (a > b) swap(a, b);
    int k = b - a;
    double phi = (1.0 + sqrt(5.0)) / 2.0;
    int expected_a = (int)floor(k * phi);
    return a == expected_a;
}

// Wythoff Nim: вычисление P-позиции по номеру k.
// Возвращает пару (a_k, b_k) где a_k = ⌊k · φ⌋, b_k = a_k + k.
pair<int, int> wythoff_position(int k) {
    double phi = (1.0 + sqrt(5.0)) / 2.0;
    int a = (int)floor(k * phi);
    return {a, a + k};
}

// Wythoff Nim: найти номер P-позиции для заданной позиции.
// Возвращает k, если (a, b) — P-позиция, иначе -1.
int wythoff_k(int a, int b) {
    if (a > b) swap(a, b);
    int k = b - a;
    if (wythoff_nim(a, b)) return k;
    return -1;
}

// =============================================================
// D. ИГРЫ НА ГРАФАХ
// =============================================================

// --- D.11. Acyclic Game Solve (ретроспективный анализ на DAG) ---
// Вычисление G(v) для ациклического графа в обратном топологическом порядке.
// topo_order — топологический порядок вершин.
// moves(v) → vector<int> — достижимые позиции.
// G(v) = mex{G(u) : (v,u) — ребро}; терминальные: G = 0.

vector<int> acyclic_game_solve(int n,
                               const vector<int>& topo_order,
                               function<vector<int>(int)> moves) {
    vector<int> grundy(n, 0);
    // Обход в обратном топологическом порядке
    for (int i = (int)topo_order.size() - 1; i >= 0; i--) {
        int v = topo_order[i];
        auto reachable = moves(v);
        if (reachable.empty()) {
            grundy[v] = 0; // терминальная позиция
        } else {
            unordered_set<int> values;
            for (int u : reachable)
                values.insert(grundy[u]);
            int g = 0;
            while (values.count(g)) g++;
            grundy[v] = g;
        }
    }
    return grundy;
}

// Версия без топологического порядка (строит его автоматически):
vector<int> acyclic_game_solve(int n,
                               function<vector<int>(int)> moves) {
    // Топологическая сортировка
    vector<int> indeg(n, 0);
    for (int v = 0; v < n; v++)
        for (int u : moves(v)) indeg[u]++;
    queue<int> q;
    for (int v = 0; v < n; v++)
        if (indeg[v] == 0) q.push(v);
    vector<int> topo;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        topo.push_back(v);
        for (int u : moves(v)) {
            indeg[u]--;
            if (indeg[u] == 0) q.push(u);
        }
    }
    return acyclic_game_solve(n, topo, moves);
}

// --- D.12. Cyclic Game Solve (итеративное разрешение циклов) ---
// Для графов с циклами: итеративное удаление «бесполезных» вершин.
// Вершины в циклах → draw (grundy = -1).

vector<int> cyclic_game_solve(int n,
                              function<vector<int>(int)> moves) {
    vector<int> grundy(n, -1); // -1 = draw/неизвестно
    vector<int> outdeg(n, 0);
    for (int v = 0; v < n; v++) {
        auto reachable = moves(v);
        outdeg[v] = (int)reachable.size();
    }
    // Терминальные вершины → G = 0
    queue<int> q;
    for (int v = 0; v < n; v++) {
        if (outdeg[v] == 0) {
            grundy[v] = 0;
            q.push(v);
        }
    }
    // Обратный BFS: для вершин, где все достижимые — разрешены
    // вычисляем mex
    vector<vector<int>> reverse_adj(n);
    for (int v = 0; v < n; v++)
        for (int u : moves(v))
            reverse_adj[u].push_back(v);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : reverse_adj[u]) {
            if (grundy[v] != -1) continue;
            auto reachable = moves(v);
            bool all_resolved = true;
            unordered_set<int> values;
            for (int w : reachable) {
                if (grundy[w] == -1) { all_resolved = false; break; }
                values.insert(grundy[w]);
            }
            if (all_resolved) {
                int g = 0;
                while (values.count(g)) g++;
                grundy[v] = g;
                q.push(v);
            }
        }
    }
    // Оставшиеся вершины в цикалах → draw (-1)
    return grundy;
}

// =============================================================
// D2. ВЕРОЯТНОСТНЫЕ ИГРЫ
// =============================================================

// --- D.13. Expectiminimax ---
// Обобщение Minimax для игр со случайными элементами.
// Chance-узлы: value = Σ P(child) · value(child).

template<typename State>
double expectiminimax(const State& state,
                      int depth,
                      int player, // 0 = max, 1 = min, 2 = chance
                      const function<vector<pair<State, double>>(State)>& chance_children,
                      const function<vector<State>(State)>& deterministic_children,
                      const function<double(State)>& evaluation,
                      const function<bool(State)>& is_terminal,
                      int max_depth = INT_MAX) {
    if (is_terminal(state) || depth >= max_depth)
        return evaluation(state);

    if (player == 2) {
        // Chance-узел: взвешенное среднее
        auto kids = chance_children(state);
        double value = 0.0;
        for (auto& [child, prob] : kids)
            value += prob * expectiminimax(child, depth + 1, 0,
                                           chance_children, deterministic_children,
                                           evaluation, is_terminal, max_depth);
        return value;
    } else {
        // Deterministic ходы
        auto kids = deterministic_children(state);
        if (kids.empty()) return evaluation(state);
        if (player == 0) {
            double best = -1e18;
            for (const State& child : kids)
                best = max(best, expectiminimax(child, depth + 1, 1,
                                                chance_children, deterministic_children,
                                                evaluation, is_terminal, max_depth));
            return best;
        } else {
            double best = 1e18;
            for (const State& child : kids)
                best = min(best, expectiminimax(child, depth + 1, 0,
                                                chance_children, deterministic_children,
                                                evaluation, is_terminal, max_depth));
            return best;
        }
    }
}

// --- D.14. MDP: Value Iteration ---
// V_{k+1}(s) = max_a Σ_{s'} P(s'|s,a) · (R(s,a,s') + γ · V_k(s'))
// Параметризация: пространство состояний, действий, переходов, наград.

// Состояние — int (индекс), действие — int (индекс).
// transitions(s, a) → vector<pair<int, double>>: (s', probability)
// reward(s, a, s') → double

vector<double> value_iteration(int num_states,
                               int num_actions,
                               function<vector<pair<int, double>>(int, int)> transitions,
                               function<double(int, int, int)> reward,
                               double gamma,
                               int max_iterations,
                               double eps = 1e-8) {
    vector<double> V(num_states, 0.0);
    for (int iter = 0; iter < max_iterations; iter++) {
        vector<double> V_new(num_states);
        double max_diff = 0.0;
        for (int s = 0; s < num_states; s++) {
            double best = -1e18;
            for (int a = 0; a < num_actions; a++) {
                double q = 0.0;
                for (auto& [s_next, prob] : transitions(s, a))
                    q += prob * (reward(s, a, s_next) + gamma * V[s_next]);
                best = max(best, q);
            }
            V_new[s] = best;
            max_diff = max(max_diff, abs(V_new[s] - V[s]));
        }
        V = V_new;
        if (max_diff < eps) break;
    }
    return V;
}

// --- D.15. Q-Learning ---
// model-free RL: Q(s,a) ← Q(s,a) + α · (r + γ · max_{a'} Q(s', a') − Q(s,a))
// Параметризация: число состояний, действий, learning rate α, discount γ,
// exploration rate ε, число эпизодов.

vector<vector<double>> q_learning(int num_states,
                                  int num_actions,
                                  function<int(int, int)> transition, // (s, a) → s'
                                  function<double(int, int, int)> reward, // (s, a, s') → r
                                  function<bool(int)> is_terminal, // s → bool
                                  double alpha,
                                  double gamma,
                                  double epsilon,
                                  int num_episodes,
                                  int max_steps_per_episode) {
    mt19937 rng(random_device{}());
    uniform_real_distribution<double> uniform(0.0, 1.0);
    uniform_int_distribution<int> action_dist(0, num_actions - 1);

    vector<vector<double>> Q(num_states, vector<double>(num_actions, 0.0));

    for (int ep = 0; ep < num_episodes; ep++) {
        // Случайное начальное состояние (не терминальное)
        int s;
        do { s = rng() % num_states; } while (is_terminal(s));

        for (int step = 0; step < max_steps_per_episode; step++) {
            // ε-greedy выбор действия
            int a;
            if (uniform(rng) < epsilon)
                a = action_dist(rng);
            else {
                a = 0;
                for (int a2 = 1; a2 < num_actions; a2++)
                    if (Q[s][a2] > Q[s][a]) a = a2;
            }

            int s_next = transition(s, a);
            double r = reward(s, a, s_next);

            // Обновление Q
            double max_q_next = 0.0;
            if (!is_terminal(s_next))
                for (int a2 = 0; a2 < num_actions; a2++)
                    max_q_next = max(max_q_next, Q[s_next][a2]);

            Q[s][a] += alpha * (r + gamma * max_q_next - Q[s][a]);

            if (is_terminal(s_next)) break;
            s = s_next;
        }
    }
    return Q;
}

// --- D.16. Policy Iteration ---
// 1) Policy Evaluation: вычислить V^π(s)
// 2) Policy Improvement: обновить π(s) = argmax_a Σ P(s'|s,a)(R + γV)
// Гарантия сходимости за конечное число итераций.

// policy(s) → int (действие)
// Возвращает оптимальную политику (вектор действий для каждого состояния).

vector<int> policy_iteration(int num_states,
                             int num_actions,
                             function<vector<pair<int, double>>(int, int)> transitions,
                             function<double(int, int, int)> reward,
                             double gamma,
                             int max_iterations) {
    // Начальная политика: случайная (или детерминированная)
    vector<int> policy(num_states, 0);

    for (int iter = 0; iter < max_iterations; iter++) {
        // 1. Policy Evaluation: решаем V^π через итерации
        vector<double> V(num_states, 0.0);
        for (int eval_iter = 0; eval_iter < 100; eval_iter++) {
            vector<double> V_new(num_states);
            for (int s = 0; s < num_states; s++) {
                int a = policy[s];
                double q = 0.0;
                for (auto& [s_next, prob] : transitions(s, a))
                    q += prob * (reward(s, a, s_next) + gamma * V[s_next]);
                V_new[s] = q;
            }
            V = V_new;
        }

        // 2. Policy Improvement
        bool changed = false;
        for (int s = 0; s < num_states; s++) {
            double best_val = -1e18;
            int best_a = policy[s];
            for (int a = 0; a < num_actions; a++) {
                double q = 0.0;
                for (auto& [s_next, prob] : transitions(s, a))
                    q += prob * (reward(s, a, s_next) + gamma * V[s_next]);
                if (q > best_val) { best_val = q; best_a = a; }
            }
            if (best_a != policy[s]) {
                policy[s] = best_a;
                changed = true;
            }
        }
        if (!changed) break; // политика стабилизировалась
    }
    return policy;
}

// =============================================================
// E. СТОИМОСТЬ ПОЗИЦИЙ ЧЕРЕЗ ДП
// =============================================================
// Для игр с конечным числом позиций: стоимость позиции —
// оптимальный результат при optimal play обеих сторон.
// Рекурсия: V(terminal) = evaluation; V(state) = max/min V(child).
// Для acyclic игр — прямой DP; для cyclic — решение системы.

// DP на DAG для acyclic игр:
// topo_order — топологический порядок вершин (из I.D.22).
// children(s) → вектор достижимых позиций.
// evaluation(s) → оценка для терминальных позиций.
// is_max(s) → true если max-ход, false если min.

vector<double> position_value_dag(int num_states,
                                  const vector<int>& topo_order,
                                  function<vector<int>(int)> children,
                                  function<double(int)> evaluation,
                                  function<bool(int)> is_terminal,
                                  function<bool(int)> is_max) {
    vector<double> V(num_states, 0.0);
    // Обходим в обратном топологическом порядке
    for (int i = (int)topo_order.size() - 1; i >= 0; i--) {
        int s = topo_order[i];
        if (is_terminal(s)) {
            V[s] = evaluation(s);
        } else {
            auto kids = children(s);
            if (is_max(s)) {
                double best = -1e18;
                for (int c : kids) best = max(best, V[c]);
                V[s] = best;
            } else {
                double best = 1e18;
                for (int c : kids) best = min(best, V[c]);
                V[s] = best;
            }
        }
    }
    return V;
}

// =============================================================
// K. MCTS (Monte Carlo Tree Search) с UCB1
// =============================================================
// Этапы: Selection → Expansion → Simulation → Backpropagation
// UCB1(child) = Q(child)/N(child) + C · sqrt(ln(N(parent))/N(child))

struct MCTSNode {
    int state;
    double total_reward;
    int visits;
    vector<MCTSNode*> children;
    MCTSNode* parent;
    bool expanded;
    vector<int> untried_moves;

    MCTSNode(int s, MCTSNode* p = nullptr) : state(s), total_reward(0.0),
        visits(0), parent(p), expanded(false) {}
};

// mcts_search: параметризуемая функция симуляции.
// states — множество состояний; children(s) → ходы (состояния);
// simulate(s) → double (результат симуляции); is_terminal(s) → bool;
// num_iterations — число итераций MCTS; C — константа UCB1.
// Возвращает индекс лучшего действия (первого ребёнка корня).

int mcts_search(int root_state,
                int num_states,
                function<vector<int>(int)> children,
                function<int(int, int)> apply_move, // (state, move_index) → new_state
                function<double(int)> simulate,
                function<bool(int)> is_terminal,
                int num_iterations,
                double C = 1.414) {
    MCTSNode root(root_state);
    root.untried_moves.resize(num_states);
    iota(root.untried_moves.begin(), root.untried_moves.end(), 0);

    for (int iter = 0; iter < num_iterations; iter++) {
        MCTSNode* node = &root;

        // 1. Selection: спуск по дереву с максимальным UCB1
        while (!node->expanded && !node->untried_moves.empty()) {
            // Expansion: если есть неисследованные ходы — добавляем ребёнка
            int move = node->untried_moves.back();
            node->untried_moves.pop_back();
            int child_state = apply_move(node->state, move);
            MCTSNode* child = new MCTSNode(child_state, node);
            child->untried_moves.resize(num_states);
            iota(child->untried_moves.begin(), child->untried_moves.end(), 0);
            node->children.push_back(child);
            node = child;
            break;
        }

        // Selection продолжается: спуск через лучшего ребёнка по UCB1
        while (!node->expanded && node->children.empty() == false) {
            MCTSNode* best_child = nullptr;
            double best_ucb = -1e18;
            for (MCTSNode* c : node->children) {
                double ucb;
                if (c->visits == 0) {
                    ucb = 1e18; // не посещённые — в приоритете
                } else {
                    double exploitation = c->total_reward / c->visits;
                    double exploration = C * sqrt(log((double)node->visits) / (double)c->visits);
                    ucb = exploitation + exploration;
                }
                if (ucb > best_ucb) { best_ucb = ucb; best_child = c; }
            }
            if (best_child == nullptr) break;
            node = best_child;
        }

        // 2. Expansion: добавляем нового ребёнка, если есть неисследованные ходы
        if (!node->expanded && !node->untried_moves.empty()) {
            int move = node->untried_moves.back();
            node->untried_moves.pop_back();
            int child_state = apply_move(node->state, move);
            MCTSNode* child = new MCTSNode(child_state, node);
            child->untried_moves.resize(num_states);
            iota(child->untried_moves.begin(), child->untried_moves.end(), 0);
            node->children.push_back(child);
            node = child;
        }

        // 3. Simulation (Rollout): случайная игра до конца
        double reward = simulate(node->state);

        // 4. Backpropagation
        while (node != nullptr) {
            node->visits++;
            node->total_reward += reward;
            node = node->parent;
        }
    }

    // Выбираем лучшего ребёнка корня по числу посещений
    MCTSNode* best = nullptr;
    int best_visits = -1;
    for (MCTSNode* c : root.children) {
        if (c->visits > best_visits) {
            best_visits = c->visits;
            best = c;
        }
    }
    // Освобождаем память
    function<void(MCTSNode*)> cleanup = [&](MCTSNode* n) {
        for (MCTSNode* c : n->children) cleanup(c);
        if (n != &root) delete n;
    };
    cleanup(&root);
    return best ? best->state : root_state;
}

}; // struct GameTheory

#endif // TECHNIQUE_F_CPP
