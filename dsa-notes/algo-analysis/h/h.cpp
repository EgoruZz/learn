#ifndef ALGO_ANALYSIS_H_CPP
#define ALGO_ANALYSIS_H_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <queue>
#include <set>
#include <random>
#include <chrono>
using namespace std;

// =============================================================
// H. ПРОДВИНУТЫЕ ТЕМЫ
// =============================================================
// Структура md: A. Амортизированный анализ
//               → B. Вероятностный анализ
//               → C. Анализ онлайн-алгоритмов
//
// AdvancedTopics — наследует NPTheory (g.cpp).
// Амортизация (агрегирование, бухгалтерский, потенциал),
// вероятностные алгоритмы, онлайн-алгоритмы.

#ifndef INSIDE_ALGO_ANALYSIS_H
#define INSIDE_ALGO_ANALYSIS_H
#include "../g/g.cpp"
#endif

struct AdvancedTopics : NPTheory {

// =============================================================
// A. АМОРТИЗИРОВАННЫЙ АНАЛИЗ
// =============================================================

// --- A.1. Dynamic Array: амортизированное добавление ---
// Добавление в конец с автоматическим resize.
struct DynamicArray {
    int* data;
    int size;
    int capacity;
    int total_ops;  // подсчёт всех операций

    DynamicArray(int init_cap = 1) : size(0), capacity(init_cap), total_ops(0) {
        data = new int[capacity];
    }
    ~DynamicArray() { delete[] data; }

    void push_back(int x) {
        total_ops++;  // операция вставки
        if (size == capacity) {
            // Resize: копируем все элементы
            int new_cap = capacity * 2;
            int* new_data = new int[new_cap];
            for (int i = 0; i < size; i++) new_data[i] = data[i];
            delete[] data;
            data = new_data;
            capacity = new_cap;
            total_ops += size;  // +size за копирование
        }
        data[size++] = x;
    }

    double amortized_cost() {
        return (double)total_ops / size;
    }
};

// --- A.2. Метод агрегирования: доказательство O(1) ---
// Суммарная стоимость n push_back: O(n) (каждый элемент копируется ≤ log₂n раз).
long long aggregate_analysis(int n) {
    long long total = 0;
    int cap = 1;
    for (int i = 0; i < n; i++) {
        total++;  // push_back
        if (i == cap) {
            total += cap;  // копирование при resize
            cap *= 2;
        }
    }
    return total;  // O(n)
}

// --- A.3. Бухгалтерский метод: стек ---
// Push: «заплачено» 2, pop: «заплачено» 0 (использует кредит).
struct StackWithCredit {
    vector<int> st;
    int credit = 0;  // кредит
    int total_amortized = 0;

    void push(int x) {
        st.push_back(x);
        credit += 1;  // переплата: реальная 1, «заплачено» 2
        total_amortized += 2;
    }

    int pop() {
        int val = st.back();
        st.pop_back();
        credit -= 1;  // используем кредит: реальная 1, «заплачено» 0
        total_amortized += 0;
        return val;
    }

    double amortized_cost() {
        return (double)total_amortized / max(1, (int)st.size());
    }
};

// --- A.4. Union-Find с rank ---
struct UnionFind {
    vector<int> parent, rank_val;
    int ops = 0;

    UnionFind(int n) : parent(n), rank_val(n, 0) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int x) {
        ops++;
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);  // path compression
    }

    void unite(int x, int y) {
        ops++;
        x = find(x); y = find(y);
        if (x == y) return;
        if (rank_val[x] < rank_val[y]) swap(x, y);
        parent[y] = x;
        if (rank_val[x] == rank_val[y]) rank_val[x]++;
    }

    double amortized_cost(int total_ops_count) {
        return (double)ops / max(1, total_ops_count);
    }
};

// =============================================================
// B. ВЕРОЯТНОСТНЫЙ АНАЛИЗ
// =============================================================

// --- B.1. Randomized QuickSort: expected O(n log n) ---
long long rqs_comparisons = 0;
void randomized_quicksort(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int pivot = a[lo + rand() % (hi - lo)];
    int i = lo, j = hi - 1;
    while (i <= j) {
        while (i <= j && a[i] < pivot) { rqs_comparisons++; i++; }
        while (i <= j && a[j] > pivot) { rqs_comparisons++; j--; }
        if (i <= j) { swap(a[i], a[j]); i++; j--; }
    }
    randomized_quicksort(a, lo, j + 1);
    randomized_quicksort(a, i, hi);
}

