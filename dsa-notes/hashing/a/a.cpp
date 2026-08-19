#ifndef HASHING_A_CPP
#define HASHING_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <list>
using namespace std;

// =============================================================
// A. ОСНОВЫ ХЕШИРОВАНИЯ
// =============================================================
// Структура md: A. Введение в хеширование
//               → B. Принципы работы хеш-функций
//               → C. Практика в C++
//
// HashingBasics — базовый класс всей ветки hashing.
// Вводит определение хеш-функции, ключевые свойства,
// std::hash, unordered_map/set, кастомные хеши,
// типичные задачи (дубликаты, группировка, кэширование).

struct HashingBasics {

// =============================================================
// A. ВВЕДЕНИЕ В ХЕШИРОВАНИЕ
// =============================================================

// --- A.1. Простейшая полиномиальная хеш-функция ---
// h(s) = (Σ s[i] * p^i) mod m.
// O(n) время, O(1) память.
long long simple_polynomial_hash(const string& s, long long p = 31,
                                  long long m = 1e9 + 7) {
    long long hash = 0;
    long long power = 1;
    for (char c : s) {
        hash = (hash + (c - 'a' + 1) * power) % m;
        power = (power * p) % m;
    }
    return hash;
}

// --- A.2. Polinomial hash для вектора ---
// h(v) = (Σ v[i] * p^i) mod m.
long long vector_hash(const vector<int>& v, long long p = 31,
                       long long m = 1e9 + 7) {
    long long hash = 0;
    long long power = 1;
    for (int x : v) {
        hash = (hash + (long long)x * power) % m;
        power = (power * p) % m;
    }
    return hash;
}

// =============================================================
// B. ПРИНЦИПЫ РАБОТЫ
// =============================================================

// --- B.1. Вероятность коллизии (парадокс дней рождения) ---
// Для n элементов и M возможных хешей:
// P(коллизия) ≈ 1 - exp(-n^2 / (2*M)).
// O(1) время.
double collision_probability(int n, long long M) {
    return 1.0 - exp(-(double)n * n / (2.0 * M));
}

// --- B.2. Минимальное n для заданной вероятности коллизии ---
// n ≈ sqrt(2*M*ln(1/(1-p))).
int min_elements_for_collision(long long M, double target_p) {
    return (int)ceil(sqrt(2.0 * M * log(1.0 / (1.0 - target_p))));
}

// =============================================================
// C. ПРАКТИКА В C++
// =============================================================

// --- C.1. Кастомная хеш-функция для pair ---
struct pair_hash {
    size_t operator()(const pair<int, int>& p) const {
        return hash<long long>{}(((long long)p.first << 32) | p.second);
    }
};

// --- C.2. Кастомная хеш-функция для vector<int> ---
struct vector_int_hash {
    size_t operator()(const vector<int>& v) const {
        size_t h = 0;
        for (int x : v) {
            h ^= hash<int>{}(x) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

// --- C.3. Поиск дубликатов через unordered_set ---
// Возвращает уникальные элементы, оставляя только первые вхождения.
// O(n) время, O(n) память.
vector<int> find_duplicates(const vector<int>& v) {
    unordered_set<int> seen;
    vector<int> duplicates;
    for (int x : v) {
        if (seen.count(x)) duplicates.push_back(x);
        else seen.insert(x);
    }
    return duplicates;
}

// --- C.4. Группировка по хешу ключа ---
// groups[key] = список элементов с одинаковым ключом.
// O(n) время.
template<typename K, typename V>
unordered_map<K, vector<V>> group_by(
    const vector<pair<K, V>>& items) {
    unordered_map<K, vector<V>> groups;
    for (auto& [key, val] : items)
        groups[key].push_back(val);
    return groups;
}

// --- C.5. Простой LRU-кэш ---
// O(1) get/put через unordered_map + список.
struct LRUCache {
    int capacity;
    list<pair<int, int>> cache_list;  // (key, value)
    unordered_map<int, list<pair<int, int>>::iterator> cache_map;

    LRUCache(int cap) : capacity(cap) {}

    int get(int key) {
        if (cache_map.find(key) == cache_map.end()) return -1;
        cache_list.splice(cache_list.begin(), cache_list, cache_map[key]);
        return cache_map[key]->second;
    }

    void put(int key, int value) {
        if (cache_map.find(key) != cache_map.end()) {
            cache_list.splice(cache_list.begin(), cache_list, cache_map[key]);
            cache_map[key]->second = value;
            return;
        }
        if ((int)cache_list.size() == capacity) {
            cache_map.erase(cache_list.back().first);
            cache_list.pop_back();
        }
        cache_list.emplace_front(key, value);
        cache_map[key] = cache_list.begin();
    }
};

// --- C.6. Подсчёт частоты элементов ---
// O(n) время.
unordered_map<int, int> count_frequencies(const vector<int>& v) {
    unordered_map<int, int> freq;
    for (int x : v) freq[x]++;
    return freq;
}

// --- C.7. Хеш строки через std::hash ---
size_t std_hash_string(const string& s) {
    return hash<string>{}(s);
}

// --- C.8. Проверка: все ли строки различны ---
// O(n) время, O(n) память.
bool all_distinct(const vector<string>& v) {
    unordered_set<string> s(v.begin(), v.end());
    return s.size() == v.size();
}

}; // struct HashingBasics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_A_MAIN
int main() {
    HashingBasics hb;

    cout << "=== A. ВВЕДЕНИЕ В ХЕШИРОВАНИЕ ===" << endl;

    cout << "--- Полиномиальный хеш строки ---" << endl;
    vector<string> words = {"hello", "world", "hash", "test", "hello"};
    for (auto& w : words)
        cout << "  hash(\"" << w << "\") = " << hb.simple_polynomial_hash(w) << endl;

    cout << "\n--- Хеш вектора ---" << endl;
    vector<int> v1 = {1, 2, 3, 4, 5};
    vector<int> v2 = {1, 2, 3, 4, 6};
    cout << "  hash({1,2,3,4,5}) = " << hb.vector_hash(v1) << endl;
    cout << "  hash({1,2,3,4,6}) = " << hb.vector_hash(v2) << endl;

    cout << "\n=== B. ПРИНЦИПЫ РАБОТЫ ===" << endl;

    cout << "--- Вероятность коллизии (парадокс дней рождения) ---" << endl;
    long long M = 1LL << 32;  // 2^32 возможных хешей
    for (int n : {100, 1000, 5000, 10000, 50000}) {
        double p = hb.collision_probability(n, M);
        cout << "  n=" << n << ": P(коллизия) = " << p << endl;
    }
    cout << "  Для P>0.5 при M=2^32: n >= "
         << hb.min_elements_for_collision(M, 0.5) << endl;

    cout << "\n=== C. ПРАКТИКА В C++ ===" << endl;

    cout << "--- Поиск дубликатов ---" << endl;
    vector<int> data = {1, 3, 2, 3, 4, 1, 5, 2, 6};
    auto dups = hb.find_duplicates(data);
    cout << "  Данные: {1,3,2,3,4,1,5,2,6}" << endl;
    cout << "  Дубликаты: ";
    for (int x : dups) cout << x << " ";
    cout << endl;

    cout << "\n--- Группировка по ключу ---" << endl;
    vector<pair<string, int>> items = {{"a", 1}, {"b", 2}, {"a", 3}, {"b", 4}, {"c", 5}};
    auto groups = hb.group_by(items);
    cout << "  Группы:" << endl;
    for (auto& [key, vals] : groups) {
        cout << "    " << key << ": ";
        for (int v : vals) cout << v << " ";
        cout << endl;
    }

    cout << "\n--- Подсчёт частоты ---" << endl;
    vector<int> freq_data = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
    auto freq = hb.count_frequencies(freq_data);
    cout << "  Частоты:" << endl;
    for (auto& [x, c] : freq) cout << "    " << x << ": " << c << endl;

    cout << "\n--- LRU-кэш (cap=3) ---" << endl;
    HashingBasics::LRUCache lru(3);
    lru.put(1, 10); lru.put(2, 20); lru.put(3, 30);
    cout << "  get(1) = " << lru.get(1) << endl;
    lru.put(4, 40);  // вытесняет ключ 2
    cout << "  get(2) = " << lru.get(2) << " (вытеснен)" << endl;
    cout << "  get(4) = " << lru.get(4) << endl;

    cout << "\n--- Проверка уникальности строк ---" << endl;
    cout << "  {\"a\",\"b\",\"c\"}: " << (hb.all_distinct({"a","b","c"}) ? "все различны" : "есть дубликаты") << endl;
    cout << "  {\"a\",\"b\",\"a\"}: " << (hb.all_distinct({"a","b","a"}) ? "все различны" : "есть дубликаты") << endl;

    cout << "\n--- std::hash для строк ---" << endl;
    for (auto& w : {"hello", "world", "hash"})
        cout << "  hash(\"" << w << "\") = " << hb.std_hash_string(w) << endl;

    return 0;
}
#endif

#endif // HASHING_A_CPP
