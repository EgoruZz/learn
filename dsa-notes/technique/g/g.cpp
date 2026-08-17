#ifndef TECHNIQUE_G_CPP
#define TECHNIQUE_G_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <climits>
#include <numeric>
#include <unordered_map>
using namespace std;

// =============================================================
// VII. ПЛАНИРОВАНИЕ
// =============================================================
// Структура md: A. Задачи расписаний (Interval Scheduling, Weighted, Job Scheduling)
//               → B. Распределение ресурсов (Fractional Knapsack, Bin Packing, Load Balancing)
//               → C. Диспетчеризация (FCFS, SJF, Round Robin, Priority, MLFQ)
//
// Scheduling наследует GameTheory (f.cpp). Переиспользует:
//   * сортировки из I (a.cpp) — для предобработки интервалов и задач;
//   * бинарный поиск из II (b.cpp) — для поиска p(i) в Weighted Interval Scheduling;
//   * priority_queue (struct.md IV) — для Round Robin, SJF, Load Balancing;
//   * очереди (struct.md II) — для FCFS и Multilevel Feedback Queue.
//
// Собственной арифметики не имеет; интегрирует жадные (V), ДП (dynamic.md),
// теорию игр (VI) и структуры данных (struct.md).

#include "../f/f.cpp"

struct Scheduling : GameTheory {

// Forward declarations (used in A, defined in B)
struct LoadBalanceResult;

// =============================================================
// A. ЗАДАЧИ РАСПИСАНИЙ
// =============================================================

// --- A.1. Interval Scheduling (жадный по endTime) ---
// Возвращает индексы выбранных интервалов (непересекающихся, макс. число).
// Сортировка по endTime →贪心 выбор最早 завершающихся.
// Сложность: O(n log n) время, O(n) память.
struct Interval {
    int start, end, index;
};

vector<int> interval_scheduling(const vector<Interval>& intervals) {
    int n = (int)intervals.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return intervals[a].end < intervals[b].end;
    });

    vector<int> selected;
    int last_end = INT_MIN;
    for (int i : order) {
        if (intervals[i].start >= last_end) {
            selected.push_back(intervals[i].index);
            last_end = intervals[i].end;
        }
    }
    return selected;
}

// Параметризованная версия: произвольный компаратор для сортировки
// и предикат проверки совместимости интервалов.
vector<int> interval_scheduling_generic(
    const vector<Interval>& intervals,
    const function<bool(const Interval&, const Interval&)>& cmp = [](const Interval& a, const Interval& b) {
        return a.end < b.end;
    },
    const function<bool(const Interval&, const Interval&)>& compatible = [](const Interval& a, const Interval& b) {
        return a.end <= b.start;
    }) {
    int n = (int)intervals.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return cmp(intervals[a], intervals[b]);
    });

    vector<int> selected;
    for (int i : order) {
        bool ok = true;
        for (int j : selected) {
            if (!compatible(intervals[j], intervals[i])) { ok = false; break; }
        }
        if (ok) selected.push_back(intervals[i].index);
    }
    return selected;
}

// --- A.2. Weighted Interval Scheduling (ДП) ---
// Каждый интервал имеет вес; найти подмножество с макс. суммой весов.
// DP[i] = max(DP[i-1], w_i + DP[p(i)]), p(i) — через бинарный поиск.
// Сложность: O(n log n) время, O(n) память.
struct WeightedInterval {
    int start, end, weight, index;
};

vector<int> weighted_interval_scheduling(const vector<WeightedInterval>& intervals) {
    int n = (int)intervals.size();
    if (n == 0) return {};

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return intervals[a].end < intervals[b].end;
    });

    // сортированные по endTime
    vector<WeightedInterval> sorted(n);
    for (int i = 0; i < n; i++) sorted[i] = intervals[order[i]];

    // p(i) = последний индекс j < i, где sorted[j].end <= sorted[i].start
    vector<int> p(n, -1);
    for (int i = 0; i < n; i++) {
        // бинарный поиск: найти правыйmost j с sorted[j].end <= sorted[i].start
        int lo = 0, hi = i - 1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (sorted[mid].end <= sorted[i].start) lo = mid + 1;
            else hi = mid - 1;
        }
        p[i] = hi; // -1 если нет такого
    }

    // DP
    vector<long long> dp(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        long long take = sorted[i - 1].weight + (p[i - 1] >= 0 ? dp[p[i - 1] + 1] : 0);
        dp[i] = max(dp[i - 1], take);
    }

    // восстановление решения
    vector<int> selected;
    int i = n;
    while (i > 0) {
        long long take = sorted[i - 1].weight + (p[i - 1] >= 0 ? dp[p[i - 1] + 1] : 0);
        if (take >= dp[i - 1]) {
            selected.push_back(sorted[i - 1].index);
            i = p[i - 1] + 1;
        } else {
            i--;
        }
    }
    reverse(selected.begin(), selected.end());
    return selected;
}

