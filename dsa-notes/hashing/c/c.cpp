#ifndef HASHING_C_CPP
#define HASHING_C_CPP

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <cmath>
#include <string>
#include <cassert>
#include <chrono>
using namespace std;

// =============================================================
// C. ХЭШ-ТАБЛИЦЫ
// =============================================================
// Структура md: A. Структура хэш-таблицы
//               → B. Методы разрешения коллизий
//               → C. Продвинутые реализации
//               → D. Тестирование и бенчмаркинг
//
// HashTables — наследует HashingMath (b.cpp).
// Separate Chaining, Open Addressing (Linear/Quadratic/Double),
// Cuckoo Hashing, Universal Hashing, Robin Hood.

#ifndef INSIDE_HASHING_C
#define INSIDE_HASHING_C
#include "../b/b.cpp"
#endif

struct HashTables : HashingMath {

// =============================================================
// B. МЕТОДЫ РАЗРЕШЕНИЯ КОЛЛИЗИЙ
// =============================================================

// ===================== SEPARATE CHAINING =====================

struct ChainingHashTable {
    int size;
    int count;
    vector<list<pair<long long, int>>> buckets;

    ChainingHashTable(int s = 16) : size(s), count(0), buckets(s) {}

    int hash_func(long long key) {
        return ((key % size) + size) % size;
    }

    void insert(long long key, int value) {
        int idx = hash_func(key);
        for (auto& [k, v] : buckets[idx]) {
            if (k == key) { v = value; return; }
        }
        buckets[idx].emplace_back(key, value);
        count++;
        if ((double)count / size > 0.75) rehash();
    }

    bool search(long long key, int& value) {
        int idx = hash_func(key);
        for (auto& [k, v] : buckets[idx]) {
            if (k == key) { value = v; return true; }
        }
        return false;
    }

    bool erase(long long key) {
        int idx = hash_func(key);
        for (auto it = buckets[idx].begin(); it != buckets[idx].end(); ++it) {
            if (it->first == key) { buckets[idx].erase(it); count--; return true; }
        }
        return false;
    }

    void rehash() {
        int old_size = size;
        auto old_buckets = buckets;
        size *= 2;
        buckets.assign(size, {});
        count = 0;
        for (int i = 0; i < old_size; i++)
            for (auto& [k, v] : old_buckets[i])
                insert(k, v);
    }

    double load_factor() { return (double)count / size; }
};

// ===================== LINEAR PROBING =====================

struct LinearProbingTable {
    int size;
    int count;
    vector<long long> keys;
    vector<int> values;
    vector<bool> occupied;

    LinearProbingTable(int s = 16) : size(s), count(0),
        keys(s, -1), values(s, 0), occupied(s, false) {}

    int hash_func(long long key) { return ((key % size) + size) % size; }

    void insert(long long key, int value) {
        if ((double)count / size > 0.5) rehash();
        int idx = hash_func(key);
        while (occupied[idx]) {
            if (keys[idx] == key) { values[idx] = value; return; }
            idx = (idx + 1) % size;
        }
        keys[idx] = key; values[idx] = value; occupied[idx] = true; count++;
    }

    bool search(long long key, int& value) {
        int idx = hash_func(key);
        int start = idx;
        while (occupied[idx]) {
            if (keys[idx] == key) { value = values[idx]; return true; }
            idx = (idx + 1) % size;
            if (idx == start) break;
        }
        return false;
    }

    void rehash() {
        auto old_keys = keys;
        auto old_values = values;
        auto old_occ = occupied;
        int old_size = size;
        size *= 2;
        keys.assign(size, -1);
        values.assign(size, 0);
        occupied.assign(size, false);
        count = 0;
        for (int i = 0; i < old_size; i++) {
            if (old_occ[i]) {
                long long key = old_keys[i];
                int val = old_values[i];
                int idx = hash_func(key);
                while (occupied[idx]) idx = (idx + 1) % size;
                keys[idx] = key; values[idx] = val; occupied[idx] = true; count++;
            }
        }
    }
};

// ===================== DOUBLE HASHING =====================

struct DoubleHashTable {
    int size;
    int count;
    vector<long long> keys;
    vector<int> values;
    vector<bool> occupied;

    DoubleHashTable(int s = 16) : size(s), count(0),
        keys(s, -1), values(s, 0), occupied(s, false) {}

