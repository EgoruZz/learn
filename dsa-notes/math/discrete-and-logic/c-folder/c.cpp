#ifndef DISCRETE_LOGIC_C_CPP
#define DISCRETE_LOGIC_C_CPP

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
using namespace std;

#define FORMAL_LOGIC_MAIN
#include "../b-folder/b.cpp"

// =============================================================
// C. БУЛЕВА АЛГЕБРА И ФУНКЦИИ
// =============================================================
// Структура md: A. Булевы функции (1. Понятие и способы задания,
//               2. Существенные и фиктивные переменные, 3. Реализация
//               формулами, 4. Нормальные формы: СДНФ, СКНФ, полином
//               Жегалкина, 5. Булева алгебра, 6. Эквивалентные
//               преобразования, 7. Теорема Яблонского, 8. Предполные
//               классы, 9. Критерий Поста) → B. Логические элементы
//               и схемы (1. Вентили, 2. Мультиплексор, 3. Методы
//               минимизации: карты Карно, Квайн-МакКласки)
//
// Наследует FormalLogic (b.cpp): формулы логики высказываний — способ
// задания функций, таблицы истинности — их представление. Полином
// Жегалкина — zeta-преобразование (a.cpp A.6) с XOR. Методы обобщены:
// констант нет, кроме границ представления (n ≤ 6 для таблиц 2ⁿ,
// n ≤ 5 для числа функций 2^(2ⁿ)).
//
// Соглашение о представлении:
//   * булева функция f: {0,1}ⁿ → {0,1} — вектор int размера 2ⁿ:
//     tab[mask] — значение на наборе mask (бит i набора — xᵢ);
//   * формулы — деревья Form (b.cpp); проверка эквивалентности —
//     по таблицам (forms_equivalent, b.cpp);
//   * импликанты Квайна-МакКласки — строки длины n из '0', '1', '-':
//     позиция p ↔ переменная n−1−p (старший бит — первая позиция);
//   * вентиль — таблица из 4 битов, индекс (x<<1)|y (NOT — из 2).
//
// Содержит:
//   A. Функции: number_of_functions, function_number,
//      function_from_number, is_essential, essential_variables,
//      sdnf_formula, sknf_formula, zhegalkin_coefficients,
//      zhegalkin_to_string, in_T0, in_T1, is_self_dual, is_monotone,
//      is_linear, post_table, is_complete_system
//   B. Схемы: gate_table, gate_apply, multiplexer, demultiplexer,
//      karnaugh_order, karnaugh_map, karnaugh_print, quine_mccluskey,
//      implicant_masks, minimal_cover, implicant_to_string

struct BooleanAlgebra : FormalLogic {

// =============================================================
// A. БУЛЕВЫ ФУНКЦИИ
// =============================================================

// --- A.1.1. Число булевых функций от n переменных: 2^(2ⁿ) ---
// Каждому из 2ⁿ наборов независимо сопоставляется 0 или 1.
// n ≤ 5, иначе 2^(2ⁿ) не помещается в разрядную сетку.
unsigned long long number_of_functions(int n) {
    return 1ULL << (1ULL << n);
}

// --- A.1.2. Номер функции по вектору значений ---
// Вектор значений длины 2ⁿ читается как двоичное число:
// бит mask = 1 ⟺ f(mask) = 1. Обратное — function_from_number.
unsigned long long function_number(const vector<int>& tab) {
    unsigned long long num = 0;
    for (int i = 0; i < (int)tab.size(); i++)
        num |= (unsigned long long)tab[i] << i;
    return num;
}

// --- A.1.2. Вектор значений по номеру функции ---
vector<int> function_from_number(unsigned long long num, int n) {
    vector<int> tab(1 << n);
    for (int i = 0; i < (1 << n); i++)
        tab[i] = (num >> i) & 1;
    return tab;
}

// --- A.2. Существенная переменная ---
// xᵢ существенна, если есть пара наборов, отличающихся только битом i,
// на которых значения f различны. O(2ⁿ).
bool is_essential(const vector<int>& tab, int n, int i) {
    for (int mask = 0; mask < (1 << n); mask++) {
        if (mask & (1 << i)) continue;
        if (tab[mask] != tab[mask | (1 << i)]) return true;
    }
    return false;
}

// --- A.2. Все существенные переменные ---
vector<int> essential_variables(const vector<int>& tab, int n) {
    vector<int> res;
    for (int i = 0; i < n; i++)
        if (is_essential(tab, n, i)) res.push_back(i);
    return res;
}

// --- A.4.2. СДНФ по таблице истинности ---
// Для каждого набора с f = 1 — полная ЭК: xᵢ при xᵢ = 1, ¬xᵢ при
// xᵢ = 0; результат — дизъюнкция ЭК. Для тождественного нуля —
// формула противоречия (b.md A.4.3). n ≥ 1.
Form* sdnf_formula(const vector<int>& tab, int n) {
    vector<Form*> terms;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (!tab[mask]) continue;
        Form* conj = nullptr;
        for (int i = 0; i < n; i++) {
            Form* lit = (mask >> i) & 1 ? form_var(i) : form_not(form_var(i));
            conj = conj ? form_and(conj, lit) : lit;
        }
        terms.push_back(conj);
    }
    if (terms.empty())
        return form_and(form_var(0), form_not(form_var(0)));
    Form* res = terms[0];
    for (int t = 1; t < (int)terms.size(); t++) res = form_or(res, terms[t]);
    return res;
}

