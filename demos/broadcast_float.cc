#include <asmjit/x86.h>
#include <iostream>

using FuncType = void (*)();

int main() {
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment());

    asmjit::x86::Assembler a(&code);

    float val = 3.14f;
    asmjit::Label lbl = a.new_label();
    a.embed(&val, sizeof(val));
    a.bind(lbl);

    a.vbroadcastss(asmjit::x86::zmm2, asmjit::x86::ptr(lbl));
    a.ret();

    FuncType fn;
    rt.add(&fn, &code);

    fn();
    rt.release(fn);

    std::cout << "Executou sem segfault!" << std::endl;
}
