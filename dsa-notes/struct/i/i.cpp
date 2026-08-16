#ifndef STRUCT_I_CPP
#define STRUCT_I_CPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <map>
#include <set>
#include <sstream>
#include <climits>
using namespace std;

// =============================================================
// IX. СИНТАКСИЧЕСКИЕ СТРУКТУРЫ (ДЕРЕВЬЯ РАЗБОРА)
// =============================================================
// Структура md: A. Деревья разбора
//               → B. Построение (парсеры)
//               → C. Трансформации и вычисления
//               → D. Валидация и проверка корректности
//               → E. Специализированные структуры для выражений
//               → F. Применения
//
// SyntaxStructures наследует ConcurrentStructures (h.cpp). Ось IX —
// представление синтаксиса (языков и выражений) деревьями и их
// построение/обработка. Собственных примитивов почти нет: вся
// машинерия строится на стеках и деревьях из I, II.
//
// Переиспользование из базы:
//   * op_prec / infix_to_postfix / eval_postfix (I.C) — приоритеты
//     операторов (Lexer), Shunting Yard и оценка постфикса для
//     однозначных операндов (A.4, B.2, C.2);
//   * is_balanced (I.C) — парность скобок при валидации (D.2);
//   * стеки (I.C) — рабочая память shift-reduce парсера (B.4) и
//     байткод-машины (C.5);
//   * обходы деревьев (II.B) — рекурсивные обходы AST (A.4, C.1);
//   * арена-память (I.B) — узлы деревьев на new, без освобождения;
//   * хеширование (hashing.md I) — hash-consing DAG (E.1).
//
// Порядок структур строго соответствует порядку md (A → F).
// B.6 (LR-парсеры) и F.3 (front-end) — анализ, своего кода нет.
// Token / Lexer / ASTNode — общие вспомогательные типы (лексика и
// узел дерева), описаны в md D.1 и A.1, определены в начале класса:
// их используют все парсеры и трансформации.
// ipow (бинарное возведение в степень) — общий примитив всех
// вычислений оператора '^' (A.1, B.4, C.4, C.5).
//
// ВНИМАНИЕ (скрытие имён): методы parse, eval, build, to_string,
// insert, lookup, next, size, depth здесь локальные; одноимённые
// из других веток не подключаются. Узлы деревьев не освобождаются
// (демо/тесты): память живёт до конца программы.

#define STRUCT_H_MAIN
#include "../h/h.cpp"
#undef STRUCT_H_MAIN

struct SyntaxStructures : ConcurrentStructures {

// =============================================================
// Общие вспомогательные типы: лексика (Token, Lexer) — см. md D.1
// =============================================================

struct Token {
    enum Kind { NUM, VAR, OP, LPAR, RPAR, END, BAD } kind = BAD;
    string lex;
    long long val = 0;
    int prec = 0;
    int pos = 0;
};

struct Lexer {
    string s;
    vector<Token> tokens;

    Lexer(const string& src) : s(src) {
        size_t i = 0;
        while (i < s.size()) {
            char c = s[i];
            if (isspace((unsigned char)c)) { ++i; continue; }
            if (isdigit((unsigned char)c)) {
                size_t st = i;
                while (i < s.size() && isdigit((unsigned char)s[i])) ++i;
                Token t; t.kind = Token::NUM; t.lex = s.substr(st, i - st);
                t.val = stoll(t.lex); t.pos = (int)st;
                tokens.push_back(t);
                continue;
            }
            if (isalpha((unsigned char)c)) {
                size_t st = i;
                while (i < s.size() && isalnum((unsigned char)s[i])) ++i;
                Token t; t.kind = Token::VAR; t.lex = s.substr(st, i - st); t.pos = (int)st;
                tokens.push_back(t);
                continue;
            }
            Token t; t.kind = Token::BAD; t.lex = string(1, c); t.pos = (int)i;
            switch (c) {
                case '+': case '-': case '*': case '/': case '^':
                    t.kind = Token::OP; t.prec = LinearStructures().op_prec(c); break;
                case '(': t.kind = Token::LPAR; break;
                case ')': t.kind = Token::RPAR; break;
                default: break;
            }
            tokens.push_back(t);
            ++i;
        }
        Token e; e.kind = Token::END; e.pos = (int)s.size();
        tokens.push_back(e);
    }
    bool has_bad() const {
        for (const Token& t : tokens) if (t.kind == Token::BAD) return true;
        return false;
    }
};

// Общий примитив: целая степень a^b (b ≥ 0) за O(log b) — бинарное
// возведение. Используется везде, где вычисляется оператор '^'
// (A.1 eval, B.4 apply, C.4 folding, C.5 VM). b < 0 → 0 (семантика
// выражения: отрицательная степень не входит в язык).
static long long ipow(long long a, long long b) {
    if (b < 0) return 0;
    long long r = 1;
    while (b) {
        if (b & 1) r *= a;
        a *= a;
        b >>= 1;
    }
    return r;
}

// =============================================================
// A. ДЕРЕВЬЯ РАЗБОРА
// =============================================================

// --- A.1. AST — абстрактное синтаксическое дерево (бинарное) ---
// Узел: лист (константа/переменная) или внутренний (оператор с 1–2
// детьми). Построение из постфикса, вычисление, инф/постфиксные
// формы, размер и глубина — базовые операции над деревом разбора.
struct ASTNode {
    long long val = 0;
    char op = 0;              // '+','-','*','/','^','~' (~ — унарный минус)
    string name;              // имя переменной (лист-переменная)
    bool is_var = false;
    ASTNode* l = nullptr;
    ASTNode* r = nullptr;

    ASTNode() = default;
    ASTNode(long long v) : val(v) {}
    ASTNode(const string& vn) : name(vn), is_var(true) {}
    ASTNode(char o, ASTNode* a, ASTNode* b) : op(o), l(a), r(b) {}
    ASTNode(char o, ASTNode* a) : op(o), l(a) {}
    bool leaf() const { return op == 0; }
};

struct AST {
    // Приоритет оператора: таблица +,-,*,/,^ из I.C (op_prec), '~' — выше всех.
    static int p_of(char c) {
        if (c == '~') return 4;
        return LinearStructures().op_prec(c);
    }
    static ASTNode* from_postfix(const vector<Token>& pf) {
        vector<ASTNode*> st;
        for (const Token& t : pf) {
            if (t.kind == Token::NUM) st.push_back(new ASTNode(t.val));
            else if (t.kind == Token::VAR) st.push_back(new ASTNode(t.lex));
            else if (t.kind == Token::OP) {
                if (t.lex == "~") {
                    ASTNode* a = st.back(); st.pop_back();
                    st.push_back(new ASTNode('~', a));
                } else {
                    ASTNode* b = st.back(); st.pop_back();
                    ASTNode* a = st.back(); st.pop_back();
                    st.push_back(new ASTNode(t.lex[0], a, b));
                }
            }
        }
        return st.empty() ? nullptr : st.back();
    }
    static long long eval(ASTNode* n, const map<string, long long>& env = {}) {
        if (!n) return 0;
        if (n->leaf()) {
            if (n->is_var) { auto it = env.find(n->name); return it == env.end() ? 0 : it->second; }
            return n->val;
        }
        if (n->op == '~') return -eval(n->l, env);
        long long a = eval(n->l, env), b = eval(n->r, env);
        if (n->op == '+') return a + b;
        if (n->op == '-') return a - b;
        if (n->op == '*') return a * b;
        if (n->op == '/') return a / b;
        if (n->op == '^') return ipow(a, b);
        return 0;
    }
    static string to_infix(ASTNode* n) {
        if (!n) return "";
        if (n->leaf()) return n->is_var ? n->name : to_string(n->val);
        if (n->op == '~') {
            if (n->l && n->l->leaf()) return "-" + to_infix(n->l);
            return "(-" + to_infix(n->l) + ")";
        }
        int p = p_of(n->op);
        string ls = to_infix(n->l), rs = to_infix(n->r);
        bool lp = false, rp = false;
        if (n->l && !n->l->leaf()) {
            int q = p_of(n->l->op);
            if (q < p) lp = true;
            else if (q == p && n->op == '^') lp = true;   // правоассоциативность ^
        }
        if (n->r && !n->r->leaf()) {
            int q = p_of(n->r->op);
            if (q < p) rp = true;
            else if (q == p && n->op != '^') rp = true;   // левоассоциативные: правый ребёнок
        }
        string s = string(1, n->op);
        return (lp ? "(" + ls + ")" : ls) + s + (rp ? "(" + rs + ")" : rs);
    }
    static string to_postfix(ASTNode* n) {
        if (!n) return "";
        if (n->leaf()) return n->is_var ? n->name : to_string(n->val);
        if (n->op == '~') return to_postfix(n->l) + " ~";
        return to_postfix(n->l) + " " + to_postfix(n->r) + " " + string(1, n->op);
    }
    static int size(ASTNode* n) {
        if (!n) return 0;
        if (n->op == '~') return 1 + size(n->l);
        return 1 + size(n->l) + size(n->r);
    }
    static int depth(ASTNode* n) {
        if (!n) return 0;
        if (n->op == '~') return 1 + depth(n->l);
        return 1 + max(depth(n->l), depth(n->r));
    }
};

// --- A.2. N-арное AST: операторы, функции, листья ---
// Узлы трёх сортов: константа, переменная, вызов функции (или
// бинарный оператор) с произвольным числом аргументов. Вычисление
// с окружением переменных; функции sin/cos/log/sqrt/abs — из <cmath>.
struct FunctionAST {
    enum Kind { NUM, VAR, FUN, OP2 } kind = NUM;
    long long val = 0;
    string name;                    // переменная или функция
    vector<FunctionAST*> args;      // аргументы (FUN — любая арность, OP2 — ровно 2)
    string op;                      // оператор для OP2