// Параметризованная версия: произвольная функция веса и предикат совместимости
vector<int> weighted_interval_scheduling_generic(
    const vector<WeightedInterval>& intervals,
    const function<long long(const WeightedInterval&)>& weight_fn = [](const WeightedInterval& x) { return x.weight; },
    const function<bool(const WeightedInterval&, const WeightedInterval&)>& compatible = [](const WeightedInterval& a, const WeightedInterval& b) {
        return a.end <= b.start;
    }) {
    int n = (int)intervals.size();
    if (n == 0) return {};

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return intervals[a].end < intervals[b].end;
    });

    vector<WeightedInterval> sorted(n);
    for (int i = 0; i < n; i++) sorted[i] = intervals[order[i]];

    vector<int> p(n, -1);
    for (int i = 0; i < n; i++) {
        int lo = 0, hi = i - 1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (compatible(sorted[mid], sorted[i])) lo = mid + 1;
            else hi = mid - 1;
        }
        p[i] = hi;
    }

    vector<long long> dp(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        long long take = weight_fn(sorted[i - 1]) + (p[i - 1] >= 0 ? dp[p[i - 1] + 1] : 0);
        dp[i] = max(dp[i - 1], take);
    }

    vector<int> selected;
    int i = n;
    while (i > 0) {
        long long take = weight_fn(sorted[i - 1]) + (p[i - 1] >= 0 ? dp[p[i - 1] + 1] : 0);
        if (take >= dp[i - 1]) {
            selected.push_back(sorted[i - 1].index);
            i = p[i - 1] + 1;
        } else {
            i--;
        }
    }
    reverse(selected.begin(), selected.end());
    return selected;
}

// --- A.3. Job Scheduling with Deadlines (minimize penalties) ---
// n задач: длительность = 1, deadline d_i, штраф p_i.
// Жадный: сортировка по убыванию штрафа → размещение на последний свободный слот ≤ deadline.
// Возвращает: (порядок выполнения, суммарный штраф, множество выполненных задач).
// Сложность: O(n log n) время, O(n) память.
struct Job {
    int id, deadline, penalty;
};

struct JobScheduleResult {
    vector<int> order;        // порядок выполнения
    long long totalPenalty;   // суммарный штраф невыполненных
    vector<int> completed;    // индексы выполненных задач
};

JobScheduleResult job_scheduling_deadlines(const vector<Job>& jobs) {
    int n = (int)jobs.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return jobs[a].penalty > jobs[b].penalty;
    });

    // находим максимальный deadline
    int maxD = 0;
    for (auto& j : jobs) maxD = max(maxD, j.deadline);

    // слоты: slots[t] = индекс задачи в слоте t (0-based)
    vector<int> slots(maxD, -1);

    for (int i : order) {
        // ищем последний свободный слот ≤ deadline-1
        for (int t = jobs[i].deadline - 1; t >= 0; t--) {
            if (slots[t] == -1) {
                slots[t] = i;
                break;
            }
        }
    }

    // собираем результат
    JobScheduleResult result;
    result.totalPenalty = 0;
    unordered_map<int, int> slot_to_job;
    for (int t = 0; t < maxD; t++) {
        if (slots[t] != -1) {
            slot_to_job[t] = slots[t];
            result.completed.push_back(slots[t]);
        }
    }

    for (int i = 0; i < n; i++) {
        if (slot_to_job.count(i)) {
            result.order.push_back(slot_to_job[i]);
        } else {
            result.totalPenalty += jobs[i].penalty;
        }
    }

    // невыполненные задачи — в конце
    vector<bool> done(n, false);
    for (int x : result.completed) done[x] = true;
    for (int i = 0; i < n; i++) {
        if (!done[i]) result.order.push_back(i);
    }

    return result;
}

