#ifndef TECHNIQUE_E_CPP
#define TECHNIQUE_E_CPP

#include "../d/d.cpp"

struct GreedyAlgorithms : DataCompression {

// =============================================================
// V. ЖАДНЫЕ АЛГОРИТМЫ
// =============================================================
// Структура md: A. Теоретические основы (матроиды, оптимальная подструктура)
//               → B. Задачи расписаний (Activity Selection, Minimum Waiting Time,
//                  Optimal Merge Pattern, Smallest Range, Johnson one/two machines,
//                  Optimal selection with deadlines)
//               → C. Размен монет и покупки (Coin Change, Buy/Sell Stock I/II, Gas Station)
//               → D. Рюкзак и покрытие (Fractional Knapsack I/II, Set Cover,
//                  Fractional Cover, Recursive Knapsack, Kadane's Algorithm)
//
// GreedyAlgorithms наследует DataCompression (d.cpp). Переиспользует:
//   * сортировки из I — предобработка для Activity Selection, Fractional Knapsack;
//   * std::priority_queue (struct.md IV) — для Optimal Merge, Smallest Range;
//   * бинарный поиск (II) — для проверки решений в олимпиадных задачах.
//
// Все методы параметризованы: компараторы, функции веса, предикаты.

// =============================================================
// B. ЗАДАЧИ РАСПИСАНИЙ
// =============================================================

// --- B.3. Activity Selection ---
// Сортировка по endTime →贪心 выбор активностей, не пересекающихся с выбранными.
// O(n log n) время, O(n) доп. памяти для хранения результата.
// Параметры: start — начала, end — концы активностей.
// Возвращает индексы выбранных активностей.
vector<int> activity_selection(const vector<int>& start, const vector<int>& end) {
    int n = (int)start.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    // сортировка по endTime
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return end[a] < end[b];
    });
    vector<int> selected;
    int last_end = INT_MIN;
    for (int i : idx) {
        if (start[i] >= last_end) {
            selected.push_back(i);
            last_end = end[i];
        }
    }
    return selected;
}

// Параметризованная версия с произвольным компаратором для endTime.
vector<int> activity_selection_cmp(const vector<int>& start, const vector<int>& end,
                                   const function<bool(int, int)>& cmp_end) {
    int n = (int)start.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return cmp_end(end[a], end[b]);
    });
    vector<int> selected;
    int last_end = INT_MIN;
    for (int i : idx) {
        if (start[i] >= last_end) {
            selected.push_back(i);
            last_end = end[i];
        }
    }
    return selected;
}

// --- B.4. Minimum Waiting Time ---
// Сортировка по времени выполнения (SJF) → минимизация суммарного ожидания.
// O(n log n) время, O(1) доп. памяти.
// Параметр: dur[i] — время выполнения i-й задачи.
// Возвращает минимальное суммарное время ожидания.
long long minimum_waiting_time(vector<int> dur) {
    sort(dur.begin(), dur.end());
    long long total_wait = 0;
    long long prefix_sum = 0;
    for (int i = 0; i < (int)dur.size() - 1; i++) {
        prefix_sum += dur[i];
        total_wait += prefix_sum;
    }
    return total_wait;
}

// Параметризованная версия: произвольный компаратор для времени выполнения.
long long minimum_waiting_time_cmp(vector<int> dur,
                                   const function<bool(int, int)>& cmp) {
    sort(dur.begin(), dur.end(), cmp);
    long long total_wait = 0;
    long long prefix_sum = 0;
    for (int i = 0; i < (int)dur.size() - 1; i++) {
        prefix_sum += dur[i];
        total_wait += prefix_sum;
    }
    return total_wait;
}

// --- B.5. Optimal Merge Pattern ---
// Всегда сливаем два наименьших файла (аналог дерева Хаффмана, IV.B).
// O(n log n) через priority_queue (переиспользуем кучу из struct.md IV).
// Параметр: sizes[i] — размер i-го файла.
// Возвращает минимальную стоимость слияний.
long long optimal_merge_pattern(vector<int> sizes) {
    // min-heap
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int s : sizes) pq.push(s);
    long long total_cost = 0;
    while (pq.size() > 1) {
        int a = pq.top(); pq.pop();
        int b = pq.top(); pq.pop();
        long long merged = (long long)a + b;
        total_cost += merged;
        pq.push((int)merged);
    }
    return total_cost;
}