// --- A.4.3. СКНФ по таблице истинности ---
// Для каждого набора с f = 0 — полная ЭД: xᵢ при xᵢ = 0, ¬xᵢ при
// xᵢ = 1 (литералы «перевёрнуты» относительно СДНФ); результат —
// конъюнкция ЭД. Для тождественной единицы — формула тавтологии.
Form* sknf_formula(const vector<int>& tab, int n) {
    vector<Form*> terms;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (tab[mask]) continue;
        Form* disj = nullptr;
        for (int i = 0; i < n; i++) {
            Form* lit = (mask >> i) & 1 ? form_not(form_var(i)) : form_var(i);
            disj = disj ? form_or(disj, lit) : lit;
        }
        terms.push_back(disj);
    }
    if (terms.empty())
        return form_or(form_var(0), form_not(form_var(0)));
    Form* res = terms[0];
    for (int t = 1; t < (int)terms.size(); t++) res = form_and(res, terms[t]);
    return res;
}

// --- A.4.4. Коэффициенты полинома Жегалкина ---
// Полином по mod 2: f = ⊕Σ c_S · ∏_{i∈S} xᵢ. Коэффициенты — треугольник
// Паскаля по вектору значений, в коде — zeta-преобразование по
// подмножествам (a.cpp A.6) с XOR вместо сложения:
// c[mask] = ⊕_{sub ⊆ mask} a[sub]. Над GF(2) преобразование
// самодвойственно (обратное — то же). O(n·2ⁿ).
vector<int> zhegalkin_coefficients(const vector<int>& tab, int n) {
    vector<int> c = tab;
    for (int i = 1; i < (1 << n); i <<= 1)
        for (int mask = 0; mask < (1 << n); mask++)
            if (mask & i) c[mask] ^= c[mask ^ i];
    return c;
}

// --- A.4.4. Полином Жегалкина строкой ---
// Моном ∏ xᵢ по единичным битам маски; c[0] — константа «1»;
// разделитель ⊕. Нулевой полином — «0».
string zhegalkin_to_string(const vector<int>& c, int n) {
    string res;
    bool first = true;
    for (int mask = 0; mask < (1 << n); mask++) {
        if (!c[mask]) continue;
        string mon;
        if (mask == 0) mon = "1";
        else
            for (int i = 0; i < n; i++)
                if (mask & (1 << i)) mon += "x" + to_string(i);
        if (!first) res += "⊕";
        res += mon;
        first = false;
    }
    return first ? "0" : res;
}

// --- A.8. Предполные классы: проверки функций ---
// T₀: f(0, ..., 0) = 0; T₁: f(1, ..., 1) = 1.
bool in_T0(const vector<int>& tab) {
    return tab[0] == 0;
}

bool in_T1(const vector<int>& tab) {
    return tab.back() == 1;
}

// S — самодвойственные: f(¬x) = ¬f(x) для всех наборов.
bool is_self_dual(const vector<int>& tab, int n) {
    int full = (1 << n) - 1;
    for (int mask = 0; mask < (1 << n); mask++)
        if (tab[full ^ mask] == tab[mask]) return false;
    return true;
}

// M — монотонные: x ≤ y ⟹ f(x) ≤ f(y); достаточно проверить переходы,
// добавляющие один бит (любое увеличение — цепочка таких переходов).
bool is_monotone(const vector<int>& tab, int n) {
    for (int mask = 0; mask < (1 << n); mask++)
        if (tab[mask])
            for (int i = 0; i < n; i++)
                if (!(mask & (1 << i)) && !tab[mask | (1 << i)])
                    return false;
    return true;
}