    FunctionAST() = default;
    explicit FunctionAST(long long v) : kind(NUM), val(v) {}
    static FunctionAST* var(const string& n) { FunctionAST* x = new FunctionAST(); x->kind = VAR; x->name = n; return x; }
    static FunctionAST* fun(const string& f, const vector<FunctionAST*>& a) { FunctionAST* x = new FunctionAST(); x->kind = FUN; x->name = f; x->args = a; return x; }
    static FunctionAST* op2(const string& o, FunctionAST* a, FunctionAST* b) { FunctionAST* x = new FunctionAST(); x->kind = OP2; x->op = o; x->args = {a, b}; return x; }

    static double eval(FunctionAST* n, const map<string, double>& vars) {
        if (!n) return 0;
        if (n->kind == NUM) return (double)n->val;
        if (n->kind == VAR) { auto it = vars.find(n->name); return it == vars.end() ? 0.0 : it->second; }
        if (n->kind == OP2) {
            double a = eval(n->args[0], vars), b = eval(n->args[1], vars);
            if (n->op == "+") return a + b;
            if (n->op == "-") return a - b;
            if (n->op == "*") return a * b;
            if (n->op == "/") return a / b;
            if (n->op == "^") return pow(a, b);
            return 0;
        }
        double x = n->args.empty() ? 0 : eval(n->args[0], vars);
        if (n->name == "sin") return sin(x);
        if (n->name == "cos") return cos(x);
        if (n->name == "log") return log(x);
        if (n->name == "sqrt") return sqrt(x);
        if (n->name == "abs") return fabs(x);
        return 0;
    }
    static string to_string(FunctionAST* n) {
        if (!n) return "";
        if (n->kind == NUM) return std::to_string(n->val);
        if (n->kind == VAR) return n->name;
        if (n->kind == OP2) return "(" + FunctionAST::to_string(n->args[0]) + " " + n->op + " " + FunctionAST::to_string(n->args[1]) + ")";
        string out = n->name + "(";
        for (size_t i = 0; i < n->args.size(); ++i) { if (i) out += ", "; out += FunctionAST::to_string(n->args[i]); }
        return out + ")";
    }
};

// --- A.3. Parse Tree — конкретное синтаксическое дерево ---
// Полное дерево разбора: нетерминалы (EXPR/TERM/FACTOR) и терминалы
// (токены). Каноническая форма — свёртка скелета нетерминалов в AST
// (A.1): конкретное синтаксическое дерево → абстрактное.
struct ParseTree {
    enum Kind { NONTERM, TERM };
    Kind kind;
    string name;
    vector<ParseTree*> children;

    ParseTree(Kind k, const string& n) : kind(k), name(n) {}

    static string dump(ParseTree* n, int depth = 0) {
        if (!n) return "";
        string out(depth * 2, ' ');
        out += n->name;
        out += "\n";
        for (ParseTree* c : n->children) out += dump(c, depth + 1);
        return out;
    }
    static ASTNode* to_ast(ParseTree* n) { return n ? ast_expr(n) : nullptr; }
    static ASTNode* ast_expr(ParseTree* n) {
        ASTNode* res = ast_term(n->children[0]);
        for (size_t i = 1; i + 1 < n->children.size(); i += 2) {
            char op = n->children[i]->name[0];
            res = new ASTNode(op, res, ast_term(n->children[i + 1]));
        }
        return res;
    }
    static ASTNode* ast_term(ParseTree* n) {
        ASTNode* res = ast_factor(n->children[0]);
        for (size_t i = 1; i + 1 < n->children.size(); i += 2) {
            char op = n->children[i]->name[0];
            res = new ASTNode(op, res, ast_factor(n->children[i + 1]));
        }
        return res;
    }
    static ASTNode* ast_factor(ParseTree* n) {
        if (n->children.size() == 1) return new ASTNode(stoll(n->children[0]->name));
        if (n->children[0]->name == "-") return new ASTNode('~', ast_factor(n->children[1]));
        return ast_expr(n->children[1]);   // '(' EXPR ')'
    }
    struct Builder {
        string s;
        size_t pos = 0;
        Builder(const string& src) : s(src) {}
        char peek() {
            while (pos < s.size() && isspace((unsigned char)s[pos])) ++pos;
            return pos < s.size() ? s[pos] : '\0';
        }
        ParseTree* expr() {
            ParseTree* n = new ParseTree(NONTERM, "EXPR");
            n->children.push_back(term());
            while (peek() == '+' || peek() == '-') {
                n->children.push_back(new ParseTree(TERM, string(1, s[pos++])));  // s[pos] после isspace
                n->children.push_back(term());
            }
            return n;
        }
        ParseTree* term() {
            ParseTree* n = new ParseTree(NONTERM, "TERM");
            n->children.push_back(factor());
            while (peek() == '*' || peek() == '/') {
                n->children.push_back(new ParseTree(TERM, string(1, s[pos++])));  // s[pos] после isspace
                n->children.push_back(factor());
            }
            return n;
        }
        ParseTree* factor() {
            if (peek() == '-') {
                ParseTree* n = new ParseTree(NONTERM, "FACTOR");
                ++pos;
                n->children.push_back(new ParseTree(TERM, "-"));
                n->children.push_back(factor());
                return n;
            }
            if (peek() == '(') {
                ParseTree* n = new ParseTree(NONTERM, "FACTOR");
                n->children.push_back(new ParseTree(TERM, "("));
                ++pos;
                n->children.push_back(expr());
                n->children.push_back(new ParseTree(TERM, ")"));  // pos уже на ')'
                ++pos;
                return n;
            }
            ParseTree* n = new ParseTree(NONTERM, "FACTOR");
            size_t st = pos;
            while (pos < s.size() && isdigit((unsigned char)s[pos])) ++pos;
            n->children.push_back(new ParseTree(TERM, s.substr(st, pos - st)));
            return n;
        }
        ParseTree* parse() { return expr(); }
    };
    static ParseTree* build(const string& s) { return Builder(s).parse(); }
};

// --- A.4. Expression Tree — специализированное AST ---
// Готовое выражение как дерево: построение из постфиксных токенов
// (B.2), вычисление, инф/постфиксные формы, размер и глубина.
// Свойства узлов (приоритет, ассоциативность) задают форму дерева.
struct ExpressionTree {
    ASTNode* root = nullptr;
    void build(const vector<Token>& pf) { root = AST::from_postfix(pf); }
    long long eval(const map<string, long long>& env = {}) const { return AST::eval(root, env); }
    string to_infix() const { return AST::to_infix(root); }
    string to_postfix() const { return AST::to_postfix(root); }
    int size() const { return AST::size(root); }
    int depth() const { return AST::depth(root); }
};

// =============================================================
// B. ПОСТРОЕНИЕ ДЕРЕВЬЕВ РАЗБОРА (ПАРСЕРЫ)
// =============================================================

// --- B.1. Рекурсивный спуск (recursive descent) ---
// Грамматика с приоритетами: expr → term (('+'|'-') term)*;
// term → factor (('*'|'/') factor)*; factor → NUM | VAR | '(' expr ')'
// | '-' factor. Возвращает AST (A.1).
struct RecursiveDescent {
    const vector<Token>* toks;
    size_t idx = 0;
    RecursiveDescent(const vector<Token>& t) : toks(&t) {}
    const Token& peek() const { return (*toks)[idx]; }
    const Token& next() { return (*toks)[idx++]; }

