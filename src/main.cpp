#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex m1, m2;

bool try_lock_with_timeout(std::mutex& m, std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (!m.try_lock()) {
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return true;
}

auto f1 = [&m1, &m2]() {
    std::unique_lock<std::mutex> ul1(m1, std::defer_lock);
    if (ul1.try_lock()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (try_lock_with_timeout(m2, std::chrono::milliseconds(100))) {
            std::unique_lock<std::mutex> ul2(m2, std::adopt_lock);  // Assume o lock já adquirido
            std::cout << "f1: locks adquiridos com sucesso" << std::endl;
        } else {
            std::cout << "f1: DEADLOCK DETECTADO - não conseguiu m2" << std::endl;
            ul1.unlock();
        }
    } else {
        std::cout << "f1: não conseguiu m1" << std::endl;
    }
};

auto f2 = [&m1, &m2]() {
    std::unique_lock<std::mutex> ul1(m2, std::defer_lock);
    if (ul1.try_lock()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (try_lock_with_timeout(m1, std::chrono::milliseconds(100))) {
            std::unique_lock<std::mutex> ul2(m1, std::adopt_lock);
            std::cout << "f2: locks adquiridos com sucesso" << std::endl;
        } else {
            std::cout << "f2: DEADLOCK DETECTADO - não conseguiu m1" << std::endl;
            ul1.unlock();
        }
    } else {
        std::cout << "f2: não conseguiu m2" << std::endl;
    }
};

int main() {
    std::thread t1(f1);
    std::thread t2(f2);

    t1.join();
    t2.join();

    return 0;
}