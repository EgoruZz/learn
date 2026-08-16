#ifndef DISCRETE_LOGIC_D_CPP
#define DISCRETE_LOGIC_D_CPP

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <random>
using namespace std;

#define FORMAL_LOGIC_MAIN
#include "../b-folder/b.cpp"

// =============================================================
// D. АВТОМАТЫ И ФОРМАЛЬНЫЕ СИСТЕМЫ
// =============================================================
// Структура md: A. Одномерные клеточные автоматы (1. Понятие
//               клеточного автомата, 2. Элементарные КА: правила
//               0..255, 3. Классы поведения Вольфрама, 4.
//               Универсальность: правило 110) → B. Двумерные
//               (1. Игра «Жизнь»: B/S-правила, фигуры,
//               универсальность, 2. Мир «Ва-Тор»: рыбы и акулы,
//               динамика популяций, 3. Муравей Лэнгтона: правило,
//               шоссе) → C. Модели транспортных потоков
//               (1. Модель Нагеля-Шрекенберга: 4 правила шага,
//               пробки)
//
// Наследует FormalLogic (b.cpp): клеточный автомат — формальная
// система в динамике (b.md C.1), эволюция — вычисление (b.md 9.1);
// универсальные автоматы — «компьютеры» (b.md 9.2). Локальные
// функции элементарных КА — булевы функции от 3 переменных:
// правило — номер функции (c.cpp A.1). Методы обобщены: констант
// нет, кроме границ представления (правила — аргументы: B/S-наборы,
// параметры Ва-Тора и Нагеля-Шрекенберга; случайность — через
// генератор с seed).
//
// Соглашение о представлении:
//   * элементарный КА — правило 0..255 (номер булевой функции
//     от 3 переменных), строка из '0'/'1', границы — нули;
//   * «Жизнь» — сетка int 0/1, окрестность Мура (8 соседей),
//     правила B/S — множества чисел соседей (аргументы);
//   * «Ва-Тор» — сетка WatorCell: вид 0/1/2 (вода/рыба/акула),
//     возраст (до размножения), голод (для акул: смерть при
//     shark_starve, еда обнуляет); шаг обрабатывает клетки в
//     случайном порядке (через rng);
//   * муравей Лэнгтона — сетка цветов int, правило — вектор
//     (поворот: −1 налево, 0 прямо, +1 направо; следующий цвет);
//     направления 0..3: N, E, S, W; сетка растёт при выходе за
//     края, координаты — мировые (могут быть отрицательными);
//   * дорога Нагеля-Шрекенберга — кольцевой вектор скоростей
//     −1 (пусто) или 0..v_max (скорость машины).
//
// Содержит:
//   A. Одномерные: elementary_step, elementary_run,
//      elementary_print, evolution_period
//   B. Двумерные: life_step, life_print, WatorCell, wator_step,
//      wator_counts, wator_run, ant_step, ant_run
//   C. Транспорт: nagel_step, nagel_avg_speed

