#ifndef PICTURES_B_CPP
#define PICTURES_B_CPP

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
using namespace std;

// =============================================================
// B. ЦИФРОВАЯ ОБРАБОТКА ИЗОБРАЖЕНИЙ
// =============================================================
// Структура md: A. Яркость/контраст/негатив/сепия
//               → B. Dithering (Burkes)
//               → C. Canny Edge Detection
//               → D. Фильтры (8 шт.)
//               → E. Гистограмма
//               → F. Морфология
//               → G. Геометрические преобразования
//
// ImageProcessing — наследует GraphicsBasics (a.cpp).
// Яркость/контраст/негатив/сепия, Burkes dithering,
// Canny, 8 фильтров, гистограмма, морфология, resize/rotation.

#ifndef INSIDE_PICTURES_B
#define INSIDE_PICTURES_B
#include "../a/a.cpp"
#endif

struct ImageProcessing : GraphicsBasics {

// =============================================================
// A. ОСНОВНЫЕ ПРЕОБРАЗОВАНИЯ
// =============================================================

// --- A.1. Яркость ---
// I' = I + c.
void adjust_brightness(vector<vector<int>>& img, int c) {
    for (auto& row : img)
        for (int& p : row)
            p = max(0, min(255, p + c));
}

// --- A.2. Контраст ---
// I' = (I - 128) * c + 128.
void adjust_contrast(vector<vector<int>>& img, double c) {
    for (auto& row : img)
        for (int& p : row)
            p = max(0, min(255, (int)round((p - 128) * c + 128)));
}

// --- A.3. Негатив ---
// I' = 255 - I.
void negate(vector<vector<int>>& img) {
    for (auto& row : img)
        for (int& p : row)
            p = 255 - p;
}

// --- A.4. Сепия ---
struct RGB { int r, g, b; };

RGB to_sepia(int r, int g, int b) {
    return {
        max(0, min(255, (int)round(0.393 * r + 0.769 * g + 0.189 * b))),
        max(0, min(255, (int)round(0.349 * r + 0.686 * g + 0.168 * b))),
        max(0, min(255, (int)round(0.272 * r + 0.534 * g + 0.131 * b)))
    };
}

// =============================================================
// B. DITHERING (BURKES)
// =============================================================

// --- B.1. Burkes Dithering ---
// Квантизация изображения с распределением ошибки.
// Ближайший уровень из levels[].
void burkes_dithering(vector<vector<int>>& img, const vector<int>& levels) {
    int h = (int)img.size(), w = (int)img[0].size();
    vector<vector<double>> err(h, vector<double>(w));
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            err[i][j] = img[i][j];

    // Веса Burkes: (dx, dy, weight)
    vector<tuple<int,int,double>> neighbors = {
        {1,0,8.0/16}, {1,-1,4.0/16}, {1,1,3.0/16},
        {0,1,8.0/16}, {-1,1,4.0/16}, {0,2,4.0/16}, {1,2,2.0/16}, {2,0,1.0/16}
    };

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            double old_val = err[i][j];
            // Найти ближайший уровень
            int best = levels[0];
            double best_dist = fabs(old_val - levels[0]);
            for (int l : levels) {
                if (fabs(old_val - l) < best_dist) {
                    best_dist = fabs(old_val - l);
                    best = l;
                }
            }
            img[i][j] = best;
            double error = old_val - best;
            // Распределить ошибку
            for (auto& [dx, dy, weight] : neighbors) {
                int ni = i + dx, nj = j + dy;
                if (ni >= 0 && ni < h && nj >= 0 && nj < w)
                    err[ni][nj] += error * weight;
            }
        }
    }
}

// =============================================================
// C. CANNY EDGE DETECTION
// =============================================================

// --- C.1. Гауссово сглаживание ---
void gaussian_blur(vector<vector<double>>& img, int kernel_size, double sigma) {
    int h = (int)img.size(), w = (int)img[0].size();
    int k = kernel_size / 2;
    vector<vector<double>> kernel(kernel_size, vector<double>(kernel_size));
    double sum = 0;
    for (int i = -k; i <= k; i++)
        for (int j = -k; j <= k; j++) {
            kernel[i + k][j + k] = exp(-(i*i + j*j) / (2.0 * sigma * sigma));
            sum += kernel[i + k][j + k];
        }
    for (int i = 0; i < kernel_size; i++)
        for (int j = 0; j < kernel_size; j++)
            kernel[i][j] /= sum;

    vector<vector<double>> result(h, vector<double>(w, 0));
    for (int i = k; i < h - k; i++)
        for (int j = k; j < w - k; j++) {
            double val = 0;
            for (int di = -k; di <= k; di++)
                for (int dj = -k; dj <= k; dj++)
                    val += img[i + di][j + dj] * kernel[di + k][dj + k];
            result[i][j] = val;
        }
    img = result;
}

