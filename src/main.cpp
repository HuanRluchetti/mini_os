#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <vector>

using namespace std;
using namespace std::chrono;

class Process {
public:
    int pid;
    int total_time;
    int remaining_time;
    char state; // R - pronto | E - executando | B - bloqueado | F - finalizado

    Process(int id, int time) : pid(id), total_time(time), remaining_time(time), state('R') {}

    void execute(int quantum, atomic<bool> &running) {
        state = 'E';
        cout << "Processo " << pid << " executando por " << quantum << " quantum\n";

        auto start = high_resolution_clock::now();

        while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < quantum * 100 && running) {
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        remaining_time -= quantum;

        if (remaining_time <= 0) {
            state = 'F';
            cout << "Execucao do processo " << pid << " finalizada\n";
        } else {
            state = 'R';
            cout << "Execucao do processo " << pid << " pausada, tempo restante: " << remaining_time << "\n";
        }
    }
};

class Scheduler {
private:
    queue<Process *> ready_queue;
    mutex mtx;
    atomic<bool> running;
    thread scheduler_thread;
    int quantum;
    condition_variable cv;

    void schedule() {
        while (running) {
            unique_lock<mutex> lock(mtx);

            if (ready_queue.empty()) {
                cv.wait_for(lock, chrono::milliseconds(100));
                continue;
            }

            Process *current = ready_queue.front();
            ready_queue.pop();
            lock.unlock();

            current->execute(quantum, running);

            lock.lock();
            if (current->state == 'R') {
                ready_queue.push(current);
            }
            lock.unlock();
        }
    }

public:
    Scheduler(int q) : quantum(q), running(false) {}

    ~Scheduler() {
        stop();
    }

    void add_process(Process *p) {
        lock_guard<mutex> lock(mtx);
        ready_queue.push(p);

        cout << "Processo " << p->pid << " adicionado ao scheduler\n";

        cv.notify_one();
    }

    void start() {
        if (!running) {
            running = true;
            scheduler_thread = thread(&Scheduler::schedule, this);
            cout << "Scheduler comecou com quantum = " << quantum << "\n";
        }
    }

    void stop() {
        if (running) {
            running = false;
            cv.notify_all();
            if (scheduler_thread.joinable()) {
                scheduler_thread.join();
            }
            cout << "Scheduler parado\n";
        }
    }

    bool is_running() const {
        return running;
    }
};

int main() {
    Scheduler scheduler(2); // Quantum de 2 unidades de tempo

    vector<Process> processes = {
        Process(1, 5),  // Pid 1, tempo total 5
        Process(2, 3),
        Process(3, 4),
        Process(4, 2)
    };

    scheduler.start();

    for (auto &p : processes) {
        scheduler.add_process(&p);
        this_thread::sleep_for(chrono::seconds(1));
    }

    this_thread::sleep_for(chrono::seconds(15));

    scheduler.stop();

    return 0;
}

// teste git