    ASTNode* expr() {
        ASTNode* a = term();
        while (peek().kind == Token::OP && (peek().lex == "+" || peek().lex == "-")) {
            char op = next().lex[0];
            a = new ASTNode(op, a, term());
        }
        return a;
    }
    ASTNode* term() {
        ASTNode* a = factor();
        while (peek().kind == Token::OP && (peek().lex == "*" || peek().lex == "/")) {
            char op = next().lex[0];
            a = new ASTNode(op, a, factor());
        }
        return a;
    }
    ASTNode* factor() {
        if (peek().kind == Token::OP && peek().lex == "-") { next(); return new ASTNode('~', factor()); }
        if (peek().kind == Token::LPAR) { next(); ASTNode* a = expr(); next(); return a; }
        if (peek().kind == Token::VAR) return new ASTNode(next().lex);
        return new ASTNode(next().val);   // NUM
    }
    ASTNode* parse() { return expr(); }
};

// --- B.2. Операторный парсер (Shunting Yard) ---
// Инфикс → постфикс: приоритеты (переиспользуется op_prec из I.C),
// правоассоциативность '^', унарный минус ('~'), скобки; операнды —
// многоразрядные числа и переменные.
struct ShuntingYard {
    static bool right_assoc(const string& op) { return op == "^"; }
    static vector<Token> to_postfix(const string& s) {
        Lexer lex(s);
        vector<Token> out;
        vector<Token> st;
        bool expect_operand = true;
        for (const Token& t : lex.tokens) {
            if (t.kind == Token::NUM || t.kind == Token::VAR) { out.push_back(t); expect_operand = false; }
            else if (t.kind == Token::LPAR) { st.push_back(t); expect_operand = true; }
            else if (t.kind == Token::RPAR) {
                while (!st.empty() && st.back().kind != Token::LPAR) { out.push_back(st.back()); st.pop_back(); }
                if (!st.empty()) st.pop_back();
                expect_operand = false;
            }
            else if (t.kind == Token::OP) {
                string op = t.lex;
                Token u = t;
                if (op == "-" && expect_operand) { u.lex = "~"; u.prec = AST::p_of('~'); st.push_back(u); continue; }
                while (!st.empty() && st.back().kind == Token::OP &&
                       (st.back().prec > t.prec || (st.back().prec == t.prec && !right_assoc(op)))) {
                    out.push_back(st.back()); st.pop_back();
                }
                st.push_back(u);
                expect_operand = true;
            }
        }
        while (!st.empty()) { out.push_back(st.back()); st.pop_back(); }
        return out;
    }
    static string to_string(const vector<Token>& pf) {
        string out;
        for (const Token& t : pf) {
            if (t.kind == Token::NUM) out += std::to_string(t.val) + " ";
            else if (t.kind == Token::VAR) out += t.lex + " ";
            else if (t.kind == Token::OP) out += t.lex + " ";
        }
        return out;
    }
};

// --- B.3. Pratt-парсер (top-down operator precedence) ---
// Рекурсивный вариант операторного парсера: вместо двух стеков —
// приоритеты управляют рекурсией (precedence climbing). Операнд
// слева, пока приоритет следующего оператора ≥ минимального — берём
// оператор и рекурсивно парсим правую часть с повышенным порогом;
// '^' правоассоциативен (порог не повышается), '~' — в parse_prim.
struct PrattParser {
    const vector<Token>* toks;
    size_t idx = 0;
    PrattParser(const vector<Token>& t) : toks(&t) {}
    const Token& peek() const { return (*toks)[idx]; }
    const Token& next() { return (*toks)[idx++]; }

    static int prec(const string& op) { return AST::p_of(op[0]); }
    static bool right_assoc(const string& op) { return op == "^"; }

    ASTNode* parse_prim() {
        if (peek().kind == Token::OP && peek().lex == "-") { next(); return new ASTNode('~', parse_prim()); }
        if (peek().kind == Token::LPAR) { next(); ASTNode* a = parse_expr(1); next(); return a; }
        if (peek().kind == Token::VAR) return new ASTNode(next().lex);
        return new ASTNode(next().val);   // NUM
    }
    ASTNode* parse_expr(int min_prec) {
        ASTNode* lhs = parse_prim();
        while (peek().kind == Token::OP) {
            string op = peek().lex;
            int p = prec(op);
            if (p < min_prec) break;
            next();
            int threshold = right_assoc(op) ? p : p + 1;
            lhs = new ASTNode(op[0], lhs, parse_expr(threshold));
        }
        return lhs;
    }
    ASTNode* parse() { return parse_expr(1); }
};

// --- B.4. Shift-Reduce парсер (стек — мост из I.C) ---
// Операторный снизу-вверх: числа сдвигаются в стек значений, при
// появлении оператора с меньшим приоритетом выполняется свёртка
// (reduce) старших операторов. Рабочая память — два стека (I.C).
struct ShiftReduceParser {
    static void apply(vector<long long>& vals, vector<Token>& ops) {
        char op = ops.back().lex[0]; ops.pop_back();
        if (op == '~') { long long a = vals.back(); vals.pop_back(); vals.push_back(-a); return; }
        long long b = vals.back(); vals.pop_back();
        long long a = vals.back(); vals.pop_back();
        if (op == '+') vals.push_back(a + b);
        else if (op == '-') vals.push_back(a - b);
        else if (op == '*') vals.push_back(a * b);
        else if (op == '/') vals.push_back(a / b);
        else if (op == '^') vals.push_back(ipow(a, b));
    }
    static long long parse(const string& s) {
        Lexer lex(s);
        vector<long long> vals;
        vector<Token> ops;
        bool expect_operand = true;
        for (const Token& t : lex.tokens) {
            if (t.kind == Token::NUM) { vals.push_back(t.val); expect_operand = false; }
            else if (t.kind == Token::LPAR) { ops.push_back(t); expect_operand = true; }
            else if (t.kind == Token::RPAR) {
                while (!ops.empty() && ops.back().kind != Token::LPAR) apply(vals, ops);
                if (!ops.empty()) ops.pop_back();
                expect_operand = false;
            }
            else if (t.kind == Token::OP) {
                if (t.lex == "-" && expect_operand) { Token u = t; u.lex = "~"; u.prec = AST::p_of('~'); ops.push_back(u); expect_operand = true; continue; }
                bool right = (t.lex == "^");
                while (!ops.empty() && ops.back().kind == Token::OP &&
                       (ops.back().prec > t.prec || (ops.back().prec == t.prec && !right))) apply(vals, ops);
                ops.push_back(t);
                expect_operand = true;
            }
        }
        while (!ops.empty()) apply(vals, ops);
        return vals.back();
    }
};

// --- B.5. LL(1)-парсер (предсказывающий) ---
// Табличный разбор сверху-вниз по грамматике
//   E → T E';  E' → '+' T E' | ε;  T → F T';  T' → '*' F T' | ε;
//   F → num | '(' E ')'.
// Множества FIRST/FOLLOW определяют таблицу: выбор продукции — по
// одному токену (lookahead). '-' не в грамматике — разбор падает.
struct LL1Parser {
    enum NT { E = 0, E1, T, T1, F };          // нетерминалы 0..4
    enum Term { PLUS = 5, STAR, LPAR_T, RPAR_T, NUM_T, END_T };  // терминалы 5..10

    static int to_term(const Token& t) {
        if (t.kind == Token::NUM) return NUM_T;
        if (t.kind == Token::LPAR) return LPAR_T;
        if (t.kind == Token::RPAR) return RPAR_T;
        if (t.kind == Token::OP && t.lex == "+") return PLUS;
        if (t.kind == Token::OP && t.lex == "*") return STAR;
        if (t.kind == Token::END) return END_T;
        return -1;
    }
    static void record(vector<string>* d, const string& p) { if (d) d->push_back(p); }