// Параметризованная версия: произвольная функция штрафа и длительность задач
JobScheduleResult job_scheduling_deadlines_generic(
    const vector<Job>& jobs,
    const function<long long(const Job&)>& penalty_fn = [](const Job& j) { return j.penalty; },
    const function<int(const Job&)>& duration_fn = [](const Job& j) { return 1; }) {
    int n = (int)jobs.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return penalty_fn(jobs[a]) > penalty_fn(jobs[b]);
    });

    int maxSlot = 0;
    for (auto& j : jobs) maxSlot = max(maxSlot, j.deadline);
    vector<int> slots(maxSlot, -1);

    for (int i : order) {
        int dur = duration_fn(jobs[i]);
        for (int t = jobs[i].deadline - dur; t >= 0; t--) {
            bool ok = true;
            for (int k = 0; k < dur && t + k < maxSlot; k++) {
                if (slots[t + k] != -1) { ok = false; break; }
            }
            if (ok) {
                for (int k = 0; k < dur && t + k < maxSlot; k++) slots[t + k] = i;
                break;
            }
        }
    }

    JobScheduleResult result;
    result.totalPenalty = 0;
    unordered_map<int, int> slot_to_job;
    for (int t = 0; t < maxSlot; t++) {
        if (slots[t] != -1) slot_to_job[t] = slots[t];
    }

    vector<bool> done(n, false);
    for (auto& [t, idx] : slot_to_job) {
        if (!done[idx]) {
            result.completed.push_back(idx);
            done[idx] = true;
        }
    }

    for (int i = 0; i < n; i++) {
        if (done[i]) result.order.push_back(i);
        else result.totalPenalty += penalty_fn(jobs[i]);
    }

    vector<bool> in_order(n, false);
    for (int x : result.order) in_order[x] = true;
    for (int i = 0; i < n; i++) {
        if (!in_order[i]) result.order.push_back(i);
    }

    return result;
}

// --- A.4. Task Scheduling on Multiple Machines ---
// n задач с временем t_i; m одинаковых параллельных машин.
// LPT (Longest Processing Time): сортировка по убыванию → каждая задача на наименее загруженную.
// Возвращает: (makespan, распределение задач по машинам).
// Сложность: O(n log n + n log m) время, O(n + m) память.
LoadBalanceResult task_scheduling_multiple_machines(const vector<long long>& taskTimes, int numMachines) {
    int n = (int)taskTimes.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return taskTimes[a] > taskTimes[b];
    });

    using P = pair<long long, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    for (int i = 0; i < numMachines; i++) pq.push({0, i});

    LoadBalanceResult result;
    result.makespan = 0;
    result.assignment.resize(numMachines);

    for (int i : order) {
        auto [load, mach] = pq.top(); pq.pop();
        result.assignment[mach].push_back(i);
        load += taskTimes[i];
        result.makespan = max(result.makespan, load);
        pq.push({load, mach});
    }

    return result;
}

// =============================================================
// B. РАСПРЕДЕЛЕНИЕ РЕСУРСОВ
// =============================================================

// --- B.4. Fractional Knapsack ---
// Предметы с весами и стоимостями; можно брать доли.
// Жадный по плотности (value/weight).
// Возвращает: (максимальная стоимость, состав — пары (индекс, доля)).
// Сложность: O(n log n) время, O(n) память.
struct Item {
    int index;
    double weight, value;
};

struct KnapsackResult {
    double totalValue;
    vector<pair<int, double>> taken; // (index, fraction)
};

KnapsackResult fractional_knapsack(const vector<Item>& items, double capacity) {
    int n = (int)items.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        double da = items[a].value / items[a].weight;
        double db = items[b].value / items[b].weight;
        return da > db;
    });

    KnapsackResult result;
    result.totalValue = 0;
    double remaining = capacity;

    for (int i : order) {
        if (remaining <= 0) break;
        double take = min(items[i].weight, remaining);
        double fraction = take / items[i].weight;
        result.taken.push_back({items[i].index, fraction});
        result.totalValue += items[i].value * fraction;
        remaining -= take;
    }

    return result;
}