struct CellularAutomata : FormalLogic {

// =============================================================
// A. ОДНОМЕРНЫЕ КЛЕТОЧНЫЕ АВТОМАТЫ
// =============================================================

// --- A.2.1. Шаг элементарного КА ---
// Правило — номер булевой функции от 3 переменных (c.cpp A.1):
// бит n правила — значение на наборе из трёх соседей (левый,
// клетка, правый); next[i] = f(строка[i−1..i+1]), границы — нули.
string elementary_step(int rule, const string& line) {
    string next(line.size(), '0');
    for (int i = 0; i < (int)line.size(); i++) {
        int mask = 0;
        if (i > 0 && line[i - 1] == '1') mask |= 4;
        if (line[i] == '1') mask |= 2;
        if (i + 1 < (int)line.size() && line[i + 1] == '1') mask |= 1;
        next[i] = ((rule >> mask) & 1) ? '1' : '0';
    }
    return next;
}

// --- A.2.2. Несколько поколений подряд ---
// Строка растёт на один ноль с каждой стороны за поколение —
// «море нулей» вокруг (границы считаются нулями, A.2.1): из одной 1
// вырастает треугольник (для правила 90 — треугольник Серпинского).
vector<string> elementary_run(int rule, const string& line,
                              int generations) {
    vector<string> gens;
    string cur = line;
    for (int g = 0; g <= generations; g++) {
        gens.push_back(cur);
        cur = "0" + cur + "0";
        cur = elementary_step(rule, cur);
    }
    return gens;
}

// --- A.2.2. Печать эволюции треугольником ---
void elementary_print(const vector<string>& gens) {
    for (auto& s : gens) cout << s << endl;
}

// --- A.3.1. Период стабилизации эволюции ---
// Первое повторение строки: 1 — фиксированная точка (класс 1),
// k > 1 — периодическое поведение (класс 2), 0 — не
// стабилизировалась за generations (классы 3/4) — машинная
// характеристика класса Вольфрама.
int evolution_period(int rule, const string& line, int generations) {
    vector<string> seen;
    string cur = line;
    for (int g = 0; g < generations; g++) {
        for (int i = 0; i < (int)seen.size(); i++)
            if (seen[i] == cur) return (int)seen.size() - i;
        seen.push_back(cur);
        cur = elementary_step(rule, cur);
    }
    return 0;
}

// =============================================================
// B. ДВУМЕРНЫЕ КЛЕТОЧНЫЕ АВТОМАТЫ
// =============================================================

// --- B.1.1. Шаг «Жизни» с произвольными правилами B/S ---
// Окрестность Мура (8 соседей), синхронное обновление. Правила —
// множества числа живых соседей: живая клетка выживает, если
// число ∈ survival; мёртвая рождается, если ∈ birth.
// «Жизнь» = life_step(grid, {3}, {2, 3}). wrap — кольцевая сетка
// (планер бегает по тору), иначе края считаются мёртвыми.
vector<vector<int>> life_step(const vector<vector<int>>& grid,
                              const vector<int>& birth,
                              const vector<int>& survival, bool wrap) {
    int h = grid.size(), w = grid[0].size();
    vector<vector<int>> next(h, vector<int>(w, 0));
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int n = 0;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++) {
                    if (dy == 0 && dx == 0) continue;
                    int ny = y + dy, nx = x + dx;
                    if (wrap) {
                        ny = (ny + h) % h;
                        nx = (nx + w) % w;
                    } else if (ny < 0 || ny >= h || nx < 0 || nx >= w) {
                        continue;
                    }
                    n += grid[ny][nx];
                }
            bool alive = grid[y][x];
            bool live =
                alive ? find(survival.begin(), survival.end(), n) != survival.end()
                      : find(birth.begin(), birth.end(), n) != birth.end();
            next[y][x] = live ? 1 : 0;
        }
    return next;
}

// --- B.1.1. Печать сетки (живые — '#') ---
void life_print(const vector<vector<int>>& grid) {
    for (auto& row : grid) {
        for (int v : row) cout << (v ? '#' : '.');
        cout << endl;
    }
}

// --- B.2.1. Мир «Ва-Тор»: клетка ---
// kind: 0 — вода, 1 — рыба, 2 — акула; age — возраст (клетка
// размножается при age == порога размножения, потом сбрасывает);
// hunger — голод акулы (0 — сытая, смерть при hunger == shark_starve).
struct WatorCell {
    int kind;
    int age;
    int hunger;
};

