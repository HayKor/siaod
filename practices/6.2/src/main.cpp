#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

// ===== Задание 1 =====

const std::string SEPARATORS = " ,.;:!?-()\"";

// Разбивает предложение на слова. Слово - непрерывная последовательность
// символов, не являющихся разделителями.
std::vector<std::string> splitWords(const std::string& s) {
    std::vector<std::string> words;
    std::string w;
    for (char c : s) {
        if (SEPARATORS.find(c) == std::string::npos) {
            w += c;
        } else if (!w.empty()) {
            words.push_back(w);
            w.clear();
        }
    }
    if (!w.empty()) words.push_back(w);
    return words;
}

// Предусловие: s - предложение из слов, разделенных знаками препинания.
// Постусловие: возвращено число вхождений первого слова в предложение
// (с учетом его самого и регистра букв); 0, если слов нет.
int countFirstWord(const std::string& s) {
    std::vector<std::string> words = splitWords(s);
    int count = 0;
    for (const std::string& w : words)
        if (w == words[0]) count++;
    return count;
}

// ===== Задание 2 =====

const long long B = 256;         // основание b полиномиального хеша
const long long Q = 1000000007;  // простой модуль Q

// Полиномиальный хеш m символов строки s, начиная с позиции start:
// H = (c1*b^(m-1) + c2*b^(m-2) + ... + cm*b^0) mod Q
long long polyHash(const std::string& s, int start, int m, long long q) {
    long long h = 0;
    for (int i = start; i < start + m; i++)
        h = (h * B + (unsigned char)s[i]) % q;
    return h;
}

// b^(m-1) mod Q - вес первого символа подстроки
long long topWeight(int m, long long q) {
    long long w = 1;
    for (int i = 0; i < m - 1; i++) w = w * B % q;
    return w;
}

// Кольцевой (скользящий) хеш - хеш подстроки, сдвинутой на 1 вправо:
// H = ((Hp - Cp*b^(m-1)) * b + Cn) mod Q,
// Hp - предыдущий хеш, Cp - ушедший символ, Cn - новый символ, w = b^(m-1)
long long rollHash(long long hp, char cp, char cn, long long w, long long q) {
    hp = (hp - (unsigned char)cp * w % q + q) % q;
    return (hp * B + (unsigned char)cn) % q;
}

// Поиск Рабина-Карпа одиночного образца.
// Предусловие: q - простое число.
// Постусловие: возвращен индекс первого вхождения sub в s или -1;
// к cmp прибавлено число выполненных сравнений (хешей и символов).
int rabinKarp(const std::string& s, const std::string& sub, long long& cmp,
              long long q = Q) {
    int n = s.size(), m = sub.size();
    if (m == 0) return 0;
    if (m > n) return -1;

    long long w = topWeight(m, q);
    long long hsub = polyHash(sub, 0, m, q);  // хеш образца
    long long hs = polyHash(s, 0, m, q);      // хеш подстроки текста
    for (int i = 0; i <= n - m; i++) {
        cmp++;
        if (hs == hsub) {  // хеши совпали - проверяем символы
            int j = 0;
            while (j < m) {
                cmp++;
                if (s[i + j] != sub[j]) break;
                j++;
            }
            if (j == m) return i;
        }
        if (i < n - m) hs = rollHash(hs, s[i], s[i + m], w, q);
    }
    return -1;
}

// Проверка на плагиат - поиск Рабина-Карпа множества образцов одинаковой
// длины. Образцы - все фрагменты проверяемого текста длиной k байт.
// Предусловие: k > 0.
// Постусловие: borrowed[i] = true, если i-й байт проверяемого текста входит
// во фрагмент длины k, который встречается в другом тексте.
std::vector<bool> plagiarism(const std::string& checked,
                             const std::string& source, int k) {
    int n = checked.size(), m = source.size();
    std::vector<bool> borrowed(n, false);
    if (k > n || k > m) return borrowed;
    long long w = topWeight(k, Q);

    // 1. Хеши всех образцов: хеш -> позиции образцов в проверяемом тексте
    std::map<long long, std::vector<int>> hsubs;
    long long h = polyHash(checked, 0, k, Q);
    for (int i = 0; i + k <= n; i++) {
        hsubs[h].push_back(i);
        if (i + k < n) h = rollHash(h, checked[i], checked[i + k], w, Q);
    }

    // 2. Окно длины k проходит по другому тексту
    long long hs = polyHash(source, 0, k, Q);
    for (int i = 0; i + k <= m; i++) {
        auto it = hsubs.find(hs);
        if (it != hsubs.end())  // хеш есть среди образцов - сверяем символы
            for (int p : it->second)
                if (source.compare(i, k, checked, p, k) == 0)
                    for (int j = p; j < p + k; j++) borrowed[j] = true;
        if (i + k < m) hs = rollHash(hs, source[i], source[i + k], w, Q);
    }
    return borrowed;
}

