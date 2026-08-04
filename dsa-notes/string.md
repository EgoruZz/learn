## **9. СТРОКИ И ТЕКСТОВЫЕ АЛГОРИТМЫ**

### **I. БАЗОВЫЕ СТРУКТУРЫ ДАННЫХ ДЛЯ СТРОК**

#### **A. СТАНДАРТНЫЕ СТРОКОВЫЕ КОНТЕЙНЕРЫ**
*   **`std::string` (C++):**
    *   Динамический массив символов
    *   Small String Optimization (SSO)
    *   Copy-on-Write (устаревшее в современных реализациях)
    *   Member functions: `append()`, `substr()`, `find()`, `replace()`, etc.
*   **`std::string_view` (C++17):**
    *   Легковесная read-only view на строку
    *   Не владеет данными, только ссылка
    *   Идеален для параметров функций
*   **`std::wstring`, `std::u16string`, `std::u32string`:**
    *   Для Unicode символов (UTF-16, UTF-32)
*   **`std::basic_string<T>`:** шаблонная версия для любых типов
*   **С-строки:** null-terminated char arrays
*   **`std::stringstream`, `std::ostringstream`, `std::istringstream`:**
    *   Потоки для форматирования строк
    *   Преобразование типов
    *   Парсинг данных из строк
*   Small String Optimization (SSO)
*   Copy-on-Write строки

#### **B. ПРОДВИНУТЫЕ СТРОКОВЫЕ СТРУКТУРЫ**
*   **Rope (Канатная структура):**
    *   Для очень больших строк (>10MB)
    *   Сбалансированное бинарное дерево строк
    *   Быстрые вставки/удаления в любой позиции
    *   Используется в текстовых редакторах
*   **Gap Buffer:**
    *   Для текстовых редакторов
    *   Быстрые операции в позиции курсора
    *   "Разрыв" в массиве для эффективных вставок
*   **Piece Table:**
    *   Используется в Visual Studio Code
    *   Хранение оригинального и модифицированного текста
    *   Легковесные модификации

#### **C. ПАМЯТЬ И ОПТИМИЗАЦИИ**
*   **String interning** - пул строк для экономии памяти
*   **Flyweight pattern** для строк
*   **Copy-on-Write** (исторически, теперь редко)
*   **Short String Optimization** (SSO) - до 15-23 символов в стеке

### **II. ПОИСК ПОДСТРОКИ (STRING MATCHING)**

#### **A. БАЗОВЫЕ АЛГОРИТМЫ**
*   **Наивный поиск (Naive):**
    *   Naive String Search
    *   O(n*m) в худшем случае
*   **Bitap Algorithm (Shift-Or/Shift-And):**
    *   Bitap String Match
    *   Для поиска с нечётким соответствием
    *   Использует битовые операции

#### **B. КЛАССИЧЕСКИЕ АЛГОРИТМЫ**
*   **Алгоритм Кнута-Морриса-Пратта (KMP):**
    *   Префикс-функция
    *   Prefix Function
    *   O(n+m) время
*   **Алгоритм Рабина-Карпа:**
    *   Использует хеширование
    *   Rolling hash technique
    *   Вероятностный, но быстрый на практике
*   **Алгоритм Бойера-Мура:**
    *   Bad character rule
    *   Good suffix rule
    *   Очень быстрый на практике
*   **Алгоритм З (Z-algorithm):**
    *   Z-функция
    *   Z-box technique
    *   Применения в различных задачах

#### **C. СПЕЦИАЛИЗИРОВАННЫЕ АЛГОРИТМЫ**
*   **Алгоритм Манакера:**
    *   Поиск всех палиндромов за O(n)
    *   Использует уже вычисленную информацию
*   **Two-way string matching algorithm:**
    *   Алгоритм Крочемора-Перрина
    *   Гарантированно линейное время
*   **Apostolico-Giancarlo algorithm**
*   **Commentz-Walter algorithm** (для множества паттернов)

#### **D. ПОИСК С ШАБЛОНАМИ**
*   **Wildcard Pattern Matching:**
    *   Символы '*' и '?'
    *   Динамическое программирование
*   **Regular Expression Matching:**
    *   NFA/DFA построение
    *   Thompson's construction
    *   Регулярные выражения с обратными ссылками

### **III. СТРОКОВЫЕ РАССТОЯНИЯ И РЕДАКТИРОВАНИЕ**

#### **A. МЕТРИКИ РАССТОЯНИЯ**
*   **Расстояние Левенштейна (Levenshtein Distance):**
    *   Минимальное число вставок, удалений, замен
    *   Динамическое программирование O(n*m)
    *   Edit Distance (общий термин)
*   **Расстояние Дамерау-Левенштейна:**
    *   Damerau Levenshtein Distance
    *   Добавлена операция транспозиции (перестановки соседних символов)