// --- C.2. Градиент (Собель) ---
void sobel_gradient(const vector<vector<double>>& img,
                     vector<vector<double>>& magnitude,
                     vector<vector<double>>& direction) {
    int h = (int)img.size(), w = (int)img[0].size();
    magnitude.assign(h, vector<double>(w, 0));
    direction.assign(h, vector<double>(w, 0));

    // Ядра Собеля
    double Gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    double Gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};

    for (int i = 1; i < h - 1; i++) {
        for (int j = 1; j < w - 1; j++) {
            double gx = 0, gy = 0;
            for (int di = -1; di <= 1; di++)
                for (int dj = -1; dj <= 1; dj++) {
                    gx += img[i + di][j + dj] * Gx[di + 1][dj + 1];
                    gy += img[i + di][j + dj] * Gy[di + 1][dj + 1];
                }
            magnitude[i][j] = sqrt(gx * gx + gy * gy);
            direction[i][j] = atan2(gy, gx);
        }
    }
}

// --- C.3. Подавление немаксимумов ---
void non_maximum_suppression(const vector<vector<double>>& mag,
                              const vector<vector<double>>& dir,
                              vector<vector<double>>& result) {
    int h = (int)mag.size(), w = (int)mag[0].size();
    result.assign(h, vector<double>(w, 0));
    for (int i = 1; i < h - 1; i++) {
        for (int j = 1; j < w - 1; j++) {
            double angle = dir[i][j] * 180.0 / M_PI;
            if (angle < 0) angle += 180;
            double q = 0, r = 0;
            // Квантизация направления
            if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180)) {
                q = mag[i][j + 1]; r = mag[i][j - 1];
            } else if (angle >= 22.5 && angle < 67.5) {
                q = mag[i + 1][j - 1]; r = mag[i - 1][j + 1];
            } else if (angle >= 67.5 && angle < 112.5) {
                q = mag[i + 1][j]; r = mag[i - 1][j];
            } else {
                q = mag[i + 1][j + 1]; r = mag[i - 1][j - 1];
            }
            result[i][j] = (mag[i][j] >= q && mag[i][j] >= r) ? mag[i][j] : 0;
        }
    }
}

// --- C.4. Двойная пороговая фильтрация (hysteresis) ---
void hysteresis(vector<vector<double>>& img, double low, double high) {
    int h = (int)img.size(), w = (int)img[0].size();
    // Маркировка: 0 — не граница, 1 — слабая, 2 — сильная
    vector<vector<int>> state(h, vector<int>(w, 0));
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++) {
            if (img[i][j] >= high) state[i][j] = 2;
            else if (img[i][j] >= low) state[i][j] = 1;
        }
    // Связывание слабых с сильными
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 1; i < h - 1; i++)
            for (int j = 1; j < w - 1; j++) {
                if (state[i][j] == 1) {
                    for (int di = -1; di <= 1; di++)
                        for (int dj = -1; dj <= 1; dj++) {
                            if (state[i + di][j + dj] == 2) {
                                state[i][j] = 2;
                                changed = true;
                                break;
                            }
                        }
                }
            }
    }
    // Удаление слабых
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            img[i][j] = (state[i][j] == 2) ? 255 : 0;
}

// =============================================================
// D. ФИЛЬТРЫ
// =============================================================

// --- D.1. Свёртка (общая) ---
void convolve(const vector<vector<double>>& img,
              const vector<vector<double>>& kernel,
              vector<vector<double>>& result) {
    int h = (int)img.size(), w = (int)img[0].size();
    int kh = (int)kernel.size(), kw = (int)kernel[0].size();
    int khr = kh / 2, kwr = kw / 2;
    result.assign(h, vector<double>(w, 0));
    for (int i = khr; i < h - khr; i++)
        for (int j = kwr; j < w - kwr; j++) {
            double val = 0;
            for (int di = -khr; di <= khr; di++)
                for (int dj = -kwr; dj <= kwr; dj++)
                    val += img[i + di][j + dj] * kernel[di + khr][dj + kwr];
            result[i][j] = val;
        }
}

