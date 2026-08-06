#ifndef COMBINATORICS_D_CPP
#define COMBINATORICS_D_CPP

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// =============================================================
// D. ТЕОРИЯ ПЕРЕЧИСЛЕНИЯ
// =============================================================
// Структура md: A. Действия групп и циклы
//               → B. Лемма Бернсайда (ожерелья, куб)
//               → C. Теория Пойа (цикловой индекс, графы)
//
// Переиспользуем: powmod, modinv (ModArithmetic), готовые ожерелья
// necklace_count и браслеты bracelet_count (a.cpp, E.4) — здесь общий
// механизм Бернсайда-Пойа, воспроизводящий их результаты.
// Перестановки — 0-based массивы размера n; раскраски по модулю.
//
// Содержит:
//   A. Циклы: cycle_type, cycles_count, perms_with_cycle_type
//   B. Бернсайд: burnside_orbits; группы: cyclic_group, dihedral_group,
//      cube_rotations (перестановки граней и вершин куба)
//   C. Пойа: polya_two_color, graph_orbits
//
// ВАЖНО (модуль): mod простой, |G| обратим по mod (для 1e9+7 — ок).
// Перестановки задают группу в перестановочном представлении: ответ
// не зависит от нумерации элементов.

#define COMBINATORICS_MAIN
#include "../a/a.cpp"