// Параметризованная версия: произвольный компаратор для кучи.
long long optimal_merge_pattern_cmp(vector<int> sizes,
                                    const function<bool(int, int)>& cmp) {
    priority_queue<int, vector<int>, decltype(cmp)> pq(cmp);
    for (int s : sizes) pq.push(s);
    long long total_cost = 0;
    while (pq.size() > 1) {
        int a = pq.top(); pq.pop();
        int b = pq.top(); pq.pop();
        long long merged = (long long)a + b;
        total_cost += merged;
        pq.push((int)merged);
    }
    return total_cost;
}

// --- B.6. Smallest Range Covering Elements from K Lists ---
// k отсортированных списков → минимальный диапазон [a, b], содержащий по элементу из каждого.
// O(N log k) через min-heap, где N — суммарное число элементов.
// Возвращает pair<int, int> — границы диапазона.
pair<int, int> smallest_range(const vector<vector<int>>& lists) {
    int k = (int)lists.size();
    // min-heap: (value, list_index, element_index)
    using T = tuple<int, int, int>;
    priority_queue<T, vector<T>, greater<T>> pq;
    int cur_max = INT_MIN;
    for (int i = 0; i < k; i++) {
        if (!lists[i].empty()) {
            pq.push({lists[i][0], i, 0});
            cur_max = max(cur_max, lists[i][0]);
        }
    }
    int best_range = INT_MAX;
    int best_a = INT_MIN, best_b = INT_MAX;
    while (!pq.empty()) {
        auto [val, li, ei] = pq.top(); pq.pop();
        int cur_min = val;
        if (cur_max - cur_min < best_range) {
            best_range = cur_max - cur_min;
            best_a = cur_min;
            best_b = cur_max;
        }
        if (ei + 1 < (int)lists[li].size()) {
            int next_val = lists[li][ei + 1];
            pq.push({next_val, li, ei + 1});
            cur_max = max(cur_max, next_val);
        } else {
            break; // один из списков исчерпан
        }
    }
    return {best_a, best_b};
}

// --- B.7. Johnson One Machine ---
// Минимизация суммарного времени ожидания на одном станке (SJF).
// Сортировка по убыванию dur → O(n log n).
// Возвращает минимальное суммарное время ожидания.
long long johnson_one_machine(vector<int> dur) {
    sort(dur.begin(), dur.end());
    long long total_wait = 0;
    long long prefix_sum = 0;
    for (int i = 0; i < (int)dur.size() - 1; i++) {
        prefix_sum += dur[i];
        total_wait += prefix_sum;
    }
    return total_wait;
}

// --- B.8. Johnson Two Machines ---
// Правило Джонсона: G1 = {i : a[i] ≤ b[i]} (по возрастанию a),
//                   G2 = {i : a[i] > b[i]} (по убыванию b).
// Финальный порядок = G1 + G2. Makespan = max completing time.
// O(n log n) время.
// Возвращает порядок выполнения заданий (индексы).
vector<int> johnson_two_machines(const vector<int>& a, const vector<int>& b) {
    int n = (int)a.size();
    vector<int> g1, g2;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) g1.push_back(i);
        else g2.push_back(i);
    }
    sort(g1.begin(), g1.end(), [&](int i, int j) { return a[i] < a[j]; });
    sort(g2.begin(), g2.end(), [&](int i, int j) { return b[i] > b[j]; });
    vector<int> order;
    order.reserve(n);
    for (int i : g1) order.push_back(i);
    for (int i : g2) order.push_back(i);
    return order;
}

// Вычисление Makespan для заданного порядка.
long long makespan_two_machines(const vector<int>& a, const vector<int>& b,
                                const vector<int>& order) {
    long long time_a = 0, time_b = 0;
    for (int i : order) {
        time_a += a[i];
        if (time_b < time_a) time_b = time_a;
        time_b += b[i];
    }
    return time_b;
}

// =============================================================
// C. РАЗМЕН МОНЕТ И ПОКУПКИ
// =============================================================

// --- C.7. Minimum Coin Change (жадный) ---
// Жадный: всегда берём максимальный номинал ≤ остатка.
// Корректен только для канонических систем номиналов (каждый кратен предыдущему).
// O(n log n) сортировка + O(n) проход. n — число номиналов.
// Возвращает минимальное число монет (или -1, если невозможно).
int minimum_coin_change_greedy(vector<int> coins, int target) {
    sort(coins.rbegin(), coins.rend()); // по убыванию
    int count = 0;
    int remaining = target;
    for (int c : coins) {
        while (remaining >= c) {
            remaining -= c;
            count++;
        }
    }
    return (remaining == 0) ? count : -1;
}