// --- B.2.1. Шаг мира «Ва-Тор» ---
// Синхронный шаг с клетками в случайном порядке (без направленного
// смещения). Рыбы: движение в случайную соседнюю воду; при
// age == fish_breed — потомок остаётся на месте, родитель уходит
// (age = 0); без свободного соседа ждут (age копится до порога).
// Акулы: голод растёт каждый шаг; при hunger == shark_starve —
// смерть; иначе едят соседнюю рыбу (съедают, hunger = 0), иначе
// двигаются в случайную воду; при age == shark_breed — потомок
// (hunger = 0) на старом месте, родитель уходит (age = 0).
vector<vector<WatorCell>> wator_step(const vector<vector<WatorCell>>& world,
                                     int fish_breed, int shark_breed,
                                     int shark_starve, mt19937& rng) {
    int h = world.size(), w = world[0].size();
    vector<vector<WatorCell>> next(h, vector<WatorCell>(w));
    vector<pair<int, int>> cells;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            if (world[y][x].kind) cells.push_back({y, x});
    shuffle(cells.begin(), cells.end(), rng);
    auto free_neighbors = [&](int y, int x, int kind) {
        vector<pair<int, int>> res;
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                if (dy == 0 && dx == 0) continue;
                int ny = y + dy, nx = x + dx;
                if (ny < 0 || ny >= h || nx < 0 || nx >= w) continue;
                if (world[ny][nx].kind == kind) res.push_back({ny, nx});
            }
        return res;
    };
    for (auto [y, x] : cells) {
        WatorCell c = world[y][x];
        if (c.kind == 1) { // рыба
            vector<pair<int, int>> water = free_neighbors(y, x, 0);
            if (water.empty()) {
                next[y][x] = {1, min(c.age + 1, fish_breed), 0};
                continue;
            }
            pair<int, int> to = water[uniform_int_distribution<int>(0,
                (int)water.size() - 1)(rng)];
            if (c.age >= fish_breed) { // размножение: потомок на месте
                next[y][x] = {1, 0, 0};
                next[to.first][to.second] = {1, 0, 0}; // родитель уходит
            } else {
                next[to.first][to.second] = {1, min(c.age + 1, fish_breed), 0};
            }
        } else { // акула
            if (c.hunger + 1 >= shark_starve) continue;      // смерть
            WatorCell moved = {2, c.age + 1, c.hunger + 1};
            vector<pair<int, int>> fish = free_neighbors(y, x, 1);
            pair<int, int> to;
            bool moved_to = false;
            if (!fish.empty()) { // ест рыбу
                to = fish[uniform_int_distribution<int>(0,
                    (int)fish.size() - 1)(rng)];
                moved.hunger = 0;
                moved_to = true;
            } else {
                vector<pair<int, int>> water = free_neighbors(y, x, 0);
                if (!water.empty()) {
                    to = water[uniform_int_distribution<int>(0,
                        (int)water.size() - 1)(rng)];
                    moved_to = true;
                }
            }
            if (moved.age >= shark_breed)
                next[y][x] = {2, 0, 0};                       // потомок
            if (moved_to) next[to.first][to.second] = moved;
            else next[y][x] = moved;
        }
    }
    return next;
}

// --- B.2.2. Численности популяций: (рыбы, акулы) ---
pair<int, int> wator_counts(const vector<vector<WatorCell>>& world) {
    int fish = 0, sharks = 0;
    for (auto& row : world)
        for (auto& c : row) {
            fish += (c.kind == 1);
            sharks += (c.kind == 2);
        }
    return {fish, sharks};
}

// --- B.2.2. Динамика популяций по шагам ---
// Возвращает численности (рыбы, акулы) после каждого шага — циклы
// «хищник–жертва» (Лотки-Вольтерра) видны как колебания.
vector<pair<int, int>> wator_run(const vector<vector<WatorCell>>& start,
                                 int steps, int fish_breed, int shark_breed,
                                 int shark_starve, mt19937& rng) {
    vector<pair<int, int>> history;
    vector<vector<WatorCell>> world = start;
    for (int s = 0; s < steps; s++) {
        world = wator_step(world, fish_breed, shark_breed, shark_starve, rng);
        history.push_back(wator_counts(world));
    }
    return history;
}

// --- B.3.1. Муравей Лэнгтона: направления (N, E, S, W) ---
static constexpr int DX[4] = {0, 1, 0, -1};
static constexpr int DY[4] = {-1, 0, 1, 0};