struct BurnsidePolya : Combinatorics {

// =============================================================
// A. ЦИКЛОВОЕ РАЗЛОЖЕНИЕ ПЕРЕСТАНОВОК
// =============================================================

// --- A.1.1. Цикловой тип перестановки O(n) ---
// p — перестановка 0..n−1; возвращает cnt[k] — число циклов длины k.
vector<long long> cycle_type(const vector<long long>& p) {
    int n = (int)p.size();
    vector<long long> cnt(n + 1, 0);
    vector<char> vis(n, 0);
    for (int i = 0; i < n; i++)
        if (!vis[i]) {
            int len = 0, v = i;
            while (!vis[v]) { vis[v] = 1; len++; v = (int)p[v]; }
            cnt[len]++;
        }
    return cnt;
}

// --- A.1.1. Число циклов перестановки O(n) ---
long long cycles_count(const vector<long long>& p) {
    long long c = 0;
    int n = (int)p.size();
    vector<char> vis(n, 0);
    for (int i = 0; i < n; i++)
        if (!vis[i]) {
            c++;
            int v = i;
            while (!vis[v]) { vis[v] = 1; v = (int)p[v]; }
        }
    return c;
}

// --- A.1.1. Число перестановок с цикловым типом cnt O(n) ---
// n! / ∏ₖ (k^{cnt[k]}·cnt[k]!): выбрать элементы циклов (мультином),
// намотать на круги ((k−1)! на цикл), разделить на cnt[k]! — циклы
// одной длины неразличимы. Деление — по модулю (mod простой).
long long perms_with_cycle_type(int n, const vector<long long>& cnt, long long mod) {
    long long res = 1;
    for (int i = 2; i <= n; i++) res = res * i % mod;       // n!
    long long den = 1;
    for (int k = 1; k <= n; k++) {
        for (int i = 0; i < (int)cnt[k]; i++) den = den * k % mod;    // k^{cnt[k]}
        for (int i = 1; i <= (int)cnt[k]; i++) den = den * i % mod;   // cnt[k]!
    }
    return res * modinv(den, mod) % mod;
}

// =============================================================
// B. ЛЕММА БЕРНСАЙДА
// =============================================================

// --- B.1.3. Число орбит раскрасок n элементов k цветами O(|G|·n) ---
// N = (1/|G|)·Σ_g k^{c(g)}, c(g) — число циклов: неподвижная раскраска
// постоянна на каждом цикле, значение на цикле выбирается независимо.
long long burnside_orbits(const vector<vector<long long>>& perms, int n,
                          long long k, long long mod) {
    long long sum = 0;
    for (const auto& p : perms)
        sum = (sum + powmod(k, cycles_count(p), mod)) % mod;
    return sum * modinv((long long)perms.size(), mod) % mod;
}

// --- B.2.1. Группа поворотов n-угольника C_n (n перестановок) ---
vector<vector<long long>> cyclic_group(int n) {
    vector<vector<long long>> g;
    for (int s = 0; s < n; s++) {
        vector<long long> p(n);
        for (int i = 0; i < n; i++) p[i] = (i + s) % n;
        g.push_back(p);
    }
    return g;
}

// --- B.2.2. Диэдральная группа D_n (2n перестановок) ---
// n поворотов + n отражений p[i] = (s − i) mod n; при нечётном n каждое
// отражение фиксирует одну бусину, при чётном — две (попеременно).
vector<vector<long long>> dihedral_group(int n) {
    vector<vector<long long>> g = cyclic_group(n);
    for (int s = 0; s < n; s++) {
        vector<long long> p(n);
        for (int i = 0; i < n; i++) p[i] = (s - i + n) % n;
        g.push_back(p);
    }
    return g;
}

// --- B.3.1. Повороты куба: перестановки 6 граней и 8 вершин ---
// Генератор: все 24 ортогональные матрицы 3×3 с det = +1 (правая тройка
// x', y', z' = x'×y' единичных направлений). Грань задаётся нормалью
// ±x/±y/±z, вершина — координатами (±1, ±1, ±1).
// Возвращает {перестановки граней, перестановки вершин} — по 24 штуки.
pair<vector<vector<long long>>, vector<vector<long long>>> cube_rotations() {
    int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    auto idx_dir = [&](int x, int y, int z) {
        for (int i = 0; i < 6; i++)
            if (dirs[i][0] == x && dirs[i][1] == y && dirs[i][2] == z) return i;
        return -1;
    };
    vector<vector<long long>> faces, verts;
    for (int a = 0; a < 6; a++) {
        for (int b = 0; b < 6; b++) {
            int x1 = dirs[a][0], y1 = dirs[a][1], z1 = dirs[a][2];
            int x2 = dirs[b][0], y2 = dirs[b][1], z2 = dirs[b][2];
            if (x1 * x2 + y1 * y2 + z1 * z2 != 0) continue;   // y' ⊥ x'
            int x3 = y1 * z2 - z1 * y2;                       // z' = x' × y'
            int y3 = z1 * x2 - x1 * z2;
            int z3 = x1 * y2 - y1 * x2;
            vector<long long> pf(6), pv(8);
            for (int i = 0; i < 6; i++) {                     // грань по нормали
                int nx = dirs[i][0], ny = dirs[i][1], nz = dirs[i][2];
                // R·n в базисе (x', y', z'): координаты = скалярные проекции
                int rx = nx * x1 + ny * y1 + nz * z1;
                int ry = nx * x2 + ny * y2 + nz * z2;
                int rz = nx * x3 + ny * y3 + nz * z3;
                pf[i] = idx_dir(rx, ry, rz);
            }
            for (int v = 0; v < 8; v++) {                     // вершина (±1,±1,±1)
                int vx = (v & 1) ? 1 : -1, vy = (v & 2) ? 1 : -1, vz = (v & 4) ? 1 : -1;
                int rx = vx * x1 + vy * y1 + vz * z1;         // R·v — координаты
                int ry = vx * x2 + vy * y2 + vz * z2;
                int rz = vx * x3 + vy * y3 + vz * z3;
                int idx = (rx > 0) | ((ry > 0) << 1) | ((rz > 0) << 2);
                pv[v] = idx;
            }
            faces.push_back(pf);
            verts.push_back(pv);
        }
    }
    return {faces, verts};
}

// =============================================================
// C. ТЕОРИЯ ПОЙА
// =============================================================

// --- C.2.3. Распределение орбит по числу «чёрных» элементов ---
// Два цвета (веса 1 и t): a[j] = число орбит с ровно j элементами
// первого цвета: a = (1/|G|)·Σ_g ∏ₖ (1 + tᵏ)^{cntₖ(g)}. Цикл длины k
// вносит множитель (1 + tᵏ) — полином умножаем сдвигом: poly += tᵏ·poly.
// Сложность: O(|G|·n²).
vector<long long> polya_two_color(const vector<vector<long long>>& perms,
                                  int n, long long mod) {
    vector<long long> res(n + 1, 0);
    for (const auto& p : perms) {
        vector<long long> cnt = cycle_type(p);
        vector<long long> poly(n + 1, 0);
        poly[0] = 1;
        for (int k = 1; k <= n; k++)
            for (int r = 0; r < cnt[k]; r++) {
                vector<long long> nxt = poly;
                for (int j = k; j <= n; j++)
                    nxt[j] = (nxt[j] + poly[j - k]) % mod;
                poly = nxt;
            }
        for (int j = 0; j <= n; j++) res[j] = (res[j] + poly[j]) % mod;
    }
    long long inv = modinv((long long)perms.size(), mod);
    for (auto& x : res) x = x * inv % mod;
    return res;
}

// --- C.3.3. Число неизоморфных графов на n вершинах O(n!·n²) ---
// S_n действует на рёбрах (их C(n,2)): перестановка вершин индуцирует
// перестановку рёбер; граф = 2-раскраска рёбер → Бернсайд по всем n!
// перестановкам. Только малые n (≤ 7): при n = 8 перебор уже 40320·28.
long long graph_orbits(int n, long long mod) {
    if (n <= 1) return 1;                       // один граф на 0/1 вершине
    vector<pair<int, int>> edges;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) edges.push_back({i, j});
    int m = (int)edges.size();
    vector<long long> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i;
    long long sum = 0;
    do {
        vector<char> vis(m, 0);
        long long c = 0;
        for (int e = 0; e < m; e++)
            if (!vis[e]) {
                c++;
                int cur = e;
                while (!vis[cur]) {
                    vis[cur] = 1;
                    int a = (int)perm[edges[cur].first];
                    int b = (int)perm[edges[cur].second];
                    if (a > b) swap(a, b);
                    // индекс ребра {a, b}, a < b: a·(2n−a−1)/2 + (b−a−1)
                    cur = a * (2 * n - a - 1) / 2 + (b - a - 1);
                }
            }
        sum = (sum + powmod(2, c, mod)) % mod;
    } while (next_permutation(perm.begin(), perm.end()));
    long long fact = 1;
    for (int i = 2; i <= n; i++) fact = fact * i % mod;
    return sum * modinv(fact, mod) % mod;
}

}; // конец struct BurnsidePolya