// --- D.2. Медианный фильтр ---
void median_filter(vector<vector<int>>& img, int kernel_size) {
    int h = (int)img.size(), w = (int)img[0].size();
    int k = kernel_size / 2;
    vector<vector<int>> result = img;
    for (int i = k; i < h - k; i++)
        for (int j = k; j < w - k; j++) {
            vector<int> vals;
            for (int di = -k; di <= k; di++)
                for (int dj = -k; dj <= k; dj++)
                    vals.push_back(img[i + di][j + dj]);
            sort(vals.begin(), vals.end());
            result[i][j] = vals[vals.size() / 2];
        }
    img = result;
}

// =============================================================
// E. ГИСТОГРАММА
// =============================================================

// --- E.1. Построение гистограммы ---
vector<int> histogram(const vector<vector<int>>& img, int bins = 256) {
    vector<int> hist(bins, 0);
    for (auto& row : img)
        for (int p : row)
            hist[min(p, bins - 1)]++;
    return hist;
}

// --- E.2. Эквализация ---
void histogram_equalization(vector<vector<int>>& img) {
    auto hist = histogram(img, 256);
    int total = 0;
    for (int i = 0; i < 256; i++) total += hist[i];
    // CDF
    vector<double> cdf(256, 0);
    cdf[0] = (double)hist[0] / total;
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i - 1] + (double)hist[i] / total;
    // Преобразование
    for (auto& row : img)
        for (int& p : row)
            p = max(0, min(255, (int)round(cdf[p] * 255)));
}

// --- E.3. Растяжение (Stretch) ---
void histogram_stretch(vector<vector<int>>& img) {
    int min_val = 255, max_val = 0;
    for (auto& row : img)
        for (int p : row) {
            min_val = min(min_val, p);
            max_val = max(max_val, p);
        }
    if (max_val == min_val) return;
    for (auto& row : img)
        for (int& p : row)
            p = max(0, min(255, (int)round((p - min_val) * 255.0 / (max_val - min_val))));
}

// =============================================================
// F. МОРФОЛОГИЧЕСКИЕ ОПЕРАЦИИ
// =============================================================

// --- F.1. Дилатация ---
void dilate(vector<vector<int>>& img, int kernel_size) {
    int h = (int)img.size(), w = (int)img[0].size();
    int k = kernel_size / 2;
    vector<vector<int>> result = img;
    for (int i = k; i < h - k; i++)
        for (int j = k; j < w - k; j++) {
            int max_val = 0;
            for (int di = -k; di <= k; di++)
                for (int dj = -k; dj <= k; dj++)
                    max_val = max(max_val, img[i + di][j + dj]);
            result[i][j] = max_val;
        }
    img = result;
}

// --- F.2. Эрозия ---
void erode(vector<vector<int>>& img, int kernel_size) {
    int h = (int)img.size(), w = (int)img[0].size();
    int k = kernel_size / 2;
    vector<vector<int>> result = img;
    for (int i = k; i < h - k; i++)
        for (int j = k; j < w - k; j++) {
            int min_val = 255;
            for (int di = -k; di <= k; di++)
                for (int dj = -k; dj <= k; dj++)
                    min_val = min(min_val, img[i + di][j + dj]);
            result[i][j] = min_val;
        }
    img = result;
}

// =============================================================
// G. ГЕОМЕТРИЧЕСКИЕ ПРЕОБРАЗОВАНИЯ
// =============================================================

// --- G.1. Resize (Nearest Neighbor) ---
vector<vector<int>> resize_nearest(const vector<vector<int>>& img,
                                    int new_h, int new_w) {
    int h = (int)img.size(), w = (int)img[0].size();
    vector<vector<int>> result(new_h, vector<int>(new_w));
    for (int i = 0; i < new_h; i++)
        for (int j = 0; j < new_w; j++) {
            int src_i = min(h - 1, (int)(i * h / new_h));
            int src_j = min(w - 1, (int)(j * w / new_w));
            result[i][j] = img[src_i][src_j];
        }
    return result;
}