// --- B.3.1. Шаг муравья ---
// По цвету клетки — поворот (rule[c].first: −1 налево, 0 прямо,
// +1 направо), перекраска (rule[c].second — следующий цвет),
// шаг вперёд. Классика: rule = {{+1, 1}, {−1, 0}}.
void ant_step(vector<vector<int>>& grid, int& x, int& y, int& dir,
              const vector<pair<int, int>>& rule) {
    int c = grid[y][x];
    dir = (dir + rule[c].first + 4) % 4;
    grid[y][x] = rule[c].second;
    x += DX[dir];
    y += DY[dir];
}

// --- B.3.1. Выполнение муравья: траектория с ростом сетки ---
// (x, y) муравья внутри ant_run — индексы сетки (0 ≤ x < cols,
// 0 ≤ y < rows): ant_step индексирует grid[y][x] корректно всегда.
// Сетка растёт при выходе за края; мировые координаты = индексы +
// сдвиг начала (ox, oy) — их ant_run возвращает в path и AntResult.
struct AntResult {
    vector<pair<int, int>> path;
    int x, y, dir;
};

AntResult ant_run(vector<vector<int>>& grid, int x, int y, int dir,
                  const vector<pair<int, int>>& rule, int steps) {
    vector<pair<int, int>> path;
    int ox = 0, oy = 0; // мировые координаты клетки grid[0][0]
    for (int s = 0; s < steps; s++) {
        path.push_back({x + ox, y + oy});
        ant_step(grid, x, y, dir, rule);
        if (y < 0) {                        // выше верхнего края
            grid.insert(grid.begin(), vector<int>(grid[0].size(), 0));
            y++;
            oy--;
        } else if (y >= (int)grid.size())
            grid.push_back(vector<int>(grid[0].size(), 0));
        if (x < 0) {                        // левее левого края
            for (auto& row : grid) row.insert(row.begin(), 0);
            x++;
            ox--;
        } else if (x >= (int)grid[0].size())
            for (auto& row : grid) row.push_back(0);
    }
    path.push_back({x + ox, y + oy});
    return {path, x + ox, y + oy, dir};
}

// =============================================================
// C. МОДЕЛИ ТРАНСПОРТНЫХ ПОТОКОВ
// =============================================================

// --- C.1.1. Шаг модели Нагеля-Шрекенберга ---
// Кольцевая дорога — вектор скоростей (−1 пусто, 0..vmax машина).
// Синхронный шаг: разгон (1), торможение до зазора g (2),
// случайное сбрёс скорости с вероятностью p (3), движение (4).
// Торможение гарантирует v ≤ g — машины не сталкиваются.
vector<int> nagel_step(const vector<int>& road, int vmax, double p,
                       mt19937& rng) {
    int n = road.size();
    vector<int> next(n, -1);
    for (int i = 0; i < n; i++) {
        if (road[i] < 0) continue;
        int gap = 0;
        while (gap < n && road[(i + gap + 1) % n] < 0) gap++;
        int v = road[i];
        v = min(v + 1, vmax);
        v = min(v, gap);
        if (uniform_real_distribution<double>(0.0, 1.0)(rng) < p)
            v = max(v - 1, 0);
        next[(i + v) % n] = v;
    }
    return next;
}

// --- C.1.1. Средняя скорость по шагам ---
// Сумма скоростей всех машин по всем шагам / (шаги · машины);
// при p = 0 и низкой плотности — vmax (максимальный поток).
double nagel_avg_speed(const vector<int>& road, int vmax, double p,
                       mt19937& rng, int steps) {
    vector<int> cur = road;
    double total = 0.0;
    int cars = 0;
    for (int s = 0; s < steps; s++) {
        cur = nagel_step(cur, vmax, p, rng);
        for (int v : cur)
            if (v >= 0) {
                total += v;
                cars++;
            }
    }
    return cars ? total / cars : 0.0;
}

}; // struct CellularAutomata

#ifndef CELLULAR_AUTOMATA_MAIN

