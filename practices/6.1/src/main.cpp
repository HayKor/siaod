#include <iostream>
#include <string>

#include "table.h"

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int readInt(const std::string& prompt) {
    return std::stoi(readLine(prompt));
}

int main() {
    EnterpriseTable t;

    // Автозаполнение. При m = 7: h(100002) = h(100009) = h(100016) = 0,
    // h(100005) = h(100012) = 3 - есть коллизии.
    std::cout << "Автозаполнение таблицы:\n";
    t.insert({100002, "Ромашка", "Иванов И.И."});
    t.insert({100005, "Василек", "Петров П.П."});
    t.insert({100009, "Береза", "Сидоров С.С."});
    t.insert({100012, "Ландыш", "Смирнова А.А."});
    t.insert({100016, "Клевер", "Кузнецов Д.Н."});
    t.print();

    while (true) {
        std::cout << "\n1 - вставка, 2 - удаление, 3 - поиск, 4 - вывод, "
                     "0 - выход\n";
        std::string cmd = readLine("> ");
        if (!std::cin || cmd == "0") break;

        if (cmd == "1") {
            Enterprise e;
            e.license = readInt("Номер лицензии: ");
            e.name = readLine("Название: ");
            e.founder = readLine("Учредитель: ");
            if (!t.insert(e))
                std::cout << "Ключ " << e.license << " уже есть\n";
        } else if (cmd == "2") {
            int key = readInt("Номер лицензии: ");
            std::cout << (t.remove(key) ? "Удалено\n" : "Не найдено\n");
        } else if (cmd == "3") {
            int key = readInt("Номер лицензии: ");
            const Enterprise* e = t.find(key);
            if (e)
                std::cout << "Найдено: " << e->license << " | " << e->name
                          << " | " << e->founder << '\n';
            else
                std::cout << "Не найдено\n";
        } else if (cmd == "4") {
            t.print();
        } else {
            std::cout << "Неизвестная команда\n";
        }
    }
}