// --- B.2. Skip List (упрощённый) ---
struct SkipListNode {
    int value;
    int level;
    vector<SkipListNode*> next;
    SkipListNode(int v, int l) : value(v), level(l), next(l + 1, nullptr) {}
};

struct SimpleSkipList {
    int max_level;
    SkipListNode* head;

    SimpleSkipList(int ml = 16) : max_level(ml) {
        head = new SkipListNode(-1, max_level);
    }

    ~SimpleSkipList() {
        SkipListNode* curr = head;
        while (curr) {
            SkipListNode* next = curr->next[0];
            delete curr;
            curr = next;
        }
    }

    int random_level() {
        int level = 0;
        while ((double)rand() / RAND_MAX < 0.5 && level < max_level - 1)
            level++;
        return level;
    }

    void insert(int value) {
        vector<SkipListNode*> update(max_level, nullptr);
        SkipListNode* curr = head;
        for (int i = max_level - 1; i >= 0; i--) {
            while (curr->next[i] && curr->next[i]->value < value)
                curr = curr->next[i];
            update[i] = curr;
        }
        int new_level = random_level();
        SkipListNode* new_node = new SkipListNode(value, new_level);
        for (int i = 0; i <= new_level; i++) {
            new_node->next[i] = update[i]->next[i];
            update[i]->next[i] = new_node;
        }
    }

    bool search(int value) {
        SkipListNode* curr = head;
        for (int i = max_level - 1; i >= 0; i--)
            while (curr->next[i] && curr->next[i]->value < value)
                curr = curr->next[i];
        curr = curr->next[0];
        return curr && curr->value == value;
    }
};

// =============================================================
// C. АНАЛИЗ ОНЛАЙН-АЛГОРИТМОВ
// =============================================================

// --- C.1. Paging: FIFO vs LRU vs OPT ---
struct PagingSimulator {
    int cache_size;
    int faults_fifo = 0, faults_lru = 0, faults_opt = 0;

    PagingSimulator(int k) : cache_size(k) {}