// Параметризованная версия: произвольный компаратор для номиналов.
int minimum_coin_change_greedy_cmp(vector<int> coins, int target,
                                   const function<bool(int, int)>& cmp) {
    sort(coins.begin(), coins.end(), cmp);
    int count = 0;
    int remaining = target;
    for (int c : coins) {
        while (remaining >= c) {
            remaining -= c;
            count++;
        }
    }
    return (remaining == 0) ? count : -1;
}

// --- C.9. Best Time to Buy and Sell Stock ---
// Один проход: min_so_far и max_profit.
// O(n) время, O(1) память.
// Параметр: prices[i] — цена в день i.
// Возвращает максимальную прибыль (или 0, если нет прибыльной сделки).
int best_time_buy_sell_stock(const vector<int>& prices) {
    if (prices.empty()) return 0;
    int min_so_far = prices[0];
    int max_profit = 0;
    for (int i = 1; i < (int)prices.size(); i++) {
        max_profit = max(max_profit, prices[i] - min_so_far);
        min_so_far = min(min_so_far, prices[i]);
    }
    return max_profit;
}

// Обобщённая версия: функция сравнения цен (по умолчанию: a < b).
int best_time_buy_sell_stock_cmp(const vector<int>& prices,
                                 const function<bool(int, int)>& cmp = [](int a, int b) {
                                     return a < b;
                                 }) {
    if (prices.empty()) return 0;
    int min_so_far = prices[0];
    int max_profit = 0;
    for (int i = 1; i < (int)prices.size(); i++) {
        // profit = prices[i] - min_so_far; максимизируем
        if (!cmp(prices[i] - min_so_far, max_profit) && prices[i] - min_so_far != max_profit)
            max_profit = prices[i] - min_so_far;
        if (cmp(prices[i], min_so_far)) min_so_far = prices[i];
    }
    return max_profit;
}

// --- C.10. Best Time to Buy and Sell Stock II (множественные сделки) ---
// Неограниченное число сделок: сумма всех положительных ростов.
// O(n) время, O(1) память.
// Возвращает максимальную прибыль.
int best_time_buy_sell_stock_ii(const vector<int>& prices) {
    int profit = 0;
    for (int i = 1; i < (int)prices.size(); i++) {
        if (prices[i] > prices[i - 1])
            profit += prices[i] - prices[i - 1];
    }
    return profit;
}

// --- C.11. Gas Station ---
// circle[i] — газ, cost[i] — стоимость проезда.
// Если суммарный газ ≥ суммарных затрат — решение существует.
// Начальная станция = индекс, с которого накопленный баланс не падает ниже 0.
// O(n) время, O(1) память.
// Возвращает индекс начальной станции или -1.
int gas_station(const vector<int>& circle, const vector<int>& cost) {
    int total_tank = 0;
    int current_tank = 0;
    int start = 0;
    for (int i = 0; i < (int)circle.size(); i++) {
        int diff = circle[i] - cost[i];
        total_tank += diff;
        current_tank += diff;
        if (current_tank < 0) {
            start = i + 1;
            current_tank = 0;
        }
    }
    return (total_tank >= 0) ? start : -1;
}

// Параметризованная версия: функция вычисления разницы газ/стоимость.
int gas_station_fn(const vector<int>& circle, const vector<int>& cost,
                   const function<int(int, int)>& diff_fn) {
    int total_tank = 0;
    int current_tank = 0;
    int start = 0;
    for (int i = 0; i < (int)circle.size(); i++) {
        int diff = diff_fn(circle[i], cost[i]);
        total_tank += diff;
        current_tank += diff;
        if (current_tank < 0) {
            start = i + 1;
            current_tank = 0;
        }
    }
    return (total_tank >= 0) ? start : -1;
}

// =============================================================
// D. РЮКЗАК И ПОКРЫТИЕ
// =============================================================

