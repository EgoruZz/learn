#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>

#define DISCRETE_LOG_MAIN
#include "../b-folder/b.cpp"

using namespace std;
typedef long long ll;

// =============================================================
// D. ДИСКРЕТНЫЙ АНАЛИЗ И КОРНИ
// =============================================================
// Наследует ModArithmetic из b.cpp и добавляет:
//   - Primitive roots (первообразные корни)
//   - Index (индексы)
//   - BSGS (дискретное логарифмирование)
//   - Pohlig-Hellman (оптимизация BSGS)
//   - Tonelli-Shanks (извлечение квадратного корня)
//   - Discrete root (извлечение корня степени k)

struct DiscreteLog : ModArithmetic {

// =============================================================
// A. ПЕРВООБРАЗНЫЕ КОРНИ
// =============================================================
// Идея: найти g такое что g, g^2, ..., g^(p-1) дают все числа 1..p-1
// Проверка: g^((p-1)/q) ≢ 1 (mod p) для всех простых делителей q числа p-1
//
// Сложность: O(p^(1/2) * log p) в среднем
// Пример: primitive_root(7) = 3

ll primitive_root(ll p) {
    if (p == 2) return 1;

    ll phi = p - 1;
    ll n = phi;

    vector<ll> factors;
    for (ll i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            factors.push_back(i);
            while (n % i == 0) {
                n /= i;
            }
        }
    }
    if (n > 1) {
        factors.push_back(n);
    }

    for (ll g = 2; g < p; g++) {
        bool ok = true;
        for (ll q : factors) {
            if (powmod(g, phi / q, p) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) return g;
    }

    return -1;
}

// --- A. Все первообразные корни ---
// Идея: если g — первообразный корень, то g^k тоже первообразный ⟺ gcd(k, φ(p)) = 1
//
// Сложность: O(p * log p)
vector<ll> all_primitive_roots(ll p) {
    vector<ll> roots;
    ll g = primitive_root(p);
    if (g == -1) return roots;

    ll phi = p - 1;
    for (ll k = 1; k < phi; k++) {
        if (gcd_mod(k, phi) == 1) {
            roots.push_back(powmod(g, k, p));
        }
    }

    sort(roots.begin(), roots.end());
    return roots;
}

// =============================================================
// B. ИНДЕКСЫ (ДИСКРЕТНЫЕ ЛОГАРИФМЫ)
// =============================================================
// Идея: построить таблицу ind_g(b) для фиксированного g
// Сложность построения: O(p)
// Запрос: O(1)

vector<ll> index_table(ll g, ll p) {
    vector<ll> table(p, -1);
    ll cur = 1;
    for (ll i = 0; i < p - 1; i++) {
        table[cur] = i;
        cur = cur * g % p;
    }
    return table;
}

// --- B. Запрос индекса ---
// Возвращает ind_g(b) или -1
ll index(const vector<ll>& table, ll b, ll p) {
    b %= p;
    if (b == 0) return -1;
    return table[b];
}

// =============================================================
// C. BABY-STEP GIANT-STEP (BSGS)
// =============================================================
// Идея: найти x такое что a^x ≡ b (mod m)
// Разлагаем x = i*n + j, где n = ⌈√m⌉
// a^j ≡ b * (a^(-n))^i (mod m)
// Baby step: храним a^j в хеш-таблице
// Giant step: проверяем b * (a^(-n))^i
//
// Сложность: O(√m) время, O(√m) память
// Пример: discrete_log(2, 5, 11) = 4, потому что 2^4 = 16 ≡ 5 (mod 11)

ll discrete_log(ll a, ll b, ll m) {
    if (m == 1) return 0;
    a %= m; b %= m;
    if (b == 1) return 0;

    ll n = (ll)sqrt((double)m) + 1;

    unordered_map<ll, ll> vals;
    ll cur = 1;
    for (ll j = 0; j < n; j++) {
        if (!vals.count(cur)) {
            vals[cur] = j;
        }
        cur = (__int128)cur * a % m;
    }

    ll an = cur;
    ll an_inv = modinv(an, m);
    cur = b;
    for (ll i = 0; i <= n; i++) {
        if (vals.count(cur)) {
            return vals[cur] + i * n;
        }
        cur = (__int128)cur * an_inv % m;
    }

    return -1;
}

// --- C. BSGS для общего случая (когда gcd(a, m) ≠ 1) ---
// Идея: выносим общий делитель g = gcd(a, m) пока b не станет взаимно простым с m
//
// Сложность: O(√m) в среднем
ll discrete_log_general(ll a, ll b, ll m) {
    if (m == 1) return 0;
    a %= m; b %= m;
    if (b == 1) return 0;

    ll cnt = 0;
    ll t = 1;
    ll g;
    while ((g = gcd_mod(a, m)) > 1) {
        if (b % g) return -1;
        b /= g;
        m /= g;
        t = (__int128)t * (a / g) % m;
        cnt++;
        if (t == b) return cnt;
    }

    ll n = (ll)sqrt((double)m) + 1;

    unordered_map<ll, ll> vals;
    ll cur = 1;
    for (ll j = 0; j < n; j++) {
        if (!vals.count(cur)) {
            vals[cur] = j;
        }
        cur = (__int128)cur * a % m;
    }

    ll an = cur;
    ll an_inv = modinv(an, m);
    cur = b;
    for (ll i = 0; i <= n; i++) {
        if (vals.count(cur)) {
            return vals[cur] + i * n + cnt;
        }
        cur = (__int128)cur * an_inv % m;
    }

    return -1;
}

// =============================================================
// D. POHLIG-HELLMAN (ОПТИМИЗАЦИЯ BSGS)
// =============================================================
// Идея: разложить p-1 = ∏ q_i^{e_i} и решить задачу по каждому множителю
// Извлекаем x по цифрам в системе счисления по q_i
// Затем собираем ответ через КРТ (Китайская теорема об остатках)
//
// Сложность: O(∑ e_i * √q_i) где p-1 = ∏ q_i^{e_i}

ll pohlig_hellman(ll a, ll b, ll p) {
    ll phi = p - 1;

    vector<ll> primes, exps;
    ll n = phi;
    for (ll i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            primes.push_back(i);
            ll e = 0;
            while (n % i == 0) {
                n /= i;
                e++;
            }
            exps.push_back(e);
        }
    }
    if (n > 1) {
        primes.push_back(n);
        exps.push_back(1);
    }