*   **Расстояние Хэмминга:**
    *   Hamming Distance
    *   Только для строк одинаковой длины
    *   Число позиций с разными символами
*   **Расстояние Джарро-Винклера:**
    *   Jaro Winkler Distance
    *   Для сравнения имён и коротких строк
    *   Учитывает префиксы

#### **B. ДРУГИЕ МЕТРИКИ СХОДСТВА**
*   **Cosine similarity** для текстов
*   **Jaccard similarity** для наборов n-грамм
*   **Sørensen–Dice coefficient**
*   **Longest Common Subsequence (LCS)**
*   **Longest Common Substring**

#### **C. ОПТИМИЗАЦИИ И ПРИМЕНЕНИЯ**
*   **Min Cost String Conversion:**
    *   Задача редактирования с различными стоимостями операций
*   **Алгоритм Укконена для edit distance**
*   **Bit-parallel DP** для ускорения вычислений
*   **Four Russians method** для edit distance

### **IV. ХЕШИРОВАНИЕ СТРОК**

#### **A. ПОЛИНОМИАЛЬНОЕ ХЕШИРОВАНИЕ**
*   **Базовое хеширование:**
    *   String hashing
    *   Полиномиальные хеши
    *   Выбор основания и модуля
*   **Двойное хеширование:**
    *   String hashing с двойным модулем
    *   Для уменьшения коллизий
*   **Rolling hash:**
    *   Для скользящего окна
    *   Rabin-Karp algorithm

#### **B. ПРИМЕНЕНИЯ ХЕШИРОВАНИЯ**
*   **Сравнение подстрок за O(1):**
    *   Префиксные хеши
    *   Хеширование подстрок
*   **Поиск повторяющихся подстрок**
*   **Проверки на изоморфизм** строк
*   **Циклические сдвиги и минимальный сдвиг**
*   **Поиск палиндромов через хеши**

#### **C. КОЛЛИЗИИ И БЕЗОПАСНОСТЬ**
*   **Коллизии хешей:**
    *   Birthday paradox применительно к хешам
    *   Атаки на хеш-функции
*   **Universal hashing** для строк
*   **Cryptographic string hashes** (SHA, MD5 для строк)

### **V. СТРОКОВЫЕ АВТОМАТЫ И СТРУКТУРЫ**

#### **A. ПРЕФИКСНОЕ ДЕРЕВО (TRIE)**
*   **Базовый Trie:**
    *   Trie
    *   Вставка, поиск, удаление
    *   Префиксный поиск
*   **Специализированные Trie:**
    *   **Radix Tree (Patricia Tree):**
        *   Radix Tree
        *   Сжатый trie
        *   Экономия памяти
    *   **Suffix Trie**
    *   **Ternary Search Trie**
*   **Применения Trie:**
    *   Autocomplete Using Trie
    *   Word Break (разбиение строки на слова)
    *   Word Search (поиск слов в матрице)
    *   Spell checker
    *   IP routing tables

#### **B. АЛГОРИТМ АХО-КОРАСИК**
*   **Построение автомата:**
    *   Trie + failure links
    *   BFS для построения fail-ссылок
*   **Множественный поиск паттернов:**
    *   Линейное время O(n + m + k)
    *   Где k - число вхождений
*   **Применения:**
    *   Intrusion detection systems
    *   Bioinformatics pattern search

#### **C. ДЕРЕВО ПАЛИНДРОМОВ**
*   **Структура дерева палиндромов:**
    *   Хранение всех различных палиндромных подстрок
    *   Ссылки на суффиксные палиндромы
*   **Алгоритм построения за O(n):**
    *   Online algorithm
    *   Использует серийные ссылки
*   **Применения:**
    *   Число различных палиндромных подстрок
    *   Самый длинный палиндромный префикс/суффикс

#### **D. СУФФИКСНЫЕ СТРУКТУРЫ**

##### **1. Суффиксный массив:**
*   **Построение:**
    *   Naive O(n² log n)
    *   Prefix doubling O(n log n)
    *   **Алгоритм Фараха** O(n) - сложный, но оптимальный
    *   DC3/Skew algorithm O(n)
*   **LCP массив:**
    *   **Алгоритм Касаи** O(n) для построения LCP
    *   Наибольший общий префикс соседних суффиксов
*   **Применения:**
    *   Поиск подстроки O(m log n)
    *   Число различных подстрок
    *   Самый длинный повторяющийся подстрок
    *   Поиск наибольшей общей подстроки

##### **2. Суффиксное дерево:**
*   **Структура:**
    *   Suffix Tree
    *   Suffix Tree Node
    *   Примеры:
        *   Example
        *   Example Usage
