#include <asmjit/x86.h>
#include <iostream>

using Func = int (*)(void);

int main() {
    const uint8_t expected = 42;
    asmjit::JitRuntime rt;

    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpu_features());
    asmjit::x86::Assembler a(&code);


    // Calculando 20 * 2 + 6 / 3 
    a.mov(asmjit::x86::ebx, 20);  // mov ebx, 20
    a.shl(asmjit::x86::ebx, 1);   // shl ebx, 1 (ebx << 1) shift left by 1 bit

    a.mov(asmjit::x86::eax, 6); // mov eax, 6
    a.xor_(asmjit::x86::edx, asmjit::x86::edx); // xor edx, edx (clear edx)
    a.mov(asmjit::x86::ecx, 3); // mov ecx, 3

    a.div(asmjit::x86::ecx);   // eax = eax / ecx (resto da divisão vai pro edx)

    a.add(asmjit::x86::eax, asmjit::x86::ebx); // eax = eax + ebx (add eax, ebx)

    a.ret();

    Func fn;

    asmjit::Error err = rt.add(&fn, &code);

    if ((bool)err) {
        std::cerr << "Error: " << asmjit::DebugUtils::error_as_string(err) << std::endl;
        return 1;
    }

    int result = fn();
    std::cout << "Result: " << result << std::endl;

    if (result != expected) {
        std::cerr << "Unexpected result: " << result << std::endl;
        return 1;
    }

    rt.release(fn);

    return 0;
}