// --- D.10. Fractional Knapsack ---
// Сортировка по убыванию плотности (v[i]/w[i]) →贪чное заполнение.
// O(n log n) время, O(1) доп. памяти.
// Параметры: w — веса, v — стоимости, W — вместимость рюкзака.
// Параметр value_per_weight — функция плотности (по умолчанию v/w).
// Возвращает максимальную стоимость.
double fractional_knapsack(const vector<int>& w, const vector<int>& v, int W,
                           const function<double(int, int)>& value_per_weight =
                               [](int val, int wt) -> double {
                                   return (double)val / wt;
                               }) {
    int n = (int)w.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    // сортировка по убыванию плотности
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return value_per_weight(v[a], w[a]) > value_per_weight(v[b], w[b]);
    });
    double total_value = 0.0;
    int remaining = W;
    for (int i : idx) {
        if (remaining <= 0) break;
        if (w[i] <= remaining) {
            total_value += v[i];
            remaining -= w[i];
        } else {
            total_value += (double)v[i] * remaining / w[i];
            remaining = 0;
        }
    }
    return total_value;
}

// Параметризованная версия: произвольный компаратор для плотности.
double fractional_knapsack_cmp(const vector<int>& w, const vector<int>& v, int W,
                               const function<bool(int, int)>& cmp_density) {
    int n = (int)w.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return cmp_density(a, b);
    });
    double total_value = 0.0;
    int remaining = W;
    for (int i : idx) {
        if (remaining <= 0) break;
        if (w[i] <= remaining) {
            total_value += v[i];
            remaining -= w[i];
        } else {
            total_value += (double)v[i] * remaining / w[i];
            remaining = 0;
        }
    }
    return total_value;
}

// --- D.11. Greedy Set Cover ---
// Множество X (как universum [0, n)) и семейство подмножеств sets.
// Жадный: на каждом шаге — множество, покрывающее больше всех ещё не покрытых.
// Аппроксимация: O(ln n).
// O(|X| · |sets|) время, O(|X|) доп. памяти.
// Параметр: n — размер универсума, sets[i] — i-е подмножество (список элементов).
// Возвращает индексы выбранных подмножеств.
vector<int> greedy_set_cover(int n, const vector<vector<int>>& sets) {
    vector<char> covered(n, 0);
    int covered_count = 0;
    vector<int> selected;
    vector<char> used(sets.size(), 0);

    while (covered_count < n) {
        int best_set = -1;
        int best_new = 0;
        for (int i = 0; i < (int)sets.size(); i++) {
            if (used[i]) continue;
            int new_covered = 0;
            for (int x : sets[i])
                if (!covered[x]) new_covered++;
            if (new_covered > best_new) {
                best_new = new_covered;
                best_set = i;
            }
        }
        if (best_set == -1) break; // невозможно покрыть
        used[best_set] = 1;
        selected.push_back(best_set);
        for (int x : sets[best_set])
            if (!covered[x]) { covered[x] = 1; covered_count++; }
    }
    return selected;
}

// Параметризованная версия: функция оценки качества подмножества (по умолчанию — число новых элементов).
vector<int> greedy_set_cover_fn(int n, const vector<vector<int>>& sets,
                                const function<int(const vector<int>&, const vector<char>&)>& score_fn =
                                    [](const vector<int>& s, const vector<char>& cov) -> int {
                                        int cnt = 0;
                                        for (int x : s) if (!cov[x]) cnt++;
                                        return cnt;
                                    }) {
    vector<char> covered(n, 0);
    int covered_count = 0;
    vector<int> selected;
    vector<char> used(sets.size(), 0);

    while (covered_count < n) {
        int best_set = -1;
        int best_score = 0;
        for (int i = 0; i < (int)sets.size(); i++) {
            if (used[i]) continue;
            int sc = score_fn(sets[i], covered);
            if (sc > best_score) {
                best_score = sc;
                best_set = i;
            }
        }
        if (best_set == -1) break;
        used[best_set] = 1;
        selected.push_back(best_set);
        for (int x : sets[best_set])
            if (!covered[x]) { covered[x] = 1; covered_count++; }
    }
    return selected;
}