    int h1(long long key) { return ((key % size) + size) % size; }
    int h2(long long key) { return 1 + ((key % (size - 1)) + (size - 1)) % (size - 1); }

    void insert(long long key, int value) {
        if ((double)count / size > 0.5) rehash();
        int idx = h1(key);
        int step = h2(key);
        while (occupied[idx]) {
            if (keys[idx] == key) { values[idx] = value; return; }
            idx = (idx + step) % size;
        }
        keys[idx] = key; values[idx] = value; occupied[idx] = true; count++;
    }

    bool search(long long key, int& value) {
        int idx = h1(key);
        int step = h2(key);
        int start = idx;
        while (occupied[idx]) {
            if (keys[idx] == key) { value = values[idx]; return true; }
            idx = (idx + step) % size;
            if (idx == start) break;
        }
        return false;
    }

    void rehash() {
        auto old_keys = keys;
        auto old_values = values;
        auto old_occ = occupied;
        int os = size;
        size *= 2;
        keys.assign(size, -1);
        values.assign(size, 0);
        occupied.assign(size, false);
        count = 0;
        for (int i = 0; i < os; i++) {
            if (old_occ[i]) {
                long long key = old_keys[i];
                int val = old_values[i];
                int idx = h1(key);
                int step = h2(key);
                while (occupied[idx]) idx = (idx + step) % size;
                keys[idx] = key; values[idx] = val; occupied[idx] = true; count++;
            }
        }
    }
};

// ===================== CUCKOO HASHING =====================

struct CuckooHashTable {
    int size;
    vector<long long> table1, table2;
    vector<bool> occ1, occ2;
    function<int(long long)> h1, h2;

    CuckooHashTable(int s = 16) : size(s), table1(s, -1), table2(s, -1),
        occ1(s, false), occ2(s, false) {
        h1 = [](long long k) -> int { return ((k % 17) + 17) % 17; };
        h2 = [](long long k) -> int { return ((k % 13) + 13) % 13; };
    }

    bool insert(long long key, int max_loops = 32) {
        for (int i = 0; i < max_loops; i++) {
            int pos = h1(key) % size;
            if (!occ1[pos]) { table1[pos] = key; occ1[pos] = true; return true; }
            long long displaced = table1[pos];
            table1[pos] = key; key = displaced;
            pos = h2(key) % size;
            if (!occ2[pos]) { table2[pos] = key; occ2[pos] = true; return true; }
            displaced = table2[pos];
            table2[pos] = key; key = displaced;
        }
        return false;  // need rehash
    }

    bool search(long long key) {
        int p1 = h1(key) % size;
        if (occ1[p1] && table1[p1] == key) return true;
        int p2 = h2(key) % size;
        if (occ2[p2] && table2[p2] == key) return true;
        return false;
    }
};

// =============================================================
// C. ПРОДВИНУТЫЕ РЕАЛИЗАЦИИ
// =============================================================

// ===================== UNIVERSAL HASHING =====================

struct UniversalHashTable {
    long long p, a, b, m;
    int count;

    UniversalHashTable(long long prime = 1000003, long long modulus = 1000000)
        : p(prime), m(modulus), count(0) {
        a = 1 + rand() % (p - 1);
        b = rand() % p;
    }

    int hash(long long key) {
        return (int)(((a * key + b) % p) % m);
    }

    bool check_universal(int trials = 10000) {
        // P(h(x) = h(y)) should be <= 1/m for x != y
        int collisions = 0;
        for (int i = 0; i < trials; i++) {
            long long x = rand(), y = rand();
            while (y == x) y = rand();
            if (hash(x) == hash(y)) collisions++;
        }
        double observed = (double)collisions / trials;
        return observed <= 2.0 / m;  // generous bound
    }
};

// =============================================================
// D. ТЕСТИРОВАНИЕ И БЕНЧМАРКИНГ
// =============================================================

// --- D.1. Бенчмарк: сравнение стратегий ---
// Возвращает {среднее время вставки (ns), среднее время поиска (ns)}.
pair<double, double> benchmark_chaining(int n) {
    ChainingHashTable ht(max(16, n / 4));
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) ht.insert(rand(), i);
    auto mid = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        int v;
        ht.search(rand(), v);
    }
    auto end = chrono::high_resolution_clock::now();
    double insert_time = chrono::duration_cast<chrono::nanoseconds>(mid - start).count() / (double)n;
    double search_time = chrono::duration_cast<chrono::nanoseconds>(end - mid).count() / (double)n;
    return {insert_time, search_time};
}