*   **Алгоритм Укконена:**
    *   Online построение за O(n)
    *   Suffix links, active point
    *   Tests:
        *   Test Suffix Tree
*   **Применения:**
    *   Все применения суффиксного массива + больше
    *   Поиск всех вхождений паттерна за O(m)
    *   Наибольшая общая подстрока нескольких строк

##### **3. Суффиксный автомат:**
*   **Минимальный автомат для всех подстрок**
*   **Построение за O(n)**
*   **Применения:**
    *   Число различных подстрок
    *   Поиск кратчайшей строки, не входящей в текст
    *   Статистика подстрок

### **VI. АНАЛИЗ И ПРЕОБРАЗОВАНИЕ СТРОК**

#### **A. СТАТИСТИЧЕСКИЙ АНАЛИЗ**
*   **Частотный анализ:**
    *   Frequency Finder
    *   Word Occurrence
    *   Count Vowels
    *   Character frequency distribution
*   **Топ-K анализ:**
    *   Top K Frequent Words
    *   Heap-based approach
    *   Bucket sort approach
*   **N-gram анализ:**
    *   Ngram generation
    *   Для анализа текста и предсказания

#### **B. ПРОВЕРКА СВОЙСТВ СТРОК**
*   **Уникальность:**
    *   Is Contains Unique Chars
    *   Bitmask approach для ASCII
*   **Изограммы и панграммы:**
    *   Is Isogram (слово без повторяющихся букв)
    *   Is Pangram (текст со всеми буквами алфавита)
*   **Палиндромы:**
    *   Palindrome
    *   Can String Be Rearranged As Palindrome
*   **Анаграммы:**
    *   Anagrams
    *   Check Anagrams
    *   Sorting vs frequency counting

#### **C. ТРАНСФОРМАЦИИ СТРОК**
*   **Регистр:**
    *   Capitalize
    *   Lower
    *   Upper
    *   Title
    *   String Switch Case
*   **Форматы:**
    *   Camel Case To Snake Case
    *   Snake Case To Camel Pascal Case
    *   Kebab case, dot case, etc.
*   **Перестановки:**
    *   Reverse Letters
    *   Reverse Words
    *   Alternative String Arrange
*   **Разделение и объединение:**
    *   Join
    *   Split
    *   Tokenization
    *   Strip (удаление пробелов)

#### **D. ТЕКСТОВАЯ ОБРАБОТКА**
*   **Выравнивание текста:**
    *   Text Justification
    *   Перенос слов, выравнивание по ширине
*   **Удаление дубликатов:**
    *   Remove Duplicate
    *   In-place removal
*   **Волнообразные преобразования:**
    *   Wave String
    *   Zigzag conversion

### **VII. СЖАТИЕ СТРОК И ДАННЫХ**

#### **A. БЕЗПОТЕРЬНЫЕ АЛГОРИТМЫ СЖАТИЯ**
*   **Run-Length Encoding (RLE):**
    *   Алгоритм RLE
    *   Для данных с повторениями
*   **Алгоритм Хаффмана:**
    *   Префиксные коды
    *   Оптимальное сжатие без потерь
    *   Canonical Huffman codes
*   **Алгоритм LZW (Lempel-Ziv-Welch):**
    *   Словарное сжатие
    *   Используется в GIF, UNIX compress
*   **Алгоритмы LZ77/LZ78:**
    *   Sliding window compression
    *   Deflate (gzip) использует LZ77 + Huffman

#### **B. ПРЕОБРАЗОВАНИЯ ДЛЯ СЖАТИЯ**
*   **Преобразование Барроуза-Уилера (BWT):**
    *   Burrows-Wheeler Transform
    *   Делает данные более сжимаемыми
    *   Используется в bzip2
*   **Move-to-Front transform**
*   **Delta encoding**

#### **C. СПЕЦИАЛИЗИРОВАННЫЕ АЛГОРИТМЫ**
*   **Алгоритм Крочемора:**
    *   Для сжатия строк
    *   На основе LZ77 с оптимизациями

### **VIII. ВАЛИДАЦИЯ И ПАТТЕРНЫ**

#### **A. ВАЛИДАЦИЯ ДАННЫХ**
*   **Email валидация:**
    *   Is Valid Email Address
    *   RFC 5322 compliance
*   **Phone number валидация:**
    *   Indian Phone Validator
    *   Is Srilankan Phone Number
    *   E.164 format validation
*   **ID валидация:**
    *   Is Polish National Id
    *   Is Spain National Id
    *   Luhn algorithm для проверки цифр
*   **Прочие валидаторы:**
    *   Credit Card Validator
    *   Barcode Validator (UPC, EAN)
    *   ISBN validator

#### **B. РАСПОЗНАВАНИЕ ПАТТЕРНОВ**
*   **Word Patterns:**
    *   Match Word Pattern
    *   Word Patterns
    *   Биекция между строками и паттернами
