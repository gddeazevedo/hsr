#include <asmjit/x86.h>
#include <iostream>
#include <vector>
#include <cpuid.h>
#include <cstring>

bool cpu_has_avx512f();

int main() {
    using FuncUInt32_t = void(*)(const uint32_t*);

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpu_features());
    asmjit::x86::Assembler a(&code);

    // MOVING BROADCAST COM AVX-512 INTEGER
    a.mov(asmjit::x86::eax, 5); // mov eax, 5
    a.vpbroadcastd(asmjit::x86::zmm1, asmjit::x86::eax); // vpbroadcastd zmm1, eax; move o valor de eax para todos os elementos de zmm1
    
    // usado apenas para verificar o resultado do broadcast
    a.vmovdqu(asmjit::x86::ptr(asmjit::x86::rdi), asmjit::x86::zmm1); // vmovdqu [rdi], zmm1; armazena o conteúdo de zmm1 no endereço apontado por rdi

    a.ret();

    FuncUInt32_t fn;

    rt.add(&fn, &code);

    std::vector<uint32_t> output(16, 0); // vetor de 16 inteiros de 32 bits inicializados com 0

    // out.data() devolve um ponteiro para o primeiro elemento do vetor
    std::memset(output.data(), 0, output.size() * sizeof(uint32_t));

    fn(output.data());

    std::cout << "EXEMPLO COM INTEIROS\n";
    std::cout << "zmm1 lane after broadcast (should all be 5):\n";
    for (size_t i = 0; i < output.size(); i++) {
        std::cout << "lane[" << i << "] = " << output[i] << "\n";
    }
    std::cout << std::endl;
    rt.release(fn);

    return 0;
}


bool cpu_has_avx512f() {
    uint32_t eax, ebx, ecx, edx;
    // cpuid devolve as informações do processador nos registradores eax, ebx, ecx, edx
    // 7 é o leaf para features estendidas
    // verificamos se o bit 16 do ebx está setado, que indica suporte ao AVX-512F
    if (!__get_cpuid(7, &eax, &ebx, &ecx, &edx))
        return false;
    // verifica o bit 16 do ebx
    // 1 << 16 cria uma máscara com apenas o bit 16 setado
    // o operador & faz uma operação AND bit a bit entre ebx e a máscara
    // se o resultado for diferente de zero, o bit 16 está setado
    // AVX-512 é suportado se o bit 16 estiver setado
    return (ebx & (1 << 16)) != 0;
}
