#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <locale.h>

using namespace std;

class Monitor {
private:
    mutex mtx;
    condition_variable cv;
    bool event_ready = false;
    shared_ptr<string> event_data; // Пример несериализуемых данных

public:
    // Функция поставщика
    void provide() {
        for (int i = 1; i <= 5; ++i) { // 5 событий для примера
            // Задержка 1 секунда
            this_thread::sleep_for(chrono::seconds(1));

            // Создаем "несериализуемые" данные
            event_data = make_shared<string>("Event data " + to_string(i));

            {
                lock_guard<mutex> lock(mtx);
                event_ready = true;
                cout << "Поставщик: отправил событие '" << *event_data << "'" << endl;
            }

            // Уведомляем потребителя
            cv.notify_one();
        }
    }

    // Функция потребителя
    void consume() {
        for (int i = 1; i <= 5; ++i) {
            unique_lock<mutex> lock(mtx);

            // Ожидание события с временным освобождением мьютекса
            cv.wait(lock, [this]() { return event_ready; });

            // Обработка события
            cout << "Потребитель: получил событие '" << *event_data << "'" << endl;
            event_ready = false;

            // Мьютекс автоматически освобождается при выходе из scope
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    Monitor monitor;

    // Запускаем потоки
    thread producer_thread(&Monitor::provide, &monitor);
    thread consumer_thread(&Monitor::consume, &monitor);

    // Ждем завершения потоков
    producer_thread.join();
    consumer_thread.join();

    return 0;
}