*   **Language detection:**
    *   Detecting English Programmatically
    *   N-gram based detection
    *   Stop words analysis
*   **Bioinformatics patterns:**
    *   Dna sequence processing
    *   Protein sequence analysis
    *   Restriction enzyme sites

#### **C. РЕГУЛЯРНЫЕ ВЫРАЖЕНИЯ**
*   **Регулярные выражения:**
    *   Реализация engine
    *   NFA/DFA construction
    *   Backtracking vs automaton
*   **Конечные автоматы** для паттернов

### **IX. РАЗБОР И ОБРАБОТКА ВЫРАЖЕНИЙ**

#### **A. НОТАЦИИ И ПРЕОБРАЗОВАНИЯ**
*   **Инфиксная, префиксная, постфиксная нотации**
*   **Алгоритм сортировочной станции (Shunting Yard):**
    *   Edsger Dijkstra's algorithm
    *   Преобразование инфикс → постфикс
    *   Обработка операторов и приоритетов
*   **Обратная польская нотация (RPN):**
    *   Вычисление выражений
    *   Stack-based evaluation

#### **B. СИНТАКСИЧЕСКИЙ АНАЛИЗ**
*   **Рекурсивный спуск:**
    *   Top-down parsing
    *   LL(k) parsers
    *   Построение AST
*   **Parser combinators** в функциональных языках
*   **Operator precedence parsing**

#### **C. ВАЛИДАЦИЯ ВЫРАЖЕНИЙ**
*   **Проверка скобок:**
    *   Stack-based validation
    *   Multiple bracket types
*   **Expression evaluation:**
    *   С поддержкой переменных
    *   Function calls
*   **Syntax highlighting** основы

### **X. ПРОДВИНУТЫЕ СТРОКОВЫЕ АЛГОРИТМЫ**

#### **A. ТЕОРИЯ СТРОК И КОМБИНАТОРИКА**
*   **Декомпозиция Линдона:**
    *   Lyndon decomposition
    *   **Алгоритм Дюваля** за O(n)
    *   Нахождение наименьшего циклического сдвига
*   **Border array:**
    *   Обобщение префикс-функции
    *   Связь с периодичностями
*   **Z-функция и её обобщения**

#### **B. ПЕРИОДИЧНОСТИ И ПОВТОРЫ**
*   **Поиск периода строки:**
    *   Префикс-функция для поиска периода
    *   Border theory
*   **Тандемные повторы:**
    *   **Алгоритм Мейна-Лоренца:**
        *   O(n log n) divide-and-conquer
        *   Поиск всех тандемных повторов
    *   **Crochemore's algorithm** за O(n log n)
    *   Применения в биоинформатике
*   **Квадраты в строках:**
    *   Квадрат = два вхождения одной строки
    *   Алгоритмы поиска квадратов

#### **C. МИНИМАЛЬНЫЕ И МАКСИМАЛЬНЫЕ СТРОКИ**
*   **Минимальный циклический сдвиг:**
    *   Booth's algorithm O(n)
    *   Использование префикс-функции
*   **Лексикографически наименьший/наибольший**
*   **Сортировка циклических сдвигов**

### **XI. ФОНЕТИЧЕСКИЕ И ИГРОВЫЕ АЛГОРИТМЫ**

#### **A. ФОНЕТИЧЕСКИЕ АЛГОРИТМЫ**
*   **Soundex algorithm:**
    *   Для фонетического сравнения имён
    *   Используется в генеалогии
*   **Metaphone и Double Metaphone**
*   **NYSIIS** (New York State Identification and Intelligence System)

#### **B. ИГРЫ СО СТРОКАМИ**
*   **Pig Latin:**
    *   Языковая игра
    *   Простые трансформации строк
*   **Word Ladder:**
    *   Преобразование одного слова в другое
    *   BFS на графе слов
*   **Boggle word search**
*   **Scrabble word validation**

### **XII. БИТОВЫЕ И НИЗКОУРОВНЕВЫЕ ОПЕРАЦИИ**

#### **A. БИТОВЫЕ МАНИПУЛЯЦИИ**
*   **Битовые операции со строками:**
    *   Перевод строк в двоичный вид
    *   XOR, AND, OR операций над строками
*   **Bitmask для строк:**
    *   Для строк из малого алфавита
    *   Проверка уникальности символов через битовую маску
*   **Bit-parallel string matching:**
    *   Shift-Or algorithm
    *   Для небольших алфавитов

#### **B. СИМВОЛЫ И КОДИРОВКИ**
*   **ASCII, UTF-8, UTF-16, UTF-32:**
    *   Кодировки символов
    *   Unicode normalization
*   **Byte order mark (BOM) handling**
*   **String encoding conversion**