// --- G.2. Resize (Bilinear Interpolation) ---
double bilinear_interpolate(const vector<vector<int>>& img,
                             double x, double y) {
    int h = (int)img.size(), w = (int)img[0].size();
    int x0 = max(0, min(h - 1, (int)floor(x)));
    int x1 = max(0, min(h - 1, x0 + 1));
    int y0 = max(0, min(w - 1, (int)floor(y)));
    int y1 = max(0, min(w - 1, y0 + 1));
    double dx = x - x0, dy = y - y0;
    return img[x0][y0] * (1 - dx) * (1 - dy) +
           img[x0][y1] * (1 - dx) * dy +
           img[x1][y0] * dx * (1 - dy) +
           img[x1][y1] * dx * dy;
}

vector<vector<int>> resize_bilinear(const vector<vector<int>>& img,
                                     int new_h, int new_w) {
    int h = (int)img.size(), w = (int)img[0].size();
    vector<vector<int>> result(new_h, vector<int>(new_w));
    for (int i = 0; i < new_h; i++)
        for (int j = 0; j < new_w; j++) {
            double x = (double)i * h / new_h;
            double y = (double)j * w / new_w;
            result[i][j] = max(0, min(255, (int)round(bilinear_interpolate(img, x, y))));
        }
    return result;
}

// --- G.3. Rotation (матрица поворота) ---
// Поворот изображения на angle градусов.
vector<vector<int>> rotate_image(const vector<vector<int>>& img, double angle_deg) {
    int h = (int)img.size(), w = (int)img[0].size();
    double angle = angle_deg * M_PI / 180.0;
    double cos_a = cos(angle), sin_a = sin(angle);
    int new_h = (int)ceil(fabs(h * cos_a) + fabs(w * sin_a));
    int new_w = (int)ceil(fabs(w * cos_a) + fabs(h * sin_a));
    vector<vector<int>> result(new_h, vector<int>(new_w, 128));
    double cx = w / 2.0, cy = h / 2.0;
    double ncx = new_w / 2.0, ncy = new_h / 2.0;
    for (int i = 0; i < new_h; i++)
        for (int j = 0; j < new_w; j++) {
            double dx = j - ncx, dy = i - ncy;
            double src_x = cos_a * dx + sin_a * dy + cx;
            double src_y = -sin_a * dx + cos_a * dy + cy;
            if (src_x >= 0 && src_x < w && src_y >= 0 && src_y < h)
                result[i][j] = max(0, min(255, (int)round(bilinear_interpolate(img, src_y, src_x))));
        }
    return result;
}

}; // struct ImageProcessing

