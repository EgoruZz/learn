#ifndef DISCRETE_LOGIC_B_CPP
#define DISCRETE_LOGIC_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <map>
#include <tuple>
#include <functional>
#include <random>
#include <cmath>
using namespace std;

#define SETS_RELATIONS_MAIN
#include "../a-folder/a.cpp"

// =============================================================
// B. ЛОГИКА И ФОРМАЛЬНЫЕ СИСТЕМЫ
// =============================================================
// Структура md: A. Логика высказываний (1. Формальная логика,
//               2. Формализация, 3. Язык, 4. Логическое значение,
//               5. Алгебра высказываний, 6. Логическое следование)
//               → B. Логика предикатов (1. Предикат, 2. Множество
//               истинности, 3. Кванторы, 4. Формулы, 5. Законы,
//               6. Приведённая форма, 7. ПНФ) → C. ФАТ (1. Понятие,
//               2. ФИВ, 3. Вывод, 4. Теорема о дедукции, 5. Свойства
//               ФИВ, 6. ФИП, 7. ФА, 8. Гёдель, 9. Вычислимость:
//               машина Тьюринга, λ-исчисление Чёрча, рекурсивные
//               функции Клина, тезис Чёрча и неразрешимость)
//               → D. Нечёткая логика (1. Нечёткие множества и
//               α-срезы, 2. Операции, 3. Нечёткая алгебраическая
//               система (Fuzzy Operations), 4. t-нормы и t-конормы,
//               5. Нечёткие отношения и композиция)
//
// Наследует SetsAndRelations (a.cpp): множества, функции и соответствия
// используются в языке логики (предикат — характеристическая функция
// отношения, a.md B.8). Все методы обобщены: констант в алгоритмах нет,
// кроме границ представления (переменные 0..25, n ≤ 20 для таблиц,
// лимиты шагов/поиска и сетка уровней — параметры).
//
// Соглашение о представлении:
//   * формула — бинарное дерево Form: op — код операции:
//     'v' — переменная (var), 'n' — ¬ (l), 'a' — ∧, 'o' — ∨, 'i' — →,
//     'e' — ↔ (l, r), 'A' — ∀x (var, l), 'E' — ∃x (var, l);
//   * оценка переменных — vector<int> val размера n: val[x] ∈ {0, 1};
//   * предикат P: A₁×...×Aₖ → {0,1} — вектор int (0/1) размера ∏dimᵢ
//     (таблица истинности), dims[i] — мощность домена i-й координаты,
//     индекс кортежа — позиционный код (a.md B.1);
//   * вывод ФИВ/ФИП — вектор указателей на формулы, гипотезы —
//     vector<bool> по позициям вывода;
//   * нумерация Гёделя — формула в ASCII-записи над алфавитом ALPHABET;
//   * машина Тьюринга — программа TuringProgram: (состояние, символ) →
//     (новое состояние, символ, сдвиг), лента — строка из '0'/'1'
//     с пробелами, остановка — отсутствие перехода;
//   * числа Чёрча — функции: n — «итератор» (применяет f n раз);
//   * рекурсивные функции — std::function над вектором аргументов;
//   * нечёткое множество — вектор double (универсум — индексы),
//     отношение — матрица double; t-нормы/конормы — по имени.
//
// Содержит:
//   A. Высказывания: form_var, form_not, form_and, form_or, form_imp,
//      form_iff, form_forall, form_exists, eval_form, truth_table,
//      is_tautology, is_contradiction, is_satisfiable, forms_equivalent,
//      substitute, dual_form, eliminate_imp_iff, logical_consequence
//   B. Предикаты: pred_size, pred_index, pred_decode, pred_truth_set,
//      pred_not, pred_and, pred_or, pred_imp, pred_quantify,
//      preds_equivalent, free_vars, all_vars, substitute_free,
//      push_negations, reduced_form, to_pnf, extract_quantifiers,
//      move_quantifiers_left, is_quantifier_free, is_pnf
//   C. ФАТ: form_to_string, forms_equal, is_axiom_scheme,
//      verify_derivation, deduce_self_imp, deduction_transform,
//      soundness_check, is_axiom_scheme_pred, is_generalization,
//      verify_pred_derivation, pa_axioms, godel_number, godel_decode,
//      sieve_primes; вычислимость: TuringProgram/TuringConfig/
//      TuringResult, turing_step, turing_run, ChurchFunc/ChurchNumeral,
//      church_numeral, church_succ, church_add, church_mul,
//      church_to_int, church_apply, IntFunc/RecFunc, basis_zero,
//      basis_successor, basis_projection, substitution,
//      primitive_recursion, mu_minimize
//   D. Нечёткая логика: FuzzySet, fuzzy_support, fuzzy_core,
//      alpha_cut, fuzzy_decompose, fuzzy_complement,
//      fuzzy_intersection, fuzzy_union, fuzzy_subset, FuzzyOp,
//      fuzzy_binary_op, t_norm, t_conorm, check_t_norm,
//      check_t_conorm, fuzzy_intersection_t, fuzzy_union_t,
//      FuzzyRelation, fuzzy_relation_compose,
//      fuzzy_transitive_closure

// --- A.3.1. Формула как бинарное дерево ---
// Узел-операция с левым и правым поддеревьями; лист ('v') — переменная.
// Кванторы ('A', 'E') хранят связанную переменную в var и формулу в l.
struct Form {
    char op;
    int var;
    Form* l;
    Form* r;
    Form(char op, int var, Form* l, Form* r) : op(op), var(var), l(l), r(r) {}
};

