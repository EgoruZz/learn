#ifndef PICTURES_A_CPP
#define PICTURES_A_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

// =============================================================
// A. ГРАФИКА
// =============================================================
// Структура md: A. Кривые Безье
//               → B. DDA Line
//               → C. Vector3
//               → D. Узор «бабочка»
//
// GraphicsBasics — базовый класс всей ветки pictures.
// Безье (линейная/квадратичная/кубическая, де Кастельжо),
// DDA растеризация, Vector3, Lissajous-фигуры.

struct Point2D {
    double x, y;
    Point2D(double x = 0, double y = 0) : x(x), y(y) {}
    Point2D operator+(const Point2D& o) const { return {x + o.x, y + o.y}; }
    Point2D operator-(const Point2D& o) const { return {x - o.x, y - o.y}; }
    Point2D operator*(double s) const { return {x * s, y * s}; }
    double length() const { return sqrt(x * x + y * y); }
    Point2D normalize() const { double l = length(); return (l > 1e-12) ? Point2D{x/l, y/l} : *this; }
    double dot(const Point2D& o) const { return x * o.x + y * o.y; }
    double cross(const Point2D& o) const { return x * o.y - y * o.x; }
    friend ostream& operator<<(ostream& os, const Point2D& p) {
        os << "(" << p.x << "," << p.y << ")"; return os;
    }
};