// --- Тесты ---
signed main() {
    CellularAutomata C;

    cout << "=== A.2. Элементарные КА ===" << endl;
    // правило 90: next = left ⊕ right (треугольник Паскаля mod 2)
    cout << "rule 90: 0001000 → " << C.elementary_step(90, "0001000")
         << " (ожидаем 0010100)" << endl;
    cout << "Эволюция правила 90 из одной 1 (8 поколений):" << endl;
    C.elementary_print(C.elementary_run(90, "1", 8));
    cout << "Правило 0: период стабилизации = "
         << C.evolution_period(0, "0001000", 8) << " (ожидаем 1)" << endl;
    cout << "Правило 30 (хаос): период за 12 поколений = "
         << C.evolution_period(30, "1011010111", 12) << " (ожидаем 0)"
         << endl;

    cout << "\n=== A.4. Правило 110: универсальность ===" << endl;
    cout << "Эволюция правила 110 из случайной строки (12 поколений):"
         << endl;
    C.elementary_print(C.elementary_run(110, "1011010111", 12));

    cout << "\n=== B.1. Игра «Жизнь» ===" << endl;
    vector<int> birth = {3}, survival = {2, 3};
    // блок 2×2 устойчив
    vector<vector<int>> block = {
        {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}};
    auto block1 = C.life_step(block, birth, survival, false);
    cout << "блок устойчив: " << (block1 == block) << " (ожидаем 1)" << endl;
    // мигалка: горизонтальная ⟷ вертикальная (период 2)
    vector<vector<int>> blink = {
        {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
    auto b1 = C.life_step(blink, birth, survival, false);
    auto b2 = C.life_step(b1, birth, survival, false);
    cout << "мигалка периодична (период 2): " << (b1 != blink && b2 == blink)
         << " (ожидаем 1)" << endl;
    // планер: за 4 шага сдвинут на (1, 1) (кольцевая сетка 8×8)
    vector<vector<int>> glider(8, vector<int>(8, 0));
    glider[1][1] = 1; glider[2][2] = 1; glider[0][3] = 1;
    glider[1][3] = 1; glider[2][3] = 1;
    auto g = glider;
    for (int i = 0; i < 4; i++)
        g = C.life_step(g, birth, survival, true);
    vector<vector<int>> glider_shifted(8, vector<int>(8, 0));
    glider_shifted[2][2] = 1; glider_shifted[3][3] = 1;
    glider_shifted[1][4] = 1; glider_shifted[2][4] = 1;
    glider_shifted[3][4] = 1;
    cout << "планер за 4 шага сдвинут на (1,1): " << (g == glider_shifted)
         << " (ожидаем 1)" << endl;
    cout << "Планер:" << endl;
    C.life_print(glider);

    cout << "\n=== B.2. Мир «Ва-Тор» ===" << endl;
    mt19937 rng(42);
    // рыба в пустом мире: возраст растёт с каждым ходом, на 4-м шаге
    // достигает fish_breed = 3 и размножается (потомок на месте)
    vector<vector<CellularAutomata::WatorCell>> sea(3,
        vector<CellularAutomata::WatorCell>(3));
    sea[1][1] = {1, 0, 0};
    for (int s = 0; s < 4; s++)
        sea = C.wator_step(sea, 3, 5, 5, rng);
    auto cnt = C.wator_counts(sea);
    cout << "рыба через 4 шага: рыб = " << cnt.first
         << " (ожидаем 2)" << endl;
    // акула в окружении рыб, которым некуда плыть: съедает одну
    // (рыба не может убежать — воды вокруг нет), голод обнуляется
    vector<vector<CellularAutomata::WatorCell>> pool(2,
        vector<CellularAutomata::WatorCell>(2));
    pool[0][0] = {2, 0, 0};
    pool[0][1] = {1, 0, 0};
    pool[1][0] = {1, 0, 0};
    pool[1][1] = {1, 0, 0};
    pool = C.wator_step(pool, 3, 5, 5, rng);
    auto cnt2 = C.wator_counts(pool);
    cout << "акула съела рыбу: рыб = " << cnt2.first << ", акул = "
         << cnt2.second << " (ожидаем 2, 1)" << endl;
    // акула без еды: умирает после shark_starve = 2 шагов голода
    vector<vector<CellularAutomata::WatorCell>> empty(3,
        vector<CellularAutomata::WatorCell>(3));
    empty[1][1] = {2, 0, 0};
    for (int s = 0; s < 2; s++)
        empty = C.wator_step(empty, 3, 5, 2, rng);
    auto cnt3 = C.wator_counts(empty);
    cout << "акула без еды через 2 шага: акул = " << cnt3.second
         << " (ожидаем 0)" << endl;
    // динамика популяций: рыбы-акулы на маленьком мире, 20 шагов
    vector<vector<CellularAutomata::WatorCell>> eco(6,
        vector<CellularAutomata::WatorCell>(6));
    for (int i = 0; i < 6; i++) {
        eco[0][i] = {1, i % 3, 0};
        eco[5][i] = {1, (i + 1) % 3, 0};
    }
    eco[2][2] = {2, 0, 0};
    eco[3][3] = {2, 1, 0};
    auto hist = C.wator_run(eco, 20, 3, 4, 5, rng);
    cout << "динамика (рыбы, акулы) по 5-м шагам:" << endl;
    for (int s = 0; s < 20; s += 5)
        cout << "  шаг " << (s + 1) << ": (" << hist[s].first << ", "
             << hist[s].second << ")" << endl;

    cout << "\n=== B.3. Муравей Лэнгтона ===" << endl;
    vector<pair<int, int>> ant_rule = {{1, 1}, {-1, 0}}; // классика
    vector<vector<int>> plane(5, vector<int>(5, 0));
    auto ant = C.ant_run(plane, 2, 2, 0, ant_rule, 2);
    cout << "путь за 2 шага: (" << ant.path[0].first << ","
         << ant.path[0].second << ") (" << ant.path[1].first << ","
         << ant.path[1].second << ") (" << ant.path[2].first << ","
         << ant.path[2].second << ") (ожидаем (2,2) (3,2) (3,3))" << endl;
    cout << "клетка (2,2) перекрашена: " << plane[2][2]
         << " (ожидаем 1)" << endl;
    vector<vector<int>> plane2(5, vector<int>(5, 0));
    auto ant2 = C.ant_run(plane2, 2, 2, 0, ant_rule, 100);
    cout << "через 100 шагов: позиция (" << ant2.x << "," << ant2.y
         << "), сетка " << plane2.size() << "×" << plane2[0].size()
         << " (сетка выросла — муравей ушёл от центра)" << endl;

    cout << "\n=== C.1. Нагель-Шрекенберг ===" << endl;
    mt19937 rng2(7);
    // vmax = 1, p = 0 (правило 184): машины едут вперёд на 1
    vector<int> road = {1, -1, 1, -1, 1, -1};
    auto road1 = C.nagel_step(road, 1, 0.0, rng2);
    cout << "шаг при p=0: {";
    for (int v : road1) cout << v << " ";
    cout << "} (ожидаем -1 1 -1 1 -1 1)" << endl;
    // p > 0: скорость не превышает vmax, число машин сохраняется
    vector<int> road2 = {2, -1, -1, 2, -1, -1};
    auto road3 = C.nagel_step(road2, 2, 0.5, rng2);
    int cars2 = 0, cars3 = 0, over = 0;
    for (int v : road2) cars2 += (v >= 0);
    for (int v : road3) {
        cars3 += (v >= 0);
        over += (v > 2);
    }
    cout << "число машин: " << cars2 << " → " << cars3
         << ", скоростей > vmax: " << over << " (ожидаем 2 → 2, 0)" << endl;
    cout << "средняя скорость (vmax=2, p=0.3, 20 шагов, плотность 1/3): "
         << C.nagel_avg_speed({2, -1, -1, 2, -1, -1}, 2, 0.3, rng2, 20)
         << endl;
}
#endif // CELLULAR_AUTOMATA_MAIN
#endif // DISCRETE_LOGIC_D_CPP