struct FormalLogic : SetsAndRelations {

// =============================================================
// A. ЛОГИКА ВЫСКАЗЫВАНИЙ
// =============================================================

// --- A.3.1. Конструкторы формул ---
// Индуктивное определение формулы (a.md A.3.1): база — переменная,
// шаги — применение связок и кванторов.
Form* form_var(int x) {
    return new Form('v', x, nullptr, nullptr);
}

Form* form_not(Form* a) {
    return new Form('n', 0, a, nullptr);
}

Form* form_and(Form* a, Form* b) {
    return new Form('a', 0, a, b);
}

Form* form_or(Form* a, Form* b) {
    return new Form('o', 0, a, b);
}

Form* form_imp(Form* a, Form* b) {
    return new Form('i', 0, a, b);
}

Form* form_iff(Form* a, Form* b) {
    return new Form('e', 0, a, b);
}

// ∀x F: связанная переменная x в var, формула в l.
Form* form_forall(int x, Form* a) {
    return new Form('A', x, a, nullptr);
}

// ∃x F: связанная переменная x в var, формула в l.
Form* form_exists(int x, Form* a) {
    return new Form('E', x, a, nullptr);
}

// --- A.4.2. Вычисление логического значения формулы (теорема о вычислении) ---
// Теорема: для любой формулы и оценки существует ровно одно значение.
// Доказательство — индукция по построению (база: переменная задана
// оценкой; шаги: связки — функции истинности), что и реализует обход
// дерева. O(|f|).
int eval_form(Form* f, const vector<int>& val) {
    if (f->op == 'v') return val[f->var];
    if (f->op == 'n') return !eval_form(f->l, val);
    int a = eval_form(f->l, val);
    int b = eval_form(f->r, val);
    if (f->op == 'a') return a && b;
    if (f->op == 'o') return a || b;
    if (f->op == 'i') return !a || b;
    return a == b; // 'e': эквивалентность
}

// --- A.4.3. Таблица истинности формулы от n переменных ---
// Перебор всех 2ⁿ оценок бинарным счётчиком; res[m] — значение формулы
// на оценке m (бит x числа m — значение переменной x). O(2ⁿ · |f|).
vector<int> truth_table(Form* f, int n) {
    vector<int> res(1 << n);
    for (int m = 0; m < (1 << n); m++) {
        vector<int> val(n);
        for (int x = 0; x < n; x++) val[x] = (m >> x) & 1;
        res[m] = eval_form(f, val);
    }
    return res;
}

// --- A.4.3. Классификация формул по таблице истинности ---
// Тавтология: значение 1 на всех 2ⁿ оценках.
bool is_tautology(Form* f, int n) {
    for (int m = 0; m < (1 << n); m++) {
        vector<int> val(n);
        for (int x = 0; x < n; x++) val[x] = (m >> x) & 1;
        if (!eval_form(f, val)) return false;
    }
    return true;
}

// Противоречие: значение 0 на всех оценках.
bool is_contradiction(Form* f, int n) {
    for (int m = 0; m < (1 << n); m++) {
        vector<int> val(n);
        for (int x = 0; x < n; x++) val[x] = (m >> x) & 1;
        if (eval_form(f, val)) return false;
    }
    return true;
}

// Выполнимая: существует оценка со значением 1 (не противоречие).
bool is_satisfiable(Form* f, int n) {
    return !is_contradiction(f, n);
}

// --- A.5. Эквивалентность формул: совпадение таблиц истинности ---
bool forms_equivalent(Form* a, Form* b, int n) {
    for (int m = 0; m < (1 << n); m++) {
        vector<int> val(n);
        for (int x = 0; x < n; x++) val[x] = (m >> x) & 1;
        if (eval_form(a, val) != eval_form(b, val)) return false;
    }
    return true;
}

// --- A.5. Подстановка формулы g вместо переменной x ---
// Замена всех вхождений x на формулу g. Если A ≡ B, то и после
// подстановки A[g/x] ≡ B[g/x] (теорема о замене) — метод упрощения.
Form* substitute(Form* f, int x, Form* g) {
    if (f->op == 'v') return (f->var == x) ? g : f;
    if (f->op == 'n') return form_not(substitute(f->l, x, g));
    return new Form(f->op, f->var, substitute(f->l, x, g),
                    substitute(f->r, x, g));
}

// --- A.5. Двойственная формула: ∧ ↔ ∨ ---
// Принцип двойственности: замена ∧ ↔ ∨ переводит тождества в тождества.
Form* dual_form(Form* f) {
    if (f->op == 'v') return f;
    if (f->op == 'n') return form_not(dual_form(f->l));
    char op = (f->op == 'a') ? 'o' : (f->op == 'o') ? 'a' : f->op;
    return new Form(op, 0, dual_form(f->l), dual_form(f->r));
}

// --- A.2. Устранение → и ↔ (базис {¬, ∨}) ---
// Функциональная полнота: A→B ≡ ¬A∨B; A↔B ≡ (¬A∨B)∧(A∨¬B).
// Строит эквивалентную формулу без → и ↔.
Form* eliminate_imp_iff(Form* f) {
    if (f->op == 'v' || f->op == 'A' || f->op == 'E') return f;
    if (f->op == 'n') return form_not(eliminate_imp_iff(f->l));
    Form* a = eliminate_imp_iff(f->l);
    Form* b = eliminate_imp_iff(f->r);
    if (f->op == 'i') return form_or(form_not(a), b);
    if (f->op == 'e') return form_and(form_or(form_not(a), b), form_or(a, form_not(b)));
    return new Form(f->op, 0, a, b); // 'a' / 'o'
}

// --- A.6. Логическое следование Γ ⊨ α ---
// α следует из Γ, если на всякой оценке, где истинны все формулы Γ,
// истинна и α. Эквивалентно: тавтология (∧Γ) → α (семантический
// аналог теоремы о дедукции, C.4). O(2ⁿ · (|Γ| + |α|)).
bool logical_consequence(const vector<Form*>& premises, Form* concl, int n) {
    for (int m = 0; m < (1 << n); m++) {
        vector<int> val(n);
        for (int x = 0; x < n; x++) val[x] = (m >> x) & 1;
        bool ok = true;
        for (Form* p : premises)
            if (!eval_form(p, val)) { ok = false; break; }
        if (ok && !eval_form(concl, val)) return false;
    }
    return true;
}

// =============================================================
// B. ЛОГИКА ПРЕДИКАТОВ
// =============================================================

// --- B.1.2. Размер области определения предиката: ∏ dims[i] ---
int pred_size(const vector<int>& dims) {
    int res = 1;
    for (int d : dims) res *= d;
    return res;
}

// --- B.1.2. Индекс кортежа в таблице (позиционный код) ---
// idx = Σ tup[i] · ∏_{j>i} dims[j] — тот же код, что в декартовом
// произведении (a.md B.1): старшая координата — первая.
int pred_index(const vector<int>& dims, const vector<int>& tup) {
    int res = 0, mul = 1;
    for (int i = (int)dims.size() - 1; i >= 0; i--) {
        res += tup[i] * mul;
        mul *= dims[i];
    }
    return res;
}

// --- B.1.2. Кортеж по индексу (обратно к pred_index) ---
vector<int> pred_decode(const vector<int>& dims, int idx) {
    vector<int> tup(dims.size());
    for (int i = (int)dims.size() - 1; i >= 0; i--) {
        tup[i] = idx % dims[i];
        idx /= dims[i];
    }
    return tup;
}

// --- B.2. Множество истинности предиката ---
// IP = {(x₁,...,xₖ) : P(x₁,...,xₖ) = 1} ⊆ A₁×...×Aₖ; возвращает кортежи
// (декодированные значения координат), на которых таблица равна 1.
vector<vector<int>> pred_truth_set(const vector<int>& dims, const vector<int>& tab) {
    vector<vector<int>> res;
    for (int i = 0; i < (int)tab.size(); i++)
        if (tab[i]) res.push_back(pred_decode(dims, i));
    return res;
}

// --- B.2. Операции над предикатами (одинаковые области определения) ---
// Истинность ∧/∨/¬ на кортеже — поэлементные операции над таблицами;
// множествам истинности соответствуют пересечение/объединение/дополнение
// (a.md A.2).
vector<int> pred_not(const vector<int>& tab) {
    vector<int> res(tab.size());
    for (int i = 0; i < (int)tab.size(); i++) res[i] = !tab[i];
    return res;
}

vector<int> pred_and(const vector<int>& a, const vector<int>& b) {
    vector<int> res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = a[i] && b[i];
    return res;
}

vector<int> pred_or(const vector<int>& a, const vector<int>& b) {
    vector<int> res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = a[i] || b[i];
    return res;
}

// P→Q ≡ ¬P∨Q на таблицах.
vector<int> pred_imp(const vector<int>& a, const vector<int>& b) {
    vector<int> res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = !a[i] || b[i];
    return res;
}

// --- B.3.2. Кванторные операции над предикатами ---
// ∀ по координате c: новый предикат от остальных координат, значение 1,
// если P равен 1 на ВСЕХ значениях координаты c при фиксированных
// остальных; ∃ — если хотя бы на одном. Результат — таблица меньшей
// арности (без координаты c). O(k · ∏dimᵢ): декодирование кортежа.
vector<int> pred_quantify(const vector<int>& dims, int c, bool is_forall,
                          const vector<int>& tab) {
    vector<int> ndims;
    for (int i = 0; i < (int)dims.size(); i++)
        if (i != c) ndims.push_back(dims[i]);
    vector<int> res(pred_size(ndims), is_forall ? 1 : 0);
    for (int i = 0; i < (int)tab.size(); i++) {
        vector<int> tup = pred_decode(dims, i);
        vector<int> ntup;
        for (int j = 0; j < (int)tup.size(); j++)
            if (j != c) ntup.push_back(tup[j]);
        int ni = pred_index(ndims, ntup);
        if (is_forall) { if (!tab[i]) res[ni] = 0; }
        else if (tab[i]) res[ni] = 1;
    }
    return res;
}

// --- B.2. Эквивалентность предикатов (одинаковые области) ---
bool preds_equivalent(const vector<int>& a, const vector<int>& b) {
    return a == b;
}

// --- B.4.3. Свободные переменные формулы ---
// Переменная свободна, если не входит в область действия квантора по ней.
// Возвращает маску: бит x = 1 ⟺ x свободна. O(|f|).
int free_vars(Form* f) {
    if (f->op == 'v') return 1 << f->var;
    if (f->op == 'n') return free_vars(f->l);
    if (f->op == 'A' || f->op == 'E')
        return free_vars(f->l) & ~(1 << f->var);
    return free_vars(f->l) | free_vars(f->r);
}

// --- B.4.3. Все переменные формулы (свободные и связанные) ---
// Для перебора термов-переменных в аксиомах ФИП (C.6).
vector<int> all_vars(Form* f) {
    if (f->op == 'v') return {f->var};
    if (f->op == 'n') return all_vars(f->l);
    if (f->op == 'A' || f->op == 'E') {
        vector<int> v = all_vars(f->l);
        v.push_back(f->var);
        return v;
    }
    vector<int> a = all_vars(f->l), b = all_vars(f->r);
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// --- B.4.4. Подстановка переменной t вместо свободных x ---
// Заменяет свободные вхождения x на t. Избегание захвата: если квантор
// связывает t, связанная переменная переименовывается в свежую (счётчик
// fresh) — подстановка не «портит» смысл формулы (в отличие от наивной).
Form* substitute_free(Form* f, int x, int t, int& fresh) {
    if (f->op == 'v') return (f->var == x) ? form_var(t) : f;
    if (f->op == 'n') return form_not(substitute_free(f->l, x, t, fresh));
    if (f->op == 'A' || f->op == 'E') {
        if (f->var == x) return f; // в области действия x связаны
        if (f->var == t) {         // захват: переименовать связанную
            int y = fresh++;
            Form* body = substitute_free(f->l, f->var, y, fresh);
            return (f->op == 'A') ? form_forall(y, body) : form_exists(y, body);
        }
        Form* body = substitute_free(f->l, x, t, fresh);
        return (f->op == 'A') ? form_forall(f->var, body) : form_exists(f->var, body);
    }
    return new Form(f->op, 0, substitute_free(f->l, x, t, fresh),
                    substitute_free(f->r, x, t, fresh));
}

// --- B.6. Перенос отрицаний вглубь (приведённая форма, шаг 2) ---
// ¬¬A ≡ A; де Морган: ¬(A∧B) ≡ ¬A∨¬B, ¬(A∨B) ≡ ¬A∧¬B;
// кванторы: ¬∀xA ≡ ∃x¬A, ¬∃xA ≡ ∀x¬A.
// Предполагается, что → и ↔ устранены (eliminate_imp_iff).
Form* push_negations(Form* f) {
    if (f->op == 'v') return f;
    if (f->op == 'n') {
        Form* a = f->l;
        if (a->op == 'v') return f;              // ¬p — уже атомарно
        if (a->op == 'n') return push_negations(a->l); // ¬¬A ≡ A
        if (a->op == 'a')
            return form_or(push_negations(form_not(a->l)),
                           push_negations(form_not(a->r)));
        if (a->op == 'o')
            return form_and(push_negations(form_not(a->l)),
                            push_negations(form_not(a->r)));
        if (a->op == 'A')
            return form_exists(a->var, push_negations(form_not(a->l)));
        if (a->op == 'E')
            return form_forall(a->var, push_negations(form_not(a->l)));
        return f;
    }
    if (f->op == 'A' || f->op == 'E')
        return new Form(f->op, f->var, push_negations(f->l), nullptr);
    return new Form(f->op, f->var, push_negations(f->l), push_negations(f->r));
}

// --- B.6. Приведённая форма ---
// Связки только ¬, ∧, ∨; отрицания — только перед атомарными формулами.
// Алгоритм (теорема о приведённой форме): устранить → и ↔, затем
// перенести отрицания вглубь. Каждый шаг — тождество, результат
// эквивалентен исходной формуле.
Form* reduced_form(Form* f) {
    return push_negations(eliminate_imp_iff(f));
}

// --- B.7. ПНФ: вынос кванторов из q перед (q op other) ---
// Если x квантора свободна в other — переименовываем x в свежую
// (альфа-эквивалентность), затем переносим: Qx (F op G) ≡ Qx(F) op G
// при условии, что x не свободна в G (законы B.5).
Form* move_quantifiers_left(Form* q, char op, Form* other, int& fresh) {
    if (q->op != 'A' && q->op != 'E') {
        if (other->op != 'A' && other->op != 'E')
            return (op == 'a') ? form_and(q, other) : form_or(q, other);
        return move_quantifiers_left(other, op, q, fresh);
    }
    Form* body = q->l;
    int x = q->var;
    if (free_vars(other) & (1 << x)) {
        int y = fresh++;
        body = substitute_free(body, x, y, fresh);
        x = y;
    }
    Form* merged = move_quantifiers_left(body, op, other, fresh);
    return (q->op == 'A') ? form_forall(x, merged) : form_exists(x, merged);
}

// --- B.7. ПНФ: вынос кванторов из подформул ---
// Рекурсивно: кванторы каждой части выносятся перед связкой
// (move_quantifiers_left) — порядок левой части сохраняется.
Form* extract_quantifiers(Form* f, int& fresh) {
    if (f->op == 'v') return f;
    if (f->op == 'n') return form_not(extract_quantifiers(f->l, fresh));
    if (f->op == 'A' || f->op == 'E')
        return (f->op == 'A') ? form_forall(f->var, extract_quantifiers(f->l, fresh))
                              : form_exists(f->var, extract_quantifiers(f->l, fresh));
    Form* a = extract_quantifiers(f->l, fresh);
    Form* b = extract_quantifiers(f->r, fresh);
    return move_quantifiers_left(a, f->op, b, fresh);
}

// --- B.7. Предваренная нормальная форма ---
// Q₁x₁...Qₖxₖ F, F бескванторна (теорема о ПНФ). Алгоритм: 1) устранить
// →, ↔; 2) перенести отрицания (приведённая форма, B.6); 3) вынести
// кванторы влево. Переименование связанных переменных — в свежие.
Form* to_pnf(Form* f, int& fresh) {
    return extract_quantifiers(push_negations(eliminate_imp_iff(f)), fresh);
}

// --- B.7. Бескванторная формула ---
bool is_quantifier_free(Form* f) {
    if (f->op == 'A' || f->op == 'E') return false;
    if (f->op == 'v') return true;
    if (f->op == 'n') return is_quantifier_free(f->l);
    return is_quantifier_free(f->l) && is_quantifier_free(f->r);
}

// --- B.7. Предваренная форма: цепочка кванторов над бескванторной ---
bool is_pnf(Form* f) {
    if (f->op == 'A' || f->op == 'E') return is_pnf(f->l);
    return is_quantifier_free(f);
}

// =============================================================
// C. ФОРМАЛЬНЫЕ АКСИОМАТИЧЕСКИЕ ТЕОРИИ (ФАТ)
// =============================================================

// --- C.2. Инфиксная запись формулы ---
// Полная скобочная форма (без сокращения скобок) — однозначное чтение.
string form_to_string(Form* f) {
    if (f->op == 'v') return "x" + to_string(f->var);
    if (f->op == 'n') return "¬" + form_to_string(f->l);
    if (f->op == 'A') return "∀x" + to_string(f->var) + "(" + form_to_string(f->l) + ")";
    if (f->op == 'E') return "∃x" + to_string(f->var) + "(" + form_to_string(f->l) + ")";
    string s = "(" + form_to_string(f->l);
    s += (f->op == 'a') ? "∧" : (f->op == 'o') ? "∨" : (f->op == 'i') ? "→" : "↔";
    s += form_to_string(f->r) + ")";
    return s;
}

// --- C.2. Равенство формул (структурное) ---
// Сравнение деревьев: совпадение операций, переменных и поддеревьев.
bool forms_equal(Form* a, Form* b) {
    if (a->op != b->op) return false;
    if (a->op == 'v') return a->var == b->var;
    if (a->op == 'n') return forms_equal(a->l, b->l);
    if (a->op == 'A' || a->op == 'E')
        return a->var == b->var && forms_equal(a->l, b->l);
    return forms_equal(a->l, b->l) && forms_equal(a->r, b->r);
}

// --- C.2. Схемы аксиом ФИВ ---
// A1: A→(B→A); A2: (A→(B→C))→((A→B)→(A→C)); A3: (¬B→¬A)→(A→B).
// Схема — шаблон: подстановка любых формул в A, B, C даёт аксиому.
// Проверка — согласованное сопоставление поддеревьев формулы.
bool is_axiom_scheme(Form* f) {
    if (f->op != 'i') return false;
    // A1: A→(B→A): правая часть — импликация B→A с той же A, что слева
    if (f->r->op == 'i' && forms_equal(f->l, f->r->r)) return true;
    // A2: (A→(B→C))→((A→B)→(A→C))
    if (f->l->op == 'i' && f->r->op == 'i' && f->r->r->op == 'i') {
        Form* A = f->l->l;
        if (f->l->r->op != 'i') return false;
        Form* B = f->l->r->l;
        Form* C = f->l->r->r;
        if (forms_equal(A, f->r->l->l) && forms_equal(A, f->r->r->l)
            && forms_equal(B, f->r->l->r) && forms_equal(C, f->r->r->r))
            return true;
    }
    // A3: (¬B→¬A)→(A→B)
    if (f->l->op == 'i' && f->l->l->op == 'n' && f->l->r->op == 'n'
        && f->r->op == 'i') {
        if (forms_equal(f->l->l->l, f->r->r) && forms_equal(f->l->r->l, f->r->l))
            return true;
    }
    return false;
}

// --- C.3. Проверка вывода (последовательности формул) ---
// Каждая формула вывода — гипотеза из Γ, аксиома или modus ponens из
// двух предыдущих (существуют j, k < i: der[k] = der[j] → der[i]).
// Проверка алгоритмична: O(k³) для вывода длины k.
bool verify_derivation(const vector<Form*>& der, const vector<bool>& hyp) {
    int n = (int)der.size();
    for (int i = 0; i < n; i++) {
        if (hyp[i] || is_axiom_scheme(der[i])) continue;
        bool ok = false;
        for (int j = 0; j < i && !ok; j++)
            for (int k = 0; k < i; k++)
                if (der[k]->op == 'i' && forms_equal(der[k]->l, der[j])
                    && forms_equal(der[k]->r, der[i])) { ok = true; break; }
        if (!ok) return false;
    }
    return true;
}

// --- C.4. Вывод A→A из аксиом ФИВ (без гипотез) ---
// 1. (A→((A→A)→A))→((A→(A→A))→(A→A))  [A2: A, B = A→A, C = A]
// 2. A→((A→A)→A)                        [A1]
// 3. (A→(A→A))→(A→A)                    [MP 1, 2]
// 4. A→(A→A)                            [A1]
// 5. A→A                                [MP 3, 4]
vector<Form*> deduce_self_imp(Form* A) {
    Form* X = form_imp(A, A);                        // A→A
    Form* step1 = form_imp(form_imp(A, form_imp(X, A)),   // A2
                           form_imp(form_imp(A, X), X));
    Form* step2 = form_imp(A, form_imp(X, A));            // A1
    Form* step3 = form_imp(form_imp(A, X), X);            // MP 1, 2
    Form* step4 = form_imp(A, X);                         // A1
    return {step1, step2, step3, step4, X};
}

// --- C.4. Теорема о дедукции (алгоритмическая версия) ---
// Γ, A ⊢ B ⇒ Γ ⊢ A→B. Каждая формула Cᵢ вывода заменяется выводом
// A→Cᵢ: 1) гипотеза Γ или аксиома: Cᵢ, Cᵢ→(A→Cᵢ) [A1], A→Cᵢ [MP];
// 2) Cᵢ = A: стандартный вывод A→A; 3) Cᵢ = MP(Cⱼ, Cₖ), Cₖ = Cⱼ→Cᵢ:
// аксиома A2 (A→(Cⱼ→Cᵢ))→((A→Cⱼ)→(A→Cᵢ)) и два MP.
// Возвращает новый вывод и метки гипотез (только Γ, без A).
pair<vector<Form*>, vector<bool>> deduction_transform(const vector<Form*>& der,
                                                      const vector<bool>& hyp,
                                                      Form* A) {
    vector<Form*> out;
    vector<bool> out_hyp;
    for (int i = 0; i < (int)der.size(); i++) {
        if (forms_equal(der[i], A)) {            // случай 2: шаг = A
            auto v = deduce_self_imp(A);
            for (Form* g : v) { out.push_back(g); out_hyp.push_back(false); }
            continue;
        }
        if (hyp[i] || is_axiom_scheme(der[i])) { // случай 1
            out.push_back(der[i]);
            out_hyp.push_back(hyp[i]);
            Form* ax = form_imp(der[i], form_imp(A, der[i])); // A1: C→(A→C)
            out.push_back(ax);
            out_hyp.push_back(false);
            out.push_back(form_imp(A, der[i]));               // MP(ax, C)
            out_hyp.push_back(false);
            continue;
        }
        // случай 3: modus ponens из двух предыдущих шагов
        int j = -1, k = -1;
        for (int a = 0; a < i && j == -1; a++)
            for (int b = 0; b < i; b++)
                if (der[b]->op == 'i' && forms_equal(der[b]->l, der[a])
                    && forms_equal(der[b]->r, der[i])) { j = a; k = b; break; }
        Form* ACj = form_imp(A, der[j]);
        Form* ACk = form_imp(A, der[k]);
        int p1 = -1, p2 = -1;
        for (int p = (int)out.size() - 1; p >= 0; p--) {
            if (p2 == -1 && forms_equal(out[p], ACk)) p2 = p;
            else if (p1 == -1 && forms_equal(out[p], ACj)) p1 = p;
        }
        out.push_back(form_imp(ACk, form_imp(ACj, form_imp(A, der[i])))); // A2
        out_hyp.push_back(false);
        out.push_back(form_imp(ACj, form_imp(A, der[i])));  // MP(A2, ACk)
        out_hyp.push_back(false);
        out.push_back(form_imp(A, der[i]));                 // MP(prev, ACj)
        out_hyp.push_back(false);
    }
    return {out, out_hyp};
}

// --- C.5. Корректность ФИВ: все выводимые формулы — тавтологии ---
// Аксиомы — тавтологии; MP сохраняет тавтологичность; значит, любая
// формула вывода — тавтология (проверяется по таблицам истинности).
bool soundness_check(const vector<Form*>& der, int n) {
    for (Form* f : der)
        if (!is_tautology(f, n)) return false;
    return true;
}

// --- C.6. Схемы аксиом ФИП ---
// Схемы ФИВ (C.2) плюс кванторные: ∀xA(x)→A(t) и A(t)→∃xA(x), где
// A(t) — подстановка терма t вместо x в A(x) (B.4.4). В языке термы —
// переменные; t перебирается по переменным формулы.
bool is_axiom_scheme_pred(Form* f) {
    if (is_axiom_scheme(f)) return true;
    if (f->op != 'i') return false;
    vector<int> vars = all_vars(f);
    // ∀xA(x) → A(t)
    if (f->l->op == 'A') {
        int x = f->l->var;
        Form* body = f->l->l;
        for (int t : vars) {
            int fresh = 100;
            if (forms_equal(f->r, substitute_free(body, x, t, fresh)))
                return true;
        }
        return false;
    }
    // A(t) → ∃xA(x)
    if (f->r->op == 'E') {
        int x = f->r->var;
        Form* body = f->r->l;
        for (int t : vars) {
            int fresh = 100;
            if (forms_equal(f->l, substitute_free(body, x, t, fresh)))
                return true;
        }
    }
    return false;
}

// --- C.6.2. Правило обобщения (Gen) ---
// der[i] = ∀x der[j] — обобщение по любой переменной (в чистом выводе
// условие «x не свободна в гипотезах» выполнено автоматически).
bool is_generalization(const vector<Form*>& der, int i, int j) {
    return der[i]->op == 'A' && forms_equal(der[i]->l, der[j]);
}

// --- C.6.2. Проверка вывода в ФИП ---
// Формулы — гипотезы, аксиомы ФИП (схемы ФИВ и кванторные) или
// результаты MP и Gen из предыдущих формул.
bool verify_pred_derivation(const vector<Form*>& der, const vector<bool>& hyp) {
    int n = (int)der.size();
    for (int i = 0; i < n; i++) {
        if (hyp[i] || is_axiom_scheme_pred(der[i])) continue;
        bool ok = false;
        for (int j = 0; j < i && !ok; j++) {
            if (is_generalization(der, i, j)) { ok = true; break; }
            for (int k = 0; k < i; k++)
                if (der[k]->op == 'i' && forms_equal(der[k]->l, der[j])
                    && forms_equal(der[k]->r, der[i])) { ok = true; break; }
        }
        if (!ok) return false;
    }
    return true;
}

// --- C.7. Аксиомы формальной арифметики (схемы Пеано) ---
// Язык: 0, S (следующий), +, ·, =. Семь схем (в коде — их записи
// строками: язык ФА богаче языка высказываний, формулы не строятся).
vector<string> pa_axioms() {
    return {
        "¬S(x)=0",
        "S(x)=S(y)→x=y",
        "x+0=x",
        "x+S(y)=S(x+y)",
        "x·0=0",
        "x·S(y)=(x·y)+x",
        "(F(0)∧∀x(F(x)→F(S(x))))→∀xF(x)"
    };
}

// --- C.8.1. Простые числа для нумерации Гёделя ---
// Решето Эратосфена: основания степеней числа формулы. Граница limit
// выбирается по длине формул: число формулы быстро растёт (см. ниже).
vector<long long> sieve_primes(long long limit) {
    vector<bool> comp(limit + 1, false);
    vector<long long> res;
    for (long long x = 2; x <= limit; x++) {
        if (!comp[x]) {
            res.push_back(x);
            for (long long y = x * x; y <= limit; y += x) comp[y] = true;
        }
    }
    return res;
}

// --- C.8.1. Нумерация Гёделя формулы ---
// Арифметизация синтаксиса: символы алфавита нумеруются кодами
// (позиция в ALPHABET + 1); формула s₁...sₖ кодируется произведением
// первых простых в степенях кодов: 2^{c₁}·3^{c₂}·...·pₖ^{cₖ}.
// Декодирование однозначно (основная теорема арифметики).
// Внимание: число растёт экспоненциально с длиной формулы — годятся
// короткие формулы, иначе нужна длинная арифметика.
const string ALPHABET = "()!&|>pqrstuvwxyz";

long long godel_number(const string& s, const vector<long long>& primes) {
    long long res = 1;
    for (int i = 0; i < (int)s.size(); i++) {
        int code = 0;
        while (code < (int)ALPHABET.size() && ALPHABET[code] != s[i]) code++;
        for (int e = 0; e < code + 1; e++) res *= primes[i];
    }
    return res;
}

// --- C.8.1. Декодирование числа Гёделя ---
// Обратное преобразование: делим на простые 2, 3, 5, ...; показатель
// степени — код символа. Проверка: godel_decode(godel_number(s)) = s.
string godel_decode(long long n, const vector<long long>& primes) {
    string res;
    for (int i = 0; n > 1; i++) {
        int e = 0;
        while (n % primes[i] == 0) { n /= primes[i]; e++; }
        res += ALPHABET[e - 1];
    }
    return res;
}

// =============================================================
// C.9. ВЫЧИСЛИМОСТЬ: МАШИНА ТЬЮРИНГА, ЧЁРЧ, КЛИНИ
// =============================================================

// --- C.9.2. Машина Тьюринга: представление ---
// Программа — частичная функция переходов (состояние, символ) →
// (новое состояние, символ, сдвиг −1/0/+1); отсутствие перехода —
// остановка. Конфигурация — (состояние, позиция, лента) (md 9.2).
using TuringProgram = map<pair<int, char>, tuple<int, char, int>>;
struct TuringConfig {
    int state;
    int pos;
    string tape;
};
struct TuringResult {
    bool halted;
    int steps;
    string tape;
    int state;
    int pos;
};

// --- C.9.2. Один шаг машины ---
// По паре (состояние, символ) — переход: записать символ, сдвинуть
// головку, сменить состояние; лента при выходе за края дополняется
// пробелом. Возвращает false, если перехода нет (машина встала).
bool turing_step(const TuringProgram& prog, TuringConfig& cfg) {
    auto it = prog.find({cfg.state, cfg.tape[cfg.pos]});
    if (it == prog.end()) return false;
    auto [st, ch, mv] = it->second;
    cfg.state = st;
    cfg.tape[cfg.pos] = ch;
    cfg.pos += mv;
    if (cfg.pos < 0) {
        cfg.tape = " " + cfg.tape;
        cfg.pos = 0;
    } else if (cfg.pos >= (int)cfg.tape.size()) {
        cfg.tape += " ";
    }
    return true;
}

// --- C.9.2. Выполнение машины с лимитом шагов ---
// md 9.5: остановка «за не более чем K шагов» разрешима — выполнить
// K шагов; без лимита выполнение может не завершиться.
TuringResult turing_run(const TuringProgram& prog, const string& tape,
                        int limit) {
    TuringConfig cfg{0, 0, tape};
    for (int s = 0; s < limit; s++) {
        if (!turing_step(prog, cfg))
            return {true, s, cfg.tape, cfg.state, cfg.pos};
    }
    return {false, limit, cfg.tape, cfg.state, cfg.pos};
}

// --- C.9.3. Лямбда-исчисление: типы ---
// Число Чёрча — функция над функциями (λf.λx.fⁿ(x)): принимает
// «функцию f» и возвращает «итератор» fⁿ (применение f к x n раз);
// применение (M N) в C++ — вызов функции функцией.
using ChurchFunc = function<int(int)>;
using ChurchNumeral = function<ChurchFunc(const ChurchFunc&)>;

// --- C.9.3. Числа Чёрча ---
// n := λf.λx. f(f(...(f x)...)) — применение f к x ровно n раз.
ChurchNumeral church_numeral(int n) {
    return [n](const ChurchFunc& f) {
        return ChurchFunc([f, n](int x) {
            for (int i = 0; i < n; i++) x = f(x);
            return x;
        });
    };
}

// --- C.9.3. SUCC := λn.λf.λx. f (n f x) — применить f ещё раз ---
ChurchNumeral church_succ(const ChurchNumeral& n) {
    return [n](const ChurchFunc& f) {
        return ChurchFunc([n, f](int x) { return f(n(f)(x)); });
    };
}

// --- C.9.3. ADD := λm.λn.λf.λx. m f (n f x) — n раз, потом m раз ---
ChurchNumeral church_add(const ChurchNumeral& m, const ChurchNumeral& n) {
    return [m, n](const ChurchFunc& f) {
        return ChurchFunc([m, n, f](int x) { return m(f)(n(f)(x)); });
    };
}

// --- C.9.3. MUL := λm.λn.λf. m (n f) — n-кратный итератор, m раз ---
ChurchNumeral church_mul(const ChurchNumeral& m, const ChurchNumeral& n) {
    return [m, n](const ChurchFunc& f) {
        return ChurchFunc([m, n, f](int x) { return m(n(f))(x); });
    };
}

// --- C.9.3. Число Чёрча в int ---
// Применить терм к «счётчику»: f(x) = x + 1, начиная с 0 — получаем n.
int church_to_int(const ChurchNumeral& t) {
    ChurchFunc f = [](int x) { return x + 1; };
    return t(f)(0);
}

// --- C.9.3. Применение терма к функции и аргументу ---
// Модель β-редукции «(λx.M) N ⟶ M[x := N]»: применение терма-итератора
// к конкретной функции и начальному значению.
int church_apply(const ChurchNumeral& t, const ChurchFunc& f, int x) {
    return t(f)(x);
}

// --- C.9.4. Рекурсивные функции: типы и базис ---
// IntFunc — функция произвольной арности (аргументы вектором);
// RecFunc — вспомогательная функция h(x, y, f(x,y)) примитивной
// рекурсии. Базис: O(x) = 0, S(x) = x + 1, Iᵏᵢ(x) = xᵢ — «аксиомы»
// аксиоматической теории вычислимых функций (md 9.4).
using IntFunc = function<int(const vector<int>&)>;
using RecFunc = function<int(const vector<int>&, int, int)>;

IntFunc basis_zero() {
    return [](const vector<int>&) { return 0; };
}

IntFunc basis_successor() {
    return [](const vector<int>& x) { return x[0] + 1; };
}

IntFunc basis_projection(int k, int i) {
    (void)k;
    return [i](const vector<int>& x) { return x[i]; };
}

// --- C.9.4. Подстановка (суперпозиция) ---
// h(f₁(x), ..., fₘ(x)) — из h и m функций строится новая функция.
IntFunc substitution(const IntFunc& h, const vector<IntFunc>& fs) {
    return [h, fs](const vector<int>& x) {
        vector<int> args;
        for (auto& f : fs) args.push_back(f(x));
        return h(args);
    };
}

// --- C.9.4. Примитивная рекурсия ---
// f(x, 0) = g(x); f(x, y + 1) = h(x, y, f(x, y)) — итерация по y
// от 0 до y; завершается всегда (в отличие от минимизации).
IntFunc primitive_recursion(const IntFunc& g, const RecFunc& h) {
    return [g, h](const vector<int>& args) {
        vector<int> x(args.begin(), args.end() - 1);
        int y = args.back();
        int cur = g(x);
        for (int i = 0; i < y; i++) cur = h(x, i, cur);
        return cur;
    };
}

// --- C.9.4. Минимизация (μ-оператор) ---
// f(x) = μy[g(x, y) = 0] — наименьшее y с g(x, y) = 0; поиск
// ограничен (md 9.5: неограниченный поиск не алгоритмичен),
// −1 — значение не найдено (функция на x не определена).
IntFunc mu_minimize(const IntFunc& g, int limit) {
    return [g, limit](const vector<int>& x) {
        for (int y = 0; y <= limit; y++) {
            vector<int> xy(x);
            xy.push_back(y);
            if (g(xy) == 0) return y;
        }
        return -1;
    };
}

// =============================================================
// D. НЕЧЁТКАЯ ЛОГИКА
// =============================================================

// --- D.1.1. Нечёткое множество ---
// Функция принадлежности μ: U → [0, 1]; универсум — индексы вектора,
// значение в позиции i — μ(i) (характеристическая функция множеств
// a.cpp, «отпущенная» в [0, 1]).
using FuzzySet = vector<double>;

// --- D.1.2. Носитель: элементы с ненулевой степенью ---
vector<int> fuzzy_support(const FuzzySet& a) {
    vector<int> res;
    for (int i = 0; i < (int)a.size(); i++)
        if (a[i] > 1e-12) res.push_back(i);
    return res;
}

// --- D.1.2. Ядро: элементы со степенью 1 ---
vector<int> fuzzy_core(const FuzzySet& a) {
    vector<int> res;
    for (int i = 0; i < (int)a.size(); i++)
        if (fabs(a[i] - 1.0) < 1e-12) res.push_back(i);
    return res;
}

// --- D.1.2. α-срез: элементы со степенью ≥ α ---
vector<int> alpha_cut(const FuzzySet& a, double alpha) {
    vector<int> res;
    for (int i = 0; i < (int)a.size(); i++)
        if (a[i] >= alpha - 1e-12) res.push_back(i);
    return res;
}

// --- D.1.2. Декомпозиция: восстановление по α-срезам ---
// Теорема о декомпозиции: μ(x) = max{α : x ∈ A_α}; cuts[k] — срез
// уровня k/steps (k = 0..steps), результат — поточечный максимум
// уровней по всем срезам, содержащим элемент.
FuzzySet fuzzy_decompose(const vector<vector<int>>& cuts, int steps) {
    FuzzySet res(cuts[0].size(), 0.0);
    for (int k = 0; k <= steps; k++) {
        double level = (double)k / steps;
        for (int i : cuts[k]) res[i] = max(res[i], level);
    }
    return res;
}

// --- D.2. Дополнение: μ_¬A(x) = 1 − μ_A(x) ---
FuzzySet fuzzy_complement(const FuzzySet& a) {
    FuzzySet res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = 1.0 - a[i];
    return res;
}

// --- D.2. Пересечение: min степеней ---
FuzzySet fuzzy_intersection(const FuzzySet& a, const FuzzySet& b) {
    FuzzySet res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = min(a[i], b[i]);
    return res;
}

// --- D.2. Объединение: max степеней ---
FuzzySet fuzzy_union(const FuzzySet& a, const FuzzySet& b) {
    FuzzySet res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = max(a[i], b[i]);
    return res;
}

// --- D.2. Включение: A ⊆ B ⟺ μ_A ≤ μ_B поточечно ---
bool fuzzy_subset(const FuzzySet& a, const FuzzySet& b) {
    for (int i = 0; i < (int)a.size(); i++)
        if (a[i] > b[i] + 1e-12) return false;
    return true;
}

// --- D.3.1. Нечёткая алгебраическая система: поточечная операция ---
// Операция над степенями [0, 1]² → [0, 1] «поднимается» на функции
// принадлежности: (A op B)(x) = op(μ_A(x), μ_B(x)) — произвольная
// бинарная операция задаёт свою нечёткую алгебраическую систему
// (стандартная — min/max, D.2; t-нормы/конормы — D.4).
using FuzzyOp = function<double(double, double)>;

FuzzySet fuzzy_binary_op(const FuzzySet& a, const FuzzySet& b,
                         const FuzzyOp& op) {
    FuzzySet res(a.size());
    for (int i = 0; i < (int)a.size(); i++) res[i] = op(a[i], b[i]);
    return res;
}

// --- D.4.1. t-нормы: обобщение конъюнкции ∧ ---
// T: [0, 1]² → [0, 1]: коммутативна, ассоциативна, монотонна,
// T(a, 1) = a. Стандартные: min (Гёдель), произведение, Лукасевич
// max(0, a + b − 1); для μ ∈ {0, 1} любая t-норма — это ∧.
double t_norm(const string& name, double a, double b) {
    if (name == "product") return a * b;
    if (name == "lukasiewicz") return max(0.0, a + b - 1.0);
    return min(a, b);
}

// --- D.4.2. t-конормы: обобщение дизъюнкции ∨ ---
// Двойственны t-нормам: S(a, b) = 1 − T(1 − a, 1 − b) (де Морган);
// S(a, 0) = a. Стандартные: max, вероятностная a + b − ab,
// Лукасевич min(1, a + b).
double t_conorm(const string& name, double a, double b) {
    if (name == "probabilistic") return a + b - a * b;
    if (name == "lukasiewicz") return min(1.0, a + b);
    return max(a, b);
}

// --- D.4.1. Проверка аксиом t-нормы на сетке ---
// Коммутативность, ассоциативность, монотонность, единица 1 — по
// всем уровням сетки 0, 0.1, ..., 1; аксиомы (4.1) проверяются
// машиной для каждой конкретной t-нормы.
bool check_t_norm(const string& name) {
    vector<double> grid;
    for (int k = 0; k <= 10; k++) grid.push_back(k / 10.0);
    for (double a : grid)
        for (double b : grid) {
            if (fabs(t_norm(name, a, b) - t_norm(name, b, a)) > 1e-9)
                return false;
            if (fabs(t_norm(name, a, 1.0) - a) > 1e-9)
                return false;
            for (double c : grid) {
                double l = t_norm(name, t_norm(name, a, b), c);
                double r = t_norm(name, a, t_norm(name, b, c));
                if (fabs(l - r) > 1e-9) return false;
                if (a <= c && t_norm(name, a, b) > t_norm(name, c, b) + 1e-9)
                    return false;
            }
        }
    return true;
}

// --- D.4.2. Проверка аксиом t-конормы на сетке ---
// Те же аксиомы с единицей 0: S(a, 0) = a.
bool check_t_conorm(const string& name) {
    vector<double> grid;
    for (int k = 0; k <= 10; k++) grid.push_back(k / 10.0);
    for (double a : grid)
        for (double b : grid) {
            if (fabs(t_conorm(name, a, b) - t_conorm(name, b, a)) > 1e-9)
                return false;
            if (fabs(t_conorm(name, a, 0.0) - a) > 1e-9)
                return false;
            for (double c : grid) {
                double l = t_conorm(name, t_conorm(name, a, b), c);
                double r = t_conorm(name, a, t_conorm(name, b, c));
                if (fabs(l - r) > 1e-9) return false;
                if (a <= c && t_conorm(name, a, b) > t_conorm(name, c, b) + 1e-9)
                    return false;
            }
        }
    return true;
}

// --- D.4.3. Fuzzy Operations через t-нормы/конормы ---
// Нечёткое пересечение через T и объединение через S: поточечное
// применение (D.3.1) с выбранной операцией вместо min/max.
FuzzySet fuzzy_intersection_t(const FuzzySet& a, const FuzzySet& b,
                              const string& name) {
    return fuzzy_binary_op(a, b,
        [this, &name](double x, double y) { return t_norm(name, x, y); });
}

FuzzySet fuzzy_union_t(const FuzzySet& a, const FuzzySet& b,
                       const string& name) {
    return fuzzy_binary_op(a, b,
        [this, &name](double x, double y) { return t_conorm(name, x, y); });
}

// --- D.5.1. Нечёткое отношение ---
// Функция принадлежности μ_R: A × B → [0, 1] — матрица степеней
// (булева матрица отношений a.cpp — частный случай со значениями
// {0, 1}).
using FuzzyRelation = vector<vector<double>>;

// --- D.5.2. Max-min композиция ---
// (R ∘ S)(x, z) = max_y min(μ_R(x, y), μ_S(y, z)) — обобщение
// композиции отношений (a.cpp A.4): ∃y ∧/∨ заменены max/min.
FuzzyRelation fuzzy_relation_compose(const FuzzyRelation& r,
                                     const FuzzyRelation& s) {
    int n = r.size();
    int m = s[0].size();
    FuzzyRelation res(n, FuzzySet(m, 0.0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < m; k++) {
            double best = 0.0;
            for (int j = 0; j < (int)s.size(); j++)
                best = max(best, min(r[i][j], s[j][k]));
            res[i][k] = best;
        }
    return res;
}

// --- D.5.2. Транзитивное замыкание нечёткого отношения ---
// Наименьшее транзитивное отношение, содержащее R (обобщение
// замыкания a.cpp B.4): итерации cur ← cur ∪ (cur ∘ cur) до
// стабилизации.
FuzzyRelation fuzzy_transitive_closure(const FuzzyRelation& r) {
    FuzzyRelation cur = r;
    while (true) {
        FuzzyRelation next = fuzzy_relation_compose(cur, cur);
        bool changed = false;
        for (int i = 0; i < (int)cur.size(); i++)
            for (int j = 0; j < (int)cur[i].size(); j++)
                if (next[i][j] > cur[i][j] + 1e-12) {
                    cur[i][j] = next[i][j];
                    changed = true;
                }
        if (!changed) break;
    }
    return cur;
}

}; // конец struct FormalLogic

#ifndef FORMAL_LOGIC_MAIN
signed main() {
    FormalLogic L;

    cout << "=== A. Логика высказываний ===" << endl;
    Form* A = L.form_var(0);
    Form* Bq = L.form_var(1);
    Form* pp = L.form_imp(A, A);
    cout << "p→p тавтология: " << L.is_tautology(pp, 1) << endl;
    Form* contr = L.form_and(A, L.form_not(A));
    cout << "p∧¬p противоречие: " << L.is_contradiction(contr, 1)
         << ", выполнима: " << L.is_satisfiable(contr, 1) << endl;
    Form* f1 = L.form_imp(A, Bq);
    Form* f2 = L.form_or(L.form_not(A), Bq);
    cout << "(p→q) ≡ (¬p∨q): " << L.forms_equivalent(f1, f2, 2) << endl;
    Form* dm = L.form_not(L.form_and(A, Bq));
    Form* dm2 = L.form_or(L.form_not(A), L.form_not(Bq));
    cout << "¬(p∧q) ≡ ¬p∨¬q: " << L.forms_equivalent(dm, dm2, 2) << endl;
    cout << "Двойственная форма (p∧q)∨r: ";
    cout << L.form_to_string(L.dual_form(L.form_or(L.form_and(A, Bq), L.form_var(2))))
         << " (ожидаем (x0∨x1)∧x2)" << endl;
    vector<Form*> prem = {A, L.form_imp(A, Bq)};
    cout << "p, p→q ⊨ q: " << L.logical_consequence(prem, Bq, 2) << endl;
    auto tt = L.truth_table(f1, 2);
    cout << "Таблица p→q: ";
    for (int v : tt) cout << v << " ";
    cout << "(ожидаем 1 0 1 1)" << endl;

    cout << "\n=== B. Предикаты ===" << endl;
    // x ≤ y на {0,1}²: P(0,0)=1, P(0,1)=1, P(1,0)=0, P(1,1)=1
    vector<int> dims = {2, 2};
    vector<int> le = {1, 1, 0, 1};
    cout << "Множество истинности x≤y: ";
    for (auto& tup : L.pred_truth_set(dims, le))
        cout << "(" << tup[0] << "," << tup[1] << ") ";
    cout << endl;
    vector<int> al = L.pred_quantify(dims, 0, true, le);
    cout << "∀x (x≤y): [";
    for (int v : al) cout << v << " ";
    cout << "] (ожидаем 0 1)" << endl;
    vector<int> ex = L.pred_quantify(dims, 1, false, le);
    cout << "∃y (x≤y): [";
    for (int v : ex) cout << v << " ";
    cout << "] (ожидаем 1 1)" << endl;
    // закон де Моргана для кванторов: ¬∀xP ≡ ∃x¬P
    vector<int> neg_all = L.pred_not(L.pred_quantify(dims, 0, true, le));
    vector<int> ex_neg = L.pred_quantify(dims, 0, false, L.pred_not(le));
    cout << "¬∀xP ≡ ∃x¬P: " << L.preds_equivalent(neg_all, ex_neg) << endl;

    cout << "\n=== B. Формулы: подстановка и ПНФ ===" << endl;
    // ∀x₀(x₀∧x₁)[x₁ := x₀] — захват избегается переименованием
    Form* fxy = L.form_forall(0, L.form_and(L.form_var(0), L.form_var(1)));
    int fresh = 10;
    Form* sub = L.substitute_free(fxy, 1, 0, fresh);
    cout << "∀x(x∧y)[y:=x]: " << L.form_to_string(sub) << endl;
    // ПНФ: ∀x₀P(x₀) → ∃x₁Q(x₁)
    Form* ex1 = L.form_imp(L.form_forall(0, L.form_var(0)),
                           L.form_exists(1, L.form_var(1)));
    Form* pnf = L.to_pnf(ex1, fresh);
    cout << "ПНФ: " << L.form_to_string(pnf) << endl;
    cout << "Все кванторы впереди: " << L.is_pnf(pnf) << endl;

    cout << "\n=== C. ФИВ: аксиомы и вывод ===" << endl;
    Form* ax1 = L.form_imp(A, L.form_imp(Bq, A));
    cout << "p→(q→p) — аксиома A1: " << L.is_axiom_scheme(ax1) << endl;
    Form* notax = L.form_imp(A, Bq);
    cout << "p→q — аксиома: " << L.is_axiom_scheme(notax) << endl;
    auto self = L.deduce_self_imp(A);
    vector<bool> nohyp(self.size(), false);
    cout << "Вывод A→A корректен: " << L.verify_derivation(self, nohyp)
         << ", все формулы — тавтологии: " << L.soundness_check(self, 1) << endl;

    cout << "\n=== C. Теорема о дедукции ===" << endl;
    vector<Form*> der = {A, L.form_imp(A, Bq), Bq};
    vector<bool> der_hyp = {true, true, false};
    cout << "Исходный вывод [A, A→B, B] корректен: "
         << L.verify_derivation(der, der_hyp) << endl;
    auto [der2, hyp2] = L.deduction_transform(der, der_hyp, A);
    cout << "После дедукции вывод корректен (без гипотезы A): "
         << L.verify_derivation(der2, hyp2) << endl;
    cout << "Последняя формула — A→B: "
         << L.forms_equal(der2.back(), L.form_imp(A, Bq)) << endl;

    cout << "\n=== C. ФИП ===" << endl;
    Form* ax_uni = L.form_imp(L.form_forall(0, L.form_var(0)), L.form_var(1));
    cout << "∀xP(x)→P(y) — аксиома ФИП: "
         << L.is_axiom_scheme_pred(ax_uni) << endl;
    Form* notuni = L.form_imp(L.form_forall(0, L.form_var(0)),
                              L.form_and(L.form_var(0), L.form_var(1)));
    cout << "∀xP(x)→P(x)∧Q(x) — аксиома: "
         << L.is_axiom_scheme_pred(notuni) << endl;
    vector<Form*> pder = {L.form_var(0), L.form_forall(0, L.form_var(0))};
    vector<bool> phyp = {true, false};
    cout << "Вывод ФИП [P(x), ∀xP(x)] корректен (Gen): "
         << L.verify_pred_derivation(pder, phyp) << endl;

    cout << "\n=== C. ФА и нумерация Гёделя ===" << endl;
    auto pa = L.pa_axioms();
    cout << "Аксиомы Пеано (" << pa.size() << " схем):" << endl;
    for (auto s : pa) cout << "  " << s << endl;
    auto primes = L.sieve_primes(1000);
    string formula = "(p>q)";
    long long gn = L.godel_number(formula, primes);
    cout << "g(" << formula << ") = " << gn << endl;
    cout << "Декодирование: " << L.godel_decode(gn, primes) << endl;

    cout << "\n=== C.9. Машина Тьюринга ===" << endl;
    FormalLogic::TuringProgram inc;
    inc[{0, '1'}] = {0, '1', +1};
    inc[{0, ' '}] = {1, '1', 0};
    auto tr = L.turing_run(inc, "11", 100);
    cout << "0₁11 → " << tr.tape << " (остановка: " << tr.halted
         << ", шагов: " << tr.steps << ")" << endl;
    // незавершающаяся программа: бесконечное движение вправо —
    // ограничение лимитом шагов (md 9.5: проверка остановки за K шагов
    // разрешима, без лимита — нет)
    FormalLogic::TuringProgram loop;
    loop[{0, ' '}] = {0, ' ', +1};
    auto tl = L.turing_run(loop, " ", 10);
    cout << "Зацикливание (движение вправо): остановка за 10 шагов: "
         << tl.halted << " (верно: 0 — тезис Чёрча 9.5)" << endl;

    cout << "\n=== C.9. Числа Чёрча ===" << endl;
    FormalLogic::ChurchNumeral c2 = L.church_numeral(2);
    FormalLogic::ChurchNumeral c3 = L.church_numeral(3);
    cout << "2 + 3 = " << L.church_to_int(L.church_add(c2, c3)) << endl;
    cout << "2 * 3 = " << L.church_to_int(L.church_mul(c2, c3)) << endl;
    cout << "SUCC(2) = " << L.church_to_int(L.church_succ(c2)) << endl;
    FormalLogic::ChurchFunc f = [](int x) { return x + 10; };
    cout << "2(λx.x+10, 1) = " << L.church_apply(c2, f, 1) << endl;

    cout << "\n=== C.9. Рекурсивные функции ===" << endl;
    // сложение: add(x, 0) = I¹₁(x); add(x, y+1) = S(add(x, y))
    // — из базисных функций двумя операторами (аксиоматика, 9.4)
    FormalLogic::IntFunc g_add = L.basis_projection(1, 0);
    FormalLogic::RecFunc h_add =
        [](const vector<int>&, int, int prev) { return prev + 1; };
    FormalLogic::IntFunc add = L.primitive_recursion(g_add, h_add);
    cout << "add(3,4) = pr(I,S)(3,4) = " << add({3, 4}) << " (ожидаем 7)"
         << endl;
    // умножение: mul(x, 0) = 0; mul(x, y+1) = mul(x, y) + x
    FormalLogic::RecFunc h_mul =
        [](const vector<int>& x, int, int prev) { return prev + x[0]; };
    FormalLogic::IntFunc mul =
        L.primitive_recursion(L.basis_zero(), h_mul);
    cout << "mul(6,7) = " << mul({6, 7}) << " (ожидаем 42)" << endl;
    // факториал: fact(0) = 1; fact(y+1) = (y+1)·fact(y)
    FormalLogic::RecFunc h_fact =
        [](const vector<int>&, int y, int prev) { return (y + 1) * prev; };
    FormalLogic::IntFunc fact =
        L.primitive_recursion([](const vector<int>&) { return 1; }, h_fact);
    cout << "fact(5) = " << fact({5}) << " (ожидаем 120)" << endl;
    // подстановка: S(S(x)) = x + 2
    FormalLogic::IntFunc ss = L.substitution(L.basis_successor(),
        {L.basis_successor()});
    cout << "S∘S(5) = " << ss({5}) << " (ожидаем 7)" << endl;
    cout << "I³₁(7,8,9) = " << L.basis_projection(3, 1)({7, 8, 9})
         << " (ожидаем 8)" << endl;
    // минимизация: целая часть квадратного корня: μy[y² ≥ x]
    FormalLogic::IntFunc ge_sq =
        [](const vector<int>& xy) {
            int x = xy[0], y = xy[1];
            return y * y >= x ? 0 : 1;
        };
    FormalLogic::IntFunc isqrt = L.mu_minimize(ge_sq, 100);
    cout << "μy[y² ≥ 17] = " << isqrt({17}) << " (ожидаем 5)" << endl;

    cout << "\n=== D.1. Нечёткие множества ===" << endl;
    FormalLogic::FuzzySet A1 = {0.0, 0.3, 0.8, 1.0, 0.5, 0.0};
    cout << "Носитель A: {";
    for (int i : L.fuzzy_support(A1)) cout << i << " ";
    cout << "}" << endl;
    cout << "Ядро A: {";
    for (int i : L.fuzzy_core(A1)) cout << i << " ";
    cout << "}" << endl;
    for (double a : {0.4, 0.8}) {
        cout << "A_" << a << ": {";
        for (int i : L.alpha_cut(A1, a)) cout << i << " ";
        cout << "}" << endl;
    }
    // декомпозиция: восстановление A по срезам
    vector<vector<int>> cuts;
    for (int k = 0; k <= 4; k++) cuts.push_back(L.alpha_cut(A1, k / 4.0));
    FormalLogic::FuzzySet dec = L.fuzzy_decompose(cuts, 4);
    cout << "Декомпозиция восстановила A: ";
    bool ok = true;
    for (int i = 0; i < 6; i++)
        if (fabs(dec[i] - A1[i]) > 0.2) ok = false;
    cout << ok << endl;

    cout << "\n=== D.2. Операции ===" << endl;
    FormalLogic::FuzzySet B1 = {0.0, 0.6, 0.4, 0.2, 0.9, 0.7};
    FormalLogic::FuzzySet C1 = L.fuzzy_intersection(A1, B1);
    cout << "A∩B = {";
    for (double v : C1) cout << v << " ";
    cout << "} (min)" << endl;
    FormalLogic::FuzzySet D1 = L.fuzzy_union(A1, B1);
    cout << "A∪B = {";
    for (double v : D1) cout << v << " ";
    cout << "} (max)" << endl;
    FormalLogic::FuzzySet nA = L.fuzzy_complement(A1);
    cout << "¬A = {";
    for (double v : nA) cout << v << " ";
    cout << "} (1−μ)" << endl;
    FormalLogic::FuzzySet small = {0.0, 0.2, 0.5, 0.7, 0.4, 0.0};
    cout << "small ⊆ A: " << L.fuzzy_subset(small, A1) << endl;
    cout << "A ⊆ small: " << L.fuzzy_subset(A1, small) << endl;

    cout << "\n=== D.3. Нечёткая алгебраическая система ===" << endl;
    // операция «среднее» — поточечно на степенях
    FormalLogic::FuzzyOp avg = [](double x, double y) { return (x + y) / 2; };
    FormalLogic::FuzzySet avgset = L.fuzzy_binary_op(A1, B1, avg);
    cout << "Среднее A, B: {";
    for (double v : avgset) cout << v << " ";
    cout << "}" << endl;

    cout << "\n=== D.4. t-нормы и t-конормы ===" << endl;
    for (string name : {"godel", "product", "lukasiewicz"})
        cout << "t-норма " << name << ": " << L.check_t_norm(name) << endl;
    for (string name : {"max", "probabilistic", "lukasiewicz"})
        cout << "t-конорма " << name << ": " << L.check_t_conorm(name)
             << endl;
    cout << "T_Лукасевич(0.4, 0.7) = "
         << L.t_norm("lukasiewicz", 0.4, 0.7) << endl;
    cout << "S_Лукасевич(0.4, 0.7) = "
         << L.t_conorm("lukasiewicz", 0.4, 0.7) << endl;
    FormalLogic::FuzzySet Ts = L.fuzzy_intersection_t(A1, B1, "product");
    cout << "A ∩_prod B = {";
    for (double v : Ts) cout << v << " ";
    cout << "}" << endl;
    FormalLogic::FuzzySet U = L.fuzzy_union_t(A1, B1, "probabilistic");
    cout << "A ∪_prob B = {";
    for (double v : U) cout << v << " ";
    cout << "}" << endl;

    cout << "\n=== D.5. Нечёткие отношения ===" << endl;
    // R: молодые-высокие, S: высокие-быстрые; композиция — max-min
    FormalLogic::FuzzyRelation R = {{0.8, 0.2}, {0.4, 0.9}};
    FormalLogic::FuzzyRelation S = {{0.6, 0.3}, {0.1, 0.7}};
    FormalLogic::FuzzyRelation comp = L.fuzzy_relation_compose(R, S);
    cout << "R∘S = {" << endl;
    for (auto& row : comp) {
        cout << "  {";
        for (double v : row) cout << v << " ";
        cout << "}" << endl;
    }
    cout << "}" << endl;
    FormalLogic::FuzzyRelation T = {{0.5, 0.1}, {0.2, 0.4}};
    FormalLogic::FuzzyRelation TC = L.fuzzy_transitive_closure(T);
    cout << "T* = {" << endl;
    for (auto& row : TC) {
        cout << "  {";
        for (double v : row) cout << v << " ";
        cout << "}" << endl;
    }
    cout << "}" << endl;
    // транзитивность замыкания: (T*∘T*) ≤ T* поточечно
    FormalLogic::FuzzyRelation TT = L.fuzzy_relation_compose(TC, TC);
    bool trans = true;
    for (int i = 0; i < 2 && trans; i++)
        for (int j = 0; j < 2 && trans; j++)
            if (TT[i][j] > TC[i][j] + 1e-12) trans = false;
    cout << "T* транзитивно: " << trans << endl;
}
#endif // FORMAL_LOGIC_MAIN
#endif // DISCRETE_LOGIC_B_CPP