// =============================================================
// MAIN — демонстрация
// =============================================================
#ifdef PICTURES_B_MAIN
int main() {
    ImageProcessing ip;
    srand(42);

    cout << "=== A. ОСНОВНЫЕ ПРЕОБРАЗОВАНИЯ ===" << endl;
    {
        // 4x4 изображение
        vector<vector<int>> img = {
            {100, 150, 200, 250},
            {50, 100, 150, 200},
            {0, 50, 100, 150},
            {0, 0, 50, 100}
        };
        cout << "  Исходное: [" << img[0][0] << "," << img[0][1] << "," << img[0][2] << "," << img[0][3] << "]" << endl;

        auto bright = img;
        ip.adjust_brightness(bright, 50);
        cout << "  Яркость +50: [" << bright[0][0] << "," << bright[0][1] << "," << bright[0][2] << "," << bright[0][3] << "]" << endl;

        auto contrast = img;
        ip.adjust_contrast(contrast, 1.5);
        cout << "  Контраст x1.5: [" << contrast[0][0] << "," << contrast[0][1] << "," << contrast[0][2] << "," << contrast[0][3] << "]" << endl;

        auto neg = img;
        ip.negate(neg);
        cout << "  Негатив: [" << neg[0][0] << "," << neg[0][1] << "," << neg[0][2] << "," << neg[0][3] << "]" << endl;

        auto sep = ip.to_sepia(100, 150, 200);
        cout << "  Сепия (100,150,200) -> (" << sep.r << "," << sep.g << "," << sep.b << ")" << endl;
    }

    cout << "\n=== B. BURKES DITHERING ===" << endl;
    {
        vector<vector<int>> img = {
            {10, 40, 70, 100, 130, 160, 190, 220},
            {20, 50, 80, 110, 140, 170, 200, 230},
            {30, 60, 90, 120, 150, 180, 210, 240},
            {40, 70, 100, 130, 160, 190, 220, 250}
        };
        cout << "  До: [" << img[0][0] << "," << img[0][1] << "," << img[0][2] << "," << img[0][3] << "]" << endl;
        ip.burkes_dithering(img, {0, 85, 170, 255});
        cout << "  После (4 уровня): [" << img[0][0] << "," << img[0][1] << "," << img[0][2] << "," << img[0][3] << "]" << endl;
    }

    cout << "\n=== C. CANNY EDGE DETECTION ===" << endl;
    {
        // Простое изображение: градиент
        int size = 20;
        vector<vector<double>> img(size, vector<double>(size));
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                img[i][j] = (double)(i + j) / (2 * (size - 1)) * 255;

        // Шаг 1: Гауссово сглаживание
        ip.gaussian_blur(img, 3, 1.0);
        cout << "  После размытия: f(5,5)=" << img[5][5] << endl;

        // Шаг 2: Градиент
        vector<vector<double>> mag, dir;
        ip.sobel_gradient(img, mag, dir);
        cout << "  Градиент: mag(5,5)=" << mag[5][5] << " dir(5,5)=" << dir[5][5] << endl;

        // Шаг 3: Подавление немаксимумов
        vector<vector<double>> nms;
        ip.non_maximum_suppression(mag, dir, nms);
        int nms_count = 0;
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                if (nms[i][j] > 0) nms_count++;
        cout << "  После NMS: " << nms_count << " ненулевых пикселей" << endl;
    }

    cout << "\n=== D. ФИЛЬТРЫ ===" << endl;
    {
        vector<vector<double>> img = {
            {100, 100, 100, 100, 100},
            {100, 200, 200, 200, 100},
            {100, 200, 255, 200, 100},
            {100, 200, 200, 200, 100},
            {100, 100, 100, 100, 100}
        };

        // Гауссово размытие
        vector<vector<double>> blurred;
        ip.gaussian_blur(img, 3, 1.0);
        cout << "  Гаусс (центр): " << img[2][2] << " (было 255)" << endl;

        // Медианный фильтр
        vector<vector<int>> int_img = {
            {100, 100, 100, 100, 100},
            {100, 200, 200, 200, 100},
            {100, 200, 1000, 200, 100},  // шум "соль"
            {100, 200, 200, 200, 100},
            {100, 100, 100, 100, 100}
        };
        ip.median_filter(int_img, 3);
        cout << "  Медианный (центр, был шум 1000): " << int_img[2][2] << endl;
    }

    cout << "\n=== E. ГИСТОГРАММА ===" << endl;
    {
        vector<vector<int>> img = {
            {10, 20, 30, 40, 50},
            {60, 70, 80, 90, 100},
            {110, 120, 130, 140, 150},
            {160, 170, 180, 190, 200}
        };
        auto hist = ip.histogram(img, 256);
        int non_zero = 0;
        for (int h : hist) if (h > 0) non_zero++;
        cout << "  Ненулевых бинов: " << non_zero << "/256" << endl;

        ip.histogram_stretch(img);
        cout << "  После stretch: [" << img[0][0] << "," << img[0][4] << "] -> [0,255]" << endl;
    }

    cout << "\n=== F. МОРФОЛОГИЯ ===" << endl;
    {
        vector<vector<int>> img = {
            {0, 0, 0, 0, 0},
            {0, 255, 0, 255, 0},
            {0, 0, 0, 0, 0},
            {0, 255, 0, 255, 0},
            {0, 0, 0, 0, 0}
        };
        auto dilated = img;
        ip.dilate(dilated, 3);
        cout << "  Дилатация (центр): " << dilated[2][2] << " (было 0)" << endl;
        auto eroded = img;
        ip.erode(eroded, 3);
        cout << "  Эрозия (1,1): " << eroded[1][1] << " (было 255)" << endl;
    }

    cout << "\n=== G. ПРЕОБРАЗОВАНИЯ ===" << endl;
    {
        vector<vector<int>> img = {
            {255, 200, 150, 100},
            {200, 255, 200, 150},
            {150, 200, 255, 200},
            {100, 150, 200, 255}
        };

        auto resized = ip.resize_nearest(img, 2, 2);
        cout << "  Resize 4x4 -> 2x2 (nearest):" << endl;
        for (auto& row : resized) { for (int p : row) cout << p << " "; cout << endl; }

        auto rotated = ip.rotate_image(img, 45);
        cout << "  Rotation 45°: " << rotated.size() << "x" << rotated[0].size() << endl;
    }

    return 0;
}
#endif

#endif // PICTURES_B_CPP