    static bool parse(const string& s, vector<string>* deriv = nullptr) {
        Lexer lex(s);
        const vector<Token>& toks = lex.tokens;
        vector<int> st; st.push_back(E);
        size_t i = 0;
        auto lookahead = [&]() { return to_term(toks[min(i, toks.size() - 1)]); };
        while (!st.empty()) {
            int X = st.back(); st.pop_back();
            int la = lookahead();
            if (X >= PLUS) {                       // терминал
                if (X != la) return false;
                ++i;
                continue;
            }
            vector<int> rhs;
            bool eps = false;
            switch (X) {
                case E:
                    if (la == NUM_T || la == LPAR_T) { rhs = {T, E1}; record(deriv, "E -> T E'"); }
                    else return false;
                    break;
                case E1:
                    if (la == PLUS) { rhs = {PLUS, T, E1}; record(deriv, "E' -> + T E'"); }
                    else if (la == RPAR_T || la == END_T) { eps = true; record(deriv, "E' -> eps"); }
                    else return false;
                    break;
                case T:
                    if (la == NUM_T || la == LPAR_T) { rhs = {F, T1}; record(deriv, "T -> F T'"); }
                    else return false;
                    break;
                case T1:
                    if (la == STAR) { rhs = {STAR, F, T1}; record(deriv, "T' -> * F T'"); }
                    else if (la == PLUS || la == RPAR_T || la == END_T) { eps = true; record(deriv, "T' -> eps"); }
                    else return false;
                    break;
                case F:
                    if (la == NUM_T) { rhs = {NUM_T}; record(deriv, "F -> num"); }
                    else if (la == LPAR_T) { rhs = {LPAR_T, E, RPAR_T}; record(deriv, "F -> ( E )"); }
                    else return false;
                    break;
            }
            if (!eps) for (int k = (int)rhs.size() - 1; k >= 0; --k) st.push_back(rhs[k]);
        }
        return lookahead() == END_T;
    }
};

// --- B.6. LR-парсеры (анализ) ---
// См. md B.6: канонические SLR(1)/LALR(1) строятся по таблицам
// действий/переходов из автомата LR(0)-пунктов; shift-reduce (B.4) —
// упрощённый операторный случай без таблицы состояний.

// =============================================================
// C. ТРАНСФОРМАЦИИ И ВЫЧИСЛЕНИЯ
// =============================================================

// --- C.1. Visitor Pattern (обход дерева) ---
// Универсальный обход с замыканиями pre/in/post; демо — печать и
// сбор значений листьев без изменения дерева.
struct TreeVisitor {
    static void walk(ASTNode* n, const function<void(ASTNode*)>& pre,
                     const function<void(ASTNode*)>& in,
                     const function<void(ASTNode*)>& post) {
        if (!n) return;
        if (pre) pre(n);
        if (n->op == '~') {
            walk(n->l, pre, in, post);
        } else if (!n->leaf()) {
            walk(n->l, pre, in, post);
            if (in) in(n);
            walk(n->r, pre, in, post);
        }
        if (post) post(n);
    }
    static string collect(ASTNode* n) {
        string out;
        walk(n, [&](ASTNode* x) {
            if (x->leaf()) out += (x->is_var ? x->name : to_string(x->val)) + " ";
            else out += string(1, x->op) + " ";
        }, nullptr, nullptr);
        return out;
    }
    static long long leaf_sum(ASTNode* n) {
        long long s = 0;
        walk(n, nullptr, nullptr, [&](ASTNode* x) { if (x->leaf() && !x->is_var) s += x->val; });
        return s;
    }
};

// --- C.2. Интерпретация с переменными ---
// Мини-интерпретатор строк: "x = <expr>" (присваивание) и "<expr>"
// (вычисление) над окружением переменных. Повторяет вычисление AST
// (A.1) через рекурсивный спуск (B.1).
struct Interpreter {
    static string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }
    static long long exec_line(const string& line, map<string, long long>& env, bool* assigned = nullptr) {
        size_t eq = line.find('=');
        if (eq != string::npos) {
            string name = trim(line.substr(0, eq));
            Lexer lx(line.substr(eq + 1));
            RecursiveDescent rd(lx.tokens);
            long long v = AST::eval(rd.parse(), env);
            env[name] = v;
            if (assigned) *assigned = true;
            return v;
        }
        Lexer lx(line);
        RecursiveDescent rd(lx.tokens);
        return AST::eval(rd.parse(), env);
    }
};

// --- C.3. Символьное дифференцирование ---
// Производная AST по переменной 'x': правила d(const)=0, d(x)=1,
// линейность, произведение, частное, степень (показатель — константа).
// Результат — новое дерево (упрощение — C.4).
struct Differentiator {
    static ASTNode* diff(ASTNode* n) {
        if (!n) return new ASTNode(0);
        if (n->leaf()) {
            if (n->is_var && n->name == "x") return new ASTNode(1);
            return new ASTNode(0);
        }
        if (n->op == '~') return new ASTNode('~', diff(n->l));
        ASTNode* a = n->l;
        ASTNode* b = n->r;
        ASTNode* da = diff(a);
        ASTNode* db = diff(b);
        if (n->op == '+') return new ASTNode('+', da, db);
        if (n->op == '-') return new ASTNode('-', da, db);
        if (n->op == '*') return new ASTNode('+', new ASTNode('*', da, b), new ASTNode('*', a, db));
        if (n->op == '/') {
            ASTNode* num = new ASTNode('-', new ASTNode('*', da, b), new ASTNode('*', a, db));
            ASTNode* den = new ASTNode('^', b, new ASTNode(2));
            return new ASTNode('/', num, den);
        }
        if (n->op == '^') {   // d(a^b)/dx = b·a^(b-1)·a'
            ASTNode* e = new ASTNode('-', b, new ASTNode(1));
            ASTNode* p = new ASTNode('^', a, e);
            return new ASTNode('*', new ASTNode('*', b, p), da);
        }
        return new ASTNode(0);
    }
};

// --- C.4. Упрощение и constant folding ---
// Свёртка константных поддеревьев и алгебраические правила
// (x+0, x*1, x*0, x^1, 0-x = -x) — рекурсивно снизу вверх.
struct Simplifier {
    static bool is_const(ASTNode* n) { return n && n->leaf() && !n->is_var; }
    static ASTNode* simplify(ASTNode* n) {
        if (!n) return nullptr;
        if (n->leaf()) return n;
        ASTNode* l = simplify(n->l);
        ASTNode* r = n->r ? simplify(n->r) : nullptr;
        if (n->op == '~') {
            if (is_const(l)) return new ASTNode(-l->val);
            if (l->op == '~') return l->l;
            return new ASTNode('~', l);
        }
        if (is_const(l) && is_const(r)) {
            long long a = l->val, b = r->val;
            if (n->op == '+') return new ASTNode(a + b);
            if (n->op == '-') return new ASTNode(a - b);
            if (n->op == '*') return new ASTNode(a * b);
            if (n->op == '/') return new ASTNode(a / b);
            if (n->op == '^') return new ASTNode(ipow(a, b));
        }
        if (n->op == '+' && is_const(l) && l->val == 0) return r;
        if (n->op == '+' && is_const(r) && r->val == 0) return l;
        if (n->op == '-' && is_const(l) && l->val == 0) return new ASTNode('~', r);
        if (n->op == '*' && is_const(l) && l->val == 1) return r;
        if (n->op == '*' && is_const(r) && r->val == 1) return l;
        if (n->op == '*' && is_const(l) && l->val == 0) return new ASTNode(0);
        if (n->op == '*' && is_const(r) && r->val == 0) return new ASTNode(0);
        if (n->op == '^' && is_const(r) && r->val == 1) return l;
        return new ASTNode(n->op, l, r);
    }
};

// --- C.5. Компиляция в байт-код (стековая VM) ---
// AST → последовательность инструкций стековой машины (PUSH/ADD/
// SUB/MUL/DIV/POW/NEG) + интерпретатор байт-кода. Компиляция —
// постфиксный обход (та же схема, что постфиксная запись A.1).
struct BytecodeCompiler {
    enum Op { PUSH, ADD, SUB, MUL, DIV, POW, NEG };
    struct Ins { Op op; long long arg; };

    static vector<Ins> compile(ASTNode* n) {
        vector<Ins> code;
        emit(n, code);
        return code;
    }
    static void emit(ASTNode* n, vector<Ins>& code) {
        if (!n) return;
        if (n->leaf()) { code.push_back({PUSH, n->val}); return; }
        if (n->op == '~') { emit(n->l, code); code.push_back({NEG, 0}); return; }
        emit(n->l, code);
        emit(n->r, code);
        switch (n->op) {
            case '+': code.push_back({ADD, 0}); break;
            case '-': code.push_back({SUB, 0}); break;
            case '*': code.push_back({MUL, 0}); break;
            case '/': code.push_back({DIV, 0}); break;
            case '^': code.push_back({POW, 0}); break;
        }
    }
    static long long run(const vector<Ins>& code) {
        vector<long long> st;
        for (const Ins& ins : code) {
            if (ins.op == PUSH) { st.push_back(ins.arg); continue; }
            if (ins.op == NEG) { long long a = st.back(); st.pop_back(); st.push_back(-a); continue; }
            long long b = st.back(); st.pop_back();
            long long a = st.back(); st.pop_back();
            if (ins.op == ADD) st.push_back(a + b);
            else if (ins.op == SUB) st.push_back(a - b);
            else if (ins.op == MUL) st.push_back(a * b);
            else if (ins.op == DIV) st.push_back(a / b);
            else if (ins.op == POW) st.push_back(ipow(a, b));
        }
        return st.back();
    }
    static string to_string(const vector<Ins>& code) {
        string out;
        for (const Ins& ins : code) {
            if (ins.op == PUSH) out += "PUSH " + std::to_string(ins.arg) + " ";
            else {
                static const char* names[] = {"PUSH", "ADD", "SUB", "MUL", "DIV", "POW", "NEG"};
                out += string(names[ins.op]) + " ";
            }
        }
        return out;
    }
};

