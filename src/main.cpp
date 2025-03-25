#include <iostream>
#include <thread>

void funcao_thread()
{
    std::cout << "Executando em uma thread separada!" << std::endl;
}

int main()
{
    std::thread t(funcao_thread);
    t.join();
    std::cout << "Thread finalizada." << std::endl;
    return 0;
}