    vector<ll> residues, moduli;

    for (size_t i = 0; i < primes.size(); i++) {
        ll q = primes[i];
        ll e = exps[i];
        ll qe = 1;
        for (ll j = 0; j < e; j++) qe *= q;

        ll gamma = powmod(a, phi / q, p);

        ll cur_b = b;
        ll x_mod = 0;
        ll q_pow = 1;

        bool ok = true;
        for (ll j = 0; j < e; j++) {
            ll target = powmod(cur_b, phi / (q_pow * q), p);

            ll d_j = -1;
            {
                ll m = (ll)sqrt((double)q) + 1;
                unordered_map<ll, ll> table;
                ll cur = 1;
                for (ll jj = 0; jj < m; jj++) {
                    if (!table.count(cur)) table[cur] = jj;
                    cur = (__int128)cur * gamma % p;
                }
                ll gamma_m = powmod(gamma, m, p);
                ll gamma_m_inv = modinv(gamma_m, p);
                cur = target;
                for (ll ii = 0; ii <= m; ii++) {
                    if (table.count(cur)) {
                        d_j = table[cur] + ii * m;
                        if (d_j >= q) d_j = -1;
                        break;
                    }
                    cur = (__int128)cur * gamma_m_inv % p;
                }
            }

            if (d_j == -1) { ok = false; break; }

            x_mod += d_j * q_pow;
            cur_b = (__int128)cur_b * modinv(powmod(a, d_j * q_pow, p), p) % p;
            q_pow *= q;
        }

        if (!ok) return -1;
        residues.push_back(x_mod);
        moduli.push_back(qe);
    }

    return crt(residues, moduli);
}

// =============================================================
// E. TONELLI-SHANKS (КВАДРАТНЫЙ КОРЕНЬ ПО МОДУЛЮ ПРОСТОГО)
// =============================================================
// Идея: найти x такое что x^2 ≡ n (mod p), p — простое
// Если p ≡ 3 (mod 4): x = n^((p+1)/4) mod p
// Иначе: общий алгоритм Tonelli-Shanks
//
// Сложность: O(log² p) в среднем
// Пример: tonelli_shanks(2, 7) = 3, потому что 3^2 = 9 ≡ 2 (mod 7)

ll tonelli_shanks(ll n, ll p) {
    if (p == 2) return n % 2;

    n %= p;
    if (n == 0) return 0;

    if (powmod(n, (p - 1) / 2, p) != 1) return -1;

    if (p % 4 == 3) {
        return powmod(n, (p + 1) / 4, p);
    }

    ll s = 0;
    ll q = p - 1;
    while (q % 2 == 0) {
        q /= 2;
        s++;
    }

    ll z = 2;
    while (powmod(z, (p - 1) / 2, p) != p - 1) {
        z++;
    }

    ll c = powmod(z, q, p);
    ll t = powmod(n, q, p);
    ll r = powmod(n, (q + 1) / 2, p);
    ll m = s;

    while (t != 1) {
        ll i = 1;
        ll temp = (__int128)t * t % p;
        while (temp != 1) {
            temp = (__int128)temp * temp % p;
            i++;
        }

        ll b = powmod(c, 1LL << (m - i - 1), p);
        r = (__int128)r * b % p;
        c = (__int128)b * b % p;
        t = (__int128)t * c % p;
        m = i;
    }

    return r;
}

// --- E. Все квадратные корни по модулю простого ---
// Возвращает вектор всех x: x^2 ≡ n (mod p)
vector<ll> all_sqrt_mod_p(ll n, ll p) {
    n %= p;
    if (n == 0) return {0};

    ll r = tonelli_shanks(n, p);
    if (r == -1) return {};

    if (r == p - r) return {r};
    return {min(r, p - r), max(r, p - r)};
}