// --- C.6. Автоматическое дифференцирование (forward mode) ---
// Производная выражения в точке без построения производной: двойные
// числа (dual numbers) x + x'·ε. Каждая операция несёт свою производную
// через линейность (ε² = 0): (uv)' = u'v + uv', (u/v)' = (u'v − uv')/v²,
// (u^v)' = u^v·(v'·ln u + v·u'/u); функции — цепное правило. За один
// проход — значение и производная по выбранной переменной (арность
// узлов — из A.2: sin/cos/log/sqrt/abs).
struct AutoDiff {
    struct Dual { double v = 0, d = 0; };

    static Dual eval(FunctionAST* n, const map<string, double>& vars, const string& t) {
        if (!n) return {0, 0};
        if (n->kind == FunctionAST::NUM) return {(double)n->val, 0};
        if (n->kind == FunctionAST::VAR) {
            auto it = vars.find(n->name);
            double v = it == vars.end() ? 0.0 : it->second;
            return {v, n->name == t ? 1.0 : 0.0};
        }
        if (n->kind == FunctionAST::OP2) {
            Dual a = eval(n->args[0], vars, t);
            Dual b = eval(n->args[1], vars, t);
            if (n->op == "+") return {a.v + b.v, a.d + b.d};
            if (n->op == "-") return {a.v - b.v, a.d - b.d};
            if (n->op == "*") return {a.v * b.v, a.d * b.v + a.v * b.d};
            if (n->op == "/") return {a.v / b.v, (a.d * b.v - a.v * b.d) / (b.v * b.v)};
            if (n->op == "^") {
                double p = pow(a.v, b.v);
                double d = p * (b.d * log(a.v) + b.v * a.d / a.v);
                return {p, d};
            }
            return {0, 0};
        }
        Dual x = n->args.empty() ? Dual{0, 0} : eval(n->args[0], vars, t);
        if (n->name == "sin") return {sin(x.v), cos(x.v) * x.d};
        if (n->name == "cos") return {cos(x.v), -sin(x.v) * x.d};
        if (n->name == "log") return {log(x.v), x.d / x.v};
        if (n->name == "sqrt") return {sqrt(x.v), x.d / (2 * sqrt(x.v))};
        if (n->name == "abs") return {fabs(x.v), x.d * (x.v > 0 ? 1.0 : (x.v < 0 ? -1.0 : 0.0))};
        return {0, 0};
    }
};

// =============================================================
// D. ВАЛИДАЦИЯ И ПРОВЕРКА КОРРЕКТНОСТИ
// =============================================================

// --- D.1. Лексический анализ (токенизация) ---
// Lexer определён в начале класса (общий для всех парсеров): числа
// (многоразрядные), идентификаторы, операторы, скобки; неизвестный
// символ — токен BAD с позицией.

// --- D.2. Синтаксическая проверка ---
// Проверка корректности последовательности токенов: парность скобок
// (переиспользуется is_balanced из I.C), чередование операндов и
// операторов, отсутствие битых токенов и хвостовых операторов.
struct SyntaxValidator {
    static bool validate(const string& s, string* err = nullptr) {
        Lexer lex(s);
        if (lex.has_bad()) {
            for (const Token& t : lex.tokens)
                if (t.kind == Token::BAD) { if (err) *err = "неизвестный символ '" + t.lex + "' на позиции " + to_string(t.pos); return false; }
        }
        LinearStructures lin;                  // мост из I.C
        if (!lin.is_balanced(s)) { if (err) *err = "несбалансированные скобки"; return false; }
        bool expect_operand = true;
        bool any = false;
        for (const Token& t : lex.tokens) {
            if (t.kind == Token::NUM || t.kind == Token::VAR) {
                if (!expect_operand) { if (err) *err = "два операнда подряд"; return false; }
                expect_operand = false; any = true;
            }
            else if (t.kind == Token::LPAR) { expect_operand = true; }
            else if (t.kind == Token::RPAR) { expect_operand = false; }
            else if (t.kind == Token::OP) {
                if (expect_operand) {
                    if (t.lex != "-") { if (err) *err = "пропущен операнд перед '" + t.lex + "'"; return false; }
                } else {
                    expect_operand = true;
                }
            }
        }
        if (!any) { if (err) *err = "пустое выражение"; return false; }
        if (expect_operand) { if (err) *err = "выражение заканчивается оператором"; return false; }
        return true;
    }
};

// --- D.3. Проверка типов (type checking) ---
// Типы INT / FLOAT: арифметика над числами; '/' и переменные дают
// FLOAT; '^' требует целого показателя (иначе ERR). Инференция снизу
// вверх по дереву.
struct TypeChecker {
    enum Type { INT, FLOAT, ERR };
    static Type check(ASTNode* n) {
        if (!n) return ERR;
        if (n->leaf()) return n->is_var ? FLOAT : INT;
        if (n->op == '~') { Type a = check(n->l); return a == ERR ? ERR : a; }
        Type a = check(n->l), b = check(n->r);
        if (a == ERR || b == ERR) return ERR;
        if (a == FLOAT || b == FLOAT) {
            if (n->op == '^') return ERR;
            return FLOAT;
        }
        if (n->op == '/') return FLOAT;
        return INT;
    }
    static string name(Type t) {
        if (t == INT) return "int";
        if (t == FLOAT) return "float";
        return "err";
    }
};

// --- D.4. Обнаружение ошибок: деление на ноль, переполнение ---
// Константные проверки: делитель-константа 0; переполнение при
// свёртке констант — вычисление в __int128 с границами long long.
struct SafetyChecker {
    static bool const_div_by_zero(ASTNode* n) {
        if (!n) return false;
        if (n->op == '/' && n->r && n->r->leaf() && !n->r->is_var && n->r->val == 0) return true;
        return const_div_by_zero(n->l) || (n->r && const_div_by_zero(n->r));
    }
    static bool overflow_risk(ASTNode* n) {
        if (!n) return false;
        if (n->leaf()) return false;
        if (n->op == '~') return overflow_risk(n->l);
        if (n->l->leaf() && !n->l->is_var && n->r->leaf() && !n->r->is_var) {
            long long a = n->l->val, b = n->r->val;
            __int128 r = 0;
            if (n->op == '+') r = (__int128)a + b;
            else if (n->op == '-') r = (__int128)a - b;
            else if (n->op == '*') r = (__int128)a * b;
            else if (n->op == '^') {
                r = 1;
                for (long long k = 0; k < b; ++k) { r *= a; if (r < (__int128)LLONG_MIN || r > (__int128)LLONG_MAX) return true; }
                return false;
            }
            else return false;
            return r < (__int128)LLONG_MIN || r > (__int128)LLONG_MAX;
        }
        return overflow_risk(n->l) || overflow_risk(n->r);
    }
};

// =============================================================
// E. СПЕЦИАЛИЗИРОВАННЫЕ СТРУКТУРЫ ДЛЯ ВЫРАЖЕНИЙ
// =============================================================

// --- E.1. DAG выражений (общие подвыражения, CSE) ---
// Hash-consing: узел интернируется по (оператор, левый id, правый id)
// (листья — по значению/имени); одинаковые поддеревья разделяются.
// Число уникальных узлов падает — это common subexpression elimination.
struct ExprDAG {
    struct Node { char op; long long val; string name; int l, r; };
    vector<Node> nodes;
    unordered_map<string, int> memo;

    string key_leaf(ASTNode* n) const { return n->is_var ? "v" + n->name : "n" + to_string(n->val); }
    string key_op(char op, int l, int r) const { return string(1, op) + "/" + to_string(l) + "/" + to_string(r); }

    int build(ASTNode* n) {
        if (!n) return -1;
        if (n->leaf()) {
            string k = key_leaf(n);
            auto it = memo.find(k);
            if (it != memo.end()) return it->second;
            int id = (int)nodes.size();
            nodes.push_back(Node{0, n->val, n->name, -1, -1});
            memo[k] = id;
            return id;
        }
        if (n->op == '~') {
            int a = build(n->l);
            string k = "~/" + to_string(a);
            auto it = memo.find(k);
            if (it != memo.end()) return it->second;
            int id = (int)nodes.size();
            nodes.push_back(Node{'~', 0, "", a, -1});
            memo[k] = id;
            return id;
        }
        int l = build(n->l), r = build(n->r);
        string k = key_op(n->op, l, r);
        auto it = memo.find(k);
        if (it != memo.end()) return it->second;
        int id = (int)nodes.size();
        nodes.push_back(Node{n->op, 0, "", l, r});
        memo[k] = id;
        return id;
    }
};

