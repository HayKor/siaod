#include "table.h"

#include <cstdio>
#include <iostream>

EnterpriseTable::EnterpriseTable(int size) : table(size, nullptr) {}

EnterpriseTable::~EnterpriseTable() {
    clear();
}

// Хеш-функция методом деления: h(K) = K mod m
int EnterpriseTable::hash(int key) const {
    return key % (int)table.size();
}

// Возвращает узел с ключом key или nullptr.
// Просматривается только одна цепочка - с индексом h(key).
Node* EnterpriseTable::findNode(int key) const {
    for (Node* p = table[hash(key)]; p != nullptr; p = p->next)
        if (p->key == key) return p;
    return nullptr;
}

// Добавляет пару (key, index) в начало цепочки с индексом h(key)
void EnterpriseTable::pushFront(int key, int index) {
    int h = hash(key);
    table[h] = new Node{key, index, table[h]};
}

// Освобождает память всех узлов, все цепочки становятся пустыми
void EnterpriseTable::clear() {
    for (Node*& head : table) {
        while (head != nullptr) {
            Node* next = head->next;
            delete head;
            head = next;
        }
    }
}

// Увеличивает таблицу вдвое и заново раскладывает все ключи по цепочкам
void EnterpriseTable::rehash() {
    int m = 2 * table.size();
    std::cout << "Коэффициент нагрузки > 0.75 -> рехеширование: m = "
              << table.size() << " -> " << m << '\n';
    clear();
    table.assign(m, nullptr);
    for (int i = 0; i < (int)records.size(); i++)
        pushFront(records[i].license, i);
}

bool EnterpriseTable::insert(const Enterprise& e) {
    if (findNode(e.license) != nullptr) return false;
    int h = hash(e.license);
    std::cout << "Ключ " << e.license << ": h = " << h
              << (table[h] ? " (коллизия, в начало цепочки)" : "") << '\n';
    records.push_back(e);
    pushFront(e.license, records.size() - 1);
    if (records.size() > 0.75 * table.size()) rehash();  // n/m > 0.75
    return true;
}

bool EnterpriseTable::remove(int license) {
    // ищем узел в цепочке, запоминая предыдущий
    int h = hash(license);
    Node* prev = nullptr;
    Node* cur = table[h];
    while (cur != nullptr && cur->key != license) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == nullptr) return false;

    // исключаем узел из цепочки
    if (prev == nullptr) table[h] = cur->next;  // узел - начало цепочки
    else prev->next = cur->next;
    int idx = cur->index;
    delete cur;

    // на место удаленной записи переносим последнюю запись массива
    int last = records.size() - 1;
    if (idx != last) {
        records[idx] = records[last];
        findNode(records[idx].license)->index = idx;
    }
    records.pop_back();
    return true;
}

const Enterprise* EnterpriseTable::find(int license) const {
    Node* p = findNode(license);
    return p == nullptr ? nullptr : &records[p->index];
}

void EnterpriseTable::print() const {
    int m = table.size(), n = records.size();
    std::printf("Хеш-таблица: m = %d, записей n = %d, n/m = %.2f\n", m, n,
                (double)n / m);
    for (int h = 0; h < m; h++) {
        std::printf("%4d: ", h);
        if (table[h] == nullptr) std::cout << "nil";
        for (Node* p = table[h]; p != nullptr; p = p->next)
            std::cout << p->key << '[' << p->index << ']'
                      << (p->next ? " -> " : "");
        std::cout << '\n';
    }
    std::cout << "Массив записей:\n";
    for (int i = 0; i < n; i++)
        std::cout << "  [" << i << "] " << records[i].license << " | "
                  << records[i].name << " | " << records[i].founder << '\n';
}