// =============================================================
// F. ДИСКРЕТНЫЙ КОРЕНЬ СТЕПЕНИ K
// =============================================================
// Идея: найти x такое что x^k ≡ a (mod p)
// Через первообразный корень: g^(k*x) ≡ g^ind(a) → k*x ≡ ind(a) (mod p-1)
//
// Сложность: O(√p * log p) в среднем
// Пример: discrete_root(2, 3, 11) = 5, потому что 5^2 = 25 ≡ 3 (mod 11)

ll discrete_root(ll k, ll a, ll p) {
    if (a == 0) return 0;

    a %= p;
    if (a == 0) return 0;

    ll g = primitive_root(p);
    if (g == -1) return -1;

    ll ind_a = discrete_log(g, a, p);
    if (ind_a == -1) return -1;

    ll phi = p - 1;
    ll g_val = gcd_mod(k, phi);

    if (ind_a % g_val != 0) return -1;

    ll k1 = k / g_val;
    ll a1 = ind_a / g_val;
    ll phi1 = phi / g_val;

    ll inv_k1 = modinv(k1, phi1);
    ll x1 = (__int128)a1 * inv_k1 % phi1;

    return powmod(g, x1, p);
}

// --- F. Все решения x^k ≡ a (mod p) ---
// Возвращает вектор всех x: x^k ≡ a (mod p)
vector<ll> all_discrete_roots(ll k, ll a, ll p) {
    vector<ll> roots;
    if (a == 0) {
        roots.push_back(0);
        return roots;
    }

    a %= p;
    if (a == 0) {
        roots.push_back(0);
        return roots;
    }

    ll g = primitive_root(p);
    if (g == -1) return roots;

    ll ind_a = discrete_log(g, a, p);
    if (ind_a == -1) return roots;

    ll phi = p - 1;
    ll g_val = gcd_mod(k, phi);

    if (ind_a % g_val != 0) return roots;

    ll k1 = k / g_val;
    ll a1 = ind_a / g_val;
    ll phi1 = phi / g_val;

    ll inv_k1 = modinv(k1, phi1);
    ll x1 = (__int128)a1 * inv_k1 % phi1;

    ll result = powmod(g, x1, p);

    for (ll i = 0; i < g_val; i++) {
        roots.push_back(result);
        result = (__int128)result * powmod(g, phi / g_val, p) % p;
    }

    sort(roots.begin(), roots.end());
    roots.erase(unique(roots.begin(), roots.end()), roots.end());

    return roots;
}

}; // конец struct DiscreteLog

// =============================================================
// ТЕСТЫ
// =============================================================
#ifndef DISCRETE_LOG_STANDALONE
signed main() {
    DiscreteLog s;

    cout << "=== BSGS ===" << endl;
    cout << "discrete_log(2, 5, 11) = " << s.discrete_log(2, 5, 11) << " (ожид: 4)" << endl;
    cout << "discrete_log_general(2, 5, 11) = " << s.discrete_log_general(2, 5, 11) << endl;

    cout << "\n=== Tonelli-Shanks ===" << endl;
    cout << "tonelli_shanks(3, 11) = " << s.tonelli_shanks(3, 11) << " (ожид: 5)" << endl;
    cout << "tonelli_shanks(2, 7) = " << s.tonelli_shanks(2, 7) << endl;
    vector<ll> sqrts = s.all_sqrt_mod_p(3, 11);
    cout << "all_sqrt_mod_p(3, 11): ";
    for (ll x : sqrts) cout << x << " ";
    cout << endl;

    cout << "\n=== Primitive root ===" << endl;
    cout << "primitive_root(7) = " << s.primitive_root(7) << " (ожид: 3)" << endl;
    cout << "primitive_root(11) = " << s.primitive_root(11) << endl;
    vector<ll> roots = s.all_primitive_roots(7);
    cout << "all_primitive_roots(7): ";
    for (ll x : roots) cout << x << " ";
    cout << endl;

    cout << "\n=== Index ===" << endl;
    ll g = 3, p = 7;
    vector<ll> table = s.index_table(g, p);
    cout << "index_table for g=" << g << ", p=" << p << ":" << endl;
    for (ll i = 1; i < p; i++) {
        cout << "  ind_" << g << "(" << i << ") = " << s.index(table, i, p) << endl;
    }

    cout << "\n=== Pohlig-Hellman ===" << endl;
    cout << "pohlig_hellman(2, 5, 11) = " << s.pohlig_hellman(2, 5, 11) << " (ожид: 4)" << endl;

    cout << "\n=== Discrete root ===" << endl;
    cout << "discrete_root(2, 3, 11) = " << s.discrete_root(2, 3, 11) << " (ожид: 5)" << endl;
    cout << "discrete_root(3, 5, 7) = " << s.discrete_root(3, 5, 7) << endl;
    vector<ll> droots = s.all_discrete_roots(2, 3, 11);
    cout << "all_discrete_roots(2, 3, 11): ";
    for (ll x : droots) cout << x << " ";
    cout << endl;

    return 0;
}
#endif