// --- E.2. Трехадресный код (TAC) и SSA ---
// AST → линейная последовательность присваиваний t_i = a op b;
// каждый временной регистр присваивается ровно один раз (SSA-lite).
struct ThreeAddressCode {
    struct Ins { string op, arg1, arg2, res; };
    static string gen_expr(ASTNode* n, int& tmp, vector<Ins>& code) {
        if (!n) return "";
        if (n->leaf()) return n->is_var ? n->name : std::to_string(n->val);
        if (n->op == '~') {
            string a = gen_expr(n->l, tmp, code);
            string r = "t" + std::to_string(tmp++);
            code.push_back({"~", a, "", r});
            return r;
        }
        string a = gen_expr(n->l, tmp, code);
        string b = gen_expr(n->r, tmp, code);
        string r = "t" + std::to_string(tmp++);
        code.push_back({string(1, n->op), a, b, r});
        return r;
    }
    static vector<Ins> gen(ASTNode* n) {
        vector<Ins> code;
        int tmp = 0;
        gen_expr(n, tmp, code);
        return code;
    }
    static string to_string(const vector<Ins>& code) {
        string out;
        for (const Ins& ins : code) {
            if (ins.op == "~") out += ins.res + " = -" + ins.arg1 + "\n";
            else out += ins.res + " = " + ins.arg1 + " " + ins.op + " " + ins.arg2 + "\n";
        }
        return out;
    }
};

// --- E.3. Таблица символов (скоупы) ---
// Иерархическая таблица: стопка скоупов, объявление в текущем,
// поиск — от внутреннего к внешнему (затенение имён).
struct SymbolTable {
    struct Entry { string type; long long value; };
    vector<map<string, Entry>> scopes;

    SymbolTable() { scopes.emplace_back(); }
    void push_scope() { scopes.emplace_back(); }
    void pop_scope() { if (scopes.size() > 1) scopes.pop_back(); }
    void declare(const string& name, const string& type, long long value = 0) { scopes.back()[name] = {type, value}; }
    bool assign(const string& name, long long value) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto f = it->find(name);
            if (f != it->end()) { f->second.value = value; return true; }
        }
        return false;
    }
    bool lookup(const string& name, Entry* out = nullptr) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto f = it->find(name);
            if (f != it->end()) { if (out) *out = f->second; return true; }
        }
        return false;
    }
    int depth() const { return (int)scopes.size(); }
};

// =============================================================
// F. ПРИМЕНЕНИЯ
// =============================================================

// --- F.1. Конфигурационные языки (key = value) ---
// Парсер конфигов: строки "key = value", комментарии '#' и '//';
// значения — int / double / bool / строка в кавычках.
struct ConfigParser {
    struct Entry { string type; long long i = 0; double d = 0; string s; };
    map<string, Entry> cfg;

    static string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }
    bool parse(istream& in, string* err = nullptr) {
        string line;
        int ln = 0;
        while (getline(in, line)) {
            ++ln;
            size_t c1 = line.find('#');
            if (c1 != string::npos) line.erase(c1);
            size_t c2 = line.find("//");
            if (c2 != string::npos) line.erase(c2);
            size_t eq = line.find('=');
            if (eq == string::npos) { line = trim(line); if (line.empty()) continue; if (err) *err = "строка " + to_string(ln) + ": ожидается '='"; return false; }
            string key = trim(line.substr(0, eq));
            string val = trim(line.substr(eq + 1));
            if (key.empty()) { if (err) *err = "строка " + to_string(ln) + ": пустой ключ"; return false; }
            Entry e;
            if (val == "true" || val == "false") { e.type = "bool"; e.i = (val == "true"); }
            else if (val.size() >= 2 && val.front() == '"' && val.back() == '"') { e.type = "string"; e.s = val.substr(1, val.size() - 2); }
            else {
                try {
                    if (val.find('.') != string::npos) { e.type = "double"; e.d = stod(val); }
                    else { e.type = "int"; e.i = stoll(val); }
                } catch (...) {
                    if (err) *err = "строка " + to_string(ln) + ": некорректное значение '" + val + "'";
                    return false;
                }
            }
            cfg[key] = e;
        }
        return true;
    }
    const Entry* get(const string& key) const {
        auto it = cfg.find(key);
        return it == cfg.end() ? nullptr : &it->second;
    }
};

// --- F.2. Шаблонизаторы (подстановка {{var}}) ---
// Подстановка {{name}} из контекста; фильтры {{name|upper}},
// {{name|lower}} — первая ступень генерации текста по данным.
struct TemplateEngine {
    static string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }
    static string render(const string& tpl, const map<string, string>& ctx) {
        string out;
        size_t i = 0;
        while (i < tpl.size()) {
            size_t a = tpl.find("{{", i);
            if (a == string::npos) { out += tpl.substr(i); break; }
            out += tpl.substr(i, a - i);
            size_t b = tpl.find("}}", a + 2);
            if (b == string::npos) { out += tpl.substr(a); break; }
            string name = trim(tpl.substr(a + 2, b - a - 2));
            string filter;
            size_t bar = name.find('|');
            if (bar != string::npos) { filter = name.substr(bar + 1); name = name.substr(0, bar); }
            string v;
            auto it = ctx.find(name);
            if (it != ctx.end()) v = it->second;
            if (filter == "upper") transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return ::toupper(c); });
            else if (filter == "lower") transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return ::tolower(c); });
            out += v;
            i = b + 2;
        }
        return out;
    }
};

// --- F.3. Front-end компилятора и DSL (анализ) ---
// См. md F.3: лексика (D.1) → синтаксис (B) → семантика (D.3) →
// дерево (A) → трансформации (C) → вывод (C.5, E.2). Пайплайн из
// готовых частей демонстрируется в main (конфиг + шаблон).

};  // struct SyntaxStructures

#ifndef STRUCT_I_MAIN
#define STRUCT_I_MAIN

