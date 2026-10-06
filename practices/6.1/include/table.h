#pragma once

#include <string>
#include <vector>

// Запись: регистрация малого предприятия. Ключ - номер лицензии.
struct Enterprise {
    int license;          // номер лицензии
    std::string name;     // название
    std::string founder;  // учредитель
};

// Элемент цепочки (однонаправленного списка): ключ и индекс записи в массиве
struct Node {
    int key;     // номер лицензии
    int index;   // индекс записи в массиве records
    Node* next;  // следующий элемент цепочки или nullptr
};

// Хеш-таблица с цепным хешированием
class EnterpriseTable {
public:
    EnterpriseTable(int size = 7);
    ~EnterpriseTable();
    // копирование запрещено: две таблицы освободили бы одни и те же узлы
    EnterpriseTable(const EnterpriseTable&) = delete;
    EnterpriseTable& operator=(const EnterpriseTable&) = delete;

    bool insert(const Enterprise& e);           // false, если ключ уже есть
    bool remove(int license);                   // false, если ключа нет
    const Enterprise* find(int license) const;  // nullptr, если ключа нет
    void print() const;

private:
    std::vector<Enterprise> records;  // массив полезных данных
    std::vector<Node*> table;         // хеш-таблица: указатели на цепочки

    int hash(int key) const;
    Node* findNode(int key) const;
    void pushFront(int key, int index);
    void clear();
    void rehash();
};
