#include <asmjit/x86.h>
#include <iostream>
#include <memory>
#include <vector>


int main() {
    using Func = void(*)(const float*, float*, size_t);

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpuFeatures());
    asmjit::x86::Assembler a(&code);

    asmjit::x86::Gp input_ptr = asmjit::x86::rdi; // Gp stands for general purpose register
    asmjit::x86::Gp output_ptr = asmjit::x86::rsi;
    asmjit::x86::Gp size = asmjit::x86::rdx;
    asmjit::x86::Gp index = asmjit::x86::rcx;

    // em avx podemos armazenar 8 floats num registrador ymm 256 bits
    float ten[8] = {10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f};
    float two[8] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};

    // moving tens to ymm0
    a.mov(asmjit::x86::rax, reinterpret_cast<uintptr_t>(ten));
    a.vmovups(asmjit::x86::ymm0, asmjit::x86::ptr(asmjit::x86::rax));

    // moving twos to ymm1
    a.mov(asmjit::x86::rax, reinterpret_cast<uintptr_t>(two));
    a.vmovups(asmjit::x86::ymm1, asmjit::x86::ptr(asmjit::x86::rax));

    // index = rcx (registrador de contador)
    // forçando inicilizar para zero A ^ A = 0 para qualquer A
    a.xor_(index, index);

    // asmjit::Label loop = a.newLabel(); // loops serão usados para processar mais de 8 floats
    // a.bind(loop);

    // // Carregar 8 floats e calcular 10*x + 2
    a.vmovups(asmjit::x86::ymm2, asmjit::x86::ptr(input_ptr, index, 3)); // movendo x para ymm2
    a.vmulps(asmjit::x86::ymm2, asmjit::x86::ymm2, asmjit::x86::ymm0); // x * 10; vmulps ymm2, ymm2, ymm0 === ymm2 = tens * ymm2
    a.vaddps(asmjit::x86::ymm2, asmjit::x86::ymm2, asmjit::x86::ymm1); // + 2
    a.vmovups(asmjit::x86::ptr(output_ptr, index, 3), asmjit::x86::ymm2);

    // a.add(index, 8); // index += 8
    // a.cmp(index, size); // compara index com size
    // a.jb(loop); // se index < size, volta para o loop

    a.ret();

    Func func;
    rt.add(&func, &code);

    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> output(input.size());

    func(input.data(), output.data(), input.size());

    std::cout << "Resultados 10x + 2:\n";
    for (size_t i = 0; i < input.size(); i++) {
        std::cout << input[i] << " -> " << output[i] << std::endl;
    }
    
    rt.release(func);

    return 0;
}