// L — линейные: все коэффициенты полинома Жегалкина степени ≥ 2 — нули.
bool is_linear(const vector<int>& tab, int n) {
    vector<int> c = zhegalkin_coefficients(tab, n);
    for (int mask = 0; mask < (1 << n); mask++)
        if (__builtin_popcount(mask) >= 2 && c[mask]) return false;
    return true;
}

// --- A.7. Таблица Поста системы функций ---
// Строки — функции, столбцы — классы T₀, T₁, S, M, L: 1, если функция
// лежит в классе (по 8).
vector<vector<int>> post_table(const vector<vector<int>>& functions, int n) {
    vector<vector<int>> res;
    for (auto& f : functions)
        res.push_back({in_T0(f), in_T1(f), is_self_dual(f, n),
                       is_monotone(f, n), is_linear(f, n)});
    return res;
}

// --- A.7. Критерий полноты (теорема Яблонского, критерий Поста) ---
// Система полна ⟺ для каждого из пяти классов найдётся функция вне
// него: не существует класса, в котором лежат все функции системы.
bool is_complete_system(const vector<vector<int>>& functions, int n) {
    vector<int> all_in(5, 1);
    for (auto& f : functions) {
        all_in[0] &= in_T0(f);
        all_in[1] &= in_T1(f);
        all_in[2] &= is_self_dual(f, n);
        all_in[3] &= is_monotone(f, n);
        all_in[4] &= is_linear(f, n);
    }
    for (int c = 0; c < 5; c++)
        if (all_in[c]) return false;
    return true;
}

// =============================================================
// B. ЛОГИЧЕСКИЕ ЭЛЕМЕНТЫ И СХЕМЫ
// =============================================================

// --- B.1. Базовые логические вентили ---
// Таблица вентиля: индекс (x<<1)|y — значение на входах x, y
// (NOT — одновходовой, таблица из 2 битов по x).
vector<int> gate_table(const string& name) {
    if (name == "AND")    return {0, 0, 0, 1};
    if (name == "OR")     return {0, 1, 1, 1};
    if (name == "NOT")    return {1, 0};
    if (name == "NAND")   return {1, 1, 1, 0};
    if (name == "NOR")    return {1, 0, 0, 0};
    if (name == "XOR")    return {0, 1, 1, 0};
    if (name == "XNOR")   return {1, 0, 0, 1};
    if (name == "IMPLY")  return {1, 1, 0, 1};
    if (name == "NIMPLY") return {0, 0, 1, 0};
    return {};
}

// --- B.1. Применение вентиля к входам ---
int gate_apply(const string& name, int x, int y) {
    vector<int> t = gate_table(name);
    return (name == "NOT") ? t[x] : t[(x << 1) | y];
}

// --- B.2. Мультиплексор 2ᵏ → 1 ---
// k адресных битов addr выбирают вход: out = inputs[addr];
// |inputs| = 2ᵏ, addr ∈ [0, 2ᵏ). Реализует любую функцию от k
// переменных: vector значений функции — на входы, аргументы — адрес
// (СДНФ «в железе», A.4.2).
int multiplexer(const vector<int>& inputs, int addr) {
    return inputs[addr];
}

// --- B.2. Демультиплексор ---
// Один вход x направляется в выход sel: вектор длины 2ᵏ с x в позиции
// sel и нулями в остальных.
vector<int> demultiplexer(int k, int x, int sel) {
    vector<int> out(1 << k, 0);
    out[sel] = x;
    return out;
}

// --- B.3.1. Код Грея: соседние номера отличаются одним битом ---
int gray_code(int x) {
    return x ^ (x >> 1);
}

// --- B.3.1. Карта Карно: порядок клеток ---
// Строки (r = n/2 битов) и столбцы (n−r битов) нумеруются кодами Грея —
// соседние клетки (и через край) отличаются ровно одним битом.
// karnaugh_order[i] — номер набора (индекс в tab) для i-й клетки.
vector<int> karnaugh_order(int n) {
    int rows = n / 2, cols = n - rows;
    int rh = 1 << rows, ch = 1 << cols;
    vector<int> ord;
    for (int i = 0; i < rh; i++)
        for (int j = 0; j < ch; j++)
            ord.push_back((gray_code(i) << cols) | gray_code(j));
    return ord;
}