struct GraphicsBasics {

// =============================================================
// A. КРИВЫЕ БЕЗЬЕ
// =============================================================

// --- A.1. Линейная кривая Безье ---
// B(t) = (1-t)*P0 + t*P1, t в [0, 1].
Point2D bezier_linear(Point2D p0, Point2D p1, double t) {
    return p0 * (1.0 - t) + p1 * t;
}

// --- A.2. Квадратичная кривая Безье ---
// B(t) = (1-t)^2*P0 + 2*(1-t)*t*P1 + t^2*P2.
Point2D bezier_quadratic(Point2D p0, Point2D p1, Point2D p2, double t) {
    double u = 1.0 - t;
    return p0 * (u * u) + p1 * (2.0 * u * t) + p2 * (t * t);
}

// --- A.3. Кубическая кривая Безье ---
// B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3.
Point2D bezier_cubic(Point2D p0, Point2D p1, Point2D p2, Point2D p3, double t) {
    double u = 1.0 - t;
    return p0 * (u * u * u) + p1 * (3.0 * u * u * t) + p2 * (3.0 * u * t * t) + p3 * (t * t * t);
}

// --- A.4. Алгоритм де Кастельжо ---
// Рекурсивное разбиение: строит точку кривой для заданного t.
Point2D de_casteljau(const vector<Point2D>& points, double t) {
    vector<Point2D> tmp = points;
    int n = (int)tmp.size();
    for (int r = n; r > 1; r--) {
        for (int i = 0; i < r - 1; i++)
            tmp[i] = tmp[i] * (1.0 - t) + tmp[i + 1] * t;
    }
    return tmp[0];
}

// --- A.5. Генерация точек кривой ---
vector<Point2D> generate_bezier_curve(const vector<Point2D>& control_points,
                                       int num_points = 100) {
    vector<Point2D> curve;
    for (int i = 0; i <= num_points; i++) {
        double t = (double)i / num_points;
        curve.push_back(de_casteljau(control_points, t));
    }
    return curve;
}

// =============================================================
// B. DDA ЛИНИЯ
// =============================================================

// --- B.1. Растеризация отрезка методом DDA ---
// Возвращает список пикселей (координат) на отрезке [A, B].
vector<pair<int,int>> dda_line(Point2D a, Point2D b) {
    vector<pair<int,int>> pixels;
    int dx = (int)round(b.x - a.x);
    int dy = (int)round(b.y - a.y);
    int steps = max(abs(dx), abs(dy));
    if (steps == 0) { pixels.push_back({(int)round(a.x), (int)round(a.y)}); return pixels; }
    double x_inc = (double)dx / steps;
    double y_inc = (double)dy / steps;
    double x = a.x, y = a.y;
    for (int i = 0; i <= steps; i++) {
        pixels.push_back({(int)round(x), (int)round(y)});
        x += x_inc;
        y += y_inc;
    }
    return pixels;
}

// =============================================================
// C. VECTOR3
// =============================================================

// --- C.1. Определение ориентации (предикат) ---
// +1: левый поворот, -1: правый, 0: коллинеарность.
int orientation(Point2D a, Point2D b, Point2D c) {
    double val = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (fabs(val) < 1e-12) return 0;
    return (val > 0) ? 1 : -1;
}

// --- C.2. Расстояние между двумя точками ---
double distance(Point2D a, Point2D b) {
    return (a - b).length();
}

// --- C.3. Угол между двумя векторами ---
double angle_between(Point2D a, Point2D b) {
    double cos_theta = a.dot(b) / (a.length() * b.length());
    cos_theta = max(-1.0, min(1.0, cos_theta));
    return acos(cos_theta);
}

// =============================================================
// D. УЗОР «БАБОЧКА» (LISSAJOUS)
// =============================================================

// --- D.1. Генерация Lissajous-фигуры ---
// x = A*sin(a*t + delta), y = B*sin(b*t).
vector<Point2D> lissajous(double A, double B, int a_freq, int b_freq,
                           double delta, int num_points = 1000) {
    vector<Point2D> curve;
    for (int i = 0; i <= num_points; i++) {
        double t = 2.0 * M_PI * i / num_points;
        curve.push_back({A * sin(a_freq * t + delta), B * sin(b_freq * t)});
    }
    return curve;
}

}; // struct GraphicsBasics

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PICTURES_A_MAIN
int main() {
    GraphicsBasics gb;

    cout << "=== A. КРИВЫЕ БЕЗЬЕ ===" << endl;

    cout << "--- Линейная кривая (отрезок) ---" << endl;
    {
        Point2D p0(0, 0), p1(10, 5);
        cout << "  P0=" << p0 << " P1=" << p1 << endl;
        for (double t = 0; t <= 1.01; t += 0.25) {
            Point2D pt = gb.bezier_linear(p0, p1, t);
            cout << "  t=" << t << " -> " << pt << endl;
        }
    }

    cout << "\n--- Квадратичная кривая ---" << endl;
    {
        Point2D p0(0, 0), p1(5, 10), p2(10, 0);
        cout << "  P0=" << p0 << " P1=" << p1 << " P2=" << p2 << endl;
        for (double t = 0; t <= 1.01; t += 0.25) {
            Point2D pt = gb.bezier_quadratic(p0, p1, p2, t);
            cout << "  t=" << t << " -> " << pt << endl;
        }
    }

    cout << "\n--- Кубическая кривая ---" << endl;
    {
        Point2D p0(0,0), p1(2,8), p2(8,8), p3(10,0);
        auto curve = gb.generate_bezier_curve({p0, p1, p2, p3}, 8);
        cout << "  8 точек кривой:" << endl;
        for (auto& pt : curve) cout << "    " << pt << endl;
    }

    cout << "\n--- Де Кастельжо vs прямое вычисление ---" << endl;
    {
        Point2D p0(0,0), p1(3,6), p2(6,6), p3(9,0);
        double t = 0.5;
        Point2D dc = gb.de_casteljau({p0, p1, p2, p3}, t);
        Point2D direct = gb.bezier_cubic(p0, p1, p2, p3, t);
        cout << "  de Casteljau: " << dc << endl;
        cout << "  Прямое:       " << direct << endl;
        cout << "  Разница:      " << (dc - direct).length() << endl;
    }

    cout << "\n=== B. DDA ЛИНИЯ ===" << endl;
    {
        auto pixels = gb.dda_line(Point2D(0, 0), Point2D(8, 5));
        cout << "  (0,0) -> (8,5): " << pixels.size() << " пикселей" << endl;
        for (auto& [x, y] : pixels) cout << "    (" << x << "," << y << ") ";
        cout << endl;
    }

    cout << "\n=== C. VECTOR3 / ПРЕДИКАТ ОРИЕНТАЦИИ ===" << endl;
    {
        Point2D a(0, 0), b(1, 0), c(0, 1);
        cout << "  orient((0,0),(1,0),(0,1)) = " << gb.orientation(a, b, c)
             << " (ожидаем +1 = левый поворот)" << endl;
        cout << "  orient((0,0),(1,0),(1,1)) = " << gb.orientation(a, b, Point2D(1,1))
             << " (ожидаем +1)" << endl;
        cout << "  orient((0,0),(1,0),(2,0)) = " << gb.orientation(a, b, Point2D(2,0))
             << " (ожидаем 0 = коллинеарность)" << endl;
        cout << "  orient((0,0),(1,0),(0,-1)) = " << gb.orientation(a, b, Point2D(0,-1))
             << " (ожидаем -1 = правый поворот)" << endl;
    }

    cout << "\n--- Расстояние и угол ---" << endl;
    {
        Point2D a(0, 0), b(3, 4);
        cout << "  dist((0,0),(3,4)) = " << gb.distance(a, b) << " (ожидаем 5)" << endl;
        Point2D u(1, 0), v(0, 1);
        cout << "  angle((1,0),(0,1)) = " << gb.angle_between(u, v) * 180 / M_PI << "° (ожидаем 90°)" << endl;
    }

    cout << "\n=== D. УЗОР «БАБОЧКА» ===" << endl;
    {
        auto curve = gb.lissajous(5, 5, 3, 2, M_PI / 2, 100);
        cout << "  Lissajous A=5, B=5, a=3, b=2, delta=pi/2:" << endl;
        cout << "  Точек: " << curve.size() << endl;
        cout << "  Первая: " << curve[0] << ", средняя: " << curve[50] << endl;
    }

    return 0;
}
#endif

#endif // PICTURES_A_CPP