// Длина символа UTF-8, начинающегося с байта c (русская буква - 2 байта)
int charLen(unsigned char c) {
    if (c < 0x80) return 1;
    if (c < 0xE0) return 2;
    if (c < 0xF0) return 3;
    return 4;
}

// Выводит проверяемый текст, заключая заимствованные фрагменты в [ ],
// и число заимствованных символов. Символ заимствован, если заимствованы
// все его байты.
void printPlagiarism(const std::string& checked,
                     const std::vector<bool>& borrowed) {
    int total = 0, copied = 0;
    bool prev = false;
    std::string out;
    for (int i = 0; i < (int)checked.size(); total++) {
        int len = charLen(checked[i]);
        bool b = true;
        for (int j = i; j < i + len; j++) b = b && borrowed[j];
        if (b && !prev) out += '[';
        if (!b && prev) out += ']';
        out += checked.substr(i, len);
        if (b) copied++;
        prev = b;
        i += len;
    }
    if (prev) out += ']';
    std::cout << "  " << out << "\n  Заимствовано символов: " << copied
              << " из " << total << '\n';
}

// ===== Тесты и замеры =====

using Clock = std::chrono::steady_clock;

std::string randomText(int n, std::mt19937& rng) {
    std::string s(n, ' ');
    for (char& c : s) c = 'a' + rng() % 26;
    return s;
}

void test(const std::string& name, const std::string& text,
          const std::string& pattern, long long q = 1000000007) {
    long long cmp = 0;
    int pos = rabinKarp(text, pattern, cmp, q);
    std::printf("n = %7zu, m = %5zu: индекс %7d, сравнений %7lld  (%s)\n",
                text.size(), pattern.size(), pos, cmp, name.c_str());
}

int main() {
    std::cout << "== Задание 1 ==\n";
    for (std::string s : {"кот, кот; собака: кот! мышь, котёнок.",
                          "Мама мыла раму, мама - молодец.",
                          "один",
                          "раз, два, три, раз-раз, раз!",
                          ""})
        std::cout << "\"" << s << "\" -> " << countFirstWord(s) << '\n';

    std::cout << "\n== Задание 2 ==\n";
    std::cout << "Пример из отчета (q = 13):\n";
    test("успешный поиск", "abracadabra", "cad", 13);
    test("безуспешный поиск", "abracadabra", "rab", 13);

    std::cout << "\nПроверка на плагиат (длина фрагмента k = 30 байт):\n";
    std::string source =
        "Хеш-таблица хранит пары ключ-значение. "
        "Поиск в ней выполняется в среднем за константное время. "
        "Коллизии разрешаются методом цепочек или открытой адресацией.";
    for (std::string checked :
         {"Поиск в ней выполняется в среднем за константное время, "
          "это удобно. Коллизии разрешают методом цепочек.",
          "Дерево - это связный граф без циклов.",
          "Хеш-таблица хранит пары ключ-значение."})
        printPlagiarism(checked, plagiarism(checked, source, 30));

    std::cout << "\nТесты:\n";
    std::mt19937 rng(4);
    std::string big = randomText(1000000, rng);
    test("малый текст, малый образец", "hello world", "world");
    test("малый текст, малый образец", "hello world", "word");
    test("малый текст, образец длиннее", "abc", "abcd");
    test("малый текст, пустой образец", "abc", "");
    test("большой текст, малый образец", big, big.substr(500000, 10));
    test("большой текст, малый образец", big, "zzzzzzzzz#");
    test("большой текст, большой образец", big, big.substr(900000, 50000));
    test("большой текст, большой образец", big,
         big.substr(900000, 49999) + "#");

    std::cout << "\nЗамеры (безуспешный поиск):\n";
    std::cout << "        n       m    сравнений     время, мс\n";
    for (int n : {10000, 100000, 1000000}) {
        std::string text = big.substr(0, n);
        for (int m : {10, 100, 1000}) {
            std::string pattern = std::string(m - 1, 'a') + "#";
            long long cmp = 0;
            auto start = Clock::now();
            rabinKarp(text, pattern, cmp);
            auto dt = Clock::now() - start;
            double ms = std::chrono::duration<double, std::milli>(dt).count();
            std::printf("%9d %7d %12lld %13.3f\n", n, m, cmp, ms);
        }
    }

    std::cout << "\nМалое q = 13 (много ложных совпадений хешей):\n";
    test("q = 13", big, std::string(99, 'a') + "#", 13);
}
