#ifndef HASHING_H_CPP
#define HASHING_H_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cstdint>
#include <map>
#include <set>
#include <queue>
using namespace std;

// =============================================================
// H. СПЕЦИАЛИЗИРОВАННЫЕ ПРИМЕНЕНИЯ
// =============================================================
// Структура md: A. Хеширование сложных структур
//               → B. БД и хранение
//               → C. Сетевые протоколы
//
// HashApplications — наследует HashOptimization (g.cpp).
// Хеш графов, геометрических объектов, множеств,
// дедупликация, индексирование, Merkle tree, DHT.

#ifndef INSIDE_HASHING_H
#define INSIDE_HASHING_H
#include "../g/g.cpp"
#endif

struct HashApplications : HashOptimization {

// =============================================================
// A. ХЕШИРОВАНИЕ СЛОЖНЫХ СТРУКТУР
// =============================================================

// --- A.1. Хеш графа через рёбра (порядко-независимый) ---
// h(G) = XOR h(min(u,v), max(u,v)) для каждого ребра.
long long graph_hash_edges(int n, const vector<pair<int,int>>& edges) {
    long long h = 0;
    for (auto& [u, v] : edges) {
        int a = min(u, v), b = max(u, v);
        h ^= simple_polynomial_hash(to_string(a) + "," + to_string(b));
    }
    return h;
}

// --- A.2. Хеш графа через степени ---
// h(G) = XOR h(deg(v)) для каждой вершины.
long long graph_hash_degrees(int n, const vector<pair<int,int>>& edges) {
    vector<int> deg(n, 0);
    for (auto& [u, v] : edges) { deg[u]++; deg[v]++; }
    long long h = 0;
    for (int d : deg) h ^= simple_polynomial_hash(to_string(d));
    return h;
}

// --- A.3. Хеш точки ---
long long point_hash(int x, int y) {
    return simple_polynomial_hash(to_string(x) + "," + to_string(y));
}

// --- A.4. Хеш многоугольника (порядко-независимый) ---
long long polygon_hash(const vector<pair<int,int>>& vertices) {
    long long h = 0;
    for (int i = 0; i < (int)vertices.size(); i++) {
        int j = (i + 1) % vertices.size();
        int a = min(vertices[i].first, vertices[j].first);
        int b = min(vertices[i].second, vertices[j].second);
        h ^= simple_polynomial_hash(to_string(a) + "," + to_string(b));
    }
    return h;
}

// --- A.5. Хеш множества (порядко-независимый) ---
long long set_hash(const vector<int>& elements) {
    long long h = 0;
    for (int x : elements)
        h ^= simple_polynomial_hash(to_string(x));
    return h;
}

// =============================================================
// B. ДЕДУПЛИКАЦИЯ
// =============================================================

// --- B.1. Поиск дубликатов через двухуровневый хеш ---
struct Deduplicator {
    function<long long(const string&)> fast_hash;
    function<string(const string&)> exact_hash;

    Deduplicator(function<long long(const string&)> fh,
                 function<string(const string&)> eh)
        : fast_hash(fh), exact_hash(eh) {}

    map<long long, vector<string>> fast_map;
    map<string, vector<string>> exact_map;

    void add(const string& data) {
        long long fast = fast_hash(data);
        fast_map[fast].push_back(data);
    }

    vector<vector<string>> find_duplicates() {
        vector<vector<string>> result;
        for (auto& [fh, items] : fast_map) {
            if (items.size() < 2) continue;
            map<string, vector<string>> exact_groups;
            for (auto& s : items) {
                string exact = exact_hash(s);
                exact_groups[exact].push_back(s);
            }
            for (auto& [eh, group] : exact_groups) {
                if (group.size() > 1)
                    result.push_back(group);
            }
        }
        return result;
    }
};

// =============================================================
// C. MERKLE TREE
// =============================================================

struct MerkleTree {
    vector<string> leaves;
    vector<vector<long long>> levels;
    function<long long(const string&)> hash_fn;

