#include "car.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

static const char LETTERS[] = "ABEKMHOPCTYX";  // буквы, допустимые в госномере
static const char* BRANDS[] = {"Lada", "Toyota", "Kia", "Hyundai", "Skoda",
                               "Renault", "Volkswagen", "BMW", "Haval", "Geely"};
static const char* SURNAMES[] = {"Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов",
                                 "Попов", "Волков", "Козлов", "Соколов", "Морозов"};
static const char* INITIALS[] = {"А.А.", "И.С.", "П.В.", "Д.Н.", "С.М.", "Е.К."};

// Предусловие: n > 0, файл path доступен для записи.
// Постусловие: в path записано n строк "номер;марка;владелец" с уникальными
// случайными номерами.
void generateTextFile(const std::string& path, int n, unsigned seed) {
    std::mt19937 rng(seed);
    auto rnd = [&](int m) { return (int)(rng() % m); };
    std::set<std::string> used;
    std::ofstream out(path);
    while ((int)used.size() < n) {
        char number[9];
        snprintf(number, sizeof number, "%c%03d%c%c%02d", LETTERS[rnd(12)], rnd(1000),
                 LETTERS[rnd(12)], LETTERS[rnd(12)], 1 + rnd(99));
        if (!used.insert(number).second) continue;  // ключ уже есть
        out << number << ';' << BRANDS[rnd(10)] << ';'
            << SURNAMES[rnd(10)] << ' ' << INITIALS[rnd(6)] << '\n';
    }
}

// Предусловие: txtPath - файл, созданный generateTextFile.
// Постусловие: binPath содержит те же записи в виде структур Car.
void textToBinary(const std::string& txtPath, const std::string& binPath) {
    std::ifstream in(txtPath);
    std::ofstream out(binPath, std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string number, brand, owner;
        std::getline(ss, number, ';');
        std::getline(ss, brand, ';');
        std::getline(ss, owner);
        Car car{};
        strncpy(car.number, number.c_str(), sizeof car.number - 1);
        strncpy(car.brand, brand.c_str(), sizeof car.brand - 1);
        strncpy(car.owner, owner.c_str(), sizeof car.owner - 1);
        out.write((const char*)&car, sizeof car);
    }
}

// Предусловие: file открыт в двоичном режиме.
// Постусловие: возвращено число записей в файле.
long recordCount(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::end);
    return (long)file.tellg() / (long)sizeof(Car);
}

// Предусловие: file открыт в двоичном режиме, 0 <= index < recordCount(file).
// Постусловие: возвращена запись с порядковым номером index (прямой доступ).
Car readRecord(std::ifstream& file, long index) {
    Car car{};
    file.clear();
    file.seekg(index * (long)sizeof(Car));
    file.read((char*)&car, sizeof car);
    return car;
}

// Предусловие: car - заполненная запись.
// Постусловие: поля записи выведены в одну строку в std::cout.
void printCar(const Car& car) {
    std::cout << car.number << " | " << car.brand << " | " << car.owner << '\n';
}

// Предусловие: file открыт в двоичном режиме, key - строка-ключ.
// Постусловие: если запись с ключом key есть, она помещена в result и возвращен
// ее порядковый номер; иначе возвращено -1.
long linearSearch(std::ifstream& file, const char* key, Car& result) {
    file.clear();
    file.seekg(0);
    Car car;
    for (long i = 0; file.read((char*)&car, sizeof car); i++) {
        if (strcmp(car.number, key) == 0) {
            result = car;
            return i;
        }
    }
    return -1;
}

// Предусловие: file открыт в двоичном режиме.
// Постусловие: возвращена таблица (ключ, смещение) всех записей файла,
// упорядоченная по возрастанию ключа.
std::vector<IndexEntry> buildTable(std::ifstream& file) {
    std::vector<IndexEntry> table;
    file.clear();
    file.seekg(0);
    Car car;
    for (long offset = 0; file.read((char*)&car, sizeof car); offset += sizeof car) {
        IndexEntry e;
        memcpy(e.number, car.number, sizeof e.number);
        e.offset = offset;
        table.push_back(e);
    }
    std::sort(table.begin(), table.end(), [](const IndexEntry& a, const IndexEntry& b) {
        return strcmp(a.number, b.number) < 0;
    });
    return table;
}

// Предусловие: table упорядочена по возрастанию ключа.
// Постусловие: возвращено смещение записи с ключом key в файле или -1,
// если ключа в таблице нет.
long fibonacciSearch(const std::vector<IndexEntry>& table, const char* key) {
    long n = table.size();
    long f2 = 0, f1 = 1, f = 1;  // F(k-2), F(k-1), F(k)
    while (f < n) {
        f2 = f1;
        f1 = f;
        f = f1 + f2;
    }
    long shift = -1;  // элементы с индексом <= shift меньше key
    while (f > 1) {
        long i = std::min(shift + f2, n - 1);
        int cmp = strcmp(table[i].number, key);
        if (cmp == 0) return table[i].offset;
        if (cmp < 0) {  // ключ правее: отрезок длины F(k-1)
            f = f1;
            f1 = f2;
            f2 = f - f1;
            shift = i;
        } else {        // ключ левее: отрезок длины F(k-2)
            f = f2;
            f1 = f1 - f2;
            f2 = f - f1;
        }
    }
    if (f1 == 1 && shift + 1 < n && strcmp(table[shift + 1].number, key) == 0)
        return table[shift + 1].offset;
    return -1;
}

// Предусловие: file открыт в двоичном режиме, offset - смещение начала записи.
// Постусловие: возвращена запись, расположенная по смещению offset.
Car readByOffset(std::ifstream& file, long offset) {
    Car car{};
    file.clear();
    file.seekg(offset);
    file.read((char*)&car, sizeof car);
    return car;
}