pair<double, double> benchmark_linear(int n) {
    LinearProbingTable ht(max(16, n / 2));
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) ht.insert(rand(), i);
    auto mid = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        int v;
        ht.search(rand(), v);
    }
    auto end = chrono::high_resolution_clock::now();
    double insert_time = chrono::duration_cast<chrono::nanoseconds>(mid - start).count() / (double)n;
    double search_time = chrono::duration_cast<chrono::nanoseconds>(end - mid).count() / (double)n;
    return {insert_time, search_time};
}

// --- D.2. Фактор загрузки vs длина поиска ---
// Возвращает среднюю длину поиска для заданного α.
double avg_search_length_chaining(int n, int m) {
    ChainingHashTable ht(m);
    for (int i = 0; i < n; i++) ht.insert(rand(), i);
    int total = 0;
    for (int i = 0; i < n; i++) {
        int v, steps = 0;
        int idx = ht.hash_func(rand());
        for (auto& [k, val] : ht.buckets[idx]) steps++;
        total += max(1, steps);
    }
    return (double)total / n;
};

}; // struct HashTables

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_C_MAIN
int main() {
    HashTables ht;
    srand(42);

    cout << "=== B. МЕТОДЫ РАЗРЕШЕНИЯ КОЛЛИЗИЙ ===" << endl;

    cout << "--- Separate Chaining ---" << endl;
    {
        HashTables::ChainingHashTable table(8);
        for (int i = 0; i < 20; i++) table.insert(i * 7, i);
        cout << "  size=" << table.size << " count=" << table.count
             << " α=" << table.load_factor() << endl;
        int v;
        cout << "  search(14): " << (table.search(14, v) ? "found " + to_string(v) : "not found") << endl;
        cout << "  search(99): " << (table.search(99, v) ? "found" : "not found") << endl;
    }

    cout << "\n--- Linear Probing ---" << endl;
    {
        HashTables::LinearProbingTable table(16);
        for (int i = 0; i < 10; i++) table.insert(i * 13, i);
        int v;
        cout << "  search(26): " << (table.search(26, v) ? "found " + to_string(v) : "not found") << endl;
        cout << "  search(50): " << (table.search(50, v) ? "found" : "not found") << endl;
    }

    cout << "\n--- Double Hashing ---" << endl;
    {
        HashTables::DoubleHashTable table(16);
        for (int i = 0; i < 10; i++) table.insert(i * 17, i);
        int v;
        cout << "  search(34): " << (table.search(34, v) ? "found " + to_string(v) : "not found") << endl;
    }

    cout << "\n--- Cuckoo Hashing ---" << endl;
    {
        HashTables::CuckooHashTable table(16);
        for (int i = 0; i < 12; i++) table.insert(i * 11);
        cout << "  search(33): " << (table.search(33) ? "found" : "not found") << endl;
        cout << "  search(50): " << (table.search(50) ? "found" : "not found") << endl;
    }

    cout << "\n=== C. ПРОДВИНУТЫЕ РЕАЛИЗАЦИИ ===" << endl;

    cout << "--- Universal Hashing ---" << endl;
    {
        HashTables::UniversalHashTable uht;
        cout << "  hash(42) = " << uht.hash(42) << endl;
        cout << "  hash(100) = " << uht.hash(100) << endl;
        cout << "  Universal property: " << (uht.check_universal() ? "OK" : "FAIL") << endl;
    }

    cout << "\n=== D. БЕНЧМАРКИНГ ===" << endl;

    cout << "--- Сравнение Chaining vs Linear Probing ---" << endl;
    for (int n : {100, 1000, 10000}) {
        auto [ci, cs] = ht.benchmark_chaining(n);
        auto [li, ls] = ht.benchmark_linear(n);
        cout << "  n=" << n << ":" << endl;
        cout << "    Chaining:  insert=" << ci << "ns search=" << cs << "ns" << endl;
        cout << "    Linear:    insert=" << li << "ns search=" << ls << "ns" << endl;
    }

    return 0;
}
#endif

#endif // HASHING_C_CPP