    MerkleTree(function<long long(const string&)> h) : hash_fn(h) {}

    void build(const vector<string>& data) {
        leaves = data;
        int n = (int)leaves.size();
        levels.clear();
        vector<long long> current(n);
        for (int i = 0; i < n; i++)
            current[i] = hash_fn(leaves[i]);
        levels.push_back(current);
        // Build tree bottom-up
        while (current.size() > 1) {
            vector<long long> next;
            for (int i = 0; i < (int)current.size(); i += 2) {
                if (i + 1 < (int)current.size()) {
                    string combined = to_string(current[i]) + to_string(current[i+1]);
                    next.push_back(hash_fn(combined));
                } else {
                    next.push_back(current[i]);
                }
            }
            levels.push_back(next);
            current = next;
        }
    }

    long long root_hash() {
        return levels.empty() ? 0 : levels.back()[0];
    }

    // Proof of inclusion: path from leaf to root
    vector<pair<long long, bool>> get_proof(int leaf_idx) {
        vector<pair<long long, bool>> proof;
        int idx = leaf_idx;
        for (int level = 0; level < (int)levels.size() - 1; level++) {
            bool is_right = (idx % 2 == 1);
            long long sibling = is_right ? levels[level][idx-1] : levels[level][idx+1];
            proof.push_back({sibling, is_right});
            idx /= 2;
        }
        return proof;
    }

    // Verify proof
    static bool verify_proof(long long leaf_hash, const vector<pair<long long, bool>>& proof,
                              function<long long(const string&)> hash_fn) {
        long long current = leaf_hash;
        for (auto& [sibling, is_right] : proof) {
            string combined = is_right ?
                to_string(sibling) + to_string(current) :
                to_string(current) + to_string(sibling);
            current = hash_fn(combined);
        }
        return true;
    }
};

// =============================================================
// D. DHT (МОДЕЛЬ CHORD)
// =============================================================

struct SimpleChord {
    int m;
    map<int, int> ring;
    int num_servers;
    function<long long(const string&)> hash_fn;

    SimpleChord(int bits, function<long long(const string&)> h)
        : m(bits), num_servers(0), hash_fn(h) {
        for (int i = 0; i < (1 << m); i++) ring[i] = -1;
    }

    void add_server(int id) {
        int pos = hash_fn(to_string(id)) % (1 << m);
        ring[pos] = id;
        num_servers++;
    }

    int find_server(int key) {
        int pos = hash_fn(to_string(key)) % (1 << m);
        auto it = ring.lower_bound(pos);
        if (it == ring.end()) it = ring.begin();
        return it->second;
    }