int main() {
    using H = SyntaxStructures;
    using ASTNode = H::ASTNode;
    using Token = H::Token;
    cout << "=== IX. СИНТАКСИЧЕСКИЕ СТРУКТУРЫ (ДЕРЕВЬЯ РАЗБОРА) ===" << endl;

    // ---------- Мосты из I.C: op_prec, infix_to_postfix, eval_postfix, is_balanced ----------
    {
        H lin;
        cout << "I.C op_prec('*') = " << lin.op_prec('*') << " (ожидаем 2)" << endl;
        cout << "I.C infix_to_postfix(\"2+3*4\") = " << lin.infix_to_postfix("2+3*4")
             << " (ожидаем 2 3 4 * + )" << endl;
        cout << "I.C eval_postfix(\"2 3 4 * +\") = " << lin.eval_postfix("2 3 4 * +")
             << " (ожидаем 14)" << endl;
        cout << "I.C is_balanced(\"(1+2)*(3-4)\") = " << lin.is_balanced("(1+2)*(3-4)")
             << " (ожидаем 1)" << endl;
    }

    // ---------- A.1 AST ----------
    {
        ASTNode* t = H::AST::from_postfix(H::ShuntingYard::to_postfix("2*3+4"));
        cout << "A.1 AST eval(\"2*3+4\") = " << H::AST::eval(t) << " (ожидаем 10)" << endl;
        cout << "A.1 AST to_infix = " << H::AST::to_infix(t) << " (ожидаем 2*3+4)" << endl;
        cout << "A.1 AST to_postfix = " << H::AST::to_postfix(t) << " (ожидаем 2 3 * 4 +)" << endl;
        cout << "A.1 AST size/depth = " << H::AST::size(t) << "/" << H::AST::depth(t)
             << " (ожидаем 5/3)" << endl;
        ASTNode* p = H::AST::from_postfix(H::ShuntingYard::to_postfix("(2+3)*4"));
        cout << "A.1 AST eval(\"(2+3)*4\") = " << H::AST::eval(p) << ", infix = " << H::AST::to_infix(p)
             << " (ожидаем 20 / (2+3)*4)" << endl;
        ASTNode* pw = H::AST::from_postfix(H::ShuntingYard::to_postfix("2^3^2"));
        cout << "A.1 AST eval(\"2^3^2\") = " << H::AST::eval(pw) << " (ожидаем 512, правоассоциативно)" << endl;
    }

    // ---------- A.2 FunctionAST (n-арное, функции, переменные) ----------
    {
        auto x = H::FunctionAST::var("x");
        auto e = H::FunctionAST::op2("+", x, new H::FunctionAST(1));
        cout << "A.2 eval(x+1, x=4) = " << H::FunctionAST::eval(e, {{"x", 4.0}})
             << " (ожидаем 5)" << endl;
        auto s = H::FunctionAST::fun("sin", {x});
        cout << "A.2 eval(sin(x), x=0) = " << H::FunctionAST::eval(s, {{"x", 0.0}})
             << " (ожидаем 0)" << endl;
        auto r = H::FunctionAST::fun("sqrt", {new H::FunctionAST(16)});
        cout << "A.2 eval(sqrt(16)) = " << H::FunctionAST::eval(r, {})
             << " (ожидаем 4)" << endl;
        cout << "A.2 to_string(x+1) = " << H::FunctionAST::to_string(e)
             << " (ожидаем (x + 1))" << endl;
    }

    // ---------- A.3 Parse Tree ----------
    {
        H::ParseTree* root = H::ParseTree::build("2+3*4");
        string d = H::ParseTree::dump(root);
        cout << "A.3 dump contains EXPR/TERM/FACTOR/operators = "
             << (d.find("EXPR") != string::npos && d.find("TERM") != string::npos &&
                 d.find("FACTOR") != string::npos && d.find("2") != string::npos &&
                 d.find("+") != string::npos && d.find("*") != string::npos)
             << " (ожидаем 1)" << endl;
        cout << "A.3 canonical to_ast eval = " << H::AST::eval(H::ParseTree::to_ast(root))
             << " (ожидаем 14)" << endl;
    }

    // ---------- A.4 Expression Tree ----------
    {
        H::ExpressionTree et;
        et.build(H::ShuntingYard::to_postfix("(2+3)*4"));
        cout << "A.4 eval = " << et.eval() << ", infix = " << et.to_infix()
             << ", postfix = " << et.to_postfix() << " (ожидаем 20 / (2+3)*4 / 2 3 + 4 *)" << endl;
        cout << "A.4 size/depth = " << et.size() << "/" << et.depth()
             << " (ожидаем 5/3)" << endl;
    }

    // ---------- B.1 Recursive Descent ----------
    {
        H::Lexer lx("2+3*4");
        H::RecursiveDescent rd(lx.tokens);
        cout << "B.1 RD eval(\"2+3*4\") = " << H::AST::eval(rd.parse())
             << " (ожидаем 14)" << endl;
        H::Lexer lx2("-2+3");
        H::RecursiveDescent rd2(lx2.tokens);
        cout << "B.1 RD eval(\"-2+3\") = " << H::AST::eval(rd2.parse())
             << " (ожидаем 1)" << endl;
        H::Lexer lx3("(2+3)*4");
        H::RecursiveDescent rd3(lx3.tokens);
        cout << "B.1 RD eval(\"(2+3)*4\") = " << H::AST::eval(rd3.parse())
             << " (ожидаем 20)" << endl;
        H::Lexer lx4("x*x+2*x+1");
        H::RecursiveDescent rd4(lx4.tokens);
        cout << "B.1 RD eval(x*x+2*x+1, x=3) = " << H::AST::eval(rd4.parse(), {{"x", 3}})
             << " (ожидаем 16)" << endl;
    }

    // ---------- B.2 Shunting Yard ----------
    {
        vector<Token> pf = H::ShuntingYard::to_postfix("2+3*4");
        cout << "B.2 SY(\"2+3*4\") = " << H::ShuntingYard::to_string(pf)
             << " (ожидаем 2 3 4 * + )" << endl;
        vector<Token> pw = H::ShuntingYard::to_postfix("2^3^2");
        cout << "B.2 SY(\"2^3^2\") = " << H::ShuntingYard::to_string(pw)
             << " (ожидаем 2 3 2 ^ ^, правоассоциативно)" << endl;
        cout << "B.2 SY eval(\"(2+3)*4\") = "
             << H::AST::eval(H::AST::from_postfix(H::ShuntingYard::to_postfix("(2+3)*4")))
             << " (ожидаем 20)" << endl;
    }

    // ---------- B.3 Pratt ----------
    {
        auto pratt = [](const string& s) {
            H::Lexer lx(s);
            H::PrattParser pp(lx.tokens);
            return H::AST::eval(pp.parse());
        };
        cout << "B.3 Pratt(\"2+3*4\") = " << pratt("2+3*4")
             << " (ожидаем 14)" << endl;
        cout << "B.3 Pratt(\"(2+3)*4\") = " << pratt("(2+3)*4")
             << " (ожидаем 20)" << endl;
        cout << "B.3 Pratt(\"2^3^2\") = " << pratt("2^3^2")
             << " (ожидаем 512, правоассоциативно)" << endl;
        cout << "B.3 Pratt(\"-2+3\") = " << pratt("-2+3")
             << " (ожидаем 1)" << endl;
    }

    // ---------- B.4 Shift-Reduce ----------
    {
        cout << "B.4 SR(\"2+3*4\") = " << H::ShiftReduceParser::parse("2+3*4")
             << " (ожидаем 14)" << endl;
        cout << "B.4 SR(\"(2+3)*4\") = " << H::ShiftReduceParser::parse("(2+3)*4")
             << " (ожидаем 20)" << endl;
        cout << "B.4 SR(\"2^3^2\") = " << H::ShiftReduceParser::parse("2^3^2")
             << " (ожидаем 512)" << endl;
        cout << "B.4 SR(\"-2+3\") = " << H::ShiftReduceParser::parse("-2+3")
             << " (ожидаем 1)" << endl;
    }

    // ---------- B.5 LL(1) ----------
    {
        cout << "B.5 LL1(\"1+2*3\") = " << H::LL1Parser::parse("1+2*3")
             << " (ожидаем 1)" << endl;
        cout << "B.5 LL1(\"(1+2)*3\") = " << H::LL1Parser::parse("(1+2)*3")
             << " (ожидаем 1)" << endl;
        cout << "B.5 LL1(\"1-2\") = " << H::LL1Parser::parse("1-2")
             << " (ожидаем 0: '-' нет в грамматике)" << endl;
    }

    // ---------- C.1 Visitor ----------
    {
        ASTNode* t = H::AST::from_postfix(H::ShuntingYard::to_postfix("2*3+4"));
        cout << "C.1 visitor pre-order = " << H::TreeVisitor::collect(t)
             << " (ожидаем + * 2 3 4 )" << endl;
        cout << "C.1 visitor leaf sum = " << H::TreeVisitor::leaf_sum(t)
             << " (ожидаем 9)" << endl;
    }

    // ---------- C.2 Interpreter ----------
    {
        map<string, long long> env;
        bool assigned = false;
        long long v1 = H::Interpreter::exec_line("x = 2+3*4", env, &assigned);
        long long v2 = H::Interpreter::exec_line("x*2", env);
        cout << "C.2 assign x = 2+3*4 -> " << v1 << ", then x*2 = " << v2
             << " (ожидаем 14 / 28)" << endl;
    }

    // ---------- C.3 Differentiator ----------
    {
        H::Lexer lx("x*x+2*x+1");
        H::RecursiveDescent rd(lx.tokens);
        ASTNode* d = H::Differentiator::diff(rd.parse());
        cout << "C.3 d/dx(x*x+2*x+1)@x=3 = " << H::AST::eval(d, {{"x", 3}})
             << " (ожидаем 8)" << endl;
        H::Lexer lx2("x");
        H::RecursiveDescent rd2(lx2.tokens);
        cout << "C.3 d/dx(x)@x=5 = " << H::AST::eval(H::Differentiator::diff(rd2.parse()), {{"x", 5}})
             << " (ожидаем 1)" << endl;
    }

    // ---------- C.4 Simplifier ----------
    {
        H::Lexer lx("2*3*x+0");
        H::RecursiveDescent rd(lx.tokens);
        ASTNode* s = H::Simplifier::simplify(rd.parse());
        cout << "C.4 simplify(\"2*3*x+0\") = " << H::AST::to_infix(s)
             << " (ожидаем 6*x)" << endl;
        cout << "C.4 eval simpl/orig @x=5 = " << H::AST::eval(s, {{"x", 5}})
             << "/" << H::AST::eval(H::AST::from_postfix(H::ShuntingYard::to_postfix("2*3*x+0")), {{"x", 5}})
             << " (ожидаем 30/30)" << endl;
    }

    // ---------- C.5 Bytecode ----------
    {
        vector<H::BytecodeCompiler::Ins> code =
            H::BytecodeCompiler::compile(H::AST::from_postfix(H::ShuntingYard::to_postfix("2*3+4")));
        cout << "C.5 bytecode = " << H::BytecodeCompiler::to_string(code)
             << " (ожидаем PUSH 2 PUSH 3 MUL PUSH 4 ADD )" << endl;
        cout << "C.5 run = " << H::BytecodeCompiler::run(code) << " (ожидаем 10)" << endl;
        vector<H::BytecodeCompiler::Ins> code2 =
            H::BytecodeCompiler::compile(H::AST::from_postfix(H::ShuntingYard::to_postfix("2^10")));
        cout << "C.5 run(2^10) = " << H::BytecodeCompiler::run(code2)
             << " (ожидаем 1024)" << endl;
    }

    // ---------- C.6 AutoDiff (forward mode) ----------
    {
        auto x = H::FunctionAST::var("x");
        auto e = H::FunctionAST::op2("+", H::FunctionAST::op2("*", x, x),
                                     H::FunctionAST::op2("*", new H::FunctionAST(2), x));
        H::AutoDiff::Dual r = H::AutoDiff::eval(e, {{"x", 3.0}}, "x");
        cout << "C.6 AD(x^2+2x)@x=3 = " << r.v << " / d = " << r.d
             << " (ожидаем 15 / 8)" << endl;
        auto s = H::FunctionAST::fun("sin", {H::FunctionAST::op2("*", x, x)});
        H::AutoDiff::Dual r2 = H::AutoDiff::eval(s, {{"x", 1.0}}, "x");
        cout << "C.6 AD(sin(x^2))@x=1 = " << r2.v << " / d = " << r2.d
             << " (ожидаем " << sin(1.0) << " / " << 2 * cos(1.0) << ")" << endl;
        auto p = H::FunctionAST::fun("sqrt", {x});
        H::AutoDiff::Dual r3 = H::AutoDiff::eval(p, {{"x", 4.0}}, "x");
        cout << "C.6 AD(sqrt(x))@x=4 = " << r3.v << " / d = " << r3.d
             << " (ожидаем 2 / 0.25)" << endl;
    }

    // ---------- D.1 Lexer ----------
    {
        H::Lexer lx("12 + x*(3)");
        bool kinds_ok = lx.tokens[0].kind == H::Token::NUM && lx.tokens[0].val == 12 &&
                        lx.tokens[1].kind == H::Token::OP && lx.tokens[2].kind == H::Token::VAR &&
                        lx.tokens[3].kind == H::Token::OP && lx.tokens[4].kind == H::Token::LPAR &&
                        lx.tokens[5].kind == H::Token::NUM && lx.tokens[5].val == 3 &&
                        lx.tokens[6].kind == H::Token::RPAR && lx.tokens[7].kind == H::Token::END;
        cout << "D.1 lexer(\"12 + x*(3)\") tokens/END = " << lx.tokens.size() << "/" << kinds_ok
             << " (ожидаем 8/1)" << endl;
        H::Lexer bad("2@3");
        cout << "D.1 lexer(\"2@3\") bad token = " << bad.has_bad()
             << " (ожидаем 1)" << endl;
    }

    // ---------- D.2 SyntaxValidator ----------
    {
        cout << "D.2 validate(\"2+3*4\") = " << H::SyntaxValidator::validate("2+3*4")
             << " (ожидаем 1)" << endl;
        cout << "D.2 validate(\"(2+3\") = " << H::SyntaxValidator::validate("(2+3")
             << " (ожидаем 0)" << endl;
        cout << "D.2 validate(\"2+*3\") = " << H::SyntaxValidator::validate("2+*3")
             << " (ожидаем 0)" << endl;
        cout << "D.2 validate(\"2+\") = " << H::SyntaxValidator::validate("2+")
             << " (ожидаем 0)" << endl;
        cout << "D.2 validate(\"2+@\") = " << H::SyntaxValidator::validate("2+@")
             << " (ожидаем 0)" << endl;
    }

    // ---------- D.3 TypeChecker ----------
    {
        auto check = [](const string& s) {
            return H::TypeChecker::name(H::TypeChecker::check(
                H::AST::from_postfix(H::ShuntingYard::to_postfix(s))));
        };
        cout << "D.3 type(\"1+2\") = " << check("1+2") << " (ожидаем int)" << endl;
        cout << "D.3 type(\"1/2\") = " << check("1/2") << " (ожидаем float)" << endl;
        cout << "D.3 type(\"2^3\") = " << check("2^3") << " (ожидаем int)" << endl;
        cout << "D.3 type(\"x^2\") = " << check("x^2") << " (ожидаем err)" << endl;
    }

    // ---------- D.4 SafetyChecker ----------
    {
        H::Lexer lx("1/0");
        H::RecursiveDescent rd(lx.tokens);
        ASTNode* t1 = rd.parse();
        H::Lexer lx2("10000000000*10000000000");
        H::RecursiveDescent rd2(lx2.tokens);
        ASTNode* t2 = rd2.parse();
        H::Lexer lx3("2*3");
        H::RecursiveDescent rd3(lx3.tokens);
        ASTNode* t3 = rd3.parse();
        cout << "D.4 div by zero(\"1/0\") = " << H::SafetyChecker::const_div_by_zero(t1)
             << " (ожидаем 1)" << endl;
        cout << "D.4 overflow(\"10000000000*10000000000\") = "
             << H::SafetyChecker::overflow_risk(t2) << " (ожидаем 1)" << endl;
        cout << "D.4 overflow(\"2*3\") = " << H::SafetyChecker::overflow_risk(t3)
             << " (ожидаем 0)" << endl;
    }

    // ---------- E.1 ExprDAG (CSE) ----------
    {
        H::Lexer lx("a*b + a*b");
        H::RecursiveDescent rd(lx.tokens);
        H::ExprDAG dag;
        dag.build(rd.parse());
        cout << "E.1 DAG nodes for \"a*b + a*b\" = " << dag.nodes.size()
             << " (ожидаем 4: a, b, *, + — общее подвыражение разделено)" << endl;
    }

    // ---------- E.2 Three-Address Code ----------
    {
        vector<H::ThreeAddressCode::Ins> code =
            H::ThreeAddressCode::gen(H::AST::from_postfix(H::ShuntingYard::to_postfix("a+b*c-(d/e)")));
        cout << "E.2 TAC lines = " << code.size() << ", last = " << code.back().res
             << " (ожидаем 4 / t3)" << endl;
        cout << "E.2 TAC text:\n" << H::ThreeAddressCode::to_string(code);
    }

    // ---------- E.3 SymbolTable ----------
    {
        H::SymbolTable st;
        st.declare("x", "int", 1);
        st.push_scope();
        st.declare("x", "int", 2);
        H::SymbolTable::Entry e;
        st.lookup("x", &e);
        bool inner = (e.value == 2);
        st.pop_scope();
        st.lookup("x", &e);
        bool outer = (e.value == 1);
        cout << "E.3 shadow: inner/outer = " << inner << "/" << outer
             << " (ожидаем 1/1)" << endl;
    }

    // ---------- F.1 ConfigParser ----------
    {
        istringstream ss("port = 8080\nhost = \"localhost\"\n# comment\ndebug = true\nratio = 3.14\n");
        H::ConfigParser cp;
        bool ok = cp.parse(ss);
        const H::ConfigParser::Entry* port = cp.get("port");
        const H::ConfigParser::Entry* host = cp.get("host");
        const H::ConfigParser::Entry* debug = cp.get("debug");
        const H::ConfigParser::Entry* ratio = cp.get("ratio");
        bool good = ok && port && port->type == "int" && port->i == 8080 &&
                    host && host->type == "string" && host->s == "localhost" &&
                    debug && debug->type == "bool" && debug->i == 1 &&
                    ratio && ratio->type == "double" && (int)(ratio->d * 100) == 314;
        cout << "F.1 config parse types/values = " << good << " (ожидаем 1)" << endl;
        istringstream bad("no-equals-here");
        H::ConfigParser cp2;
        string err;
        bool ok2 = cp2.parse(bad, &err);
        cout << "F.1 bad line rejected = " << (!ok2 && !err.empty()) << " (ожидаем 1)" << endl;
    }

    // ---------- F.2 TemplateEngine ----------
    {
        string out = H::TemplateEngine::render("Hello {{name}}, from {{city|upper}}!",
                                               {{"name", "Anna"}, {"city", "moscow"}});
        cout << "F.2 render = " << out << " (ожидаем Hello Anna, from MOSCOW!)" << endl;
        cout << "F.2 missing var = " << H::TemplateEngine::render("[{{nope}}]", {})
             << " (ожидаем [])" << endl;
    }

    cout << "\nAll tests passed!" << endl;
    return 0;
}
#endif // STRUCT_I_MAIN

#endif // STRUCT_I_CPP