// Параметризованная версия: произвольная функция плотности
KnapsackResult fractional_knapsack_generic(
    const vector<Item>& items, double capacity,
    const function<double(const Item&)>& density_fn = [](const Item& x) {
        return x.value / x.weight;
    }) {
    int n = (int)items.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return density_fn(items[a]) > density_fn(items[b]);
    });

    KnapsackResult result;
    result.totalValue = 0;
    double remaining = capacity;

    for (int i : order) {
        if (remaining <= 0) break;
        double take = min(items[i].weight, remaining);
        double fraction = take / items[i].weight;
        result.taken.push_back({items[i].index, fraction});
        result.totalValue += items[i].value * fraction;
        remaining -= take;
    }

    return result;
}

// --- B.5. Bin Packing ---
// Предметы размеров s_i; бины ёмкости C; упаковать в мин. число бинов.
// Возвращает: (число бинов, распределение предметов по бинам).

// First Fit: каждый предмет в первый подходящий бин.
// Сложность: O(n²) наивно или O(n log n) через set.
struct BinPackResult {
    int numBins;
    vector<vector<int>> bins; // bins[i] = список индексов предметов в бине i
};

BinPackResult bin_packing_first_fit(const vector<double>& sizes, double capacity) {
    BinPackResult result;
    result.numBins = 0;

    for (int i = 0; i < (int)sizes.size(); i++) {
        bool placed = false;
        for (int b = 0; b < result.numBins; b++) {
            double used = 0;
            for (int idx : result.bins[b]) used += sizes[idx];
            if (used + sizes[i] <= capacity) {
                result.bins[b].push_back(i);
                placed = true;
                break;
            }
        }
        if (!placed) {
            result.bins.push_back({i});
            result.numBins++;
        }
    }

    return result;
}

// Best Fit: каждый предмет в бин с мин. оставшимся пространством.
// Сложность: O(n²) наивно.
BinPackResult bin_packing_best_fit(const vector<double>& sizes, double capacity) {
    BinPackResult result;
    result.numBins = 0;

    for (int i = 0; i < (int)sizes.size(); i++) {
        int bestBin = -1;
        double bestRemaining = capacity + 1;

        for (int b = 0; b < result.numBins; b++) {
            double used = 0;
            for (int idx : result.bins[b]) used += sizes[idx];
            double remaining = capacity - used;
            if (remaining >= sizes[i] && remaining < bestRemaining) {
                bestRemaining = remaining;
                bestBin = b;
            }
        }

        if (bestBin != -1) {
            result.bins[bestBin].push_back(i);
        } else {
            result.bins.push_back({i});
            result.numBins++;
        }
    }

    return result;
}

// First Fit Decreasing: сортировка по убыванию + First Fit.
// Сложность: O(n log n) время.
BinPackResult bin_packing_first_fit_decreasing(const vector<double>& sizes, double capacity) {
    int n = (int)sizes.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return sizes[a] > sizes[b];
    });

    vector<double> sortedSizes(n);
    for (int i = 0; i < n; i++) sortedSizes[i] = sizes[order[i]];

    BinPackResult tmp = bin_packing_first_fit(sortedSizes, capacity);

    // маппинг обратно на исходные индексы
    BinPackResult result;
    result.numBins = tmp.numBins;
    for (auto& bin : tmp.bins) {
        vector<int> mapped;
        for (int idx : bin) mapped.push_back(order[idx]);
        result.bins.push_back(mapped);
    }

    return result;
}

// --- B.6. Load Balancing (minimize makespan) ---
// n задач с временем t_i; m машин; минимизировать время завершения.
// Жадный (LPT): сортировка по убыванию → каждая задача на наименее загруженную машину.
// Возвращает: (makespan, распределение задач по машинам).
// Сложность: O(n log n + n log m) время.
struct LoadBalanceResult {
    long long makespan;
    vector<vector<int>> assignment; // assignment[m] = индексы задач
};

LoadBalanceResult load_balancing_makespan(const vector<long long>& taskTimes, int numMachines) {
    int n = (int)taskTimes.size();

    // сортировка по убыванию времени
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return taskTimes[a] > taskTimes[b];
    });

    // min-heap: (currentLoad, machineIndex)
    using P = pair<long long, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    for (int i = 0; i < numMachines; i++) pq.push({0, i});

    LoadBalanceResult result;
    result.makespan = 0;
    result.assignment.resize(numMachines);

    for (int i : order) {
        auto [load, mach] = pq.top(); pq.pop();
        result.assignment[mach].push_back(i);
        load += taskTimes[i];
        result.makespan = max(result.makespan, load);
        pq.push({load, mach});
    }

    return result;
}

