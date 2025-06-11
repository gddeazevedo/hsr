#include <asmjit/x86.h>
#include <iostream>


using Func = int (*)(void);


void test(int a = 10) {
    std::cout << "Test function called with a = " << a << std::endl;
}

int main() {
    std::string expr = "20 * 2 + 6 / 3";
    std::cout << "Expression: " << expr << std::endl;

    return 0;
}