// --- B.3.1. Карта Карно: таблица в порядке клеток ---
vector<int> karnaugh_map(const vector<int>& tab, int n) {
    vector<int> ord = karnaugh_order(n);
    vector<int> res(ord.size());
    for (int i = 0; i < (int)ord.size(); i++) res[i] = tab[ord[i]];
    return res;
}

// --- B.3.1. Вывод карты Карно (строки × столбцы) ---
void karnaugh_print(const vector<int>& tab, int n) {
    int rows = 1 << (n / 2), cols = 1 << (n - n / 2);
    vector<int> m = karnaugh_map(tab, n);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) cout << m[i * cols + j] << " ";
        cout << endl;
    }
}

// --- B.3.2. Набор как строка битов ---
// Строка длины n, первая позиция — старший бит (как двоичное число).
string mask_to_bits(int mask, int n) {
    string s(n, '0');
    for (int i = 0; i < n; i++)
        s[n - 1 - i] = ((mask >> i) & 1) ? '1' : '0';
    return s;
}

// --- B.3.2. Склейка двух строк ---
// Если строки отличаются ровно в одной позиции и в обеих нет '-',
// возвращает строку с '-' в этой позиции; иначе — пустую строку.
string merge_terms(const string& a, const string& b) {
    string res = a;
    int diff = -1;
    for (int i = 0; i < (int)a.size(); i++) {
        if (a[i] == b[i]) continue;
        if (a[i] == '-' || b[i] == '-' || diff != -1) return "";
        diff = i;
        res[i] = '-';
    }
    return (diff == -1) ? "" : res;
}

// --- B.3.2. Метод Квайна-МакКласки: простые импликанты ---
// 1) минтермы — наборы с f = 1; 2) склейка пар, отличающихся одним
// битом, пока возможна (склеенные помечаются); 3) непомеченные
// каждого раунда — простые импликанты. O(m²·n) на раунд склейки.
vector<string> quine_mccluskey(const vector<int>& tab, int n) {
    vector<string> terms, primes;
    for (int mask = 0; mask < (1 << n); mask++)
        if (tab[mask]) terms.push_back(mask_to_bits(mask, n));
    while (!terms.empty()) {
        vector<bool> used(terms.size(), false);
        vector<string> next;
        for (int i = 0; i < (int)terms.size(); i++)
            for (int j = i + 1; j < (int)terms.size(); j++) {
                string m = merge_terms(terms[i], terms[j]);
                if (m.empty()) continue;
                used[i] = used[j] = true;
                bool dup = false;
                for (auto& t : next)
                    if (t == m) { dup = true; break; }
                if (!dup) next.push_back(m);
            }
        for (int i = 0; i < (int)terms.size(); i++)
            if (!used[i]) primes.push_back(terms[i]);
        terms = next;
    }
    return primes;
}

// --- B.3.2. Минтермы, покрываемые импликантой ---
// '-' в позиции p — бит n−1−p принимает и 0, и 1: все такие наборы.
vector<int> implicant_masks(const string& imp, int n) {
    vector<int> fixed, free_bits;
    for (int p = 0; p < n; p++) {
        if (imp[p] == '-') free_bits.push_back(n - 1 - p);
        else if (imp[p] == '1') fixed.push_back(n - 1 - p);
    }
    vector<int> res;
    int fcnt = (int)free_bits.size();
    for (int s = 0; s < (1 << fcnt); s++) {
        int mask = 0;
        for (int b : fixed) mask |= 1 << b;
        for (int b = 0; b < fcnt; b++)
            if (s & (1 << b)) mask |= 1 << free_bits[b];
        res.push_back(mask);
    }
    return res;
}

// --- B.3.2. Минимальное покрытие ---
// Перебор всех подмножеств простых импликант: подмножество, накрывающее
// все минтермы; выбирается минимальное по числу импликант (первое из
// минимальных). Импликант немного: O(2ᵏ·m), практично при n ≤ 6.
vector<int> minimal_cover(const vector<int>& tab, int n,
                          const vector<string>& primes) {
    vector<int> minterms;
    for (int mask = 0; mask < (1 << n); mask++)
        if (tab[mask]) minterms.push_back(mask);
    int k = (int)primes.size();
    vector<vector<int>> cov(k);
    for (int i = 0; i < k; i++) {
        cov[i] = implicant_masks(primes[i], n);
        sort(cov[i].begin(), cov[i].end());
    }
    vector<int> best;
    for (int s = 1; s < (1 << k); s++) {
        vector<bool> hit(1 << n, false);
        int cnt = 0;
        for (int i = 0; i < k; i++)
            if (s & (1 << i)) {
                cnt++;
                for (int m : cov[i]) hit[m] = true;
            }
        bool ok = true;
        for (int m : minterms)
            if (!hit[m]) { ok = false; break; }
        if (!ok) continue;
        if (best.empty() || cnt < (int)best.size()) {
            best.clear();
            for (int i = 0; i < k; i++)
                if (s & (1 << i)) best.push_back(i);
        }
    }
    return best;
}