#ifndef BURNSIDE_POLYA_MAIN
signed main() {
    BurnsidePolya bp;
    const long long MOD = 1000000007;

    cout << "=== A. Циклы ===" << endl;
    vector<long long> p = {1, 2, 0, 4, 3};            // (0 1 2)(3 4)
    auto cnt = bp.cycle_type(p);
    cout << "cycle_type((0 1 2)(3 4)): cnt[2] = " << cnt[2] << ", cnt[3] = " << cnt[3]
         << ", циклов = " << bp.cycles_count(p) << " (ожид. 1, 1, 2)" << endl;
    vector<long long> ct(6, 0);                       // n = 5: два цикла: длины 2 и 3
    ct[2] = 1; ct[3] = 1;
    cout << "perms_with_cycle_type(5, {2,3}) = " << bp.perms_with_cycle_type(5, ct, MOD)
         << " (ожид. 20)" << endl;
    // самопроверка: сумма по всем типам = n!
    vector<long long> c6(7, 0);
    long long total = 0;
    function<void(int, int)> rec = [&](int k, int left) {
        if (k == 0) {
            if (left == 0) total += bp.perms_with_cycle_type(6, c6, MOD);
            return;
        }
        for (int c = 0; c * k <= left; c++) {
            c6[k] = c;
            rec(k - 1, left - c * k);
        }
        c6[k] = 0;
    };
    rec(6, 6);
    cout << "сумма perms_with_cycle_type по всем типам (n=6) = " << total
         << " (ожид. 720)" << endl;

    cout << "\n=== B. Лемма Бернсайда ===" << endl;
    auto c6g = bp.cyclic_group(6);
    cout << "ожерелья C_6, 2 цвета = " << bp.burnside_orbits(c6g, 6, 2, MOD)
         << " (ожид. 14; necklace_count(2,6) = " << bp.necklace_count(2, 6) << ")" << endl;
    auto d6g = bp.dihedral_group(6);
    cout << "браслеты D_6, 2 цвета = " << bp.burnside_orbits(d6g, 6, 2, MOD)
         << " (ожид. 13; bracelet_count(2,6) = " << bp.bracelet_count(2, 6) << ")" << endl;
    auto c5g = bp.cyclic_group(5);
    cout << "ожерелья C_5, 3 цвета = " << bp.burnside_orbits(c5g, 5, 3, MOD)
         << " (ожид. 51; necklace_count(3,5) = " << bp.necklace_count(3, 5) << ")" << endl;
    auto [faces, verts] = bp.cube_rotations();
    cout << "поворотов куба = " << faces.size() << " (ожид. 24)" << endl;
    cout << "куб, грани, 2 цвета = " << bp.burnside_orbits(faces, 6, 2, MOD)
         << " (ожид. 10), вершины, 2 цвета = " << bp.burnside_orbits(verts, 8, 2, MOD)
         << " (ожид. 23), грани, 3 цвета = " << bp.burnside_orbits(faces, 6, 3, MOD)
         << " (ожид. 57)" << endl;

    cout << "\n=== C. Теория Пойа ===" << endl;
    auto dist6 = bp.polya_two_color(c6g, 6, MOD);
    cout << "polya_two_color(C_6) = ";
    for (long long x : dist6) cout << x << " ";
    cout << "(ожид. 1 1 3 4 3 1 1; дихлорбензол: a[2] = " << dist6[2] << ")" << endl;
    auto distf = bp.polya_two_color(faces, 6, MOD);
    cout << "polya_two_color(куб, грани) = ";
    for (long long x : distf) cout << x << " ";
    cout << "(ожид. 1 1 2 2 2 1 1)" << endl;
    long long sum6 = 0, sumf = 0;
    for (long long x : dist6) sum6 += x;
    for (long long x : distf) sumf += x;
    cout << "сумма распределения = число орбит 2-цветной раскраски: "
         << (sum6 == bp.burnside_orbits(c6g, 6, 2, MOD) &&
             sumf == bp.burnside_orbits(faces, 6, 2, MOD) ? "OK" : "FAIL") << endl;
    cout << "graph_orbits(4) = " << bp.graph_orbits(4, MOD)
         << " (ожид. 11), (5) = " << bp.graph_orbits(5, MOD)
         << " (ожид. 34), (6) = " << bp.graph_orbits(6, MOD)
         << " (ожид. 156)" << endl;
}
#endif // BURNSIDE_POLYA_MAIN
#endif // COMBINATORICS_D_CPP