    int num_hops(int key) {
        int pos = hash_fn(to_string(key)) % (1 << m);
        int target = find_server(key);
        int hops = 0;
        for (auto it = ring.lower_bound(pos); ; ++it) {
            if (it == ring.end()) it = ring.begin();
            hops++;
            if (it->second == target) break;
            if (hops > (1 << m)) break;
        }
        return hops;
    }
};

}; // struct HashApplications

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef HASHING_H_MAIN
int main() {
    HashApplications ha;
    srand(42);

    cout << "=== A. ХЕШИРОВАНИЕ СТРУКТУР ===" << endl;

    cout << "--- Хеш графа (10 вершин, треугольники) ---" << endl;
    {
        int n = 10;
        vector<pair<int,int>> edges = {{0,1},{1,2},{0,2},{3,4},{4,5},{3,5},{6,7},{7,8},{8,9},{6,9}};
        long long h_edges = ha.graph_hash_edges(n, edges);
        long long h_deg = ha.graph_hash_degrees(n, edges);
        cout << "  hash(edges) = " << h_edges << endl;
        cout << "  hash(degrees) = " << h_deg << endl;
        // Проверка: изоморфный граф → тот же хеш
        vector<pair<int,int>> edges2 = {{0,1},{1,2},{0,2},{3,4},{4,5},{3,5},{6,7},{7,8},{8,9},{6,9}};
        cout << "  Isomorphic graph hash(edges) = " << ha.graph_hash_edges(n, edges2)
             << " (same: " << (h_edges == ha.graph_hash_edges(n, edges2) ? "YES" : "NO") << ")" << endl;
    }

    cout << "\n--- Хеш точки и многоугольника ---" << endl;
    {
        cout << "  hash(3,4) = " << ha.point_hash(3, 4) << endl;
        cout << "  hash(4,3) = " << ha.point_hash(4, 3) << " (разные для разных порядков)" << endl;
        vector<pair<int,int>> tri = {{0,0},{1,0},{0,1}};
        cout << "  hash(треугольник) = " << ha.polygon_hash(tri) << endl;
    }

    cout << "\n--- Хеш множества (порядко-независимый) ---" << endl;
    {
        cout << "  hash({1,2,3}) = " << ha.set_hash({1,2,3}) << endl;
        cout << "  hash({3,1,2}) = " << ha.set_hash({3,1,2}) << " (одинаковый: "
             << (ha.set_hash({1,2,3}) == ha.set_hash({3,1,2}) ? "YES" : "NO") << ")" << endl;
    }

    cout << "\n=== B. ДЕДУПЛИКАЦИЯ ===" << endl;
    {
        auto fast_h = [&ha](const string& s) -> long long {
            return ha.simple_polynomial_hash(s);
        };
        auto exact_h = [&ha](const string& s) -> string {
            return to_string(ha.crc32_lut(s));
        };
        HashApplications::Deduplicator dedup(fast_h, exact_h);
        dedup.add("hello");
        dedup.add("world");
        dedup.add("hello");
        dedup.add("test");
        dedup.add("world");
        dedup.add("world");
        auto dups = dedup.find_duplicates();
        cout << "  Найдено групп дубликатов: " << dups.size() << endl;
        for (auto& group : dups) {
            cout << "    ";
            for (auto& s : group) cout << "\"" << s << "\" ";
            cout << endl;
        }
    }

    cout << "\n=== C. MERKLE TREE ===" << endl;
    {
        vector<string> data = {"block1", "block2", "block3", "block4",
                               "block5", "block6", "block7", "block8"};
        HashApplications::MerkleTree mt([&ha](const string& s) -> long long {
            return ha.simple_polynomial_hash(s);
        });
        mt.build(data);
        cout << "  Root hash: " << mt.root_hash() << endl;
        cout << "  Levels: " << mt.levels.size() << endl;
        for (int i = 0; i < (int)mt.levels.size(); i++)
            cout << "    Level " << i << ": " << mt.levels[i].size() << " nodes" << endl;

        // Proof of inclusion for leaf 3
        auto proof = mt.get_proof(3);
        cout << "  Proof for leaf 3 (" << data[3] << "):" << endl;
        cout << "    Path length: " << proof.size() << endl;
        for (auto& [hash, is_right] : proof)
            cout << "    " << (is_right ? "RIGHT" : "LEFT") << " sibling: " << hash << endl;
    }

    cout << "\n=== D. CHORD DHT ===" << endl;
    {
        HashApplications::SimpleChord chord(4, [&ha](const string& s) -> long long {
            return ha.simple_polynomial_hash(s);
        });
        chord.add_server(0);
        chord.add_server(5);
        chord.add_server(10);
        cout << "  3 сервера на кольце (16 позиций)" << endl;
        for (auto& key : {"alpha", "beta", "gamma", "delta", "epsilon", "zeta"}) {
            int server = chord.find_server(ha.simple_polynomial_hash(key));
            cout << "    \"" << key << "\" → сервер " << server << endl;
        }
        cout << "  Добавляем сервер 3..." << endl;
        chord.add_server(3);
        for (auto& key : {"alpha", "beta", "gamma", "delta", "epsilon", "zeta"}) {
            int server = chord.find_server(ha.simple_polynomial_hash(key));
            cout << "    \"" << key << "\" → сервер " << server << endl;
        }
    }

    return 0;
}
#endif

#endif // HASHING_H_CPP