// --- D.17. Fractional Knapsack with Shares ---
// То же, что fractional_knapsack, но возвращает вектор долей каждого предмета.
// O(n log n) время.
// Возвращает pair<double, vector<pair<int, double>>> — (общая стоимость, {(индекс, доля)}).
pair<double, vector<pair<int, double>>> fractional_knapsack_with_shares(
    const vector<int>& w, const vector<int>& v, int W) {
    int n = (int)w.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) {
        return (double)v[a] / w[a] > (double)v[b] / w[b];
    });
    double total_value = 0.0;
    int remaining = W;
    vector<pair<int, double>> shares;
    for (int i : idx) {
        if (remaining <= 0) break;
        if (w[i] <= remaining) {
            total_value += v[i];
            shares.push_back({i, 1.0});
            remaining -= w[i];
        } else {
            double frac = (double)remaining / w[i];
            total_value += v[i] * frac;
            shares.push_back({i, frac});
            remaining = 0;
        }
    }
    return {total_value, shares};
}

// --- D.18. Fractional Cover Problem ---
// Дробное покрытие: LP-релаксация Set Cover.
// Жадная эвристика: на каждом шаге — подмножество с лучшим соотношением (новые элементы / вес).
// O(|X| · |sets| · log |X|) время.
// Возвращает pair<double, vector<pair<int, double>>> — (общий вес, {(индекс, доля)}).
pair<double, vector<pair<int, double>>> fractional_cover(
    int n, const vector<vector<int>>& sets, const vector<double>& weights) {
    vector<double> covered(n, 0.0);
    double total_weight = 0.0;
    vector<pair<int, double>> result;
    vector<char> used(sets.size(), 0);

    while (true) {
        int best_set = -1;
        double best_ratio = 0.0;
        for (int i = 0; i < (int)sets.size(); i++) {
            if (used[i]) continue;
            double new_covered = 0.0;
            for (int x : sets[i])
                if (covered[x] < 1.0)
                    new_covered += 1.0 - covered[x];
            if (new_covered > 0) {
                double ratio = new_covered / weights[i];
                if (ratio > best_ratio) {
                    best_ratio = ratio;
                    best_set = i;
                }
            }
        }
        if (best_set == -1) break;

        // вычисляем долю
        double need = 0.0;
        for (int x : sets[best_set])
            if (covered[x] < 1.0) need += 1.0 - covered[x];
        double frac = min(1.0, need / need); // доля = 1, т.к. берём всё подмножество
        // но если need < total capacity — можно взять дробь; здесь贪心 берём 1.0

        used[best_set] = 1;
        result.push_back({best_set, 1.0});
        total_weight += weights[best_set];
        for (int x : sets[best_set])
            covered[x] = min(1.0, covered[x] + 1.0);

        bool all_covered = true;
        for (int x = 0; x < n; x++)
            if (covered[x] < 1.0) { all_covered = false; break; }
        if (all_covered) break;
    }
    return {total_weight, result};
}

// --- D.19. Kadane's Algorithm (максимальная сумма подотрезка) ---
// O(N) время, O(1) память.
// Возвращает максимальную сумму подотрезка.
long long kadane_max_sum(const vector<int>& a) {
    if (a.empty()) return 0;
    long long cur = a[0], best = a[0];
    for (int i = 1; i < (int)a.size(); i++) {
        cur = max((long long)a[i], cur + a[i]);
        best = max(best, cur);
    }
    return best;
}

// --- Kadane's Algorithm (минимальная сумма подотрезка) ---
// O(N) время, O(1) память.
// Возвращает минимальную сумму подотрезка.
long long kadane_min_sum(const vector<int>& a) {
    if (a.empty()) return 0;
    long long cur = a[0], best = a[0];
    for (int i = 1; i < (int)a.size(); i++) {
        cur = min((long long)a[i], cur + a[i]);
        best = min(best, cur);
    }
    return best;
}

// --- D.16. Recursive Knapsack (рекурсивный 0/1 Knapsack) ---
// Рекурсия с мемоизацией через vector<vector<long long>>.
// O(nW) время и память.
long long recursive_knapsack(const vector<int>& w, const vector<int>& v, int W) {
    int n = (int)w.size();
    vector<vector<long long>> memo(n + 1, vector<long long>(W + 1, -1));

    function<long long(int, int)> solve = [&](int i, int rem) -> long long {
        if (i == n || rem == 0) return 0;
        if (memo[i][rem] != -1) return memo[i][rem];
        long long skip = solve(i + 1, rem);
        long long take = 0;
        if (w[i] <= rem)
            take = v[i] + solve(i + 1, rem - w[i]);
        return memo[i][rem] = max(skip, take);
    };

    return solve(0, W);
}

}; // struct GreedyAlgorithms

#endif // TECHNIQUE_E_CPP