// --- B.3.2. Импликанта строкой ---
// Позиция p ↔ переменная n−1−p; '-' опускается; все '-' — константа 1.
string implicant_to_string(const string& imp, int n) {
    string res;
    for (int p = 0; p < n; p++) {
        if (imp[p] == '-') continue;
        res += (imp[p] == '1') ? "x" : "¬x";
        res += to_string(n - 1 - p);
    }
    return res.empty() ? "1" : res;
}

}; // конец struct BooleanAlgebra

#ifndef BOOLEAN_ALGEBRA_MAIN
signed main() {
    BooleanAlgebra B;

    cout << "=== A. Булевы функции: способы задания ===" << endl;
    cout << "Число функций от 2 переменных: " << B.number_of_functions(2)
         << " (ожидаем 16)" << endl;
    vector<int> f_or = {0, 1, 1, 1}; // x0∨x1
    cout << "Номер функции x0∨x1: " << B.function_number(f_or)
         << " (ожидаем 14 = 1110₂)" << endl;
    auto back = B.function_from_number(14, 2);
    cout << "Вектор по номеру 14: ";
    for (int v : back) cout << v;
    cout << " (ожидаем 0111)" << endl;

    cout << "\n=== A. Существенные и фиктивные переменные ===" << endl;
    vector<int> f_not3 = {1, 0, 1, 0, 1, 0, 1, 0}; // ¬x0 от 3 переменных
    cout << "¬x0: существенные: ";
    for (int v : B.essential_variables(f_not3, 3)) cout << v << " ";
    cout << "(ожидаем 0)" << endl;
    cout << "x0∨x1: существенные: ";
    for (int v : B.essential_variables(f_or, 2)) cout << v << " ";
    cout << "(ожидаем 0 1)" << endl;

    cout << "\n=== A. Нормальные формы ===" << endl;
    // f = x0∨x1: СДНФ = (¬x0∧x1)∨(x0∧¬x1)∨(x0∧x1), СКНФ = x0∨x1
    Form* sd = B.sdnf_formula(f_or, 2);
    Form* sk = B.sknf_formula(f_or, 2);
    cout << "СДНФ: " << B.form_to_string(sd) << endl;
    cout << "СКНФ: " << B.form_to_string(sk) << endl;
    Form* fx0fx1 = B.form_or(B.form_var(0), B.form_var(1));
    cout << "СДНФ ≡ x0∨x1: " << B.forms_equivalent(sd, fx0fx1, 2)
         << ", СКНФ ≡ x0∨x1: " << B.forms_equivalent(sk, fx0fx1, 2) << endl;
    vector<int> zg = B.zhegalkin_coefficients(f_or, 2);
    cout << "Полином Жегалкина x0∨x1: " << B.zhegalkin_to_string(zg, 2)
         << " (ожидаем x0⊕x1⊕x0x1)" << endl;

    cout << "\n=== A. Предполные классы ===" << endl;
    vector<int> f0 = {0, 0, 0, 0}; // константа 0: T0, M, L
    cout << "0: T0=" << B.in_T0(f0) << " T1=" << B.in_T1(f0)
         << " S=" << B.is_self_dual(f0, 2)
         << " M=" << B.is_monotone(f0, 2)
         << " L=" << B.is_linear(f0, 2) << " (ожидаем 1 0 0 1 1)" << endl;
    cout << "x0∨x1: T0=" << B.in_T0(f_or) << " T1=" << B.in_T1(f_or)
         << " S=" << B.is_self_dual(f_or, 2)
         << " M=" << B.is_monotone(f_or, 2)
         << " L=" << B.is_linear(f_or, 2) << " (ожидаем 1 1 0 1 0)" << endl;
    vector<int> f_xor = {0, 1, 1, 0}; // x0⊕x1: T0, L
    cout << "x0⊕x1: T0=" << B.in_T0(f_xor) << " T1=" << B.in_T1(f_xor)
         << " S=" << B.is_self_dual(f_xor, 2)
         << " M=" << B.is_monotone(f_xor, 2)
         << " L=" << B.is_linear(f_xor, 2) << " (ожидаем 1 0 0 0 1)" << endl;

    cout << "\n=== A. Полнота (критерий Поста) ===" << endl;
    vector<int> f_and = {0, 0, 0, 1};   // x0∧x1
    vector<int> f_not = {1, 0, 1, 0};   // ¬x0 (x1 фиктивна)
    vector<int> f_nor = {1, 0, 0, 0};   // стрелка Пирса ↓
    cout << "{∧, ∨} полна: " << B.is_complete_system({f_and, f_or}, 2)
         << " (ожидаем 0)" << endl;
    cout << "{¬, ∧} полна: " << B.is_complete_system({f_not, f_and}, 2)
         << " (ожидаем 1)" << endl;
    cout << "{↓} полна: " << B.is_complete_system({f_nor}, 2)
         << " (ожидаем 1)" << endl;
    cout << "Таблица Поста {¬, ∧}:" << endl;
    for (auto& row : B.post_table({f_not, f_and}, 2)) {
        for (int v : row) cout << v << " ";
        cout << endl;
    }

    cout << "\n=== B. Вентили ===" << endl;
    cout << "AND(1,1)=" << B.gate_apply("AND", 1, 1)
         << " NOR(1,1)=" << B.gate_apply("NOR", 1, 1)
         << " XOR(1,0)=" << B.gate_apply("XOR", 1, 0)
         << " IMPLY(1,0)=" << B.gate_apply("IMPLY", 1, 0)
         << " NIMPLY(1,0)=" << B.gate_apply("NIMPLY", 1, 0)
         << " NOT(0)=" << B.gate_apply("NOT", 0, 0) << endl;
    cout << "Таблицы вентилей:" << endl;
    vector<string> gates = {"AND", "OR", "NOT", "NAND", "NOR",
                            "XOR", "XNOR", "IMPLY", "NIMPLY"};
    for (auto& g : gates) {
        cout << g << ": ";
        for (int v : B.gate_table(g)) cout << v << " ";
        cout << endl;
    }

    cout << "\n=== B. Мультиплексор ===" << endl;
    vector<int> inputs = {1, 0, 1, 1};
    cout << "mux(inputs, 0)=" << B.multiplexer(inputs, 0)
         << " mux(inputs, 2)=" << B.multiplexer(inputs, 2) << endl;
    auto dm = B.demultiplexer(2, 1, 2);
    cout << "demux(sel=2): ";
    for (int v : dm) cout << v << " ";
    cout << "(ожидаем 0 0 1 0)" << endl;

    cout << "\n=== B. Карты Карно ===" << endl;
    cout << "Карта x0∨x1 (2 переменных):" << endl;
    B.karnaugh_print(f_or, 2);
    cout << "Карта x2 (3 переменных, 2×4):" << endl;
    vector<int> f_x2 = {0, 0, 0, 0, 1, 1, 0, 0}; // бит 2
    B.karnaugh_print(f_x2, 3);

    cout << "\n=== B. Метод Квайна-МакКласки ===" << endl;
    vector<string> primes = B.quine_mccluskey(f_or, 2);
    cout << "Простые импликанты x0∨x1: ";
    for (auto& p : primes) cout << p << " ";
    cout << "(ожидаем -1 1-)" << endl;
    auto cover = B.minimal_cover(f_or, 2, primes);
    cout << "Минимальное покрытие: ";
    for (int i : cover) cout << B.implicant_to_string(primes[i], 2) << " ";
    cout << "(ожидаем x0 x1)" << endl;
    vector<int> f_or3 = {0, 1, 1, 1, 0, 1, 1, 1}; // x0∨x1 от 3 переменных
    vector<string> primes3 = B.quine_mccluskey(f_or3, 3);
    cout << "Простые импликанты x0∨x1 (3 переменные): ";
    for (auto& p : primes3) cout << p << " ";
    cout << "(ожидаем --1 -1-)" << endl;
    auto cover3 = B.minimal_cover(f_or3, 3, primes3);
    cout << "Минимальное покрытие: ";
    for (int i : cover3) cout << B.implicant_to_string(primes3[i], 3) << " ";
    cout << "(ожидаем x0 x1)" << endl;
}
#endif // BOOLEAN_ALGEBRA_MAIN
#endif // DISCRETE_LOGIC_C_CPP