// Параметризованная версия: произвольная функция времени выполнения
LoadBalanceResult load_balancing_makespan_generic(
    const vector<long long>& taskTimes, int numMachines,
    const function<long long(long long)>& time_fn = [](long long t) { return t; }) {
    int n = (int)taskTimes.size();

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return time_fn(taskTimes[a]) > time_fn(taskTimes[b]);
    });

    using P = pair<long long, int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    for (int i = 0; i < numMachines; i++) pq.push({0, i});

    LoadBalanceResult result;
    result.makespan = 0;
    result.assignment.resize(numMachines);

    for (int i : order) {
        auto [load, mach] = pq.top(); pq.pop();
        result.assignment[mach].push_back(i);
        load += time_fn(taskTimes[i]);
        result.makespan = max(result.makespan, load);
        pq.push({load, mach});
    }

    return result;
}

// =============================================================
// C. ДИСПЕТЧЕРИЗАЦИЯ (ПЛАНИРОВЩИКИ ОС)
// =============================================================

// --- Общая структура задачи для планировщика ---
struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int priority; // используется в Priority Scheduling
};

struct ScheduleEvent {
    int processId;
    int startTime;
    int endTime;
    string type; // "run", "preempt", "complete"
};

// --- Результат симуляции ---
struct ScheduleResult {
    vector<ScheduleEvent> timeline; // шкала выполнения
    vector<double> waitingTime;     // время ожидания для каждого процесса
    vector<double> turnaroundTime;  // turnaround time для каждого процесса
    double avgWaitingTime;
    double avgTurnaroundTime;
};

// --- Вспомогательные функции ---
void compute_metrics(ScheduleResult& result, const vector<Process>& processes, int n) {
    result.waitingTime.assign(n, 0);
    result.turnaroundTime.assign(n, 0);

    for (auto& ev : result.timeline) {
        if (ev.type == "complete") {
            int pid = ev.processId;
            int completion = ev.endTime;
            int arrival = processes[pid].arrivalTime;
            int burst = processes[pid].burstTime;
            result.turnaroundTime[pid] = completion - arrival;
            result.waitingTime[pid] = result.turnaroundTime[pid] - burst;
        }
    }

    double sumW = 0, sumT = 0;
    for (int i = 0; i < n; i++) {
        sumW += result.waitingTime[i];
        sumT += result.turnaroundTime[i];
    }
    result.avgWaitingTime = sumW / n;
    result.avgTurnaroundTime = sumT / n;
}

// --- C.7. FCFS (First Come First Served) ---
// Задачи выполняются в порядке поступления.
// Сложность: O(n log n) время (сортировка по arrival), O(n) память.
ScheduleResult process_scheduling_sim_fcfs(const vector<Process>& processes) {
    int n = (int)processes.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    ScheduleResult result;
    int currentTime = 0;

    for (int i : order) {
        currentTime = max(currentTime, processes[i].arrivalTime);
        result.timeline.push_back({i, currentTime, currentTime + processes[i].burstTime, "complete"});
        currentTime += processes[i].burstTime;
    }

    compute_metrics(result, processes, n);
    return result;
}

// --- C.8. SJF (Shortest Job First, non-preemptive) ---
// Задача с наименьшим burstTime выполняется первой из доступных.
// Сложность: O(n²) время (наивно), O(n log n) через priority queue.
ScheduleResult process_scheduling_sim_sjf(const vector<Process>& processes) {
    int n = (int)processes.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    // min-heap: (burstTime, index)
    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    ScheduleResult result;
    int currentTime = 0;
    int idx = 0;

    while (idx < n || !pq.empty()) {
        while (idx < n && processes[order[idx]].arrivalTime <= currentTime) {
            pq.push({processes[order[idx]].burstTime, order[idx]});
            idx++;
        }

        if (pq.empty()) {
            currentTime = processes[order[idx]].arrivalTime;
            continue;
        }

        auto [burst, pid] = pq.top(); pq.pop();
        result.timeline.push_back({pid, currentTime, currentTime + burst, "complete"});
        currentTime += burst;
    }

    compute_metrics(result, processes, n);
    return result;
}

