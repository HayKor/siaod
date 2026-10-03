#pragma once

#include <fstream>
#include <string>
#include <vector>

// Запись файла: владелец автомобиля. Ключ - номер машины.
struct Car {
    char number[9];  // госномер, например "A123BC77"
    char brand[16];  // марка
    char owner[48];  // сведения о владельце (ФИО)
};

// Элемент таблицы: ключ и смещение записи в файле
struct IndexEntry {
    char number[9];
    long offset;
};

// Задание 1
void generateTextFile(const std::string& path, int n, unsigned seed);
void textToBinary(const std::string& txtPath, const std::string& binPath);
long recordCount(std::ifstream& file);
Car readRecord(std::ifstream& file, long index);
void printCar(const Car& car);

// Задание 2
long linearSearch(std::ifstream& file, const char* key, Car& result);

// Задание 3
std::vector<IndexEntry> buildTable(std::ifstream& file);
long fibonacciSearch(const std::vector<IndexEntry>& table, const char* key);
Car readByOffset(std::ifstream& file, long offset);