    void simulate(const vector<int>& requests) {
        // FIFO
        {
            queue<int> q;
            set<int> in_cache;
            for (int page : requests) {
                if (in_cache.count(page)) continue;
                faults_fifo++;
                if ((int)q.size() == cache_size) {
                    int old = q.front(); q.pop();
                    in_cache.erase(old);
                }
                q.push(page);
                in_cache.insert(page);
            }
        }
        // LRU
        {
            vector<int> lru_order;
            set<int> in_cache;
            for (int page : requests) {
                if (in_cache.count(page)) {
                    // Move to front
                    lru_order.erase(find(lru_order.begin(), lru_order.end(), page));
                    lru_order.push_back(page);
                    continue;
                }
                faults_lru++;
                if ((int)lru_order.size() == cache_size) {
                    int old = lru_order.front();
                    lru_order.erase(lru_order.begin());
                    in_cache.erase(old);
                }
                lru_order.push_back(page);
                in_cache.insert(page);
            }
        }
        // OPT (clairvoyant)
        {
            set<int> in_cache;
            for (int i = 0; i < (int)requests.size(); i++) {
                int page = requests[i];
                if (in_cache.count(page)) continue;
                faults_opt++;
                if ((int)in_cache.size() < cache_size) {
                    in_cache.insert(page);
                    continue;
                }
                // Find page used farthest in future
                int farthest = -1, farthest_pos = -1;
                for (int p : in_cache) {
                    int next_use = requests.size();
                    for (int j = i + 1; j < (int)requests.size(); j++) {
                        if (requests[j] == p) { next_use = j; break; }
                    }
                    if (next_use > farthest_pos) {
                        farthest_pos = next_use;
                        farthest = p;
                    }
                }
                in_cache.erase(farthest);
                in_cache.insert(page);
            }
        }
    }
};

}; // struct AdvancedTopics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef ALGO_ANALYSIS_H_MAIN
int main() {
    AdvancedTopics at;
    srand(42);

    cout << "=== A. АМОРТИЗИРОВАННЫЙ АНАЛИЗ ===" << endl;

    cout << "--- Dynamic Array: амортизированное добавление ---" << endl;
    {
        AdvancedTopics::DynamicArray da(1);
        for (int i = 0; i < 1000; i++) da.push_back(i);
        cout << "  1000 push_back: total_ops=" << da.total_ops
             << " amortized=" << da.amortized_cost() << " (ожидаем ~1)" << endl;
    }

    cout << "\n--- Метод агрегирования ---" << endl;
    for (int n : {100, 1000, 10000, 100000}) {
        long long total = at.aggregate_analysis(n);
        cout << "  n=" << n << ": total_ops=" << total
             << " amortized=" << (double)total / n << endl;
    }

    cout << "\n--- Бухгалтерский метод: стек ---" << endl;
    {
        AdvancedTopics::StackWithCredit st;
        for (int i = 0; i < 100; i++) st.push(i);
        for (int i = 0; i < 50; i++) st.pop();
        cout << "  100 push + 50 pop: amortized=" << st.amortized_cost()
             << " (ожидаем ~1.33)" << endl;
    }

    cout << "\n--- Union-Find: амортизированное объединение ---" << endl;
    {
        int n = 1000;
        AdvancedTopics::UnionFind uf(n);
        int ops_count = 0;
        for (int i = 0; i < n - 1; i++) {
            uf.unite(i, i + 1);
            ops_count++;
        }
        for (int i = 0; i < n; i++) uf.find(i);
        ops_count += n;
        cout << "  " << n - 1 << " unions + " << n << " finds: total=" << uf.ops
             << " amortized=" << (double)uf.ops / ops_count << " (ожидаем ~O(1))" << endl;
    }

    cout << "\n=== B. ВЕРОЯТНОСТНЫЙ АНАЛИЗ ===" << endl;

    cout << "--- Randomized QuickSort: expected O(n log n) ---" << endl;
    {
        for (int n : {1000, 10000, 100000}) {
            vector<int> a(n);
            for (int i = 0; i < n; i++) a[i] = rand();
            at.rqs_comparisons = 0;
            at.randomized_quicksort(a, 0, n);
            double expected = 2.0 * n * log(n);
            cout << "  n=" << n << ": comparisons=" << at.rqs_comparisons
                 << " (2n ln n=" << (long long)expected << ")" << endl;
        }
    }

    cout << "\n--- Skip List: O(log n) expected ---" << endl;
    {
        AdvancedTopics::SimpleSkipList sl;
        for (int i = 0; i < 1000; i++) sl.insert(rand() % 10000);
        cout << "  1000 insertions" << endl;
        cout << "  search(5000): " << (sl.search(5000) ? "found" : "not found") << endl;
        cout << "  search(99999): " << (sl.search(99999) ? "found" : "not found") << endl;
    }

    cout << "\n=== C. ОНЛАЙН-АЛГОРИТМЫ ===" << endl;

    cout << "--- Paging: FIFO vs LRU vs OPT ---" << endl;
    {
        // Генерация запросов: случайные страницы
        int num_pages = 10, num_requests = 1000, cache_size = 5;
        vector<int> requests(num_requests);
        for (int i = 0; i < num_requests; i++) requests[i] = rand() % num_pages;

        AdvancedTopics::PagingSimulator ps(cache_size);
        ps.simulate(requests);

        cout << "  cache_size=" << cache_size << ", pages=" << num_pages
             << ", requests=" << num_requests << endl;
        cout << "    FIFO: " << ps.faults_fifo << " faults" << endl;
        cout << "    LRU:  " << ps.faults_lru << " faults" << endl;
        cout << "    OPT:  " << ps.faults_opt << " faults (lower bound)" << endl;
        cout << "    FIFO/OPT ratio: " << (double)ps.faults_fifo / ps.faults_opt << endl;
        cout << "    LRU/OPT ratio:  " << (double)ps.faults_lru / ps.faults_opt << endl;
    }

    return 0;
}
#endif

#endif // ALGO_ANALYSIS_H_CPP