// --- C.9. Round Robin ---
// Каждая задача получает квант quantum; по истечении — в конец очереди.
// Сложность: O(n) время (с использованием очереди), O(n) память.
ScheduleResult process_scheduling_sim_rr(const vector<Process>& processes, int quantum) {
    int n = (int)processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = processes[i].burstTime;

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    queue<int> q;
    ScheduleResult result;
    int currentTime = 0;
    int idx = 0;

    // добавляем первые процессы
    while (idx < n && processes[order[idx]].arrivalTime <= currentTime) {
        q.push(order[idx]);
        idx++;
    }

    while (!q.empty()) {
        int pid = q.front(); q.pop();

        int runTime = min(quantum, remaining[pid]);
        int startTime = currentTime;
        currentTime += runTime;
        remaining[pid] -= runTime;

        // добавляем новые процессы, прибывшие до currentTime
        while (idx < n && processes[order[idx]].arrivalTime <= currentTime) {
            q.push(order[idx]);
            idx++;
        }

        if (remaining[pid] == 0) {
            result.timeline.push_back({pid, startTime, currentTime, "complete"});
        } else {
            result.timeline.push_back({pid, startTime, currentTime, "preempt"});
            q.push(pid);
        }
    }

    compute_metrics(result, processes, n);
    return result;
}

// --- C.10. Priority Scheduling (preemptive) ---
// Задача с наивысшим приоритетом выполняется первой.
// Preemptive: при поступлении задачи с более высоким приоритетом — прерывание.
// Сложность: O(n log n) время.
ScheduleResult process_scheduling_sim_priority(const vector<Process>& processes) {
    int n = (int)processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = processes[i].burstTime;

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    // min-heap: (-priority, index) для наивысшего приоритета наверху
    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    ScheduleResult result;
    int currentTime = 0;
    int idx = 0;
    int currentPid = -1;

    while (idx < n || !pq.empty()) {
        // добавляем прибывшие процессы
        while (idx < n && processes[order[idx]].arrivalTime <= currentTime) {
            pq.push({-processes[order[idx]].priority, order[idx]});
            idx++;
        }

        if (pq.empty()) {
            currentTime = processes[order[idx]].arrivalTime;
            continue;
        }

        auto [negPri, pid] = pq.top();

        // если текущий процесс был preempted — записываем событие
        if (currentPid != -1 && currentPid != pid) {
            // текущий процесс прерван — его burstTime уже уменьшен
        }

        currentPid = pid;
        pq.pop();

        // определяем, сколько выполним до следнего arrival
        int nextArrival = (idx < n) ? processes[order[idx]].arrivalTime : INT_MAX;
        int runTime = min(remaining[pid], nextArrival - currentTime);

        int startTime = currentTime;
        currentTime += runTime;
        remaining[pid] -= runTime;

        if (remaining[pid] == 0) {
            result.timeline.push_back({pid, startTime, currentTime, "complete"});
        } else {
            result.timeline.push_back({pid, startTime, currentTime, "preempt"});
            pq.push({-processes[pid].priority, pid});
        }
    }

    compute_metrics(result, processes, n);
    return result;
}

// --- C.11. Multilevel Feedback Queue ---
// Несколько очередей: уровня i — квант quantum_i.
// Задача, не завершившаяся за quantum_i, перемещается в уровень i+1.
// Новые задачи поступают в уровень 0.
// Сложность: O(n · k) время (k — число уровней).
ScheduleResult multilevel_feedback_queue(
    const vector<Process>& processes,
    const vector<int>& quantums, // quantums[i] — квант уровня i
    int numLevels) {
    int n = (int)processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = processes[i].burstTime;
    vector<int> level(n, 0); // текущий уровень каждого процесса

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    // очереди по уровням
    vector<queue<int>> queues(numLevels);
    ScheduleResult result;
    int currentTime = 0;
    int idx = 0;

    auto add_arrivals = [&]() {
        while (idx < n && processes[order[idx]].arrivalTime <= currentTime) {
            queues[0].push(order[idx]);
            idx++;
        }
    };

    add_arrivals();

    int currentLevel = 0;
    while (true) {
        // ищем непустую очередь, начиная с текущего уровня
        while (currentLevel < numLevels && queues[currentLevel].empty()) currentLevel++;
        if (currentLevel >= numLevels) {
            add_arrivals();
            if (currentLevel >= numLevels && idx >= n) break;
            currentLevel = 0;
            continue;
        }

        int pid = queues[currentLevel].front(); queues[currentLevel].pop();

        int q = (currentLevel < (int)quantums.size()) ? quantums[currentLevel] : quantums.back();
        int runTime = min(remaining[pid], q);

        int startTime = currentTime;
        currentTime += runTime;
        remaining[pid] -= runTime;

        add_arrivals();

        if (remaining[pid] == 0) {
            result.timeline.push_back({pid, startTime, currentTime, "complete"});
        } else {
            result.timeline.push_back({pid, startTime, currentTime, "preempt"});
            int nextLevel = min(currentLevel + 1, numLevels - 1);
            queues[nextLevel].push(pid);
            level[pid] = nextLevel;
        }
    }

    compute_metrics(result, processes, n);
    return result;
}

