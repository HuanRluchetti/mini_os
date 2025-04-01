#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

class Process(char pid) {
    public:
         int pid;
         int total_time;
         int remaning_time;
         char state; // R pronto | E executanto | B bloqueado | F finalizado

         Process ( int pid, int total_time) : pid(id), total_time(time), remaning_time(time), state("R") {}

         void execute(int quantum, atomic<bool>&runing) {
             state = "E";
             std::cout << "Processo" << pid << " executando em " << quantum << "quantum\n";

             auto start = high_resolution_clock::now();

             while (duration_cast<milliseconds>(high_resolution_clock::now() - start().count() < quantum * 100 && running)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
             }

             remaning_time -= quantum;

             if (remaning_time <= 0) {
                 state = "F";
                 std::cout << "Execucao do processo " << pid << " finalizado\n";
             } else {
                 state = "R";
                 std::cout << "Execucao do processo " << pid << " pausado, tempo restante: " remaning_time << "\n";
             }
         }
};

class Scheduler {
    private:
        queue<Process*> ready_queue;
        mutex mtx;
        atomic<bool>&runing;
        thread scheduler_thread;
        int quantum;
        condition_variable cv;

    void schedule() {
        while(running) {
            unique_lock<std::mutex> lock(mtx);

            if (ready_queue.empty()) {
                cv.wait_for(lock, std::chrono::milliseconds(100));
                continue;
            }

            Process* current = ready_queue.front();
            ready_queue.pop();
            lock.unlock()

            current->execute(quantum, running);

            lock.lock();
            if (current->state == "R") {
                ready_queue.push(current);
            }
            lock.unlock();
        }
    }

    public:
        Scheduler(int q) : quantum(q), running(false) {}

        ~Scheduler {
            stop();
        }

        void add_process(Process* p) {
            lock_guard<std::mutex> lock(mtx);
            ready_queue.push(p);
           
            std::cout << "Processo adicionado " << p->pid << " ao scheduler\n";
           
            cv.notify_one();
        }

        void start() {
            if (!running) {
                running = true;
                scheduler_thread = thread(&Scheduler::schedule, this);
                std::cout << "Scheduler comecou com o quantum = " << quantum << "\n";
            }
        }

        void stop() {
            if(running) {
                running = false;
                cv.notify_all();
                if (scheduler_thread.joinable()) {
                    scheduler_thread.join();
                }
                std::cout << "Scheduler parado\n";
            }
        }

        bool is_running() const {
            return running;
        }
}


int main() {
    Scheduler scheduler(2); // Quantum de 2 unidades de tempo

    vector<Process> process = {
        Process(1, 5),  // Pid 1 tempo total 5
        Process(2, 3),
        Process(3, 4),
        Process(4, 2)
    }

    scheduler.start();

    for (auto& p : process) {
        scheduler.add_process(&p);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::this_thread::sleep_for(std::chrono::seconds(15));

    scheduler.stop();

    return 0;
}