#include <chrono>
#include <cstdio>
#include <iostream>

#include "car.h"

using Clock = std::chrono::steady_clock;

// Среднее время выполнения f за repeats запусков, мкс
template <class F>
double measure(F f, int repeats) {
    auto start = Clock::now();
    for (int i = 0; i < repeats; i++) f();
    std::chrono::duration<double, std::micro> d = Clock::now() - start;
    return d.count() / repeats;
}

std::string makeFile(int n) {
    std::string base = "data/cars_" + std::to_string(n);
    generateTextFile(base + ".txt", n, n);
    textToBinary(base + ".txt", base + ".bin");
    return base + ".bin";
}

int main(int argc, char** argv) {
    const char* missing = "X000XX00";
    std::string path = makeFile(100);
    std::ifstream file(path, std::ios::binary);

    std::cout << "== Задание 1 ==\n";
    std::cout << "Файл " << path << ": записей " << recordCount(file)
              << ", размер записи " << sizeof(Car) << " байт\n";
    for (long i : {0L, 1L, 2L, 3L, 4L, 99L}) {
        std::cout << '[' << i << "] ";
        printCar(readRecord(file, i));
    }

    // ключ задается аргументом, иначе берется ключ записи с номером 57
    std::string key = argc > 1 ? argv[1] : readRecord(file, 57).number;

    std::cout << "\n== Задание 2 ==\n";
    for (const char* k : {key.c_str(), missing}) {
        Car car;
        long pos = linearSearch(file, k, car);
        std::cout << "Ключ " << k << ": ";
        if (pos < 0) std::cout << "не найден\n";
        else {
            std::cout << "запись " << pos << " -> ";
            printCar(car);
        }
    }

    std::cout << "\n== Задание 3 ==\n";
    auto table = buildTable(file);
    std::cout << "Таблица (первые 3 из " << table.size() << "):\n";
    for (int i = 0; i < 3; i++)
        std::cout << "  " << table[i].number << " -> " << table[i].offset << '\n';
    for (const char* k : {key.c_str(), missing}) {
        long offset = fibonacciSearch(table, k);
        std::cout << "Ключ " << k << ": ";
        if (offset < 0) std::cout << "не найден\n";
        else {
            std::cout << "смещение " << offset << " -> ";
            printCar(readByOffset(file, offset));
        }
    }

    std::cout << "\n== Замеры времени (ключ последней записи файла), мкс ==\n";
    std::cout << "       N     линейный      Фибоначчи  Фибоначчи+чтение     таблица\n";
    for (int n : {100, 1000, 10000}) {
        std::ifstream f(makeFile(n), std::ios::binary);
        std::string last = readRecord(f, n - 1).number;
        Car car;
        auto t = buildTable(f);
        double tLinear = measure([&] { linearSearch(f, last.c_str(), car); }, 200);
        double tFib = measure([&] { fibonacciSearch(t, last.c_str()); }, 100000);
        double tFibRead = measure([&] { readByOffset(f, fibonacciSearch(t, last.c_str())); }, 10000);
        double tBuild = measure([&] { buildTable(f); }, 50);
        std::printf("%8d %12.3f %14.3f %17.3f %11.3f\n", n, tLinear, tFib, tFibRead, tBuild);
    }
}