// =============================================================
// D. ДОПОЛНИТЕЛЬНО
// =============================================================

// --- D.19. Highest Response Ratio Next (HRRN) ---
// Некоммутативный планировщик: на каждом шаге выбирается задача с макс. Response Ratio.
// R = (waitingTime + serviceTime) / serviceTime.
// Сложность: O(n²) время, O(n) память.
ScheduleResult hrrn_scheduling(const vector<Process>& processes) {
    int n = (int)processes.size();
    vector<int> remaining(n);
    for (int i = 0; i < n; i++) remaining[i] = processes[i].burstTime;
    vector<bool> done(n, false);

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    ScheduleResult result;
    int currentTime = 0;
    int completed = 0;

    while (completed < n) {
        // собираем доступные задачи
        int bestIdx = -1;
        double bestRR = -1.0;
        for (int i : order) {
            if (!done[i] && processes[i].arrivalTime <= currentTime) {
                double waiting = currentTime - processes[i].arrivalTime;
                double rr = (waiting + processes[i].burstTime) / (double)processes[i].burstTime;
                if (rr > bestRR) {
                    bestRR = rr;
                    bestIdx = i;
                }
            }
        }

        if (bestIdx == -1) {
            // нет доступных задач — перемещаем время
            for (int i : order) {
                if (!done[i]) {
                    currentTime = processes[i].arrivalTime;
                    break;
                }
            }
            continue;
        }

        result.timeline.push_back({bestIdx, currentTime, currentTime + processes[bestIdx].burstTime, "complete"});
        currentTime += processes[bestIdx].burstTime;
        done[bestIdx] = true;
        completed++;
    }

    compute_metrics(result, processes, n);
    return result;
}

// --- D.20. Job Sequence With Deadline (макс. число задач) ---
// n задач, длительность = 1, deadline d_i.
// Жадный: сортировка по убыванию d_i → размещение на последний свободный слот.
// Возвращает: (порядок выполнения, число выполненных задач).
// Слотовая реализация без DSU — O(n log n) время.
struct JobSequenceResult {
    vector<int> order;       // порядок выполнения (индексы задач)
    int completedCount;      // число выполненных задач
};

JobSequenceResult job_sequence_with_deadline(const vector<Job>& jobs) {
    int n = (int)jobs.size();
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return jobs[a].deadline > jobs[b].deadline;
    });

    int maxD = 0;
    for (auto& j : jobs) maxD = max(maxD, j.deadline);

    vector<int> slots(maxD, -1);

    for (int i : order) {
        for (int t = jobs[i].deadline - 1; t >= 0; t--) {
            if (slots[t] == -1) {
                slots[t] = i;
                break;
            }
        }
    }

    JobSequenceResult result;
    result.completedCount = 0;
    vector<bool> used(n, false);
    for (int t = 0; t < maxD; t++) {
        if (slots[t] != -1 && !used[slots[t]]) {
            result.order.push_back(slots[t]);
            used[slots[t]] = true;
            result.completedCount++;
        }
    }

    // добавляем невыполненные задачи в конец (для полноты)
    for (int i = 0; i < n; i++) {
        if (!used[i]) result.order.push_back(i);
    }

    return result;
}

}; // struct Scheduling

#endif // TECHNIQUE_G_CPP
